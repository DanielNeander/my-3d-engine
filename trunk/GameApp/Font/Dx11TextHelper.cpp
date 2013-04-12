#include "stdafx.h"
#include "Dx11TextHelper.h"
#include <GameApp/Util/EffectUtil.h>
#include "GameApp/GameApp.h"

CHAR g_strUIEffectFile[] = \
	"Texture2D g_Texture;"\
	""\
	"SamplerState Sampler"\
	"{"\
	"    Filter = MIN_MAG_MIP_LINEAR;"\
	"    AddressU = Wrap;"\
	"    AddressV = Wrap;"\
	"};"\
	""\
	"BlendState UIBlend"\
	"{"\
	"    AlphaToCoverageEnable = FALSE;"\
	"    BlendEnable[0] = TRUE;"\
	"    SrcBlend = SRC_ALPHA;"\
	"    DestBlend = INV_SRC_ALPHA;"\
	"    BlendOp = ADD;"\
	"    SrcBlendAlpha = ONE;"\
	"    DestBlendAlpha = ZERO;"\
	"    BlendOpAlpha = ADD;"\
	"    RenderTargetWriteMask[0] = 0x0F;"\
	"};"\
	""\
	"BlendState NoBlending"\
	"{"\
	"    BlendEnable[0] = FALSE;"\
	"    RenderTargetWriteMask[0] = 0x0F;"\
	"};"\
	""\
	"DepthStencilState DisableDepth"\
	"{"\
	"    DepthEnable = false;"\
	"};"\
	"DepthStencilState EnableDepth"\
	"{"\
	"    DepthEnable = true;"\
	"};"\
	"struct VS_OUTPUT"\
	"{"\
	"    float4 Pos : POSITION;"\
	"    float4 Dif : COLOR;"\
	"    float2 Tex : TEXCOORD;"\
	"};"\
	""\
	"VS_OUTPUT VS( float3 vPos : POSITION,"\
	"              float4 Dif : COLOR,"\
	"              float2 vTexCoord0 : TEXCOORD )"\
	"{"\
	"    VS_OUTPUT Output;"\
	""\
	"    Output.Pos = float4( vPos, 1.0f );"\
	"    Output.Dif = Dif;"\
	"    Output.Tex = vTexCoord0;"\
	""\
	"    return Output;"\
	"}"\
	""\
	"float4 PS( VS_OUTPUT In ) : SV_Target"\
	"{"\
	"    return g_Texture.Sample( Sampler, In.Tex ) * In.Dif;"\
	"}"\
	""\
	"float4 PSUntex( VS_OUTPUT In ) : SV_Target"\
	"{"\
	"    return In.Dif;"\
	"}"\
	""\
	"technique10 RenderUI"\
	"{"\
	"    pass P0"\
	"    {"\
	"        SetVertexShader( CompileShader( vs_4_0, VS() ) );"\
	"        SetGeometryShader( NULL );"\
	"        SetPixelShader( CompileShader( ps_4_0, PS() ) );"\
	"        SetDepthStencilState( DisableDepth, 0 );"\
	"        SetBlendState( UIBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );"\
	"    }"\
	"}"\
	"technique10 RenderUIUntex"\
	"{"\
	"    pass P0"\
	"    {"\
	"        SetVertexShader( CompileShader( vs_4_0, VS() ) );"\
	"        SetGeometryShader( NULL );"\
	"        SetPixelShader( CompileShader( ps_4_0, PSUntex() ) );"\
	"        SetDepthStencilState( DisableDepth, 0 );"\
	"        SetBlendState( UIBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );"\
	"    }"\
	"}"\
	"technique10 RestoreState"\
	"{"\
	"    pass P0"\
	"    {"\
	"        SetDepthStencilState( EnableDepth, 0 );"\
	"        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );"\
	"    }"\
	"}";
const UINT              g_uUIEffectFileSize = sizeof( g_strUIEffectFile );

ID3D11Buffer* g_pFontBuffer11 = NULL;
UINT g_FontBufferBytes11 = 0;

CGrowableArray<DXUTSpriteVertex> g_FontVertices;
ID3D11ShaderResourceView* g_pFont11 = NULL;
ID3D11InputLayout* g_pInputLayout11 = NULL;
HRESULT InitFont11( ID3D11Device* pd3d11Device, ID3D11InputLayout* pInputLayout )
{
	HRESULT hr = S_OK;
	CHAR str[MAX_PATH] = "UI\\Font.dds";
	//V_RETURN( DXUTFindDXSDKMediaFileCch( str, MAX_PATH, L"UI\\Font.dds" ) );

	if (pd3d11Device->GetFeatureLevel() < D3D_FEATURE_LEVEL_10_0 ) {

		D3DX11_IMAGE_INFO dii;
		D3DX11GetImageInfoFromFile( str, NULL, &dii, NULL );

		D3DX11_IMAGE_LOAD_INFO dili;
		dili.BindFlags = D3DX11_DEFAULT;
		dili.CpuAccessFlags = D3DX11_DEFAULT;
		dili.Depth = D3DX11_DEFAULT;
		dili.Filter = D3DX11_DEFAULT;
		dili.FirstMipLevel = 0;
		dili.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dili.Height = D3DX11_DEFAULT;
		dili.MipFilter = D3DX11_DEFAULT;
		dili.MipLevels = 1;
		dili.MiscFlags = D3DX11_DEFAULT;
		dili.pSrcInfo = &dii;
		dili.Usage = D3D11_USAGE_DEFAULT ;
		dili.Width = D3DX11_DEFAULT;

		 D3DX11CreateShaderResourceViewFromFile( pd3d11Device, str, &dili, NULL, &g_pFont11, &hr) ;
	}
	else
	{
		 D3DX11CreateShaderResourceViewFromFile( pd3d11Device, str, NULL, NULL, &g_pFont11, &hr);
	}

#if defined(PROFILE) || defined(DEBUG)
	if (g_pFont11)
	{
		ID3D11Resource *pRes = NULL;
		g_pFont11->GetResource( &pRes );		
		SAFE_RELEASE( pRes );
	}    	
#endif

	g_pInputLayout11 = pInputLayout;
	return hr;
}

void EndFont11()
{
	SAFE_RELEASE( g_pFontBuffer11 );
	g_FontBufferBytes11 = 0;
	SAFE_RELEASE( g_pFont11 );
}

Dx11TextHelper::Dx11TextHelper( ID3D11Device* pd3d11Device, ID3D11DeviceContext* pd3d11DeviceContext, int nLineHeight )
{
	Init( nLineHeight );
	m_pd3d11Device = pd3d11Device;
	m_pd3d11DeviceContext = pd3d11DeviceContext;
}

Dx11TextHelper::~Dx11TextHelper()
{

}

void Dx11TextHelper::Init( int nLineHeight /*= 15 */ )
{
	m_clr = D3DXCOLOR( 1, 1, 1, 1 );
	m_pt.x = 0;
	m_pt.y = 0;
	m_nLineHeight = nLineHeight;
	m_pd3d11Device = NULL;
	m_pd3d11DeviceContext = NULL;
	
}

void Dx11TextHelper::Begin()
{
	//if( m_pSprite9 )
	//	m_pSprite9->Begin( D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE );

	if( m_pd3d11DeviceContext )
	{
		StoreD3D11State( m_pd3d11DeviceContext );
		//m_pManager->ApplyRenderUI11( m_pd3d11DeviceContext );
		m_pd3d11DeviceContext->OMSetDepthStencilState( m_pDepthStencilStateUI11, 0 );
		m_pd3d11DeviceContext->RSSetState( m_pRasterizerStateUI11 );
		float BlendFactor[4] = { 0, 0, 0, 0 };
		m_pd3d11DeviceContext->OMSetBlendState( m_pBlendStateUI11, BlendFactor, 0xFFFFFFFF );
		m_pd3d11DeviceContext->PSSetSamplers( 0, 1, &m_pSamplerStateUI11 );
	}

}

HRESULT Dx11TextHelper::DrawFormattedTextLine( const CHAR* strMsg, ... )
{
	CHAR strBuffer[512];

	va_list args;
	va_start( args, strMsg );
	vsprintf_s( strBuffer, 512, strMsg, args );
	strBuffer[511] = '\0';
	va_end( args );

	return DrawTextLine( strBuffer );
}

HRESULT Dx11TextHelper::DrawFormattedTextLine( RECT& rc, DWORD dwFlags, const CHAR* strMsg, ... )
{
	CHAR strBuffer[512];

	va_list args;
	va_start( args, strMsg );
	vsprintf_s( strBuffer, 512, strMsg, args );
	strBuffer[511] = L'\0';
	va_end( args );

	return DrawTextLine( rc, dwFlags, strBuffer );
}

HRESULT Dx11TextHelper::DrawTextLine( const CHAR* strMsg )
{
		HRESULT hr = S_OK;
	RECT rc;
	SetRect( &rc, m_pt.x, m_pt.y, 0, 0 );
	/*if( m_pFont9 )
		hr = m_pFont9->DrawText( m_pSprite9, strMsg, -1, &rc, DT_NOCLIP, m_clr );
	else */if( m_pd3d11DeviceContext )
		DrawText11DXUT( m_pd3d11Device, m_pd3d11DeviceContext, strMsg, rc, m_clr,
		(float)GetApp()->getWidth(), (float)GetApp()->getHeight(), false );

	if( FAILED( hr ) )
		return DXTRACE_ERR_MSGBOX( "DrawText", hr );

	m_pt.y += m_nLineHeight;

	return S_OK;
}

HRESULT Dx11TextHelper::DrawTextLine( RECT& rc, DWORD dwFlags, const CHAR* strMsg )
{
	HRESULT hr = S_OK;
	/*if( m_pFont9 )
		hr = m_pFont9->DrawText( m_pSprite9, strMsg, -1, &rc, dwFlags, m_clr );
	else*/ if( m_pd3d11DeviceContext )
		DrawText11DXUT( m_pd3d11Device, m_pd3d11DeviceContext, strMsg, rc, m_clr,
		(float)GetApp()->getWidth(), (float)GetApp()->getHeight(), false );

	if( FAILED( hr ) )
		return DXTRACE_ERR_MSGBOX( "DrawText", hr );

	m_pt.y += m_nLineHeight;

	return S_OK;
}

void Dx11TextHelper::End()
{
	//if( m_pSprite9 )
	//	m_pSprite9->End();

	if( m_pd3d11DeviceContext )
	{
		RestoreD3D11State( m_pd3d11DeviceContext );
	}
}

void Dx11TextHelper::StoreD3D11State( ID3D11DeviceContext* pd3dImmediateContext )
{
	pd3dImmediateContext->OMGetDepthStencilState( &m_pDepthStencilStateStored11, &m_StencilRefStored11 );
	pd3dImmediateContext->RSGetState( &m_pRasterizerStateStored11 );
	pd3dImmediateContext->OMGetBlendState( &m_pBlendStateStored11, m_BlendFactorStored11, &m_SampleMaskStored11 );
	pd3dImmediateContext->PSGetSamplers( 0, 1, &m_pSamplerStateStored11 );
}

void Dx11TextHelper::RestoreD3D11State( ID3D11DeviceContext* pd3dImmediateContext )
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


void Dx11TextHelper::DrawText11DXUT( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3d11DeviceContext,
	LPCSTR strText, RECT rcScreen, D3DXCOLOR vFontColor,
	float fBBWidth, float fBBHeight, bool bCenter )
{
	float fCharTexSizeX = 0.010526315f;
	//float fGlyphSizeX = 14.0f / fBBWidth;
	//float fGlyphSizeY = 32.0f / fBBHeight;
	float fGlyphSizeX = 15.0f / fBBWidth;
	float fGlyphSizeY = 42.0f / fBBHeight;


	float fRectLeft = rcScreen.left / fBBWidth;
	float fRectTop = 1.0f - rcScreen.top / fBBHeight;

	fRectLeft = fRectLeft * 2.0f - 1.0f;
	fRectTop = fRectTop * 2.0f - 1.0f;

	int NumChars = (int)strlen( strText );
	if (bCenter) {
		float fRectRight = rcScreen.right / fBBWidth;
		fRectRight = fRectRight * 2.0f - 1.0f;
		float fRectBottom = 1.0f - rcScreen.bottom / fBBHeight;
		fRectBottom = fRectBottom * 2.0f - 1.0f;
		float fcenterx = ((fRectRight - fRectLeft) - (float)NumChars*fGlyphSizeX) *0.5f;
		float fcentery = ((fRectTop - fRectBottom) - (float)1*fGlyphSizeY) *0.5f;
		fRectLeft += fcenterx ;    
		fRectTop -= fcentery;
	}
	float fOriginalLeft = fRectLeft;
	float fTexTop = 0.0f;
	float fTexBottom = 1.0f;

	float fDepth = 0.5f;
	for( int i=0; i<NumChars; i++ )
	{
		if( strText[i] == '\n' )
		{
			fRectLeft = fOriginalLeft;
			fRectTop -= fGlyphSizeY;

			continue;
		}
		else if( strText[i] < 32 || strText[i] > 126 )
		{
			continue;
		}

		// Add 6 sprite vertices
		DXUTSpriteVertex SpriteVertex;
		float fRectRight = fRectLeft + fGlyphSizeX;
		float fRectBottom = fRectTop - fGlyphSizeY;
		float fTexLeft = ( strText[i] - 32 ) * fCharTexSizeX;
		float fTexRight = fTexLeft + fCharTexSizeX;

		// tri1
		SpriteVertex.vPos = D3DXVECTOR3( fRectLeft, fRectTop, fDepth );
		SpriteVertex.vTex = D3DXVECTOR2( fTexLeft, fTexTop );
		SpriteVertex.vColor = vFontColor;
		g_FontVertices.Add( SpriteVertex );

		SpriteVertex.vPos = D3DXVECTOR3( fRectRight, fRectTop, fDepth );
		SpriteVertex.vTex = D3DXVECTOR2( fTexRight, fTexTop );
		SpriteVertex.vColor = vFontColor;
		g_FontVertices.Add( SpriteVertex );

		SpriteVertex.vPos = D3DXVECTOR3( fRectLeft, fRectBottom, fDepth );
		SpriteVertex.vTex = D3DXVECTOR2( fTexLeft, fTexBottom );
		SpriteVertex.vColor = vFontColor;
		g_FontVertices.Add( SpriteVertex );

		// tri2
		SpriteVertex.vPos = D3DXVECTOR3( fRectRight, fRectTop, fDepth );
		SpriteVertex.vTex = D3DXVECTOR2( fTexRight, fTexTop );
		SpriteVertex.vColor = vFontColor;
		g_FontVertices.Add( SpriteVertex );

		SpriteVertex.vPos = D3DXVECTOR3( fRectRight, fRectBottom, fDepth );
		SpriteVertex.vTex = D3DXVECTOR2( fTexRight, fTexBottom );
		SpriteVertex.vColor = vFontColor;
		g_FontVertices.Add( SpriteVertex );

		SpriteVertex.vPos = D3DXVECTOR3( fRectLeft, fRectBottom, fDepth );
		SpriteVertex.vTex = D3DXVECTOR2( fTexLeft, fTexBottom );
		SpriteVertex.vColor = vFontColor;
		g_FontVertices.Add( SpriteVertex );

		fRectLeft += fGlyphSizeX;

	}

	// We have to end text after every line so that rendering order between sprites and fonts is preserved
	EndText11( pd3dDevice, pd3d11DeviceContext );
}

void Dx11TextHelper::EndText11( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3d11DeviceContext )
{

	// ensure our buffer size can hold our sprites
	UINT FontDataBytes = g_FontVertices.GetSize() * sizeof( DXUTSpriteVertex );
	if( g_FontBufferBytes11 < FontDataBytes )
	{
		SAFE_RELEASE( g_pFontBuffer11 );
		g_FontBufferBytes11 = FontDataBytes;

		D3D11_BUFFER_DESC BufferDesc;
		BufferDesc.ByteWidth = g_FontBufferBytes11;
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.MiscFlags = 0;

		pd3dDevice->CreateBuffer( &BufferDesc, NULL, &g_pFontBuffer11 );
		//DXUT_SetDebugName( g_pFontBuffer11, "DXUT Text11" );
	}

	// Copy the sprites over
	D3D11_BOX destRegion;
	destRegion.left = 0;
	destRegion.right = FontDataBytes;
	destRegion.top = 0;
	destRegion.bottom = 1;
	destRegion.front = 0;
	destRegion.back = 1;
	D3D11_MAPPED_SUBRESOURCE MappedResource;
	if ( S_OK == pd3d11DeviceContext->Map( g_pFontBuffer11, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) ) { 
		CopyMemory( MappedResource.pData, (void*)g_FontVertices.GetData(), FontDataBytes );
		pd3d11DeviceContext->Unmap(g_pFontBuffer11, 0);
	}

	

	ID3D11ShaderResourceView* pOldTexture = NULL;
	pd3d11DeviceContext->PSGetShaderResources( 0, 1, &pOldTexture );
	//pd3d11DeviceContext->PSSetShaderResources( 0, 1, &g_pFont11 );

	

	// Draw
	UINT Stride = sizeof( DXUTSpriteVertex );
	UINT Offset = 0;
	pd3d11DeviceContext->IASetVertexBuffers( 0, 1, &g_pFontBuffer11, &Stride, &Offset );
	pd3d11DeviceContext->IASetIndexBuffer( NULL, DXGI_FORMAT_R16_UINT, 0 );
	pd3d11DeviceContext->IASetInputLayout( g_pInputLayout11 );
	pd3d11DeviceContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	m_pFontTex->SetResource(g_pFont11);

	D3DX11_TECHNIQUE_DESC techDesc;
	g_pFontTec->GetDesc( &techDesc );

	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		g_pFontTec->GetPassByIndex( p )->Apply( 0, pd3d11DeviceContext );

		pd3d11DeviceContext->Draw( g_FontVertices.GetSize(), 0 );
	}

	pd3d11DeviceContext->PSSetShaderResources( 0, 1, &pOldTexture );
	SAFE_RELEASE( pOldTexture );

	g_FontVertices.Reset();
}

void Dx11TextHelper::InitFont()
{
	ID3DBlob* pVSBlob = NULL;
	ID3DBlob* pPSBlob = NULL;
	char filename[MAX_PATH] = "UI/Font.fx";
	LoadEffectFromFile( m_pd3d11Device, filename, &g_pFontEffect);	
	//ID3DBlob* pErrorBlob = NULL;
	//DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;

	
		
	//hr = D3DX11CreateEffectFromMemory(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), 0, m_pd3d11Device, &g_pFontEffect);


	g_pFontTec = g_pFontEffect->GetTechniqueByName( "RenderUI" );

	
	// Create input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",     0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3DX11_PASS_DESC PassDesc;
	g_pFontTec->GetPassByIndex( 0 )->GetDesc( &PassDesc ) ;
	m_pd3d11Device->CreateInputLayout( layout, ARRAYSIZE( layout ), PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &m_pInputLayout11 );
	

	// Release the blobs
	SAFE_RELEASE( pVSBlob );
	//SAFE_RELEASE( pPSBlob );
	//SAFE_RELEASE( pPSUntexBlob );

	D3D11_DEPTH_STENCIL_DESC DSDesc;
	ZeroMemory( &DSDesc, sizeof( D3D11_DEPTH_STENCIL_DESC ) );
	DSDesc.DepthEnable = FALSE;
	DSDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DSDesc.DepthFunc = D3D11_COMPARISON_LESS;
	DSDesc.StencilEnable = FALSE;
	m_pd3d11Device->CreateDepthStencilState( &DSDesc, &m_pDepthStencilStateUI11 );

	D3D11_RASTERIZER_DESC RSDesc;
	RSDesc.AntialiasedLineEnable = FALSE;
	RSDesc.CullMode = D3D11_CULL_BACK;
	RSDesc.DepthBias = 0;
	RSDesc.DepthBiasClamp = 0.0f;
	RSDesc.DepthClipEnable = TRUE;
	RSDesc.FillMode = D3D11_FILL_SOLID;
	RSDesc.FrontCounterClockwise = FALSE;
	RSDesc.MultisampleEnable = TRUE;
	RSDesc.ScissorEnable = FALSE;
	RSDesc.SlopeScaledDepthBias = 0.0f;
	m_pd3d11Device->CreateRasterizerState( &RSDesc, &m_pRasterizerStateUI11 );

	D3D11_BLEND_DESC BSDesc;
	ZeroMemory( &BSDesc, sizeof( D3D11_BLEND_DESC ) );

	BSDesc.RenderTarget[0].BlendEnable = TRUE;
	BSDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BSDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BSDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BSDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BSDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BSDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BSDesc.RenderTarget[0].RenderTargetWriteMask = 0x0F;

	m_pd3d11Device->CreateBlendState( &BSDesc, &m_pBlendStateUI11 );
	

	D3D11_SAMPLER_DESC SSDesc;
	ZeroMemory( &SSDesc, sizeof( D3D11_SAMPLER_DESC ) );
	SSDesc.Filter = D3D11_FILTER_ANISOTROPIC   ;
	SSDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	SSDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	SSDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	SSDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SSDesc.MaxAnisotropy = 16;
	SSDesc.MinLOD = 0;
	SSDesc.MaxLOD = D3D11_FLOAT32_MAX;
	if ( m_pd3d11Device->GetFeatureLevel() < D3D_FEATURE_LEVEL_9_3 ) {
		SSDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		SSDesc.MaxAnisotropy = 0;
	}
	m_pd3d11Device->CreateSamplerState( &SSDesc, &m_pSamplerStateUI11 );

	 m_pFontTex = g_pFontEffect->GetVariableByName( "g_Texture" )->AsShaderResource();


	InitFont11( m_pd3d11Device, m_pInputLayout11 );

}

