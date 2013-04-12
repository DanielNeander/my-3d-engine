#include "stdafx.h"
#include "Point.h"
#include "ClipMap.h"
#include "Dx11Resource.h"

using namespace SpeedTree;

struct AltasVert2 
{
	noVec3 point;
	noVec3 normal;
	noVec2 texCoord;
	noVec2 texMorphOffset;
	noVec3 pointMorphOffset;
};

U32 ClipMap::getMipLevel( F32 scale )
{
	return noMath::Log( scale ) + noMath::Log( mClipMapSize );
}

void ClipMap::clipAgainstGrid( const S32 gridSpacing, const RectI &rect, S32 *outCount, RectI *outBuffer )
{
	// Check against X grids...
	const S32 startX = rect.point.x;
	const S32 endX   = rect.point.x + rect.extent.x;

	const S32 gridMask = ~(gridSpacing-1);
	const S32 startGridX = startX & gridMask;
	const S32 endGridX   = endX   & gridMask;

	RectI buffer[2];
	S32 rectCount = 0;

	// Check X...
	if(startGridX != endGridX && endX - endGridX > 0)
	{
		// We have a clip! Split against the grid multiple and store.
		rectCount = 2;
		buffer[0].point.x  = startX;
		buffer[0].point.y  = rect.point.y;
		buffer[0].extent.x = endGridX - startX;
		buffer[0].extent.y = rect.extent.y;

		buffer[1].point.x  = endGridX;
		buffer[1].point.y  = rect.point.y;
		buffer[1].extent.x = endX - endGridX;
		buffer[1].extent.y = rect.extent.y;
	}
	else
	{
		// Copy it in.
		rectCount = 1;
		buffer[0] = rect;
	}

	// Now, check Y for the one or two rects we have from above.
	*outCount = 0;
	for(S32 i=0; i<rectCount; i++)
	{
		// Figure our extent and grid information.
		const S32 startY = buffer[i].point.y;
		const S32 endY   = buffer[i].point.y + rect.extent.y;
		const S32 startGridY = startY & gridMask;
		const S32 endGridY   = endY   & gridMask;

		if(startGridY != endGridY && endY - endGridY > 0)
		{
			// We have a clip! Split against the grid multiple and store.
			RectI *buffA = outBuffer + *outCount;
			RectI *buffB = buffA + 1;
			(*outCount) += 2;

			buffA->point.x  = buffer[i].point.x;
			buffA->point.y  = startY;
			buffA->extent.x = buffer[i].extent.x;
			buffA->extent.y = endGridY - startY;

			buffB->point.x  = buffer[i].point.x;
			buffB->point.y  = endGridY;
			buffB->extent.x = buffer[i].extent.x;
			buffB->extent.y = endY - endGridY;
		}
		else
		{
			// Copy it in.
			outBuffer[*outCount] = buffer[i];
			(*outCount)++;
		}      
	}
}

ClipMap::ClipMap()
{
	// Start with some modest requirements.
	mClipMapSize = 512;

	mMaxTexelUploadPerRecenter = mClipMapSize * mClipMapSize * 2;

#ifdef DEBUG
	mMaxTexelUploadPerRecenter /= 2;
#endif

	mClipStackDepth = 0;

	// Use an 8k texture by default.
	mTextureSize = 1 << 13;

	mImageCache = NULL;

	mTile = false;

	mTotalUpdates = mTexelsUpdated = 0;

	// Set shaders to NULL initially.
	for(S32 i=0; i<4; i++)
	{
		mClipmapMat[i] = NULL;
	}

	mClipmapMatBasePassFF = NULL;
	mClipmapMatAddPassFF = NULL;

	mTexCallbackHandle = -1;

	mForceClipmapPurge = false;
	mNeedRefill = false;

}

ClipMap::~ClipMap()
{
	SAFE_DELETE(mImageCache);


	for (S32 i=0; i < 4; i++)
	{
		SAFE_DELETE(mClipmapMat[i]);      
	}

	SAFE_DELETE(mClipmapMatBasePassFF);
	SAFE_DELETE(mClipmapMatAddPassFF);   

}

static const char* szShaderFilename[3] = {



};

void ClipMap::initClipStack()
{
	while(mLevels.size())
	{
		mLevels.back().mDebugTex = NULL;
		mLevels.back().mTex = NULL;
		mLevels.pop_back();
	}

	// Figure out how many clipstack textures we'll have.
	mClipStackDepth = noMath::Log(mTextureSize) - noMath::Log(mClipMapSize) + 1;
	mLevels.resize(mClipMapSize);

	// Print a little report on our allocation.
	//Con::printf("Allocating a %d px clipmap for a %dpx source texture.", mClipMapSize, mTextureSize);
	//Con::printf("   - %d base clipstack entries, + 1 cap.", mClipStackDepth - 1);

	U32 baseTexSize = (mClipMapSize * mClipMapSize * 4);
	//Con::printf("   - Using approximately %fMB of texture memory.", 
	//	(F32(baseTexSize * mClipStackDepth) * 1.33) / (1024.0*1024.0));
	// First do our base textures - they are not mipped.
	// We rely on auto-mipmapping, but if the device/card doesn't support it, we should just not ask for it.
	//U32 numMips = GFX->getCardProfiler()->queryProfile("autoMipMapLevel", true) ? 0 : 1;
	U32 numMips = 0;

	for(S32 i=0; i<mClipStackDepth; i++)
	{
		mLevels[i].mScale = (F32)BIT(mClipStackDepth - (1 + i));
		//mLevels[i].mTex.set(mClipMapSize, mClipMapSize, GFXFormatR8G8B8X8, texProfile, avar("%s() - mLevels[%d].mTex (line %d)", __FUNCTION__, i, __LINE__), numMips);
	}

	// Don't forget to allocate our debug textures...
	for(S32 i=0; i<mClipStackDepth; i++)
		mLevels[i].initDebugTexture(i);

	// Do shader lookup for 2,3,4 level shaders.
	for(S32 i=2; i<5; i++)
	{		
		// Init materials
		const U32 arrayOffset = i-1;		
		mClipmapMat[arrayOffset] = new CShaderLoader();
		CArray<CString> defines;
		mClipmapMat[arrayOffset]->Load(szShaderFilename[i-2], defines);
		CShaderLoader* matParams = mClipmapMat[arrayOffset];
		matParams->GetConstant("morphT", mMorphTSC[arrayOffset]);
		matParams->GetConstant("mapInfo", mMapInfoTC[arrayOffset]);
		matParams->GetConstant("diffuseMap0", mDiffuseMap0TC[arrayOffset]);
		matParams->GetConstant("diffuseMap1", mDiffuseMap1TC[arrayOffset]);
		matParams->GetConstant("diffuseMap2", mDiffuseMap2TC[arrayOffset]);
		matParams->GetConstant("diffuseMap3", mDiffuseMap3TC[arrayOffset]);		
	}
}

void ClipMap::fillWithTestPattern()
{
	//AssertISV(false, "ClipMap::fillWithTestPattern - assumes bitmaps, which "
	//	"are no longer present, switch to lock() semantics if you need this!");

	// Table of random colors.
	U8 colorTbl[16][3] = 
	{
		{ 0xFF, 0x00, 0x0F },
		{ 0xFF, 0x00, 0xA0 },
		{ 0xFF, 0x00, 0xFF },
		{ 0x00, 0xA0, 0x00 },
		{ 0x00, 0xA0, 0xAF },
		{ 0x00, 0xA0, 0xF0 },
		{ 0xA0, 0xFF, 0xA0 },
		{ 0x00, 0xF0, 0xA0 },
		{ 0x00, 0xF0, 0xFF },
		{ 0xA0, 0x00, 0x00 },
		{ 0xA0, 0x00, 0xAF },
		{ 0xA0, 0x00, 0xF0 },
		{ 0xA0, 0xF0, 0x0F },
		{ 0xA0, 0xF0, 0xA0 },
		{ 0xA0, 0xF0, 0xFF },
		{ 0x00, 0xFF, 0x00 },
	};

	// Lock each layer of each texture and write a test pattern in.

	// Base levels first.
	for(S32 i=0; i<mClipStackDepth; i++)
	{
		GFXTexHandle &gth = mLevels[i].mTex;
		U8 *bmpData = (U8*)gth.getBitmap()->getWritableBits(0);

		for(S32 x=0; x<gth.getHeight(); x++)
		{
			for(S32 y=0; y<gth.getWidth(); y++)
			{
				S32 xFlag = x & 4;
				S32 yFlag = y & 4;

				U32 offset = (x * gth.getWidth() + y) * 4;
				if(xFlag ^ yFlag)
				{
					// Set bright.
					bmpData[offset+0] = colorTbl[i][0];
					bmpData[offset+1] = colorTbl[i][1];
					bmpData[offset+2] = colorTbl[i][2];
					bmpData[offset+3] = 0xFF;
				}
				else
				{
					// Set dim.
					bmpData[offset+0] = colorTbl[i][0] / 3;
					bmpData[offset+1] = colorTbl[i][1] / 3;
					bmpData[offset+2] = colorTbl[i][2] / 3;
					bmpData[offset+3] = 0xFF;
				}
			}
		}

		if(i == mClipStackDepth - 1)
			gth.getBitmap()->extrudeMipLevels();
		else
		{
			// Write black/translucent in the higher levels.
			GBitmap *gb = gth.getBitmap();
			for(S32 j=1; j<gb->getNumMipLevels(); j++)
			{
				U8 *b = gb->getWritableBits(j);
				memset(b, 0, 4 * gb->getWidth(j) * gb->getHeight(j));
			}
		}

		gth.refresh();
	}
}

bool ClipMap::fillWithTextureData()
{
	mForceClipmapPurge = false;
	// Note our interest for the cache.
	{
		// Calculate the new texel at most detailed level.
		Point2F texelCenterF = mLevels[0].mClipCenter * F32(mClipMapSize) * mLevels[0].mScale;
		Point2I texelCenter((S32)mFloor(texelCenterF.y), (S32)mFloor(texelCenterF.x));

		// Update interest region.
		mImageCache->setInterestCenter(texelCenter);
	}

	// First generate our desired rects for each level.
	FrameTemp< RectI > desiredData( mClipStackDepth );
	for(S32 i=0; i<mClipStackDepth; i++)
	{
		ClipStackEntry &cse = mLevels[i];

		// Calculate new center point for this texture.
		Point2F texelCenterF = cse.mClipCenter * F32(mClipMapSize) * cse.mScale;

		const S32 texelMin = mClipMapSize/2;
		//const S32 texelMax = S32(F32(mClipMapSize) * cse.mScale) - texelMin;

		Point2I texelTopLeft;

		//if(mTile)
		//{
		texelTopLeft.x = S32(mFloor(texelCenterF.y)) - texelMin;
		texelTopLeft.y = S32(mFloor(texelCenterF.x)) - texelMin;
		//}
		//else
		//{
		//   texelTopLeft.x = mClamp(S32(mFloor(texelCenterF.y)), texelMin, texelMax) - texelMin;
		//   texelTopLeft.y = mClamp(S32(mFloor(texelCenterF.x)), texelMin, texelMax) - texelMin;
		//}

		desiredData[i] = RectI(texelTopLeft, Point2I(mClipMapSize, mClipMapSize));

		// Make sure we have available data...
		if(!mImageCache->isDataAvailable(getMipLevel(cse.mScale), desiredData[i]))
			return false;
	}

	// Upload all the textures...
	for(S32 i=0; i<mClipStackDepth; i++)
	{
		ClipStackEntry &cse = mLevels[i];

		RectI buffer[8];
		S32   rectCount = 0;

		clipAgainstGrid(mClipMapSize, desiredData[i], &rectCount, buffer);
		//AssertFatal(rectCount < 8, "ClipMap::fillWithTextureData - got too many rects back!");

		if(rectCount)
		{
			if (!mImageCache->beginRectUpdates(cse))
			{
				mForceClipmapPurge = true;
				return false;
			}

			for(S32 j=0; j<rectCount; j++)
			{
				RectI srcRegion = buffer[j];
				buffer[j].point.x = srcRegion.point.x % mClipMapSize;
				buffer[j].point.y = srcRegion.point.y % mClipMapSize;

				mImageCache->doRectUpdate(getMipLevel(cse.mScale), cse, srcRegion, buffer[j]);
			}

			mImageCache->finishRectUpdates(cse);
		}

		cse.mToroidalOffset  = desiredData[i].point;
	}

	//GFX->disableShaders();

	// Success!
	return true;
}

void ClipMap::recenter( Point2F center )
{
	bool wantCompleteRefill = false;
   if(mNeedRefill || mForceClipmapPurge )
      wantCompleteRefill = true;

   //PROFILE_START(ClipMap_recenter);

   // Reset our budget.
   mMaxTexelUploadPerRecenter = mClipMapSize * mClipMapSize * 2;

   //AssertFatal(isPow2(mClipMapSize),  "ClipMap::recenter - require pow2 clipmap size!");

   // Clamp the center to the unit square.

   /*
   if(!mTile)
   {
      center.x = mClampF(center.x, 0.f, 1.f);
      center.y = mClampF(center.y, 0.f, 1.f);
   }
   */

   // Ok, we're going to do toroidal updates on each entry of the clipstack
   // (except for the cap, which covers the whole texture), based on this
   // new center point.
   if( !wantCompleteRefill )
   {
      // Calculate the new texel at most detailed level.
      Point2F texelCenterF = center * F32(mClipMapSize) * mLevels[0].mScale;
      Point2I texelCenter((S32)mFloor(texelCenterF.y), (S32)mFloor(texelCenterF.x));
      
      // Update interest region.
      mImageCache->setInterestCenter(texelCenter);
   }

   // Note how many we were at so we can cut off at the right time.
   S32 lastTexelsUpdated = mTexelsUpdated;

   // For each texture...
   for(S32 i=mClipStackDepth-2; i>=0; i--)
   {
      ClipStackEntry &cse = mLevels[i];

      // Calculate new center point for this texture.
      Point2F texelCenterF = center * F32(mClipMapSize) * cse.mScale;

      const S32 texelMin = mClipMapSize/2;
      //const S32 texelMax = S32(F32(mClipMapSize) * cse.mScale) - texelMin;

      Point2I texelTopLeft;

      //if(mTile)
      //{
         texelTopLeft.x = S32(mFloor(texelCenterF.y)) - texelMin;
         texelTopLeft.y = S32(mFloor(texelCenterF.x)) - texelMin;
      //}
      //else
      //{
      //   texelTopLeft.x = mClamp(S32(mFloor(texelCenterF.y)), texelMin, texelMax) - texelMin;
      //   texelTopLeft.y = mClamp(S32(mFloor(texelCenterF.x)), texelMin, texelMax) - texelMin;
      //}

      // Also, prevent very small updates - the RT changes are costly.
      Point2I d = cse.mToroidalOffset - texelTopLeft;
      if(mAbs(d.x) <= 2 && mAbs(d.y) <= 2)
      {
         // Update the center; otherwise we get some weird conditions around
         // edges of the clipmap space.
         cse.mClipCenter = center;
         continue;
      }

      // This + current toroid offset tells us what regions have to be blasted.
      RectI oldData(cse.mToroidalOffset,  Point2I(mClipMapSize, mClipMapSize));
      RectI newData(texelTopLeft,         Point2I(mClipMapSize, mClipMapSize));

      // Update clipstack level.
      cse.mClipCenter      = center;
      cse.mToroidalOffset  = texelTopLeft;

      // If we're refilling, that's all we want; continue with next level.
      if( wantCompleteRefill )
         continue;

      // Make sure we have available data...
      if(!mImageCache->isDataAvailable(getMipLevel(cse.mScale), newData))
		  continue;

      // Alright, determine the set of data we actually need to upload.
      S32   rectCount = 0;
      RectI buffer[8];

      calculateModuloDeltaBounds(oldData, newData, buffer, &rectCount);
      //AssertFatal(rectCount < 8, "ClipMap::recenter - got too many rects back!");

      /*if(rectCount)
         Con::printf("    issuing %d updates to clipmap level %d (offset=%dx%d)", 
                        rectCount, i, texelTopLeft.x, texelTopLeft.y); */

      if(rectCount)
      {
         if (!mImageCache->beginRectUpdates(cse))
         {
            mForceClipmapPurge = true;
            return;
         }
         //Con::errorf("layer %x, %d updates", &cse,  rectCount);

         // And GO!
         for(S32 j=0; j<rectCount; j++)
         {
            PROFILE_START(ClipMap_recenter_upload);

            AssertFatal(buffer[j].isValidRect(),"ClipMap::recenter - got invalid rect!");

            // Note the rect, so we can then wrap and let the image cache do its thing.
            RectI srcRegion = buffer[j];
            buffer[j].point.x = srcRegion.point.x % mClipMapSize;
            buffer[j].point.y = srcRegion.point.y % mClipMapSize;

            //AssertFatal(newData.contains(srcRegion), 
			//	"ClipMap::recenter - got update buffer outside of expected new data bounds.");

            mTotalUpdates++;
            mTexelsUpdated += srcRegion.extent.x  * srcRegion.extent.y;

            //Con::printf("updating (%d %d %d %d)",
            //   buffer[j].point.x, buffer[j].point.y, buffer[j].extent.x, buffer[j].extent.y);

            mImageCache->doRectUpdate(getMipLevel(cse.mScale), cse, srcRegion, buffer[j]);

            PROFILE_END();
         }

         mImageCache->finishRectUpdates(cse);
      }

      // Check if we've overrun our budget.
      if((mTexelsUpdated - lastTexelsUpdated) > mMaxTexelUploadPerRecenter)
      {
         //Con::warnf("ClipMap::recenter - exceeded budget for this frame, deferring till next frame.");
         break;
      }

   }

   if( wantCompleteRefill )
   {
      fillWithTextureData();
      mNeedRefill = false;
   }

   //PROFILE_END();
}

void ClipMap::calculateModuloDeltaBounds( const RectI &oldData, const RectI &newData, RectI *outRects, S32 *outRectCount )
{
	if(oldData.point == newData.point)
	{
		*outRectCount = 0;
		return;
	}

	// Easy case - if there's no overlap then it's all new!
	if(!oldData.overlaps(newData))
	{
		// Clip out to return buffer, and we're done.
		clipAgainstGrid(mClipMapSize, newData, outRectCount, outRects);
		return;
	}

	// Calculate some useful values for both X and Y. Delta is used a lot
	// in determining bounds, and the boundary values are important for
	// determining where to start copying new data in.
	const S32 xDelta = newData.point.x - oldData.point.x;
	const S32 yDelta = newData.point.y - oldData.point.y;

	const S32 xBoundary = (oldData.point.x + oldData.extent.x) % mClipMapSize;
	const S32 yBoundary = (oldData.point.y + oldData.extent.y) % mClipMapSize;

	//AssertFatal(xBoundary % mClipMapSize == oldData.point.x % mClipMapSize, 
	//	"ClipMap::calculateModuleDeltaBounds - we assume that left and "
	//	"right of the dataset are identical (ie, it's periodical on size of clipmap!) (x)");

	//AssertFatal(yBoundary % mClipMapSize == oldData.point.y % mClipMapSize, 
	//	"ClipMap::calculateModuleDeltaBounds - we assume that left and "
	//	"right of the dataset are identical (ie, it's periodical on size of clipmap!) (y)");

	// Now, let's build up our rects. We have one rect if we are moving
	// on the X or Y axis, two if both. We dealt with the no-move case
	// previously.
	if(xDelta == 0)
	{
		// Moving on Y! So generate and store clipped results.
		RectI yRect;

		if(yDelta < 0)
		{
			// We need to generate the box from right of old to right of new.
			yRect.point = newData.point;
			yRect.extent.x = mClipMapSize;
			yRect.extent.y = -yDelta;
		}
		else
		{
			// We need to generate the box from left of old to left of new.
			yRect.point.x = newData.point.x; // Doesn't matter which rect we get this from.
			yRect.point.y = (oldData.point.y + oldData.extent.y);
			yRect.extent.x = mClipMapSize;
			yRect.extent.y = yDelta;
		}

		// Clip out to return buffer, and we're done.
		clipAgainstGrid(mClipMapSize, yRect, outRectCount, outRects);

		return;
	}
	else if(yDelta == 0)
	{
		// Moving on X! So generate and store clipped results.
		RectI xRect;

		if(xDelta < 0)
		{
			// We need to generate the box from right of old to right of new.
			xRect.point = newData.point;
			xRect.extent.x = -xDelta;
			xRect.extent.y = mClipMapSize;
		}
		else
		{
			// We need to generate the box from left of old to left of new.
			xRect.point.x = (oldData.point.x + oldData.extent.x);
			xRect.point.y = newData.point.y; // Doesn't matter which rect we get this from.
			xRect.extent.x = xDelta;
			xRect.extent.y = mClipMapSize;
		}

		// Clip out to return buffer, and we're done.
		clipAgainstGrid(mClipMapSize, xRect, outRectCount, outRects);

		return;
	}
	else
	{
		// Both! We have an L shape. So let's do the bulk of it in one rect,
		// and the remainder in the other. We'll choose X as the dominant axis.
		//
		// a-----b---------c   going from e to a.
		// |     |         |
		// |     |         |
		// d-----e---------f   So the dominant rect is abgh and the passive
		// |     |         |   rect is bcef. Obviously depending on delta we
		// |     |         |   have to switch things around a bit.
		// |     |         |          y+ ^
		// |     |         |             |  
		// g-----h---------i   x+->      |

		RectI xRect, yRect;

		if(xDelta < 0)
		{
			// Case in the diagram.
			xRect.point = newData.point;
			xRect.extent.x = -xDelta;
			xRect.extent.y = mClipMapSize;

			// Set up what of yRect we know, too.
			yRect.point.x = xRect.point.x + xRect.extent.x;
			yRect.extent.x = mClipMapSize - mAbs(xDelta); 
		}
		else
		{
			// Opposite of case in diagram!
			xRect.point.x = oldData.point.x + oldData.extent.x;
			xRect.point.y = newData.point.y;
			xRect.extent.x = xDelta;
			xRect.extent.y = mClipMapSize;

			// Set up what of yRect we know,  too.
			yRect.point.x = (xRect.point.x + xRect.extent.x )- mClipMapSize;
			yRect.extent.x = mClipMapSize - xRect.extent.x;
		}

		if(yDelta < 0)
		{
			// Case in the diagram.
			yRect.point.y = newData.point.y;
			yRect.extent.y = -yDelta;
		}
		else
		{
			// Opposite of case in diagram!
			yRect.point.y = oldData.point.y + oldData.extent.y;
			yRect.extent.y = yDelta;
		}

		// Make sure we don't overlap.
		//AssertFatal(!yRect.overlaps(xRect), 
		//	"ClipMap::calculateModuloDeltaBounds - have overlap in result rects!");

		// Ok, now run them through the clipper.
		S32 firstCount;
		clipAgainstGrid(mClipMapSize, xRect, &firstCount, outRects);
		clipAgainstGrid(mClipMapSize, yRect, outRectCount, outRects + firstCount);
		*outRectCount += firstCount;

		// All done!
		return;
	}

}

void ClipMap::calculateClipMapLevels( const F32 near, const F32 far, const RectF &texBounds, S32 &outStartLevel, S32 &outEndLevel )
{
	F32 ftexsize = F32(mTextureSize);
	RectF tcR(Point2F(texBounds.point.y * ftexsize, texBounds.point.x * ftexsize), ftexsize * texBounds.extent);

	// If we're tiling, make sure we're only insetting away from the clipmap bounds.
	// This avoids making bad LOD selections at clipmap boundaries.
	// Note: compress several compares into one since a*b=0 iff a==0 or b==0
	bool doInset = true;//mTile || (tcR.point.x * tcR.point.y * (tcR.extent.x+tcR.point.x-mTextureSize) * (tcR.extent.y+tcR.point.y-mTextureSize) != 0); 

	if(doInset)
		tcR.inset(-1, -1);

	// Put some safe defaults in for starters.
	outEndLevel   = mClipStackDepth-1;
	outStartLevel = max(outEndLevel-3, S32(0));

	// Now iterate over every clipstack entry and find the smallest that contains
	// the relevant TCs.
	S32 minLevelOverlap = mClipStackDepth - 1;
	S32 maxLevelOverlap = mClipStackDepth - 1;

	for(S32 i=mClipStackDepth-2; i>=0; i--)
	{
		// Find region for entry at this level.
		RectF r;
		F32 biti = F32(BIT(i));
		F32 biticms = F32(BIT(i) * mClipMapSize);
		r.point = Point2F(
			biti * mLevels[i].mToroidalOffset.x, 
			biti * mLevels[i].mToroidalOffset.y);
		r.extent.set(biticms,biticms);

		// Is our tex region fully contained?
		if(r.contains(tcR))
		{
			// If we're fully contained, then this is our new max.
			maxLevelOverlap = i;
			minLevelOverlap = i;
			continue;
		}

		// Or else maybe we've got overlap?
		if (!r.overlaps(tcR))
			break;

		// If we're overlapping then this is our new min...
		minLevelOverlap = getMin(minLevelOverlap, i);
	}

	// Given our level range, do a best fit. We ALWAYS have to have
	// enough for the minimum detail, so we fit that constraint then
	// do our best to give additional detail on top of that.

	// bias the minimum detail to allow smooth transitions to work properly,
	// this avoids a LOT of texture popping.
	maxLevelOverlap++;

	outEndLevel   = noMath::ClampInt(maxLevelOverlap, 0, mClipStackDepth-1);
	outStartLevel = noMath::ClampInt(minLevelOverlap, outEndLevel - 3, outEndLevel - 1);

	// Make sure we're not exceeding our max delta.
	const S32 delta = outEndLevel - outStartLevel;
	//AssertFatal(delta >= 1 && delta <= 4, 
	//	"ClipMap::calculateClipMapLevels - range in levels outside of 2..4 range!");
}

void ClipMap::setCache( IClipMapImageCache *cache )
{
	mImageCache = cache;
	cache->initialize(this);
}

CShaderLoader* ClipMap::getMaterialAndTextures( U32 layer1, U32 layer2, U32 layer3/*=-1*/, U32 layer4/*=-1*/, bool doTerrainRenderHack /*= true*/ )
{
	// Figure out how many layers we have.
	U32 levelCount = 2;
	if(level3 != -1) levelCount++;
	if(level4 != -1) levelCount++;

	GFX->clearSamplerOverrides();   

	mMapInfoConst.setSize(levelCount);
	// Set up constants, bind textures.
	for(S32 i=0; i<levelCount; i++)
	{
		U32 curLevel = -1;
		switch(i)
		{
		case 0: curLevel = level1; break;
		case 1: curLevel = level2; break;
		case 2: curLevel = level3; break;
		case 3: curLevel = level4; break;
		}

		AssertFatal(curLevel != -1, "ClipMap::bindShaderAndTextures - tried to draw an unspecified layer!");
		AssertFatal(curLevel < mLevels.size(), "ClipMap::bindShaderAndTextures - out of range level specified!");

		// Grab references to relevant data.
		Point4F &pt = mMapInfoConst[i];
		ClipMap::ClipStackEntry &cse = mLevels[curLevel];

		// Bind debug or real textures, depending on flag.
		if(smDebugTextures)
			GFX->setTexture(i, cse.mDebugTex);
		else
			GFX->setTexture(i, cse.mTex);

		// Note the offset and center for this level as well.
		pt.x = cse.mClipCenter.x * cse.mScale;
		pt.y = cse.mClipCenter.y * cse.mScale;
		pt.z = cse.mScale;
		pt.w = 0.f;

		if (doTerrainRenderHack && TerrainRender::mCurrentBlock && !TerrainRender::mCurrentBlock->isTiling())
		{
			// At scale 1.0 we can clamp the UV edges avoiding edge bleed between terrain blocks
			if (cse.mScale == 1.0f)
			{            
				GFX->setSamplerAddressModeOverride(i, true, GFXAddressClamp);
			}

			// Ideally, we want to clamp at scale 2.0 as well.
			// however, we can't clamp as the shader is scaling the UV coords
			// edge bleed is caused by mipmaps wrapping, so let's bias the mipmap selection
			// out a little bit, this is only necessary for level 2 of the clip map
			if (cse.mScale == 2.0f)
			{

				GFX->setSamplerMipLODBiasOverride(i, true, -1.0f);
			}
			else
			{
				GFX->setSamplerMipLODBiasOverride(i, true, 0.0f);
			}
		}

	}

	S32 idx = levelCount - 1;
	BaseMatInstance* ret = mClipmapMat[idx];
	MaterialParameters* params = ret->getMaterialParameters();

	// We do not use the morph target in the shader, but
	// we also cannot assume it will be cleared for us.     
	params->set(mMorphTSC[idx], 0.0f);   

	// Set all the constants in one go.   
	params->set(mMapInfoTC[idx], mMapInfoConst);

	params->set(mDiffuseMap0TC[idx], (S32)0);
	params->set(mDiffuseMap1TC[idx], (S32)1);
	params->set(mDiffuseMap2TC[idx], (S32)2);
	params->set(mDiffuseMap3TC[idx], (S32)3);


	GFX->updateStates(true);

	// Ok - all bound!
	return ret;
}

CShaderLoader* ClipMap::bindTexturesFF( U32 curLevel, Point4F &pt, bool firstLayer, bool doTerrainRenderHack /*= true*/ )
{
	// Set up constants, bind textures.
	//AssertFatal(curLevel != -1, "ClipMap::bindTextureFF - tried to draw an unspecified layer!");
	//AssertFatal(curLevel < mLevels.size(), "ClipMap::bindTextureFF - out of range level specified!");

	GFX->clearSamplerOverrides();

	// Grab references to relevant data.
	ClipMap::ClipStackEntry &cse = mLevels[curLevel];

	// Bind debug or real textures, depending on flag.
	if(smDebugTextures)
		GFX->setTexture(0, cse.mDebugTex);
	else
		GFX->setTexture(0, cse.mTex);

	// Note the offset and center for this level as well.
	pt.x = cse.mClipCenter.x * cse.mScale;
	pt.y = cse.mClipCenter.y * cse.mScale;
	pt.z = cse.mScale;
	pt.w = 0.f;

	if (doTerrainRenderHack && TerrainRender::mCurrentBlock && !TerrainRender::mCurrentBlock->isTiling())
	{
		// At scale 1.0 we can clamp the UV edges avoiding edge bleed between terrain blocks
		if (cse.mScale == 1.0f)
		{
			GFX->setSamplerAddressModeOverride(0, true, GFXAddressClamp);
		}

		// Ideally, we want to clamp at scale 2.0 as well.
		// however, we can't clamp as the shader is scaling the UV coords
		// edge bleed is caused by mipmaps wrapping, so let's bias the mipmap selection
		// out a little bit, this is only necessary for level 2 of the clip map
		if (cse.mScale == 2.0f)
		{         
			GFX->setSamplerMipLODBiasOverride(0, true, -1.0f);
		}
		else
		{
			GFX->setSamplerMipLODBiasOverride(0, true, 0.0f);
		}
	}

	GFX->updateStates(true);

	return firstLayer ? mClipmapMatBasePassFF : mClipmapMatAddPassFF;
}

void ClipMap::refreshCache()
{
	//RD: this is bullshit and will lead to problems
	mImageCache->initialize(this);
}


const U32 ClipMap::MAX_LEVELS;

const std::string ClipMap::smDetData;

const std::string ClipMap::smBrightnessScale;

const std::string ClipMap::smMapInfo;

const std::string ClipMap::smMorphT;

bool ClipMap::smDebugTextures;

void ClipMap::ClipStackEntry::initDebugTexture( U32 level )
{
	/*static U8 colors[10][3] = 
	{
		{0xFF, 0x00, 0x00},
		{0xFF, 0xFF, 0x00},
		{0x00, 0xFF, 0x00},
		{0x00, 0xFF, 0xFF},
		{0x00, 0x00, 0xFF},
		{0xFF, 0x00, 0xFF},
		{0xFF, 0xFF, 0xFF},
		{0xaa, 0x00, 0xaa},
		{0xaa, 0xaa, 0x00},
		{0x00, 0xaa, 0xaa},
	};

	GBitmap *debugBmp = new GBitmap(4, 4);

	ColorI debugClr(colors[level][0], colors[level][1], colors[level][2]);
	ColorI edgeClr(colors[level][2], colors[level][1], colors[level][0]);

	for(S32 i=0; i<4; i++)
	{
		for(S32 j=0; j<4; j++)
		{
			bool edge = i==0 || j==0 || i==3 || j==3;
			debugBmp->setColor(i,j, edge ? edgeClr : debugClr);
		}
	}*/

	//mDebugTex.set(debugBmp, &GFXDefaultStaticDiffuseProfile, true, "Clipmap Debug Tex");
}
