#pragma once 
//--------------------------------------------------------------------------------------
// File: Terrain.h
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

class HeightQuery;

#include <windows.h>	// InterlockedIncrement Call


//#include <U2Misc/Src/U2Misc.h>

#include <U2Lib/Src/U2LibType.h>
#include <U2Lib/Src/U2FilePath.h>

#include <U2_3D/Src/U2_3DDefine.h>
#include <U2_3D/Src/main/U2Object.h>
#include <U2_3D/Src/main/U2Node.h>
#include <U2_3D/Src/main/U2CameraNode.h>
#include <U2_3D/Src/main/U2SceneMgr.h>
#include <U2_3D/Src/dx9/U2Dx9Texture.h>

#include <U2_3D/Src/shader/U2D3DXEffectShader.h>
#include <U2_3D/src/effect/U2Dx9FxShaderEffect.h>
//#include <U2_3D/Src/dx9/U2N2Mesh.h>

#define MAX_ELEVATION 200

class HeightQuery;

class Terrain : public U2Node
{
public:
	Terrain(U2Camera* pSceneCam);
	~Terrain();

	bool Initialize(int iBlockSize);
	void Terminate();

	bool InitHeightField(bool bInitHeight = true, bool bInitNormal = true);
	bool InitShader();
	bool SetBlockSize(int iBlockSize);

	bool SetHeightMap(const TCHAR* heightmap);
	bool SetNormalMap(const TCHAR* normalMap);

	virtual void UpdateWorldData(float fTime);
	class TerrainMesh*		GetMesh() const { return m_pMesh; }
	int	GetMaxLevels() const { return m_iMaxLevels; }

	void Render(float fMinU, float fMinV, float fMaxU, float fMaxV, int iLevel, float fScale);

private:
	static int CalculateMaxLevels(int iBlockSize, int iHeigthfieldSize);
	
	
	bool Draw(float fBiasU, float fBiasV, float fScale, int iLevel);

	void AvoidCollision();
	void SetCameraElevation(float fLocalHeight, float fHeightOverTerrain);

	

private:
	
	D3DSURFACE_DESC					m_surfDesc;
	D3DXVECTOR4						m_vLightDir;

	
	U2Dx9TexturePtr m_spVertexTex;
	U2Dx9TexturePtr m_spNormalMap;

	U2Dx9FxShaderEffectPtr m_spEffect;
	U2D3DXEffectShaderPtr		m_spTerrainShader;
	
	HeightQuery*	m_pHeightQuery;

	class TerrainMesh*	m_pMesh;
	U2Camera*	m_pSceneCam;
	U2CameraNodePtr m_spCamNode;

	int		m_iMaxLevels;
	int		m_iBlockSize;
	int		m_iNumTriangles;
	int		m_iTime;
	float	m_fLOD;

	bool	m_bNormalInVS;

	U2DynString m_szHeightMap;
	U2DynString m_szNormalMap;

	friend class TerrainMesh;
	

};
