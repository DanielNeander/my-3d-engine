/**************************************************************************************************
module	:	U2Frustum
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef	U2_FRUSTUM_H
#define	U2_FRUSTUM_H

#include <u2_3D/src/collision/U2Aabb.h>

class U2_3D U2Frustum
{

public:
	
	U2Frustum(bool bOrtho = false);
	U2Frustum(float fLeft, float fRight, float fTop, float fBottom, float fNear, float fFar, 
		bool bOrtho = false);

	U2Frustum( const D3DXMATRIX* matrix );

	float m_fLeft;
	float m_fRight;
	float m_fTop;
	float m_fBottom;
	float m_fNear;
	float m_fFar;
	bool m_bOrtho;

	// 8 corner points of frustum
	D3DXVECTOR3 m_Points[8];
	// computes AABB from corner points
	inline void CalculateAABB(void);

	
	// world space bounding box
	U2Aabb m_AABB;
	
};

inline U2Frustum::U2Frustum(bool bOrtho) : m_bOrtho(bOrtho)
{
}

//---------------------------------------------------------------------------
inline U2Frustum::U2Frustum(float fLeft, float fRight, float fTop, float fBottom,
					 float fNear, float fFar, bool bOrtho)
					 :
m_fLeft(fLeft),
m_fRight(fRight),
m_fTop(fTop),
m_fBottom(fBottom),
m_fNear(fNear),
m_fFar(fFar),
m_bOrtho(bOrtho)
{
}

// computes AABB vectors from corner points
inline void U2Frustum::CalculateAABB(void)
{
	m_AABB.Set(m_Points, 8, sizeof(D3DXVECTOR3));
}


#endif