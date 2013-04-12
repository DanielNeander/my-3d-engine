#include "stdafx.h"
#include "Dx11Util.h"



//--------------------------------------------------------------------------------------
// Helper functions to create SRGB formats from typeless formats and vice versa
//--------------------------------------------------------------------------------------
DXGI_FORMAT MAKE_SRGB( DXGI_FORMAT format )
{
	//if( !DXUTIsInGammaCorrectMode() )
	//	return format;

	switch( format )
	{
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
		return DXGI_FORMAT_BC1_UNORM_SRGB;
	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
		return DXGI_FORMAT_BC2_UNORM_SRGB;
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
		return DXGI_FORMAT_BC3_UNORM_SRGB;

	};

	return format;
}


//--------------------------------------------------------------------------------------
// Returns pointer to static media search buffer
//--------------------------------------------------------------------------------------
WCHAR* DXUTMediaSearchPath()
{
	static WCHAR s_strMediaSearchPath[MAX_PATH] =
	{
		0
	};
	return s_strMediaSearchPath;

}

bool FindMediaSearchParentDirs( WCHAR* strSearchPath, int cchSearch, WCHAR* strStartAt, 
	WCHAR* strLeafName )
{
	WCHAR strFullPath[MAX_PATH] =
	{
		0
	};
	WCHAR strFullFileName[MAX_PATH] =
	{
		0
	};
	WCHAR strSearch[MAX_PATH] =
	{
		0
	};
	WCHAR* strFilePart = NULL;

	GetFullPathNameW( strStartAt, MAX_PATH, strFullPath, &strFilePart );
	if( strFilePart == NULL )
		return false;

	while( strFilePart != NULL && *strFilePart != '\0' )
	{
		swprintf_s( strFullFileName, MAX_PATH, L"%s\\%s", strFullPath, strLeafName );
		if( GetFileAttributesW( strFullFileName ) != 0xFFFFFFFF )
		{
			wcscpy_s( strSearchPath, cchSearch, strFullFileName );
			return true;
		}

		swprintf_s( strSearch, MAX_PATH, L"%s\\..", strFullPath );
		GetFullPathNameW( strSearch, MAX_PATH, strFullPath, &strFilePart );
	}

	return false;
}

bool FindMediaSearchTypicalDirs( WCHAR* strSearchPath, int cchSearch, LPCWSTR strLeaf, 
	WCHAR* strExePath, WCHAR* strExeName )
{
	// Typical directories:
	//      .\
	//      ..\
	//      ..\..\
	//      %EXE_DIR%\
	//      %EXE_DIR%\..\
	//      %EXE_DIR%\..\..\
	//      %EXE_DIR%\..\%EXE_NAME%
	//      %EXE_DIR%\..\..\%EXE_NAME%
	//      DXSDK media path

	// Search in .\  
	wcscpy_s( strSearchPath, cchSearch, strLeaf );
	if( GetFileAttributesW( strSearchPath ) != 0xFFFFFFFF )
		return true;

	// Search in ..\  
	swprintf_s( strSearchPath, cchSearch, L"..\\%s", strLeaf );
	if( GetFileAttributesW( strSearchPath ) != 0xFFFFFFFF )
		return true;

	// Search in ..\..\ 
	swprintf_s( strSearchPath, cchSearch, L"..\\..\\%s", strLeaf );
	if( GetFileAttributesW( strSearchPath ) != 0xFFFFFFFF )
		return true;

	// Search in ..\..\ 
	swprintf_s( strSearchPath, cchSearch, L"..\\..\\%s", strLeaf );
	if( GetFileAttributesW( strSearchPath ) != 0xFFFFFFFF )
		return true;

	// Search in the %EXE_DIR%\ 
	swprintf_s( strSearchPath, cchSearch, L"%s\\%s", strExePath, strLeaf );
	if( GetFileAttributesW( strSearchPath ) != 0xFFFFFFFF )
		return true;

	// Search in the %EXE_DIR%\..\ 
	swprintf_s( strSearchPath, cchSearch, L"%s\\..\\%s", strExePath, strLeaf );
	if( GetFileAttributesW( strSearchPath ) != 0xFFFFFFFF )
		return true;

	// Search in the %EXE_DIR%\..\..\ 
	swprintf_s( strSearchPath, cchSearch, L"%s\\..\\..\\%s", strExePath, strLeaf );
	if( GetFileAttributesW( strSearchPath ) != 0xFFFFFFFF )
		return true;

	// Search in "%EXE_DIR%\..\%EXE_NAME%\".  This matches the DirectX SDK layout
	swprintf_s( strSearchPath, cchSearch, L"%s\\..\\%s\\%s", strExePath, strExeName, strLeaf );
	if( GetFileAttributesW( strSearchPath ) != 0xFFFFFFFF )
		return true;

	// Search in "%EXE_DIR%\..\..\%EXE_NAME%\".  This matches the DirectX SDK layout
	swprintf_s( strSearchPath, cchSearch, L"%s\\..\\..\\%s\\%s", strExePath, strExeName, strLeaf );
	if( GetFileAttributesW( strSearchPath ) != 0xFFFFFFFF )
		return true;

	// Search in media search dir 
	WCHAR* s_strSearchPath = DXUTMediaSearchPath();
	if( s_strSearchPath[0] != 0 )
	{
		swprintf_s( strSearchPath, cchSearch, L"%s%s", s_strSearchPath, strLeaf );
		if( GetFileAttributesW( strSearchPath ) != 0xFFFFFFFF )
			return true;
	}

	return false;
}


HRESULT WINAPI FindMediaFileCch( WCHAR* strDestPath, int cchDest, 
                                          LPCWSTR strFilename )
{
    bool bFound;
    WCHAR strSearchFor[MAX_PATH];

    if( NULL == strFilename || strFilename[0] == 0 || NULL == strDestPath || cchDest < 10 )
        return E_INVALIDARG;

    // Get the exe name, and exe path
    WCHAR strExePath[MAX_PATH] =
    {
        0
    };
    WCHAR strExeName[MAX_PATH] =
    {
        0
    };
    WCHAR* strLastSlash = NULL;
    GetModuleFileNameW( NULL, strExePath, MAX_PATH );
    strExePath[MAX_PATH - 1] = 0;
    strLastSlash = wcsrchr( strExePath, TEXT( '\\' ) );
    if( strLastSlash )
    {
        wcscpy_s( strExeName, MAX_PATH, &strLastSlash[1] );

        // Chop the exe name from the exe path
        *strLastSlash = 0;

        // Chop the .exe from the exe name
        strLastSlash = wcsrchr( strExeName, TEXT( '.' ) );
        if( strLastSlash )
            *strLastSlash = 0;
    }

    // Typical directories:
    //      .\
    //      ..\
    //      ..\..\
    //      %EXE_DIR%\
    //      %EXE_DIR%\..\
    //      %EXE_DIR%\..\..\
    //      %EXE_DIR%\..\%EXE_NAME%
    //      %EXE_DIR%\..\..\%EXE_NAME%

    // Typical directory search
    bFound = FindMediaSearchTypicalDirs( strDestPath, cchDest, strFilename, strExePath, strExeName );
    if( bFound )
        return S_OK;

    // Typical directory search again, but also look in a subdir called "\media\" 
    swprintf_s( strSearchFor, MAX_PATH, L"media\\%s", strFilename );
    bFound = FindMediaSearchTypicalDirs( strDestPath, cchDest, strSearchFor, strExePath, strExeName );
    if( bFound )
        return S_OK;

    WCHAR strLeafName[MAX_PATH] =
    {
        0
    };

    // Search all parent directories starting at .\ and using strFilename as the leaf name
    wcscpy_s( strLeafName, MAX_PATH, strFilename );
    bFound = FindMediaSearchParentDirs( strDestPath, cchDest, L".", strLeafName );
    if( bFound )
        return S_OK;

    // Search all parent directories starting at the exe's dir and using strFilename as the leaf name
    bFound = FindMediaSearchParentDirs( strDestPath, cchDest, strExePath, strLeafName );
    if( bFound )
        return S_OK;

    // Search all parent directories starting at .\ and using "media\strFilename" as the leaf name
    swprintf_s( strLeafName, MAX_PATH, L"media\\%s", strFilename );
    bFound = FindMediaSearchParentDirs( strDestPath, cchDest, L".", strLeafName );
    if( bFound )
        return S_OK;

    // Search all parent directories starting at the exe's dir and using "media\strFilename" as the leaf name
    bFound = FindMediaSearchParentDirs( strDestPath, cchDest, strExePath, strLeafName );
    if( bFound )
        return S_OK;

    // On failure, return the file as the path but also return an error code
    wcscpy_s( strDestPath, cchDest, strFilename );

    return E_FAIL;
}

HRESULT loadTextureFromFile(LPCSTR file, LPCSTR shaderTextureName, ID3D11Device* pd3dDevice, ID3DX11Effect* pEffect)
{

	HRESULT hr;

	D3DX11_IMAGE_LOAD_INFO texLoadInfo;
	texLoadInfo.MipLevels = 20;
	texLoadInfo.MipFilter = D3DX11_FILTER_TRIANGLE;
	texLoadInfo.Filter = D3DX11_FILTER_TRIANGLE;
	// texLoadInfo.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	ID3D11Resource *pRes = NULL;

	D3DX11CreateTextureFromFile(pd3dDevice, file, &texLoadInfo, NULL, &pRes, NULL);
	if (pRes)
	{
		ID3D11Texture2D* texture;

		pRes->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID*)&texture);
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		ZeroMemory(&SRVDesc, sizeof(SRVDesc));
		SRVDesc.Format = desc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = desc.MipLevels;

		ID3D11ShaderResourceView* textureRview;
		pd3dDevice->CreateShaderResourceView(texture, &SRVDesc, &textureRview);
		ID3DX11EffectShaderResourceVariable* textureRVar = pEffect->GetVariableByName(shaderTextureName)->AsShaderResource();
		textureRVar->SetResource(textureRview);

		SAFE_RELEASE(texture);
		SAFE_RELEASE(textureRview);
	}

	SAFE_RELEASE(pRes);
	return S_OK;
}


HRESULT loadTextureFromFile(LPCSTR file, LPCSTR shaderTextureName, ID3D11Device* pd3dDevice, ID3DX11Effect* pEffect, ID3D11ShaderResourceView** pOut)
{

	HRESULT hr;

	D3DX11_IMAGE_LOAD_INFO texLoadInfo;
	texLoadInfo.MipLevels = 20;
	texLoadInfo.MipFilter = D3DX11_FILTER_TRIANGLE;
	texLoadInfo.Filter = D3DX11_FILTER_TRIANGLE;
	// texLoadInfo.Format = DXGI_FORMAT_R16G16B16A16_UNORM;
	ID3D11Resource *pRes = NULL;

	D3DX11CreateTextureFromFile(pd3dDevice, file, &texLoadInfo, NULL, &pRes, NULL);
	if (pRes)
	{
		ID3D11Texture2D* texture;

		pRes->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID*)&texture);
		D3D11_TEXTURE2D_DESC desc;
		texture->GetDesc(&desc);
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		ZeroMemory(&SRVDesc, sizeof(SRVDesc));
		SRVDesc.Format = desc.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = desc.MipLevels;

		ID3D11ShaderResourceView* textureRview;
		pd3dDevice->CreateShaderResourceView(texture, &SRVDesc, &textureRview);
		ID3DX11EffectShaderResourceVariable* textureRVar = pEffect->GetVariableByName(shaderTextureName)->AsShaderResource();
		textureRVar->SetResource(textureRview);
		*pOut = textureRview;
		SAFE_RELEASE(texture);
		//SAFE_RELEASE(textureRview);
	}

	SAFE_RELEASE(pRes);
	return S_OK;
}

HRESULT CreateTextureFromFile( ID3D11Device* pDevice, ID3D11DeviceContext *pContext, LPCTSTR pSrcFile,
	ID3D11ShaderResourceView** ppOutputRV, bool bSRGB )
{
	return CreateTextureFromFileEx( pDevice, pContext, pSrcFile, NULL, NULL, ppOutputRV, bSRGB );
}

HRESULT CreateTextureFromFileEx( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LPCTSTR pSrcFile,
	D3DX11_IMAGE_LOAD_INFO* pLoadInfo, ID3DX11ThreadPump* pPump,
	ID3D11ShaderResourceView** ppOutputRV, bool bSRGB )
{

	//bool is10L9 = .DeviceFeatureLevel < D3D_FEATURE_LEVEL_10_0; 
	HRESULT hr = S_OK;
	D3DX11_IMAGE_LOAD_INFO ZeroInfo;	//D3DX11_IMAGE_LOAD_INFO has a default constructor
	D3DX11_IMAGE_INFO SrcInfo;

	if( !pLoadInfo )
	{
		pLoadInfo = &ZeroInfo;
	}

	if( !pLoadInfo->pSrcInfo )
	{
		D3DX11GetImageInfoFromFile( pSrcFile, NULL, &SrcInfo, NULL );
		pLoadInfo->pSrcInfo = &SrcInfo;

		pLoadInfo->Format = pLoadInfo->pSrcInfo->Format;
	}

	// Search the cache for a matching entry.
	//for( int i = 0; i < m_TextureCache.GetSize(); ++i )
	//{
	//	DXUTCache_Texture& Entry = m_TextureCache[i];
	//	if( Entry.Location == DXUTCACHE_LOCATION_FILE &&
	//		!lstrcmpW( Entry.wszSource, pSrcFile ) &&
	//		Entry.Width == pLoadInfo->Width &&
	//		Entry.Height == pLoadInfo->Height &&
	//		Entry.MipLevels == pLoadInfo->MipLevels &&
	//		Entry.Usage11 == pLoadInfo->Usage &&
	//		Entry.Format == pLoadInfo->Format &&
	//		Entry.CpuAccessFlags == pLoadInfo->CpuAccessFlags &&
	//		Entry.BindFlags == pLoadInfo->BindFlags &&
	//		Entry.MiscFlags == pLoadInfo->MiscFlags )
	//	{
	//		// A match is found. Obtain the IDirect3DTexture9 interface and return that.
	//		return Entry.pSRV11->QueryInterface( __uuidof( ID3D11ShaderResourceView ), ( LPVOID* )ppOutputRV );
	//	}
	//}

//#if defined(PROFILE) || defined(DEBUG)
//	CHAR strFileA[MAX_PATH];
//	WideCharToMultiByte( CP_ACP, 0, pSrcFile, -1, strFileA, MAX_PATH, NULL, FALSE );
//	CHAR* pstrName = strrchr( strFileA, '\\' );
//	if( pstrName == NULL )
//		pstrName = strFileA;
//	else
//		pstrName++;
//#endif

	//Ready a new entry to the texture cache
	//Do this before creating the texture since pLoadInfo may be volatile
	//DXUTCache_Texture NewEntry;
	//NewEntry.Location = DXUTCACHE_LOCATION_FILE;
	//wcscpy_s( NewEntry.wszSource, MAX_PATH, pSrcFile );
	//NewEntry.Width = pLoadInfo->Width;
	//NewEntry.Height = pLoadInfo->Height;
	//NewEntry.MipLevels = pLoadInfo->MipLevels;
	//NewEntry.Usage11 = pLoadInfo->Usage;
	//// 10L9 can't handle typesless, so we cant make a typesless format 
	//if (is10L9 && bSRGB) {
	//	NewEntry.Format = MAKE_SRGB(pLoadInfo->Format);
	//}else {
	//	NewEntry.Format = pLoadInfo->Format;
	//}
	//NewEntry.CpuAccessFlags = pLoadInfo->CpuAccessFlags;
	//NewEntry.BindFlags = pLoadInfo->BindFlags;
	//NewEntry.MiscFlags = pLoadInfo->MiscFlags;

	//Create the rexture
	ID3D11Texture2D* pRes = NULL;
	hr = D3DX11CreateTextureFromFile( pDevice, pSrcFile, pLoadInfo, pPump, ( ID3D11Resource** )&pRes, NULL );

	if( FAILED( hr ) )
		return hr;
	D3D11_TEXTURE2D_DESC tex_dsc;
	pRes->GetDesc(&tex_dsc);



	if (bSRGB ) {
		// This is a workaround so that we can load linearly, but sample in SRGB.  Right now, we can't load
		// as linear since D3DX will try to do conversion on load.  Loading as TYPELESS doesn't work either, and
		// loading as typed _UNORM doesn't allow us to create an SRGB view.

		// on d3d11 featuer levels this is just a copy, but on 10L9 we must use a cpu side copy with 2 staging resources.
		ID3D11Texture2D* unormStaging = NULL;
		ID3D11Texture2D* srgbStaging = NULL;

		D3D11_TEXTURE2D_DESC CopyDesc;
		pRes->GetDesc( &CopyDesc );

		pLoadInfo->BindFlags = 0;
		pLoadInfo->CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		pLoadInfo->Depth = 0;
		pLoadInfo->Filter = D3DX11_FILTER_LINEAR;
		pLoadInfo->FirstMipLevel = 0;
		pLoadInfo->Format = CopyDesc.Format;
		pLoadInfo->Height = CopyDesc.Height;
		pLoadInfo->MipFilter = D3DX11_FILTER_LINEAR;
		pLoadInfo->MiscFlags = CopyDesc.MiscFlags;
		pLoadInfo->Usage = D3D11_USAGE_STAGING;
		pLoadInfo->Width = CopyDesc.Width;

		CopyDesc.BindFlags = 0;
		CopyDesc.Usage = D3D11_USAGE_STAGING;
		CopyDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
		CopyDesc.Format = MAKE_SRGB(CopyDesc.Format);

		hr = D3DX11CreateTextureFromFile( pDevice, pSrcFile, pLoadInfo, pPump, ( ID3D11Resource** )&unormStaging, NULL );
		//DXUT_SetDebugName( unormStaging, "CDXUTResourceCache" );

		hr = pDevice->CreateTexture2D(&CopyDesc, NULL, &srgbStaging);
		//DXUT_SetDebugName( srgbStaging, "CDXUTResourceCache" );
		pContext->CopyResource( srgbStaging, unormStaging );
		ID3D11Texture2D* srgbGPU;

		pRes->GetDesc( &CopyDesc );
		CopyDesc.Format = MAKE_SRGB(CopyDesc.Format);
		hr = pDevice->CreateTexture2D(&CopyDesc, NULL, &srgbGPU);
		pContext->CopyResource( srgbGPU, srgbStaging );

		SAFE_RELEASE(pRes);
		SAFE_RELEASE(srgbStaging);
		SAFE_RELEASE(unormStaging);
		pRes = srgbGPU;
	}

	//DXUT_SetDebugName( pRes, pstrName );

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	if( bSRGB )
		SRVDesc.Format = MAKE_SRGB( ZeroInfo.Format );
	else
		SRVDesc.Format = ZeroInfo.Format;
	if( pLoadInfo->pSrcInfo->ResourceDimension == D3D11_RESOURCE_DIMENSION_TEXTURE1D )
	{
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
		SRVDesc.Texture1D.MostDetailedMip = 0;
		SRVDesc.Texture1D.MipLevels = pLoadInfo->pSrcInfo->MipLevels;
	}
	else if( pLoadInfo->pSrcInfo->ResourceDimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D )
	{
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = pLoadInfo->pSrcInfo->MipLevels;

		if( pLoadInfo->pSrcInfo->MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE )
		{
			SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			SRVDesc.TextureCube.MostDetailedMip = 0;
			SRVDesc.TextureCube.MipLevels = pLoadInfo->pSrcInfo->MipLevels;
		}
	}
	else if( pLoadInfo->pSrcInfo->ResourceDimension == D3D11_RESOURCE_DIMENSION_TEXTURE3D )
	{
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
		SRVDesc.Texture3D.MostDetailedMip = 0;
		SRVDesc.Texture3D.MipLevels = pLoadInfo->pSrcInfo->MipLevels;
	}
	if (bSRGB) {
		SRVDesc.Format = MAKE_SRGB(tex_dsc.Format);
	}else {
		SRVDesc.Format = tex_dsc.Format;
	}
	SRVDesc.Texture2D.MipLevels = tex_dsc.MipLevels;
	SRVDesc.Texture2D.MostDetailedMip = 0;
	hr = pDevice->CreateShaderResourceView( pRes, &SRVDesc, ppOutputRV );
	pRes->Release();
	if( FAILED( hr ) )
		return hr;
	
	//( *ppOutputRV )->QueryInterface( __uuidof( ID3D11ShaderResourceView ), ( LPVOID* )&NewEntry.pSRV11 );

	//m_TextureCache.Add( NewEntry );

	return S_OK;
}
