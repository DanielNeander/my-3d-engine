#pragma once 
/**************************************************************************************************
module	:	U2ShadowTechnique
Author	:	Yun sangyong
Date	:	2010/06/11
Desc	:	
*************************************************************************************************/

#include "U2ShadowCommon.h"

class U2_3D U2ShadowTechnique
{
public:

	U2ShadowTechnique();
	~U2ShadowTechnique();
	
	bool CreateShadowMap();

	bool Initialize();
	void Terminate();

public:
	float m_fAspect, m_zNear, m_zFar, m_ppNear, m_ppFar, m_fSlideBack, m_fLSPSM_Nopt, m_fCosGamma;

	float m_fMinInfinityZ;
	bool m_bSlideBack;
	bool m_bUnitCubeClip;

	void BeginPass(const nMeshGroup& curMesh);

	void EndPass();

	void UpdateShadowParams();


	void RenderShadowMap(nMeshGroup* curMesh);

	void OnLostDevice();
	void OnResetDevice();

	void DrawQuad();

private:

	void    BuildPSMProjectionMatrix();
	bool	CreateQuad();

	
private:
	friend class U2Dx9ShadowMgr;

	D3DXMATRIX m_LightViewProj;


	U2RenderTargetPtr m_spShadowMapRT;
	U2Dx9TexturePtr m_spPSMColorTex;
	U2Dx9TexturePtr m_spPSMDepthTex;

	bool m_bSupportsPixelShaders20;
	bool m_bSupportsHWShadowMaps;
	bool m_bDisplayShadowMap;


	bool m_bShadowTestInverted;


	U2CameraPtr m_spShadowCam;
	U2N2MeshPtr m_spSMQuad;
	U2D3DXEffectShaderPtr m_spShadowMapShader;

	uint32 m_uiNumShaderPasses;
	int m_iDepthBias;
	float m_fBiasSlope;

	

	IDirect3DVertexDeclaration9* m_pShadowVertexDecl;



	
};

