#include <U2_3D/src/U23DLibPCH.h>
#include "U2IntectRaySphere3D.h"


U2IntectRaySphere3D::U2IntectRaySphere3D(const U2Ray3D& ray, const U2Sphere& sphere)
:m_pRay(&ray),
m_pSphere(&sphere)
{
	m_iCnt = 0;
}

bool U2IntectRaySphere3D::Test ()
{
	D3DXVECTOR3 diff = m_pRay->m_vOrigin - m_pSphere->m_center;	
	float fA0 = D3DXVec3Dot(&diff, &diff) - m_pSphere->m_fRadius * m_pSphere->m_fRadius;
	if (fA0 <= 0.0f)
	{
		// P is inside the sphere
		return true;
	}
	// else: P is outside the sphere

	float fA1 = D3DXVec3Dot(&m_pRay->m_vDir, &diff);
	if (fA1 >= 0.0f)
	{
		return false;
	}

	// quadratic has a real root if discriminant is nonnegative
	return fA1 * fA1 >= fA0;
}

bool U2IntectRaySphere3D::Find ()
{
	D3DXVECTOR3 diff = m_pRay->m_vOrigin - m_pSphere->m_center;
	float fA0 = D3DXVec3Dot(&diff, &diff) - m_pSphere->m_fRadius * m_pSphere->m_fRadius;
	float fA1, fDiscr, fRoot;
	if (fA0 <= 0.0f)
	{
		// P is inside the sphere
		fA1 = D3DXVec3Dot(&m_pRay->m_vDir, &diff);
		fDiscr = fA1 * fA1 - fA0;
		fRoot = U2Math::Sqrt(fDiscr);
		m_afRayT[0] = -fA1 + fRoot;
		m_aPoint[0] = m_pRay->m_vOrigin + m_afRayT[0] * m_pRay->m_vDir;
		m_eIntersectionType = IT_POINT;
		m_iCnt = 1;
		return true;
	}
	// else: P is outside the sphere

	fA1 = D3DXVec3Dot(&m_pRay->m_vDir, &diff);
	if (fA1 >= 0.0f)
	{
		m_eIntersectionType = IT_EMPTY;
		m_iCnt = 0;
		return false;
	}

	fDiscr = fA1 * fA1 - fA0;
	if (fDiscr < 0.0f)
	{
		m_eIntersectionType = IT_EMPTY;
		m_iCnt = 0;
	}
	else if (fDiscr >= ZERO_TOLERANCE)
	{
		fRoot = U2Math::Sqrt(fDiscr);
		m_afRayT[0] = -fA1 - fRoot;
		m_afRayT[1] = -fA1 + fRoot;
		m_aPoint[0] = m_pRay->m_vOrigin + m_afRayT[0] * m_pRay->m_vDir;
		m_aPoint[1] = m_pRay->m_vOrigin + m_afRayT[1] * m_pRay->m_vDir;
		m_eIntersectionType = IT_SEGMENT;
		m_iCnt = 2;
	}
	else 
	{
		m_afRayT[0] = -fA1;
		m_aPoint[0] = m_pRay->m_vOrigin + m_afRayT[0] * m_pRay->m_vDir;
		m_eIntersectionType = IT_POINT;
		m_iCnt = 1;
	}

	return m_iCnt > 0;
}


