#include <U2_3D/Src/U23DLibPCH.h>
#include "U2SwitchNode.h"


IMPLEMENT_RTTI(U2SwitchNode, U2Node);

U2SwitchNode::U2SwitchNode()
:m_uiActiveChild(SN_INVALID_CHILD)
{
	
}


U2SwitchNode::~U2SwitchNode()
{

}


void U2SwitchNode::SetActiveChild(uint32 uActiveChild)
{
	U2ASSERT(uActiveChild == SN_INVALID_CHILD || uActiveChild < GetChildCnt());
	m_uiActiveChild = uActiveChild;
}


uint32 U2SwitchNode::GetActiveChild() const
{
	return m_uiActiveChild;
}


void U2SwitchNode::DisableAllChildren()
{
	m_uiActiveChild = SN_INVALID_CHILD;
}


void U2SwitchNode::GetVisibleSet(U2Culler& culler, bool bNoCull)
{
	if(m_uiActiveChild == (unsigned int)SN_INVALID_CHILD)
	{
		return;
	}

	uint32 i;
	for(i=0; i < m_aspEffects.Size(); ++i)
	{
		culler.Insert(this, m_aspEffects.GetElem(i));
	}

	U2Spatial* pChild = m_childArray.GetElem(m_uiActiveChild);
	if(pChild)
	{
		pChild->GetVisibleSet(culler, bNoCull);
	}

	for(i=0; i < m_aspEffects.Size(); ++i)
	{
		culler.Insert(0, 0);
	}

}

