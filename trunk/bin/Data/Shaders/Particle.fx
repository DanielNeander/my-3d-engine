#include "common.fx"

cbuffer cbPerFrame
{
	//float  g_fTime;   
	//float3 g_LightDir;
	float3 g_vEyePt;
	//float3 g_vRight;
	//float3 g_vUp;
	//float3 g_vForward;
	float4x4 g_mWorldViewProjection;   
	//float4x4 g_mInvViewProj;
	float4x4 g_mWorld;
};

Texture2D g_txMeshTexture;          // Color texture for mesh
//Texture2D g_txParticleColor;        // Particle color buffer
SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    //DepthWriteMask = ALL;
	DepthWriteMask = 0;
    DepthFunc = LESS_EQUAL;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = 0;
    //DepthFunc = LESS_EQUAL;
	DepthFunc = NEVER;
};

DepthStencilState DepthRead
{
    DepthEnable = TRUE;
    DepthWriteMask = 0;
    DepthFunc = LESS_EQUAL;
};

//--------------------------------------------------------------------------------------
// Blending States
//--------------------------------------------------------------------------------------
//BlendState NoBlending
//{
//    BlendEnable[0] = FALSE;
//};

BlendState SrcAlphaBlendingAdd
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = ONE;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

BlendState SrcAlphaBlendingSub
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = ONE;
    BlendOp = SUBTRACT;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

BlendState SrcColorBlendingAdd
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_COLOR;
    DestBlend = ONE;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

BlendState SrcColorBlendingSub
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_COLOR;
    DestBlend = ONE;
    BlendOp = SUBTRACT;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

BlendState SrcAlphaBlending
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};

//--------------------------------------------------------------------------------------
// Depth/Stencil States
//--------------------------------------------------------------------------------------
DepthStencilState RenderWithStencilState
{
    DepthEnable = false;
    DepthWriteMask = ZERO;
    DepthFunc = Less;
    
    // Setup stencil states
    StencilEnable = true;
    StencilReadMask = 0xFF;
    StencilWriteMask = 0x00;
    
    FrontFaceStencilFunc = Not_Equal;
    FrontFaceStencilPass = Keep;
    FrontFaceStencilFail = Zero;
    
    BackFaceStencilFunc = Not_Equal;
    BackFaceStencilPass = Keep;
    BackFaceStencilFail = Zero;
};

BlendState AdditiveBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = ONE;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
};



BlendState ForwardBlending
{
	AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    BlendEnable[1] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
    RenderTargetWriteMask[1] = 0x0F;
};

BlendState DisableBlending
{
	AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
    BlendEnable[1] = FALSE;
    RenderTargetWriteMask[0] = 0x0F;
    RenderTargetWriteMask[1] = 0x0F;
};

//RasterizerState RSWireframe
//{
//	FillMode = Wireframe;
//};

RasterizerState RSSolid
{
	FillMode = Solid;
	CullMode = None;
};

struct VS_PARTICLEINPUT
{
	float3 Position   : POSITION;		
	float2 TextureUV  : TEXCOORD0;
	//float  fLife      : LIFE; 
	//float  fRot	  : THETA;
	float4 Color	  : COLOR0;
};

struct VS_PARTICLEOUTPUT
{
    float4 Position   : SV_POSITION; // vertex position 
    float2 TextureUV : TEXCOORD0;   // vertex texture coords    
    float4 Color	  : COLOR0;
};

VS_PARTICLEOUTPUT RenderParticlesVS( VS_PARTICLEINPUT input )
{
    VS_PARTICLEOUTPUT Output;
    
    // Standard transform	
	//Output.Position = mul( input.Position, g_mWorldViewProjection);  Æ²¸²..
    Output.Position = mul( float4(input.Position,1), g_mWorldViewProjection);
	
    Output.TextureUV = input.TextureUV; 
    Output.Color = input.Color;	
    
    // Get the world position
    //float3 WorldPos = mul( input.Position, g_mWorld ).xyz;
    
	float3 eye = g_vEyePt;
    
    return Output;    
}

float4 RenderParticlesPS( VS_PARTICLEOUTPUT input ) : SV_TARGET
{ 	
	float4 diffuse = g_txMeshTexture.Sample( samLinear, input.TextureUV.xy );		
	diffuse *=input.Color;		
	return diffuse;
	
}

technique10 RenderParticles
{
    pass P0
    {	
	   SetVertexShader( CompileShader( vs_4_0, RenderParticlesVS( ) ) );
       SetGeometryShader( NULL );
       SetPixelShader( CompileShader( ps_4_0, RenderParticlesPS( ) ) );
	   
	   	   
	   SetBlendState( SrcAlphaBlendingAdd, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );	   	   
       SetDepthStencilState( EnableDepth, 0 );
       SetRasterizerState( RSSolid );
	}
}
	