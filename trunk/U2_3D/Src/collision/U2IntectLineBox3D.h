/************************************************************************
module	:	U2IntectLineBox3D
Author	:	Yun sangyong
Desc	:	GT, Section 11.12.2: Linear Components and Axis-Aligned
			Bounding Box
			GT, Section 11.12.3: Linear Components and Oriented Bounding
			Box
************************************************************************/
#pragma once
#ifndef U2_INTECTLINEBOX3D_H
#define U2_INTECTLINEBOX3D_H

#include <U2_3D/src/U2_3DLibType.h>
//#include <U2lib/src/U2RefObject.h>
//#include <U2lib/src/U2SmartPtr.h>
#include <U2_3D/src/Main/U2Box.h>
#include <U2_3D/src/Main/U2Line3D.h>

#include "U2Intersector.h"

class U2_3D U2IntectLineBox3D : public U2Intersector
{
public:
	U2IntectLineBox3D(const U2Line3D& line, const U2Box& box);

	const U2Line3D& GetLine() const;
	const U2Box& GetBox() const;

	virtual bool Test();
	virtual bool Find();

	uint32 GetCnt() const;
	const D3DXVECTOR3& GetPoint(int i) const;

	static bool DoClipping(float fT0, float fT1, const D3DXVECTOR3& origin, 
		const D3DXVECTOR3& dir, const U2Box& box, bool bSolid, int& cnt, 
		D3DXVECTOR3 aPoint[2], int& intectType );

private:
	using U2Intersector::IT_EMPTY;
	using U2Intersector::IT_POINT;
	using U2Intersector::IT_SEGMENT;
	using U2Intersector::m_eIntersectionType;

	static bool Clip(float fDenom, float fNumer, float& fT0, float& fT1);

	const U2Line3D* m_pLine;
	const U2Box* m_pBox;

	int m_uCnt;
	D3DXVECTOR3 m_aPoint[2];

};

typedef U2SmartPtr<U2IntectLineBox3D> U2IntectLineBox3DPtr;

#endif 