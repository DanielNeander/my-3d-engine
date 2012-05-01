#include <U2_3D/src/U23DLibPCH.h>
#include "U2IntectLineBox3D.h"

U2IntectLineBox3D::U2IntectLineBox3D(const U2Line3D& line, const U2Box& box)
:m_pLine(&line),m_pBox(&box)
{

}

const U2Line3D& U2IntectLineBox3D::GetLine() const
{
	return *m_pLine;
}


const U2Box& U2IntectLineBox3D::GetBox() const
{
	return *m_pBox;
}


bool U2IntectLineBox3D::Test()
{
	float afAWdU[3], afAWxDdU[3], fRhs;

	D3DXVECTOR3 diff = m_pLine->m_vOrigin - m_pBox->m_vCenter;
	D3DXVECTOR3 wXD;
	D3DXVec3Cross(&wXD, &m_pLine->m_vDir, &diff);

	afAWdU[1] = U2Math::FAbs(D3DXVec3Dot(&m_pLine->m_vDir, &m_pBox->m_avAxis[1]));
	afAWdU[2] = U2Math::FAbs(D3DXVec3Dot(&m_pLine->m_vDir, &m_pBox->m_avAxis[2]));
	afAWxDdU[0] = U2Math::FAbs(D3DXVec3Dot(&wXD, &m_pBox->m_avAxis[0]));
	fRhs = m_pBox->m_afExtent[1] * afAWdU[2] + m_pBox->m_afExtent[2] * afAWdU[1];
	if(afAWxDdU[0] > fRhs)
		return false;
	afAWxDdU[2] = U2Math::FAbs(D3DXVec3Dot(&wXD, &m_pBox->m_avAxis[2]));
	fRhs = m_pBox->m_afExtent[0] * afAWxDdU[1] + m_pBox->m_afExtent[1] * afAWxDdU[0];
	if(afAWxDdU[2] > fRhs)
	{
		return false;
	}

	return true;
}


bool U2IntectLineBox3D::Find()
{
	float fT0 = -FLT_MAX, fT1 = FLT_MAX;
	return DoClipping(fT0, fT1, m_pLine->m_vOrigin, m_pLine->m_vDir, *m_pBox, 
		true, m_uCnt, m_aPoint, m_eIntersectionType);

}

uint32 U2IntectLineBox3D::GetCnt() const
{
	return m_uCnt;
}


const D3DXVECTOR3& U2IntectLineBox3D::GetPoint(int i) const
{
	U2ASSERT(0 <= i && i < m_uCnt);
	return m_aPoint[i];
}

bool U2IntectLineBox3D::DoClipping(float fT0, float fT1, const D3DXVECTOR3& origin, 
					   const D3DXVECTOR3& dir, const U2Box& box, bool bSolid, int& cnt, 
					   D3DXVECTOR3 aPoint[2], int& intectType )
{
	U2ASSERT(fT0 < fT1);

	D3DXVECTOR3 diff = origin - box.m_vCenter;
		
	D3DXVECTOR3 vBOrigin(D3DXVec3Dot(&diff, &box.m_avAxis[0]), 
		D3DXVec3Dot(&diff, &box.m_avAxis[1]), 
		D3DXVec3Dot(&diff, &box.m_avAxis[2]));
	D3DXVECTOR3 vBDir(D3DXVec3Dot(&dir, &box.m_avAxis[0]),
		D3DXVec3Dot(&dir, &box.m_avAxis[1]),
		D3DXVec3Dot(&dir, &box.m_avAxis[2]));

	float fSaveT0 = fT0, fSaveT1 = fT1;
	bool bNotAllClipped = 
		Clip(+vBDir.x, -vBOrigin.x - box.m_afExtent[0],fT0,fT1) &&
		Clip(-vBDir.x, +vBOrigin.x - box.m_afExtent[0],fT0,fT1) &&
		Clip(+vBDir.y, -vBOrigin.y - box.m_afExtent[1],fT0,fT1) &&
		Clip(-vBDir.y, +vBOrigin.y - box.m_afExtent[1],fT0,fT1) &&
		Clip(+vBDir.z, -vBOrigin.z - box.m_afExtent[2],fT0,fT1) &&
		Clip(-vBDir.z, +vBOrigin.z - box.m_afExtent[2],fT0,fT1);

	if(bNotAllClipped && (bSolid || fT0 != fSaveT0 || fT1 != fSaveT1))
	{
		if(fT1 > fT0)
		{
			intectType = IT_SEGMENT;
			cnt = 2;
			aPoint[0] = origin + fT0 * dir;
			aPoint[1] = origin + fT1 * dir;
		}
		else 
		{
			intectType = IT_POINT;
			cnt = 1;
			aPoint[0] = origin + fT0 * dir;
		}

	}
	else 
	{
		cnt = 0;
		intectType = IT_EMPTY;
	}

	return intectType != IT_EMPTY;

}

bool U2IntectLineBox3D::Clip(float fDenom, float fNumer, float& fT0, float& fT1)
{
	// Return value is 'true' if line segment intersects the current test
	// plane.  Otherwise 'false' is returned in which case the line segment
	// is entirely clipped.
	if(fDenom > 0.f)
	{
		if(fNumer > fDenom * fT1)
			return false;

		if(fNumer > fDenom * fT0)
			fT1 = fNumer/fDenom;

		return true;
	}
	else if(fDenom < 0.f) 
	{
		if(fNumer > fDenom * fT0)
			return false;

		if(fNumer > fDenom * fT1)
			fT1 = fNumer / fDenom;

		return true;
	}
	else 
		return fNumer <= 0.f;
}