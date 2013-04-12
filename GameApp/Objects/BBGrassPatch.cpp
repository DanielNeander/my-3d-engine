// Copyright ?2008 Intel Corporation
// All Rights Reserved
// 
// The sample source code contained or described herein and all documents
// related to the sample source code ("Material") are owned by Intel Corporation
// or its suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its suppliers
// and licensors, and is protected by worldwide copyright and trade secret laws and
// treaty provisions. The sample source code is provided AS IS, with no warranties
// of any kind, express or implied. Except as expressly permitted by the Software
// license, neither Intel Corporation nor its suppliers assumes any responsibility
// or liability for any errors or inaccuracies that may appear herein.


#include "stdafx.h"
#include "BBGrassPatch.h"
#include "GameApp/Utility.h"
#include "GameApp/Terrain/AdaptiveModelDX11Render.h"
#include "GameApp/GameApp.h"

BOOL SphereConeTest ( D3DXVECTOR3 sphereCenter, float radius, float fov, D3DXVECTOR3 eyePt, D3DXVECTOR3 lookAt);

static const D3D11_INPUT_ELEMENT_DESC grassLayout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA,   0 },
	{ "vPPos",    0, DXGI_FORMAT_R32G32B32_FLOAT,	  1, 0,	 D3D11_INPUT_PER_INSTANCE_DATA, 1 },

};

static const UINT numGrassLayoutElements = sizeof( grassLayout ) / sizeof( grassLayout[0] );


BBGrassPatch::BBGrassPatch( int numBB, int inPatchCount, D3DXVECTOR3* inPatchPos )
{

	m_numGrassBB = numBB;
	patchPos     = inPatchPos;
	patchCount	 = inPatchCount;
}
 
BBGrassPatch::~BBGrassPatch( )
{
	SAFE_DELETE_ARRAY ( patchPos );
	SAFE_DELETE_ARRAY ( visiblePatches );
	SAFE_DELETE_ARRAY ( m_grassVerts );
}

HRESULT BBGrassPatch::CreateDevice( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, ID3DX11Effect* pEffect )
{
	HRESULT hr = S_OK;
	__super::Init(grassTexName, patchPos, patchCount, 
					m_grassVerts, 6*m_numGrassBB,(D3D11_INPUT_ELEMENT_DESC*)grassLayout, numGrassLayoutElements );
	ID3DX11EffectTechnique*    pRenderTechnique;
	pRenderTechnique = pEffect->GetTechniqueByName( "RenderGrass" );
	hr = __super::CreateDevice( pd3dDevice , pd3dContext, pRenderTechnique );

//__super::SetInstData( patchPos, patchCount );
	return hr;

}


void BBGrassPatch::DestroyDevice()
{
	InstancedBillboard::DestroyDevice();

}


void BBGrassPatch::GeneratePatch( unsigned int bladeCnt, float scale )
{
	m_numGrassBB = bladeCnt;
	m_grassVerts = new BBGrassVertex[m_numGrassBB*6];

	D3DXMATRIX mat;
	D3DXMATRIX matRandom;
	D3DXMatrixIdentity( &mat);

	for( int j=0; j< m_numGrassBB; j++ )
	{
		// Generate each quad at random position, orientation, height
		D3DXMatrixIdentity( &matRandom);
	//	float scale = 10.0f;
		float angle = ((float)rand()/RAND_MAX - 0.5f)*2 * noMath::PI; // angle = [-pi,pi]
		float dx = ((float)rand()/RAND_MAX - 0.5f)*2 * scale;
		float dz = ((float)rand()/RAND_MAX - 0.5f)*2 * scale;
		float heightScale = ((float)rand()/RAND_MAX - 0.5f) / 2.0f + 1.0f;

		D3DXMatrixRotationY( &mat, angle);
		D3DXMatrixMultiply( &matRandom, &matRandom, &mat );				
		D3DXMatrixTranslation( &mat, dx, 0.0f, dz);			
		D3DXMatrixMultiply( &matRandom, &matRandom, &mat );
		D3DXMatrixScaling( &mat, 1.0f, 1.0f, heightScale);			
		D3DXMatrixMultiply( &matRandom, &matRandom, &mat );

		// Apply the transformation to each vertex of the quad
		for( DWORD i=0; i< 6; i++ )
		{
			D3DXVECTOR4 pos, outPos;
			pos.x = grassBBVerts[i].Pos.x;
			pos.y = grassBBVerts[i].Pos.y;
			pos.z = grassBBVerts[i].Pos.z;
			pos.w = 1.0;
			D3DXVec3Transform(&outPos, (D3DXVECTOR3 *)&pos, (D3DXMATRIX *)&matRandom);
			int index = j * 6 + i;
			m_grassVerts[index].Pos.x = outPos.x;
			m_grassVerts[index].Pos.y = outPos.y;
			m_grassVerts[index].Pos.z = outPos.z;
			m_grassVerts[index].Tex = grassBBVerts[i].Tex;
		}
	}

}

void BBGrassPatch::Cull( ID3D11DeviceContext* pd3dContext, float fov, D3DXVECTOR3 eyePt, D3DXVECTOR3 lookAt )
{
	int numVisibleInstances = 0;
	D3DXVECTOR3 patchCenter3D;
	BOOL visible;
	for( unsigned int i = 0 ; i < patchCount; i++)
	{	
		patchCenter3D.x = patchPos[i].x; 	
		patchCenter3D.z = patchPos[i].z; 	
		patchCenter3D.y = patchPos[i].y; // we are only storing x,z positions for the patches
		// we want to cull the patch with a sphere of radius = to diagonal of square patch 
		// hence radiusPatchBoundingSphere = 1.414*rpatch
		visible = SphereConeTest ( patchCenter3D, patchRadius*1.414f, fov, eyePt, lookAt);
		if( visible )
		{
			// visible add to draw list 
			visiblePatches[numVisibleInstances] = patchPos[i];
			numVisibleInstances++;
		}
	}

	SetInstData( pd3dContext, visiblePatches, numVisibleInstances );
}


void BBGrassPatch::Generate( D3DXVECTOR3 surfaceCenter, float surfaceR, unsigned int patchBladeCnt, float inPatchRadius )
{
	patchRadius     = inPatchRadius;
	GeneratePatch( patchBladeCnt, patchRadius );

	patchCountX		= int( surfaceR/patchRadius );
	patchCountZ		= int( surfaceR/patchRadius );
	patchCount		= (2*patchCountX-1)*(2*patchCountZ-1);
	patchPos		= new D3DXVECTOR3[patchCount];
	visiblePatches  = new D3DXVECTOR3[patchCount];
	int k = 0;
	for(int i = -(patchCountX-1); i < patchCountX; i++)
	{
		for(int j = -(patchCountZ-1); j < patchCountZ; j++)
		{
			patchPos[k].x = surfaceCenter.x + i*patchRadius;			
			patchPos[k].z = surfaceCenter.z + j*patchRadius;

			extern CAdaptiveModelDX11Render g_TerrainDX11Render;		
			float height = 0.0f;
			if (g_TerrainDX11Render.IsInit()) {
				float DistToSurface = 0.f;
				D3DXVECTOR3 orig(patchPos[k].x, 1000.0f, patchPos[k].z), dir(0.0f, -1.0f, 0.0f);
				g_TerrainDX11Render.RayCast(orig, dir, &DistToSurface);	
				D3DXVECTOR3 temp  = orig + dir * DistToSurface;
				height = GetApp()->Pick().y + 5.0f;
				height = g_TerrainDX11Render.GetCurrPatch()->GetHeight(noVec2(patchPos[k].x, patchPos[k].z));
				height += (5.f + 0.5 * (10.f - 5.f));
			}
			patchPos[k].y = height;
			k++;
		}

	}
}