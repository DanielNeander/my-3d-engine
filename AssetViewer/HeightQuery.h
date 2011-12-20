#pragma once 
//--------------------------------------------------------------------------------------
// File: HeightQuery.h
//
// Copyright (C) 2005 - Harald Vistnes (harald@vistnes.org)
// All rights reserved worldwide.
//
// This software is provided "as is" without express or implied
// warranties. You may freely copy and compile this source into
// applications you distribute provided that the copyright text
// below is included in the resulting source code, for example:
// "Portions Copyright (C) Harald Vistnes, 2005"
//--------------------------------------------------------------------------------------
#include <windows.h>	// InterlockedIncrement Call
#include <U2_3D/Src/main/U2Object.h>
#include <U2_3D/Src/main/U2Node.h>
#include <U2_3D/Src/main/U2Camera.h>
#include <U2_3D/Src/Dx9/U2RenderTarget.h>
#include <U2_3D/Src/dx9/U2Dx9Texture.h>
#include <U2_3D/Src/shader/U2D3DXEffectShader.h>

U2SmartPointer(U2Dx9Texture);
U2SmartPointer(U2N2Mesh);

class HeightQuery : public U2Object
{
public:
	HeightQuery();
	~HeightQuery();

	bool Initialize();
	void Terminate();

	bool OnReset();
	bool InitMesh();
	bool InitShader();
	bool InitRenderTarget();
	bool InitCamera();
	void Render();
	
	bool SetVertexData(float u, float v);
	bool QueryPoint(float u, float v, float *height);
	

private:
	U2Dx9TexturePtr m_spHeightField;
	U2Dx9TexturePtr m_spTex;
	U2N2MeshPtr m_spMesh;
	U2RenderTargetPtr m_spRT;
	IDirect3DSurface9*		m_pSysMemSurface;

	U2D3DXEffectShaderPtr		m_spHeightShader;
	U2CameraPtr	m_spScreenCam;

	D3DXMATRIX m_matView, m_matProj, m_matWorld;
	D3DXMATRIX m_matViewOrig, m_matProjOrig, m_matWorldOrig;
	



};