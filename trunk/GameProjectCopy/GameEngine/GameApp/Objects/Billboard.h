// Copyright ?2008 Intel Corporation
// All Rights Reserved
// 
// The sample source code contained or described herein and all documents
// related to the sample source code ("Material") are owned by Intel Corporation
// or its suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and
// proprietary and confidential information of Intel Corporation and its suppliers
// and licensors, and is protected by worldwide copyright and trade secret laws and
// treaty provisions. The sample source code is provided AS IS, with no warranties
// of any kind, express or implied. Except as expressly permitted by the Software
// license, neither Intel Corporation nor its suppliers assumes any responsibility
// or liability for any errors or inaccuracies that may appear herein.


#pragma once
#pragma warning ( disable : 4995 4996 )
#include <d3d111.h>
#include <d3dx11.h>
#include <string.h>

struct SimpleVertex
{
	D3DXVECTOR3 Pos;
	D3DXVECTOR2 Tex;
};

// Define a default input layout
static const D3D11_INPUT_ELEMENT_DESC defaultlayout[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
static const UINT defaultnumElements = sizeof( defaultlayout ) / sizeof( defaultlayout[0] );


template < class T_VERTEX >
class Billboard 
{
	int									vertexCnt;
	T_VERTEX*							pVertices;
	D3D11_INPUT_ELEMENT_DESC*			pLayout;
	int									numElements;
	ID3D11InputLayout*                  pVertexLayout;
	ID3D11Buffer*                       pVertexBuffer;
	TCHAR								pTextureName[MAX_PATH];
	ID3D11ShaderResourceView*           pTextureRV;
	ID3DX11EffectTechnique*              pRenderTechnique;

public:
	Billboard( );
	virtual ~Billboard(){};

	virtual void	Init		 ( TCHAR* texName, T_VERTEX* pInVertices, int numVerts, D3D11_INPUT_ELEMENT_DESC* layout = (D3D11_INPUT_ELEMENT_DESC*)defaultlayout, int numLayoutElements = defaultnumElements, 
									WORD* indices = 0, int numIndices = 0 );
	virtual void	Destroy		 ( ) { };
	
	virtual HRESULT CreateDevice ( ID3D11Device* pd3dDevice, ID3DX11EffectTechnique* pTechique );
	
	virtual void	DestroyDevice( );
	
	virtual void	Render		 ( ID3D11Device* pd3dDevice, ID3DX11EffectShaderResourceVariable* pDiffuseVariable );

};

template < class T_VERTEX  >
HRESULT Billboard<T_VERTEX >
::CreateDevice( ID3D11Device* pd3dDevice, ID3DX11EffectTechnique* pTechique )
{
	HRESULT hr		 = S_OK;
	pRenderTechnique = pTechique;

	// Create the input layout
	D3D11_PASS_DESC PassDesc;
	pRenderTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	hr = pd3dDevice->CreateInputLayout( pLayout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &pVertexLayout );
	V_RETURN( hr );

	// Initialize Vertex Buffers
	D3D11_BUFFER_DESC		bd;
	D3D11_SUBRESOURCE_DATA	InitData;

	bd.Usage			= D3D11_USAGE_DEFAULT;
	bd.ByteWidth		= sizeof( T_VERTEX ) * vertexCnt;
	bd.BindFlags		= D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags	= 0;
	bd.MiscFlags		= 0;
	InitData.pSysMem	= pVertices;

	V_RETURN( pd3dDevice->CreateBuffer( &bd, &InitData, &pVertexBuffer ));

	// Load the Texture
	V_RETURN( D3DX10CreateShaderResourceViewFromFile( pd3dDevice, pTextureName, NULL, NULL, &pTextureRV, NULL ));

	return hr;

}

template < class T_VERTEX  >
Billboard<T_VERTEX >::Billboard()
{
	pVertexLayout	 = NULL;
	pVertexBuffer	 = NULL;
	pTextureRV		 = NULL;
	pRenderTechnique = NULL;

}

template < class T_VERTEX  >
void Billboard<T_VERTEX >
::Init( TCHAR* texName, T_VERTEX* pInVertices, int numVerts, D3D11_INPUT_ELEMENT_DESC* layout, int numLayoutElements, WORD* indices /*= 0*/, int numIndices /*= 0 */ )
{
	_tcscpy(pTextureName, texName);
	pVertices	= pInVertices;
	vertexCnt	= numVerts;
	pLayout		= layout;
	numElements = numLayoutElements;
}



template < class T_VERTEX  >
void Billboard<T_VERTEX >
::DestroyDevice()
{
	SAFE_RELEASE( pVertexBuffer );
	SAFE_RELEASE( pVertexLayout );
	SAFE_RELEASE( pTextureRV );
}
template < class T_VERTEX  >
void Billboard<T_VERTEX >
::Render( ID3D11Device* pd3dDevice, ID3DX11EffectShaderResourceVariable* pDiffuseVariable )
{
	// Set diffuse texture 
	pDiffuseVariable->SetResource( pTextureRV );

	// Set Vertex Buffers & layout
	pd3dDevice->IASetInputLayout( pVertexLayout );

	unsigned int offset = 0,  stride = sizeof( SimpleVertex );
	pd3dDevice->IASetVertexBuffers( 0, 1, &pVertexBuffer, &stride, &offset );
	pd3dDevice->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	// Render Billboard
	D3D11_TECHNIQUE_DESC techDesc;
	pRenderTechnique->GetDesc( &techDesc );

	for( unsigned int p = 0; p < techDesc.Passes; ++p )
	{
		pRenderTechnique->GetPassByIndex( p )->Apply( 0 );
		pd3dDevice->Draw( vertexCnt, 0 );
	}

}

