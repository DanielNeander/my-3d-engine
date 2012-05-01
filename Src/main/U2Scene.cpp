#include <U2_3D/Src/U23DLibPCH.h>
#include "U2Scene.h"



U2Scene* U2Scene::Create()
{
	U2Scene* pThis = U2_NEW U2Scene();
	U2ASSERT(pThis);
	if( !pThis->Initialize() )
	{
		U2_DELETE pThis;
		return NULL;
	}
	return pThis;
}



U2Scene::U2Scene()
:m_spRootNode(0),
m_pDefaultLitNode(0)
{

}

U2Scene::~U2Scene()
{
	m_pDefaultLitNode = 0;
	m_spRootNode = 0;	
}

bool U2Scene::Initialize()
{
	m_spRootNode = U2_NEW U2Node;

	U2ASSERT(U2VariableMgr::Instance());

	m_timeHandle = U2VariableMgr::Instance()->GetVariableHandleByName(_T("time"));

	// 메모리 릭
	AddDefaultLight();

	return true;
}


void U2Scene::Trigger(float fAppTime, unsigned int frameId)
{
	uint32 idx;
	uint32 numNodes = m_aspNodes.FilledSize();
	for(idx = 0; idx < numNodes; ++idx)
	{
		TransferGlobalVars(*m_aspRenderContexts[idx], fAppTime, frameId);
	}	

}


void U2Scene::TransferGlobalVars(U2RenderContext& context, float time, unsigned int frameId)
{
	context.GetVariable(m_timeHandle)->SetFloat(time);
	context.SetFrameID(frameId);

	const U2VariableContext& globalContext = U2VariableMgr::Instance()->GetGlobalVariableContext();
	unsigned int numGlobalVars = globalContext.GetNumVariables();

	unsigned int globalVarIdx;
	for(globalVarIdx = 0; globalVarIdx < numGlobalVars; ++globalVarIdx)
	{
		const U2Variable& globalVar = *globalContext.GetVariable(globalVarIdx);
		U2Variable* pVar = context.GetVariable(globalVar.GetHandle());
		if(pVar)
		{
			*pVar = globalVar;
		}
		else 
		{
			U2Variable *pNewVar = U2_NEW U2Variable(globalVar);
			context.AddVariable(*pNewVar);
		}
	}
}

//-------------------------------------------------------------------------------------------------
void U2Scene::AddDefaultVariables(U2RenderContext& context)
{
	static const U2Float4 wind = { 1.0f, 0.0f, 0.0f, 0.5f};
	U2Variable::Handle oneHandle = U2VariableMgr::Instance()->GetVariableHandleByName(_T("one"));
	U2Variable::Handle windHandle = U2VariableMgr::Instance()->GetVariableHandleByName(_T("wind"));
	context.AddVariable(*U2_NEW U2Variable(m_timeHandle, 0.5f));
	context.AddVariable(*U2_NEW U2Variable(oneHandle, 1.0f));
	context.AddVariable(*U2_NEW U2Variable(windHandle, wind));
}

//-------------------------------------------------------------------------------------------------
void U2Scene::LoadObject(const U2DynString& szNamez)
{
			
	

}


void RecursiveFillMeshBucket(U2Spatial* pSpatial, U2RenderContext* pRenderContext)
{
	if(DynamicCast<U2N2Mesh>(pSpatial))
	{
		U2N2Mesh* pMesh = (U2N2Mesh*)pSpatial;

		uint16 i;
		for(i = 0; i < pMesh->GetNumGroups(); ++i)
		{
			U2SceneMgr::MeshInfo* pMeshInfo = U2_NEW U2SceneMgr::MeshInfo;
			pMeshInfo->m_pMeshGroup = &pMesh->Group(i);						

			pMeshInfo->m_pRenderContext = pRenderContext;
			pMeshInfo->m_iLightPass = 0;
			U2SceneMgr::Instance()->m_aMeshes.AddElem(pMeshInfo);

			uint32 shaderIdx = -1;

			for(uint32 j=0; j < pMeshInfo->m_pMeshGroup ->GetEffectCnt(); ++j)
			{
				U2Dx9FxShaderEffect* pEffect = DynamicCast<U2Dx9FxShaderEffect>
					(pMeshInfo->m_pMeshGroup ->GetEffect(j));
				if(pEffect)
				{
					pEffect->LoadResoures(NULL, NULL);
					shaderIdx = pEffect->GetShaderIdx();
					unsigned int meshIdx = U2SceneMgr::Instance()->m_aMeshes.FilledSize() - 1;
					U2SceneMgr::Instance()->m_aMeshBuckts[shaderIdx].AddElem(meshIdx);
					break;		
				}			
			}		
		}			
	}
	if(DynamicCast<U2ShadowNode>(pSpatial))
	{
		U2ShadowNode *pShadowNode = (U2ShadowNode*)pSpatial;

		U2SceneMgr::ShadowInfo *pShadow = U2_NEW U2SceneMgr::ShadowInfo;
		pShadow->m_pRenderContext = pRenderContext;
		pShadow->m_pShadowNode = pShadowNode;
		pShadow->m_iLightPass = 0;
		U2SceneMgr::Instance()->m_aShadows.AddElem(pShadow);
	}

	
	if(DynamicCast<U2Node>(pSpatial))
	{
		U2Node* pNode = (U2Node*)pSpatial;

		for(unsigned int i=0; i < pNode->GetChildCnt(); ++i)
		{
			U2Spatial *pChild = pNode->GetChild(i);
			
			if(pChild)
				RecursiveFillMeshBucket(pChild, pRenderContext);			
		}
	}
}

//-------------------------------------------------------------------------------------------------
void U2Scene::AddObject(U2Spatial* pSpatial)
{
	if(!pSpatial)
		return;

	U2RenderContext* pRenderContext = AddRenderContext(_T(""));
	U2ASSERT(pRenderContext->GetRootNode());
	
	m_spRootNode->AttachChild(pRenderContext->GetRootNode());
	m_spRootNode->UpdateBound();
	
	// Nebula Device에서 Mesh는 그리기 위해 필요한 정보만 포함하고 있으므로 
	// Transform 외에 많은 정보를 포함하는 엔진의 Mesh와 맞지 않는다. 
	// 그러므로 엔진에 맞게 nMeshGroup을 U2Mesh로 바꿔준다든지 등 변경해야 함.	
	// 현재 Mesh는 Scene Graph에 추가시키지 않는다. 
	// 단 nMeshGroup에 bbox 등 Geometry 정보를 추가시킬 필요가 있다.
	//pRenderContext->GetRootNode()->AttachChild(pSpatial);

	U2SceneMgr::Instance()->m_spRootNode->AttachChild(pRenderContext->GetRootNode());
	
	pRenderContext->GetRootNode()->AttachChild(pSpatial);

	// CreateObject.. 
	/*if(DynamicCast<U2N2Mesh>(pSpatial))
	{
		U2N2Mesh* pMesh = (U2N2Mesh*)pSpatial;

		uint16 i;
		for(i = 0; i < pMesh->GetNumGroups(); ++i)
		{
			U2SceneMgr::MeshInfo* pMeshInfo = U2_NEW U2SceneMgr::MeshInfo;
			pMeshInfo->m_pMeshGroup = &pMesh->Group(i);			

			pMeshInfo->m_pRenderContext = pRenderContext;
			pMeshInfo->m_iLightPass = 0;
			U2SceneMgr::Instance()->m_aMeshes.AddElem(pMeshInfo);

			uint32 shaderIdx = -1;

			for(uint32 j=0; j < pMeshInfo->m_pMeshGroup ->GetEffectCnt(); ++j)
			{
				U2Dx9FxShaderEffect* pEffect = DynamicCast<U2Dx9FxShaderEffect>
					(pMeshInfo->m_pMeshGroup ->GetEffect(j));
				if(pEffect)
				{
					pEffect->LoadResoures(NULL, NULL);
					shaderIdx = pEffect->GetShaderIdx();
					U2SceneMgr::Instance()->m_aMeshBuckts[shaderIdx].AddElem(i);
					break;
				}			
			}		
		}	
	}*/

	RecursiveFillMeshBucket(pSpatial, pRenderContext);

	U2SceneMgr::Instance()->m_spRootNode->UpdateRenderContext(pRenderContext);	
}

U2RenderContext*  U2Scene::AddRenderContext(const U2DynString& szName)
{

	U2RenderContext* pNewContext = U2_NEW U2RenderContext;

	U2Node* pContextRootNode = U2_NEW U2Node;	

	// 노드 목록에 추가 
	m_aspNodes.AddElem(pContextRootNode);

	// 기본 변수 추가.
	AddDefaultVariables(*pNewContext);

	// Set the Root Node of RenderContext
	pNewContext->SetRootNode(pContextRootNode);

	pNewContext->SetFlag(U2RenderContext::OCCLUDED, false);

	// 렌더 컨텍스트에 추가
	m_aspRenderContexts.AddElem(pNewContext);

	return pNewContext;
}


void U2Scene::UpdateRenderContext()
{
	uint32 i;
	uint32 numNodes = m_aspNodes.FilledSize();
	for(i = 0; i < numNodes; ++i)
	{
		m_aspNodes[i]->UpdateRenderContext(m_aspRenderContexts[i]);
	}
}



void U2Scene::AddDefaultLight()
{
	
	U2Light* pDirLight = U2_NEW U2Light(U2Light::LT_DIRECTIONAL);

	pDirLight->m_vLitPos = D3DXVECTOR3(0, 100, 0);
	pDirLight->SetDir(D3DXVECTOR3(0.0f, -1.0f, 0.0f));
	pDirLight->m_bCastShadows = true; // default true
	
	// 메모리 문제는 MemTracker의 구현상의 문제이므로
	// 로직에 너무 신경쓰지 말것.
	// U2DefaultAllocator::Allocate(__aligned_malloc)할당된 
	// 객체들만 메모리 릭이 발생하고 있음.
	U2LightNode* pLightNode = U2_NEW U2LightNode(pDirLight);

	m_pDefaultLitNode = pLightNode;

	D3DXMATRIX rotMat;

	D3DXMatrixRotationYawPitchRoll(&rotMat, DegToRad(-45.0f), DegToRad(45.0f), 0.f);

	pLightNode->SetLocalRot(rotMat);

	pLightNode->GetShaderAttb().SetArg(U2FxShaderState::LightRange, 1000.0f);
	pLightNode->GetShaderAttb().SetArg(U2FxShaderState::LightDiffuse, 
		D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
	pLightNode->GetShaderAttb().SetArg(U2FxShaderState::LightSpecular, 
		D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
	pLightNode->GetShaderAttb().SetArg(U2FxShaderState::LightAmbient, 
		D3DXVECTOR4(0.25f, 0.25f, 0.25f, 1.0f));

	U2SceneMgr *pSceneMgr = U2SceneMgr::Instance();

	U2SceneMgr::LightInfo* pNewLightInfo = U2_NEW U2SceneMgr::LightInfo;

	pNewLightInfo->m_pLightNode = pLightNode;

	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();
	pRenderer->GetLightMgr()->AddLight(*pLightNode);

	pNewLightInfo->m_pRenderContext = AddRenderContext(_T("stdlight"));
		
	pSceneMgr->m_aLights.AddElem(pNewLightInfo);
	

}