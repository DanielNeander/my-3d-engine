#include <U2_3D/src/U23DLibPCH.h>
#include <U2_3D/src/collision/U2ContainSphere3D.h>
#include "U2SphereBV.h"


U2BoundingVolume* U2BoundingVolume::Create ()
{
	return U2_NEW U2SphereBV;
}

U2SphereBV::U2SphereBV()
:m_sphere(VECTOR3_ZERO, 0.0f)
{

}

U2SphereBV::U2SphereBV(const U2Sphere& sphere)
:m_sphere(sphere)
{

}

//-------------------------------------------------------------------------------------------------
void U2SphereBV::ComputeFromData (const U2MeshData* pModelData)
{
	if(pModelData)
	{
		uint32 activeVertCnt = pModelData->GetActiveVertexCount();

		m_sphere.m_center = VECTOR3_ZERO;
		m_sphere.m_fRadius = 0.0f;
		uint32 i;
		for(i=0; i < activeVertCnt; ++i)
		{
			m_sphere.m_center += pModelData->GetVertices()[i];
		}
		m_sphere.m_center /= (float)activeVertCnt;

		for(i=0; i < activeVertCnt; ++i)
		{
			D3DXVECTOR3 diff = pModelData->GetVertices()[i] - m_sphere.m_center;
			float fRadiusSqr = D3DXVec3LengthSq(&diff);
			if(fRadiusSqr > m_sphere.m_fRadius)
			{
				m_sphere.m_fRadius = fRadiusSqr;
			}
		}

		m_sphere.m_fRadius = U2Math::Sqrt(m_sphere.m_fRadius);
	}
}


//-------------------------------------------------------------------------------------------------
void U2SphereBV::TransformBy (const U2Transform& rkTransform,
						  U2BoundingVolume* pkResult)
{
	U2Sphere& target = ((U2SphereBV*)pkResult)->m_sphere;
	target.m_center = rkTransform.Apply(m_sphere.m_center);
	target.m_fRadius = rkTransform.GetNorm() * m_sphere.m_fRadius;
}

//-------------------------------------------------------------------------------------------------
int U2SphereBV::WhichSide (const U2Plane& rkPlane) const
{
	float fDistance = rkPlane.GetDistance(m_sphere.m_center);

	if(fDistance <= -m_sphere.m_fRadius)
	{
		return -1;
	}

	if(fDistance >= m_sphere.m_fRadius)
	{
		return +1;
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
bool U2SphereBV::TestIntersection (const D3DXVECTOR3& rkOrigin,
							   const D3DXVECTOR3& rkDirection, float fTMin, float fTMax) const
{
	if(fTMin == -FLT_MAX)
	{
		U2Line3D line(rkOrigin, rkDirection);
		return U2IntectLineSphere3D(line, m_sphere).Test();
	}

	if(fTMax == FLT_MAX)
	{
		U2Ray3D ray(rkOrigin, rkDirection);
		return U2IntectRaySphere3D(ray, m_sphere).Test();	
	}

	U2Segment3D segment;
	segment.m_fExtent = 0.5f * fTMax;
	segment.m_vOrigin = rkOrigin + segment.m_fExtent * rkDirection;
	segment.m_vDir = rkDirection;
	return true;
}

// Test for intersection of the two bounds.
bool U2SphereBV::TestIntersection (const U2BoundingVolume* pkInput) const
{
	return true;
}

// Make a copy of the bounding volume.
void U2SphereBV::CopyFrom (const U2BoundingVolume* pkInput)
{
	m_sphere = ((U2SphereBV*)pkInput)->m_sphere;
}

// Change the current bounding volume so that it contains the input
// bounding volume as well as its old bounding volume.
void U2SphereBV::GrowToContain (const U2BoundingVolume* pkInput)
{
	m_sphere = MergeU2Spheres(m_sphere, ((U2SphereBV*)pkInput)->m_sphere);
}

// test for containment of a point
bool U2SphereBV::Contains (const D3DXVECTOR3& rkPoint) const
{
	return InU2Sphere(rkPoint, m_sphere);
}