/**************************************************************************************************
module	:	U2Line3D
Author	:	Yun sangyong
Desc	:	
**************************************************************************************************/
#pragma once
#ifndef U2_LINE3D_H
#define U2_LINE3D_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2lib/src/U2RefObject.h>
#include <U2lib/src/U2SmartPtr.h>


class U2Line3D : public U2RefObject 
{
public:
	U2Line3D();
	U2Line3D(const D3DXVECTOR3& origin, const D3DXVECTOR3& dir);

	D3DXVECTOR3 m_vOrigin, m_vDir;
};

typedef U2SmartPtr<U2Line3D> U2Line3DPtr;

#include "U2Line3D.inl"

#endif 

