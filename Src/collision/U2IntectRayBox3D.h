/************************************************************************
module	:	U2IntectRayBox3D
Author	:	Yun sangyong
Desc	:	GT, Section 11.12.2: Linear Components and Axis-Aligned
Bounding Box
GT, Section 11.12.3: Linear Components and Oriented Bounding
Box
************************************************************************/
#pragma once
#ifndef U2_INTECTRAYBOX3D_H
#define U2_INTECTRAYBOX3D_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2lib/src/U2RefObject.h>
#include <U2lib/src/U2SmartPtr.h>
#include <U2_3D/src/Main/U2Box.h>
#include <U2_3D/src/Main/U2Ray3D.h>

#include "U2Intersector.h"

class U2_3D U2IntectRayBox3D : public U2Intersector
{
public:
	U2IntectRayBox3D(const U2Ray3D& line, const U2Box& box);

	const U2Ray3D& GetLine() const;
	const U2Box& GetBox() const;

	virtual bool Test();
	virtual bool Find();

	uint32 GetCnt() const;
	const D3DXVECTOR3& GetPoint(int i) const;

private:
	using U2Intersector::m_eIntersectionType;


	const U2Ray3D* m_pRay;
	const U2Box* m_pBox;

	int m_uCnt;
	D3DXVECTOR3 m_aPoint[2];

};

typedef U2SmartPtr<U2IntectRayBox3D> U2IntectRayBox3DPtr;

#endif 