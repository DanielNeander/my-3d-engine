/**************************************************************************************************
module	:	U2BspNode
Author	:	Yun sangyong
Desc	:	Child 의 수는 3개로 고정되어 있다.(Front Plane, Back Plane, Mesh)
			child 0 index : Front Plane 
			1 index : Geometry that is coplanar with seperating planes
			2 index : Back Plane 
*************************************************************************************************/
#pragma once 
#ifndef U2_BSPNODE_H
#define U2_BSPNODE_H

#include "U2Node.h"

U2SmartPointer(U2BspNode);

class U2_3D U2BspNode : public U2Node
{
	DECLARE_RTTI;

public:
	U2BspNode();
	U2BspNode(const U2Plane& localPlane);
	virtual ~U2BspNode();

	U2SpatialPtr AttachFrontChild(U2Spatial* pChild);
	U2SpatialPtr AttachCoplanarChild(U2Spatial* pChild);
	U2SpatialPtr AttachBackChild(U2Spatial* pChild);

	U2SpatialPtr DetachFrontChild();
	U2SpatialPtr DetachCoplanarChild();
	U2SpatialPtr DetachBackChild();
	U2SpatialPtr GetFrontChild();
	U2SpatialPtr GetCoplanraChild();
	U2SpatialPtr GetBackChild();

	U2Plane& LocalPlane();
	const U2Plane& GetLocalPlane() const;
	const U2Plane& GetWorldPlane() const;

	U2Spatial* GetContainingNode(const D3DXVECTOR3& point);

protected:

	virtual void UpdateWorldData(float fAppTime);

	virtual void GetVisibleSet(U2Culler& culler, bool bNoCull);

	U2Plane m_localPlane;
	U2Plane m_worldPlane;
};

#include "U2BspNode.inl"

#endif

	