/************************************************************************
module	:	U2Box
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once
#ifndef U2_BOX_H
#define U2_BOX_H

#include <U2Lib/src/memory/U2MemObj.h>

class U2Box : public U2MemObj 
{
public:
	U2Box();
	U2Box(const D3DXVECTOR3& vCenter, const D3DXVECTOR3* pAxis, 
		const float* pfExtent);
	U2Box(const D3DXVECTOR3& vCenter, const D3DXVECTOR3& vAxis0, 
		const D3DXVECTOR3& vAxis1, const D3DXVECTOR3& vAxis2, 
		float fExtent0, float fExtent1, float fExtent2);

	void ComputeVertices(D3DXVECTOR3 aVertex[8]) const;

	D3DXVECTOR3 m_vCenter;
	D3DXVECTOR3 m_avAxis[3];
	float m_afExtent[3];

};

#include "U2Box.inl"

#endif
