/************************************************************************
module	:	U2BoxBVTree
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_BOXBVTREE_H
#define	U2_BOXBVTREE_H

#include <U2_3D/Src/U2_3DLibType.h>
#include "U2BoundingVolumeTree.h"
#include <U2_3D/src/main/U2BoxBV.h>

class U2_3D U2BoxBVTree : public U2BoundingVolumeTree
{
	DECLARE_INITIALIZE;	
public:
	U2BoxBVTree(const U2TriList*pMesh, uint16 usMaxTrisPerLeaf = 1, bool bStoreInteriorTris = false);

protected:
	static U2BoundingVolume* CreateModelBound(const U2TriList* pMesh, uint16 i0, 
		uint16 i1, uint16* pusISplit, U2Line3D& line);

	static U2BoundingVolume* CreateWorldBound();
};

#endif