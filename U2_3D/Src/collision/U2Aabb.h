/************************************************************************
module	:	U2Aabb
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once
#ifndef U2_AABB_H
#define U2_AABB_H
//------------------------------------------------------------------------------
/**
@class bbox3
@ingroup NebulaMathDataTypes

A non-oriented bounding box class.

(C) 2004 RadonLabs GmbH
*/

#include <U2_3D/Src/U2_3DLibType.h>
#include <U2lib/Src/Memory/U2MemObj.h>
#include <U2_3d/Src/Main/U2Math.h>
#include <d3dx9math.h>

#define MAX_BOUND 1000000.0f
#define MIN_BOUND -1000000.0f

class U2Aabb : public U2MemObj
{

public:


	U2Aabb();
	U2Aabb(const D3DXVECTOR3& center, const D3DXVECTOR3& extents);
	U2Aabb(const D3DXMATRIX& m);
	U2Aabb(const void* pPoints, int iNumPoints, int iStride);

	D3DXVECTOR3 Center() const;
	D3DXVECTOR3 Extents() const;
	D3DXVECTOR3 Size() const;
	float DiagonalSize() const;
	void Set(const D3DXMATRIX& m);
	void Set(const D3DXVECTOR3& center, const D3DXVECTOR3& extents);
	// set from minimum and maximum vectors
	void Set2(const D3DXVECTOR3& vMin, const D3DXVECTOR3& vMax);
	// set from set of points
	void Set(const void *pPoints, int iNumPoints, int iStride);
	

	void BeginExtend();
	void Extend(const D3DXVECTOR3& v);
	void Extend(float x, float y, float z);
	void Extend(const U2Aabb& box);
	void EndExtend();

	bool Intersect( float* hitDist, const D3DXVECTOR3* origPt, const D3DXVECTOR3* dir );

	
	void Transform(const D3DXMATRIX& m);

	// Screen Space Transform
	//void TransformDivW(const D3DXMATRIX& m);
		


	D3DXVECTOR3 CornerPoint(int idx) const;

	//bool Intersects(const U2Aabb& box) const;
	D3DXVECTOR3 m_vMin;
	D3DXVECTOR3 m_vMax;
	D3DXVECTOR3 m_Points[8];

};

typedef U2Aabb U2Obb;

#include "U2Aabb.inl"

#endif 