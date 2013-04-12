#include "stdafx.h"
#include "SSAORenderer.h"
#include "GameApp/GameApp.h"




SSAORenderer::SSAORenderer()
{
	g_pDepthNormalBuffer = 0;
	g_pDepthNormalBufferRTV = 0;
	g_pDepthNormalBufferSRV = 0;

	g_pOcclusionBuffer = 0;
	g_pOcclusionBufferUAV = 0;
	g_pOcclusionBufferSRV = 0;

	
	g_pInputLayout = 0;

	g_pDepthVS = 0;
	g_pDepthSkinVS = 0;
	g_pDepthPS = 0;

	g_pAmbientOcclusionCS = 0;
	g_pHorizontalBilateralCS = 0;
	g_pVerticalBilateralCS = 0;

	g_pFinalVS = 0;
	g_pFinalPS = 0;

	g_pConstantBuffer = 0;
	g_pSkinConstantBuffer = 0;
}

bool SSAORenderer::Init( ID3D11Device* dev, ID3D11DeviceContext* context )
{
	Device = dev;
	Context = context;
	
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
		
	

	static const int MATRIX_PALETTE_SIZE_DEFAULT = 164;
	cbuffer.ByteWidth = MATRIX_PALETTE_SIZE_DEFAULT*sizeof( mat4 );
	cbuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbuffer.MiscFlags = 0;
	cbuffer.StructureByteStride = 0;
	cbuffer.Usage = D3D11_USAGE_DYNAMIC;
	cbuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	GetApp()->GetDevice()->CreateBuffer( &cbuffer, NULL, &g_pSkinConstantBuffer );
	
	return true;
}

void SSAORenderer::SetSkinDepthShader( noMat4* world, noMat4* boneMat, int numBones )
{

	{	
		ID3D11Resource* pRes = g_pConstantBuffer;
		D3D11_MAPPED_SUBRESOURCE Data;

		struct transforms
		{
			noMat4 wvp;
			noMat4 wv;
		};

		transforms xforms;

		noMat4 view(GetCamera()->getViewMatrix());
		noMat4 proj(GetCamera()->getProjectionMatrix());
		noMat4 vp =  *world * view * proj;

		view.TransposeSelf();
		vp.TransposeSelf();
		xforms.wv = *world * view;//Transpose(view);
		xforms.wvp = vp;//Transpose(vp);

		HRESULT hr = GetApp()->GetContext()->Map( pRes, 0, D3D11_MAP_WRITE_DISCARD, 0, &Data );
		memcpy( Data.pData, &xforms, sizeof(noMat4) * 2);

		GetApp()->GetContext()->Unmap( pRes, 0 );
	}

	{
		ID3D11Resource* pRes = g_pSkinConstantBuffer;
		D3D11_MAPPED_SUBRESOURCE Data;
		
		HRESULT hr = GetApp()->GetContext()->Map( pRes, 0, D3D11_MAP_WRITE_DISCARD, 0, &Data );
		memcpy( Data.pData, boneMat, sizeof(noMat4) * numBones);
		GetApp()->GetContext()->Unmap( pRes, 0 );
	}
	

	GetApp()->GetContext()->VSSetShader( g_pDepthSkinVS, 0, 0 );
	GetApp()->GetContext()->PSSetShader( g_pDepthPS, 0, 0 );
	GetApp()->GetContext()->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
	GetApp()->GetContext()->VSSetConstantBuffers( 1, 1, &g_pSkinConstantBuffer );
}



void SSAORenderer::SetDepthShader( noMat4* world )
{

	ID3D11Resource* pRes = g_pConstantBuffer;
	D3D11_MAPPED_SUBRESOURCE Data;

	struct transforms
	{
		noMat4 wvp;
		noMat4 wv;
	};

	transforms xforms;

	noMat4 view(GetCamera()->getViewMatrix());
	noMat4 proj(GetCamera()->getProjectionMatrix());
	noMat4 vp =  *world * view * proj;

	view.TransposeSelf();
	vp.TransposeSelf();
	xforms.wv = *world * view;//Transpose(view);
	xforms.wvp = vp;//Transpose(vp);

	HRESULT hr = GetApp()->GetContext()->Map( pRes, 0, D3D11_MAP_WRITE_DISCARD, 0, &Data );
	memcpy( Data.pData, &xforms, sizeof(Matrix) * 2);

	GetApp()->GetContext()->Unmap( pRes, 0 );
	
	GetApp()->GetContext()->VSSetShader( g_pDepthVS, 0, 0 );
	GetApp()->GetContext()->PSSetShader( g_pDepthPS, 0, 0 );
	GetApp()->GetContext()->VSSetConstantBuffers( 0, 1, &g_pConstantBuffer );
}

HRESULT SSAORenderer::LoadShaders()
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

	hr = D3DX11CompileFromFile( "Data/Shaders/SSAO/DepthVS.hlsl", 0, 0, "VS_SKIN_MAIN", "vs_4_0", 0, 0, 0, &pCompiledShader, &pErrorMessages, &hr );
	hr = GetApp()->GetDevice()->CreateVertexShader( pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), 0, &g_pDepthSkinVS );
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
#if 0
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
		{ "TEXCOORDS", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);
	GetApp()->GetDevice()->CreateInputLayout( layout, numElements, pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), &g_pInputLayout );
	SAFE_RELEASE( pCompiledShader );
#endif

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

	D3D_SHADER_MACRO defines[] = 
	{
		"gridsize_x", "40",
		"gridsize_y", "30",
		"totalsize_x", "1280",
		"totalsize_y", "960",
		"xres", "640",
		"yres", "480",		
		NULL, NULL
	};
	char cXResDefinition[32];
	char cYResDefinition[32];
	sprintf_s( cXResDefinition, "%d", GetApp()->getWidth() );
	sprintf_s( cYResDefinition, "%d", GetApp()->getHeight() );
	defines[4].Definition = cXResDefinition;
	defines[5].Definition = cYResDefinition;

	//#define USE_16x16
#ifdef USE_16x16
	hr = D3DX11CompileFromFile( L"Data/Shaders/SSAO/AmbientOcclusionTraditionalCS_randomized_viewspace_16.hlsl", 0, 0, "CSMAIN", "cs_5_0", 0, 0, 0, &pCompiledShader, &pErrorMessages, &hr );
#else
	hr = D3DX11CompileFromFile( "Data/Shaders/SSAO/AmbientOcclusionTraditionalCS_randomized_viewspace_32.hlsl", defines, 0, "CSMAIN", "cs_5_0", 0, 0, 0, &pCompiledShader, &pErrorMessages, &hr );
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
	hr = D3DX11CompileFromFile( "Data/Shaders/SSAO/SeparableBilateralCS.hlsl", defines, 0, "CS_Horizontal", "cs_5_0", 0, 0, 0, &pCompiledShader, &pErrorMessages, &hr );
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
	hr = D3DX11CompileFromFile( "Data/Shaders/SSAO/SeparableBilateralCS.hlsl", defines, 0, "CS_Vertical", "cs_5_0", 0, 0, 0, &pCompiledShader, &pErrorMessages, &hr );
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

void SSAORenderer::Reset()
{
	// ÇØÁ¦.
	Release();
	Init(Device, Context);
}

void SSAORenderer::Release()
{

	SAFE_RELEASE( g_pDepthNormalBuffer );
	SAFE_RELEASE( g_pDepthNormalBufferRTV );
	SAFE_RELEASE( g_pDepthNormalBufferSRV );

	SAFE_RELEASE( g_pOcclusionBuffer );
	SAFE_RELEASE( g_pOcclusionBufferUAV );
	SAFE_RELEASE( g_pOcclusionBufferSRV );
		

	SAFE_RELEASE( g_pInputLayout );

	SAFE_RELEASE( g_pDepthVS );
	SAFE_RELEASE( g_pDepthSkinVS );
	SAFE_RELEASE( g_pDepthPS );

	SAFE_RELEASE( g_pAmbientOcclusionCS );
	SAFE_RELEASE( g_pHorizontalBilateralCS );
	SAFE_RELEASE( g_pVerticalBilateralCS );

	SAFE_RELEASE( g_pFinalVS );
	SAFE_RELEASE( g_pFinalPS );

	SAFE_RELEASE( g_pConstantBuffer );
	SAFE_RELEASE( g_pSkinConstantBuffer );
}

void SSAORenderer::BeginDepthRender()
{
	// Back Buffer
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
	
}

void SSAORenderer::RenderCS()
{
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
	GetApp()->GetContext()->Dispatch( 40, 30, 1 );
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

}


void SSAORenderer::EndDepthRender()
{
	// Clear the render targets from the Output Merger so that it can be bound to the CS next.
	ID3D11RenderTargetView* pNullRT = { 0 };
	ID3D11DepthStencilView* pNullDSV = 0;
	GetApp()->GetContext()->OMSetRenderTargets( 1, &pNullRT, pNullDSV );
}
