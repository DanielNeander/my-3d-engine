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
#include "SkydomeMesh.h"

#include "GameApp/BaseCamera.h"
#include "GameApp/GameApp.h"
#include "VCloudSkySystem.h"

namespace VCloud
{
	SkyDomeMesh::SkyDomeMesh(VCloudSkySystem *s)
		: mSkyX(s)
		, mCreated(false)		
		, mVertices(0)        
	    , mSceneNode(0)
		, mSteps(70)
		, mCircles(95)
		, mUnderHorizonCircles(12)
		, mUnderHorizonFading(true)
		, mUnderHorizonFadingExponent(0.75)
		, mUnderHorizonFadingMultiplier(2)
		, mRadiusMultiplier(0.95f)        
	{

	}

	SkyDomeMesh::~SkyDomeMesh()
	{
		remove();
	}

	void SkyDomeMesh::remove()
	{
		if (!mCreated)
		{
			return;
		}

		/*mSceneNode->detachAllObjects();
		mSceneNode->getParentSceneNode()->removeAndDestroyChild(mSceneNode->getName());*/
		mSceneNode = 0;

		/*MeshManager::getSingleton().remove("SkyXMesh");
		mSkyX->getSceneManager()->destroyEntity(mEntity);*/
						
		delete [] mVertices;

		mCreated = false;
	}

	void SkyDomeMesh::create()
	{
		if (mCreated)
		{
			return;
		}

		// Create sky mesh 
        
		// Create mesh geometry
		//mShader = GetApp()->getRenderer()->addShader("Data/Shaders/Skydome.hlsl");
		_createGeometry();
		mDepthWrite = GetRenderer()->addDepthState(false, false);
					
		
		mSceneNode = SceneNode::NewNode();		
        //mSceneNode->attachObject(mEntity);
		mCreated = true;
	}

	void SkyDomeMesh::updateGeometry(BaseCamera* cam)
	{
		if (!mCreated)
		{
			return;
		}

		float Radius = getSkydomeRadius(cam);

		mVertices[0].x = 0; mVertices[0].z = 0;	mVertices[0].y = Radius;
		mVertices[0].nx = 0; mVertices[0].nz = 0; mVertices[0].ny = 1; 
		mVertices[0].u = 4; mVertices[0].v = 4;
		mVertices[0].o = 1;

		float AngleStep = (noMath::PI/2) / (mCircles-mUnderHorizonCircles);

		float r, uvr, c, s, h;
		float currentPhiAngle, currentTethaAngle;
		int x, y;

		// Above-horizon
		for(y=0;y<mCircles-mUnderHorizonCircles;y++) 
		{
			currentTethaAngle = noMath::PI/2 - AngleStep*(y+1);

			r = noMath::Cos(currentTethaAngle);
			h = noMath::Sin(currentTethaAngle);

			uvr = static_cast<float>(y+1)/(mCircles-mUnderHorizonCircles);

			for(x=0;x<mSteps;x++) 
			{
				currentPhiAngle = noMath::TWO_PI * x / mSteps;

				c = noMath::Cos(currentPhiAngle) * r;
				s = noMath::Sin(currentPhiAngle) * r;

				mVertices[1+y*mSteps + x].x = c * Radius;
				mVertices[1+y*mSteps + x].z = s * Radius;
				mVertices[1+y*mSteps + x].y = h * Radius;

				mVertices[1+y*mSteps + x].nx = c;
				mVertices[1+y*mSteps + x].nz = s;
				mVertices[1+y*mSteps + x].ny = h;

				mVertices[1+y*mSteps + x].u = (1 + c*uvr/r)*4;
				mVertices[1+y*mSteps + x].v = (1 + s*uvr/r)*4;

				mVertices[1+y*mSteps + x].o = 1;
			}
		}

		float op; // Opacity

		// Under-horizon
		for(y=mCircles-mUnderHorizonCircles;y<mCircles;y++) 
		{
			currentTethaAngle = noMath::PI/2 - AngleStep*(y+1);

			r = noMath::Cos(currentTethaAngle);
			h = noMath::Sin(currentTethaAngle);

			uvr = static_cast<float>(y+1)/(mCircles-mUnderHorizonCircles);

			float pow_ = noMath::Pow(static_cast<float>(mCircles-y-1) / mUnderHorizonCircles, mUnderHorizonFadingExponent)*mUnderHorizonFadingMultiplier;
			op = noMath::ClampFloat(0, 1, pow_);

			for(x=0;x<mSteps;x++) 
			{
				currentPhiAngle = noMath::TWO_PI * x / mSteps;

				c = noMath::Cos(currentPhiAngle) * r;
				s = noMath::Sin(currentPhiAngle) * r;

				mVertices[1+y*mSteps + x].x = c * Radius;
				mVertices[1+y*mSteps + x].z = s * Radius;
				mVertices[1+y*mSteps + x].y = h * Radius;

				mVertices[1+y*mSteps + x].nx = c;
				mVertices[1+y*mSteps + x].nz = s;
				mVertices[1+y*mSteps + x].ny = h;

				mVertices[1+y*mSteps + x].u = (1 + c*uvr/r)*4;
				mVertices[1+y*mSteps + x].v = (1 + s*uvr/r)*4;

				mVertices[1+y*mSteps + x].o = op;
			}
		}


		Direct3D11Renderer* Dx11R = (Direct3D11Renderer*)GetRenderer();
		Dx11R->CopyToVB(mVB, mVertices, sizeof(VERTEX) * (1+mSteps * mCircles));
		// Update data
		
		//// Update bounds
	 //   AxisAlignedBox meshBounds =
		//	AxisAlignedBox(-Radius, 0,     -Radius,
		//	                      Radius, Radius, Radius);

		//mMesh->_setBounds(meshBounds);
		//mSceneNode->_updateBounds();
	}

	void SkyDomeMesh::_createGeometry()
	{
		int numVertices = mSteps * mCircles + 1;
		int numEle = 6 * mSteps * (mCircles-1) + 3 * mSteps;
		mNumIndices = numEle;
		mNumVertices = numVertices;

		Renderer* renderer = GetApp()->getRenderer();

		FormatDesc vbFmt[] = {
			0, TYPE_VERTEX, FORMAT_FLOAT,  3,	// pos
			0, TYPE_TEXCOORD, FORMAT_FLOAT, 3,	// 3d tex
			0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,	// noise
			0, TYPE_TEXCOORD, FORMAT_FLOAT, 1,	// opacity			
		};

		
		mVF = renderer->addVertexFormat(vbFmt, elementsOf(vbFmt), mShader);

		// Create our internal buffer for manipulations
		mVertices = new VERTEX[numVertices];
		memset(mVertices, 0, sizeof(VERTEX) * numVertices);
				
		mVB = renderer->addVertexBuffer(sizeof(VERTEX) * numVertices, DYNAMIC , mVertices);
			
	
		unsigned short *indexbuffer = new unsigned short[numEle];

		for (int k = 0; k < mSteps; k++)
		{
			indexbuffer[k*3] = 0;
			indexbuffer[k*3+1] = k+1;

			if (k != mSteps-1)
			{
			    indexbuffer[k*3+2] = k+2;
			}
			else
			{
				indexbuffer[k*3+2] = 1;
			}
		}

		unsigned short *twoface;

		for(int y=0; y<mCircles-1; y++) 
		{
		    for(int x=0; x<mSteps; x++) 
			{
			    twoface = indexbuffer + (y*mSteps+x)*6 + 3 * mSteps;

			    int p0 = 1+y * mSteps + x ;
			    int p1 = 1+y * mSteps + x + 1 ;
			    int p2 = 1+(y+1)* mSteps + x ;
			    int p3 = 1+(y+1)* mSteps + x + 1 ;

				if (x == mSteps-1)
				{
					p1 -= x+1;
					p3 -= x+1;
				}

				// First triangle
			    twoface[2]=p0;
			    twoface[1]=p1;
			    twoface[0]=p2;

				// Second triangle
			    twoface[5]=p1;
			    twoface[4]=p3;
			    twoface[3]=p2;
		    }
	    }

		// Prepare buffer for indices
		mIB = renderer->addIndexBuffer(numEle, sizeof(unsigned short), 
			STATIC, indexbuffer);
		
		//delete []indexbuffer;	
	}

	void SkyDomeMesh::setGeometryParameters(const int &Steps, const int &Circles)
	{
		mSteps = Steps;
		mCircles = Circles;

		if (mCreated)
		{
		    remove();
		    create();
		}
	}

	void SkyDomeMesh::setUnderHorizonParams(const int& UnderHorizonCircles, const bool& UnderHorizonFading, const float& UnderHorizonFadingExponent, const float& UnderHorizonFadingMultiplier)
	{
		bool needToRecreate = (mUnderHorizonCircles != UnderHorizonCircles);

		mUnderHorizonCircles = UnderHorizonCircles;
		mUnderHorizonFading = UnderHorizonFading;
		mUnderHorizonFadingExponent = UnderHorizonFadingExponent;
		mUnderHorizonFadingMultiplier = UnderHorizonFadingMultiplier;

		if (needToRecreate)
		{
		    remove();
		    create();
		}
	}

	void SkyDomeMesh::setMaterial(ShaderID shader)
	{
		mShader = shader;
	}

	const float SkyDomeMesh::getSkydomeRadius(BaseCamera* c) const
	{
		float cameraFarClipDistance = c->GetFar();

		if (!cameraFarClipDistance)
		{
			cameraFarClipDistance = mSkyX->getInfiniteCameraFarClipDistance();
		}

		return cameraFarClipDistance*mRadiusMultiplier;
	}

	void SkyDomeMesh::Render( BaseCamera* camera )
	{
		GetRenderer()->resetToDefaults();
		GetRenderer()->reset();
		GetRenderer()->setShader(mShader);
		GetRenderer()->setVertexFormat(mVF);		
		GetRenderer()->setVertexBuffer(0, mVB);
		GetRenderer()->setIndexBuffer(mIB);			
		GetRenderer()->setDepthState(mDepthWrite);
		GetRenderer()->setTexture("StarTex", mSkyX->mStarfieldTex);
		GetRenderer()->setBlendState(GetApp()->GetBlendMode(5));
		GetRenderer()->setSamplerState("uStarfield", mSkyX->linearWarp);
		mat4 view =ViewMat();
		view.rows[3].x = view.rows[3].y = view.rows[3].z = 0.f;
		mat4 wvp =  view * ProjMat();
		GetRenderer()->setShaderConstant4x4f("uWorldViewProj", transpose(wvp));
		GetRenderer()->apply();
		GetRenderer()->drawElements(PRIM_TRIANGLES, 0, mNumIndices, 0, 0);	
		
		
	}

}