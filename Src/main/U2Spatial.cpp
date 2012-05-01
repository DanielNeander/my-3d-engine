#include <U2_3D/src/U23DLibPCH.h>
#include "U2Node.h"

IMPLEMENT_RTTI(U2Spatial, U2Object);

U2Spatial::U2Spatial()
:m_spWorldBound(U2BoundingVolume::Create()),
m_bActive(true)
{
	m_uFlags = 0;
	m_pParent = 0;	
	m_uStartEffect = 0;
	
	m_spRSC = 0;

	m_eCulling = CULL_DYNAMIC;
	m_bCurrWorldBound = false;
	m_bCurrWorldTM = false;	
}


U2Spatial::~U2Spatial()
{
	DetachAllRenderStates();
	DetachAllEffects();
	DetachAllLights();		
}




void U2Spatial::AttachParent(U2Node* pkParent)
{
	U2Node* pOldParent = GetParent();
	if(pOldParent)
		pOldParent->DetachChild(this);
	m_pParent = pkParent;
}


void U2Spatial::DetachParent()
{
	m_pParent = 0;
}


void U2Spatial::Update(float fTime, bool bInit)
{
	UpdateWorldData(fTime);
	UpdateWorldBound();

	if(bInit)
		 PropagateBoundToRoot();
}


void U2Spatial::UpdateWorldData(float fTime)
{

	if(!m_bCurrWorldTM)
	{
		if(m_pParent)
		{
			m_tmWorld.Product(m_pParent->m_tmWorld, m_tmLocal);
		}
		else
		{
			//m_tmWorld.SetRot(m_tmLocal.GetRot());
			//m_tmWorld.SetTrans(m_tmLocal.GetTrans());
			//m_tmWorld.SetScale(m_tmLocal.GetScale());
			m_tmWorld = m_tmLocal;
		}
	}	

	// 실시간으로 업데이트 하므로 성능에 영향이 크므로 주의할 것.
//#ifdef _DEBUG
//	if(m_pParent)
//	{	
//		DPrintf("Parent Object Name : %s\n", FromUnicode(m_pParent->GetName().c_str()));
//		DPrintf("Parent Local Trans : %8.3f, %8.3f, %8.3f\n", m_pParent->m_tmLocal.GetTrans().x,
//			m_pParent->m_tmLocal.GetTrans().y, m_pParent->m_tmLocal.GetTrans().z);
//		DPrintf("Parent Local Rotate : ", m_pParent->m_tmLocal.GetRot());
//
//		DPrintf("Parent World Trans : %8.3f, %8.3f, %8.3f\n", m_pParent->m_tmWorld.GetTrans().x,
//			m_pParent->m_tmWorld.GetTrans().y, m_pParent->m_tmWorld.GetTrans().z);
//
//		DPrintf("Parent World Rotate : ",m_pParent->m_tmWorld.GetRot());
//	}
//
//	
//	DPrintf("THIS Object Name : %s\n", FromUnicode(GetName().c_str()));	
//	DPrintf("THIS Local Trans : %8.3f, %8.3f, %8.3f\n", m_tmLocal.GetTrans().x,
//		m_tmLocal.GetTrans().y, m_tmLocal.GetTrans().z);
//	DPrintf("THIS Local Rotate : ", m_tmLocal.GetRot());
//
//	DPrintf("THIS World Trans : %8.3f, %8.3f, %8.3f\n", m_tmWorld.GetTrans().x,
//		m_tmWorld.GetTrans().y, m_tmWorld.GetTrans().z);
//
//	DPrintf("THIS World Rotate : ",m_tmWorld.GetRot());
//#endif 
	
}


void U2Spatial::UpdateBound()
{
	UpdateWorldBound();
	PropagateBoundToRoot();
}



void U2Spatial::UpdateSelected(float fTime)
{

}



void U2Spatial::PropagateBoundToRoot ()
{
	if (m_pParent)
	{
		m_pParent->UpdateWorldBound();
		m_pParent->PropagateBoundToRoot();
	}
}


void U2Spatial::UpdateRenderStates(U2RenderStateCollectionPtr spRSC, 	
								   U2ObjVec<U2LightPtr>* pLightArray )
{	
	bool bCreateRS = (spRSC == 0);

	if(bCreateRS)
	{
		spRSC = U2_NEW U2RenderStateCollection;

		pLightArray = U2_NEW U2ObjVec<U2LightPtr>;

		PropagateStateFromRoot(spRSC, pLightArray);		
	}
	else 
	{
	m_spRSC = PushRenderStates(spRSC, true);
		PushLights(pLightArray);
	}

	// 순수가상함수로서 Spatial 상속 클래스 고유의 기능을 실행한다. 
	UpdateState(spRSC, pLightArray);

	if(bCreateRS)
	{
		spRSC = 0;
		U2_DELETE pLightArray;
	}
	else 
	{
		uint32 i;

		for(i=0; i < m_aspLights.Size();++i)		
		{		
			pLightArray->Remove(i);
		}
	}
}


void U2Spatial::PropagateStateFromRoot(U2RenderStateCollectionPtr& spRSC, 
									   U2ObjVec<U2LightPtr>* pLightArray)
{
	if(m_pParent)
		m_pParent->PropagateStateFromRoot(spRSC, pLightArray);
	
	m_spRSC = PushRenderStates(spRSC, false);
	PushLights(pLightArray);
}


U2RenderStateCollectionPtr U2Spatial::PushRenderStates(
	U2RenderStateCollectionPtr spParentRS, bool bCopyOnChange)
{
	if(GetRSList().IsEmpty())
	{
		return spParentRS;
	}
	else 
	{
		U2RenderStateCollectionPtr spState;
		// 로칼 렌더 스테이트를 사용할 것인지 
		// 그렇지 않을지 체크
		if(bCopyOnChange)
			spState = U2_NEW U2RenderStateCollection(*spParentRS);
		else 
			spState = spParentRS;

		U2ListNode<U2RenderState*>* pNode = m_renderStateList.GetHeadNode();
		while(pNode)
		{
			spState->SetRenderState(pNode->m_elem);
			pNode = m_renderStateList.GetNextNode(pNode);
		}		
		return spState;
	}
}

uint32 U2Spatial::PushLights(
				  U2ObjVec<U2LightPtr>* pLightArray)
{
	if(!m_aspLights.Size())
		return 0;
	else 
	{
		for(uint32 u=0; u < m_aspLights.FilledSize(); ++u)
		{
			pLightArray->AddElem(m_aspLights.GetElem(u));
		}
		return pLightArray->Size();
	}
}

void U2Spatial::AttachLight(U2Light* pLight)
{
	U2ASSERT(pLight);

	for(uint32 i=0; i < m_aspLights.Size(); ++i)
	{
		if(m_aspLights.GetElem(i) == pLight)
			return;
	}

	m_aspLights.AddElem(pLight);
}

void U2Spatial::DetachLight(U2Light* pLight)
{
	for(uint32 i=0; i < m_aspLights.Size(); ++i)
	{
		if(m_aspLights.GetElem(i) == pLight)
		{
			m_aspLights.Remove(i);
			return;
		}
	}
}


void U2Spatial::AttachEffect(U2Effect* pEffect)
{
	U2ASSERT(pEffect);

	for(uint32 i=0; i < m_aspEffects.Size(); ++i)
	{
		if(m_aspEffects.GetElem(i) == pEffect)
			return;
	}

	m_aspEffects.AddElem(pEffect);

}

void U2Spatial::DetachEffect(U2Effect* pEffect)
{
	for(uint32 i=0; i < m_aspEffects.Size(); ++i)
	{
		if(m_aspEffects.GetElem(i) == pEffect)
		{
			m_aspEffects.Remove(i);
			return;
		}
	}

}


void U2Spatial::OnVisibleSet(U2Culler& rkCuller, bool bNoCull)
{
	if(m_eCulling == CULL_ALWAYS)
	{
		return;
	}

	if(m_eCulling == CULL_NEVER)
	{
		bNoCull = true;
	}

	uint32 uSavePlaneState = rkCuller.GetPlaneState();
	if(bNoCull || rkCuller.IsVisible(m_spWorldBound))	
	{
		GetVisibleSet(rkCuller, bNoCull);
	}
	rkCuller.SetPlaneState(uSavePlaneState)	;
}


//void						Cull();
