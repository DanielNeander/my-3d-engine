// Copyright (c) 2011 NVIDIA Corporation. All rights reserved.
//
// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
// *AS IS*  AND NVIDIA AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, NONINFRINGEMENT,IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  NVIDIA 
// OR ITS SUPPLIERS BE  LIABLE  FOR  ANY  DIRECT, SPECIAL,  INCIDENTAL,  INDIRECT,  OR  
// CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS 
// OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY 
// OTHER PECUNIARY LOSS) ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, 
// EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
// Please direct any bugs or questions to SDKFeedback@nvidia.com

#include "stdafx.h"
#include "skybox11.h"

#define DXUT_SetDebugName(a, b)	;

//--------------------------------------------------------------------------------------
// Helper function to compile an hlsl shader from file, 
// its binary compiled code is returned
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    // find the file    
    // open the file
    HANDLE hFile = CreateFile( szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN, NULL );
    if( INVALID_HANDLE_VALUE == hFile )
        return E_FAIL;

    // Get the file size
    LARGE_INTEGER FileSize;
    GetFileSizeEx( hFile, &FileSize );

    // create enough space for the file data
    BYTE* pFileData = new BYTE[ FileSize.LowPart ];
    if( !pFileData )
        return E_OUTOFMEMORY;

    // read the data in
    DWORD BytesRead;
    if( !ReadFile( hFile, pFileData, FileSize.LowPart, &BytesRead, NULL ) )
        return E_FAIL; 

    CloseHandle( hFile );

    // Compile the shader
    char pFilePathName[MAX_PATH];        
	char pMaxSizeValue[MAX_PATH];
    //WideCharToMultiByte(CP_ACP, 0, str, -1, pFilePathName, MAX_PATH, NULL, NULL);
    ID3DBlob* pErrorBlob;
	D3D_SHADER_MACRO defines[2] = { { "MAXSIZE", pMaxSizeValue }, NULL };
	hr = D3DCompile( pFileData, FileSize.LowPart, szFileName, defines, NULL, szEntryPoint, szShaderModel, D3D10_SHADER_ENABLE_STRICTNESS, 0, ppBlobOut, &pErrorBlob );

    delete []pFileData;

    if( FAILED(hr) )
    {
        OutputDebugStringA( (char*)pErrorBlob->GetBufferPointer() );
        SAFE_RELEASE( pErrorBlob );
        return hr;
    }
    SAFE_RELEASE( pErrorBlob );

    return S_OK;
}

struct SKYBOX_VERTEX
{
    D3DXVECTOR4 pos;
};

const D3D11_INPUT_ELEMENT_DESC g_aVertexLayout[] =
{
    { "POSITION",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

CSkybox11::CSkybox11()
{
    m_pEnvironmentMap11 = NULL;
    m_pEnvironmentRV11 = NULL;
    m_pd3dDevice11 = NULL;
    m_pVertexShader = NULL;
    m_pPixelShader = NULL;
    m_pSam = NULL;
    m_pVertexLayout11 = NULL;
    m_pcbVSPerObject = NULL;
    m_pVB11 = NULL;
    m_pDepthStencilState11 = NULL;
	m_pRasterizerState11 = NULL;

    m_fSize = 1.0f;
}

HRESULT CSkybox11::OnD3D11CreateDevice2( ID3D11Device* pd3dDevice, float fSize, 
	ID3D11Texture2D* pCubeTexture, ID3D11ShaderResourceView* pCubeRV )
{
	HRESULT hr;

	m_pd3dDevice11 = pd3dDevice;
	m_fSize = fSize;
	m_pEnvironmentMap11 = pCubeTexture;
	m_pEnvironmentRV11 = pCubeRV;

	ID3DBlob* pBlobVS = NULL;
	ID3DBlob* pBlobPS = NULL;

	// Create the shaders
	( CompileShaderFromFile( "Data\\Shaders\\skybox11_day.hlsl", "SkyboxVS", "vs_4_0", &pBlobVS ) );
	( CompileShaderFromFile( "Data\\Shaders\\skybox11_day.hlsl", "SkyboxPS", "ps_4_0", &pBlobPS ) );

	( pd3dDevice->CreateVertexShader( pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &m_pVertexShader ) );
	( pd3dDevice->CreatePixelShader( pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &m_pPixelShader ) );

	//DXUT_SetDebugName( m_pVertexShader, "SkyboxVS" );
	//DXUT_SetDebugName( m_pPixelShader, "SkyboxPS" );

	// Create an input layout
	( pd3dDevice->CreateInputLayout( g_aVertexLayout, 1, pBlobVS->GetBufferPointer(),
		pBlobVS->GetBufferSize(), &m_pVertexLayout11 ) );
	DXUT_SetDebugName( m_pVertexLayout11, "Primary" );

	SAFE_RELEASE( pBlobVS );
	SAFE_RELEASE( pBlobPS );

	// Query support for linear filtering on DXGI_FORMAT_R32G32B32A32
	UINT FormatSupport = 0;
	( pd3dDevice->CheckFormatSupport( DXGI_FORMAT_R32G32B32A32_FLOAT, &FormatSupport ) );

	// Setup linear or point sampler according to the format Query result
	D3D11_SAMPLER_DESC SamDesc;
	SamDesc.Filter = ( FormatSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE ) > 0 ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
	SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SamDesc.MipLODBias = 0.0f;
	SamDesc.MaxAnisotropy = 1;
	SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0;
	SamDesc.MinLOD = 0;
	SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
	( pd3dDevice->CreateSamplerState( &SamDesc, &m_pSam ) );  
	DXUT_SetDebugName( m_pSam, "Primary" );

	// Setup constant buffer
	D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE_DYNAMIC;
	Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = 0;
	Desc.ByteWidth = sizeof( CB_VS_PER_OBJECT );
	( pd3dDevice->CreateBuffer( &Desc, NULL, &m_pcbVSPerObject ) );
	DXUT_SetDebugName( m_pcbVSPerObject, "CB_VS_PER_OBJECT" );

	// Depth stencil state
	D3D11_DEPTH_STENCIL_DESC DSDesc;
	ZeroMemory( &DSDesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
	DSDesc.DepthEnable = FALSE;
	DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DSDesc.StencilEnable = FALSE;
	( pd3dDevice->CreateDepthStencilState( &DSDesc, &m_pDepthStencilState11 ) );
	DXUT_SetDebugName( m_pDepthStencilState11, "DepthStencil" );

	// set default render state to msaa enabled
	D3D11_RASTERIZER_DESC drd = {
		D3D11_FILL_SOLID, //D3D11_FILL_MODE FillMode;
		D3D11_CULL_NONE,//D3D11_CULL_MODE CullMode;
		FALSE, //BOOL FrontCounterClockwise;
		0, //INT DepthBias;
		0.0f,//FLOAT DepthBiasClamp;
		0.0f,//FLOAT SlopeScaledDepthBias;
		FALSE,//BOOL DepthClipEnable;
		FALSE,//BOOL ScissorEnable;
		TRUE,//BOOL MultisampleEnable;
		FALSE//BOOL AntialiasedLineEnable;        
	};
	pd3dDevice->CreateRasterizerState(&drd, &m_pRasterizerState11);

	return S_OK;
}


HRESULT CSkybox11::OnD3D11CreateDevice( ID3D11Device* pd3dDevice, float fSize, 
                                        ID3D11Texture2D* pCubeTexture, ID3D11ShaderResourceView* pCubeRV )
{
    HRESULT hr;
    
    m_pd3dDevice11 = pd3dDevice;
    m_fSize = fSize;
    m_pEnvironmentMap11 = pCubeTexture;
    m_pEnvironmentRV11 = pCubeRV;

    ID3DBlob* pBlobVS = NULL;
    ID3DBlob* pBlobPS = NULL;

    // Create the shaders
    ( CompileShaderFromFile( "Data\\Shaders\\TerrainTessellation\\skybox11_night.hlsl", "SkyboxVS", "vs_4_0", &pBlobVS ) );
    ( CompileShaderFromFile( "Data\\Shaders\\TerrainTessellation\\skybox11_night.hlsl", "SkyboxPS", "ps_4_0", &pBlobPS ) );

    ( pd3dDevice->CreateVertexShader( pBlobVS->GetBufferPointer(), pBlobVS->GetBufferSize(), NULL, &m_pVertexShader ) );
    ( pd3dDevice->CreatePixelShader( pBlobPS->GetBufferPointer(), pBlobPS->GetBufferSize(), NULL, &m_pPixelShader ) );

    //DXUT_SetDebugName( m_pVertexShader, "SkyboxVS" );
    //DXUT_SetDebugName( m_pPixelShader, "SkyboxPS" );

    // Create an input layout
    ( pd3dDevice->CreateInputLayout( g_aVertexLayout, 1, pBlobVS->GetBufferPointer(),
                                             pBlobVS->GetBufferSize(), &m_pVertexLayout11 ) );
    DXUT_SetDebugName( m_pVertexLayout11, "Primary" );

    SAFE_RELEASE( pBlobVS );
    SAFE_RELEASE( pBlobPS );

    // Query support for linear filtering on DXGI_FORMAT_R32G32B32A32
    UINT FormatSupport = 0;
    ( pd3dDevice->CheckFormatSupport( DXGI_FORMAT_R32G32B32A32_FLOAT, &FormatSupport ) );

    // Setup linear or point sampler according to the format Query result
    D3D11_SAMPLER_DESC SamDesc;
    SamDesc.Filter = ( FormatSupport & D3D11_FORMAT_SUPPORT_SHADER_SAMPLE ) > 0 ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
    SamDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    SamDesc.MipLODBias = 0.0f;
    SamDesc.MaxAnisotropy = 1;
    SamDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    SamDesc.BorderColor[0] = SamDesc.BorderColor[1] = SamDesc.BorderColor[2] = SamDesc.BorderColor[3] = 0;
    SamDesc.MinLOD = 0;
    SamDesc.MaxLOD = D3D11_FLOAT32_MAX;
    ( pd3dDevice->CreateSamplerState( &SamDesc, &m_pSam ) );  
    DXUT_SetDebugName( m_pSam, "Primary" );

    // Setup constant buffer
    D3D11_BUFFER_DESC Desc;
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;
    Desc.ByteWidth = sizeof( CB_VS_PER_OBJECT );
    ( pd3dDevice->CreateBuffer( &Desc, NULL, &m_pcbVSPerObject ) );
    DXUT_SetDebugName( m_pcbVSPerObject, "CB_VS_PER_OBJECT" );
    
    // Depth stencil state
    D3D11_DEPTH_STENCIL_DESC DSDesc;
    ZeroMemory( &DSDesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
    DSDesc.DepthEnable = FALSE;
    DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
    DSDesc.StencilEnable = FALSE;
    ( pd3dDevice->CreateDepthStencilState( &DSDesc, &m_pDepthStencilState11 ) );
    DXUT_SetDebugName( m_pDepthStencilState11, "DepthStencil" );

    // set default render state to msaa enabled
    D3D11_RASTERIZER_DESC drd = {
        D3D11_FILL_SOLID, //D3D11_FILL_MODE FillMode;
        D3D11_CULL_NONE,//D3D11_CULL_MODE CullMode;
        FALSE, //BOOL FrontCounterClockwise;
        0, //INT DepthBias;
        0.0f,//FLOAT DepthBiasClamp;
        0.0f,//FLOAT SlopeScaledDepthBias;
        FALSE,//BOOL DepthClipEnable;
        FALSE,//BOOL ScissorEnable;
        TRUE,//BOOL MultisampleEnable;
        FALSE//BOOL AntialiasedLineEnable;        
    };
    pd3dDevice->CreateRasterizerState(&drd, &m_pRasterizerState11);
	
	return S_OK;
}

void CSkybox11::OnD3D11ResizedSwapChain( const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
    HRESULT hr;

    if ( m_pd3dDevice11 == NULL )
        return;

    // Fill the vertex buffer
    SKYBOX_VERTEX* pVertex = new SKYBOX_VERTEX[4];
    if ( !pVertex )
        return;

    // Map texels to pixels 
    float fHighW = -1.0f - ( 1.0f / ( float )pBackBufferSurfaceDesc->Width );
    float fHighH = -1.0f - ( 1.0f / ( float )pBackBufferSurfaceDesc->Height );
    float fLowW = 1.0f + ( 1.0f / ( float )pBackBufferSurfaceDesc->Width );
    float fLowH = 1.0f + ( 1.0f / ( float )pBackBufferSurfaceDesc->Height );
    
    pVertex[0].pos = D3DXVECTOR4( fLowW, fLowH, 1.0f, 1.0f );
    pVertex[1].pos = D3DXVECTOR4( fLowW, fHighH, 1.0f, 1.0f );
    pVertex[2].pos = D3DXVECTOR4( fHighW, fLowH, 1.0f, 1.0f );
    pVertex[3].pos = D3DXVECTOR4( fHighW, fHighH, 1.0f, 1.0f );

    UINT uiVertBufSize = 4 * sizeof( SKYBOX_VERTEX );
    //Vertex Buffer
    D3D11_BUFFER_DESC vbdesc;
    vbdesc.ByteWidth = uiVertBufSize;
    vbdesc.Usage = D3D11_USAGE_IMMUTABLE;
    vbdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbdesc.CPUAccessFlags = 0;
    vbdesc.MiscFlags = 0;    

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = pVertex;    
    V( m_pd3dDevice11->CreateBuffer( &vbdesc, &InitData, &m_pVB11 ) );
    DXUT_SetDebugName( m_pVB11, "SkyBox" );
    SAFE_DELETE_ARRAY( pVertex ); 
}

void CSkybox11::StoreD3D11State( ID3D11DeviceContext* pd3dImmediateContext )
{
    pd3dImmediateContext->OMGetDepthStencilState( &m_pDepthStencilStateStored11, &m_StencilRefStored11 );
    pd3dImmediateContext->RSGetState( &m_pRasterizerStateStored11 );
    pd3dImmediateContext->OMGetBlendState( &m_pBlendStateStored11, m_BlendFactorStored11, &m_SampleMaskStored11 );
    pd3dImmediateContext->PSGetSamplers( 0, 1, &m_pSamplerStateStored11 );
}

void CSkybox11::RestoreD3D11State( ID3D11DeviceContext* pd3dImmediateContext )
{
    pd3dImmediateContext->OMSetDepthStencilState( m_pDepthStencilStateStored11, m_StencilRefStored11 );
    pd3dImmediateContext->RSSetState( m_pRasterizerStateStored11 );
    pd3dImmediateContext->OMSetBlendState( m_pBlendStateStored11, m_BlendFactorStored11, m_SampleMaskStored11 );
    pd3dImmediateContext->PSSetSamplers( 0, 1, &m_pSamplerStateStored11 );

    SAFE_RELEASE( m_pDepthStencilStateStored11 );
    SAFE_RELEASE( m_pRasterizerStateStored11 );
    SAFE_RELEASE( m_pBlendStateStored11 );
    SAFE_RELEASE( m_pSamplerStateStored11 );
}

void CSkybox11::D3D11Render( D3DXMATRIX* pmWorldViewProj, ID3D11DeviceContext* pd3dImmediateContext )
{
    HRESULT hr;
    
	StoreD3D11State(pd3dImmediateContext);
    pd3dImmediateContext->IASetInputLayout( m_pVertexLayout11 );

    UINT uStrides = sizeof( SKYBOX_VERTEX );
    UINT uOffsets = 0;
    ID3D11Buffer* pBuffers[1] = { m_pVB11 };
    pd3dImmediateContext->IASetVertexBuffers( 0, 1, pBuffers, &uStrides, &uOffsets );
    pd3dImmediateContext->IASetIndexBuffer( NULL, DXGI_FORMAT_R32_UINT, 0 );
    pd3dImmediateContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP );

    pd3dImmediateContext->VSSetShader( m_pVertexShader, NULL, 0 );
    pd3dImmediateContext->GSSetShader( NULL, NULL, 0 );
    pd3dImmediateContext->HSSetShader( NULL, NULL, 0 );
    pd3dImmediateContext->DSSetShader( NULL, NULL, 0 );
    pd3dImmediateContext->PSSetShader( m_pPixelShader, NULL, 0 );

    D3D11_MAPPED_SUBRESOURCE MappedResource;
    pd3dImmediateContext->Map( m_pcbVSPerObject, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource );
    CB_VS_PER_OBJECT* pVSPerObject = ( CB_VS_PER_OBJECT* )MappedResource.pData;  
    D3DXMatrixInverse( &pVSPerObject->m_WorldViewProj, NULL, pmWorldViewProj );
    pd3dImmediateContext->Unmap( m_pcbVSPerObject, 0 );
    pd3dImmediateContext->VSSetConstantBuffers( 0, 1, &m_pcbVSPerObject );

    pd3dImmediateContext->PSSetSamplers( 0, 1, &m_pSam );
    pd3dImmediateContext->PSSetShaderResources( 0, 1, &m_pEnvironmentRV11 );

    pd3dImmediateContext->OMSetDepthStencilState( m_pDepthStencilState11, 0 );
	pd3dImmediateContext->OMSetBlendState(NULL, 0, 0xffffffff);
    pd3dImmediateContext->RSSetState(m_pRasterizerState11);

	pd3dImmediateContext->Draw( 4, 0 );

	RestoreD3D11State(pd3dImmediateContext);
}

void CSkybox11::OnD3D11ReleasingSwapChain()
{
    SAFE_RELEASE( m_pVB11 );
}

void CSkybox11::OnD3D11DestroyDevice()
{
    m_pd3dDevice11 = NULL;
    SAFE_RELEASE( m_pEnvironmentRV11 );
    SAFE_RELEASE( m_pEnvironmentMap11 );
    SAFE_RELEASE( m_pSam );
    SAFE_RELEASE( m_pVertexShader );
    SAFE_RELEASE( m_pPixelShader );
    SAFE_RELEASE( m_pVertexLayout11 );
    SAFE_RELEASE( m_pcbVSPerObject );
    SAFE_RELEASE( m_pDepthStencilState11 );
    SAFE_RELEASE( m_pRasterizerState11 );
}