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
#include <d3d11.h>
#include <D3DX11Effect.h>
#include <string.h>

template <class T_VERTEX, class T_INSTANCE >
class InstancedBillboard
{

	int									vertexCnt;
	T_VERTEX*							pVertices;
	ID3D11Buffer*                       pVertexBuffer;

	D3D11_INPUT_ELEMENT_DESC*			pLayout;
	int									numElements;
	ID3D11InputLayout*                  pVertexLayout;

	TCHAR								pTextureName[MAX_PATH];
	ID3D11ShaderResourceView*           pTextureRV;
	ID3DX11EffectTechnique*              pRenderTechnique;

	int									instanceCnt;
	int									maxInstanceCnt;
	T_INSTANCE*							pInstanceData;
	ID3D11Buffer*						pInstanceBuffer;

	HRESULT			LoadInstData ( ID3D11Device* pd3dDevice, ID3D11Buffer** ppInstanceData, 
								   void* pInData, int dataSize );

public:

	virtual void	Init		 ( TCHAR* texName, T_INSTANCE* pInInstanceData, int cnt, T_VERTEX* pInVertices, int numVerts, 
								   D3D11_INPUT_ELEMENT_DESC* layout, int numLayoutElements, 
								   WORD* indices = 0, int numIndices = 0 );

	virtual void	Destroy		 ( ){ };

	virtual HRESULT CreateDevice ( ID3D11Device* pd3dDevice,  ID3D11DeviceContext* pd3dContext, ID3DX11EffectTechnique* pTechique );

	virtual void	DestroyDevice( );
//	virtual void    PreRender	 ( double fTime, float fElapsedTime ){};

	virtual void Render ( ID3D11DeviceContext* pd3dContext, ID3DX11EffectShaderResourceVariable* pDiffuseVariable );
	
	virtual void SetInstData ( ID3D11DeviceContext* pd3dContext, T_INSTANCE* pInData, int numInstances );

};

template <class T_VERTEX, class T_INSTANCE>
void InstancedBillboard<T_VERTEX, T_INSTANCE>::SetInstData( ID3D11DeviceContext* pd3dContext, T_INSTANCE* pInData, int numInstances )
{
	// ASSERT( numInstances > maxInstanceCnt );
	instanceCnt		= numInstances;
	D3D11_MAPPED_SUBRESOURCE mapped;	
	pd3dContext->Map( pInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapped);
	memcpy( mapped.pData, (void*)pInData, sizeof(T_INSTANCE)*instanceCnt );
	pd3dContext->Unmap(pInstanceBuffer, 0);
}


template <class T_VERTEX, class T_INSTANCE>
void InstancedBillboard<T_VERTEX, T_INSTANCE>::Init( TCHAR* texName, T_INSTANCE* pInInstanceData, int cnt , T_VERTEX* pInVertices, int numVerts, 
													D3D11_INPUT_ELEMENT_DESC* layout, int numLayoutElements, 
													WORD* indices /*= 0*/, int numIndices /*= 0 */ )
{
	_tcscpy(pTextureName, texName);

	pVertices		 =  pInVertices;
	vertexCnt		 =  numVerts;
	pLayout			 =  layout;
	numElements		 =  numLayoutElements;
	pInstanceData	 =  pInInstanceData;
	instanceCnt		 =  cnt;
	maxInstanceCnt	 =  cnt;
}

template <class T_VERTEX, class T_INSTANCE>
HRESULT InstancedBillboard<T_VERTEX, T_INSTANCE>::CreateDevice( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext,
	ID3DX11EffectTechnique* pTechique)
{
	HRESULT hr = S_OK;
	pRenderTechnique = pTechique;
	// Create the input layout
	D3DX11_PASS_DESC PassDesc;
	pRenderTechnique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	hr = pd3dDevice->CreateInputLayout( pLayout, numElements, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &pVertexLayout );

	// Initialize Vertex Buffers
	D3D11_BUFFER_DESC		bd;
	D3D11_SUBRESOURCE_DATA	InitData;

	bd.Usage			= D3D11_USAGE_DEFAULT;
	bd.ByteWidth		= sizeof( T_VERTEX ) * vertexCnt;
	bd.BindFlags		= D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags	= 0;
	bd.MiscFlags		= 0;
	InitData.pSysMem	= pVertices;

	DXCall( pd3dDevice->CreateBuffer( &bd, &InitData, &pVertexBuffer ));

	// Create Instance Buffer
	// We're creating this buffer as dynamic because in a game, the instance data could be dynamic... aka
	// we could have moving trees.
	D3D11_BUFFER_DESC bufferDesc =
	{
		sizeof( T_INSTANCE )*instanceCnt,
		D3D11_USAGE_DYNAMIC,
		D3D11_BIND_VERTEX_BUFFER,
		D3D11_CPU_ACCESS_WRITE,
		0
	};

	DXCall( pd3dDevice->CreateBuffer( &bufferDesc, NULL, &pInstanceBuffer ) );


	// Load the Texture
	DXCall( D3DX11CreateShaderResourceViewFromFile( pd3dDevice, pTextureName, NULL, NULL, &pTextureRV, NULL ));

	//Initialize instance buffer with data passed in
	SetInstData( pd3dContext, pInstanceData, instanceCnt ); 

	return hr;

}

template <class T_VERTEX, class T_INSTANCE>
void InstancedBillboard<T_VERTEX, T_INSTANCE>::DestroyDevice()
{
	SAFE_RELEASE( pVertexBuffer );
	SAFE_RELEASE( pVertexLayout );
	SAFE_RELEASE( pTextureRV );
	SAFE_RELEASE( pInstanceBuffer );
}

template <class T_VERTEX, class T_INSTANCE>
void InstancedBillboard<T_VERTEX, T_INSTANCE>::Render (ID3D11DeviceContext* pd3dContext,  ID3DX11EffectShaderResourceVariable* pDiffuseVariable )
{
	// Set the input layout
	pd3dContext->IASetInputLayout( pVertexLayout );

	ID3D11Buffer* pVB[2];
	UINT strides[2];
	UINT offsets[2] = {0,0};
	pVB[0] = pVertexBuffer;
	pVB[1] = pInstanceBuffer;
	strides[0] = sizeof ( T_VERTEX   );
	strides[1] = sizeof ( T_INSTANCE );
	pd3dContext->IASetVertexBuffers( 0,       //first input slot for binding
									2,       //number of buffers in the array
									pVB,     //array of three vertex buffers
									strides, //array of stride values, one for each buffer
									offsets	 //array of offset values, one for each buffer
									);

	// Set primitive topology
	pd3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	pDiffuseVariable->SetResource( pTextureRV );

	// Render Instanced Billboards
	D3DX11_TECHNIQUE_DESC techDesc;
	pRenderTechnique->GetDesc( &techDesc );

	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		pRenderTechnique->GetPassByIndex( p )->Apply( 0, pd3dContext );
		pd3dContext->DrawInstanced( vertexCnt,    	// number of vertices per instance
					  instanceCnt,	 // number of instances
					  0,		 // Index of the first vertex
					  0		 // Index of the first instance
					  );
	}
}

template <class T_VERTEX, class T_INSTANCE>
HRESULT InstancedBillboard<T_VERTEX, T_INSTANCE>::LoadInstData( ID3D11Device* pd3dDevice, ID3D11Buffer** ppInstanceData, void* pInData, int dataSize )
{


	HRESULT hr = S_OK;

	// Create a resource with the input matrices
	// We're creating this buffer as dynamic because in a game, the instance data could be dynamic
	D3D11_BUFFER_DESC bufferDesc =
	{
		dataSize,
		D3D11_USAGE_DYNAMIC,
		D3D11_BIND_VERTEX_BUFFER,
		D3D11_CPU_ACCESS_WRITE,
		0
	};

	DXCall( pd3dDevice->CreateBuffer( &bufferDesc, NULL, ppInstanceData ) );

	void* pData = NULL;
	( *ppInstanceData )->Map( D3D11_MAP_WRITE_DISCARD, NULL, ( void** )&pData );

	memcpy( pData, pInData, dataSize );

	( *ppInstanceData )->Unmap();
	return hr;
}

