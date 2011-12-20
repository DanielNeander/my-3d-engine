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

	// ���̴� ���̽� Ŭ������ ����� ���̴� ����� ���� ���� ������ �����ϰ� 
	// �Ǽ� �����Ͱ� ���̴� ���α׷��� �ε�� �� �� ����Ҹ� ����Ű�� �Ѵ�. 
	// �׷����� ���̴� ��� Ŭ������ �ڽ��� ��������� ������ ���� �ְ� 
	// �Ǽ� �����͸� ���������� ����Ű�� �� �� �ִ�. �׷��� ��� Ŭ�������� 
	// �޸� ������ ������ å���� �ִ�. 
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