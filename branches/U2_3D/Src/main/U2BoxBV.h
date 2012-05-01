/************************************************************************
module	:	U2BoxBV
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once
#ifndef U2_BOXBV_H
#define U2_BOXBV_H

#include "U2BoundingVolume.h"


class U2_3D U2BoxBV : public U2BoundingVolume
{
	DECLARE_RTTI;
public:
	U2BoxBV();
	U2BoxBV(const U2Box& rhs);
	virtual ~U2BoxBV();

	virtual int GetBVType() const;

	virtual void SetCenter(const D3DXVECTOR3& vCenter);
	virtual void SetRadius (float fRadius);
	virtual D3DXVECTOR3 GetCenter () const;
	virtual float GetRadius () const;

	U2Box& GetBox();
	const U2Box& GetBox() const;

	// Compute a box that contains all the points.
	virtual void ComputeFromData (const U2MeshData* pkVBuffer);

	virtual void TransformBy (const U2Transform& rkTransform,
		U2BoundingVolume* pkResult);

	virtual int WhichSide (const U2Plane& rkPlane) const;


	virtual bool TestIntersection (const D3DXVECTOR3& rkOrigin,
		const D3DXVECTOR3& rkDirection, float fTMin, float fTMax) const;

	virtual bool TestIntersection (const U2BoundingVolume* pkInput) const;

	virtual void CopyFrom (const U2BoundingVolume* pkInput);

	// Change the current bounding volume so that it contains the input
	// bounding volume as well as its old bounding volume.
	virtual void GrowToContain (const U2BoundingVolume* pkInput);

	// test for containment of a point
	virtual bool Contains (const D3DXVECTOR3& rkPoint) const;

protected:
	U2Box m_box;

};



#endif
