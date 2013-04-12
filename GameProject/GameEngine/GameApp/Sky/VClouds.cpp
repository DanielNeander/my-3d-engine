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
#include "VClouds.h"
#include "GameApp/GameApp.h"

#include "VCloudSkySystem.h"

namespace VCloud { 

	VClouds::VClouds()
		: mCamera(0)
		, mCreated(false)
		, mGeometrySettings(GeometrySettings())
		, mDistanceFallingParams(noVec2(1,-1))		
		, mWindDirection(0)
		, mWindSpeed(80.0f)
		, mWheater(noVec2(0.5f, 1.0f))
		, mDelayedResponse(false)
		, mSunDirection(noVec3(0,-1,0))
		, mSunColor(noVec3(1,1,1))
		, mAmbientColor(noVec3(0.63f,0.63f,0.7f))
		, mLightResponse(noVec4(0.25f,0.2f,1.0f,0.1f))
		, mAmbientFactors(noVec4(0.45f,0.3f,0.6f,1))
		, mGlobalOpacity(1.0f)
		, mCloudFieldScale(1.0f)
		, mNoiseScale(4.2f)
		, mVisible(true)
		, mDataManager(new DataManager(this))
		, mGeometryManager(new GeometryManager(this))
		, mLightningManager(new LightningManager(this))
		, mCamerasData(std::vector<CameraData>())
		, mVolCloudsMaterial(-1)
		, mVolCloudsLightningMaterial(-1)
	{
	}

	VClouds::~VClouds()
	{
		remove();

		delete mDataManager;
		delete mGeometryManager;
		delete mLightningManager;
	}

	void VClouds::create()
	{
		remove();

		mVolCloudsMaterial = GetApp()->getRenderer()->addShader("Data/Shaders/VolumeClouds.hlsl");
		mVolCloudsLightningMaterial = GetApp()->getRenderer()->addShader("Data/Shaders/VolumeCloudsLightning.hlsl");
		// Data manager
		mDataManager->create(128,128,20);

		// Geometry manager
		mGeometryManager->create(mGeometrySettings.Height, mGeometrySettings.Radius, mGeometrySettings.Alpha, 
			mGeometrySettings.Beta, mGeometrySettings.NumberOfBlocks, mGeometrySettings.Na, mGeometrySettings.Nb, mGeometrySettings.Nc);

		mGeometryManager->getSceneNode()->setVisible(mVisible);


		GetApp()->getRenderer()->setShader(mVolCloudsMaterial);
		GetApp()->getRenderer()->setShaderConstant1f("uRadius", mGeometrySettings.Radius);
		GetApp()->getRenderer()->setShader(mVolCloudsLightningMaterial);
		GetApp()->getRenderer()->setShaderConstant1f("uRadius", mGeometrySettings.Radius);		

		// Lightning manager
		mLightningManager->create();

		mCreated = true;

		// Update material parameters
		setSunColor(mSunColor);
		setAmbientColor(mAmbientColor);
		setLightResponse(mLightResponse);
		setAmbientFactors(mAmbientFactors);

		// Set current wheater
		setWheater(mWheater.x, mWheater.y, mDelayedResponse);
	}

	void VClouds::create(const GeometrySettings& gs)
	{
		// Update geometry settings
		mGeometrySettings = gs;

		create();
	}

	void VClouds::create(const noVec2& Height, const float& Radius)
	{
		// Update geometry params
		mGeometrySettings.Height = Height;
		mGeometrySettings.Radius = Radius;

		create();
	}

	void VClouds::remove()
	{
		if (!mCreated)
		{
			return;
		}

		mDataManager->remove();
		mGeometryManager->remove();
		mLightningManager->remove();

		mCamera = 0;
		mCamerasData.clear();
		

		mCreated = false;
	}

	void VClouds::update(const float& timeSinceLastFrame)
	{
		if (!mCreated)
		{
			return;
		}

		mDataManager->update(timeSinceLastFrame);
		mGeometryManager->update(timeSinceLastFrame);
		mLightningManager->update(timeSinceLastFrame);

		if (mLightningManager->isEnabled())
		{
			GetRenderer()->setShader(mVolCloudsLightningMaterial);
			GetRenderer()->setShaderConstant1f("uInterpolation", mDataManager->_getInterpolation());			
			
			GetRenderer()->setShaderConstant3f("uSunDirection", (-mSunDirection).ToFloatPtr());									
		}
		else
		{
			GetRenderer()->setShader(mVolCloudsMaterial);
			GetRenderer()->setShaderConstant1f("uInterpolation", mDataManager->_getInterpolation());
			GetRenderer()->setShaderConstant3f("uSunDirection", (-mSunDirection).ToFloatPtr());											
		}
	}

	//void VClouds::notifyCameraRender(Camera* c, const float& timeSinceLastCameraFrame)

		//if (!mCreated)
		//{
		//	return;
		//}

		//mCamera = c;

		//// Check if the camera is registered
		//bool isRegistered = false;
		//for (uint32 k = 0; k < mCamerasData.size(); k++)
		//{
		//	if (mCamerasData.at(k).camera == c)
		//	{
		//		isRegistered = true;
		//		break;
		//	}
		//}

		//if (!isRegistered)
		//{
		//	mCamerasData.push_back(CameraData(c));
		//	SkyXLOG("VClouds warning: unregistered camera registered, manual unregistering is needed before camera destruction");
		//}

		//mGeometryManager->updateGeometry(c, timeSinceLastCameraFrame);
		//mLightningManager->updateMaterial();

		//mLightningManager->_updateRenderQueueGroup(mGeometryManager->_getCurrentDistance().y < mGeometryManager->getHeight().y/2 ?
		//	mRenderQueueGroups.vcloudsLightningsUnder : mRenderQueueGroups.vcloudsLightningsOver);
	

	void VClouds::registerCamera(BaseCamera* c)
	{
		for (uint32 k = 0; k < mCamerasData.size(); k++)
		{
			if (mCamerasData.at(k).camera == c)
			{
				return;
			}
		}

		mCamerasData.push_back(CameraData(c));
	}

	void VClouds::unregisterCamera(BaseCamera* c)
	{
		for (std::vector<CameraData>::iterator it = mCamerasData.begin(); it != mCamerasData.end(); it++)
		{
			if ((*it).camera == c)
			{
				mCamerasData.erase(it);
				return;
			}
		}
	}

	void VClouds::setVisible(const bool& visible)
	{
		mVisible = visible;

		if (!mCreated)
		{
			return;
		}

		mGeometryManager->getSceneNode()->setVisible(mVisible);
		mLightningManager->_setVisible(mVisible);
	}

	void VClouds::setSunColor(const noVec3& SunColor)
	{
		mSunColor = SunColor;

		if (!mCreated)
		{
			return;
		}
		GetRenderer()->setShader(mVolCloudsMaterial);
		GetRenderer()->setShaderConstant3f("uSunColor", mSunColor.ToFloatPtr());
		GetRenderer()->setShader(mVolCloudsLightningMaterial);
		GetRenderer()->setShaderConstant3f("uSunColor", mSunColor.ToFloatPtr());
	}

	void VClouds::setAmbientColor(const noVec3& AmbientColor)
	{
		mAmbientColor = AmbientColor;

		if (!mCreated)
		{
			return;
		}

		GetRenderer()->setShader(mVolCloudsMaterial);
		GetRenderer()->setShaderConstant3f("uAmbientColor", mAmbientColor.ToFloatPtr());
		GetRenderer()->setShader(mVolCloudsLightningMaterial);
		GetRenderer()->setShaderConstant3f("uAmbientColor", mAmbientColor.ToFloatPtr());			
		
	}

	void VClouds::setLightResponse(const noVec4& LightResponse)
	{
		mLightResponse = LightResponse;

		if (!mCreated)
		{
			return;
		}

		GetRenderer()->setShader(mVolCloudsMaterial);
		GetRenderer()->setShaderConstant4f("uLightResponse", mLightResponse.ToFloatPtr());
		GetRenderer()->setShader(mVolCloudsLightningMaterial);
		GetRenderer()->setShaderConstant4f("uLightResponse", mLightResponse.ToFloatPtr());						
	}

	void VClouds::setAmbientFactors(const noVec4& AmbientFactors)
	{
		mAmbientFactors = AmbientFactors;

		if (!mCreated)
		{
			return;
		}

		GetRenderer()->setShader(mVolCloudsMaterial);
		GetRenderer()->setShaderConstant4f("uAmbientFactors", mAmbientFactors.ToFloatPtr());
		GetRenderer()->setShader(mVolCloudsLightningMaterial);
		GetRenderer()->setShaderConstant4f("uAmbientFactors", mAmbientFactors.ToFloatPtr());										
	}

	void VClouds::setWheater(const float& Humidity, const float& AverageCloudsSize, const bool& DelayedResponse)
	{
		mWheater = noVec2(Humidity, AverageCloudsSize);
		mDelayedResponse = DelayedResponse;

		if (!mCreated)
		{
			return;
		}

		mDataManager->setWheater(mWheater.x, mWheater.y, mDelayedResponse);
	}

	void VClouds::RenderCloud( BaseCamera* c, float timeSinceLastCameraFrame)
	{
		mCamera = c;

		// Check if the camera is registered
		bool isRegistered = false;
		for (uint32 k = 0; k < mCamerasData.size(); k++)
		{
			if (mCamerasData.at(k).camera == c)
			{
				isRegistered = true;
				break;
			}
		}

		if (!isRegistered)
		{
			mCamerasData.push_back(CameraData(c));
		}

		mGeometryManager->updateGeometry(c, timeSinceLastCameraFrame);
		mLightningManager->updateMaterial();

		// Render
		//mLightningManager->_updateRenderQueueGroup(mGeometryManager->_getCurrentDistance().y < mGeometryManager->getHeight().y/2 ?
		//	mRenderQueueGroups.vcloudsLightningsUnder : mRenderQueueGroups.vcloudsLightningsOver);

		if (mGeometryManager->_getCurrentDistance().y < mGeometryManager->getHeight().y/2)
		{
			mGeometryManager->Render();
			//mLightningManager->Render();
		}
		else 
		{
			//mLightningManager->Render();
			mGeometryManager->Render();			
		}

	}

}