/**************************************************************************************************
module	:	U2PortalCuller
Author	:	Yun sangyong
Desc	:
***************************************************************************************************/
#pragma once
#ifndef U2_PORTALCULLER_H
#define U2_PORTALCULLER_H


#include "U2Culler.h"

class U2_3D U2PortalCuller : public U2Culler
{
public:
	U2PortalCuller(uint32 maxObj =0, uint32 iGrowBy = 0, const U2Camera* pCam = 0);
	virtual ~U2PortalCuller();

	// Two portals leading into the room can be visible to the camera.  The
	// Culler would store visible objects twice.  PortalCuller maintains a list
	// of unique objects.
	virtual void Insert(U2Spatial* pObj, U2Effect* pGlobalEffect);

protected:
	U2THashSet<U2Spatial*> m_uniqueObjs;
	
};

#endif
