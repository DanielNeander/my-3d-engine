#include <U2_3D/Src/U23DLibPCH.h>
#include "U2ConvexRegionMgr.h"
#include "U2ConvexRegion.h"


IMPLEMENT_RTTI(U2ConvexRegionMgr, U2BspNode);
U2ConvexRegionMgr::U2ConvexRegionMgr()
{

}


U2ConvexRegionMgr::~U2ConvexRegionMgr()
{

}

U2SpatialPtr U2ConvexRegionMgr::AttachOutsideScene(U2Spatial* pOutside)
{
	return SetChild(1, pOutside);
}


U2SpatialPtr U2ConvexRegionMgr::DetachOutsideScene()
{
	return DetachChild(1);
}


U2SpatialPtr U2ConvexRegionMgr::GetOutsideScene()
{
	return GetChild(1);
}


U2ConvexRegion* U2ConvexRegionMgr::GetContainingRegion(const D3DXVECTOR3& eye)
{
	return DynamicCast<U2ConvexRegion>(GetContainingNode(eye));
}


void U2ConvexRegionMgr::GetVisibleSet(U2Culler& culler, bool bNoCull) 
{
	U2ConvexRegion* pRegion = GetContainingRegion(culler.GetCamera()->GetEye());

	if(pRegion)
	{
		// Accumulate visible objects starting in the region containing the
		// camera.  Use the CRMCuller to maintain a list of unique objects.
		//pkRegion->GetVisibleSet(m_kCuller,bNoCull);
		m_culler.SetCamera(culler.GetCamera());
		m_culler.SetFrustumLH(culler.GetFrustum());
		m_culler.ComputeVisibleSet(pRegion);

		// Copy the unique list to the scene culler.
		U2VisibleSet& visibles = m_culler.GetVisibleSet();
		const uint32 uVisibleCnt = visibles.GetCount();
		for(uint32 i=0; i < uVisibleCnt; ++i)
		{
			U2VisibleObject& obj = visibles.GetVisible(i);
			culler.Insert(obj.m_pObj, obj.m_pGlobalEffect);
		}
	}
	else 
	{
		// The camera is outside the set of regions.  Accumulate visible
		// objects for the outside scene (if it exists).
		if(GetOutsideScene())
		{
			GetOutsideScene()->GetVisibleSet(culler, bNoCull);
		}
	}
}