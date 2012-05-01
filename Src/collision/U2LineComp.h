/************************************************************************
module	:	U2LineComp
Author	:	Yun sangyong
Desc	:	GT, Section 5.1: Linear Components
			GT, Section 9.1: Linear Components	
************************************************************************/
#pragma once
#ifndef U2_LINECOMP_H
#define U2_LINECOMP_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2lib/src/U2RefObject.h>
#include <U2lib/src/U2SmartPtr.h>


class U2_3D U2LineComp : public U2RefObject
{

public: 
	virtual ~U2LineComp();


	// The linear component is represented as P+t*D where P is the component
	// origin and D is a unit-length direction vector.  The user must ensure
	// that the direction vector satisfies this condition.  The t-intervals
	// for lines, rays, segments, points, or the empty set are described
	// later.

	enum
	{
		CT_EMPTY,
		CT_POINT,
		CT_SEGMENT,
		CT_RAY,
		CT_LINE
	};

	// The interval of restriction for t, as defined above.  The function
	// SetInterval(min,max) sets the t-interval; it handles all possible
	// inputs according to the following scheme:
	//   CT_LINE:
	//     [-MAX_REAL,MAX_REAL]
	//   CT_RAY:
	//     [min,MAX_REAL], where min is finite
	//     [-MAX_REAL,max], where max is finite
	//   CT_SEGMENT:
	//     [min,max], where min and max are finite with min < max
	//   CT_POINT:
	//     [min,max], where min and max are finite with min = max
	//   CT_EMPTY:
	//     [min,max], where min > max or min = max = MAX_REAL or
	//                min = max = -MAX_REAL
	void SetInterval (float fMin, float fMax);

	// Determine the type of an interval without having to create an instance
	// of a LinComp object.
	static int GetTypeFromInterval (float fMin, float fMax);

	// The canonical intervals are [-MAX_REAL,MAX_REAL] for a line;
	// [0,MAX_REAL] for a ray; [-e,e] for a segment, where e > 0; [0,0] for
	// a point, and [MAX_REAL,-MAX_REAL] for the empty set.  If the interval
	// is [min,max], the adjustments are as follows.
	// 
	// CT_RAY:  If max is MAX_REAL and if min is not zero, then P is modified
	// to P' = P+min*D so that the ray is represented by P'+t*D for t >= 0.
	// If min is -MAX_REAL and max is finite, then the origin and direction
	// are modified to P' = P+max*D and D' = -D.
	//
	// CT_SEGMENT:  If min is not -max, then P is modified to
	// P' = P + ((min+max)/2)*D and the extent is e' = (max-min)/2.
	//
	// CT_POINT:  If min is not zero, the P is modified to P' = P+min*D.
	//
	// CT_EMPTY:  Set max to -MAX_REAL and min to MAX_REAL.
	//
	// The first function is virtual since the updates are dependent on the
	// dimension of the vector space.
	virtual void MakeCanonical () = 0;
	bool IsCanonical () const;

	// access the interval [min,max]
	float GetMin () const;
	float GetMax () const;

	// Determine if the specified parameter is in the interval.
	bool Contains (float fParam) const;

protected:
	U2LineComp ();  // default is CT_NONE

	// assignment
	U2LineComp& operator= (const U2LineComp& rkComponent);

	// component type
	int m_eType;

	// the interval of restriction for t
	float m_fMin, m_fMax;
};

#include "U2LineComp.inl"

typedef U2SmartPtr<U2LineComp> U2LineCompPtr;

#endif 