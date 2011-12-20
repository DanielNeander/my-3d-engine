/**************************************************************************************************
module	:	U2ConvexRegionMgr
Author	:	Yun sangyong
Desc	:   eye point�� �����ϴ� convex region�� ã�´�.
*************************************************************************************************/
#pragma once 
#ifndef U2_CONVEXREGIONMGR_H
#define U2_CONVEXREGIONMGR_H

#include "U2BspNode.h"
#include "U2PortalCuller.h"

class U2Camera;
class U2ConvexRegion;

class U2_3D U2ConvexRegionMgr : public U2BspNode 
{
	DECLARE_RTTI;

public:
	U2ConvexRegionMgr();
	virtual ~U2ConvexRegionMgr();

	U2SpatialPtr AttachOutsideScene(U2Spatial* pOutside);
	U2SpatialPtr DetachOutsideScene();
	U2SpatialPtr GetOutsideScene();

	// eye�� �����ϴ� ������ �����Ѵ�. 
	// eye�� �������� ���� �ۿ� ������ NULL�� �����Ѵ�.
	U2ConvexRegion* GetContainingRegion(const D3DXVECTOR3& eye);

protected:
	virtual void GetVisibleSet(U2Culler& culler, bool bNoCull); 

	U2PortalCuller m_culler;
};

U2SmartPointer(U2ConvexRegionMgr);

#endif 

