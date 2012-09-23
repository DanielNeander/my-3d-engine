/**************************************************************************************************
module	:	U2D3DXEffectShaderInclude
Author	:	Yun sangyong
Desc	:	
**************************************************************************************************/
#pragma once
#ifndef U2_D3DXEFFECTSHADER_INCLUDE_H
#define U2_D3DXEFFECTSHADER_INCLUDE_H

#include <d3dx9.h>
#include <string>

class noD3DXEffectShaderInclude : public ID3DXInclude
{
public:
	noD3DXEffectShaderInclude(const TCHAR* szDir);

	/// open an include file and read its contents
	STDMETHOD(Open)(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFileName, 
		LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes);
	/// close an include file
	STDMETHOD(Close)(LPCVOID pData);

private:

#ifdef UNICODE 
	std::wstring m_szShaderDir;
#else 
	std::string m_szShaderDir;
#endif
};

#endif
