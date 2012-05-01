#include <U2_3D/Src/U23DLibPCH.h>
#include "U2D3DXEffectShaderInclude.h"
#include <U2lib/src/U2File.h>


U2D3DXEffectShaderInclude::U2D3DXEffectShaderInclude(const U2DynString &szDir)
{
	m_szShaderDir = szDir + _T("/");
}

HRESULT 
U2D3DXEffectShaderInclude::Open(D3DXINCLUDE_TYPE IncludeType, LPCSTR pFilename, 
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
		U2DynString filePath = m_szShaderDir + szFilename;
		pFile = U2File::GetFile(filePath, U2File::READ_ONLY);
		if(!pFile || !(*pFile))
		{
			U2_DELETE pFile;
			return E_FAIL;
		}		
	}

	uint32 uBuffLen = pFile->GetfileSize();
	if(uBuffLen == 0)
	{
		U2_DELETE pFile;
		return E_FAIL;
	}

	unsigned char* pBuffer = U2_ALLOC(unsigned char, uBuffLen);
	pFile->Read(pBuffer, uBuffLen);

	*ppData = pBuffer;
	*pBytes = uBuffLen;

	U2_DELETE pFile;

	return S_OK;
}

HRESULT
U2D3DXEffectShaderInclude::Close(LPCVOID pData)
{
	U2_FREE((void*)pData);
	pData = NULL;
	return S_OK;
}
