/**************************************************************************************************
module	:	U2ShaderArg
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once 
#ifndef U2_SHADERARG_H
#define U2_SHADERARG_H

#include <U2Lib/src/U2RefObject.h>
#include <U2Lib/src/U2DataType.h>
#include "U2ShaderConstant.h"

U2SmartPointer(U2Dx9BaseTexture);

class U2_3D U2ShaderArg : public U2RefObject
{
public:
	enum ArgType
	{
		AT_VOID,
		AT_BOOL,
		AT_INT,
		AT_FLOAT,
		AT_FLOAT4,
		AT_MATRIX44,
		AT_TEXTURE,
		AT_NUMTYPES,  // always keep this after all valid types!
		AT_INVALIDTYPE
	};


	U2ShaderArg();
	U2ShaderArg(ArgType eType);
	U2ShaderArg(bool bVal);
	U2ShaderArg(int iVal);
	U2ShaderArg(float fVal);
	U2ShaderArg(const U2Float4& f4Val);
	U2ShaderArg(const D3DXVECTOR4& vVal);
	U2ShaderArg(const D3DXMATRIX& matVal);
	U2ShaderArg(const U2Dx9BaseTexture* pTexVal);

	~U2ShaderArg();

	bool operator == (const U2ShaderArg& rhs) const;
	void operator=(const U2ShaderArg& rhs);

	void SetType(ArgType eType);
	ArgType GetType() const;

	void SetBool(bool bVal);
	bool GetBool()const;

	void SetInt(int val);
	int  GetInt() const;

	void SetFloat(float val);
	float GetFloat() const;

	void SetFloat4(const U2Float4& val);
	const U2Float4& GetFloat4() const;

	void SetVector4(const D3DXVECTOR4& val);
	D3DXVECTOR4& GetVector4() const;

	void SetMatrix44(const D3DXMATRIX* val);
	const D3DXMATRIX* GetMatrix44() const;

	void SetTexture(U2Dx9BaseTexture* val);
	U2Dx9BaseTexture* GetTexture() const;

	void Clear();

private:
	ArgType m_eType;

	union 
	{
		bool m_bVal;
		int m_iVal;
		float m_fVal;
		U2Float4 m_f4Val;
		float m[4][4];
	};

	U2Dx9BaseTexturePtr m_spTex;	

	
};

typedef U2SmartPtr<U2ShaderArg> U2ShaderArgPtr;

#include "U2ShaderArg.inl"

#endif 