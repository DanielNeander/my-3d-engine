#include <U2_3D/src/U23DLibPCH.h>
#include "U2Plane.h"



U2Plane::U2Plane()
{

}

U2Plane::U2Plane(const U2Plane& in)
{
	m_vNormal = in.m_vNormal;
	m_fDistance = in.m_fDistance;
}


U2Plane::U2Plane(const D3DXVECTOR3& pt0, const D3DXVECTOR3& pt1, const D3DXVECTOR3& pt2)
{
	D3DXVECTOR3 edge1 = pt1 - pt0;
	D3DXVECTOR3 edge2 = pt2- pt0;
	D3DXVec3Cross(&m_vNormal, &edge1, &edge2);
	D3DXVec3Normalize(&m_vNormal, &m_vNormal);
}

U2Plane::U2Plane(const D3DXVECTOR3& normal, float distance)
{
	m_vNormal = normal;
	m_fDistance = distance;
}

U2Plane::U2Plane(const D3DXVECTOR3& pt, const D3DXVECTOR3& normal)
{
	m_vNormal = normal;
	m_fDistance = D3DXVec3Dot(&normal, &pt);
}

U2Plane::~U2Plane()
{

}


