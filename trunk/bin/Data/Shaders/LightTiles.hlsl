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
#include "Lighting.hlsl"

//=================================================================================================
// Constant buffers
//=================================================================================================
cbuffer CSConstants : register(cb0)
{
    float4x4 View;
	float4x4 Projection;
    float4x4 InvViewProjection;
    float3 CameraPosWS;
	float2 CameraClipPlanes;
    uint2 NumTiles;
    uint2 DisplaySize;
}

//=================================================================================================
// Constants
//=================================================================================================
static const uint ThreadGroupSize = LightTileSize * LightTileSize;

#define MSAA_ (MSAASamples_ >= 2)

//=================================================================================================
// Resources
//=================================================================================================

// Inputs
StructuredBuffer<Light> Lights : register(t0);

#if MSAA_
    Texture2DMS<float> Depth : register(t1);
    Texture2DMS<float4> Normals : register(t2);
    Texture2DMS<float4> DiffuseAlbedo : register(t3);
    Texture2DMS<float4> SpecularAlbedo : register(t4);
    Texture2DMS<float4> Lighting : register(t5);
#else
    Texture2D<float> Depth : register(t1);
    Texture2D<float4> Normals : register(t2);
    Texture2D<float4> DiffuseAlbedo : register(t3);
    Texture2D<float4> SpecularAlbedo : register(t4);
    Texture2D<float4> Lighting : register(t5);
#endif

// Outputs
#if IndexedDeferred_
    RWBuffer<uint> TileOutput : register(u0);
#else
    RWTexture2D<float4> OutputTexture : register(u0);
#endif

// Shared memory
groupshared uint TileMinZ;
groupshared uint TileMaxZ;

// Light list for the tile
groupshared uint TileLightList[MaxLights];
groupshared uint NumTileLights;

#if MSAA_
    // List of pixels needing per-sample lighting
    groupshared uint TileMSAAPixels[ThreadGroupSize];
    groupshared uint NumMSAAPixels;
#endif

// ------------------------------------------------------------------------------------------------
// Converts a z-buffer depth to linear depth
// ------------------------------------------------------------------------------------------------
float LinearDepth(in float zw)
{
    return Projection._43 / (zw - Projection._33);
}

// ------------------------------------------------------------------------------------------------
// Calculates position from a depth value + pixel coordinate
// ------------------------------------------------------------------------------------------------
float3 PositionFromDepth(in float zw, in uint2 pixelCoord)
{
    float2 cpos = (pixelCoord + 0.5f) / DisplaySize;
    cpos *= 2.0f;
    cpos -= 1.0f;
    cpos.y *= -1.0f;
    float4 positionWS = mul(float4(cpos, zw, 1.0f), InvViewProjection);
    return positionWS.xyz / positionWS.w;
}

//=================================================================================================
// Light tile assignment for light indexed deferred, or full tile-based deferred lighting
//=================================================================================================
[numthreads(LightTileSize, LightTileSize, 1)]
void LightTiles(uint3 GroupID : SV_GroupID, uint3 GroupThreadID : SV_GroupThreadID)
{
	uint2 pixelCoord = GroupID.xy * uint2(LightTileSize, LightTileSize) + GroupThreadID.xy;

	const uint groupThreadIdx = GroupThreadID.y * LightTileSize + GroupThreadID.x;

	// Work out Z bounds for our samples
    float minZSample = CameraClipPlanes.y;
    float maxZSample = CameraClipPlanes.x;

    // Calculate view-space Z from Z/W depth
	#if MSAA_
        float zw = Depth.Load(pixelCoord, 0);
    #else
        float zw = Depth[pixelCoord];
    #endif

	float linearZ = LinearDepth(zw);
    float3 positionWS = PositionFromDepth(zw, pixelCoord);

    #if MSAA_
        float zSamples[MSAASamples_];
        zSamples[0] = zw;

        [unroll]
        for(uint ssIdx = 1; ssIdx < MSAASamples_; ++ssIdx)
        {
            zw = Depth.Load(pixelCoord, ssIdx);
            linearZ = LinearDepth(zw);
            minZSample = min(minZSample, linearZ);
            maxZSample = max(maxZSample, linearZ);
            zSamples[ssIdx] = zw;
        }
    #else
        minZSample = min(minZSample, linearZ);
        maxZSample = max(maxZSample, linearZ);
    #endif

    // Initialize shared memory light list and Z bounds
    if(groupThreadIdx == 0)
	{
        NumTileLights = 0;

        #if MSAA_
            NumMSAAPixels = 0;
        #endif

        TileMinZ = 0x7F7FFFFF;      // Max float
        TileMaxZ = 0;
    }

    GroupMemoryBarrierWithGroupSync();

    if(maxZSample >= minZSample)
	{
        InterlockedMin(TileMinZ, asuint(minZSample));
        InterlockedMax(TileMaxZ, asuint(maxZSample));
    }

    GroupMemoryBarrierWithGroupSync();

    float minTileZ = asfloat(TileMinZ);
    float maxTileZ = asfloat(TileMaxZ);

    // Work out scale/bias from [0, 1]
    float2 tileScale = float2(DisplaySize.xy) * rcp(2.0f * float2(LightTileSize, LightTileSize));
    float2 tileBias = tileScale - float2(GroupID.xy);

    // Now work out composite projection matrix
    // Relevant matrix columns for this tile frusta
    float4 c1 = float4(Projection._11 * tileScale.x, 0.0f, tileBias.x, 0.0f);
    float4 c2 = float4(0.0f, -Projection._22 * tileScale.y, tileBias.y, 0.0f);
    float4 c4 = float4(0.0f, 0.0f, 1.0f, 0.0f);

    // Derive frustum planes
    float4 frustumPlanes[6];

    // Sides
    frustumPlanes[0] = c4 - c1;
    frustumPlanes[1] = c4 + c1;
    frustumPlanes[2] = c4 - c2;
    frustumPlanes[3] = c4 + c2;

    // Near/far
    frustumPlanes[4] = float4(0.0f, 0.0f,  1.0f, -minTileZ);
    frustumPlanes[5] = float4(0.0f, 0.0f, -1.0f,  maxTileZ);

    // Normalize frustum planes (near/far already normalized)
    [unroll]
	for (uint i = 0; i < 4; ++i)
        frustumPlanes[i] *= rcp(length(frustumPlanes[i].xyz));

    // Cull lights for this tile
    [unroll]
    for(uint lightIndex = groupThreadIdx; lightIndex < MaxLights; lightIndex += ThreadGroupSize)
	{
		float3 lightPosition = mul(float4(Lights[lightIndex].Position, 1.0f), View).xyz;
		float cutoffRadius = Lights[lightIndex].Falloff;

        // Cull: point light sphere vs tile frustum
        bool inFrustum = true;
        [unroll]
		for(uint i = 0; i < 6; ++i)
		{
            float d = dot(frustumPlanes[i], float4(lightPosition, 1.0f));
            inFrustum = inFrustum && (d >= -cutoffRadius);
        }

        [branch]
		if(inFrustum)
		{
            uint listIndex;
            InterlockedAdd(NumTileLights, 1, listIndex);
            TileLightList[listIndex] = lightIndex;
        }
    }

    GroupMemoryBarrierWithGroupSync();

    #if IndexedDeferred_
        [branch]
        if(groupThreadIdx < MaxLights)
        {
            // Write out the indices
            uint tileIdx = GroupID.y * NumTiles.x + GroupID.x;
            uint bufferIdx = tileIdx * MaxLights + groupThreadIdx;
            uint lightIdx = groupThreadIdx < NumTileLights ? TileLightList[groupThreadIdx] : MaxLights;
            TileOutput[bufferIdx] = lightIdx;
        }
    #else
        #if MSAA_
            float3 normalWS = Normals.Load(pixelCoord, 0).xyz * 2.0f - 1.0f;
            float3 diffuseAlbedo = DiffuseAlbedo.Load(pixelCoord, 0).xyz;
            float3 specularAlbedo = SpecularAlbedo.Load(pixelCoord, 0).xyz;
            float roughness = SpecularAlbedo.Load(pixelCoord, 0).w;
            float3 lighting = Lighting.Load(pixelCoord, 0).xyz;
            uint coverage = uint(DiffuseAlbedo.Load(pixelCoord, 0).w * 255.0f);
        #else
            float3 normalWS = Normals[pixelCoord].xyz * 2.0f - 1.0f;
            float3 diffuseAlbedo = DiffuseAlbedo[pixelCoord].xyz;
            float3 specularAlbedo = SpecularAlbedo[pixelCoord].xyz;
            float roughness = SpecularAlbedo[pixelCoord].w;
            float3 lighting = Lighting[pixelCoord].xyz;
        #endif

        // Light the pixel
        for(uint tLightIdx = 0; tLightIdx < NumTileLights; ++tLightIdx)
        {
            uint lIdx = TileLightList[tLightIdx];
            Light light = Lights[lIdx];

            lighting += CalcPointLight(normalWS, light.Color, diffuseAlbedo, specularAlbedo,
                                       roughness, positionWS.xyz, light.Position, light.Falloff, CameraPosWS);
        }

        #if MSAA_
            // Write out the first subsample
            #if MSAASamples_ == 2
                uint2 outputCoord = pixelCoord * uint2(2, 1);
            #elif MSAASamples_ == 4
                uint2 outputCoord = pixelCoord * uint2(2, 2);
            #endif

            OutputTexture[outputCoord] = float4(lighting, 1.0f);

            // Check if we're on an edge pixel
            bool edgePixel = false;
            for(ssIdx = 1; ssIdx < MSAASamples_; ++ssIdx)
            {
                edgePixel = edgePixel || (abs(zSamples[ssIdx] - zSamples[0]) > 0.01f);
                float3 ssNormal = Normals.Load(pixelCoord, ssIdx).xyz * 2.0f - 1.0f;
                edgePixel = edgePixel || dot(ssNormal, normalWS) < 0.9f;
            }

            if(edgePixel)
            {
                // Add edge pixels to the list
                uint listIndex;
                InterlockedAdd(NumMSAAPixels, 1, listIndex);
                TileMSAAPixels[listIndex] = (pixelCoord.y << 16) | pixelCoord.x;
            }
            else
            {
                // Write the other subsamples
                OutputTexture[outputCoord + uint2(1, 0)] = float4(lighting, 1.0f);

                #if MSAASamples_ == 4
                    OutputTexture[outputCoord + uint2(0, 1)] = float4(lighting, 1.0f);
                    OutputTexture[outputCoord + uint2(1, 1)] = float4(lighting, 1.0f);
                #endif
            }

            GroupMemoryBarrierWithGroupSync();

            const uint extraSamples = (MSAASamples_ - 1);
            const uint numSamples = NumMSAAPixels * extraSamples;

            // Light the edge pixels
            for(uint msaaPixelIdx = groupThreadIdx; msaaPixelIdx < numSamples; msaaPixelIdx += ThreadGroupSize)
            {
                uint listIdx = msaaPixelIdx / extraSamples;
                uint sampleIdx = (msaaPixelIdx % extraSamples) + 1;

                pixelCoord.x = TileMSAAPixels[listIdx] & 0x0000FFFF;
                pixelCoord.y = TileMSAAPixels[listIdx] >> 16;

                zw = Depth.Load(pixelCoord, sampleIdx);
                positionWS = PositionFromDepth(zw, pixelCoord);

                normalWS = Normals.Load(pixelCoord, sampleIdx).xyz * 2.0f - 1.0f;
                diffuseAlbedo = DiffuseAlbedo.Load(pixelCoord, sampleIdx).xyz;
                specularAlbedo = SpecularAlbedo.Load(pixelCoord, sampleIdx).xyz;
                roughness = SpecularAlbedo.Load(pixelCoord, sampleIdx).w;
                lighting = Lighting.Load(pixelCoord, sampleIdx).xyz;

                for(tLightIdx = 0; tLightIdx < NumTileLights; ++tLightIdx)
                {
                    uint lIdx = TileLightList[tLightIdx];
                    Light light = Lights[lIdx];

                    lighting += CalcPointLight(normalWS, light.Color, diffuseAlbedo, specularAlbedo,
                                               roughness, positionWS.xyz, light.Position, light.Falloff, CameraPosWS);
                }

                #if MSAASamples_ == 2
                    outputCoord = pixelCoord * uint2(2, 1);
                    outputCoord.x += sampleIdx % 2;
                #elif MSAASamples_ == 4
                    outputCoord = pixelCoord * uint2(2, 2);
                    outputCoord.x += sampleIdx % 2;
                    outputCoord.y += sampleIdx > 1;
                #endif

                OutputTexture[outputCoord] = float4(lighting, 1.0f);
            }
        #else
            OutputTexture[pixelCoord] = float4(lighting, 1.0f);
        #endif
    #endif
}