#include "stdafx.h"
#include "GeometryLoaderDX11.h"
#include "Camera.h"
#include "GameApp.h"



ID3D11Texture2D*			g_pDepthNormalBuffer = 0;
ID3D11RenderTargetView*		g_pDepthNormalBufferRTV = 0;
ID3D11ShaderResourceView *	g_pDepthNormalBufferSRV = 0;

ID3D11Texture2D*			g_pOcclusionBuffer = 0;
ID3D11UnorderedAccessView*	g_pOcclusionBufferUAV = 0;
ID3D11ShaderResourceView *	g_pOcclusionBufferSRV = 0;

GeometryDX11*				g_pModel = 0;
ID3D11InputLayout*			g_pInputLayout = 0;

ID3D11VertexShader*			g_pDepthVS = 0;
ID3D11PixelShader*			g_pDepthPS = 0;

ID3D11ComputeShader*		g_pAmbientOcclusionCS = 0;
ID3D11ComputeShader*		g_pHorizontalBilateralCS = 0;
ID3D11ComputeShader*		g_pVerticalBilateralCS = 0;

ID3D11VertexShader*			g_pFinalVS = 0;
ID3D11PixelShader*			g_pFinalPS = 0;

ID3D11Buffer*				g_pConstantBuffer = 0;

extern ShadowCamera				g_ShadowCamera;

Matrix				g_mWorld;
Matrix				g_mView;
Matrix				g_mProj;

static HRESULT CreateShaders()
{
	HRESULT hr = S_OK;

	ID3DBlob* pCompiledShader = NULL;
	ID3DBlob* pErrorMessages = NULL;

	// Compile the depth pass vertex shader
	hr = D3DX11CompileFromFile( "Data/Shaders/SSAO/DepthVS.hlsl", 0, 0, "VSMAIN", "vs_4_0", 0, 0, 0, &pCompiledShader, &pErrorMessages, &hr );
	hr = GetApp()->GetDevice()->CreateVertexShader( pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), 0, &g_pDepthVS );
	if ( FAILED( hr ) )
	{
		if ( pErrorMessages != 0 )
		{
			LPVOID pCompileErrors = pErrorMessages->GetBufferPointer();
			const char* pMessage = (const char*)pCompileErrors;
			//CLog::Get().Write( (const char*)pCompileErrors );
		}
		return( E_FAIL );
	}

	// Get the input layout with the compiled vertex shader
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
		{ "TEXCOORDS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);
	GetApp()->GetDevice()->CreateInputLayout( layout, numElements, pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), &g_pInputLayout );
	SAFE_RELEASE( pCompiledShader );

	// Compile the depth pass pixel shader
	hr = D3DX11CompileFromFile( "Data/Shaders/SSAO/DepthPS.hlsl", 0, 0, "PSMAIN", "ps_4_0", 0, 0, 0, &pCompiledShader, &pErrorMessages, &hr );
	if ( FAILED( hr ) )
	{
		if ( pErrorMessages != 0 )
		{
			LPVOID pCompileErrors = pErrorMessages->GetBufferPointer();
			const char* pMessage = (const char*)pCompileErrors;
			//CLog::Get().Write( (const char*)pCompileErrors );
		}
		return( E_FAIL );
	}
	hr = GetApp()->GetDevice()->CreatePixelShader( pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), 0, &g_pDepthPS );
	SAFE_RELEASE( pCompiledShader );

	// Compile the ambient occlusion compute shader.  Uncomment the define below to utilize 16x16 thread groups, or comment it
	// to utilize 32x32 thread groups.

	//#define USE_16x16
#ifdef USE_16x16
	hr = D3DX11CompileFromFile( L"Data/Shaders/SSAO/AmbientOcclusionTraditionalCS_randomized_viewspace_16.hlsl", 0, 0, "CSMAIN", "cs_5_0", 0, 0, 0, &pCompiledShader, &pErrorMessages, &hr );
#else
	hr = D3DX11CompileFromFile( "Data/Shaders/SSAO/AmbientOcclusionTraditionalCS_randomized_viewspace_32.hlsl", 0, 0, "CSMAIN", "cs_5_0", 0, 0, 0, &pCompiledShader, &pErrorMessages, &hr );
#endif
	if ( FAILED( hr ) )
	{
		if ( pErrorMessages != 0 )
		{
			LPVOID pCompileErrors = pErrorMessages->GetBufferPointer();
			const char* pMessage = (const char*)pCompileErrors;
			//CLog::Get().Write( (const char*)pCompileErrors );
		}
		return( E_FAIL );
	}
	hr = GetApp()->GetDevice()->CreateComputeShader( pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), 0, &g_pAmbientOcclusionCS );
	SAFE_RELEASE( pCompiledShader );

	// Compile the horizontal bilateral filter
	hr = D3DX11CompileFromFile( "Data/Shaders/SSAO/SeparableBilateralCS.hlsl", 0, 0, "CS_Horizontal", "cs_5_0", 0, 0, 0, &pCompiledShader, &pErrorMessages, &hr );
	if ( FAILED( hr ) )
	{
		if ( pErrorMessages != 0 )
		{
			LPVOID pCompileErrors = pErrorMessages->GetBufferPointer();
			const char* pMessage = (const char*)pCompileErrors;
			//CLog::Get().Write( (const char*)pCompileErrors );
		}
		return( E_FAIL );
	}
	hr = GetApp()->GetDevice()->CreateComputeShader( pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), 0, &g_pHorizontalBilateralCS );
	SAFE_RELEASE( pCompiledShader );

	// Compile the vertical bilateral filter
	hr = D3DX11CompileFromFile( "Data/Shaders/SSAO/SeparableBilateralCS.hlsl", 0, 0, "CS_Vertical", "cs_5_0", 0, 0, 0, &pCompiledShader, &pErrorMessages, &hr );
	if ( FAILED( hr ) )
	{
		if ( pErrorMessages != 0 )
		{
			LPVOID pCompileErrors = pErrorMessages->GetBufferPointer();
			const char* pMessage = (const char*)pCompileErrors;
			//CLog::Get().Write( (const char*)pCompileErrors );
		}
		return( E_FAIL );
	}
	hr = GetApp()->GetDevice()->CreateComputeShader( pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), 0, &g_pVerticalBilateralCS );
	SAFE_RELEASE( pCompiledShader );

	// Compile the final rendering vertex shader
	hr = D3DX11CompileFromFile( "Data/Shaders/SSAO/FinalVS.hlsl", 0, 0, "VSMAIN", "vs_4_0", 0, 0, 0, &pCompiledShader, &pErrorMessages, &hr );
	if ( FAILED( hr ) )
	{
		if ( pErrorMessages != 0 )
		{
			LPVOID pCompileErrors = pErrorMessages->GetBufferPointer();
			const char* pMessage = (const char*)pCompileErrors;
			//CLog::Get().Write( (const char*)pCompileErrors );
		}
		return( E_FAIL );
	}
	hr = GetApp()->GetDevice()->CreateVertexShader( pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), 0, &g_pFinalVS );
	SAFE_RELEASE( pCompiledShader );

	// Compile the final rendering pixel shader
	hr = D3DX11CompileFromFile( "Data/Shaders/SSAO/FinalPS.hlsl", 0, 0, "PSMAIN", "ps_4_0", 0, 0, 0, &pCompiledShader, &pErrorMessages, &hr );
	if ( FAILED( hr ) )
	{
		if ( pErrorMessages != 0 )
		{
			LPVOID pCompileErrors = pErrorMessages->GetBufferPointer();
			const char* pMessage = (const char*)pCompileErrors;
			//CLog::Get().Write( (const char*)pCompileErrors );
		}
		return( E_FAIL );
	}
	hr = GetApp()->GetDevice()->CreatePixelShader( pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), 0, &g_pFinalPS );
	SAFE_RELEASE( pCompiledShader );

	return hr;
}

static void CreateMeshes()
{
	noMat4 identity;
	identity.Identity();
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &identity;
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC cbuffer;
	cbuffer.ByteWidth = 2*sizeof( noMat4 );
	cbuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbuffer.MiscFlags = 0;
	cbuffer.StructureByteStride = 0;
	cbuffer.Usage = D3D11_USAGE_DYNAMIC;
	cbuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetApp()->GetDevice()->CreateBuffer( &cbuffer, &data, &g_pConstantBuffer );



	// Load the model for rendering
	g_pModel = GeometryLoaderDX11::loadMS3DFile2( std::string( "Models/SSAO_Demo_Scene_4.ms3d" ) );
	g_pModel->LoadToBuffers( GetApp()->GetDevice() );
}

void App::LoadSSAO()
{
	// Depth/Normal buffer creation

	D3D11_TEXTURE2D_DESC desc;
	desc.Width = GetApp()->getWidth();
	desc.Height = GetApp()->getHeight();
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	GetApp()->GetDevice()->CreateTexture2D( &desc, 0, &g_pDepthNormalBuffer );								// Create the texture
	GetApp()->GetDevice()->CreateRenderTargetView( g_pDepthNormalBuffer, 0, &g_pDepthNormalBufferRTV );	// Create the RTV
	GetApp()->GetDevice()->CreateShaderResourceView( g_pDepthNormalBuffer, 0, &g_pDepthNormalBufferSRV );	// Create the SRV

	// Create the occlusion buffer

	desc.Format = DXGI_FORMAT_R32_FLOAT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;

	GetApp()->GetDevice()->CreateTexture2D( &desc, 0, &g_pOcclusionBuffer );								// Create the texture
	GetApp()->GetDevice()->CreateUnorderedAccessView( g_pOcclusionBuffer, 0, &g_pOcclusionBufferUAV );		// Create the UAV
	GetApp()->GetDevice()->CreateShaderResourceView( g_pOcclusionBuffer, 0, &g_pOcclusionBufferSRV );		// Create the SRV

	CreateShaders();

	CreateMeshes();

	g_mWorld.SetIdentity();

	noVec3 vLookAt = noVec3( 0.0f, 0.75f, 0.0f );
	noVec3 vLookFrom = noVec3( 3.0f, 3.5f, -3.0f );
	noVec3 vLookUp = noVec3( 0.0f, 1.0f, 0.0f );
	g_mView = MatrixLookAtLH(vLookFrom, vLookAt, vLookUp);

	g_mProj = MatrixPerspectiveFovLH( noMath::PI / 4.0f, GetApp()->GetAspectRatio(), 1.0f, 25.0f);
}


void App::RenderSSAO(Renderer* pRenderer)
{
	ID3D11Resource* pRes = g_pConstantBuffer;
	D3D11_MAPPED_SUBRESOURCE Data;

	struct transforms
	{
		Matrix wvp;
		Matrix wv;
	};

	transforms xforms;

	Matrix view = g_mView;
	Matrix proj = g_mProj;
	Matrix vp =  view * proj;

	xforms.wv = Transpose(view);
	xforms.wvp = Transpose(vp);

	HRESULT hr = GetApp()->GetContext()->Map( pRes, 0, D3D11_MAP_WRITE_DISCARD, 0, &Data );
	memcpy( Data.pData, &xforms, sizeof(Matrix) * 2);

	GetApp()->GetContext()->Unmap( pRes, 0 );

	pRenderer->changeToMainFramebuffer();

	ID3D11RenderTargetView* pRTV;
	ID3D11DepthStencilView* pDSV;

	GetApp()->GetContext()->OMGetRenderTargets(1, &pRTV, &pDSV);
	
	ID3D11RenderTargetView* pRenderTarget = { g_pDepthNormalBufferRTV };
	ID3D11DepthStencilView* pDepthStencilView = pDSV;

	GetApp()->GetContext()->OMSetRenderTargets(1, &pRenderTarget, pDepthStencilView);
	// Clear the depth/normal buffer and depth stencil view
	float ClearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f }; // RGBA
	GetApp()->GetContext()->ClearRenderTargetView( g_pDepthNormalBufferRTV, ClearColor );
	GetApp()->GetContext()->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0f, 0 );

	GetApp()->GetContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	ID3D11Buffer* Buffers = { g_pModel->m_pVertexBuffer };
	UINT Strides = { g_pModel->GetVertexSize() };
	UINT Offsets = { 0 };

	// Configure the Input Assembler for rendering the model
	GetApp()->GetContext()->IASetVertexBuffers( 0, 1, &Buffers, &Strides, &Offsets );
	GetApp()->GetContext()->IASetIndexBuffer( g_pModel->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );
	GetApp()->GetContext()->IASetInputLayout( g_pInputLayout );

	// Set the vertex/pixel shaders and their resources for final rendering
	GetApp()->GetContext()->VSSetShader( g_pDepthVS, 0, 0 );
	GetApp()->GetContext()->PSSetShader( g_pDepthPS, 0, 0 );
	GetApp()->GetContext()->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );

	// Draw the model
	GetApp()->GetContext()->DrawIndexed( g_pModel->GetIndexCount(), 0, 0 );

	// Clear the render targets from the Output Merger so that it can be bound to the CS next.
	ID3D11RenderTargetView* pNullRT = { 0 };
	ID3D11DepthStencilView* pNullDSV = 0;
	GetApp()->GetContext()->OMSetRenderTargets( 1, &pNullRT, pNullDSV );

	GetApp()->GetContext()->CSSetShader( g_pAmbientOcclusionCS, 0, 0);
	GetApp()->GetContext()->CSSetShaderResources(0, 1, &g_pDepthNormalBufferSRV);
	GetApp()->GetContext()->CSSetUnorderedAccessViews( 0, 1, &g_pOcclusionBufferUAV, (UINT*)&g_pOcclusionBufferUAV);

	// Dispatch appropriate number of thread groups depending on the thread group size.
#ifdef USE_16x16
	GetApp()->GetContext()->Dispatch( 40, 30, 1 );
#else
	GetApp()->GetContext()->Dispatch( 20, 15, 1 );
#endif

	// Unbind the depth/normal buffer SRV and occlusion buffer UAV from the CS
	ID3D11ShaderResourceView* pNullSRV = 0;
	ID3D11UnorderedAccessView* pNullUAV = 0;
	GetApp()->GetContext()->CSSetShaderResources( 0, 1, &pNullSRV );
	GetApp()->GetContext()->CSSetUnorderedAccessViews( 0, 1, &pNullUAV, 0 );

	// This loop performs the separable bilateral filtering.  Increasing the number of
	// iterations increases the blurring effect while mostly preserving the edges of the
	// objects within the image.
	for ( int i = 0; i < 1; i++ )
	{
		GetApp()->GetContext()->CSSetShader( g_pHorizontalBilateralCS, 0, 0 );
		GetApp()->GetContext()->CSSetShaderResources( 0, 1, &g_pDepthNormalBufferSRV );									// register( t0 )
		GetApp()->GetContext()->CSSetUnorderedAccessViews( 0, 1, &g_pOcclusionBufferUAV, (UINT*)&g_pOcclusionBufferUAV );	// register( u0 )
		GetApp()->GetContext()->Dispatch( 1, GetApp()->getHeight(), 1 );

		GetApp()->GetContext()->CSSetShader( g_pVerticalBilateralCS, 0, 0 );
		GetApp()->GetContext()->CSSetShaderResources( 0, 1, &g_pDepthNormalBufferSRV );									// register( t0 )
		GetApp()->GetContext()->CSSetUnorderedAccessViews( 0, 1, &g_pOcclusionBufferUAV, (UINT*)&g_pOcclusionBufferUAV );	// register( u0 )
		GetApp()->GetContext()->Dispatch( GetApp()->getWidth(), 1, 1 );
	}

	// Unbind resources from the CS
	GetApp()->GetContext()->CSSetShaderResources( 0, 1, &pNullSRV );
	GetApp()->GetContext()->CSSetUnorderedAccessViews( 0, 1, &pNullUAV, 0 );

	// Clear the final render target
	GetApp()->GetContext()->ClearRenderTargetView( pRTV, ClearColor );
	GetApp()->GetContext()->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	// Set the render target and depth buffer
	GetApp()->GetContext()->OMSetRenderTargets( 1, &pRTV, pDSV );

	// Configure the Input Assembler for rendering the model
	GetApp()->GetContext()->IASetVertexBuffers( 0, 1, &Buffers, &Strides, &Offsets );
	GetApp()->GetContext()->IASetIndexBuffer( g_pModel->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0 );
	GetApp()->GetContext()->IASetInputLayout( g_pInputLayout );

	// Set the vertex/pixel shaders and their resources for final rendering
	GetApp()->GetContext()->VSSetShader( g_pFinalVS, 0, 0 );
	GetApp()->GetContext()->PSSetShader( g_pFinalPS, 0, 0 );
	GetApp()->GetContext()->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
	GetApp()->GetContext()->PSSetShaderResources( 0, 1, &g_pOcclusionBufferSRV );

	// Draw the model
	GetApp()->GetContext()->DrawIndexed( g_pModel->GetIndexCount(), 0, 0 );

	// Unbind the occlusion buffer from the PS
	GetApp()->GetContext()->PSSetShaderResources( 0, 1, &pNullSRV );
}

void App::DestorySSAO()
{
	SAFE_RELEASE( g_pDepthNormalBuffer );
	SAFE_RELEASE( g_pDepthNormalBufferRTV );
	SAFE_RELEASE( g_pDepthNormalBufferSRV );

	SAFE_RELEASE( g_pOcclusionBuffer );
	SAFE_RELEASE( g_pOcclusionBufferUAV );
	SAFE_RELEASE( g_pOcclusionBufferSRV );

	SAFE_DELETE( g_pModel );

	SAFE_RELEASE( g_pInputLayout );

	SAFE_RELEASE( g_pDepthVS );
	SAFE_RELEASE( g_pDepthPS );

	SAFE_RELEASE( g_pAmbientOcclusionCS );
	SAFE_RELEASE( g_pHorizontalBilateralCS );
	SAFE_RELEASE( g_pVerticalBilateralCS );

	SAFE_RELEASE( g_pFinalVS );
	SAFE_RELEASE( g_pFinalPS );

	SAFE_RELEASE( g_pConstantBuffer );
}

