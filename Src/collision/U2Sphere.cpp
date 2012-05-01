#include <U2_3D/src/U23DLibPCH.h>
#include "U2Sphere.h"


U2Sphere::U2Sphere(const std::vector<D3DXVECTOR3>* points)
{
	U2ASSERT(points->size() > 0);
	std::vector<D3DXVECTOR3>::const_iterator ptIt = points->begin();

	m_fRadius = 0.f;
	m_center = *ptIt++;

	while ( ptIt != points->end() )
	{
		const D3DXVECTOR3& tmp = *ptIt++;
		D3DXVECTOR3 cVec = tmp - m_center;
		float d = D3DXVec3Dot( &cVec, &cVec );
		if ( d > m_fRadius * m_fRadius )
		{
			d = sqrtf(d);
			float r = 0.5f * (d + m_fRadius);
			float scale = (r - m_fRadius) / d;
			m_center = m_center + scale*cVec;
			m_fRadius = r;
		}
	}

}

U2Sphere::U2Sphere(const U2Aabb* box)
{
	D3DXVECTOR3 radiusVec;
	m_center = 0.5f * (box->m_vMax + box->m_vMin);
	radiusVec = box->m_vMax - m_center;
	float len = D3DXVec3Length(&radiusVec);
	m_fRadius = len;
}
