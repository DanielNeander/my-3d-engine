#include <U2_3D/Src/U23DLibPCH.h>

#include "U2ContainSphere3D.h"

//-------------------------------------------------------------------------------------------------
bool InU2Sphere(const D3DXVECTOR3& rkPoint, const U2Sphere& rkSphere)
{
	D3DXVECTOR3 diff = rkPoint - rkSphere.m_center;
	return D3DXVec3Length(&diff) <= rkSphere.m_fRadius;

}

//-------------------------------------------------------------------------------------------------
U2Sphere MergeU2Spheres(const U2Sphere& sphere0, const U2Sphere& sphere1)
{
	D3DXVECTOR3 diff = sphere1.m_center - sphere0.m_center;
	float fSqrLen = D3DXVec3LengthSq(&diff);
	float fRadiusDiff = sphere1.m_fRadius - sphere0.m_fRadius;
	float fRadiusDiffSqr = fRadiusDiff * fRadiusDiff;

	if(fRadiusDiffSqr >= fSqrLen)
	{		
		return (fRadiusDiff >= 0.0f ? sphere1 : sphere0);
	}

	float fLength = U2Math::Sqrt(fSqrLen);
	U2Sphere sphere;

	if(fLength > ZERO_TOLERANCE)
	{
		float fCoeff = (fLength + fRadiusDiff) / (2.0f *fLength);
		sphere.m_center = sphere0.m_center + fCoeff * diff;
	}
	else 
	{
		sphere.m_center = sphere0.m_center;
	}

	sphere.m_fRadius = 0.5f * (fLength * sphere0.m_fRadius + sphere1.m_fRadius);

	return sphere;
}
