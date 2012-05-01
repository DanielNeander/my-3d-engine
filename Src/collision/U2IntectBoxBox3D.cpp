#include <U2_3D/src/U23DLibPCH.h>
#include "U2IntectBoxBox3D.h"

U2IntectBoxBox3D::U2IntectBoxBox3D(const U2Box& box0, const U2Box& box1)
:m_pBox0(&box0), m_pBox1(&box1)
{

}


const U2Box& U2IntectBoxBox3D::GetBox0() const
{
	return *m_pBox0;
}


const U2Box& U2IntectBoxBox3D::GetBox1() const
{
	return *m_pBox1;
}


bool U2IntectBoxBox3D::Test()
{
	// Cutoff for cosine of angles between box axes.  This is used to catch
	// the cases when at least one pair of axes are parallel.  If this
	// happens, there is no need to test for separation along the
	// Cross(A[i],B[j]) directions.
	const float fCutoff = 1.0f - ZERO_TOLERANCE;
	bool bExistsParallelPair = false;
	int i;

	// convenience variables
	const D3DXVECTOR3* pAxis0s = m_pBox0->m_avAxis;
	const D3DXVECTOR3* pAxis1s = m_pBox1->m_avAxis;
	const float* pExtent0s = m_pBox0->m_afExtent;
	const float* pExtent1s = m_pBox1->m_afExtent;

	// compute difference of box centers, D = C1-C0
	D3DXVECTOR3 centerDiff = m_pBox1->m_vCenter - m_pBox0->m_vCenter;

	float aafC[3][3];     // matrix C = A^T B, c_{ij} = Dot(A_i,B_j)
	float aafAbsC[3][3];  // |c_{ij}|
	float afAD[3];        // Dot(A_i,D)
	float fR0, fR1, fR;   // interval radii and distance between centers
	float fR01;           // = R0 + R1

	// axis C0+t*A0
	for (i = 0; i < 3; i++)
	{
		aafC[0][i] = D3DXVec3Dot(&pAxis0s[0], &pAxis1s[i]);
		aafAbsC[0][i] = U2Math::FAbs(aafC[0][i]);
		if (aafAbsC[0][i] > fCutoff)
		{
			bExistsParallelPair = true;
		}
	}
	afAD[0] = D3DXVec3Dot(&pAxis0s[0],&centerDiff);
	fR = U2Math::FAbs(afAD[0]);
	fR1 = pExtent1s[0]*aafAbsC[0][0]+pExtent1s[1]*aafAbsC[0][1]+pExtent1s[2]*aafAbsC[0][2];
	fR01 = pExtent0s[0] + fR1;
	if (fR > fR01)
	{
		return false;
	}

	// axis C0+t*A1
	for (i = 0; i < 3; i++)
	{
		aafC[1][i] = D3DXVec3Dot(&pAxis0s[1],&pAxis1s[i]);
		aafAbsC[1][i] = U2Math::FAbs(aafC[1][i]);
		if (aafAbsC[1][i] > fCutoff)
		{
			bExistsParallelPair = true;
		}
	}
	afAD[1] = D3DXVec3Dot(&pAxis0s[1],&centerDiff);
	fR = U2Math::FAbs(afAD[1]);
	fR1 = pExtent1s[0]*aafAbsC[1][0]+pExtent1s[1]*aafAbsC[1][1]+pExtent1s[2]*aafAbsC[1][2];
	fR01 = pExtent0s[1] + fR1;
	if (fR > fR01)
	{
		return false;
	}

	// axis C0+t*A2
	for (i = 0; i < 3; i++)
	{
		aafC[2][i] = D3DXVec3Dot(&pAxis0s[2],&pAxis1s[i]);
		aafAbsC[2][i] = U2Math::FAbs(aafC[2][i]);
		if (aafAbsC[2][i] > fCutoff)
		{
			bExistsParallelPair = true;
		}
	}
	afAD[2] = D3DXVec3Dot(&pAxis0s[2],&centerDiff);
	fR = U2Math::FAbs(afAD[2]);
	fR1 = pExtent1s[0]*aafAbsC[2][0]+pExtent1s[1]*aafAbsC[2][1]+pExtent1s[2]*aafAbsC[2][2];
	fR01 = pExtent0s[2] + fR1;
	if (fR > fR01)
	{
		return false;
	}

	// axis C0+t*B0
	fR = U2Math::FAbs(D3DXVec3Dot(&pAxis1s[0],&centerDiff));
	fR0 = pExtent0s[0]*aafAbsC[0][0]+pExtent0s[1]*aafAbsC[1][0]+pExtent0s[2]*aafAbsC[2][0];
	fR01 = fR0 + pExtent1s[0];
	if (fR > fR01)
	{
		return false;
	}

	// axis C0+t*B1
	fR = U2Math::FAbs(D3DXVec3Dot(&pAxis1s[1],&centerDiff));
	fR0 = pExtent0s[0]*aafAbsC[0][1]+pExtent0s[1]*aafAbsC[1][1]+pExtent0s[2]*aafAbsC[2][1];
	fR01 = fR0 + pExtent1s[1];
	if (fR > fR01)
	{
		return false;
	}

	// axis C0+t*B2
	fR = U2Math::FAbs(D3DXVec3Dot(&pAxis1s[2], &centerDiff));
	fR0 = pExtent0s[0]*aafAbsC[0][2]+pExtent0s[1]*aafAbsC[1][2]+pExtent0s[2]*aafAbsC[2][2];
	fR01 = fR0 + pExtent1s[2];
	if (fR > fR01)
	{
		return false;
	}

	// At least one pair of box axes was parallel, so the separation is
	// effectively in 2D where checking the "edge" normals is sufficient for
	// the separation of the boxes.
	if (bExistsParallelPair)
	{
		return true;
	}

	// axis C0+t*A0xB0
	fR = U2Math::FAbs(afAD[2]*aafC[1][0]-afAD[1]*aafC[2][0]);
	fR0 = pExtent0s[1]*aafAbsC[2][0] + pExtent0s[2]*aafAbsC[1][0];
	fR1 = pExtent1s[1]*aafAbsC[0][2] + pExtent1s[2]*aafAbsC[0][1];
	fR01 = fR0 + fR1;
	if (fR > fR01)
	{
		return false;
	}

	// axis C0+t*A0xB1
	fR = U2Math::FAbs(afAD[2]*aafC[1][1]-afAD[1]*aafC[2][1]);
	fR0 = pExtent0s[1]*aafAbsC[2][1] + pExtent0s[2]*aafAbsC[1][1];
	fR1 = pExtent1s[0]*aafAbsC[0][2] + pExtent1s[2]*aafAbsC[0][0];
	fR01 = fR0 + fR1;
	if (fR > fR01)
	{
		return false;
	}

	// axis C0+t*A0xB2
	fR = U2Math::FAbs(afAD[2]*aafC[1][2]-afAD[1]*aafC[2][2]);
	fR0 = pExtent0s[1]*aafAbsC[2][2] + pExtent0s[2]*aafAbsC[1][2];
	fR1 = pExtent1s[0]*aafAbsC[0][1] + pExtent1s[1]*aafAbsC[0][0];
	fR01 = fR0 + fR1;
	if (fR > fR01)
	{
		return false;
	}

	// axis C0+t*A1xB0
	fR = U2Math::FAbs(afAD[0]*aafC[2][0]-afAD[2]*aafC[0][0]);
	fR0 = pExtent0s[0]*aafAbsC[2][0] + pExtent0s[2]*aafAbsC[0][0];
	fR1 = pExtent1s[1]*aafAbsC[1][2] + pExtent1s[2]*aafAbsC[1][1];
	fR01 = fR0 + fR1;
	if (fR > fR01)
	{
		return false;
	}

	// axis C0+t*A1xB1
	fR = U2Math::FAbs(afAD[0]*aafC[2][1]-afAD[2]*aafC[0][1]);
	fR0 = pExtent0s[0]*aafAbsC[2][1] + pExtent0s[2]*aafAbsC[0][1];
	fR1 = pExtent1s[0]*aafAbsC[1][2] + pExtent1s[2]*aafAbsC[1][0];
	fR01 = fR0 + fR1;
	if (fR > fR01)
	{
		return false;
	}

	// axis C0+t*A1xB2
	fR = U2Math::FAbs(afAD[0]*aafC[2][2]-afAD[2]*aafC[0][2]);
	fR0 = pExtent0s[0]*aafAbsC[2][2] + pExtent0s[2]*aafAbsC[0][2];
	fR1 = pExtent1s[0]*aafAbsC[1][1] + pExtent1s[1]*aafAbsC[1][0];
	fR01 = fR0 + fR1;
	if (fR > fR01)
	{
		return false;
	}

	// axis C0+t*A2xB0
	fR = U2Math::FAbs(afAD[1]*aafC[0][0]-afAD[0]*aafC[1][0]);
	fR0 = pExtent0s[0]*aafAbsC[1][0] + pExtent0s[1]*aafAbsC[0][0];
	fR1 = pExtent1s[1]*aafAbsC[2][2] + pExtent1s[2]*aafAbsC[2][1];
	fR01 = fR0 + fR1;
	if (fR > fR01)
	{
		return false;
	}

	// axis C0+t*A2xB1
	fR = U2Math::FAbs(afAD[1]*aafC[0][1]-afAD[0]*aafC[1][1]);
	fR0 = pExtent0s[0]*aafAbsC[1][1] + pExtent0s[1]*aafAbsC[0][1];
	fR1 = pExtent1s[0]*aafAbsC[2][2] + pExtent1s[2]*aafAbsC[2][0];
	fR01 = fR0 + fR1;
	if (fR > fR01)
	{
		return false;
	}

	// axis C0+t*A2xB2
	fR = U2Math::FAbs(afAD[1]*aafC[0][2]-afAD[0]*aafC[1][2]);
	fR0 = pExtent0s[0]*aafAbsC[1][2] + pExtent0s[1]*aafAbsC[0][2];
	fR1 = pExtent1s[0]*aafAbsC[2][1] + pExtent1s[1]*aafAbsC[2][0];
	fR01 = fR0 + fR1;
	if (fR > fR01)
	{
		return false;
	}

	return true;

}


bool U2IntectBoxBox3D::Test(float fTMax, const D3DXVECTOR3& rkVelocity0, 
				  const D3DXVECTOR3& rkVelocity1)
{
	if (rkVelocity0 == rkVelocity1)
	{
		if (Test())
		{
			m_fContactTime = 0.f;
			return true;
		}
		return false;
	}

	// Cutoff for cosine of angles between box axes.  This is used to catch
	// the cases when at least one pair of axes are parallel.  If this
	// happens, there is no need to include the cross-product axes for
	// separation.
	const float fCutoff = 1.0f - ZERO_TOLERANCE;
	bool bExistsParallelPair = false;

	// convenience variables
	const D3DXVECTOR3* pAxis0s = m_pBox0->m_avAxis;
	const D3DXVECTOR3* pAxis1s = m_pBox1->m_avAxis;
	const float* pExtent0s = m_pBox0->m_afExtent;
	const float* pExtent1s = m_pBox1->m_afExtent;
	D3DXVECTOR3 cneterDiff = m_pBox1->m_vCenter - m_pBox0->m_vCenter;
	D3DXVECTOR3 velDiff = rkVelocity1 - rkVelocity0;
	float aafC[3][3];     // matrix C = A^T B, c_{ij} = Dot(A_i,B_j)
	float aafAbsC[3][3];  // |c_{ij}|
	float afAD[3];        // Dot(A_i,D)
	float afAW[3];        // Dot(A_i,W)
	float fMin0, fMax0, fMin1, fMax1, fCenter, fRadius, fSpeed;
	int i, j;

	m_fContactTime = 0.f;
	float fTLast = FLT_MAX;

	// axes C0+t*A[i]
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			aafC[i][j] = D3DXVec3Dot(&pAxis0s[i],&pAxis1s[j]);
			aafAbsC[i][j] = U2Math::FAbs(aafC[i][j]);
			if (aafAbsC[i][j] > fCutoff)
			{
				bExistsParallelPair = true;
			}
		}
		afAD[i] = D3DXVec3Dot(&pAxis0s[i],&cneterDiff);
		afAW[i] = D3DXVec3Dot(&pAxis0s[i],&velDiff);
		fMin0 = -pExtent0s[i];
		fMax0 = +pExtent0s[i];
		fRadius = pExtent1s[0]*aafAbsC[i][0] + pExtent1s[1]*aafAbsC[i][1] + 
			pExtent1s[2]*aafAbsC[i][2];
		fMin1 = afAD[i] - fRadius;
		fMax1 = afAD[i] + fRadius;
		fSpeed = afAW[i];
		if (IsSeparated(fMin0,fMax0,fMin1,fMax1,fSpeed,fTMax,m_fContactTime,
			fTLast))
		{
			return false;
		}
	}

	// axes C0+t*B[i]
	for (i = 0; i < 3; i++)
	{
		fRadius = pExtent0s[0]*aafAbsC[0][i] + pExtent0s[1]*aafAbsC[1][i] +
			pExtent0s[2]*aafAbsC[2][i];
		fMin0 = -fRadius;
		fMax0 = +fRadius;
		fCenter = D3DXVec3Dot(&pAxis1s[i],&cneterDiff);
		fMin1 = fCenter - pExtent1s[i];
		fMax1 = fCenter + pExtent1s[i];
		fSpeed = D3DXVec3Dot(&velDiff,&pAxis1s[i]);
		if (IsSeparated(fMin0,fMax0,fMin1,fMax1,fSpeed,fTMax,m_fContactTime,
			fTLast))
		{
			return false;
		}
	}

	// At least one pair of box axes was parallel, so the separation is
	// effectively in 2D where checking the "edge" normals is sufficient for
	// the separation of the boxes.
	if (bExistsParallelPair)
	{
		return true;
	}

	// axis C0+t*A0xB0
	fRadius = pExtent0s[1]*aafAbsC[2][0] + pExtent0s[2]*aafAbsC[1][0];
	fMin0 = -fRadius;
	fMax0 = +fRadius;
	fCenter = afAD[2]*aafC[1][0] - afAD[1]*aafC[2][0];
	fRadius = pExtent1s[1]*aafAbsC[0][2] + pExtent1s[2]*aafAbsC[0][1];
	fMin1 = fCenter - fRadius;
	fMax1 = fCenter + fRadius;
	fSpeed = afAW[2]*aafC[1][0] - afAW[1]*aafC[2][0];
	if (IsSeparated(fMin0,fMax0,fMin1,fMax1,fSpeed,fTMax,m_fContactTime,
		fTLast))
	{
		return false;
	}

	// axis C0+t*A0xB1
	fRadius = pExtent0s[1]*aafAbsC[2][1] + pExtent0s[2]*aafAbsC[1][1];
	fMin0 = -fRadius;
	fMax0 = +fRadius;
	fCenter = afAD[2]*aafC[1][1] - afAD[1]*aafC[2][1];
	fRadius = pExtent1s[0]*aafAbsC[0][2] + pExtent1s[2]*aafAbsC[0][0];
	fMin1 = fCenter - fRadius;
	fMax1 = fCenter + fRadius;
	fSpeed = afAW[2]*aafC[1][1] - afAW[1]*aafC[2][1];
	if (IsSeparated(fMin0,fMax0,fMin1,fMax1,fSpeed,fTMax,m_fContactTime,
		fTLast))
	{
		return false;
	}

	// axis C0+t*A0xB2
	fRadius = pExtent0s[1]*aafAbsC[2][2] + pExtent0s[2]*aafAbsC[1][2];
	fMin0 = -fRadius;
	fMax0 = +fRadius;
	fCenter = afAD[2]*aafC[1][2] - afAD[1]*aafC[2][2];
	fRadius = pExtent1s[0]*aafAbsC[0][1] + pExtent1s[1]*aafAbsC[0][0];
	fMin1 = fCenter - fRadius;
	fMax1 = fCenter + fRadius;
	fSpeed = afAW[2]*aafC[1][2] - afAW[1]*aafC[2][2];
	if (IsSeparated(fMin0,fMax0,fMin1,fMax1,fSpeed,fTMax,m_fContactTime,
		fTLast))
	{
		return false;
	}

	// axis C0+t*A1xB0
	fRadius = pExtent0s[0]*aafAbsC[2][0] + pExtent0s[2]*aafAbsC[0][0];
	fMin0 = -fRadius;
	fMax0 = +fRadius;
	fCenter = afAD[0]*aafC[2][0] - afAD[2]*aafC[0][0];
	fRadius = pExtent1s[1]*aafAbsC[1][2] + pExtent1s[2]*aafAbsC[1][1];
	fMin1 = fCenter - fRadius;
	fMax1 = fCenter + fRadius;
	fSpeed = afAW[0]*aafC[2][0] - afAW[2]*aafC[0][0];
	if (IsSeparated(fMin0,fMax0,fMin1,fMax1,fSpeed,fTMax,m_fContactTime,
		fTLast))
	{
		return false;
	}

	// axis C0+t*A1xB1
	fRadius = pExtent0s[0]*aafAbsC[2][1] + pExtent0s[2]*aafAbsC[0][1];
	fMin0 = -fRadius;
	fMax0 = +fRadius;
	fCenter = afAD[0]*aafC[2][1] - afAD[2]*aafC[0][1];
	fRadius = pExtent1s[0]*aafAbsC[1][2] + pExtent1s[2]*aafAbsC[1][0];
	fMin1 = fCenter - fRadius;
	fMax1 = fCenter + fRadius;
	fSpeed = afAW[0]*aafC[2][1] - afAW[2]*aafC[0][1];
	if (IsSeparated(fMin0,fMax0,fMin1,fMax1,fSpeed,fTMax,m_fContactTime,
		fTLast))
	{
		return false;
	}

	// axis C0+t*A1xB2
	fRadius = pExtent0s[0]*aafAbsC[2][2] + pExtent0s[2]*aafAbsC[0][2];
	fMin0 = -fRadius;
	fMax0 = +fRadius;
	fCenter = afAD[0]*aafC[2][2] - afAD[2]*aafC[0][2];
	fRadius = pExtent1s[0]*aafAbsC[1][1] + pExtent1s[1]*aafAbsC[1][0];
	fMin1 = fCenter - fRadius;
	fMax1 = fCenter + fRadius;
	fSpeed = afAW[0]*aafC[2][2] - afAW[2]*aafC[0][2];
	if (IsSeparated(fMin0,fMax0,fMin1,fMax1,fSpeed,fTMax,m_fContactTime,
		fTLast))
	{
		return false;
	}

	// axis C0+t*A2xB0
	fRadius = pExtent0s[0]*aafAbsC[1][0] + pExtent0s[1]*aafAbsC[0][0];
	fMin0 = -fRadius;
	fMax0 = +fRadius;
	fCenter = afAD[1]*aafC[0][0] - afAD[0]*aafC[1][0];
	fRadius = pExtent1s[1]*aafAbsC[2][2] + pExtent1s[2]*aafAbsC[2][1];
	fMin1 = fCenter - fRadius;
	fMax1 = fCenter + fRadius;
	fSpeed = afAW[1]*aafC[0][0] - afAW[0]*aafC[1][0];
	if (IsSeparated(fMin0,fMax0,fMin1,fMax1,fSpeed,fTMax,m_fContactTime,
		fTLast))
	{
		return false;
	}

	// axis C0+t*A2xB1
	fRadius = pExtent0s[0]*aafAbsC[1][1] + pExtent0s[1]*aafAbsC[0][1];
	fMin0 = -fRadius;
	fMax0 = +fRadius;
	fCenter = afAD[1]*aafC[0][1] - afAD[0]*aafC[1][1];
	fRadius = pExtent1s[0]*aafAbsC[2][2] + pExtent1s[2]*aafAbsC[2][0];
	fMin1 = fCenter - fRadius;
	fMax1 = fCenter + fRadius;
	fSpeed = afAW[1]*aafC[0][1] - afAW[0]*aafC[1][1];
	if (IsSeparated(fMin0,fMax0,fMin1,fMax1,fSpeed,fTMax,m_fContactTime,
		fTLast))
	{
		return false;
	}

	// axis C0+t*A2xB2
	fRadius = pExtent0s[0]*aafAbsC[1][2] + pExtent0s[1]*aafAbsC[0][2];
	fMin0 = -fRadius;
	fMax0 = +fRadius;
	fCenter = afAD[1]*aafC[0][2] - afAD[0]*aafC[1][2];
	fRadius = pExtent1s[0]*aafAbsC[2][1] + pExtent1s[1]*aafAbsC[2][0];
	fMin1 = fCenter - fRadius;
	fMax1 = fCenter + fRadius;
	fSpeed = afAW[1]*aafC[0][2] - afAW[0]*aafC[1][2];
	if (IsSeparated(fMin0,fMax0,fMin1,fMax1,fSpeed,fTMax,m_fContactTime,
		fTLast))
	{
		return false;
	}

	return true;

}


bool U2IntectBoxBox3D::IsSeparated (float fMin0, float fMax0, float fMin1, float fMax1,
				  float fSpeed, float fTMax, float& rfTFirst, float& rfTLast)
{
	float fInvSpeed, fT;

	if (fMax1 < fMin0) // box1 initially on left of box0
	{
		if (fSpeed <= 0.0f)
		{
			// The projection intervals are moving apart.
			return true;
		}
		fInvSpeed = (1.0f)/fSpeed;

		fT = (fMin0 - fMax1)*fInvSpeed;
		if (fT > rfTFirst)
		{
			rfTFirst = fT;
		}

		if (rfTFirst > fTMax)
		{
			// Intervals do not intersect during the specified time.
			return true;
		}

		fT = (fMax0 - fMin1)*fInvSpeed;
		if (fT < rfTLast)
		{
			rfTLast = fT;
		}

		if (rfTFirst > rfTLast)
		{
			// Physically inconsistent times--the objects cannot intersect.
			return true;
		}
	}
	else if (fMax0 < fMin1) // box1 initially on right of box0
	{
		if (fSpeed >= 0.f)
		{
			// The projection intervals are moving apart.
			return true;
		}
		fInvSpeed = 1.0f/fSpeed;

		fT = (fMax0 - fMin1)*fInvSpeed;
		if (fT > rfTFirst)
		{
			rfTFirst = fT;
		}

		if (rfTFirst > fTMax)
		{
			// Intervals do not intersect during the specified time.
			return true;
		}

		fT = (fMin0 - fMax1)*fInvSpeed;
		if (fT < rfTLast)
		{
			rfTLast = fT;
		}

		if (rfTFirst > rfTLast)
		{
			// Physically inconsistent times--the objects cannot intersect.
			return true;
		}
	}
	else // box0 and box1 initially overlap
	{
		if (fSpeed > 0.f)
		{
			fT = (fMax0 - fMin1)/fSpeed;
			if (fT < rfTLast)
			{
				rfTLast = fT;
			}

			if (rfTFirst > rfTLast)
			{
				// Physically inconsistent times--the objects cannot
				// intersect.
				return true;
			}
		}
		else if (fSpeed < 0.f)
		{
			fT = (fMin0 - fMax1)/fSpeed;
			if (fT < rfTLast)
			{
				rfTLast = fT;
			}

			if (rfTFirst > rfTLast)
			{
				// Physically inconsistent times--the objects cannot
				// intersect.
				return true;
			}
		}
	}

	return false;

}