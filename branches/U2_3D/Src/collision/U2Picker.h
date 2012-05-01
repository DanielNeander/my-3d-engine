/************************************************************************
module	:	U2Picker
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once
#ifndef U2_PICKER_H
#define U2_PICKER_H

#include <U2Lib/src/U2TVec.h>
#include <U2_3D/src/U2_3DLibType.h>
#include "U2PickRecord.h"
//#include <U2Lib/Src/memory/U2MemObj.h>

class U2_3D U2Picker : public U2MemObj
{

public:
	U2Picker();

	// The linear component is parameterized by P + t*D, where P is a point on
	// the component (P is the origin), D is a unit-length direction, and t is
	// a scalar in the interval [tmin,tmax] with tmin < tmax.  The P and D
	// values must be in world coordinates.  The choices for tmin and tmax are
	//   line:     tmin = -Mathf::MAX_REAL, tmax = Mathf::MAX_REAL
	//   ray:      tmin = 0, tmax = Mathf::MAX_REAL
	//   segment:  tmin = 0, tmax > 0;
	//
	// A call to this function will automatically clear the Records array.
	// If you need any information from this array obtained by a previous
	// call to Execute, you must save it first.

	void Execute(U2Spatial* pScene, const D3DXVECTOR3& origin, 
		const D3DXVECTOR3& dir, float fTMin, float fTMax);

	const U2PickRecord& GetClosestToZero() const;

	const U2PickRecord& GetClosestNonnegative() const;

	const U2PickRecord& GetClosestNonpositive() const;

	U2PrimitiveVec<U2PickRecord*> records;

private:
	void ExecuteRecursive(U2Spatial* pObj);

	D3DXVECTOR3 m_origin, m_dir;
	float m_fTMin, m_fTMax;

	static const U2PickRecord ms_invalid;
};

#endif 




