/************************************************************************
module	:	U2NumericalConstant
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_NUMERICALCONSTANT_H
#define U2_NUMERICALCONSTANT_H

#include <U2Lib/src/U2RefObject.h>

class U2_3D U2NumericalConstant : public U2RefObject
{
public:
	U2NumericalConstant(int registerNum, float fData[4]);
	~U2NumericalConstant();

	int GetRegister() const;
	float* GetData() const;

private:
	int m_iRegisterNum;
	float m_afData[4];
};

typedef U2SmartPtr<U2NumericalConstant> U2NumericalConstantPtr;

inline 
U2NumericalConstant::U2NumericalConstant(int registerNum, float fData[4])
{
	U2ASSERT(registerNum >= 0);

	m_iRegisterNum = registerNum;

	memcpy(m_afData, fData, sizeof(float) * 4);
}


inline 
U2NumericalConstant::~U2NumericalConstant()
{

}


inline 
int U2NumericalConstant::GetRegister() const 
{
	return m_iRegisterNum;
}


inline 
float* U2NumericalConstant::GetData() const 
{
	return (float*)&m_afData[0];
}

#endif 