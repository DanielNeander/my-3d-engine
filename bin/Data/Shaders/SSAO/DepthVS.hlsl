//--------------------------------------------------------------------------------
// DepthVS
//
// This vertex shader outputs the linear view space depth to the pixel shader.
// The linear space is used to avoid precision issues on the later calculations.
//
// Copyright (C) 2009 Jason Zink.  All rights reserved.
//--------------------------------------------------------------------------------
#ifndef MATRIX_PALETTE_SIZE_DEFAULT
#define MATRIX_PALETTE_SIZE_DEFAULT 164
#endif

cbuffer Transforms
{
	matrix WorldViewProjMatrix;	
	matrix WorldViewMatrix;	
};

cbuffer animationvars
{
    matrix    BoneMatrices[MATRIX_PALETTE_SIZE_DEFAULT];
};


struct VS_INPUT
{
	float3 position		: POSITION;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
    float3 binorm		: BINORMAL;
	float2 Tex			: TEXCOORD0;
};

struct VS_SKIN_INPUT
{
	float3 position         : POSITION;
    float3 normal      : NORMAL;
    float3 vTangent		: TANGENT;
    float3 vBinorm		: BINORMAL;
    float2 vTexCoord0   : TEXCOORD0;
    float4 vBones       : BONES;
    float4 vWeights     : WEIGHTS;
};

struct VS_OUTPUT
{
	float4 position : SV_Position;
	float4 depth	: COLOR;
};


VS_OUTPUT VSMAIN( in VS_INPUT v )
{
	VS_OUTPUT output;
	output.position = mul( float4( v.position, 1.0f ), WorldViewProjMatrix );
	
	float3 ViewSpaceNormals = mul( float4( v.normal, 0.0f ), WorldViewMatrix ).xyz;
	output.depth.xyz = ViewSpaceNormals * 0.5f + 0.5f;

	output.depth.w = output.position.w / 25.0f;
		
	return output;
}

VS_OUTPUT VS_SKIN_MAIN( in VS_SKIN_INPUT v )
{
	VS_OUTPUT output;

	matrix finalMatrix;
    finalMatrix = v.vWeights.x * BoneMatrices[v.vBones.x];
    finalMatrix += v.vWeights.y * BoneMatrices[v.vBones.y];
    finalMatrix += v.vWeights.z * BoneMatrices[v.vBones.z];
    finalMatrix += v.vWeights.w * BoneMatrices[v.vBones.w];    
    
	float4 vAnimatedPos = mul(float4(v.position,1),finalMatrix);

	output.position = mul( vAnimatedPos, WorldViewProjMatrix );

	float4 vAnimatedNormal = mul(float4(v.normal, 0),finalMatrix);
	
	float3 ViewSpaceNormals = mul( float4( vAnimatedNormal.xyz, 0.0f ), WorldViewMatrix ).xyz;
	output.depth.xyz = ViewSpaceNormals * 0.5f + 0.5f;

	output.depth.w = output.position.w / 25.0f;
		
	return output;
}

