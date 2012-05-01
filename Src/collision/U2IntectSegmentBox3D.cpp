#include <U2_3D/src/U23DLibPCH.h>
#include <U2_3D/src/Collision/U2IntectSegmentBox3D.h>

U2IntectSegmentBox3D::U2IntectSegmentBox3D(const U2Segment3D& line, 
										   const U2Box& box, bool bSolid)
:m_pSegment(&line),m_pBox(&box),m_bSolid(bSolid)
{

}

const U2Segment3D& U2IntectSegmentBox3D::GetLine() const
{
	return *m_pSegment;
}


const U2Box& U2IntectSegmentBox3D::GetBox() const
{
	return *m_pBox;
}


bool U2IntectSegmentBox3D::Test()
{

	float aAWdU[3], aADdU[3], aAWxDdU[3], fRhs;

	D3DXVECTOR3 diff = m_pSegment->m_vOrigin - m_pBox->m_vCenter;

	aAWdU[0] = U2Math::FAbs(D3DXVec3Dot(&m_pSegment->m_vDir, &m_pBox->m_avAxis[0]));	 
	aADdU[0] = U2Math::FAbs(D3DXVec3Dot(&diff, &m_pBox->m_avAxis[0]));	 
	fRhs = m_pBox->m_afExtent[0] + m_pSegment->m_fExtent * aAWdU[0];
	if(aADdU[0] > fRhs)
		return false;

	aAWdU[1] = U2Math::FAbs(D3DXVec3Dot(&m_pSegment->m_vDir, &m_pBox->m_avAxis[1]));	 
	aADdU[1] = U2Math::FAbs(D3DXVec3Dot(&diff, &m_pBox->m_avAxis[1]));	 
	fRhs = m_pBox->m_afExtent[1] + m_pSegment->m_fExtent * aAWdU[1];
	if(aADdU[1] > fRhs)
		return false;

	aAWdU[2] = U2Math::FAbs(D3DXVec3Dot(&m_pSegment->m_vDir, &m_pBox->m_avAxis[2]));	 
	aADdU[2] = U2Math::FAbs(D3DXVec3Dot(&diff, &m_pBox->m_avAxis[2]));	 
	fRhs = m_pBox->m_afExtent[2] + m_pSegment->m_fExtent * aAWdU[2];
	if(aADdU[2] > fRhs)
		return false;

	D3DXVECTOR3 WxD;
	D3DXVec3Cross(&WxD, &m_pSegment->m_vDir, &diff);

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


bool U2IntectSegmentBox3D::Find()
{
	float fT0 = -m_pSegment->m_fExtent, fT1 = m_pSegment->m_fExtent;
	return U2IntectLineBox3D::DoClipping(fT0, fT1, m_pSegment->m_vOrigin,
		m_pSegment->m_vDir, *m_pBox, m_bSolid, m_uCnt, m_aPoint, 
		m_eIntersectionType);
}

uint32 U2IntectSegmentBox3D::GetCnt() const
{
	return m_uCnt;
}


const D3DXVECTOR3& U2IntectSegmentBox3D::GetPoint(int i) const
{
	U2ASSERT(0 <= i && i < m_uCnt);
	return m_aPoint[i];
}

