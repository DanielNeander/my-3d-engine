/************************************************************************
module	:	U2Sphere
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once
#ifndef U2_SPHERE_H
#define U2_SPHERE_H

#include <vector>

#include <u2lib/src/Memory/U2MemObj.h>
#include <d3dx9math.h>
#include "U2Aabb.h"

class U2Sphere : public U2MemObj
{
public:
	U2Sphere();
	U2Sphere(const D3DXVECTOR3& center, float fRadius);
	U2Sphere(const U2Sphere& sphere);
	explicit U2Sphere(const std::vector<D3DXVECTOR3>* points);
	explicit U2Sphere(const U2Aabb* box);



	U2Sphere& operator= (const U2Sphere& sphere);

	D3DXVECTOR3 m_center;
	float m_fRadius;
};

//-------------------------------------------------------------------------------------------------
inline U2Sphere::U2Sphere()
:m_center(VECTOR3_ZERO)
{

}

//-------------------------------------------------------------------------------------------------
inline U2Sphere::U2Sphere(const D3DXVECTOR3 &center, float fRadius)
:m_center(center),
m_fRadius(fRadius)
{

}

//-------------------------------------------------------------------------------------------------
inline U2Sphere::U2Sphere(const U2Sphere &sphere)
:m_center(sphere.m_center),
m_fRadius(sphere.m_fRadius)
{
	
}

//-------------------------------------------------------------------------------------------------
inline U2Sphere& U2Sphere::operator=(const U2Sphere& sphere)
{
	m_center = sphere.m_center;
	m_fRadius = sphere.m_fRadius;
	return *this;
}


#endif 