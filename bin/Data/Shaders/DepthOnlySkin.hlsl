//=================================================================================================
//
//	Light Indexed Deferred Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================
#include "lib.hlsl"
// ================================================================================================
// Constant buffers
// ================================================================================================
cbuffer VSConstants : register(cb0)
{
    float4x4 World;
	float4x4 View;
    float4x4 WorldViewProjection;
	matrix<float,4,3>	JointPalette[164];
}


// ================================================================================================
// Input/Output structs
// ================================================================================================
struct vsInputSkinnedDepth
{
    float4 position : POSITION;
    //float2 uv0 : TEXCOORD0;
    float4 weights : BLENDWEIGHT;
    float4 indices : BLENDINDICES;
};

struct vsOutputStaticDepth
{
    float4 position : POSITION;
    //float  depth : TEXCOORD0;
};

// ================================================================================================
//------------------------------------------------------------------------------
/**
    Vertex shader for skinning (write depth values).
*/
vsOutputStaticDepth vsSkinnedDepth(const vsInputSkinnedDepth vsIn)
{
    vsOutputStaticDepth vsOut;
    float4 skinPos = skinnedPosition(vsIn.position, vsIn.weights, vsIn.indices, JointPalette);
    vsOut.position = mul(skinPos, WorldViewProjection);
    //vsOut.depth = vsOut.position.z;
    return vsOut;
}