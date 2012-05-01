/************************************************************************
module	:	U2FrameShader
Author	:	Yun sangyong
Desc	:	Nebula Device 2 
************************************************************************/
#pragma once 
#ifndef U2_FRAMESHADER_H
#define U2_FRAMESHADER_H

#include <U2_3D/Src/Shader/U2D3DXEffectShader.h>

class U2_3D U2FrameShader : public U2Object 
{
public:

	U2FrameShader();
	virtual ~U2FrameShader();

	
	void SetFilename(const U2DynString& szFilename);
	const U2DynString& GetFilename() const;

	void SetBucketIdx(unsigned int idx);
	unsigned int GetBucketIdx() const;

	U2D3DXEffectShader* GetShader() const;

	void Validate();

	
	// Resource 관리를 제대로 하고 있지 않기 때문에.. .
	// 현재 임시로 사용할 함수..
	static U2D3DXEffectShader* FindEffectShader(const U2DynString& szShaderFilename);

private:

	U2DynString m_szFilename;
	unsigned int m_uiBucketIdx;
	U2D3DXEffectShader*  m_pEffectShader;

	U2D3DXEffectShader*  m_pCachedSahder;

};

#include "U2Dx9FrameShader.inl"

typedef U2SmartPtr<U2FrameShader> U2FrameShaderPtr;


#endif