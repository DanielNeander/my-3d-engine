/************************************************************************
module	:	U2Segment3D
Author	:	Yun sangyong
Desc	:	GT, Section 5.1: Linear Components
			GT, Section 9.1: Linear Components
************************************************************************/
#pragma once
#ifndef U2_SEGMENT_H
#define U2_SEGMENT_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2lib/src/U2RefObject.h>
#include <U2lib/src/U2SmartPtr.h>

class U2Segment3D : public U2RefObject 
{

public:
	U2Segment3D();
	U2Segment3D(const D3DXVECTOR3& origin, const D3DXVECTOR3& dir,
		float fExtent);

	D3DXVECTOR3 GetPosEnd() const; // P+e*D
	D3DXVECTOR3 GetNegEnd() const; // P-e*D

	D3DXVECTOR3 m_vOrigin, m_vDir;
	float m_fExtent;
};

#include "U2Segment3D.inl"


#endif 