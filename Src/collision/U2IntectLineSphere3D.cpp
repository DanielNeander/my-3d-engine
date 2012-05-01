#include <U2_3D/src/U23DLibPCH.h>
#include "U2IntectLineSphere3D.h"


U2IntectLineSphere3D::U2IntectLineSphere3D(const U2Line3D& line, 
										   const U2Sphere& sphere)
										   :m_pLine(&line),
										   m_pSphere(&sphere)

{
	m_iCnt = 0;	
}

bool U2IntectLineSphere3D::Test()
{
	D3DXVECTOR3 diff = m_pLine->m_vOrigin - m_pSphere->m_center;
	float dotDiff = D3DXVec3Dot(&diff, &diff);
	float fA0 = dotDiff - m_pSphere->m_fRadius * m_pSphere->m_fRadius;
	float fA1 = D3DXVec3Dot(&m_pLine->m_vDir, &diff);
	float fDiscr = fA1 * fA1 - fA0;
	return fDiscr >= (float)0.0f;
}

bool U2IntectLineSphere3D::Find()
{
	D3DXVECTOR3 diff = m_pLine->m_vOrigin - m_pSphere->m_center;
	float fA0 = D3DXVec3Dot(&diff, &diff) - m_pSphere->m_fRadius * m_pSphere->m_fRadius;
	float fA1 = D3DXVec3Dot(&m_pLine->m_vDir, &diff);
	float fDiscr = fA1 * fA1 - fA0;

	if (fDiscr < 0.0f)
	{
		m_eIntersectionType = IT_EMPTY;
		m_iCnt = 0;
	}
	else if (fDiscr >= ZERO_TOLERANCE)
	{
		float fRoot = U2Math::Sqrt(fDiscr);
		m_afLineT[0] = fA1 - fRoot;
		m_afLineT[1] = fA1 + fRoot;
		m_aPoint[0] = m_pLine->m_vOrigin + m_afLineT[0] * m_pLine->m_vDir;
		m_aPoint[1] = m_pLine->m_vOrigin + m_afLineT[1] * m_pLine->m_vDir;
		m_eIntersectionType = IT_SEGMENT;
		m_iCnt = 2;
	}
	else 
	{
		m_afLineT[0] = -fA1;
		m_aPoint[0] = m_pLine->m_vOrigin + m_afLineT[0] * m_pLine->m_vDir;
		m_iCnt = 1;
		m_eIntersectionType = IT_POINT;
	}

	return m_iCnt > 0;
}

