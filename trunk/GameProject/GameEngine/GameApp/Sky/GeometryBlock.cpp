DEFAULT_FOV
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
#include "EngineCore/Math/BoundingBox.h"
#include "EngineCore/Math/IntersectionTests.h"
#include "EngineCore/Math/Frustum.h"
#include "GeometryBlock.h"
#include "VClouds.h"
#include "VCloudSkySystem.h"

#include "GameApp/GameApp.h"

namespace VCloud { 

	GeometryBlock::GeometryBlock(VClouds* vc,
		    const float& Height, const float& Alpha, const float& Beta, 
			const float& Radius, const float& Phi, const int& Na, 
			const int& Nb, const int& Nc, const int& A, 
			const int& B, const int& C, const int& Position)
		: mVClouds(vc)
		, mCreated(false)		
		, mVertices(0)
		, mNumberOfTriangles(0)
		, mVertexCount(0)
		, mHeight(Height)
		, mAlpha(Alpha)
		, mBeta(Beta)
		, mRadius(Radius)
		, mPhi(Phi)
		, mNa(Na) , mNb(Nb) , mNc(Nc)
		, mA(A) , mB(B) , mC(C)
		, mPosition(Position)
		, mDisplacement(noVec3(0,0,0))
		, mWorldOffset(noVec2(0,0))
		, mCamera(0)
		, mDistance(noVec3(0,0,0))
		, mLastFallingDistance(0)
	{
		_calculateDataSize();
	}

	GeometryBlock::~GeometryBlock()
	{
		remove();
	}

	void GeometryBlock::create()
	{
		remove();

		// Create mesh and submesh		
		// Create mesh geometry
		mShader = mVClouds->mVolCloudsMaterial;
		_createGeometry();
			
        //mEntity->setMaterialName("SkyX_VolClouds");
		
		// Set bounds
		//mMesh->_setBounds(_buildAABox(mLastFallingDistance));

		mCreated = true;
	}

	void GeometryBlock::remove()
	{
		if (!mCreated)
		{
			return;
		}
				
		delete [] mVertices;

		mCreated = false;
	}

	const BoundingBox GeometryBlock::_buildAABox(const float& fd) const
	{
		noVec2 Center = noVec2(0,0);
		noVec2 V1     = mRadius*noVec2(noMath::Cos(mPhi*mPosition), noMath::Sin(mPhi*mPosition));
		noVec2 V2     = mRadius*noVec2(noMath::Cos(mPhi*(mPosition+1)), noMath::Sin(mPhi*(mPosition+1)));

		noVec2 Max    = noVec2(max<float>(max<float>(V1.x, V2.x), Center.x), max<float>(max<float>(V1.y, V2.y), Center.y) );
		noVec2 Min    = noVec2(min<float>(min<float>(V1.x, V2.x), Center.x), min<float>(min<float>(V1.y, V2.y), Center.y) );

		return BoundingBox(
							  // Min x,y,z
							noVec3( Min.x, -max<float>(fd,0),          Min.y),
							  // Max x,y,z
			                  noVec3(Max.x, mHeight - min<float>(fd,0), Max.y));
	}

	void GeometryBlock::_calculateDataSize()
	{
		mVertexCount = 7*mNa + 6*mNb + 4*mNc;
		mNumberOfTriangles = 5*mNa + 4*mNb + 2*mNc;

		mV2Cos = noVec2(noMath::Cos(mPosition*mPhi), noMath::Cos((mPosition+1)*mPhi));
		mV2Sin = noVec2(noMath::Sin(mPosition*mPhi), noMath::Sin((mPosition+1)*mPhi));

		mBetaSin  = noMath::Sin(noMath::PI-mBeta);
		mAlphaSin = noMath::Sin(noMath::PI-mAlpha);
	}

	void GeometryBlock::_createGeometry()
	{
		// Vertex buffers
		Renderer* renderer = GetApp()->getRenderer();

		FormatDesc vbFmt[] = {
			0, TYPE_VERTEX, FORMAT_FLOAT,  3,	// pos
			0, TYPE_TEXCOORD, FORMAT_FLOAT, 3,	// 3d tex
			0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,	// noise
			0, TYPE_TEXCOORD, FORMAT_FLOAT, 1,	// opacity			
		};
				
		mVF = renderer->addVertexFormat(vbFmt, elementsOf(vbFmt), mShader);

		// Create our internal buffer for manipulations
		mVertices = new VERTEX[mVertexCount];
		memset(mVertices, 0, sizeof(VERTEX) * mVertexCount);

		mVB = renderer->addVertexBuffer(sizeof(VERTEX) * mVertexCount, DYNAMIC, mVertices);
		
		unsigned short *indexbuffer = new unsigned short[mNumberOfTriangles*3];

		int IndexOffset = 0;
		int VertexOffset = 0;

		// C
		for (int k = 0; k < mNc; k++)
		{
			// First triangle
			indexbuffer[IndexOffset]   = VertexOffset;
			indexbuffer[IndexOffset+1] = VertexOffset+1;
			indexbuffer[IndexOffset+2] = VertexOffset+3;

			// Second triangle
			indexbuffer[IndexOffset+3] = VertexOffset;
			indexbuffer[IndexOffset+4] = VertexOffset+3;
			indexbuffer[IndexOffset+5] = VertexOffset+2;

			IndexOffset  += 6;
			VertexOffset += 4;
		}

		// B
		for (int k = 0; k < mNb; k++)
		{
			// First triangle
			indexbuffer[IndexOffset]   = VertexOffset;
			indexbuffer[IndexOffset+1] = VertexOffset+1;
			indexbuffer[IndexOffset+2] = VertexOffset+3;

			// Second triangle
			indexbuffer[IndexOffset+3] = VertexOffset;
			indexbuffer[IndexOffset+4] = VertexOffset+3;
			indexbuffer[IndexOffset+5] = VertexOffset+2;

			// Third triangle
			indexbuffer[IndexOffset+6] = VertexOffset+2;
			indexbuffer[IndexOffset+7] = VertexOffset+3;
			indexbuffer[IndexOffset+8] = VertexOffset+5;

			// Fourth triangle
			indexbuffer[IndexOffset+9] = VertexOffset+2;
			indexbuffer[IndexOffset+10] = VertexOffset+5;
			indexbuffer[IndexOffset+11] = VertexOffset+4;

			IndexOffset  += 12;
			VertexOffset += 6;
		}

		// A
		for (int k = 0; k < mNa; k++)
		{
			// First triangle
			indexbuffer[IndexOffset]   = VertexOffset;
			indexbuffer[IndexOffset+1] = VertexOffset+1;
			indexbuffer[IndexOffset+2] = VertexOffset+3;

			// Second triangle
			indexbuffer[IndexOffset+3] = VertexOffset;
			indexbuffer[IndexOffset+4] = VertexOffset+3;
			indexbuffer[IndexOffset+5] = VertexOffset+2;

			// Third triangle
			indexbuffer[IndexOffset+6]   = VertexOffset+2;
			indexbuffer[IndexOffset+7] = VertexOffset+3;
			indexbuffer[IndexOffset+8] = VertexOffset+5;

			// Fourth triangle
			indexbuffer[IndexOffset+9] = VertexOffset+2;
			indexbuffer[IndexOffset+10] = VertexOffset+5;
			indexbuffer[IndexOffset+11] = VertexOffset+4;

			// Fifth triangle
			indexbuffer[IndexOffset+12] = VertexOffset+4;
			indexbuffer[IndexOffset+13] = VertexOffset+5;
			indexbuffer[IndexOffset+14] = VertexOffset+6;

			IndexOffset  += 15;
			VertexOffset += 7;
		}
				
		// Prepare buffer for indices
		mIB = renderer->addIndexBuffer(mNumberOfTriangles*3, sizeof(WORD), 
			STATIC, indexbuffer);

		delete []indexbuffer;	 
	}

	void GeometryBlock::updateGeometry(BaseCamera* c, const noVec3& displacement, const noVec3& distance)
	{
		if (!mCreated)
		{
			return;
		}

		mDisplacement = displacement;
		noVec3 worldParentPos = parent_->WorldTrans();

		float fallingDistance = mVClouds->getDistanceFallingParams().x *
			(worldParentPos.y - c->GetFrom().y);

		if (mVClouds->getDistanceFallingParams().y > 0) // -1 means no max falling
		{
			if (fallingDistance > 0)
			{
				if (fallingDistance > mVClouds->getDistanceFallingParams().y)
				{
					fallingDistance = mVClouds->getDistanceFallingParams().y;
				}
			}
			else
			{
				if (-fallingDistance > mVClouds->getDistanceFallingParams().y)
				{
					fallingDistance = -mVClouds->getDistanceFallingParams().y;
				}
			}
		}

		if (fallingDistance != mLastFallingDistance)
		{
			mLastFallingDistance = fallingDistance;
			//mMesh->_setBounds();
			m_AABB = _buildAABox(mLastFallingDistance);
		}

		if (isInFrustum(c))
		{
			mCamera = c;
			setVisible(true);
			mDistance = distance;
			_updateGeometry();
		}
		else 
			setVisible(false);
	}

	void GeometryBlock::_updateGeometry()
	{
		// Update zone C
		for (int k = 0; k < mNc; k++)
		{
			_updateZoneCSlice(k);
		}

		// Update zone B
		for (int k = 0; k < mNb; k++)
		{
			_updateZoneBSlice(k);
		}

		// Update zone A
		for (int k = 0; k < mNa; k++)
		{
			_updateZoneASlice(k);
		}

		// Upload changes
		
		Direct3D11Renderer* Dx11R = (Direct3D11Renderer*)GetRenderer();
		Dx11R->CopyToVB(mVB, mVertices, sizeof(VERTEX) * mVertexCount);
	}

	void GeometryBlock::_updateZoneCSlice(const int& n)
	{
		int VertexOffset = n*4;

		// TODO, calculate constants by zone, not by slice
		float Radius = mB+((mC-mB)/mNc)*(mNc-n);

		Radius += mDisplacement.z;

		float opacity = 1;

		if (n == 0)
		{
			opacity = 1 - mDisplacement.z/((mC-mB)/mNc);
		}
		else if (n == mNc-1)
		{
			opacity = mDisplacement.z/((mC-mB)/mNc);
		}

		noVec2 x1 = Radius*mV2Cos,
				      x2 = Radius*mBetaSin*mV2Cos,
				      z1 = Radius*mV2Sin,
					  z2 = Radius*mBetaSin*mV2Sin;
	    
		noVec3 or0 = noVec3(x1.x, 0, z1.x),
			          or1 = noVec3(x1.y, 0, z1.y);

		float y0 = Radius*noMath::Sin(mAlpha),
		       d = noVec2(x1.x - x2.x, z1.x - z2.x).Length(),
			 ang = noMath::ATan(y0/d),
			 hip = mHeight / noMath::Sin(ang);

		// Vertex 0
		_setVertexData(VertexOffset, or0, opacity);
		// Vertex 1
		_setVertexData(VertexOffset+1, or1, opacity);
		// Vertex 2
		_setVertexData(VertexOffset+2, or0+(noVec3(x2.x, y0, z2.x)-or0).NormalizeCopy()*hip, opacity);
		// Vertex 3
		_setVertexData(VertexOffset+3, or1+(noVec3(x2.y, y0, z2.y)-or1).NormalizeCopy()*hip, opacity);
	}

	void GeometryBlock::_updateZoneBSlice(const int& n)
	{
		int VertexOffset = mNc*4 + n*6;

		// TODO
		float Radius = mA+((mB-mA)/mNb)*(mNb-n);

		Radius += mDisplacement.y;

		float opacity = 1;

		if (n == 0)
		{
			opacity = 1-mDisplacement.y/((mB-mA)/mNb);
		}
		else if (n == mNb-1)
		{
			opacity = mDisplacement.y/((mB-mA)/mNb);
		}

		noVec2 x1 = Radius*mV2Cos,
				      x2 = Radius*mBetaSin*mV2Cos,
				      z1 = Radius*mV2Sin,
					  z2 = Radius*mBetaSin*mV2Sin;
	    
		float y0 = Radius*noMath::Sin(mAlpha);

		// Vertex 0
		_setVertexData(VertexOffset, noVec3(x1.x, 0, z1.x), opacity);
		// Vertex 1
		_setVertexData(VertexOffset+1, noVec3(x1.y, 0, z1.y), opacity);
		// Vertex 2
		_setVertexData(VertexOffset+2, noVec3(x2.x, y0, z2.x), opacity);
		// Vertex 3
		_setVertexData(VertexOffset+3, noVec3(x2.y, y0, z2.y), opacity);

		noVec2 x3 = Radius*mAlphaSin*mV2Cos,
					  z3 = Radius*mAlphaSin*mV2Sin;

		noVec3 or0 = noVec3(x2.x, y0, z2.x),
			          or1 = noVec3(x2.y, y0, z2.y);

		float y1 = Radius*noMath::Sin(mBeta),
			  y3 = y1-y0,
		       d = noVec2(x3.x - x2.x, z3.x - z2.x).Length(),
			 ang = noMath::ATan(y3/d),
			 hip = (mHeight-y0) / noMath::Sin(ang);

		// Vertex 4
		_setVertexData(VertexOffset+4, or0 + (noVec3(x3.x, y1, z3.x)-or0).NormalizeCopy()*hip, opacity);
		// Vertex 5
		_setVertexData(VertexOffset+5, or1 + (noVec3(x3.y, y1, z3.y)-or1).NormalizeCopy()*hip, opacity);
	}

	void GeometryBlock::_updateZoneASlice(const int& n)
	{
		int VertexOffset = mNc*4 + mNb*6 +n*7;

		// TODO
		float Radius = (mA/mNa)*(mNa-n);

		Radius += mDisplacement.x;

		float opacity = (n == 0) ? (1-mDisplacement.x/(mA/mNa)) : 1.0f;

		noVec2 x1 = Radius*mV2Cos,
				      x2 = Radius*mBetaSin*mV2Cos,
				      z1 = Radius*mV2Sin,
					  z2 = Radius*mBetaSin*mV2Sin;
	    
		float y0 = Radius*noMath::Sin(mAlpha);

		// Vertex 0
		_setVertexData(VertexOffset, noVec3(x1.x, 0, z1.x), opacity);
		// Vertex 1
		_setVertexData(VertexOffset+1, noVec3(x1.y, 0, z1.y), opacity);
		// Vertex 2
		_setVertexData(VertexOffset+2, noVec3(x2.x, y0, z2.x), opacity);
		// Vertex 3
		_setVertexData(VertexOffset+3, noVec3(x2.y, y0, z2.y), opacity);

		noVec2 x3 = Radius*mAlphaSin*mV2Cos,
					  z3 = Radius*mAlphaSin*mV2Sin;

		float y1 = Radius*noMath::Sin(mBeta);

		// Vertex 4
		_setVertexData(VertexOffset+4, noVec3(x3.x, y1, z3.x), opacity);
		// Vertex 5
		_setVertexData(VertexOffset+5, noVec3(x3.y, y1, z3.y), opacity);

		// Vertex 6
		_setVertexData(VertexOffset+6, noVec3(0, Radius, 0), opacity);
	}

	void GeometryBlock::_setVertexData(const int& index, const noVec3& p, const float& o)
	{
		float yDist = -mDistance.y;
		float fallingDistance = mVClouds->getDistanceFallingParams().x*yDist*(noVec2(p.x,p.z).Length()/mRadius);
		
		if (mVClouds->getDistanceFallingParams().y > 0) // -1 means no max falling
		{
			if (fallingDistance > 0)
			{
				if (fallingDistance > mVClouds->getDistanceFallingParams().y)
				{
					fallingDistance = mVClouds->getDistanceFallingParams().y;
				}
			}
			else
			{
				if (-fallingDistance > mVClouds->getDistanceFallingParams().y)
				{
					fallingDistance = -mVClouds->getDistanceFallingParams().y;
				}
			}
		}

		fallingDistance *= noMath::Sign(yDist);
		
		// Position
		mVertices[index].x = p.x;
		if (yDist < -mHeight/2)
		{
			// Over-cloud
			mVertices[index].y = mHeight - p.y;
		}
		else // Under-cloud
		{
			mVertices[index].y = p.y - fallingDistance;
		}
		mVertices[index].z = p.z;

		// 3D coords (Z-UP)
		float scale = mVClouds->getCloudFieldScale()/mRadius;
		mVertices[index].xc = (p.x+mWorldOffset.x)*scale;
		mVertices[index].yc = (p.z+mWorldOffset.y)*scale;
		mVertices[index].zc = noMath::ClampFloat(0, 1, p.y/mHeight);
		if (yDist < -mHeight/2)
		{
			// Over-cloud
			mVertices[index].zc = 1.0f - mVertices[index].zc;
		}

		// Noise coords
		noVec3 worldParentPos = parent_->WorldTrans();
		float noise_scale = mVClouds->getNoiseScale()/mRadius;
		float xz_length_radius = noVec2(p.x,p.z).Length() / mRadius;
		noVec3 origin = noVec3(0,-(worldParentPos.y - mCamera->GetFrom().y) -mRadius*(0.5f+0.5f*noVec2(p.x,p.z).Length()/mRadius),0);
		noVec3 dir = (p-origin).NormalizeCopy();
		float hip = noMath::Sqrt(noMath::Pow(xz_length_radius * mRadius, 2) + noMath::Pow(origin.y, 2));
		noVec3 uv = dir*hip; // Only x/z, += origin doesn't need
		mVertices[index].u = (uv.x+mWorldOffset.x)*noise_scale;
		mVertices[index].v = (uv.z+mWorldOffset.y)*noise_scale;

		// Opacity
		float dist = (mDistance - noVec3(p.x, p.y - fallingDistance, p.z)).Length();
		float att = noMath::ClampFloat(0.0f,1.0f, (dist-mA/3.25f)/(mA/3.25f));
		mVertices[index].o = o * att * mVClouds->getGlobalOpacity();
	}

	const bool GeometryBlock::isInFrustum(BaseCamera *c) const
	{
		if (!mCreated)
		{
			return false;
		}

		// TODO: Use a world bounding box for each geometry zone, this way the
		// culling is going to be more acurrated and the geometry falling is going to be culled
		// when the falling factor is bigger than 1.
		//return c->isVisible(mEntity->getParentSceneNode()->_getWorldAABB());
		Frustum cameraFrustum = c->CalculateFrustum(c->GetNear(), c->GetFar());
		cameraFrustum.CalculateAABB();

		if (IntersectionTest(m_AABB, cameraFrustum.m_AABB))
			return true;		

		return false;
	}

	void GeometryBlock::Draw( void )
	{

	}

	void GeometryBlock::Render()
	{
		if (!IsVisible()) return;

		GetRenderer()->reset();
		GetRenderer()->setShader(mShader);
		GetRenderer()->setVertexFormat(mVF);
		GetRenderer()->setVertexBuffer(0, mVB);
		GetRenderer()->setIndexBuffer(mIB);
		GetRenderer()->setSamplerState("uDensity0", VCloudSkySystem::GetSingleton().linearWarp);
		GetRenderer()->setSamplerState("uDensity1", VCloudSkySystem::GetSingleton().linearWarp);
		GetRenderer()->setSamplerState("uNoise", VCloudSkySystem::GetSingleton().linearWarp);		
		GetRenderer()->setTexture("DensityTex0", DataManager::GetSingletonPtr()->mVolTextures[0]);
		GetRenderer()->setTexture("DensityTex1", DataManager::GetSingletonPtr()->mVolTextures[1]);
		GetRenderer()->setTexture("NoiseTex", DataManager::GetSingletonPtr()->mNoiseTex);
		GetRenderer()->setDepthState(GetApp()->noDepthWrite);
		GetRenderer()->setBlendState(GetApp()->GetBlendMode(5));
		mat4 view =ViewMat();
		
		noVec3 dist = mDistance;
		mat4 world = translate(dist.x, -dist.y, dist.z);
		view.rows[3].x = view.rows[3].y = view.rows[3].z = 0.f;
		mat4 wvp =  transpose(world) * view * ProjMat();
		
		noVec3 campos = mVClouds->getGeometryManager()->_getCurrentDistance();
		GetRenderer()->setShaderConstant3f("uCameraPosition", campos.ToFloatPtr());
		GetRenderer()->setShaderConstant4x4f("uWorldViewProj", transpose(wvp));
		GetRenderer()->apply();
		GetRenderer()->drawElements(PRIM_TRIANGLES, 0, mNumberOfTriangles*3, 0, 0);
	}

}