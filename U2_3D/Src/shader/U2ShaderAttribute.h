/**************************************************************************************************
module	:	U2ShaderAttribute
Author	:	Yun sangyong
Desc	:	Shader의 Attribute의 타입과 값을 정의
*************************************************************************************************/
#pragma once
#ifndef U2_SHADERATTRIBUTE_H
#define U2_SHADERATTRIBUTE_H


#include "U2ShaderArg.h"
#include <U2_3D/Src/shader/U2FxShaderParam.h>

class U2_3D U2ShaderAttribute : public U2MemObj
{
public:
	U2ShaderAttribute();

	~U2ShaderAttribute();

	void Clear();

	int GetNumValidAttbEntries() const;

	bool IsValid(U2FxShaderState::Param eParam) const;
	void SetArg(U2FxShaderState::Param eParam, const U2ShaderArg& arg);
	const U2ShaderArg& GetArg(U2FxShaderState::Param eParam) const;
	const U2ShaderArg& GetArg(int idx) const;

	void ClearAttbEntry(U2FxShaderState::Param eParam);
	U2FxShaderState::Param& GetParam(int idx) const ;
	

private:

	U2SmartPointer(AttbEntry);
	class AttbEntry : public U2RefObject
	{
	public:
		AttbEntry();
		AttbEntry(U2FxShaderState::Param eParam, 
			const U2ShaderArg& arg);

		U2FxShaderState::Param m_eParam;
		U2ShaderArg m_arg;
	};

	

	TCHAR m_tcSCMIdx[U2FxShaderState::NumParameters];	// index Shader Constant Mapping
	U2ObjVec<AttbEntryPtr> m_attbEntryArray;

	static U2ShaderArg* ms_pInvalidArg;
};

#include "U2ShaderAttribute.inl"

#endif

