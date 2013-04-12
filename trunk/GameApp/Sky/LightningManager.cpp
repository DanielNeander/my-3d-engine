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
#include "LightningManager.h"
#include "VClouds.h"
#include "GameApp/GameApp.h"

namespace VCloud {


	LightningManager::LightningManager(VClouds* vc)
		: mVClouds(vc)
		, mLightnings(std::vector<Lightning*>())
		, mSceneNodes(std::vector<SceneNode*>())
		, mEnabled(false)
		, mLightningColor(noVec3(1,0.925f,0.85f))
		, mLightningTimeMultiplier(2.0f)
		, mAverageLightningApparitionTime(1.5f)
		, mRemainingTime(1.5f)
		, mVolCloudsLightningMaterial(-1)
		, mLightningMaterial(-1)
		, mListeners(std::vector<Listener*>())
		, mCreated(false)
	{
	}

	LightningManager::~LightningManager()
	{
		remove();
	}

	void LightningManager::create()
	{
		remove();

		mVolCloudsLightningMaterial = mVClouds->mVolCloudsLightningMaterial;			
		mLightningMaterial = GetApp()->getRenderer()->addShader("Data/Shaders/Lightning.hlsl");
				
		if (mEnabled)
		{
			//mVClouds->getGeometryManager()->_setMaterialName("SkyX_VolClouds_Lightning");
			mVClouds->getGeometryManager()->_setMaterial(mLightningMaterial);
		}
		else
		{
			mVClouds->getGeometryManager()->_setMaterial(mVClouds->mVolCloudsMaterial);
		}

		mCreated = true;

		setLightningColor(mLightningColor);
	}

	void LightningManager::remove()
	{
		if (!mCreated)
		{
			return;
		}

		for(uint32 k = 0; k < mLightnings.size(); k++)
		{
			delete mLightnings.at(k);			
		}

		mLightnings.clear();
		mSceneNodes.clear();

		removeListeners();
		
		mCreated = false;
	}

	void LightningManager::update(const float& timeSinceLastFrame)
	{
		if (!mCreated)
		{
			return;
		}

		if (mEnabled)
		{
			mRemainingTime -= timeSinceLastFrame;

			if (mRemainingTime <= 0)
			{
				mRemainingTime = randfloat(0, 2*mAverageLightningApparitionTime);

				// Select a random camera to place the lightning
				if (!mVClouds->_getCamerasData().empty())
				{
					BaseCamera* c = mVClouds->_getCamerasData().at(mVClouds->_getCamerasData().size()*0.999).camera;

					float prob = RandInt(0,1);
					
					// Cloud-to-ground
					if (prob < 0.5)
					{
						addLightning(
							// Ray position
							noVec3(c->GetFrom().x + randfloat(-c->GetFar()*0.5,
							c->GetFar()*0.5)/RandInt(1,5), 
							mVClouds->getGeometrySettings().Height.x + 0.2 * 
							mVClouds->getGeometrySettings().Height.y, c->GetFrom().z + randfloat(-c->GetFar()*0.5,c->GetFar()*0.5)/RandInt(1,5)), 
							// Ray direction
							noVec3(0,-1,0), 
							// Ray length
							mVClouds->getGeometrySettings().Height.x + 0.1*mVClouds->getGeometrySettings().Height.y);
					}
					// Cloud-to-cloud
					else if (prob < 0.7)
					{
						addLightning(
							// Ray position
							noVec3(
									c->GetFrom().x + randfloat(-c->GetFar()*0.5,c->GetFar()*0.5)/RandInt(1,5), 
									mVClouds->getGeometrySettings().Height.x + 0.2*mVClouds->getGeometrySettings().Height.y,
									c->GetFrom().z + randfloat(-c->GetFar()*0.5,c->GetFar()*0.5)/RandInt(1,5)), 
									// Ray direction
									noVec3(RandInt(-1,1), randfloat(-0.1f,0.1f), RandInt(-1,1)).NormalizeCopy(), 
									// Ray length
									randfloat(0.5,1.5f) * 0.2 * mVClouds->getGeometrySettings().Height.y
									);
					}
					// Cloud-to-ground + cloud-to-cloud
					else
					{
						addLightning(
							// Ray position
							noVec3(
								c->GetFrom().x + randfloat(-c->GetFar()*0.5, c->GetFar()*0.5) / RandInt(1,5), 
								mVClouds->getGeometrySettings().Height.x + 0.2*mVClouds->getGeometrySettings().Height.y,
								c->GetFrom().z + randfloat(-c->GetFar()*0.5,c->GetFar()*0.5)/RandInt(1,5)), 
								// Ray direction
								noVec3(0,-1,0), 
								// Ray length
								mVClouds->getGeometrySettings().Height.x + 0.1*mVClouds->getGeometrySettings().Height.y);

						addLightning(
							// Ray position
							noVec3(
									c->GetFrom().x + randfloat(-c->GetFar()*0.5,c->GetFar()*0.5)/RandInt(1,5), 
									mVClouds->getGeometrySettings().Height.x + 0.2*mVClouds->getGeometrySettings().Height.y, 
									c->GetFrom().z + randfloat(-c->GetFar()*0.5,c->GetFar()*0.5) / randint(1,5)), 
							// Ray direction
							noVec3(
							randint(-1,1), 
							randfloat(-0.1f,0.1f), RandInt(-1,1)).NormalizeCopy(), 
							// Ray length
							randfloat(0.5f,1.5f)*0.2 * mVClouds->getGeometrySettings().Height.y);
					}

					updateMaterial();
				}
			}
		}

		for(std::vector<Lightning*>::iterator it = mLightnings.begin(); it != mLightnings.end();)
		{
			if ((*it)->isFinished())
			{
				SceneNode* sn = (*it)->getSceneNode();

				delete (*it);
				it = mLightnings.erase(it);

				// Remove the associated scene node
				for(std::vector<SceneNode*>::iterator it2 = mSceneNodes.begin(); it2 != mSceneNodes.end(); it2++)
				{
					if ((*it2) == sn)
					{
						//sn->getParentSceneNode()->removeAndDestroyChild(sn->getName());
						mSceneNodes.erase(it2);
						break;
					}
				}
			}
			else
			{
				(*it)->update(timeSinceLastFrame);
				it++;
			}
		}
	}

	Lightning* LightningManager::addLightning(const noVec3& p, const noVec3& d, const float l, const uint32& div)
	{
		if (!mCreated || mLightnings.size() == 3)
		{
			return static_cast<Lightning*>(NULL);
		}

		SceneNode* sn = SceneNode::NewNode();
		sn->SetTrans(p);

		Lightning* lightning = new Lightning(sn, noVec3(0,0,0), d, l, div, 3, mLightningTimeMultiplier, mVClouds->getGeometrySettings().Radius/9500);
		lightning->create();
		/*lightning->_updateRenderQueueGroup(
			mVClouds->getGeometryManager()->_getCurrentDistance().y < mVClouds->getGeometryManager()->getHeight().y/2 ?
			mVClouds->getRenderQueueGroups().vcloudsLightningsUnder : mVClouds->getRenderQueueGroups().vcloudsLightningsOver);
		lightning->getBillboardSet()->setVisible(mVClouds->isVisible());
*/
		mSceneNodes.push_back(sn);
		mLightnings.push_back(lightning);

		for(uint32 k = 0; k < mListeners.size(); k++)
		{
			mListeners.at(k)->lightningAdded(lightning);
		}

		return lightning;
	}

	void LightningManager::updateMaterial()
	{
		noVec3 pos;

		for(uint32 k = 0; k < 3; k++)
		{
			if (k < mLightnings.size())
			{
				noTransform invertTM;
				mVClouds->getGeometryManager()->getSceneNode()->worldTM_.Invert(invertTM);
				pos = invertTM * mSceneNodes.at(k)->WorldTrans();

				wxString name = wxString::Format("uLightning%d", k);
				GetRenderer()->setShader(mVolCloudsLightningMaterial);
				GetRenderer()->setShaderConstant4f(name.wx_str(), vec4(pos.x, pos.y, pos.z, mLightnings.at(k)->getIntensity()));				
			}
			else
			{
				wxString name = wxString::Format("uLightning%d", k);
				GetRenderer()->setShader(mVolCloudsLightningMaterial);
				GetRenderer()->setShaderConstant4f(name.wx_str(), vec4_zero.ToFloatPtr());								
			}
		}
	}

	void LightningManager::setLightningColor(const noVec3& c)
	{
		mLightningColor = c;

		if (!mCreated)
		{
			return;
		}
		GetRenderer()->setShader(mVolCloudsLightningMaterial);
			
		GetRenderer()->setShader(mVolCloudsLightningMaterial);
		GetRenderer()->setShaderConstant3f("uLightningColor", mLightningColor.ToFloatPtr());								

		GetRenderer()->setShader(mLightningMaterial);
		GetRenderer()->setShaderConstant3f("uColor", mLightningColor.ToFloatPtr());								

		/*mVolCloudsLightningMaterial->getTechnique(0)->getPass(0)->getFragmentProgramParameters()
			->setNamedConstant("uLightningColor", mLightningColor);

		mLightningMaterial->getTechnique(0)->getPass(0)->getFragmentProgramParameters()
			->setNamedConstant("uColor", mLightningColor);*/
	}

	void LightningManager::removeListener(Listener* listener)
	{
		for(std::vector<Listener*>::iterator it = mListeners.begin(); it != mListeners.end(); it++)
		{
			if ((*it) == listener)
			{
				mListeners.erase(it);
				return;
			}
		}
	}

	void LightningManager::setEnabled(const bool& enable)
	{
		mEnabled = enable;

		if (mCreated)
		{
			if (mEnabled)
			{
				//mVClouds->getGeometryManager()->_setMaterialName("SkyX_VolClouds_Lightning");
				mVClouds->getGeometryManager()->_setMaterial(mVolCloudsLightningMaterial);
			}
			else
			{
				mVClouds->getGeometryManager()->_setMaterial(mVClouds->mVolCloudsMaterial);				
			}
		}
	}

	void LightningManager::_updateRenderQueueGroup(const uint8& rqg)
	{
		for(uint32 k = 0; k < mLightnings.size(); k++)
		{	
			//mLightnings.at(k)->_updateRenderQueueGroup(rqg);
		}
	}

	void LightningManager::_setVisible(const bool& v)
	{
		for(uint32 k = 0; k < mLightnings.size(); k++)
		{	
			mLightnings.at(k)->getBillboardSet()->setVisible(v);
		}
	}

}