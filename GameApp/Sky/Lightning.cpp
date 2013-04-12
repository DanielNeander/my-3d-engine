/*
--------------------------------------------------------------------------------
This source file is part of SkyX.
Visit http://www.paradise-studios.net/products/skyx/

Copyright (C) 2009-2012 Xavier Vergu? Gonz?ez <xavyiy@gmail.com>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.
--------------------------------------------------------------------------------
*/

#include "stdafx.h"
#include "Lightning.h"
#include "ModelLib/util.h"

namespace VCloud { 

	Lightning::Lightning(SceneNode* sn, const noVec3& orig, const noVec3& dir, 
		const float& l, const uint32& d, const uint32& rec, const float& tm, const float& wm, const noVec2& b)
		: mOrigin(orig)
		, mDirection(dir)
		, mLength(l)
		, mRealLength(0)
		, mDivisions(d)
		, mRecursivity(rec)
		, mTime(0)
		, mTimeMultiplier(tm)
		, mIntensity(0)
		, mWidthMultiplier(wm)
		, mBounds(b)
		, mAngleRange(noVec2(randfloat(0.3,0.5), randfloat(0.6,0.8)))
		, mTimeMultipliers(noVec3(randfloat(1.75,4.25), randfloat(0.4,1.25f), randfloat(0.2,1.0f)))
		, mSegments(std::vector<Lightning::Segment>())
		, mChildren(std::vector<Lightning*>())		
		, mSceneNode(sn)
		, mCreated(false)
		, mFinished(false)
	{
	}

	Lightning::~Lightning()
	{
		remove();
	}

	void Lightning::create()
	{
		remove();

		noVec3 end = mOrigin + mDirection*mLength;
		noVec3 current, last = mOrigin;

		// Create ray segments
		for(uint32 k = 1; k < mDivisions+1; k++)
		{
			noVec3 current = mOrigin + mDirection*mLength*(static_cast<float>(k)/mDivisions);

			current += (mLength/(mDivisions*3))*noVec3(
				randfloat(-1, 1), randfloat(-1, 1), randfloat(-1, 1));

			mSegments.push_back(Segment(last, current));

			mRealLength += (current-last).Length();

			last = current;
		}

		// Create the associated billboard set
		/*mBillboardSet = mSceneManager->createBillboardSet();
		mBillboardSet->setMaterialName("SkyX_Lightning"); */
		mBillboardSet = new BillboardSet(20);
		mBillboardSet->setBillboardType(BBT_ORIENTED_SELF);

		float width = mWidthMultiplier*3*(static_cast<float>(mRecursivity)/4+1)*randfloat(0.5f, 2.5f-mRecursivity/3);

		// Create the associated billboard for each segment
		float delta;
		noVec2 bounds;
		Billboard* bb;
		for(uint32 k = 0; k < mSegments.size(); k++)
		{
			delta = 1.0f / mSegments.size();
			bounds = noVec2(k*delta,(k+1)*delta);

			bounds = noVec2(mBounds.x, mBounds.x) + bounds*(mBounds.y-mBounds.x);

			bb = mBillboardSet->createBillboard((mSegments.at(k).a+mSegments.at(k).b)/2);
			bb->setDimensions(width, (mSegments.at(k).a-mSegments.at(k).b).Length());
			bb->setColour(ColourValue(0,bounds.x,bounds.y));
			bb->mDirection = (mSegments.at(k).a-mSegments.at(k).b).NormalizeCopy();

			bb = mBillboardSet->createBillboard(mSegments.at(k).a + (mSegments.at(k).a-mSegments.at(k).b).NormalizeCopy()*width/2);
			bb->setDimensions(width, width);
			bb->setColour(ColourValue(1,bounds.x,bounds.x));
			bb->mDirection = (mSegments.at(k).a-mSegments.at(k).b).NormalizeCopy();
			
			bb = mBillboardSet->createBillboard(mSegments.at(k).b - (mSegments.at(k).a-mSegments.at(k).b).NormalizeCopy()*width/2);
			bb->setDimensions(width, width);
			bb->setColour(ColourValue(1,bounds.y,bounds.y));
			bb->mDirection = -(mSegments.at(k).a-mSegments.at(k).b).NormalizeCopy();
		
			width *= 1-(1.0f/(mRecursivity*mRecursivity+1.0f))*(1.0f/mSegments.size());
		}

		mBillboardSet->_updateBounds();

		mSceneNode->AddChild(mBillboardSet);

		//mBillboardSet->setCustomParameter(0, noVec4(1,0,0,0));

		// Ramifications
		if (mRecursivity > 0)
		{
			float angle;
			noVec3 dir;
			float lengthMult;
			for (uint32 k = 0; k < mDivisions-1; k++)
			{
				angle = (mSegments.at(k).b-mSegments.at(k).a).NormalizeCopy() * (
					((mSegments.at(k+1).b-mSegments.at(k+1).a).NormalizeCopy()));

				if (angle < randfloat(mAngleRange.x, mAngleRange.y))
				{
					dir = (mSegments.at(k).b-mSegments.at(k).a).NormalizeCopy();
					dir.x *= randfloat(0.8f, 1.2f);
					dir.y *= randfloat(0.8f, 1.2f);
					dir.z *= randfloat(0.8f, 1.2f);
					dir.Normalize();

					delta = 1.0f / mSegments.size();
					bounds = noVec2(mBounds.x+(mBounds.y-mBounds.x)*(k+1)*delta,1);

					lengthMult = randfloat(0.1f, 0.7f);

					Lightning* lightning = new Lightning(mSceneNode, mSegments.at(k).b, dir, lengthMult*mLength, 2+mDivisions*lengthMult, mRecursivity-1, mTimeMultiplier, mWidthMultiplier, bounds);
					lightning->create();
					
					mChildren.push_back(lightning);
				}
			}
		}

		mCreated = true;
	}

	void Lightning::remove()
	{
		if (!mCreated)
		{
			return;
		}

		//mSceneNode->detachObject(mBillboardSet);
		//mSceneManager->destroyBillboardSet(mBillboardSet);

		mSegments.clear();

		for(uint32 k = 0; k < mChildren.size(); k++)
		{
			delete mChildren.at(k);
		}

		mChildren.clear();

		mFinished = false;

		mCreated = false;
	}

	void Lightning::update(float timeSinceLastFrame)
	{
		if (!mCreated)
		{
			return;
		}

		timeSinceLastFrame *= mTimeMultiplier;

		// mTime timeline: (Note: Time multipliers are random)
		// 0.0 -> 1.2 : Ray creation(fordward effect) + Big flash
		// 1.2 -> 2.0 : Sinus flashing pattern
		// 2.0 -> 3.0 Ray fading

		float alpha = 0.5f;
		float maxAlpha = 1.5f;

		if (mTime < 1)
		{
			mTime += timeSinceLastFrame*mTimeMultipliers.x;

			if (mTime > 2) mTime = 1.5f; // Prevent big changes

			if (mTime > 0.8f) // Big flash start
			{
				alpha += (mTime-0.8f)*(maxAlpha/0.2f);
			}
		}
		else if (mTime > 1 && mTime < 2)
		{
			mTime += timeSinceLastFrame*mTimeMultipliers.y;

			if (mTime > 3) mTime = 2.5f; // Prevent big changes

			if (mTime < 1.2f) // Big flash end
			{
				alpha += (0.2f-(mTime-1.0f))*(maxAlpha/0.2f);
			}
			else // Sinus flashing pattern
			{
				alpha += noMath::Abs(noMath::Sin((mTime-1.2f)*1.5f*mTimeMultipliers.x));
			}
		}
		else if (mTime > 2) // Ray fading
		{
			mTime += timeSinceLastFrame*mTimeMultipliers.z;

			 if (mTime > 3) 
			 {
				 mTime = 3; // Prevent big changes
				 mFinished = true;
			 }

			alpha += noMath::Abs(noMath::Sin((2-1.2f)*1.5f*mTimeMultipliers.x));
			alpha *= 3.0f-mTime;
		}

		mIntensity = alpha;

		_updateData(alpha, mTime > 1 ? 1 : mTime, mTime);
	}

	/*void Lightning::_updateRenderQueueGroup(const uint8& rqg)
	{
		mBillboardSet->setRenderQueueGroup(rqg);

		for(uint32 k = 0; k < mChildren.size(); k++)
		{	
			mChildren.at(k)->_updateRenderQueueGroup(rqg);
		}
	}*/

	void Lightning::_updateData(const float& alpha, const float& currentPos, const float& parentTime)
	{
		noVec4 params = noVec4(alpha,currentPos,(3-mRecursivity)*0.075f+(mBounds.x*1.5f+0.2f)*0.85f,0);

		if (parentTime > 1 && parentTime < 2.2f)
		{
			params.z *= noMath::ClampFloat(0.2f,1, 1.5-parentTime);
		}
		else if (parentTime > 2.2f)
		{
			 params.z *= noMath::ClampFloat(0.2f,1, (-2.2f+parentTime)*1.25f);
		}

		//mBillboardSet->setCustomParameter(0, params);

		for(uint32 k = 0; k < mChildren.size(); k++)
		{
			mChildren.at(k)->_updateData(alpha*0.75f, currentPos, parentTime);
		}
	}
}