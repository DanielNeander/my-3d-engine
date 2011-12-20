/**************************************************************************************************
module	:	U2Plane
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_PLANE_H
#define U2_PLANE_H


class U2_3D U2Plane : public U2MemObj
{

public:
	union 
	{
		float m_fN[4];
		struct 
		{
			D3DXVECTOR3 m_vNormal;
			float m_fDistance;
		};		
	};

	U2Plane();
	U2Plane(const U2Plane& in);
	U2Plane(const D3DXVECTOR3& pt0, const D3DXVECTOR3& pt1, const D3DXVECTOR3& pt2);
	U2Plane(const D3DXVECTOR3& normal, float distance);
	U2Plane(const D3DXVECTOR3& pt, const D3DXVECTOR3& normal);
	~U2Plane();

	inline U2Plane&		operator= (const U2Plane& rhs);
	inline bool			operator==(const U2Plane& src);

	inline void			Set(const D3DXVECTOR3& pt0, const D3DXVECTOR3& pt1, const D3DXVECTOR3& pt2);
	inline void			Set(const D3DXVECTOR3& normal, float distance);
	inline void			Set(const D3DXVECTOR3& pt, const D3DXVECTOR3& normal);

	inline void			Normalize();
	inline void			Transform(const D3DXMATRIX& mat);

	// The "positive side" of the plane is the half space to which the plane
	// normal points.  The "negative side" is the other half space.  The
	// function returns +1 for the positive side, -1 for the negative side,
	// and 0 for the point being on the plane.
	int WhichSide (const D3DXVECTOR3& rkP) const;


	// Compute d = Dot(N,Q)+c where N is the plane normal and c is the plane
	// constant.  This is a signed distance.  The sign of the return value is
	// positive if the point is on the positive side of the plane, negative if
	// the point is on the negative side, and zero if the point is on the
	// plane.
	inline float			GetDistance(const D3DXVECTOR3& pt) const;

};

#include "U2Plane.inl"


#endif