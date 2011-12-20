/**************************************************************************************************
module	:	U2D3DXEffectShaderInclude
Author	:	Yun sangyong
Desc	:	
**************************************************************************************************/
#pragma once
#ifndef U2_D3DXEFFECTSHADER_INCLUDE_H
#define U2_D3DXEFFECTSHADER_INCLUDE_H

#include <d3dx9.h>

class U2D3DXEffectShaderInclude : public ID3DXInclude
{
public:
	U2D3DXEffectShaderInclude(const U2DynString& szDir);

	/// open an include file and read its contents
	STDMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, 
		LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes);
	/// close an include file
	STDMETHOD(Close)(LPCVOID pData);

private:
	U2DynString m_szShaderDir;
};

#endif
