#pragma once 



HRESULT loadTextureFromFile(LPCSTR file, LPCSTR shaderTextureName, ID3D11Device* pd3dDevice, ID3DX11Effect* pEffect);
HRESULT loadTextureFromFile(LPCSTR file, LPCSTR shaderTextureName, ID3D11Device* pd3dDevice, ID3DX11Effect* pEffect, ID3D11ShaderResourceView** pOut);
HRESULT CreateTextureFromFile( ID3D11Device* pDevice, ID3D11DeviceContext *pContext, LPCTSTR pSrcFile, ID3D11ShaderResourceView** ppOutputRV, bool bSRGB = false );
HRESULT CreateTextureFromFileEx( ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LPCTSTR pSrcFile, D3DX11_IMAGE_LOAD_INFO* pLoadInfo, ID3DX11ThreadPump* pPump, ID3D11ShaderResourceView** ppOutputRV, bool bSRGB = false );
bool FindMediaSearchTypicalDirs( WCHAR* strSearchPath, int cchSearch, LPCWSTR strLeaf, WCHAR* strExePath, WCHAR* strExeName );
HRESULT WINAPI FindMediaFileCch( WCHAR* strDestPath, int cchDest, LPCWSTR strFilename );
bool FindMediaSearchParentDirs( WCHAR* strSearchPath, int cchSearch, WCHAR* strStartAt, WCHAR* strLeafName );