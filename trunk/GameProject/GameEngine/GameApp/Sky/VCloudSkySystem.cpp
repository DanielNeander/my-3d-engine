_setLightParameters
/*
--------------------------------------------------------------------------------
This source file is part of VCloudSkySystem.
Visit http://www.paradise-studios.net/products/VCloudSkySystem/

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

#pragma warning(disable:4355)

#include "stdafx.h"
#include "VCloudSkySystem.h"
#include "GameApp/GameApp.h"

namespace VCloud
{
	VCloudSkySystem::VCloudSkySystem(Controller* c)
		: mController(c)
		, mCamera(0)
		, mMeshManager(new SkyDomeMesh(this))
		, mAtmosphereManager(new AtmosphereManager(this))		
		, mMoonManager(new MoonManager(this))
		, mCloudsManager(new CloudsManager(this))		
		, mCreated(false)
		, mLastCameraPosition(noVec3(0,0,0))
		, mLastCameraFarClipDistance(-1)
		, mInfiniteCameraFarClipDistance(100000)
		, mVisible(true)
		, mLightingMode(LM_LDR)
		, mStarfield(true)
		, mTimeMultiplier(0.1f)
		, mTimeOffset(0.0f)
	{
		// Need to be instanced here, when VCloudSkySystem::mSceneManager is valid
		mVCloudsManager = new VCloudsManager(this);
	}

	VCloudSkySystem::~VCloudSkySystem()
	{
		remove();

		delete mMeshManager;
		delete mAtmosphereManager;		
		delete mMoonManager;
		delete mCloudsManager;
		delete mVCloudsManager;

		if (mController->getDeleteBySkyX())
		{
			delete mController;
		}
	}

	void VCloudSkySystem::create()
	{
		if (mCreated)
		{
			return;
		}
				
		linearWarp = GetRenderer()->addSamplerState(TRILINEAR_ANISO, WRAP, WRAP, WRAP);
		// Load Skydom Shader		
		mSkydomeShader = GetApp()->getRenderer()->addShader("Data/Shaders/SkyDome.hlsl");
		mCloudShader = GetApp()->getRenderer()->addShader("Data/Shaders/Clouds.hlsl");

		FormatDesc vf_format[] = {
			0, TYPE_VERTEX, FORMAT_FLOAT,  3,
			0, TYPE_TEXCOORD, FORMAT_FLOAT,  3,
		};

		mCloudVF = GetRenderer()->addVertexFormat(vf_format, elementsOf(vf_format), mCloudShader);

		mDiffuseTex = GetRenderer()->addTexture("Textures/Clouds.png", true);
		mNormalTex = GetRenderer()->addTexture("Textures/CloudsNormal.png", true);
		mTileTex = GetRenderer()->addTexture("Textures/CloudsTile.png", true);
		mStarfieldTex = GetRenderer()->addTexture("Textures/Starfield.png", true);

		mMeshManager->setMaterial(mSkydomeShader);
		mMeshManager->create();
					
		mAtmosphereManager->_update(mAtmosphereManager->getOptions(), true);

		mMoonManager->create();

		setVisible(mVisible);

		mLastCameraPosition = noVec3(0,0,0);
		mLastCameraFarClipDistance = -1;

		//mVCloudsManager->create(-1);

		mCreated = true;
	}

	void VCloudSkySystem::remove()
	{
		if (!mCreated)
		{
			return;
		}

		mCloudsManager->removeAll();
		mMeshManager->remove();
		mMoonManager->remove();
		mVCloudsManager->remove();

		mCamera = 0;

		mCreated = false;
	}

	void VCloudSkySystem::update(const float& timeSinceLastFrame)
	{
		if (!mCreated)
		{
			return;
		}

		GetRenderer()->setShader(mSkydomeShader);

		if (mTimeMultiplier != 0)
		{
			float timemultiplied = timeSinceLastFrame * mTimeMultiplier;

			mTimeOffset += timemultiplied;

			mController->update(timemultiplied);

			if (mStarfield)
			{
				GetApp()->getRenderer()->setShaderConstant1f("uTime", mTimeOffset*0.5f);
			}
		}
				
		GetRenderer()->setShader(mSkydomeShader);
		GetApp()->getRenderer()->setShaderConstant3f("uLightDir", mController->getSunDirection().ToFloatPtr());
		
		mMoonManager->updateMoonPhase(mController->getMoonPhase());
		mCloudsManager->update();
		mVCloudsManager->update(timeSinceLastFrame);
	}

	/*void VCloudSkySystem::notifyCameraRender(Camera* c)
	{
		if (!mCreated)
		{
			return;
		}

		mCamera = c;
		
		if (mLastCameraPosition != c->getDerivedPosition())
		{
			mMeshManager->getSceneNode()->setPosition(mCamera->getDerivedPosition());

			mLastCameraPosition = mCamera->getDerivedPosition();
		}

		if (mLastCameraFarClipDistance != c->GetFar())
		{
			mMeshManager->updateGeometry(c);

			mLastCameraFarClipDistance = mCamera->GetFar();
		}

		mMoonManager->updateGeometry(c);

		mVCloudsManager->notifyCameraRender(c);
	}*/

	void VCloudSkySystem::setVisible(const bool& visible)
	{
		mVisible = visible;

		if (!mCreated)
		{
			return;
		}

		mMeshManager->getSceneNode()->setVisible(mVisible);
		mMoonManager->getMoonSceneNode()->setVisible(mVisible);

		if (mVCloudsManager->isCreated())
		{
			mVCloudsManager->getVClouds()->setVisible(mVisible);
		}
	}

	void VCloudSkySystem::setRenderQueueGroups(const RenderQueueGroups& rqg)
	{
	/*	mRenderQueueGroups = rqg;

		mVCloudsManager->getVClouds()->setRenderQueueGroups(
			VClouds::VClouds::RenderQueueGroups(mRenderQueueGroups.vclouds, mRenderQueueGroups.vcloudsLightningsUnder, mRenderQueueGroups.vcloudsLightningsOver));

		if (!mCreated)
		{
			return;
		}

		mMeshManager->getEntity()->setRenderQueueGroup(mRenderQueueGroups.skydome);
		mMoonManager->getMoonBillboard()->setRenderQueueGroup(mRenderQueueGroups.skydome+1);*/
	}

	void VCloudSkySystem::setLightingMode(const LightingMode& lm)
	{
		mLightingMode = lm;

		if (!mCreated)
		{
			return;
		}

		// Update skydome material
		
		// Update layered clouds material
		mCloudsManager->registerAll();
		// Update ground passes materials and update textures gamma correction
		//mGPUManager->_updateFP();

		// Update parameters
		mAtmosphereManager->_update(mAtmosphereManager->getOptions(), true);
	}

	void VCloudSkySystem::setStarfieldEnabled(const bool& Enabled)
	{
		mStarfield = Enabled;

		if (!mCreated)
		{
			return;
		}
		
		// Update skydome material
		
		// Update parameters
		mAtmosphereManager->_update(mAtmosphereManager->getOptions(), true);

	}

	void VCloudSkySystem::RenderSky( BaseCamera* c)
	{
		mCamera = c;

		if (mLastCameraPosition != c->GetFrom())
		{
			mMeshManager->getSceneNode()->SetTrans(mCamera->GetFrom());
			mLastCameraPosition = mCamera->GetFrom();
		}

		if (mLastCameraFarClipDistance != c->GetFar())
		{
			mMeshManager->updateGeometry(c);
			mLastCameraFarClipDistance = mCamera->GetFar();
		}
				
				
		mMeshManager->Render(c);
		mCloudsManager->Render(c);
		
		//mMoonManager->updateGeometry(c);
		//mMoonManager->Render(c);		
		//mVCloudsManager->Render(c);
	}

}