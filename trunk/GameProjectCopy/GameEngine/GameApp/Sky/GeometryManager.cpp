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

#include "GeometryManager.h"

#include "VClouds.h"

namespace VCloud { 

	GeometryManager::GeometryManager(VClouds* vc)
		: mVClouds(vc)
		, mCreated(false)
		, mHeight(noVec2())
		, mRadius(0)
		, mAlpha(0)
		, mBeta(0)
		, mPhi(0)
		, mNumberOfBlocks(0)
		, mNa(0), mNb(0), mNc(0)
		, mA(0), mB(0), mC(0)
		, mWorldOffset(noVec2(0,0))
		, mCurrentDistance(noVec3(0,0,0)),
		mCurrShader(-1)
	{
	}

	GeometryManager::~GeometryManager()
	{
		remove();
	}

	void GeometryManager::create(const noVec2& Height, const float& Radius,
		    const float& Alpha, const float& Beta, 
            const int& NumberOfBlocks, const int& Na, const int& Nb, const int& Nc)
	{
		remove();

		mHeight = Height;
		mRadius = Radius;
		mAlpha = Alpha;
		mBeta = Beta;
		mPhi = noMath::TWO_PI / NumberOfBlocks;
		mNumberOfBlocks = NumberOfBlocks;
		mNa = Na; mNb = Nb; mNc = Nc;

		mSceneNode = SceneNode::NewNode();
		_createGeometry();

		mCreated = true;
	}

	void GeometryManager::remove()
	{
		if (!mCreated)
		{
			return;
		}

		//mSceneNode->detachAllObjects();
		//mSceneNode->getParentSceneNode()->removeAndDestroyChild(mSceneNode->getName());
		mSceneNode = 0;

		for (int k = 0; k < mNumberOfBlocks; k++)
		{
			delete mGeometryBlocks.at(k);
			mGeometryBlocks.at(k) = 0;
		}

		mGeometryBlocks.clear();

		mCreated = false;
	}

	void GeometryManager::update(const float& timeSinceLastFrame)
	{
		if (!mCreated)
		{
			return;
		}

		mWorldOffset += mVClouds->getWindDirectionV2() * mVClouds->getWindSpeed() * timeSinceLastFrame;
	}

	void GeometryManager::updateGeometry(BaseCamera* c, const float& timeSinceLastCameraFrame)
	{
		if (!mCreated)
		{
			return;
		}

		noVec3 trans(mVClouds->getCamera()->GetFrom().x, 
			mHeight.x, 
			mVClouds->getCamera()->GetFrom().z);
		mSceneNode->SetTrans(trans);
		mSceneNode->Update(0.f);

		_updateGeometry(c, timeSinceLastCameraFrame);
	}
	
	void GeometryManager::_setMaterial(ShaderID shader)
	{
		for(uint32 k = 0; k < mGeometryBlocks.size(); k++)
		{
			mGeometryBlocks.at(k)->mCurrShader = shader;
		}
	}

	void GeometryManager::_createGeometry()
	{
		mA = mHeight.y / noMath::Cos(noMath::PI/2-mBeta);
		mB = mHeight.y / noMath::Cos(noMath::PI/2-mAlpha);
	    mC = mRadius;

		for (int k = 0; k < mNumberOfBlocks; k++)
		{
			mGeometryBlocks.push_back(new GeometryBlock(mVClouds, mHeight.y, mAlpha, mBeta, mRadius, mPhi, mNa, mNb, mNc, mA, mB, mC, k));
			mGeometryBlocks.at(k)->create();
			// Each geometry block must be in a different scene node, See: GeometryBlock::isInFrustum(Camera *c)			
			SceneNode *sn = SceneNode::NewNode();
			mSceneNode->AddChild(sn);
			sn->AddChild(mGeometryBlocks.at(k));
		}
	}

	void GeometryManager::_updateGeometry(BaseCamera* c, const float& timeSinceLastFrame)
	{
#if 1
		// Look for current camera data
		std::vector<VClouds::CameraData>& camerasData = mVClouds->_getCamerasData();
		std::vector<VClouds::CameraData>::iterator currentCameraDataIt;

		for (currentCameraDataIt = camerasData.begin(); currentCameraDataIt != camerasData.end(); currentCameraDataIt++)
		{
			if ((*currentCameraDataIt).camera == c)
			{
				break;
			}
		}

		std::vector<VClouds::CameraData>::reference currentCameraData = (*currentCameraDataIt);

		// Calculate wind offset
		noVec2 CameraDirection = noVec2(c->GetDir().x, c->GetDir().z);
		float offset = - CameraDirection * (mVClouds->getWindDirectionV2()) * mVClouds->getWindSpeed() * timeSinceLastFrame;

		// Calculate camera offset
		noVec2 CameraOffset = noVec2(c->GetFrom().x - currentCameraData.lastPosition.x, c->GetFrom().z - currentCameraData.lastPosition.z);
		offset -= CameraOffset * (CameraDirection);

		// Update camera data
		currentCameraData.cameraOffset += CameraOffset;
		currentCameraData.lastPosition = c->GetFrom();

		// Update geometry displacement
		noVec3 vOffset;
		vOffset.Set(offset, offset, offset);
		currentCameraData.geometryDisplacement += vOffset;

		if (currentCameraData.geometryDisplacement.z < 0 || currentCameraData.geometryDisplacement.z > (mC-mB)/mNc)
		{
			currentCameraData.geometryDisplacement.z -= ((mC-mB)/mNc)*noMath::Floor((currentCameraData.geometryDisplacement.z)/((mC-mB)/mNc));
		}

		if (currentCameraData.geometryDisplacement.y < 0 || currentCameraData.geometryDisplacement.y > (mB-mA)/mNb)
		{
			currentCameraData.geometryDisplacement.y -= ((mB-mA)/mNb)*noMath::Floor((currentCameraData.geometryDisplacement.y)/((mB-mA)/mNb));
		}

		if (currentCameraData.geometryDisplacement.x < 0 || currentCameraData.geometryDisplacement.x > mA/mNa)
		{
			currentCameraData.geometryDisplacement.x -= (mA/mNa)*noMath::Floor((currentCameraData.geometryDisplacement.x)/(mA/mNa));
		}

		// Check under/over cloud rendering
		mCurrentDistance = c->GetFrom()-mSceneNode->WorldTrans();

		for (int k = 0; k < mNumberOfBlocks; k++)
		{
			mGeometryBlocks.at(k)->setWorldOffset(mWorldOffset + currentCameraData.cameraOffset);
			mGeometryBlocks.at(k)->updateGeometry(c, currentCameraData.geometryDisplacement, mCurrentDistance);
		}
		#endif
	}

	void GeometryManager::Render()
	{
		for(uint32 k = 0; k < mGeometryBlocks.size(); k++)
		{
			mGeometryBlocks.at(k)->Render();
		}
	}

}