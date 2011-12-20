/************************************************************************
module	:	U2Triangle3
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_TRIANGLE3_H
#define	U2_TRIANGLE3_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2Lib/Src/memory/U2MemObj.h>
#include <d3dx9math.h>

class U2Triangle3 : public U2MemObj
{
public:
	U2Triangle3();
	U2Triangle3(const D3DXVECTOR3& v0, const D3DXVECTOR3& v1, 
		const D3DXVECTOR3& v2);
	U2Triangle3(const D3DXVECTOR3 v[3]);

	//float DistanceTo(const D3DXVECTOR3& q) const;

	D3DXVECTOR3 v[3];
};

#include "U2Triangle3.inl"


#endif
