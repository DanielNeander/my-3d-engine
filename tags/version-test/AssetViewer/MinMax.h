#ifndef MINMAX_H
#define MINMAX_H

#include <U2_3D/src/main/U2Math.h>

template <class T>
class CMinMax
{
public:
	CMinMax() { m_Min = T(); m_Max = T(); }
	CMinMax(T tMin, T tMax) { m_Min = tMin; m_Max = tMax; }
	~CMinMax() {};

	T m_Min, m_Max;

	inline T GetRandomNumInRange(void) { return(U2Math::RandomNumber(m_Min, m_Max)); }
	inline T GetRange(void)	{ return(abs(m_Max - m_Min)); }
};


#endif