/************************************************************************
module	:	U2BoundingVolume
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_BOUNDINGVOLUME_H
#define	U2_BOUNDINGVOLUME_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2lib/src/U2RefObject.h>
#include "U2Plane.h"
#include "U2Transform.h"


class U2MeshData;


class U2_3D U2BoundingVolume : public U2Object
{
	DECLARE_RTTI;

public:
	
	virtual ~U2BoundingVolume();

	enum BVType 
	{
		BV_SPHERE,
		BV_BOX,
		BV_QUANTITY,
	};

	virtual int GetBVType() const = 0;

	virtual void SetCenter(const D3DXVECTOR3& vCenter) = 0;
	virtual void SetRadius (float fRadius) = 0;
	virtual D3DXVECTOR3 GetCenter () const = 0;
	virtual float GetRadius () const = 0;

	  static U2BoundingVolume* Create ();

	  // Compute a bounding volume that contains all the points.	  
	  virtual void ComputeFromData (const U2MeshData* pkVBuffer) = 0;

	  // Transform the bounding volume (model-to-world conversion).
	  virtual void TransformBy (const U2Transform& rkTransform,
		  U2BoundingVolume* pkResult) = 0;

	  // Determine if the bounding volume is one side of the plane, the other
	  // side, or straddles the plane.  If it is on the positive side (the
	  // side to which the normal points), the return value is +1.  If it is
	  // on the negative side, the return value is -1.  If it straddles the
	  // plane, the return value is 0.
	  virtual int WhichSide (const U2Plane& rkPlane) const = 0;


	  // Test for intersection of linear component and bound (points of
	  // intersection not computed).  The linear component is parameterized by
	  // P + t*D, where P is a point on the component and D is a unit-length
	  // direction.  The interval [tmin,tmax] is
	  //   line:     tmin = -Mathf::MAX_REAL, tmax = Mathf::MAX_REAL
	  //   ray:      tmin = 0.0f, tmax = Mathf::MAX_REAL
	  //   segment:  tmin = 0.0f, tmax > 0.0f
	  virtual bool TestIntersection (const D3DXVECTOR3& rkOrigin,
		  const D3DXVECTOR3& rkDirection, float fTMin, float fTMax) const = 0;

	  // Test for intersection of the two bounds.
	  virtual bool TestIntersection (const U2BoundingVolume* pkInput) const = 0;

	  // Make a copy of the bounding volume.
	  virtual void CopyFrom (const U2BoundingVolume* pkInput) = 0;

	  // Change the current bounding volume so that it contains the input
	  // bounding volume as well as its old bounding volume.
	  virtual void GrowToContain (const U2BoundingVolume* pkInput) = 0;

	  // test for containment of a point
	  virtual bool Contains (const D3DXVECTOR3& rkPoint) const = 0;


protected:
	U2BoundingVolume ();

};

typedef U2SmartPtr<U2BoundingVolume> U2BoundingVolumePtr;

#endif 