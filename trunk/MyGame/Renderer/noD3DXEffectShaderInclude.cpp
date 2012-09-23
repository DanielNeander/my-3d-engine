#include "DXUT.h"
#include <Core/U2File.h>
#include "noD3DXEffectShaderInclude.h"


noD3DXEffectShaderInclude::noD3DXEffectShaderInclude(const TCHAR* szDir)
{
	m_szShaderDir.assign(szDir);
}

HRESULT 
	noD3DXEffectShaderInclude::Open(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFilename, 
	LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
{
	const TCHAR* szFilename;
#ifdef UNICODE
	szFilename = ToUnicode(pFilename);
#else 
	szFilename = pFilename;
#endif 

	U2File* pFile = U2File::GetFile(szFilename, U2File::READ_ONLY);
	if(!pFile || !(*pFile))	// operator bool 
	{
#if UNICODE
		std::wstring filePath = m_szShaderDir + szFilename;
#else 
		std::string filePath = m_szShaderDir + szFilename;
#endif
		pFile = U2File::GetFile(filePath.c_str(), U2File::READ_ONLY);
		if(!pFile || !(*pFile))
		{
			delete pFile;
			return E_FAIL;
		}		
	}

	uint32 uBuffLen = pFile->GetfileSize();
	if(uBuffLen == 0)
	{
		delete pFile;
		return E_FAIL;
	}

	unsigned char* pBuffer = (unsigned char*)malloc(uBuffLen);
	pFile->Read(pBuffer, uBuffLen);

	*ppData = pBuffer;
	*pBytes = uBuffLen;

	delete pFile;

	return S_OK;
}

HRESULT noD3DXEffectShaderInclude::Close(LPCVOID pData)
{
	free((void*)pData);
	pData = NULL;
	return S_OK;
}
