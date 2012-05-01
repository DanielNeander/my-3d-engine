/************************************************************************
module	:	U2BoxBV
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once
#ifndef U2_SPHEREBV_H
#define U2_SPHEREBV_H

#include <U2_3D/src/collision/U2Sphere.h>
#include "U2BoundingVolume.h"

class U2_3D U2SphereBV : public U2BoundingVolume
{

public:

	U2SphereBV();
	U2SphereBV(const U2Sphere& sphere);

	virtual int GetBVType() const;

	virtual void SetCenter(const D3DXVECTOR3& center);
	virtual void SetRadius(float fRadius);
	virtual D3DXVECTOR3 GetCenter() const;
	virtual float GetRadius() const;

	U2Sphere& Sphere();
	const U2Sphere& Sphere() const;

	virtual void ComputeFromData (const U2MeshData* pkVBuffer);

	  // Transform the sphere (model-to-world conversion).
	virtual void TransformBy (const U2Transform& rkTransform,
		U2BoundingVolume* pkResult);

	// Determine if the bounding volume is one side of the plane, the other
	// side, or straddles the plane.  If it is on the positive side (the
	// side to which the normal points), the return value is +1.  If it is
	// on the negative side, the return value is -1.  If it straddles the
	// plane, the return value is 0.
	virtual int WhichSide (const U2Plane& rkPlane) const;

	// Test for intersection of linear component and bound (points of
	// intersection not computed).  The linear component is parameterized by
	// P + t*D, where P is a point on the component and D is a unit-length
	// direction.  The interval [tmin,tmax] is
	//   line:     tmin = -Mathf::MAX_REAL, tmax = Mathf::MAX_REAL
	//   ray:      tmin = 0.0f, tmax = Mathf::MAX_REAL
	//   segment:  tmin = 0.0f, tmax > 0.0f
	virtual bool TestIntersection (const D3DXVECTOR3& rkOrigin,
		const D3DXVECTOR3& rkDirection, float fTMin, float fTMax) const;

	// Test for intersection of the two bounds.
	virtual bool TestIntersection (const U2BoundingVolume* pkInput) const;

	// Make a copy of the bounding volume.
	virtual void CopyFrom (const U2BoundingVolume* pkInput);

	// Change the current bounding volume so that it contains the input
	// bounding volume as well as its old bounding volume.
	virtual void GrowToContain (const U2BoundingVolume* pkInput);

	// test for containment of a point
	virtual bool Contains (const D3DXVECTOR3& rkPoint) const;

protected:
	U2Sphere m_sphere;

};

#include "U2SphereBV.inl"

#endif