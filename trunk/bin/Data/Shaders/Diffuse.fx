// -------------------------------------------------------------
// Diffuse shader
// 
// Copyright (c) 2004 Wolfgang F. Engel (wolf@direct3d.net)
// All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// variables that are provided by the application
// -------------------------------------------------------------

cbuffer cbPerFrame 
{
	float4x4 matWorldViewProj;	
	//float4x4 matInvTransposeWorld;	
	//float4 vecLightDir;
}

//float4 vDIC;

// -------------------------------------------------------------
// Output channels
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos  : SV_POSITION;
    //float3 Light : TEXCOORD0;
    //float3 Norm : TEXCOORD1;
	float4 Color : COLOR0;
};


struct PS_INPUT
{
	float4 Pos  : SV_POSITION;
    float4 Color : COLOR0;    
};


// -------------------------------------------------------------
// vertex shader function (input channels)
// -------------------------------------------------------------
VS_OUTPUT VS(float3 Pos : SV_POSITION, float4 Color	  : COLOR)
{
    VS_OUTPUT Out = (VS_OUTPUT)0;      
    
    // transform Position
    Out.Pos = mul(float4(Pos,1), matWorldViewProj);	
	Out.Color = float4(0.75f, 0.75f, 0.75f, 1.00f);// Color;
    
    // output light vector
    //Out.Light = normalize(vecLightDir); 
    
    // transform Normal and normalize it  
    //Out.Norm = normalize(mul(matInvTransposeWorld, Normal));  
   return Out;
}

float4 PS( VS_OUTPUT input ) : SV_Target
{
    return input.Color;
}

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

RasterizerState RSSolid
{
	FillMode = Solid;
	CullMode = None;
};

DepthStencilState DisableDepth
{
    DepthEnable = FALSE;
    DepthWriteMask = 0;
    //DepthFunc = LESS_EQUAL;
	DepthFunc = NEVER;
};

DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};

// -------------------------------------------------------------
// 
// -------------------------------------------------------------
technique11 TShader
{
    pass P0
    {
        // compile shaders
		SetVertexShader( CompileShader( vs_4_0, VS( ) ) );        
        SetGeometryShader( NULL );
        SetPixelShader(  CompileShader( ps_4_0, PS( ) ) );
		
		SetBlendState( SrcColorBlendingAdd, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
        SetRasterizerState( RSSolid );
    }
}
