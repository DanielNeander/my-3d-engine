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

#pragma once
#include <d3d11.h>
#include <d3dx11.h>
#define DEFAULT_NUM_GRASS_BB 70
#include "InstancedBillboard.h"

struct BBGrassVertex
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR2 Tex;
};

//  geometry for single grass blade 
static BBGrassVertex grassBBVerts[] =
{
	//					x     y      z					 tu1   tv1    
	{  D3DXVECTOR3( -10.0f,   0.0f,   0.0f),  D3DXVECTOR2( 1.0f,  0.01f )},
	{  D3DXVECTOR3(  10.0f,   0.0f,   0.0f),  D3DXVECTOR2( 0.01f, 0.01f )},
	{  D3DXVECTOR3(  10.0f,  -10.0f,  0.0f),  D3DXVECTOR2( 0.01f, 1.0f ) },

	{  D3DXVECTOR3(  -10.0f,   0.0f,  0.0f),  D3DXVECTOR2( 1.0f,  0.01f )},
	{  D3DXVECTOR3(   10.0f,  -10.0f, 0.0f),  D3DXVECTOR2( 0.01f, 1.0f ) },
	{  D3DXVECTOR3(  -10.0f,  -10.0f, 0.0f),  D3DXVECTOR2( 1.0f,  1.0f ) }, 
};

//  x,z instance positions ...
static D3DXVECTOR2 defaultPatchPos[] =
{
	//			   x      z					
	D3DXVECTOR2(  20.0f, 0.0f ) ,
	D3DXVECTOR2(  0.0f,  0.0f ) ,
	D3DXVECTOR2( -20.0f, 0.0f ) ,

	D3DXVECTOR2(  20.0f, -20.0f ) ,
	D3DXVECTOR2(  0.0f,  -20.0f  ) ,
	D3DXVECTOR2( -20.0f, -20.0f ) ,

	D3DXVECTOR2(  20.0f,  40.0f ) ,
	D3DXVECTOR2(  20.0f,  40.0f ) ,
	D3DXVECTOR2(  0.0f,   40.0f ) ,
	D3DXVECTOR2( -20.0f,  40.0f ),
	D3DXVECTOR2( -40.0,	  40.0f ),

	D3DXVECTOR2(  40.0f,  20.0f ) ,
	D3DXVECTOR2(  20.0f,  20.0f ) ,
	D3DXVECTOR2(  0.0f,   20.0f ) ,
	D3DXVECTOR2( -20.0f,  20.0f ) ,
	D3DXVECTOR2( -40.0f,  20.0f ) ,
};

const int defaultPatchCount = 16;

class BBGrassPatch : public InstancedBillboard <BBGrassVertex, D3DXVECTOR3> 
{
	int									m_numGrassBB;
	BBGrassVertex*						m_grassVerts;
	float								patchRadius;
	int									patchCountX;
	int									patchCountZ;
	unsigned int						patchCount;
	D3DXVECTOR3*						patchPos;
	D3DXVECTOR3*						visiblePatches;

	TCHAR								grassTexName[MAX_PATH];
	// Generates & Randomly Orients Blades in a patch
	void	GeneratePatch( unsigned int bladeCnt, float scale);

public:
	
	BBGrassPatch( int numBB = DEFAULT_NUM_GRASS_BB, int inPatchCount = defaultPatchCount, D3DXVECTOR3* inPatchPos = NULL );
	~BBGrassPatch( );
	HRESULT CreateDevice ( ID3D11Device* m_pd3dDevice, ID3D11DeviceContext* pd3dContext, ID3DX11Effect* m_pEffect );
	void Cull ( ID3D11DeviceContext* pd3dContext, float fov, D3DXVECTOR3 eyePt, D3DXVECTOR3 lookAt );
	void	DestroyDevice( );
	void	Generate	 ( D3DXVECTOR3 surfaceCenter, float surfaceR, unsigned int patchBladeCnt, float inPatchRadius );
		
	void	SetDiffuseTextureName(const TCHAR* name) { _tcscpy(grassTexName, name); }
	
};