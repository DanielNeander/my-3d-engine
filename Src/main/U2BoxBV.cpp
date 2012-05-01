#include <U2_3D/src/U23DLibPCH.h>
#include "U2BoxBV.h"
#include <U2_3D/src/Collision/U2IntectSegmentBox3D.h>
#include <U2_3D/src/Collision/U2IntectLineBox3D.h>
#include <U2_3D/src/Collision/U2IntectRayBox3D.h>
#include <U2_3D/src/Collision/U2IntectBoxBox3D.h>
#include <U2_3D/src/Collision/U2IntectSegmentBox3D.h>
#include <U2_3D/src/Collision/U2ContainBox3D.h>


IMPLEMENT_RTTI(U2BoxBV, U2BoundingVolume);

//U2BoundingVolume* U2BoundingVolume::Create ()
//{
//	return U2_NEW U2BoxBV;
//}

U2BoxBV::U2BoxBV()
:m_box(VECTOR3_ZERO, UNIT_X, UNIT_Y, UNIT_Z, 1.f, 1.f, 1.f)
{

}


U2BoxBV::U2BoxBV(const U2Box& rhs)
: m_box(rhs)
{

}


U2BoxBV::~U2BoxBV()
{

}

int U2BoxBV::GetBVType () const
{
	return U2BoundingVolume::BV_BOX;
}
//----------------------------------------------------------------------------
void U2BoxBV::SetCenter (const D3DXVECTOR3& rkCenter)
{
	m_box.m_vCenter = rkCenter;
}
//----------------------------------------------------------------------------
 D3DXVECTOR3 U2BoxBV::GetCenter () const
{
	return m_box.m_vCenter;
}
//----------------------------------------------------------------------------
 void U2BoxBV::SetRadius (float fRadius)
 {
	 m_box.m_afExtent[0] = fRadius;
	 m_box.m_afExtent[1] = fRadius;
	 m_box.m_afExtent[2] = fRadius;
 }


float U2BoxBV::GetRadius () const
{
	float fRadius = m_box.m_afExtent[0];
	if(fRadius < m_box.m_afExtent[1])
	{
		fRadius = m_box.m_afExtent[1];
	}
	if(fRadius < m_box.m_afExtent[2])
	{
		fRadius = m_box.m_afExtent[2];
	}

	return fRadius;	
}


void U2BoxBV::ComputeFromData (const U2MeshData* pkVBuffer)
{
	
}

void U2BoxBV::TransformBy (const U2Transform& rkTransform,
						  U2BoundingVolume* pkResult)
{
	U2Box& target = ((U2BoxBV*)pkResult)->m_box;
	target.m_vCenter = rkTransform.Apply(m_box.m_vCenter);
	for(uint32 i=0; i < 3; ++i)
	{
		D3DXVec3TransformCoord(&target.m_avAxis[i],
			&m_box.m_avAxis[i], &rkTransform.GetRot());
		target.m_afExtent[i] = rkTransform.GetNorm() * m_box.m_afExtent[i];	
	}
}

int U2BoxBV::WhichSide (const U2Plane& rkPlane) const
{
	float fProjCenter = ::D3DXVec3Dot(&rkPlane.m_vNormal, &m_box.m_vCenter) -
		rkPlane.m_fDistance;

	float fAbs0 = U2Math::FAbs(::D3DXVec3Dot(&rkPlane.m_vNormal, &m_box.m_avAxis[0]));
	float fAbs1 = U2Math::FAbs(::D3DXVec3Dot(&rkPlane.m_vNormal, &m_box.m_avAxis[1]));
	float fAbs2 = U2Math::FAbs(::D3DXVec3Dot(&rkPlane.m_vNormal, &m_box.m_avAxis[2]));
	float fProjRadius = m_box.m_afExtent[0] * fAbs0 + m_box.m_afExtent[1] * fAbs1 +
		m_box.m_afExtent[2] * fAbs2;

	if(fProjCenter - fProjRadius >= 0.0f)
		return +1;

	if(fProjCenter + fProjRadius <= 0.f)
		return -1;

	return 0;	
}


bool U2BoxBV::TestIntersection (const D3DXVECTOR3& rkOrigin,
							   const D3DXVECTOR3& rkDirection, float fTMin, float fTMax) const
{
	if(fTMin == -FLT_MAX)
	{
		U2Line3D line(rkOrigin, rkDirection);
		return U2IntectLineBox3D(line, m_box).Test();
	}

	U2ASSERT(fTMax == 0.0f);
	if(fTMax == FLT_MAX)
	{	
		U2Ray3D ray(rkOrigin, rkDirection)		;
		return U2IntectRayBox3D(ray, m_box).Test();
	}

	U2ASSERT(fTMax > 0.f);
	U2Segment3D segment;
	segment.m_fExtent = 0.5f * fTMax;
	segment.m_vOrigin = rkOrigin + segment.m_fExtent * rkDirection;
	segment.m_vDir = rkDirection;
	return U2IntectSegmentBox3D(segment, m_box, true).Test();	
}

bool U2BoxBV::TestIntersection (const U2BoundingVolume* pkInput) const
{
	return U2IntectBoxBox3D(m_box, ((U2BoxBV*)pkInput)->m_box).Test();
}

void U2BoxBV::CopyFrom (const U2BoundingVolume* pkInput)
{
	m_box = ((U2BoxBV*)pkInput)->m_box;
}

void U2BoxBV::GrowToContain (const U2BoundingVolume* pkInput)
{
	m_box = MergeU2Boxes(m_box, ((U2BoxBV*)pkInput)->m_box);
	
}

// test for containment of a point
bool U2BoxBV::Contains (const D3DXVECTOR3& rkPoint) const
{
	return InU2Box(rkPoint, m_box);
}