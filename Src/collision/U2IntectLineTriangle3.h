/************************************************************************
module	:	U2IntectLineTriangle3
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once
#ifndef U2_INTECTLINETRIANGLE3_H
#define U2_INTECTLINETRIANGLE3_H

#include <U2_3D/src/U2_3DLibType.h>
//#include <U2lib/src/U2RefObject.h>
//#include <U2lib/src/U2SmartPtr.h>
#include <U2_3D/src/Main/U2Triangle3.h>
#include <U2_3D/src/Main/U2Line3D.h>

#include "U2Intersector.h"

class U2_3D U2IntectLineTriangle3 : public U2Intersector
{
public:
	U2IntectLineTriangle3(const U2Line3D& line, const U2Triangle3& triangle);
	~U2IntectLineTriangle3();

	const U2Line3D& GetLine() const;
	const U2Triangle3& GetTriangle() const;

	virtual bool Test();

	// Find-intersection query.  The point of intersection is
	//   P = origin + t*direction = b0*V0 + b1*V1 + b2*V2
	virtual bool Find();
	float GetLineT() const;
	float GetTriB0() const;
	float GetTriB1() const;
	float GetTriB2() const;

private:
	U2Line3DPtr m_spLine;
	const U2Triangle3* m_pTriangle;

	float m_fLineT, m_fTriB0, m_fTriB1, m_fTriB2;
};

//-------------------------------------------------------------------------------------------------
inline const U2Line3D& U2IntectLineTriangle3::GetLine() const
{
	return *m_spLine;
}

//-------------------------------------------------------------------------------------------------
inline const U2Triangle3& U2IntectLineTriangle3::GetTriangle() const
{
	return *m_pTriangle;
}

//-------------------------------------------------------------------------------------------------
inline float U2IntectLineTriangle3::GetLineT() const
{
	return m_fLineT;
}

//-------------------------------------------------------------------------------------------------
inline float U2IntectLineTriangle3::GetTriB0() const
{
	return m_fTriB0;
}

//-------------------------------------------------------------------------------------------------
inline float U2IntectLineTriangle3::GetTriB1() const
{
	return m_fTriB1;
}

//-------------------------------------------------------------------------------------------------
inline float U2IntectLineTriangle3::GetTriB2() const
{
	return m_fTriB2;
}

#endif