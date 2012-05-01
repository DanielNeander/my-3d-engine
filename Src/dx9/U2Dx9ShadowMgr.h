/************************************************************************
module	:	U2Dx9ShadowMgr
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once 
#ifndef U2_DX9_SHADOWMGR_H
#define U2_DX9_SHADOWMGR_H

#include <u2_3d/src/main/U2Object.h>
#include "U2Dx9ShadowCaster.h"

class U2Light;
U2SmartPointer(U2D3DXEffectShader);

class U2ShadowTechnique;

class U2Dx9ShadowMgr : public U2Object 
{
	
public:
	U2Dx9ShadowMgr(U2Camera* pSceneCam);
	virtual ~U2Dx9ShadowMgr();

	static U2Dx9ShadowMgr* Instance();

	U2Dx9ShadowCaster* CreateShadowCaster(U2Dx9ShadowCaster::ShadowType type, 
		const TCHAR* szName);

	bool BeginShadow();

	void BeginLight(const U2Light& light);

	void RenderShadowCaster(U2Dx9ShadowCaster* pCaster, const D3DXMATRIX& worldMat);

	void EndLight();

	void EndShadow();

	void SetDepthFail(bool bFail);
	bool GetDepthFail() const;

	void SetEnableShadows(bool b);
	bool GetEnableShadows() const;

	bool Initialize();
	void Terminate();

	enum {
		MAX_SPLITS = 10,

	};

	inline void SetTargetScene(U2Node* pScene) { m_spScene = pScene; }

	void AddShadowCaster(U2N2Mesh* pCaster);
	void AddShadowReceiver(U2N2Mesh* pReceiver);

	void RenderShadowMap();

	inline U2Camera* GetActiveCam() const { return m_spSceneCam; }

	
private:
	void    ComputeVirtualCameraParameters();
	void	Cull(U2Spatial* pSpatial);
	void	TestIntersectBox(U2Spatial* pSpatial, bool& bHit);



public:
	D3DXMATRIX m_View;
	D3DXMATRIX m_Projection;
	
private:		
	static U2Dx9ShadowMgr* ms_pSingleton;

	//  near & far scene clip planes

	std::vector<U2Aabb> m_ShadowCasterPoints;
	std::vector<U2Aabb> m_ShadowReceiverPoints;

	U2ObjVec<U2MeshGroupPtr> m_aShadowCasters;
	U2ObjVec<U2MeshGroupPtr> m_aShadowReceivers;

	U2ObjVec<U2Dx9TexturePtr> m_aShadowMaps;

	
	// Transforms
	D3DXMATRIX m_World;

	float m_fSplitPos[MAX_SPLITS];

	D3DXVECTOR3 m_lightDir;


	int m_iShadowType;
	bool m_bUnitcubeClip;
	bool m_bDisplayShadowMap;

	U2Culler m_sceneCuller;	


	U2D3DXEffectShaderPtr m_spShadowVolShader;
	

	U2NodePtr m_spScene;
	U2CameraPtr m_spSceneCam;

	
	U2ShadowTechnique*	m_pActiveShadowTec;
	
	
	
	bool m_bBeginShadow;
	bool m_bBeginLit;
	bool m_bUseDepthFail;
	bool m_bShadowEnabled;
	U2Light* m_pCurLight;
	uint32 m_uiNumShaderPasses;
};

#include "U2Dx9ShadowMgr.inl"


#endif