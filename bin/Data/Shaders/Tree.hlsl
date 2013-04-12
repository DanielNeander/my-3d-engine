//--------------------------------------------------------------------------------------
// File: Tutorial04.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
#include "lib.hlsl"

Texture2D g_txDiffuse : register( t0 );
SamplerState g_samLinear : register( s0 );

struct VS_IN
{
	float4 Pos			: POSITION;
	float3 normal		: NORMAL;
	//float3 tangent		: TANGENT;
    //float3 binorm		: BINORMAL;
	float2 Tex			: TEXCOORD0;
	
};

struct VS_OUTPUT
{
    float4 Pos : SV_POSITION; // POSITION È£È¯ ¾ÈµÊ 
	float3 normal : NORMAL;
	float2 Tex : TEXCOORD0;
	float3 Light : TEXCOORD1;
    float3 Norm : TEXCOORD2;
    float3 View : TEXCOORD3;
};



cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 vDIC;
	float4 vSpecIC;
}

cbuffer cbPerFrame : register( b1 )
{
	float3 vecLightDir;
	float3 vecEye;
}

[Vertex shader]

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT main( VS_IN input)
{
    VS_OUTPUT output = (VS_OUTPUT)0;
	
	float4 PosWorld = mul( input.Pos, World );
    output.Pos = PosWorld;
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );	
	output.Tex = input.Tex;    
	
	  // transform Normal
    output.Norm = mul(input.normal, World);		
	output.Light = vecLightDir;	    // L
    output.View = vecEye - PosWorld;	// V
	
    return output;
}

[Fragment shader]

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(float4 Pos : SV_POSITION, float3 normal : NORMAL, float2 Tex : TEXCOORD0, float3 Light: TEXCOORD1, float3 Norm : TEXCOORD2, 
           float3 View : TEXCOORD3) : SV_Target
{
    float4 Acolor =  { 0.25098, 0.435294, 0.901961, 1 };//{ 0.1f, 0.0f, 0.0f, 1.0f};
    
    float3 Normal = normalize(Norm);
    float3 LightDir = normalize(Light);
    float3 Half = normalize(LightDir + normalize(View));    

    // diffuse component
    float4 Diff = g_txDiffuse.Sample(g_samLinear, Tex); // * saturate(dot(Normal, LightDir)); 

	

    // N.H^n
    float Specular = pow(saturate(dot(Normal, Half)), 25); 

    // I = Acolor + Dcolor * Dintensity * N.L + (N.H)n	
	// Alpha test
	clip( Diff.a < 0.1f ? -1 : 1 );	

    //return Acolor + vDIC * Diff + vSpecIC * Specular; 
	Diff.a = MatDiffuse.a;
	return Diff;
}