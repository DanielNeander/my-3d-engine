//--------------------------------------------------------------------------------------
// File: Tutorial04.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
    float3 Pos : POSITION;	
	float3 norm : NORMAL;	
	float2 uv	: TEXCOORD;	
	//float4 color : COLOR;    
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float4 col : COLOR;	
};

cbuffer ConstantBuffer //: register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 MatDiffuse = float4(0.5f, 0.5, 0.5f, 0.5f);
}



[Vertex shader]
//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main( VS_OUTPUT In)
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( float4(In.Pos,1), World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );	
	output.col = MatDiffuse;
    return output;
}

[Fragment shader]

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main( PS_INPUT input) : SV_Target
{    
	return input.col;
}

