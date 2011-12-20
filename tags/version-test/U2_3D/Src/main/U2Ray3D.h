/************************************************************************
module	:	U2Ray3D
Author	:	Yun sangyong
Desc	:	GT, Section 5.1: Linear Components
			GT, Section 9.1: Linear Components
************************************************************************/
#pragma once
#ifndef U2_RAY3D_H
#define U2_RAY3D_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2lib/src/U2RefObject.h>
#include <U2lib/src/U2SmartPtr.h>


class U2Ray3D : public U2RefObject 
{
public:
	U2Ray3D();
	U2Ray3D(const D3DXVECTOR3& origin, const D3DXVECTOR3& dir);

	D3DXVECTOR3 m_vOrigin, m_vDir;
};

typedef U2SmartPtr<U2Ray3D> U2Ray3DPtr;

#include "U2Ray3D.inl"

#endif