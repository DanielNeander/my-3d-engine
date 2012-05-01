/************************************************************************
module	:	U2IntectBoxBox3D
Author	:	Yun sangyong
Desc	:	GT, Section 7.7: The Method of Separating Axes			
************************************************************************/
#pragma once
#ifndef U2_INTECTBOXBOX3D_H
#define U2_INTECTBOXBOX3D_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2lib/src/U2RefObject.h>
#include <U2lib/src/U2SmartPtr.h>
#include <U2_3D/src/Main/U2Box.h>

#include "U2Intersector.h"

class U2_3D U2IntectBoxBox3D : public U2Intersector
{
public:
	U2IntectBoxBox3D(const U2Box& box0, const U2Box& box1);

	const U2Box& GetBox0() const;
	const U2Box& GetBox1() const;

	virtual bool Test();
	virtual bool Test(float fMax, const D3DXVECTOR3& velocity0, 
		const D3DXVECTOR3& velocity1);

private:
	using U2Intersector::m_fContactTime;

	// Support for dynamic queries.  The inputs are the projection intervals
	// for the boxes onto a potential separating axis, the relative speed of
	// the intervals, and the maximum time for the query.  The outputs are
	// the first time when separating fails and the last time when separation
	// begins again along that axis.  The outputs are *updates* in the sense
	// that this function is called repeatedly for the potential separating
	// axes.  The output first time is updated only if it is larger than
	// the input first time.  The output last time is updated only if it is
	// smaller than the input last time.
	bool IsSeparated (float fMin0, float fMax0, float fMin1, float fMax1,
		float fSpeed, float fTMax, float& rfTFirst, float& rfTLast);

	const U2Box* m_pBox0;
	const U2Box* m_pBox1;
	
};

typedef U2SmartPtr<U2IntectBoxBox3D> U2IntectBoxBox3DPtr;

#endif 