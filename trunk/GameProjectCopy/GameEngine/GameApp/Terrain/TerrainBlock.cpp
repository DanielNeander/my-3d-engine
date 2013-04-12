#include "stdafx.h"
#include "TerrainBlock.h"



Signal<void(U32,TerrainBlock*,const Point2I& ,const Point2I&)> TerrainBlock::smUpdateSignal;


TerrainBlock::TerrainBlock()
{

}

TerrainBlock::~TerrainBlock()
{

}

static U16 calcDev(const PlaneF &pl, const Point3F &pt)
{
	F32 z = (pl.d + pl.x * pt.x + pl.y * pt.y) / -pl.z;
	F32 diff = z - pt.z;
	if(diff < 0.0f)
		diff = -diff;

	if(diff > 0xFFFF)
		return 0xFFFF;
	else
		return U16(diff);
}

static U16 Umax(U16 u1, U16 u2)
{
	return u1 > u2 ? u1 : u2;
}

void TerrainBlock::updateOpacity()
{
	PROFILE_SCOPE(TerrainBlock_updateOpacity);

	// Make sure we have enough mOpacityMaps
	addOpacitySources();

	// Copy our various opacity maps in.
	for(S32 opacityLyr = 0; opacityLyr < TerrainBlock::MaterialGroups; opacityLyr++)
	{
		// Skip empty layers.
		if(!mFile->mMaterialAlphaMap[opacityLyr])
			continue;

		GBitmap* opacityMap = mOpacityMaps[U32(opacityLyr) / 4];

		// Not empty - so copy it in.
		U8 *tmpBits = (U8*)opacityMap->getWritableBits() + opacityLyr % 4;
		U8 *opacityBits = mFile->mMaterialAlphaMap[opacityLyr];
		const U8 *opacityEnd = opacityBits + (TerrainBlock::BlockSize*TerrainBlock::BlockSize);

		while(opacityBits != opacityEnd)
		{
			*tmpBits = *opacityBits;
			tmpBits += 4; opacityBits++;
		}
	}

	// Swizzle opacity map into device format.
	for (U32 i = 0; i < mOpacityMaps.size(); i++)
		GFX->getDeviceSwizzle32()->InPlace((U8*)mOpacityMaps[i]->getWritableBits(), 
		4 * TerrainBlock::BlockSize*TerrainBlock::BlockSize);

	// Update the blender image cache with our current source materials.
	if(mClipBlender)
	{
		mClipBlender->clearSourceImages();
		for(S32 i=0; i<TerrainBlock::MaterialGroups; i++)
			mClipBlender->registerSourceImage(mFile->mMaterialFileName[i]);
	}

	// Flush the clipmap (if it's active and has levels)
	if(mClipMap->mLevels.size())
		mClipMap->purgeNextUpdate();
}

void TerrainBlock::addOpacitySources()
{

}



void TerrainBlock::buildChunkDeviance( S32 x, S32 y )
{
	mFile->buildChunkDeviance(x, y);

}

void TerrainBlock::buildGridMap()
{
	mFile->buildGridMap();

}

bool TerrainBlock::onAdd()
{

}

void TerrainBlock::onRemove()
{

}

void TerrainBlock::refreshMaterialLists()
{

}

void TerrainBlock::onEditorEnable()
{

}

void TerrainBlock::onEditorDisable()
{

}

void TerrainBlock::rebuildEmptyFlags()
{
	// rebuild entire maps empty flags!
	for(U32 y = 0; y < TerrainBlock::ChunkSquareWidth; y++)
	{
		for(U32 x = 0; x < TerrainBlock::ChunkSquareWidth; x++)
		{
			GridChunk &gc = *(mChunkMap + x + (y << TerrainBlock::ChunkShift));
			gc.emptyFlags = 0;
			U32 sx = x << TerrainBlock::ChunkDownShift;
			U32 sy = y << TerrainBlock::ChunkDownShift;
			for(U32 j = 0; j < 4; j++)
			{
				for(U32 i = 0; i < 4; i++)
				{
					TerrainBlock::Material *mat = getMaterial(sx + i, sy + j);
					if(mat->flags & TerrainBlock::Material::Empty)
						gc.emptyFlags |= (1 << (j * 4 + i));
				}
			}
		}
	}

	for(S32 i = BlockShift; i >= 0; i--)
	{
		S32 squareCount = 1 << (BlockShift - i);
		S32 squareSize = (TerrainBlock::BlockSize) / squareCount;

		for(S32 squareX = 0; squareX < squareCount; squareX++)
		{
			for(S32 squareY = 0; squareY < squareCount; squareY++)
			{
				GridSquare *parent = NULL;
				if(i < BlockShift)
					parent = findSquare(i+1, Point2I(squareX * squareSize, squareY * squareSize));
				bool empty = true;

				for(S32 sizeX = 0; empty && sizeX <= squareSize; sizeX++)
				{
					for(S32 sizeY = 0; empty && sizeY <= squareSize; sizeY++)
					{
						S32 x = squareX * squareSize + sizeX;
						S32 y = squareY * squareSize + sizeY;

						if(sizeX != squareSize && sizeY != squareSize)
						{
							TerrainBlock::Material *mat = getMaterial(x, y);
							if(!(mat->flags & TerrainBlock::Material::Empty))
								empty = false;
						}
					}
				}
				GridSquare *sq = findSquare(i, Point2I(squareX * squareSize, squareY * squareSize));
				if(empty)
					sq->flags |= GridSquare::Empty;
				else
					sq->flags &= ~GridSquare::Empty;
			}
		}
	}
}

bool TerrainBlock::unpackEmptySquares()
{
	U32 size = BlockSquareWidth * BlockSquareWidth;

	U32 i;
	for(i = 0; i < size; i++)
		materialMap[i].flags &= ~Material::Empty;

	for(i = 0; i < mEmptySquareRuns.size(); i++)
	{

		U32 offset = mEmptySquareRuns[i] & 0xffff;
		U32 run = U32(mEmptySquareRuns[i]) >> 16;

		//
		for(U32 j = 0; j < run; j++)
		{
			if((offset+j) >= size)
			{
				//Con::errorf(ConsoleLogEntry::General, "TerrainBlock::unpackEmpties: invalid entry.");
				return(false);
			}
			materialMap[offset+j].flags |= Material::Empty;
		}
	}

	rebuildEmptyFlags();
	return(true);
}

void TerrainBlock::packEmptySquares()
{
	mEmptySquareRuns.clear();

	// walk the map
	U32 run = 0;
	U32 offset = 0xFFFFFFFF;

	U32 size = BlockSquareWidth * BlockSquareWidth;
	for(U32 i = 0; i < size; i++)
	{
		if(materialMap[i].flags & Material::Empty)
		{
			if(!run)
				offset = i;
			run++;
		}
		else if(run)
		{
			AssertFatal(offset != 0xFFFFFFFF, "TerrainBlock::packEmptySquares");
			mEmptySquareRuns.push_back((run << 16) | offset);
			run = 0;

			// cap it
			if(mEmptySquareRuns.size() == MaxEmptyRunPairs)
				break;
		}
	}

	//
	if(run && mEmptySquareRuns.size() != MaxEmptyRunPairs)
		mEmptySquareRuns.push_back((run << 16) | offset);

	if(mEmptySquareRuns.size() == MaxEmptyRunPairs)
		//Con::warnf(ConsoleLogEntry::General, "TerrainBlock::packEmptySquares: More than %d run pairs encountered.  Extras will not persist.", MaxEmptyRunPairs);
		assert(false);

	//
	mNetFlags |= EmptyMask;
}

void TerrainBlock::inspectPostApply()
{
	Parent::inspectPostApply();

	if (mClipMapSizeLog2 < 9)
		mClipMapSizeLog2 = 9;

	if (mClipMapSizeLog2 > 10)
		mClipMapSizeLog2 = 10;

	if (mTexelsPerMeter < 4)
		mTexelsPerMeter = 4;

	if (mTexelsPerMeter > 128)
		mTexelsPerMeter = 128;

	setMaskBits(MoveMask);
}

Material * TerrainBlock::getMaterial( U32 x, U32 y )
{

}

BaseMatInstance* TerrainBlock::getMaterialInst( U32 x, U32 y )
{

}

GridSquare * TerrainBlock::findSquare( U32 level, const Point2I &pos )
{
	return gridMap[level] + (pos.x >> level) + ((pos.y>>level) << (BlockShift - level));

}

GridSquare * TerrainBlock::findSquare( U32 level, S32 x, S32 y )
{
	return gridMap[level] + ((x  & TerrainBlock::BlockMask) >> level) + (((y & TerrainBlock::BlockMask) >> level) << (TerrainBlock::BlockShift - level));

}

GridChunk * TerrainBlock::findChunk( const Point2I &pos )
{
	return mChunkMap + (pos.x >> ChunkDownShift) + ((pos.y>>ChunkDownShift) << ChunkShift);

}

void TerrainBlock::calcBounds()
{
	// We know our bounds x, y are going to go from 0, 0 to BlockSize * SquareSize
	// but our height will be variable according to the heightfield
	mBounds.minExtents.set(0.0f, 0.0f, -1e10f);
	mBounds.maxExtents.set(BlockSize * (float)mSquareSize, BlockSize * (float)mSquareSize, 1e10f);

	// Little trick I picked up from the EditTSCtrl::renderMissionArea()
	// I *believe* that this works because the lowest level of detail has
	// the absolute max and min heights
	GridSquare * gs = findSquare(TerrainBlock::BlockShift, Point2I(0,0));
	mBounds.maxExtents.z = fixedToFloat(gs->maxHeight);
	mBounds.minExtents.z = fixedToFloat(gs->minHeight);

	// If we aren't tiling go ahead and set our mObjBox to be equal to mBounds
	// This will get overridden with global bounds if tiling is on but it is useful to store
	if (!mTile)
	{
		if (mObjBox.maxExtents != mBounds.maxExtents || mObjBox.minExtents != mBounds.minExtents)
		{
			mObjBox = mBounds;
			resetWorldBox();
			setMaskBits(TerrainChangedMask);
		}
	}
}

void TerrainBlock::setHeight( const Point2I & pos, F32 height )
{
	// set the height
	U16 ht = floatToFixed(height);
	*((U16*)getHeightAddress(pos.x, pos.y)) = ht;
}



inline void getMinMax(U16 &inMin, U16 &inMax, U16 height)
{
	if(height < inMin)
		inMin = height;
	if(height > inMax)
		inMax = height;
}

inline void checkSquareMinMax(GridSquare *parent, const GridSquare *child)
{
	if(parent->minHeight > child->minHeight)
		parent->minHeight = child->minHeight;
	if(parent->maxHeight < child->maxHeight)
		parent->maxHeight = child->maxHeight;
}

void TerrainBlock::updateGridMaterials( const Point2I &minPt, const Point2I &maxPt )
{
	// ok:
	// build the chunk materials flags for all the chunks in the bounding rect
	// ((min - 1) >> ChunkDownShift) up to ((max + ChunkWidth) >> ChunkDownShift)

	// we have to make sure to cover boundary conditions as as stated above
	// since, for example, x = 0 affects 2 chunks

	for (S32 y = minPt.y - 1; y < maxPt.y + 1; y++)
	{
		for (S32 x=minPt.x - 1; x < maxPt.x + 1; x++)
		{
			GridSquare *sq = findSquare(0, Point2I(x, y));
			sq->flags &= (GridSquare::MaterialStart -1);

			S32 xpl = (x + 1) & TerrainBlock::BlockMask;
			S32 ypl = (y + 1) & TerrainBlock::BlockMask;

			for(U32 i = 0; i < TerrainBlock::MaterialGroups; i++)
			{
				if (mFile->mMaterialAlphaMap[i] != NULL) 
				{
					U32 mapVal = (mFile->mMaterialAlphaMap[i][(y   << TerrainBlock::BlockShift) + x]     |
						mFile->mMaterialAlphaMap[i][(ypl << TerrainBlock::BlockShift) + x]   |
						mFile->mMaterialAlphaMap[i][(ypl << TerrainBlock::BlockShift) + xpl] |
						mFile->mMaterialAlphaMap[i][(y   << TerrainBlock::BlockShift) + xpl]);
					if(mapVal)
						sq->flags |= (GridSquare::MaterialStart << i);
				}
			}
		}
	}

	for (S32 y = minPt.y - 2; y < maxPt.y + 2; y += 2)
	{
		for (S32 x= minPt.x - 2; x < maxPt.x + 2; x += 2)
		{
			GridSquare *sq = findSquare(1, Point2I(x, y));
			GridSquare *s1 = findSquare(0, Point2I(x, y));
			GridSquare *s2 = findSquare(0, Point2I(x+1, y));
			GridSquare *s3 = findSquare(0, Point2I(x, y+1));
			GridSquare *s4 = findSquare(0, Point2I(x+1, y+1));
			sq->flags |= (s1->flags | s2->flags | s3->flags | s4->flags) & ~(GridSquare::MaterialStart -1);
		}
	}

	//AllocatedTexture::flushCacheRect(RectI(min, max - min));
	updateOpacity();

	// Signal anyone that cares that the opacity was changed.
	smUpdateSignal.trigger( OpacityUpdate, this, minPt, maxPt );

}

U16 TerrainBlock::getHeight( U32 x, U32 y ) const
{
	return mFile->getHeight( x, y );
}

bool TerrainBlock::getHeight( const Point2F & pos, F32 * height )
{
	F32 invSquareSize = 1.0f / (F32)mSquareSize;
	F32 xp = pos.x * invSquareSize;
	F32 yp = pos.y * invSquareSize;
	int x = S32(xp);
	int y = S32(yp);
	xp -= (F32)x;
	yp -= (F32)y;

	if(!mTile && (x &~ BlockMask || y &~ BlockMask))
		return false;

	x &= BlockMask;
	y &= BlockMask;

	GridSquare * gs = findSquare(0, Point2I(x,y));

	if (gs->flags & GridSquare::Empty)
		return false;

	F32 zBottomLeft = fixedToFloat(getHeight(x, y));
	F32 zBottomRight = fixedToFloat(getHeight(x + 1, y));
	F32 zTopLeft = fixedToFloat(getHeight(x, y + 1));
	F32 zTopRight = fixedToFloat(getHeight(x + 1, y + 1));

	if(gs->flags & GridSquare::Split45)
	{
		if (xp>yp)
			// bottom half
			*height = zBottomLeft + xp * (zBottomRight-zBottomLeft) + yp * (zTopRight-zBottomRight);
		else
			// top half
			*height = zBottomLeft + xp * (zTopRight-zTopLeft) + yp * (zTopLeft-zBottomLeft);
	}
	else
	{
		if (1.0f-xp>yp)
			// bottom half
			*height = zBottomRight + (1.0f-xp) * (zBottomLeft-zBottomRight) + yp * (zTopLeft-zBottomLeft);
		else
			// top half
			*height = zBottomRight + (1.0f-xp) * (zTopLeft-zTopRight) + yp * (zTopRight-zBottomRight);
	}
	return true;
}

Point2I TerrainBlock::getGridPos( const Point3F& worldPos )
{
	noVec3 terrainPos = worldPos;
	noVec3 worldPos = worldTM_ * terrainPos;

	F32 squareSize = ( F32 ) getSquareSize();
	F32 halfSquareSize = squareSize / 2.0;

	F32 x = ( terrainPos.x + halfSquareSize ) / squareSize;
	F32 y = ( terrainPos.y + halfSquareSize ) / squareSize;

	Point2I gridPos( ( S32 ) noMath::Floor( x ), ( S32 ) noMath::Floor( y ) );
	return gridPos;
}

void TerrainBlock::updateGrid( const Point2I &minPt, const Point2I &maxPt )
{
	// On the client we just signal everyone that the height
	// map has changed... the server does the actual changes.
	if ( isClientObject() )
	{
		smUpdateSignal.trigger( HeightmapUpdate, this, minPt, maxPt );
		return;
	}

	// ok:
	// build the chunk deviance for all the chunks in the bounding rect,
	// ((min - 1) >> ChunkDownShift) up to ((max + ChunkWidth) >> ChunkDownShift)

	// update the chunk deviances for the affected chunks
	// we have to make sure to cover boundary conditions as as stated above
	// since, for example, x = 0 affects 2 chunks

	for(S32 x = (minPt.x - 1) >> ChunkDownShift;x < (maxPt.x + ChunkSize) >> ChunkDownShift; x++)
	{
		for(S32 y = (minPt.y - 1) >> ChunkDownShift;y < (maxPt.y + ChunkSize) >> ChunkDownShift; y++)
		{
			S32 px = x;
			S32 py = y;
			if(px < 0)
				px += BlockSize >> ChunkDownShift;
			if(py < 0)
				py += BlockSize >> ChunkDownShift;

			buildChunkDeviance(px, py);
		}
	}

	// ok the chunk deviances are rebuilt... now rebuild the affected area
	// of the grid map:

	// here's how it works:
	// for the current terrain renderer we only care about
	// the minHeight and maxHeight on the GridSquare
	// so we do one pass through, updating minHeight and maxHeight
	// on the level 0 squares, then we loop up the grid map from 1 to
	// the top, expanding the bounding boxes as necessary.
	// this should end up being way, way, way, way faster for the terrain
	// editor

	for(S32 y = minPt.y - 1; y < maxPt.y + 1; y++)
	{
		for(S32 x = minPt.x - 1; x < maxPt.x + 1; x++)
		{
			S32 px = x;
			S32 py = y;
			if(px < 0)
				px += BlockSize;
			if(py < 0)
				py += BlockSize;

			GridSquare *sq = findSquare(0, px, py);

			sq->minHeight = 0xFFFF;
			sq->maxHeight = 0;

			getMinMax(sq->minHeight, sq->maxHeight, getHeight(x, y));
			getMinMax(sq->minHeight, sq->maxHeight, getHeight(x+1, y));
			getMinMax(sq->minHeight, sq->maxHeight, getHeight(x, y+1));
			getMinMax(sq->minHeight, sq->maxHeight, getHeight(x+1, y+1));
		}
	}

	// ok, all the level 0 grid squares are updated:
	// now update all the parent grid squares that need to be updated:

	for(S32 level = 1; level <= TerrainBlock::BlockShift; level++)
	{
		S32 size = 1 << level;
		S32 halfSize = size >> 1;
		for(S32 y = (minPt.y - 1) >> level; y < (maxPt.y + size) >> level; y++)
		{
			for(S32 x = (minPt.x - 1) >> level; x < (maxPt.x + size) >> level; x++)
			{
				S32 px = x << level;
				S32 py = y << level;

				GridSquare *square = findSquare(level, px, py);
				square->minHeight = 0xFFFF;
				square->maxHeight = 0;

				checkSquareMinMax(square, findSquare(level - 1, px, py));
				checkSquareMinMax(square, findSquare(level - 1, px + halfSize, py));
				checkSquareMinMax(square, findSquare(level - 1, px, py + halfSize));
				checkSquareMinMax(square, findSquare(level - 1, px + halfSize, py + halfSize));
			}
		}
	}

	calcBounds();

	// Signal again here for any server side observers.
	smUpdateSignal.trigger( HeightmapUpdate, this, minPt, maxPt );
}

void TerrainBlock::setBaseMaterial( U32 x, U32 y, U8 matGroup )
{
	
}

U8 * TerrainBlock::getMaterialAlphaMap( U32 matIndex )
{
	if (mFile->mMaterialAlphaMap[matIndex] == NULL) {
		mFile->mMaterialAlphaMap[matIndex] = new U8[TerrainBlock::BlockSize * TerrainBlock::BlockSize];
		memset(mFile->mMaterialAlphaMap[matIndex], 0, TerrainBlock::BlockSize * TerrainBlock::BlockSize);
	}

	return mFile->mMaterialAlphaMap[matIndex];
}

U8* TerrainBlock::getBaseMaterialAddress( U32 x, U32 y )
{
	return &mBaseMaterialMap[(x & BlockMask) + ((y & BlockMask) << BlockShift)];

}

U8 TerrainBlock::getBaseMaterial( U32 x, U32 y )
{
	return mBaseMaterialMap[(x & BlockMask) + ((y & BlockMask) << BlockShift)];

}

U16 * TerrainBlock::getFlagMapPtr( S32 x, S32 y )
{
	return flagMap + ((x >> 1) & TerrainBlock::FlagMapMask) +
		((y >> 1) & TerrainBlock::FlagMapMask) * TerrainBlock::FlagMapWidth;
}

void TerrainBlock::getMaterialAlpha( const Point2I &pos, U8 alphas[MaterialGroups] ) const
{
	Point2I  thePos( pos );
	thePos.x &= BlockMask;
	thePos.y &= BlockMask;

	U32 offset = thePos.x + thePos.y * BlockSize;
	for(S32 i = 0; i < MaterialGroups; i++)
	{
		U8 *map = mFile->mMaterialAlphaMap[i];
		if(map)
			alphas[i] = map[offset];
		else
			alphas[i] = 0;
	}
}

void TerrainBlock::setMaterialAlpha( const Point2I &pos, const U8 alphas[MaterialGroups] )
{
	Point2I  thePos( pos );
	thePos.x &= BlockMask;
	thePos.y &= BlockMask;

	U32 offset = thePos.x + thePos.y * BlockSize;
	for(S32 i = 0; i < MaterialGroups; i++)
	{
		U8 *map = mFile->mMaterialAlphaMap[i];
		if(map)
			map[offset] = alphas[i];
	}
}

void TerrainBlock::getMinMaxHeight( F32* minHeight, F32* maxHeight ) const
{
	*minHeight =  fixedToFloat( gridMap[BlockShift]->minHeight );
	*maxHeight =  fixedToFloat( gridMap[BlockShift]->maxHeight );
}

bool TerrainBlock::getNormal( const Point2F & pos, Point3F * normal, bool normalize /*= true*/ )
{
	F32 invSquareSize = 1.0f / (F32)mSquareSize;
	F32 xp = pos.x * invSquareSize;
	F32 yp = pos.y * invSquareSize;
	S32 x = S32(xp);
	S32 y = S32(yp);
	xp -= (F32)x;
	yp -= (F32)y;

	if(!mTile && (x &~ BlockMask || y &~ BlockMask))
		return false;

	x &= BlockMask;
	y &= BlockMask;

	GridSquare * gs = findSquare(0, Point2I(x,y));

	if (gs->flags & GridSquare::Empty)
		return false;

	F32 zBottomLeft = fixedToFloat(getHeight(x, y));
	F32 zBottomRight = fixedToFloat(getHeight(x + 1, y));
	F32 zTopLeft = fixedToFloat(getHeight(x, y + 1));
	F32 zTopRight = fixedToFloat(getHeight(x + 1, y + 1));

	if(gs->flags & GridSquare::Split45)
	{
		if (xp>yp)
			// bottom half
			normal->set(zBottomLeft-zBottomRight, zBottomRight-zTopRight, (F32)mSquareSize);
		else
			// top half
			normal->set(zTopLeft-zTopRight, zBottomLeft-zTopLeft, (F32)mSquareSize);
	}
	else
	{
		if (1.0f-xp>yp)
			// bottom half
			normal->set(zBottomLeft-zBottomRight, zBottomLeft-zTopLeft, (F32)mSquareSize);
		else
			// top half
			normal->set(zTopLeft-zTopRight, zBottomRight-zTopRight, (F32)mSquareSize);
	}
	if (normalize)
		normal->normalize();
	return true;
}

bool TerrainBlock::getNormalAndHeight( const Point2F & pos, Point3F * normal, F32 * height, bool normalize /*= true*/ )
{
	F32 invSquareSize = 1.0f / (F32)mSquareSize;
	F32 xp = pos.x * invSquareSize;
	F32 yp = pos.y * invSquareSize;
	int x = S32(xp);
	int y = S32(yp);
	xp -= (F32)x;
	yp -= (F32)y;

	if(!mTile && (x &~ BlockMask || y &~ BlockMask))
		return false;

	x &= BlockMask;
	y &= BlockMask;

	GridSquare * gs = findSquare(0, Point2I(x,y));

	if (gs->flags & GridSquare::Empty)
		return false;

	F32 zBottomLeft  = fixedToFloat(getHeight(x, y));
	F32 zBottomRight = fixedToFloat(getHeight(x + 1, y));
	F32 zTopLeft     = fixedToFloat(getHeight(x, y + 1));
	F32 zTopRight    = fixedToFloat(getHeight(x + 1, y + 1));

	if(gs->flags & GridSquare::Split45)
	{
		if (xp>yp)
		{
			// bottom half
			normal->Set(zBottomLeft-zBottomRight, zBottomRight-zTopRight, (F32)mSquareSize);
			*height = zBottomLeft + xp * (zBottomRight-zBottomLeft) + yp * (zTopRight-zBottomRight);
		}
		else
		{
			// top half
			normal->Set(zTopLeft-zTopRight, zBottomLeft-zTopLeft, (F32)mSquareSize);
			*height = zBottomLeft + xp * (zTopRight-zTopLeft) + yp * (zTopLeft-zBottomLeft);
		}
	}
	else
	{
		if (1.0f-xp>yp)
		{
			// bottom half
			normal->Set(zBottomLeft-zBottomRight, zBottomLeft-zTopLeft, (F32)mSquareSize);
			*height = zBottomRight + (1.0f-xp) * (zBottomLeft-zBottomRight) + yp * (zTopLeft-zBottomLeft);
		}
		else
		{
			// top half
			normal->Set(zTopLeft-zTopRight, zBottomRight-zTopRight, (F32)mSquareSize);
			*height = zBottomRight + (1.0f-xp) * (zTopLeft-zTopRight) + yp * (zTopRight-zBottomRight);
		}
	}
	if (normalize)
		normal->normalize();
	return true;
}

S32 TerrainBlock::getMaterialAlphaIndex( const String &materialName )
{
	for (S32 i = 0; i < MaterialGroups; i++)
	{
		if (!mMaterialFileName[i].isEmpty() && materialName.equal( mMaterialFileName[i], String::NoCase ))
			return i;
	}

	// ok, it wasn't in the list
	// see if we can add it:
	for (S32 i = 0; i < MaterialGroups; i++)
	{
		if (mMaterialFileName[i].isEmpty())
		{
			mMaterialFileName[i] = materialName;
			mFile->mMaterialAlphaMap[i] = new U8[TerrainBlock::BlockSize * TerrainBlock::BlockSize];
			dMemset(mFile->mMaterialAlphaMap[i], 0, TerrainBlock::BlockSize * TerrainBlock::BlockSize);
			buildMaterialMap();
			return i;
		}
	}
	return -1;
}

void TerrainBlock::setFile( Resource<TerrainFile> file )
{

}

void TerrainBlock::createNewTerrainFile( StringTableEntry filename, U16 *hmap /*= NULL*/, Vector<StringTableEntry> *materials /*= NULL*/ )
{

}

bool TerrainBlock::save( const char* filename )
{

}

void TerrainBlock::flushCache()
{

}

S32 TerrainBlock::getSquareSize() const
{
	return mSquareSize;

}

bool TerrainBlock::isTiling() const
{
	return mTile;
}

void TerrainBlock::refillClipmap()
{
	mClipMap->fillWithTextureData();
}

bool TerrainBlock::buildMaterialMap()
{
	//PROFILE_SCOPE(TerrainBlock_BuildMaterialMap);
	// We have to make sure we have the right number of alpha maps - should
	// have one for every specified texture. So add or delete to conform.
	for(S32 i=0; i<TerrainBlock::MaterialGroups; i++)
	{
		if(!mFile->mMaterialFileName[i].empty())
		{
			if(!mFile->mMaterialAlphaMap[i])
			{
				mFile->mMaterialAlphaMap[i] = new U8[TerrainBlock::BlockSize*TerrainBlock::BlockSize];
				memset(mFile->mMaterialAlphaMap[i], 0, sizeof(U8)*TerrainBlock::BlockSize*TerrainBlock::BlockSize);

				addOpacitySources();

				if (mClipBlender)
					mClipBlender->createOpacityScratchTextures();
			}
		}
		else
		{
			if(mFile->mMaterialAlphaMap[i])
			{
				delete [] mFile->mMaterialAlphaMap[i];
				mFile->mMaterialAlphaMap[i] = NULL;
			}
		}
	}

	// Update the blender image cache with our current source materials.
	if(mClipBlender)
	{
		mClipBlender->clearSourceImages();
		for(S32 i=0; i<TerrainBlock::MaterialGroups; i++)
			mClipBlender->registerSourceImage(mFile->mMaterialFileName[i]);
	}

	if(mClipMap && lightMap)
	{
		// Update the lightmap source.  We have to keep two copies of the lightmap around for now. ;/
		// One is the RGB version which is used by getAmbientLighting and is loaded/store in the ml file.
		// One is the swizzled BGR version which the clip mapper wants.
		SAFE_DELETE(mSwizzledLightMap);
		mSwizzledLightMap = new GBitmap(*lightMap);
		mSwizzledLightMap->swizzle(GFX->getDeviceSwizzle24());
		mLightmapSource->mBitmap = mSwizzledLightMap;

		// Refill clipmap.
		mClipMap->fillWithTextureData();
	}

	// Reload the detail texture
	if( !mDetailTextureName.isEmpty() )
		mDetailTextureHandle.set(mDetailTextureName, &GFXDefaultStaticDiffuseProfile, avar("%s() - tex (line %d)", __FUNCTION__, __LINE__));

	// Signal anyone that cares on the lightmap update.
	smUpdateSignal.trigger( LightmapUpdate, this, Point2I(), Point2I() );
}

void TerrainBlock::buildMipMap()
{

}

void TerrainBlock::setBaseMaterials( S32 argc, const char *argv[] )
{
	for (S32 i = 0; i < argc; i++)
		mMaterialFileName[i] = argv[i];
	//for (S32 j = argc; j < MaterialGroups; j++)
	//	mMaterialFileName[j];
}

bool TerrainBlock::loadBaseMaterials()
{

}

bool TerrainBlock::initMMXBlender()
{
	U32 validMaterials = 0;
	S32 i;
	for (i = 0; i < MaterialGroups; i++) 
	{
		if (!mMaterialFileName[i].isEmpty())
			validMaterials++;
		else
			break;
	}
	AssertFatal(validMaterials != 0, "Error, must have SOME materials here!");

	updateOpacity();
}

void TerrainBlock::setOverrideMaterial( BaseMatInstance* overrideMaterial )
{

}

void TerrainBlock::setTerrainFile( const FileName &newValue )
{

}

void TerrainBlock::setMaterial( S32 idx, const String &newValue )
{

}

void TerrainBlock::initPersistFields()
{

}

U32 TerrainBlock::packUpdate( NetConnection *conn, U32 mask, BitStream *stream )
{

}

void TerrainBlock::unpackUpdate( NetConnection *conn, BitStream *stream )
{

}
