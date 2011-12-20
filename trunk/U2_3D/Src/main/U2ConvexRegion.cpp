#include <U2_3D/src/U23DLibPCH.h>
#include "U2Portal.h"
#include "U2ConvexRegion.h"


IMPLEMENT_RTTI(U2ConvexRegion, U2Node);

U2ConvexRegion::U2ConvexRegion(uint32 uPortalCnt, U2PrimitiveVec<U2PortalPtr>* aspPortal)
{
	m_uPortalCnt = uPortalCnt;
	m_aspPortal = aspPortal;
	m_bVisited = false;
}

U2ConvexRegion::U2ConvexRegion()
{
	m_uPortalCnt = 0;
	m_aspPortal = 0;
	m_bVisited = false;
}


U2ConvexRegion::~U2ConvexRegion()
{
	m_aspPortal->RemoveAll();

	U2_DELETE m_aspPortal;
}


void U2ConvexRegion::UpdateWorldData(float fAppTime)
{
	U2Node::UpdateWorldData(fAppTime);

	for(uint32 i=0; i < m_uPortalCnt; ++i)
	{
		m_aspPortal->GetElem(i)->UpdateWorldData(m_tmWorld);
	}
}


void U2ConvexRegion::GetVisibleSet(U2Culler& culler, bool bNoCull)
{
	if(!m_bVisited)
	{
		m_bVisited = true;		

		// Add anything visible through open portals.
		for(uint32 i=0; i < m_uPortalCnt; ++i)
		{
			m_aspPortal->GetElem(i)->GetVisibleSet(culler, bNoCull);
		}

		// Add the region walls and contained objects.
		U2Node::GetVisibleSet(culler, bNoCull);

		m_bVisited = false;
	}
}
