#include <U2_3D/Src/U23DLibPCH.h>
#include "U2DLODNode.h"


IMPLEMENT_RTTI(U2DLODNode, U2SwitchNode);

U2DLODNode::U2DLODNode()
{

}


U2DLODNode::~U2DLODNode()
{

}

void U2DLODNode::SetLocalDistance(int i, float fMinDist, float fMaxDist)
{
	U2ASSERT(i >= 0);

	if(i < m_localMinDists.Size())
	{
		m_localMinDists.SetElem(i, fMinDist);
		m_localMaxDists.SetElem(i, fMaxDist);
		m_worldMinDists.SetElem(i, fMinDist);
		m_worldMaxDists.SetElem(i, fMaxDist);
	}
	else 
	{
		m_localMinDists.AddElem(fMinDist);
		m_localMaxDists.AddElem(fMaxDist);
		m_worldMinDists.AddElem(fMinDist);
		m_worldMaxDists.AddElem(fMaxDist);
	}
}


void U2DLODNode::SelectLOD(const U2Camera* pCamera)
{
	m_vWorldLODCenter = m_tmWorld.Apply(m_vWorldLODCenter);

	uint32 i;
	for(i=0; i < m_childArray.Size(); ++i)
	{
		float fValue = m_localMinDists.GetElem(i);
		m_worldMinDists.SetElem(i, m_tmWorld.GetScale().x * fValue);

		fValue = m_localMinDists.GetElem(i);
		m_worldMaxDists.SetElem(i, m_tmWorld.GetScale().x * fValue);
	}

	SetActiveChild(SN_INVALID_CHILD);
	if(m_childArray.Size() > 0)
	{
		D3DXVECTOR3 diff = m_vWorldLODCenter - pCamera->GetEye();
		float fDist = D3DXVec3Length(&diff);

		for(i=0; i < m_childArray.Size(); ++i)
		{
			if(m_worldMinDists.GetElem(i) <= fDist && fDist < m_worldMaxDists.GetElem(i))
			{
				SetActiveChild(i);
				break;
			}
		}
	}

}

void U2DLODNode::GetVisibleSet(U2Culler& culler, bool bNoCull)
{
	SelectLOD(culler.GetCamera());
	U2SwitchNode::GetVisibleSet(culler, bNoCull);
}
