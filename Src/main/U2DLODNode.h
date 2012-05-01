/**************************************************************************************************
module	:	U2DLODNode
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once 
#ifndef U2_DLODNODE_H
#define U2_DLODNODE_H

#include "U2SwitchNode.h"

U2SmartPointer(U2DLODNode);

class U2_3D U2DLODNode : public U2SwitchNode
{
	DECLARE_RTTI;

public:
	U2DLODNode();
	virtual ~U2DLODNode();

	// Center for LOD 
	D3DXVECTOR3& GetLocalCenter();
	const D3DXVECTOR3& GetLocalCenter() const;
	const D3DXVECTOR3& GetWorldCenter() const;

	void SetLocalDistance(int i, float fMinDist, float fMaxDist);
	
	float GetLocalMinDistance(int i) const;
	float GetLocalMaxDistance(int i) const;
	
	float GetWorldMinDistance(int i) const;
	float GetWorldMaxDistance(int i) const;

protected:

	void SelectLOD(const U2Camera* pCamera);

	virtual void GetVisibleSet(U2Culler& culler, bool bNoCull);

	D3DXVECTOR3 m_vLocalLODCenter;
	D3DXVECTOR3 m_vWorldLODCenter;

	U2PrimitiveVec<float> m_localMinDists;
	U2PrimitiveVec<float> m_localMaxDists;
	
	U2PrimitiveVec<float> m_worldMinDists;
	U2PrimitiveVec<float> m_worldMaxDists;
};

#include "U2DLODNode.inl"

#endif
	
