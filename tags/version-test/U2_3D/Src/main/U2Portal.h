/**************************************************************************************************
module	:	U2Portal
Author	:	Yun sangyong
Desc	:	
*************************************************************************************************/
#pragma once 
#ifndef U2_PORTAL_H
#define U2_PORTAL_H

#include "U2Object.h"

U2SmartPointer(U2Portal);

class U2ConvexRegion;
class U2Culler;

class U2_3D U2Portal : public U2Object
{
	DECLARE_RTTI;
public :
	U2Portal(uint32 uVertCnt, D3DXVECTOR3* pLocalVerts, 
		const U2Plane& localPlane, U2ConvexRegion* pAdajacentRegion,
		bool bOpen);

	virtual ~U2Portal();

	U2ConvexRegion*& AdjacentRegion();
	bool& Open();

protected:
	U2Portal();

	void UpdateWorldData(const U2Transform& worldTM);

	bool ReduceFrustum(const U2Culler& culler, U2Frustum& reduceFrustum);
	void GetVisibleSet(U2Culler& culler, bool bNoCull);

	uint32 m_uVertCnt;
	D3DXVECTOR3* m_pLocalVerts;
	D3DXVECTOR3* m_pWorldVerts;
	U2Plane m_localPlane, m_worldPlane;

	U2ConvexRegion* m_pAdjacentRegion;

	bool m_bOpen;

	friend class U2ConvexRegion;
	
};



#endif