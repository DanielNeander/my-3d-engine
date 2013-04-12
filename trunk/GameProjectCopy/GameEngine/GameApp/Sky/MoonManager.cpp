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
#include "MoonManager.h"

#include "VCloudSkySystem.h"
#include "GameApp/BillboardSet.h"
#include "GameApp/BaseCamera.h"
#include "GameApp/SceneNode.h"
#include "GameApp/GameApp.h"
#include "GameApp/Font/Dx11TextHelper.h"

namespace VCloud
{
	MoonManager::MoonManager(VCloudSkySystem *s)
		: mSkyX(s)
		, mMoonBillboard(0)
		, mMoonSceneNode(0)
		, mCreated(false)
		, mMoonSize(0.225f)
		, mMoonHaloIntensity(0.4f)
		, mMoonHaloStrength(0.9f)
		, mMoonMaterial(-1)
	{
	}

	MoonManager::~MoonManager()
	{
		remove();
	}

	void MoonManager::create()
	{
		if (mCreated)
		{
			return;
		}

		mMoonMaterial = GetApp()->getRenderer()->addShader("data/shaders/Moon.hlsl");

		
		mMoonSceneNode = SceneNode::NewNode();
		GetApp()->GetWorldRoot()->AddChild(mMoonSceneNode);

		mMoonBillboard = new BillboardSet(1);
        mMoonBillboard->mShader = mMoonMaterial;
		mMoonBillboard->setBillboardType(BBT_ORIENTED_COMMON);				

		mMoonBillboard->createBillboard(noVec3(0,0,0));
		mMoonSceneNode->AddChild(mMoonBillboard);

		mMoonTex = GetRenderer()->addTexture("Textures/Moon.png", true);
		mMoonHaloTex = GetRenderer()->addTexture("Textures/MoonHalo.png", true);

		mCreated = true;
	}

	void MoonManager::remove()
	{
		if (!mCreated)
		{
			return;
		}

		/*mMoonSceneNode->detachAllObjects();
		mMoonSceneNode->getParentSceneNode()->removeAndDestroyChild(mMoonSceneNode->getName());
		mMoonSceneNode = 0;*/
		//mSkyX->getSceneManager()->destroyBillboardSet(mMoonBillboard);
		//mMoonBillboard = 0;		
		mCreated = false;
	}

	void MoonManager::updateMoonPhase(const float& phase)
	{
		float center = 0, radius = 0, radius_add = 0, interpolation = 0, halo_flip = 0;

		noVec3 halo1, halo2;

		// [-1, 0]
		if (phase < 0)
		{
			// [-1, -0.5]
			if (phase < -0.5)
			{
				center = (1+phase)/2;
				radius = 0.25;

				interpolation = center*4;

				if (interpolation < 1.0f/3)
				{
					interpolation /= 1.0f/3;
					halo1 = noVec3(0.25, 0.5, (1-interpolation)*interpolation);
					halo2 = noVec3(0.25, 0.5, interpolation);
				}
				else if (interpolation < 2.0f/3)
				{
					interpolation -= 1.0f/3;
					interpolation /= 1.0f/3;
					halo1 = noVec3(0.25, 0.5, 1-interpolation);
					halo2 = noVec3(0.0, 0.5, interpolation);
				}
				else
				{
					interpolation -= 2.0f/3;
					interpolation /= 1.0f/3;
					halo1 = noVec3(0.0, 0.5, 1-interpolation);
					halo2 = noVec3(0.75, 0.0, interpolation);
				}

				radius_add =  0.1*center/(0.25001-center);

				radius += radius_add;
				center += radius_add;
			}
			// [-0.5, 0]
			else
			{
				center = (-phase)/2;
				radius = 0.25;

				interpolation = 1-center*4;

				if (interpolation < 1.0f/3)
				{
					interpolation /= 1.0f/3;
					halo1 = noVec3(0.75, 0.0, 1-interpolation);
					halo2 = noVec3(0.5, 0.0, interpolation);
				}
				else if (interpolation < 2.0f/3)
				{
					interpolation -= 1.0f/3;
					interpolation /= 1.0f/3;
					halo1 = noVec3(0.5, 0.0, 1-interpolation);
					halo2 = noVec3(0.25, 0.0, interpolation);
				}
				else
				{
					interpolation -= 2.0f/3;
					interpolation /= 1.0f/3;
					halo1 = noVec3(0.25, 0.0, 1-interpolation);
					halo2 = noVec3(0.00, 0.0, interpolation);
				}

				radius_add =  0.1*center/(0.25001-center);

				radius += radius_add;
				center += radius_add;

				radius = -radius;
				center = -center;
			}
		}
		// [0, 1]
		else
		{
			halo_flip = 1;

			// [0, 0.5]
			if (phase < 0.5)
			{
				center = phase/2;
				radius = 0.25;

				interpolation = center*4;

				if (interpolation < 1.0f/3)
				{
					interpolation /= 1.0f/3;
					halo1 = noVec3(0.00, 0.0, 1-interpolation);
					halo2 = noVec3(0.25, 0.0, interpolation);
				}
				else if (interpolation < 2.0f/3)
				{
					interpolation -= 1.0f/3;
					interpolation /= 1.0f/3;
					halo1 = noVec3(0.25, 0.0, 1-interpolation);
					halo2 = noVec3(0.5, 0.0, interpolation);
				}
				else
				{
					interpolation -= 2.0f/3;
					interpolation /= 1.0f/3;
					halo1 = noVec3(0.5, 0.0, 1-interpolation);
					halo2 = noVec3(0.75, 0.0, interpolation);
				}

				radius_add =  0.1*center/(0.25001-center);

				radius += radius_add;
				center += radius_add;

				radius = -radius;
				center = center;
			}
			// [0.5, 1]
			else
			{
				center = (1-phase)/2;
				radius = 0.25;

				interpolation = 1-center*4;

				if (interpolation < 1.0f/3)
				{
					interpolation /= 1.0f/3;
					halo1 = noVec3(0.75, 0.0, 1-interpolation);
					halo2 = noVec3(0.0, 0.5, interpolation);
				}
				else if (interpolation < 2.0f/3)
				{
					interpolation -= 1.0f/3;
					interpolation /= 1.0f/3;
					halo1 = noVec3(0.0, 0.5, 1-interpolation);
					halo2 = noVec3(0.25, 0.5, interpolation);
				}
				else
				{
					interpolation -= 2.0f/3;
					interpolation /= 1.0f/3;
					halo1 = noVec3(0.25, 0.5, 1-interpolation);
					halo2 = noVec3(0.25, 0.5, (1-interpolation)*interpolation);
				}

				radius_add =  0.1*center/(0.25001-center);

				radius += radius_add;
				center += radius_add;

				center = -center;
			}
		}
				
		GetRenderer()->setShader(mMoonMaterial);
		GetRenderer()->setShaderConstant3f("uMoonPhase", vec3(radius, center + 0.5f, mMoonHaloStrength));		

		halo1.z *= mMoonHaloIntensity;
		halo2.z *= mMoonHaloIntensity;

		GetRenderer()->setShaderConstant3f("uMoonHalo1", halo1.ToFloatPtr());
		GetRenderer()->setShaderConstant3f("uMoonHalo2", halo2.ToFloatPtr());
		GetRenderer()->setShaderConstant1f("uMoonHaloFlip", halo_flip);		
	}

	void MoonManager::updateGeometry(BaseCamera* c)
	{
		if (!mCreated)
		{
			return;
		}

		float radius = mSkyX->getMeshManager()->getSkydomeRadius(c)*0.95f,
              size = radius*mMoonSize;

		mMoonBillboard->setCommonDirection((mSkyX->getController()->getMoonDirection()).NormalizeCopy().Perpendicular());

		float rad = DEG2RAD((size/2)/radius);
		noVec3 moonRelativePos = mSkyX->getController()->getMoonDirection()*
			noMath::Cos(noMath::ASin(rad))*radius;

		mMoonSceneNode->SetTrans(c->GetFrom() + moonRelativePos);

		if (moonRelativePos.y < -size/2)
		{
			mMoonSceneNode->setVisible(false);
		}
		else
		{
			mMoonSceneNode->setVisible(mSkyX->isVisible());
						
			GetRenderer()->setShader(mMoonMaterial);
			GetRenderer()->setShaderConstant3f("uSkydomeCenter", c->GetFrom().ToFloatPtr());
		}

		if (mMoonBillboard->getBoundingBox().m_vMax.x != size)
		{
			_updateMoonBounds(c);
		}
	}

	void MoonManager::_updateMoonBounds(BaseCamera* c)
	{
		float radius = mSkyX->getMeshManager()->getSkydomeRadius(c)*0.95f,
              size = radius*mMoonSize;

		mMoonBillboard->setDefaultDimensions(size, size);
		mMoonBillboard->setBounds(BoundingBox(noVec3(-size/2, -size/2, -size/2),
														noVec3(size/2,  size/2,  size/2)), 1);
		//mMoonSceneNode->_updateBounds();
	}

	void MoonManager::Render(BaseCamera* c)
	{
		mMoonBillboard->UpdateBillboards();
		
		//if (mMoonBillboard->IsVisible())
		{	
			Dx11TextHelper::GetSingleton().Begin();
			Dx11TextHelper::GetSingleton().SetInsertionPos(50, 100);
			Dx11TextHelper::GetSingleton().SetForegroundColor(D3DXCOLOR( 1.0f, 0.0f, 1.0f, 1.0f ) );	
			Dx11TextHelper::GetSingleton().DrawFormattedTextLine("Moon Pos : %f, %f, %f, Moon is %s", mMoonSceneNode->WorldTrans().x, mMoonSceneNode->WorldTrans().y, mMoonSceneNode->WorldTrans().z,
				mMoonBillboard->IsVisible() ? "TRUE" : "FALSE");
			Dx11TextHelper::GetSingleton().End();

			GetRenderer()->reset();
			GetRenderer()->setShader(mMoonMaterial);
			GetRenderer()->setVertexFormat(mMoonBillboard->mVF);
			GetRenderer()->setVertexBuffer(0, mMoonBillboard->mVB);
			GetRenderer()->setIndexBuffer(mMoonBillboard->mIB);
			GetRenderer()->setTexture("MoonTex", mMoonTex);
			GetRenderer()->setSamplerState("uMoon", GetApp()->linearClamp);
			GetRenderer()->setSamplerState("uMoonHalo", GetApp()->linearClamp);
			GetRenderer()->setTexture("MoonHaloTex", mMoonHaloTex);
			mat4 view =ViewMat();
			view.rows[3].x = view.rows[3].y = view.rows[3].z = 0.f;
			mat4 wvp =  view * ProjMat();
			GetRenderer()->setShaderConstant4x4f("uWorldViewProj", transpose(wvp));
			GetRenderer()->apply();
			GetRenderer()->drawElements(PRIM_TRIANGLES, 0, mMoonBillboard->mNumIndices , 0, 0);
		}
	}
}