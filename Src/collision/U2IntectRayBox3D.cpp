#include <U2_3D/src/U23DLibPCH.h>
#include "U2IntectRayBox3D.h"
#include "U2IntectLineBox3D.h"

U2IntectRayBox3D::U2IntectRayBox3D(const U2Ray3D& line, const U2Box& box)
:m_pRay(&line),m_pBox(&box)
{

}

const U2Ray3D& U2IntectRayBox3D::GetLine() const
{
	return *m_pRay;
}


const U2Box& U2IntectRayBox3D::GetBox() const
{
	return *m_pBox;
}


bool U2IntectRayBox3D::Test()
{

	float aWdU[3], aAWdU[3], aDdU[3], aADdU[3], aAWxDdU[3], fRhs;

	D3DXVECTOR3 diff = m_pRay->m_vOrigin - m_pBox->m_vCenter;

	aWdU[0] = D3DXVec3Dot(&m_pRay->m_vDir, &m_pBox->m_avAxis[0]);
	aAWdU[0] = U2Math::FAbs(aWdU[0]);
	aDdU[0] = D3DXVec3Dot(&diff, &m_pBox->m_avAxis[0]);
	aADdU[0] = U2Math::FAbs(aDdU[0]);
	if(aADdU[0] > m_pBox->m_afExtent[0] && aDdU[0] * aWdU[0] >= 0.f)
		return false;

	aWdU[1] = D3DXVec3Dot(&m_pRay->m_vDir, &m_pBox->m_avAxis[1]);
	aAWdU[1] = U2Math::FAbs(aWdU[1]);
	aDdU[1] = D3DXVec3Dot(&diff, &m_pBox->m_avAxis[1]);
	aADdU[1] = U2Math::FAbs(aDdU[1]);
	if(aADdU[1] > m_pBox->m_afExtent[1] && aDdU[1] * aWdU[1] >= 0.f)
		return false;

	aWdU[2] = D3DXVec3Dot(&m_pRay->m_vDir, &m_pBox->m_avAxis[2]);
	aAWdU[2] = U2Math::FAbs(aWdU[2]);
	aDdU[2] = D3DXVec3Dot(&diff, &m_pBox->m_avAxis[2]);
	aADdU[2] = U2Math::FAbs(aDdU[2]);
	if(aADdU[2] > m_pBox->m_afExtent[2] && aDdU[2] * aWdU[2] >= 0.f)
		return false;

	D3DXVECTOR3 WxD;
	D3DXVec3Cross(&WxD, &m_pRay->m_vDir, &diff);
	
	aAWxDdU[0] = U2Math::FAbs(D3DXVec3Dot(&WxD, &m_pBox->m_avAxis[0]));
	fRhs = m_pBox->m_afExtent[1] *aAWdU[2] + m_pBox->m_afExtent[2]*aAWdU[1];
	if(aAWxDdU[0] > fRhs)
		return false;

	aAWxDdU[1] = U2Math::FAbs(D3DXVec3Dot(&WxD, &m_pBox->m_avAxis[1]));
	fRhs = m_pBox->m_afExtent[0] *aAWdU[2] + m_pBox->m_afExtent[2]*aAWdU[0];
	if(aAWxDdU[1] > fRhs)
		return false;

	aAWxDdU[2] = U2Math::FAbs(D3DXVec3Dot(&WxD, &m_pBox->m_avAxis[2]));
	fRhs = m_pBox->m_afExtent[0] *aAWdU[1] + m_pBox->m_afExtent[1]*aAWdU[0];
	if(aAWxDdU[2] > fRhs)
		return false;

	return true;
}


bool U2IntectRayBox3D::Find()
{
	float fT0 = 0.f, fT1 = FLT_MAX;
	return U2IntectLineBox3D::DoClipping(fT0, fT1, m_pRay->m_vOrigin,
		m_pRay->m_vDir, *m_pBox, true, m_uCnt, m_aPoint, 
		m_eIntersectionType);
}

uint32 U2IntectRayBox3D::GetCnt() const
{
	return m_uCnt;
}


const D3DXVECTOR3& U2IntectRayBox3D::GetPoint(int i) const
{
	U2ASSERT(0 <= i && i < m_uCnt);
	return m_aPoint[i];
}

