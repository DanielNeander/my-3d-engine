/**************************************************************************************************
module	:	U2SceneMgr
Author	:	Yun sangyong
Desc	:	
*************************************************************************************************/
#pragma once 
#ifndef	U2_SCENEMGR_H
#define	U2_SCENEMGR_H

#include <U2Lib/Src/U2RefObject.h>
#include <U2Lib/Src/U2SmartPtr.h>
#include "U2TBucket.h"
#include "U2Rect.h"
#include <U2_3D/Src/Dx9/nMeshGroup.h>


class U2LightNode;
class U2CameraNode;
class U2Mesh;
class U2N2Mesh;
class U2Dx9Renderer;
class U2VisibleSet;
class U2FrameSection;
class U2FramePass;
class U2FramePhase;
class U2FrameSequence;
class U2RenderContext;
class U2ShadowNode;
class U2Dx9ShadowMgr;

U2SmartPointer(U2Node);
U2SmartPointer(U2Frame);
U2SmartPointer(U2D3DXEffectShader);

class U2_3D U2SceneMgr : public U2RefObject
{
public:	
	

	
	
	virtual ~U2SceneMgr();
	static U2SceneMgr* Instance();

	static U2SceneMgr* Create(U2Dx9Renderer* pRenderer);

	bool Initialize();
	void Terminate();

		
	class CommonInfo : public U2MemObj
	{
	public:
		U2RenderContext* m_pRenderContext;
		int m_iLightPass;
	};
	
	class MeshInfo : public CommonInfo
	{
	public:
		nMeshGroup* m_pMeshGroup;	// 		
		int m_iPriority;	// Render Sort Priority
	};

	class ShadowInfo : public CommonInfo
	{
	public:
		U2ShadowNode* m_pShadowNode;	// 		
		int m_iPriority;	// Render Sort Priority
	};
	

	class LightInfo : public CommonInfo
	{
	public:
		LightInfo() : m_bClipping(false), m_scissorRect(0.0f, 1.0f, 0.0f, 1.0f),
			m_pLightNode(0)
		{

		}

		~LightInfo();		  

		U2LightNode* m_pLightNode;	// Light
		U2Rect<float> m_scissorRect;	// scissor rect of the light
		D3DXVECTOR4 m_shadowLightMask;	// shadow light index 
		U2Plane m_aPlane[6];
		bool m_bClipping;
	};

	class CameraInfo : public CommonInfo
	{
	public:
		U2CameraNode* m_pCameraNode;
		U2DynString		m_szFrameSection;
		
	};

	U2Frame* GetFrame() { return m_spFrame; }
	
	
	void SetObjectLightLinks(bool bEnable);
	bool GetObjectLightLinks() const;
	
	void ResetLightScissorsAndClipPlanes();

	void ApplyLightScissors(const LightInfo& lightInfo);
	void ApplyLightClipPlanes(const LightInfo& lightInfo);
	//void ComputeLightScissorAndClipPlanes();

	void GatherShadowLights();

	void DrawScene(U2VisibleSet& visibleSet);

	void DrawFrame(U2FrameSection& frameSection);

	void RenderShadowVolume(U2FramePass& curPass);
	void RenderShadowMap(U2FramePass& curPass);

	void RenderPhaseOffLight(U2FramePhase& curPahse);
	/// render a phase for light mode "Fixed Function Pipline"
	//void RenderPhaseFFPLight(U2FramePhase& curPhase);

	/// render a phase for light mode "Shader"
	void RenderPhaseShaderLight(U2FramePhase& curPhase);

	void RenderMeshShaderLight(MeshInfo& meshInfo, const U2FrameSequence& frameSeq);

	
	inline void SetCameraEnabled(bool bCameraEnable) { m_bCameraEnabled = bCameraEnable; }	
	inline const bool	GetCameraEnabled() const { return m_bCameraEnabled; }
	
	inline void SetShadowEnabled(bool bShadow) { m_bShadowEnabled = bShadow; }
	inline const bool GetShadowEnabled() const { return m_bShadowEnabled; }

	/// update shared shader parameters per frame
	void UpdatePerFrameSharedShaderParams();	// shared shader 변수 추가..	
	/// update shared shader parameters per scene
	void UpdatePerSceneSharedShaderParams();

	
	U2NodePtr GetRootNode() { return m_spRootNode; }

	bool BeginScene( const D3DXMATRIX viewMat);
	bool EndScene();


	
	// Shape Render Related Func 
	void BeginShape();
	void DrawShape(U2Mesh* pMesh, 
		const D3DXMATRIX& modelMat, const D3DXVECTOR4& color, bool bSetShaderParam = true);	
	void EndShape();
	
	
	void AttachMeshGroup(U2N2Mesh* pMesh);
	
	//void DrawShapeIndexedPrimitives(const D3DXMATRIX& modelMat);

	void CopyStencilBuffertoTexture(U2FramePass& pass,const D3DXVECTOR4& shadowLitMask);


	enum 
	{
		NUM_BUCKETS = 64,
		MAX_SHADOWLIGHTS = 4,		
	};

	
	
	


	U2PrimitiveVec<LightInfo*> m_aLights;
	U2PrimitiveVec<LightInfo*> m_aShadowLights;
	U2PrimitiveVec<CameraInfo*>	m_aCameras;
	U2PrimitiveVec<MeshInfo*>	m_aMeshes;
	U2PrimitiveVec<ShadowInfo*> m_aShadows;


	U2TBuket<U2PrimitiveVec<uint16>, uint16, NUM_BUCKETS> m_aMeshBuckts;
	
private:
	U2SceneMgr(U2Dx9Renderer* pRenderer);	

	void SortNodes();
	static int __cdecl CompareNodes(const uint16* i1, const uint16 *i2);
	//static int __cdecl CompareShadowLights(const LightInfo* pLight1, const LightInfo* pLight2);

	void UpdateCameraNodes();

	void DrawReflectOrRefractionScene();


public:	
	
	bool m_bObjLightLinks;
	bool m_bClipPlaneFencing;
	bool m_bCameraEnabled;	// Reflection, Refraction Scene을 렌더링할 때 비트플래그
	bool m_bShadowEnabled;
	
	static int ms_iSortingOrder;

		

	
	
	U2Dx9Renderer *m_pRenderer;

	static U2SceneMgr* ms_pInstance;

	
	//U2FixedPrimitiveArray<int> 

	U2FramePtr m_spFrame;	
	U2NodePtr m_spRootNode;
	U2Dx9ShadowMgr* m_pShadowMgr;

	static D3DXVECTOR3 ms_cameraPos;

};

typedef U2SmartPtr<U2SceneMgr> U2SceneMgrPtr;

#define SceneMgr		U2SceneMgr::Instance()

#include "U2SceneMgr.inl"

#endif 