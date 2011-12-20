/**************************************************************************************************
module	:	U2ShaderConstant
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_SHADERUSERCONSTANT_H
#define U2_SHADERUSERCONSTANT_H

#include <U2Lib/src/U2RefObject.h>

class U2_3D U2ShaderUserConstant : public U2RefObject
{

public:
	U2ShaderUserConstant(const U2String& name, int baseRegister, int registerCnt);
	~U2ShaderUserConstant();

	const U2String& GetName() const;
	int GetBaseRegister() const;
	int GetRegisterCnt () const;
	float* GetData () const;

	// 쉐이더 베이스 클래스는 사용자 쉐이더 상수를 위한 저장 공간을 제공하고 
	// 실수 포인터가 쉐이더 프로그램이 로드될 때 이 저장소를 가리키게 한다. 
	// 그렇지만 쉐이더 상속 클래스는 자신의 저장공간을 제공할 수도 있고 
	// 실수 포인터를 마찬가지로 가리키게 할 수 있다. 그러한 상속 클래스들은 
	// 메모리 공간을 해제할 책임이 있다. 
	void SetDataSource(float* pfData);

private:
	U2String m_name;
	int m_iBaseRegister;
	int m_iRegisterCnt;
	float* m_pfData;
};

#include "U2ShaderUserConstant.inl"

typedef U2SmartPtr<U2ShaderUserConstant> U2ShaderUserConstantPtr;

#endif