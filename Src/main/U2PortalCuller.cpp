#include <U2_3D/src/U23DLibPCH.h>
#include "U2PortalCuller.h"



U2PortalCuller::U2PortalCuller(uint32 maxObj, 
							   uint32 uiGrowBy, const U2Camera* pCam)
							   :U2Culler(maxObj, uiGrowBy, pCam),
							   m_uniqueObjs(maxObj > 0 ? maxObj : 1024)
{


}


U2PortalCuller::~U2PortalCuller()
{

}


void U2PortalCuller::Insert(U2Spatial* pObj, U2Effect* pGlobalEffect)
{
	if(pObj)
	{
		if(&pObj != m_uniqueObjs.Insert(pObj))
		{
			// The object was not in the list of unique objects.
			U2Culler::Insert(pObj, pGlobalEffect);
		}
	}
	else 
	{
		// This is an end-sentinel for a global effect.  There can be
		// multiple copies of these.
		U2Culler::Insert(0, 0);
	}
}
