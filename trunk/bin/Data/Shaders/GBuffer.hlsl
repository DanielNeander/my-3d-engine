//=================================================================================================
//
//	Light Indexed Deferred Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

//=================================================================================================
// Includes
//=================================================================================================
#include "StructureDefinitions.h"


//=================================================================================================
// Constant buffers
//=================================================================================================
cbuffer VSConstants : register(cb0)
{
    float4x4 World;
	float4x4 View;
    float4x4 WorldViewProjection;
}

cbuffer PSConstants : register(cb0)
{
    float3 LightDirWS;
    float3 LightColor;
    float3 CameraPosWS;
    float3 DiffuseAlbedo;
    float3 SpecularAlbedo;
    float Balance;
    float Roughness;
    uint2 NumTiles;
    bool EnableNormalMapping;
}

//=================================================================================================
// Resources
//=================================================================================================
Texture2D DiffuseMap : register(t0);
Texture2D NormalMap : register(t1);

SamplerState AnisoSampler : register(s0);

//=================================================================================================
// Input/Output structs
//=================================================================================================
struct VSInput
{
    float4 PositionOS 		    : POSITION;
    float3 NormalOS 		    : NORMAL;
    float2 TexCoord 		    : TEXCOORD0;
    //float3 TangentOS 		    : TANGENT;
    //float3 BitangentOS		    : BITANGENT;
};

struct VSOutput
{
    float4 PositionCS 		    : SV_Position;
    float3 PositionWS 		    : POSITIONWS;
    float3 NormalWS 		    : NORMALWS;
    //float3 TangentWS 		    : TANGENTWS;
    //float3 BitangentWS 		    : BITANGENTWS;
    float2 TexCoord 		    : TEXCOORD;
	float DepthVS			    : DEPTHVS;
};

struct PSInput
{
    float4 PositionSS 		    : SV_Position;
    float3 PositionWS 		    : POSITIONWS;
	float3 NormalWS 		    : NORMALWS;
    //float3 TangentWS 		    : TANGENTWS;
    //float3 BitangentWS 		    : BITANGENTWS;
    float2 TexCoord 		    : TEXCOORD;
	float DepthVS			    : DEPTHVS;
    uint Coverage               : SV_Coverage;
};

//=================================================================================================
// Vertex Shader
//=================================================================================================
VSOutput VS(in VSInput input, in uint VertexID : SV_VertexID)
{
    VSOutput output;

    // Calc the world-space position
    output.PositionWS = mul(input.PositionOS, World).xyz;

	// Calc the view-space depth
	output.DepthVS = mul(float4(output.PositionWS, 1.0f), View).z;

    // Calc the clip-space position
    output.PositionCS = mul(input.PositionOS, WorldViewProjection);

	// Rotate the normal into world space
    output.NormalWS = normalize(mul(input.NormalOS, (float3x3)World));

    // Rotate the rest of the tangent frame into world space
    //output.TangentWS = normalize(mul(input.TangentOS, (float3x3)World));
    //output.BitangentWS = normalize(mul(input.BitangentOS, (float3x3)World));

    // Pass along the texture coordinate
    output.TexCoord = input.TexCoord;

    return output;
}

// G-Buffer output structure
struct GBuffer
{
    float4 Normal           : SV_Target0;
    float4 DiffuseAlbedo    : SV_Target1;
    float4 SpecularAlbedo   : SV_Target2;
    float4 Lighting         : SV_Target3;
};

//=================================================================================================
// Pixel Shader
//=================================================================================================
GBuffer PS(in PSInput input)
{
	float3 vtxNormal = normalize(input.NormalWS);

    // Normalize after interpolation
    float3 normalWS = vtxNormal;

    //if(EnableNormalMapping)
    //{
    //    float3 normalTS = float3(0, 0, 1);
    //    float3 tangentWS = normalize(input.TangentWS);
    //    float3 binormalWS = normalize(input.BitangentWS);
    //    float3x3 tangentToWorld = float3x3(tangentWS, binormalWS, normalWS);

    //    // Sample the normal map, and convert the normal to world space
    //    normalTS.xyz = NormalMap.Sample(AnisoSampler, input.TexCoord).xyz * 2.0f - 1.0f;
    //    normalTS.z = sqrt(1.0f - ((normalTS.x * normalTS.x) + (normalTS.y * normalTS.y)));
    //    normalWS = normalize(mul(normalTS, tangentToWorld));
    //}

    float4 texSample = DiffuseMap.Sample(AnisoSampler, input.TexCoord);
    float3 diffuseAlbedo = texSample.xyz;
	float3 specularAlbedo = SpecularAlbedo;

    diffuseAlbedo *= 1.0f - Balance;
    specularAlbedo *= Balance;

	float3 lighting = 0.0f;
    lighting += float3(0.2f, 0.5f, 1.0f) * (1.0f / 3.14159f) * diffuseAlbedo * 0.25f;

    GBuffer gBuffer;
    gBuffer.Normal = float4(normalWS * 0.5f + 0.5f, 1.0f);
    gBuffer.DiffuseAlbedo = float4(diffuseAlbedo, input.Coverage / 255.0f);
    gBuffer.SpecularAlbedo = float4(specularAlbedo, Roughness);
    gBuffer.Lighting = float4(lighting, 1.0f);
    return gBuffer;
}