#pragma once 

#pragma warning(push)
#pragma warning(disable: 4324)

#include "ShadowSampleMisc.h"

class ShadowMapCamera;

class Mesh;
class CDXUTSDKMesh;

_DECLSPEC_ALIGN_16_ class CascadedShadowsManager : public Singleton<CascadedShadowsManager>
{
public:
	CascadedShadowsManager(CascadeConfig* cascadeConfig);
	~CascadedShadowsManager();
		
	void Init( const BoundingBox& aabb, ShadowMapCamera* pkViewCamera, ShadowMapCamera* pkLightCamera );
	void InitFrame ( ID3D11Device* pd3dDevice);

	// start rendering to texture
	void EnableRendering(void);
	// stop rendering to texture
	void DisableRendering(void);
	void RenderShadow(Mesh* pMesh);
	void RenderShadow2(CDXUTSDKMesh* pMesh);

	void DestroyAndDeallocateShadowResources();

	void RenderShadowsForAllCascades();
	
	void RenderScene( ShadowMapCamera* pActiveCamera, Mesh* pMesh );
	void RenderScene2 ( ShadowMapCamera* pActiveCamera, CDXUTSDKMesh* pMesh);
	noVec4 GetSceneAABBMin() { return m_vSceneAABBMin; };
	noVec4 GetSceneAABBMax() { return m_vSceneAABBMax; };



	// This runs per frame.  This data could be cached when the cameras do not move.
	INT                                 m_iCascadePartitionsMax;
	FLOAT                               m_fCascadePartitionsFrustum[MAX_CASCADES]; // Values are  between near and far
	INT                                 m_iCascadePartitionsZeroToOne[MAX_CASCADES]; // Values are 0 to 100 and represent a percent of the frstum
	INT                                 m_iPCFBlurSize;
	FLOAT                               m_fPCFOffset;
	INT                                 m_iDerivativeBasedOffset;
	INT                                 m_iBlurBetweenCascades;
	FLOAT                               m_fBlurBetweenCascadesAmount;

	BOOL                                m_bMoveLightTexelSize;
	FIT_PROJECTION_TO_CASCADES          m_eSelectedCascadesFit;
	FIT_TO_NEAR_FAR                     m_eSelectedNearFarFit;
	CASCADE_SELECTION                   m_eSelectedCascadeSelection;

	ShaderID m_vsorthoShadowShader;
	ShaderID m_vsSceneShader[MAX_CASCADES];
	ShaderID                  m_aaSceneAllShaders[MAX_CASCADES][2][2][2];
	VertexBufferID				m_vb;

	noVec4                            m_vSceneAABBMin;
	noVec4                            m_vSceneAABBMax;

private:

	// Compute the near and far plane by intersecting an Ortho Projection with the Scenes AABB.	
	void ComputeNearAndFar( FLOAT& fNearPlane, FLOAT& fFarPlane, const noVec4& vLightCameraOrthographicMin, const noVec4& vLightCameraOrthographicMax, noVec4* pvPointsInCameraView );
	void CreateFrustumPointsFromCascadeInterval ( FLOAT fCascadeIntervalBegin, 
		FLOAT fCascadeIntervalEnd, 
		noMat4& vProjection,
		noVec4* pvCornerPointsWorld
		);
	
	
	void ReleaseAndAllocateNewShadowResources( ID3D11Device* pd3dDevice );  // This is called when cascade config changes. 
	void CreateAABBPoints( noVec4* vAABBPoints, const noVec4& vCenter, const noVec4& vExtents );	
	
	
	char                                m_cvsModel[31];
	char                                m_cpsModel[31];
	char                                m_cgsModel[31];
	mat4                          m_matShadowProj[MAX_CASCADES]; 
	mat4                          m_matShadowView;
	CascadeConfig                       m_CopyOfCascadeConfig;      // This copy is used to determine when settings change. 
	//Some of these settings require new buffer allocations.
	CascadeConfig*                      m_pCascadeConfig;           // Pointer to the most recent setting.

	ShadowMapCamera*						m_pViewCamera;
	ShadowMapCamera*						m_pLightCamera;

	D3D11_VIEWPORT                      m_RenderVP[MAX_CASCADES];
	D3D11_VIEWPORT                      m_RenderOneTileVP;

	ID3D11Buffer*                       m_pcbGlobalConstantBuffer; // All VS and PS constants are in the same buffer.  
	// An actual title would break this up into multiple 
	// buffers updated based on frequency of variable changes

	ID3D11RasterizerState*              m_prsScene;
	ID3D11RasterizerState*              m_prsShadow;
	ID3D11RasterizerState*              m_prsShadowPancake;


	ID3D11InputLayout*                  m_pVertexLayoutMesh;
	ID3D11VertexShader*                 m_pvsRenderOrthoShadow;
	ID3DBlob*                           m_pvsRenderOrthoShadowBlob;
	ID3D11VertexShader*                 m_pvsRenderScene[MAX_CASCADES];
	ID3DBlob*                           m_pvsRenderSceneBlob[MAX_CASCADES];
	ID3D11PixelShader*                  m_ppsRenderSceneAllShaders[MAX_CASCADES][2][2][2];
	ID3DBlob*                           m_ppsRenderSceneAllShadersBlob[MAX_CASCADES][2][2][2];
	ID3D11Texture2D*                    m_pCascadedShadowMapTexture ;
	ID3D11DepthStencilView*             m_pCascadedShadowMapDSV ;
	ID3D11ShaderResourceView*           m_pCascadedShadowMapSRV ;
	
	ID3D11SamplerState*                 m_pSamLinear;
	ID3D11SamplerState*                 m_pSamShadowPCF;
	ID3D11SamplerState*                 m_pSamShadowPoint;
};

#pragma warning(pop)
