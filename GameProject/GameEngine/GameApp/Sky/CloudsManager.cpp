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

#include "CloudsManager.h"
#include "VCloudSkySystem.h"
#include "GameApp/GameApp.h"
#include "GameApp/BaseCamera.h"

namespace VCloud
{
	/// -------------- CloudLayer -----------------
	CloudLayer::CloudLayer(VCloudSkySystem *s)
		: mSkyX(s)
		, mOptions(Options())		
	{
		mAmbientGradient = ColorGradient();
		mAmbientGradient.addCFrame(ColorGradient::ColorFrame(noVec3(1,1,1)*0.95f, 1.0f));
		mAmbientGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.7,0.7,0.65), 0.625f)); 
		mAmbientGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.6,0.55,0.4), 0.5625f));
		mAmbientGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.6,0.45,0.3)*0.4, 0.5f));
		mAmbientGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.5,0.25,0.25)*0.1, 0.45f));
		mAmbientGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.2,0.2,0.3)*0.1, 0.35f));
		mAmbientGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.2,0.2,0.5)*0.15, 0));

		mSunGradient = ColorGradient();
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(1,1,1)*0.95f, 1.0f));
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(1,1,1)*0.8, 0.75f));
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.8,0.75,0.55)*1.3, 0.5625f));
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.6,0.5,0.2)*0.75, 0.5f));
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.6,0.5,0.2)*0.35, 0.4725f));
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.5,0.5,0.5)*0.15, 0.45f));
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.2,0.2,0.25)*0.5, 0.3f));
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.5,0.5,0.5)*0.35, 0.0f));
	}

	CloudLayer::CloudLayer(VCloudSkySystem *s, const Options& o)
		: mSkyX(s)
		, mOptions(o)		
	{
		mAmbientGradient = ColorGradient();
		mAmbientGradient.addCFrame(ColorGradient::ColorFrame(noVec3(1,1,1)*0.95f, 1.0f));
		mAmbientGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.7,0.7,0.65), 0.625f)); 
		mAmbientGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.6,0.55,0.4), 0.5625f));
		mAmbientGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.6,0.45,0.3)*0.4, 0.5f));
		mAmbientGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.5,0.25,0.25)*0.1, 0.45f));
		mAmbientGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.2,0.2,0.3)*0.1, 0.35f));
		mAmbientGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.2,0.2,0.5)*0.15, 0));

		mSunGradient = ColorGradient();
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(1,1,1)*0.95f, 1.0f));
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(1,1,1)*0.8, 0.75f));
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.8,0.75,0.55)*1.3, 0.5625f));
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.6,0.5,0.2)*0.75, 0.5f));
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.6,0.5,0.2)*0.35, 0.4725f));
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.5,0.5,0.5)*0.15, 0.45f));
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.2,0.2,0.25)*0.5, 0.3f));
		mSunGradient.addCFrame(ColorGradient::ColorFrame(noVec3(0.5,0.5,0.5)*0.35, 0.0f));
	}

	CloudLayer::~CloudLayer()
	{
		_unregister();
	}

	void CloudLayer::_registerCloudLayer()
	{
		_unregister();

		mRd.blendMode = GetApp()->GetBlendMode();
		mRd.cull = GetApp()->cullNone;
		mRd.depthMode = GetApp()->noDepthWrite;

		if (mSkyX->getLightingMode() == VCloudSkySystem::LM_LDR)
		{

		}
		else 
		{

		}
		mDiffuseTex = mSkyX->mDiffuseTex;
		mNormalTex = mSkyX->mNormalTex;
		mTileTex = mSkyX->mTileTex;
		

		_updatePassParameters();
		_updateInternalPassParameters();
	}
		
	void CloudLayer::_unregister()
	{
	
	}

	void CloudLayer::_updatePassParameters()
	{
		GetRenderer()->setShader(mSkyX->mCloudShader);
		GetRenderer()->setShaderConstant1f("uScale", mOptions.Scale);
		GetRenderer()->setShaderConstant1f("uHeight", mOptions.Height);

		vec2 WindDirection(mOptions.WindDirection.x, mOptions.WindDirection.y);
		GetRenderer()->setShaderConstant2f("uWindDirection", WindDirection);
		GetRenderer()->setShaderConstant1f("uCloudLayerHeightVolume", mOptions.HeightVolume);
		GetRenderer()->setShaderConstant1f("uCloudLayerVolumetricDisplacement", mOptions.VolumetricDisplacement);
		GetRenderer()->setShaderConstant1f("uDetailAttenuation", mOptions.DetailAttenuation);
		GetRenderer()->setShaderConstant1f("uDistanceAttenuation", mOptions.DistanceAttenuation);	
	}
	
	void CloudLayer::_updateInternalPassParameters()
	{
		GetRenderer()->setShader(mSkyX->mCloudShader);
		GetRenderer()->setShaderConstant1f("uExposure", mSkyX->getAtmosphereManager()->getOptions().Exposure);
		GetRenderer()->setShaderConstant1f("uTime", mSkyX->_getTimeOffset()*mOptions.TimeMultiplier);

		float point = (mSkyX->getController()->getSunDirection().y + 1.0f) / 2.0f;
		GetRenderer()->setShaderConstant3f("uSunColor", mSunGradient.getColor(point).ToFloatPtr());
		GetRenderer()->setShaderConstant3f("uAmbientLuminosity", mAmbientGradient.getColor(point).ToFloatPtr());
		
		//mCloudLayerPass->getFragmentProgramParameters()
		//	->setNamedConstant("uExposure", mSkyX->getAtmosphereManager()->getOptions().Exposure);
		//mCloudLayerPass->getFragmentProgramParameters()
		//	->setNamedConstant("uTime", mSkyX->_getTimeOffset()*mOptions.TimeMultiplier);
		/*
		mCloudLayerPass->getFragmentProgramParameters()
			->setNamedConstant("uSunPosition", -mSkyX->getAtmosphereManager()->getSunDirection()*mSkyX->getMeshManager()->getSkydomeRadius());
		
		noVec3 AmbientColor = noVec3::ZERO;

		noVec3 SunDir   = -mSkyX->getAtmosphereManager()->getSunDirection();
		float Ang = 0;
		for (int k = 0; k < 3; k++)
		{
			noVec2 Coords = noVec2(noMath::Cos(Ang), noMath::Sin(Ang));
			Ang += 2*noMath::PI/3;
			AmbientColor += mSkyX->getAtmosphereManager()->getColorAt(noVec3(Coords.x, mOptions.Height/mSkyX->getMeshManager()->getSkydomeRadius(), Coords.y).normalisedCopy());
		}

		AmbientColor /= 3;

		mCloudLayerPass->getFragmentProgramParameters()
			->setNamedConstant("uAmbientLuminosity", AmbientColor*0.75f);

		float Mult = 1.5f;

		mCloudLayerPass->getFragmentProgramParameters()
			->setNamedConstant("uSunColor", noVec3(
			     noMath::Clamp<float>(AmbientColor.x*Mult, 0, 1),
			     noMath::Clamp<float>(AmbientColor.y*Mult, 0, 1),
			     noMath::Clamp<float>(AmbientColor.z*Mult, 0, 1)));
		*/

		//noVec3 SunDir = mSkyX->getAtmosphereManager()->getSunDirection();
		//if (SunDir.y > 0.15f)
		//{
		//	SunDir = -SunDir;
		//}

	//	mCloudLayerPass->getFragmentProgramParameters()
	//		->setNamedConstant("uSunPosition", -SunDir*mSkyX->getMeshManager()->getSkydomeRadius());

		//float point = (mSkyX->getController()->getSunDirection().y + 1.0f) / 2.0f;

		/*mCloudLayerPass->getFragmentProgramParameters()
		->setNamedConstant("uSunColor", mSunGradient.getColor(point));
		mCloudLayerPass->getFragmentProgramParameters()
		->setNamedConstant("uAmbientLuminosity", mAmbientGradient.getColor(point));*/
	}

	void CloudLayer::Render(BaseCamera* c)
	{
		GetRenderer()->reset();
		GetRenderer()->setShader(mSkyX->mCloudShader);
		
		GetRenderer()->setVertexFormat(mSkyX->mCloudVF);
		GetRenderer()->setVertexBuffer(0, mSkyX->getMeshManager()->mVB);
		GetRenderer()->setIndexBuffer(mSkyX->getMeshManager()->mIB);

		GetRenderer()->setDepthState(mSkyX->getMeshManager()->mDepthWrite);
		GetRenderer()->setTexture("CloudTex", mSkyX->mDiffuseTex);
		GetRenderer()->setTexture("CloudNormal", mSkyX->mNormalTex);
		GetRenderer()->setTexture("CloudTileTex", mSkyX->mTileTex);

		GetRenderer()->setSamplerState("uClouds", mSkyX->linearWarp);		
		GetRenderer()->setSamplerState("uCloudsNormal", mSkyX->linearWarp);
		GetRenderer()->setSamplerState("uCloudsTile", mSkyX->linearWarp);		
		
		GetRenderer()->setBlendState(GetApp()->GetBlendMode(5));

		mat4 view =ViewMat();
		view.rows[3].x = view.rows[3].y = view.rows[3].z = 0.f;
		mat4 wvp =  view * ProjMat();
		GetRenderer()->setShaderConstant4x4f("uWorldViewProj", transpose(wvp));
		GetRenderer()->apply();
		GetRenderer()->drawElements(PRIM_TRIANGLES, 0, mSkyX->getMeshManager()->mNumIndices, 0, 0);	
	}

	/// ------------- CloudsManager ---------------
	CloudsManager::CloudsManager(VCloudSkySystem *s)
		: mSkyX(s)
	{
	}

	CloudsManager::~CloudsManager()
	{
		removeAll();
	}

	CloudLayer* CloudsManager::add(const CloudLayer::Options& o)
	{
		CloudLayer *NewCloudLayer = new CloudLayer(mSkyX, o);

		// TODO
		NewCloudLayer->_registerCloudLayer();

		mCloudLayers.push_back(NewCloudLayer);

		bool changeOrder = false;

		// Short layers by height
		for (unsigned int k = 0; k < mCloudLayers.size(); k++)
		{
			if (k+1 < mCloudLayers.size())
			{
				if (mCloudLayers.at(k)->getOptions().Height < mCloudLayers.at(k+1)->getOptions().Height)
				{
					// Swap
					CloudLayer* cl = mCloudLayers.at(k);
					mCloudLayers.at(k) = mCloudLayers.at(k+1);
					mCloudLayers.at(k+1) = cl;

					changeOrder = true;
					k = 0;
				}
			}
		}

		if (changeOrder)
		{
			unregisterAll();
			registerAll();
		}

		return NewCloudLayer;
	}

	void CloudsManager::remove(CloudLayer* cl)
	{
		for(CloudLayersIt = mCloudLayers.begin(); CloudLayersIt != mCloudLayers.end(); CloudLayersIt++)
        {
            if((*CloudLayersIt) == cl)
            {
				delete (*CloudLayersIt);
				mCloudLayers.erase(CloudLayersIt);
				return;
            }
		}
	}

	void CloudsManager::removeAll()
	{
		for(CloudLayersIt = mCloudLayers.begin(); CloudLayersIt != mCloudLayers.end(); CloudLayersIt++)
        {
			delete (*CloudLayersIt);
		}

		mCloudLayers.clear();
	}

	void CloudsManager::registerAll()
	{
		for(unsigned int k = 0; k < mCloudLayers.size(); k++)
		{
			mCloudLayers.at(k)->_registerCloudLayer();
		}
	}

	void CloudsManager::unregister(CloudLayer* cl)
	{
		for(CloudLayersIt = mCloudLayers.begin(); CloudLayersIt != mCloudLayers.end(); CloudLayersIt++)
        {
            if((*CloudLayersIt) == cl)
            {
				(*CloudLayersIt)->_unregister();
            }
		}
	}

	void CloudsManager::unregisterAll()
	{
		for(CloudLayersIt = mCloudLayers.begin(); CloudLayersIt != mCloudLayers.end(); CloudLayersIt++)
        {
			(*CloudLayersIt)->_unregister();
		}
	}

	void CloudsManager::update()
	{
		for(CloudLayersIt = mCloudLayers.begin(); CloudLayersIt != mCloudLayers.end(); CloudLayersIt++)
        {
			(*CloudLayersIt)->_updateInternalPassParameters();
		}
	}

	void CloudsManager::Render(BaseCamera* c)
	{
		for(CloudLayersIt = mCloudLayers.begin(); CloudLayersIt != mCloudLayers.end(); CloudLayersIt++)
		{
			(*CloudLayersIt)->Render(c);
		}
	}
}