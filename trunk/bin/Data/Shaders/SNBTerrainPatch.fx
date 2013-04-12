// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

#include "StructureDefinitions.h"
#include "Lighting.fx"
#include "cascadeshadow.fx"
#include "common.fx"

#ifndef PATCH_SIZE
#   define PATCH_SIZE 64
#endif

#ifndef ENABLE_HEIGHT_MAP_MORPH
#   define ENABLE_HEIGHT_MAP_MORPH 1
#endif

#ifndef ENABLE_NORMAL_MAP_MORPH
#   define ENABLE_NORMAL_MAP_MORPH 1
#endif

#ifndef NORMAL_MAP_COMPONENTS
#   define NORMAL_MAP_COMPONENTS xy
#endif

#ifndef ENABLE_CASCASED_SHADOW
#	define	ENABLE_CASCASED_SHADOW 0
#endif

#ifndef ENABLE_PSSM_SHADOW
#	define	ENABLE_PSSM_SHADOW 1
#endif

// Texturing modes
#define TM_HEIGHT_BASED 0             // Simple height-based texturing mode using 1D look-up table
#define TM_INDEX_MASK 1
#define TM_INDEX_MASK_WITH_NM 2

#ifndef TEXTURING_MODE
//#   define TEXTURING_MODE TM_HEIGHT_BASED
#   define TEXTURING_MODE TM_INDEX_MASK
#endif

#define POS_XYZ_SWIZZLE xzy


shared cbuffer cbImmutable
{
    float4 g_GlobalMinMaxElevation;
    bool g_bFullResHWTessellatedTriang = false;
    float g_fElevationScale = 65535.f * 0.1f;		
	
}

shared cbuffer cbWater 
{

	float		g_RenderCaustics;
	float		g_UseDynamicLOD;	
	float		g_FrustumCullInHS;
	float       g_DynamicTessFactor;
	float       g_StaticTessFactor;	
	float		g_TerrainBeingRendered;
	float		g_HalfSpaceCullSign;
	float		g_HalfSpaceCullPosition;
	float		g_SkipCausticsCalculation;
	int			g_MSSamples;

	// view/time dependent variables			
    float4x4    g_LightModelViewProjectionMatrix;
    float4x4    g_LightModelViewProjectionMatrixInv;    
	float4x4	g_ModelViewProjectionMatrixInv;
    float3		g_CameraPosition;
	float3      g_CameraDirection;


	float3      g_LightPosition;
	float2      g_WaterBumpTexcoordShift;
	float2      g_ScreenSizeInv;
	float	    g_MainBufferSizeMultiplier;
	float		g_ZNear;
	float		g_ZFar;

	// constants defining visual appearance
	float2		g_DiffuseTexcoordScale={130.0,130.0};
	float2		g_RockBumpTexcoordScale={10.0,10.0};
	float		g_RockBumpHeightScale=3.0;
	float2		g_SandBumpTexcoordScale={3.5,3.5};
	float		g_SandBumpHeightScale=0.5;
	float       g_TerrainSpecularIntensity=0.5;
	float2		g_WaterMicroBumpTexcoordScale={225,225};
	float2		g_WaterBumpTexcoordScale={7,7};
	float		g_WaterHeightBumpScale=1.0f;
	float3      g_WaterDeepColor={0.1,0.4,0.7};
	float3      g_WaterScatterColor={0.3,0.7,0.6};
	float3      g_WaterSpecularColor={1,1,1};
	float       g_WaterSpecularIntensity=350.0;

	float       g_WaterSpecularPower=1000;
	float2      g_WaterColorIntensity={0.1,0.2};
	float3      g_AtmosphereBrightColor={1.0,1.1,1.4};
	float3      g_AtmosphereDarkColor={0.6,0.6,0.7};
	float		g_FogDensity = 1.0f/700.0f;    
	float2		g_HeightFieldOrigin = float2(0, 0);
	float		g_HeightFieldSize = 512;
};

shared cbuffer cbFrameParams
{
    float g_fScrSpaceErrorThreshold = 1.f;	
	float4x4	World;
	float4x4	View;
    float4x4	g_mWorldViewProj;
};


shared cbuffer cbLightParams
{
    float4 g_vDirectionOnSun = {0.f, 0.769666f, 0.638446f, 1.f}; ///< Direction on sun
    float4 g_vSunColorAndIntensityAtGround = {0.640682f, 0.591593f, 0.489432f, 100.f}; ///< Sun color
    float4 g_vAmbientLight = {0.191534f, 0.127689f, 0.25f, 0.f}; ///< Ambient light
}

shared cbuffer cbPatchParams
{
    float g_PatchXYScale;
    float4 g_PatchLBCornerXY;
    float g_fFlangeWidth = 50.f;
    float g_fMorphCoeff = 0.f;
    int2 g_PatchOrderInSiblQuad;
}


#define MAX_ANISOTROPY 16
#define MIP_LEVELS_MAX 7

#define CLIP_LAYER_COUNT 5

cbuffer cbClipmap
{
	// Nvidia SDK 10 Version
	//int2    g_TextureSize;        // Source texture size
    //float2  g_StackCenter;        // Stack center position defined by normalized texture coordinates
    //int    g_StackDepth;         // Number of layers in a stack
    //float2  g_ScaleFactor;        // SourceImageSize / ClipmapStackSize
    //float3  g_MipColors[MIP_LEVELS_MAX];
    
    //uint     g_SphereMeridianSlices;
    //uint     g_SphereParallelSlices;
    //float   g_ScreenAspectRatio;	

	// GPG 7 Version
	float3 g_clipLayerAndCenter[CLIP_LAYER_COUNT];
}




shared cbuffer cbLighting
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

static const int g_iNumSplits = 4;

shared cbuffer cbPSSM 
{
	/////////////////////////////////////////////////////////////////////////////////
	//
	// Rendering shadows	
	matrix g_mTextureMatrix[g_iNumSplits];
	float g_fSplitPlane[g_iNumSplits];
};


//=================================================================================================
// Resources
//=================================================================================================
//Texture2D DiffuseMap : register(t0);
//Texture2D NormalMap : register(t1);
StructuredBuffer<Light> Lights : register(t2);
Buffer<uint> TileLights : register(t3);

//--------------------------------------------------------------------------------------
// Calculate local normal using height values from Texture2D
//--------------------------------------------------------------------------------------
float3 GetLocalNormal(Texture2D _texture, SamplerState _sampler, float2 _coordinates)
{
    float3 localNormal;
    
    localNormal.x =  _texture.Sample( _sampler, _coordinates, int2( 1,  0) ).x;
    localNormal.x -= _texture.Sample( _sampler, _coordinates, int2(-1,  0) ).x;
    localNormal.y =  _texture.Sample( _sampler, _coordinates, int2( 0,  1) ).x;
    localNormal.y -= _texture.Sample( _sampler, _coordinates, int2( 0, -1) ).x;
    localNormal.z = sqrt( 1.0 - localNormal.x * localNormal.x - localNormal.y * localNormal.y );
    
    return localNormal;
}

//--------------------------------------------------------------------------------------
// Calculate local normal using height values from Texture2DArray
//--------------------------------------------------------------------------------------
float3 GetLocalNormal_Array(Texture2DArray _texture, SamplerState _sampler, float3 _coordinates)
{
    float3 localNormal;
    
    localNormal.x =  _texture.Sample( _sampler, _coordinates, int2( 1,  0) ).w;
    localNormal.x -= _texture.Sample( _sampler, _coordinates, int2(-1,  0) ).w;
    localNormal.y =  _texture.Sample( _sampler, _coordinates, int2( 0,  1) ).w;
    localNormal.y -= _texture.Sample( _sampler, _coordinates, int2( 0, -1) ).w;
    localNormal.xy *= 5.0 / ( _coordinates.z + 1.0 ); // Scale the normal vector to add relief
    localNormal.z = sqrt( 1.0 - localNormal.x * localNormal.x - localNormal.y * localNormal.y );
     
    return localNormal;
}




#define MAX_ANISOTROPY 16

SamplerState samplerAnisotropic
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = MAX_ANISOTROPY;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samplerPoint
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samLinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

SamplerState samLinearWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};


SamplerState samplerStackLinear
{
    Filter = MIN_MAG_LINEAR_MIP_POINT;
    AddressU = Wrap;
    AddressV = Wrap;
};

// PSSM Shadowmap Sampler
// for use with SampleCmpLevelZero
SamplerComparisonState g_samShadowMapArray
{
  ComparisonFunc = Less;
  Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
};

// PSSM Shadowmap
TextureCube g_txShadowMapArray;

	// Clipmap
//Texture2D PyramidTexture;
//Texture2D PyramidTextureHM;
//Texture2DArray StackTexture;
//

// GPG 7 clipmap 
Texture2D clipTexture0, clipTexture1, clipTexture2, clipTexture3, clipTexture4;
SamplerState clipTexSampler
{
  //texture = <clipTexture0>;
  Filter = MIN_MAG_LINEAR_MIP_POINT;  
  AddressU = wrap;
  AddressV = wrap;
};
 
// static textures
Texture2D g_HeightfieldTexture;
Texture2D g_LayerdefTexture;
Texture2D g_RockBumpTexture;
Texture2D g_RockMicroBumpTexture;
Texture2D g_RockDiffuseTexture;
Texture2D g_SandBumpTexture;
Texture2D g_SandMicroBumpTexture;
Texture2D g_SandDiffuseTexture;
Texture2D g_GrassDiffuseTexture;
Texture2D g_SlopeDiffuseTexture;
Texture2D g_WaterBumpTexture;
Texture2D g_DepthMapTexture;

// rendertarget textures
Texture2D g_SkyTexture;
Texture2D g_ReflectionTexture;
Texture2D g_RefractionTexture;
Texture2D g_RefractionDepthTextureResolved;
Texture2D g_WaterNormalMapTexture;
Texture2D g_DepthTexture;
Texture2D g_MainTexture;
Texture2DMS<float,1> g_RefractionDepthTextureMS1;
Texture2DMS<float,2> g_RefractionDepthTextureMS2;
Texture2DMS<float,4> g_RefractionDepthTextureMS4;



Texture2D<float> g_tex2DElevationMap;
Texture2D g_tex2DNormalMap; // Normal map stores only x,y components. z component is calculated as sqrt(1 - x^2 - y^2)

Texture2D<float> g_tex2DParentElevMap;
Texture2D g_tex2DParentNormalMap; // Normal map stores only x,y components. z component is calculated as sqrt(1 - x^2 - y^2)

Texture2D<float3> g_tex2DElevationColor;

#ifndef ELEV_DATA_EXTENSION
#   define ELEV_DATA_EXTENSION 2
#endif

int2 UnpackVertexIJ(int in_PackedVertexInd)
{
    int2 UnpackedIJ;
    UnpackedIJ.x = in_PackedVertexInd & 0x0FFFF;
    UnpackedIJ.y = (in_PackedVertexInd >> 16) & 0x0FFFF;
    UnpackedIJ.xy -= int2(ELEV_DATA_EXTENSION, ELEV_DATA_EXTENSION);
    return UnpackedIJ;
}

float3 GetVertexCoords(int2 in_VertexIJ,
                       float PatchXYScale,
                       float fZShift,
                       int2 PatchOrderInSiblQuad,
                       float fMorphCoeff)
{
    float3 VertexCoords;
    float fHeight = g_tex2DElevationMap.Load( int3(in_VertexIJ.xy + int2(ELEV_DATA_EXTENSION, ELEV_DATA_EXTENSION), 0) ) * g_fElevationScale;

#if ENABLE_HEIGHT_MAP_MORPH
    // Calculate UV coordiantes in the parent patch's height map
    float2 ParentElevDataTexSize = 0;
    g_tex2DParentElevMap.GetDimensions( ParentElevDataTexSize.x, ParentElevDataTexSize.y );

    // Note that the coordinates must be shifted to the center of the texel
    float2 ParentElevDataUV = (float2(in_VertexIJ.xy + PatchOrderInSiblQuad.xy*PATCH_SIZE)/2.f + float2(ELEV_DATA_EXTENSION+0.5f, ELEV_DATA_EXTENSION+0.5f)) / ParentElevDataTexSize;

    float fParentHeight = g_tex2DParentElevMap.SampleLevel(samLinearClamp, ParentElevDataUV.xy, 0 );

    fHeight = lerp(fHeight, fParentHeight, fMorphCoeff);
#endif

    fHeight; // -= fZShift;
    
    VertexCoords.xy = float2(in_VertexIJ) * PatchXYScale;
    VertexCoords.z = fHeight;

    return VertexCoords;
}

struct RenderPatchVS_Output
{
    float4 Pos_PS           : SV_Position; // vertex position in projection space
	float3 PositionWS       : POSITIONWS; // vertex position in projection space

#if ENABLE_HEIGHT_MAP_MORPH
    float4 HeightMapUV  : TEXCOORD0;
#else 
    float2 HeightMapUV  : TEXCOORD0;
#endif
            
#if ENABLE_NORMAL_MAP_MORPH
    float4 NormalMapUV  : TEXCOORD1;
#else 
    float2 NormalMapUV  : TEXCOORD1;
#endif    
	//nointerpolation	float4 material : TEXCOORD2;
	//linear			float3 weights : TEXCOORD3;

#if ENABLE_CASCASED_SHADOW
	float4 vTexShadow : TEXCOORD2;
	float4 vInterpPos  :TEXCOORD3;
	float2 CipmapUVs[1] : TEXCOORD4;
#else 
	float4 vTexCoord[g_iNumSplits] : TEXCOORD2;	// TEXCOORD3 // TEXCOORD 4 // TEXCOORD5 
	float2 CipmapUVs[1] : TEXCOORD6;

//#if TEXTURING_MODE > TM_HEIGHT_BASED
//	#    if ENABLE_DIFFUSE_TEX_MORPH
//        float4 DiffuseTexUV : TEXCOORD7;
//	#    else 
//        float2 DiffuseTexUV : TEXCOORD7;
//	#    endif
//#endif
#if TEXTURING_MODE > TM_HEIGHT_BASED
    float2 TileTexUV        : TEXCOORD8;
#endif

#endif
	float fMorphCoeff : MORPH_COEFF;		
	float DepthVS	  : DEPTHVS;	
	float DistToCam	  : DIST_TO_CAMERA;
	float4 Depth		  : DEPTH;
};


#if TM_INDEX_MASK == TEXTURING_MODE
    Texture2DArray g_tex2DSrfMtrlArr;
    cbuffer cbTilingParams
    {
        float g_fTilingScale = 100;
    }
#endif


RenderPatchVS_Output PatchVSFunction(
                             uint PackedVertexIJ : SV_VertexID,
                             float PatchXYScale,
                             float4 PatchLBCornerXY,
                             float fPatchFlangeWidth,
                             float fMorphCoeff,
                             int2 PatchOrderInSiblQuad)
{
    RenderPatchVS_Output Out;

    int2 UnpackedIJ = UnpackVertexIJ( PackedVertexIJ );
    float fFlangeShift = 0;
    // Additional vertices on the outer patch border define flange
    if( UnpackedIJ.x < 0 || UnpackedIJ.x > PATCH_SIZE ||
        UnpackedIJ.y < 0 || UnpackedIJ.y > PATCH_SIZE )
        fFlangeShift = fPatchFlangeWidth;

    // Clamp indices to the allowable range 
    UnpackedIJ = clamp( UnpackedIJ.xy, int2(0,0), int2(PATCH_SIZE, PATCH_SIZE));

    // Calculate texture UV coordinates
    float2 ElevDataTexSize;
    float2 NormalMapTexSize;

    g_tex2DElevationMap.GetDimensions( ElevDataTexSize.x, ElevDataTexSize.y );
    g_tex2DNormalMap.GetDimensions( NormalMapTexSize.x, NormalMapTexSize.y );

    float2 HeightMapUVUnShifted = (float2(UnpackedIJ.xy) + float2(ELEV_DATA_EXTENSION, ELEV_DATA_EXTENSION)) / ElevDataTexSize;
    // + float2(0.5,0.5) is necessary to offset the coordinates to the center of the appropriate neight/normal map texel
    Out.HeightMapUV.xy = HeightMapUVUnShifted + float2(0.5,0.5)/ElevDataTexSize.xy;
    // Normal map sizes must be scales of height map sizes!
    Out.NormalMapUV.xy = HeightMapUVUnShifted + float2(0.5,0.5)/NormalMapTexSize.xy;

#if ENABLE_HEIGHT_MAP_MORPH || ENABLE_NORMAL_MAP_MORPH
    float2 ParentElevDataTexSize = 0;
    g_tex2DParentElevMap.GetDimensions( ParentElevDataTexSize.x, ParentElevDataTexSize.y );

    float2 ParentHeightMapUVUnShifted = (float2(UnpackedIJ.xy + PatchOrderInSiblQuad.xy*PATCH_SIZE)/2.f + float2(ELEV_DATA_EXTENSION, ELEV_DATA_EXTENSION)) / ParentElevDataTexSize;

#   if ENABLE_HEIGHT_MAP_MORPH 
        Out.HeightMapUV.zw = ParentHeightMapUVUnShifted + float2(0.5,0.5) / ParentElevDataTexSize.xy;
#   endif

#   if ENABLE_NORMAL_MAP_MORPH
        float2 ParentNormalMapTexSize;
        g_tex2DParentNormalMap.GetDimensions( ParentNormalMapTexSize.x, ParentNormalMapTexSize.y );
        Out.NormalMapUV.zw = ParentHeightMapUVUnShifted + float2(0.5,0.5) / ParentNormalMapTexSize.xy;
#   endif

#endif

    float3 VertexPos_WS= GetVertexCoords(UnpackedIJ, PatchXYScale, fFlangeShift, PatchOrderInSiblQuad, fMorphCoeff);

    VertexPos_WS.xy += PatchLBCornerXY.xy;

    Out.Pos_PS = mul( float4(VertexPos_WS.POS_XYZ_SWIZZLE,1), g_mWorldViewProj );
	Out.PositionWS = VertexPos_WS.POS_XYZ_SWIZZLE;
		
    Out.fMorphCoeff = fMorphCoeff;

#if ENABLE_CASCASED_SHADOW
	Out.vInterpPos = float4(VertexPos_WS.POS_XYZ_SWIZZLE,1);
	Out.vTexShadow = mul(float4(VertexPos_WS.POS_XYZ_SWIZZLE,1), m_mShadow );	// m_mShadow??Shadow.fx ?????關履?????嚥싲갭큔???
#else 
	// coordinates for shadow maps
  [unroll] for(int i=0;i<g_iNumSplits;i++)
  {
    Out.vTexCoord[i] = mul( float4(VertexPos_WS.POS_XYZ_SWIZZLE,1), g_mTextureMatrix[i]);
  }
#endif

	// Calc the view-space depth
	Out.DepthVS = mul(float4(VertexPos_WS.POS_XYZ_SWIZZLE, 1.0f), View).z;	
	Out.DistToCam = length(CameraPosWS - VertexPos_WS.POS_XYZ_SWIZZLE);


	float3 Normal = g_tex2DNormalMap.SampleLevel(samLinearWrap, Out.NormalMapUV.xy, 0);

	#if ENABLE_NORMAL_MAP_MORPH
    float2 ParentNormalXY = g_tex2DParentNormalMap.SampleLevel( samLinearClamp, Out.NormalMapUV.zw, 0).NORMAL_MAP_COMPONENTS;
    Normal.xy = lerp(Normal.xy, ParentNormalXY.xy, fMorphCoeff);
	#endif
    Normal.xy = Normal.xy*2 - 1;
    
    // Since compressed normal map is reconstructed with some errors,
    // it is possible that dot(Normal.xy,Normal.xy) > 1. In this case
    // there will be noticeable artifacts. To get better looking results,
    // clamp minimu z value to sqrt(0.1)
    Normal.z = sqrt( max(1 - dot(Normal.xy,Normal.xy), 0.1) );
    Normal = normalize( Normal );
	
	float3 ViewSpaceNormals = mul( float4( Normal, 0.0f ), View ).xyz;
	Out.Depth.xyz = ViewSpaceNormals * 0.5f + 0.5f;
	Out.Depth.w = Out.Pos_PS.w / 25.0f;

	

	//#if TEXTURING_MODE > TM_HEIGHT_BASED	
	//float2 DiffuseTexSize;
	//g_GrassDiffuseTexture.GetDimensions(DiffuseTexSize.x, DiffuseTexSize.y);
    Out.TileTexUV =	(VertexPos_WS.xy - PatchLBCornerXY) / (PATCH_SIZE * PatchXYScale);	//VertexPos_WS.xy / g_fTilingScale;
	//Out.TileTexUV.y = 1 - Out.TileTexUV.y;
	
	//#endif

	// Copy the texture coordinate through.
	//[unroll] for(i=0; i<CLIP_LAYER_COUNT; i++)
	//	Out.CipmapUVs[i] = Out.HeightMapUV.xy * g_clipLayerAndCenter[i].z;
	Out.CipmapUVs[0] = Out.TileTexUV * g_clipLayerAndCenter[0].z;
         
    return Out;
}

RenderPatchVS_Output RenderPatchVS(uint PackedVertexIJ : SV_VertexID)
{
    return PatchVSFunction(PackedVertexIJ, g_PatchXYScale, g_PatchLBCornerXY, g_fFlangeWidth, g_fMorphCoeff, g_PatchOrderInSiblQuad);
};

//--------------------------------------------------------------------------------------
// Calculate a minimum stack level to fetch from
//--------------------------------------------------------------------------------------
//int GetMinimumStackLevel(float2 coordinates)
//{
//    float2 distance;
//    
//    distance.x = abs( coordinates.x - g_StackCenter.x );
//    distance.x = min( distance.x, 1.0 - distance.x );
//    
//    distance.y = abs( coordinates.y - g_StackCenter.y );
//    distance.y = min( distance.y, 1.0 - distance.y );
//            
//    return max( log2( distance.x * g_ScaleFactor.x * 4.2 ), log2( distance.y * g_ScaleFactor.y * 4.2 ) );
//}

//float4 GetClipmapColor(float2 HeightMapUV)
//{
//	float4 output;
//	  // Calculate texture coordinates gradients.
//	float2 pixelCoord = float2( HeightMapUV.x * g_TextureSize.x, HeightMapUV.y * g_TextureSize.y );
//    float2 dx = ddx( pixelCoord );
//    float2 dy = ddy( pixelCoord );
//    float d = max( length( dx ), length( dy ) );
//    
//    // Calculate base mip level and fractional blending part for trilinear filtering.
//    float mipLevel = max( log2( d ), GetMinimumStackLevel( HeightMapUV ) );
//    float blendGlobal = saturate(g_StackDepth - mipLevel);
//        
//    float4 color0 = PyramidTexture.Sample( samLinearClamp, HeightMapUV  );
//    
//    // Make early out for cases where we don't need to fetch from clipmap stack
//    if( blendGlobal == 0.0 )
//    {
//        output = color0;
//    }
//    else
//    {
//        // This fractional part defines the factor used for blending
//        // between two neighbour stack layers
//        float blendLayers = modf(mipLevel, mipLevel);
//        blendLayers = saturate(blendLayers);
//        
//        int nextMipLevel = mipLevel + 1;
//        nextMipLevel = clamp( nextMipLevel, 0, g_StackDepth - 1 );
//        mipLevel = clamp( mipLevel, 0, g_StackDepth - 1 );
//            
//        // Here we need to perform proper scaling for input texture coordinates.
//        // For each layer we multiply input coordinates by g_ScaleFactor / pow( 2, layer ).
//        // We add 0.5 to result, because our stack center with coordinates (0.5, 0.5)
//        // starts from corner with coordinates (0, 0) of the original image.
//        float2 clipTexCoord = HeightMapUV / pow( 2, mipLevel );
//        clipTexCoord *= g_ScaleFactor;
//        float4 color1 = StackTexture.Sample( samplerStackLinear, float3( clipTexCoord + 0.5, mipLevel ) );
//            
//        clipTexCoord = HeightMapUV / pow( 2, nextMipLevel );
//        clipTexCoord *= g_ScaleFactor;
//        float4 color2 = StackTexture.Sample( samplerStackLinear, float3( clipTexCoord + 0.5, nextMipLevel ) );     
//      
//	    output = lerp( color0, lerp( color1, color2, blendLayers ), blendGlobal );
//    }
//	return output;
//}

float3 GetDeferredLightColor(RenderPatchVS_Output input, float3 diffuseAlbedo, float3 normalWS)
{	  

	float3 specularAlbedo = SpecularAlbedo;

    diffuseAlbedo *= 1.0f - Balance;
    specularAlbedo *= Balance;

	float3 lighting = 0.0f;

    // Add in the point Lights using the per-tile list
    uint2 tileIdx = uint2(input.Pos_PS.xy) / LightTileSize;
    uint bufferIdx = (tileIdx.y * NumTiles.x + tileIdx.x) * MaxLights;

    [loop]
    for(uint tileLightIdx = 0; tileLightIdx < MaxLights; ++tileLightIdx)
    {
        uint lightIndex = TileLights[bufferIdx + tileLightIdx];

        [branch]
        if(lightIndex >= MaxLights)
            break;

        Light light = Lights[lightIndex];
        lighting += CalcPointLight(normalWS, light.Color, diffuseAlbedo,
                                   specularAlbedo, Roughness, input.PositionWS,
                                   light.Position, light.Falloff, CameraPosWS);
    }

    return lighting;
}

static const float3 _vConstantOffset[6] = {
  float3(0.5, 0.5, 0.5),
  float3(-0.5, 0.5, -0.5),
  float3(-0.5, 0.5, -0.5),
  float3(-0.5, -0.5, 0.5),
  float3(-0.5, 0.5, 0.5),
  float3(0.5, 0.5, -0.5)
};

static const float3 _vPosXMultiplier[6] = {
  float3(0, 0, -1),
  float3(0, 0, 1),
  float3(1, 0, 0),
  float3(1, 0, 0),
  float3(1, 0, 0),
  float3(-1, 0, 0)
};

static const float3 _vPosYMultiplier[6] = {
  float3(0, -1, 0),
  float3(0, -1, 0),
  float3(0, 0, 1),
  float3(0, 0, -1),
  float3(0, -1, 0),
  float3(0, -1, 0)
};

float GetPSSMShadow(float Depth, float4 vTexCoord[g_iNumSplits])
{
  float fLightingFactor = 1;
  float fDistance = Depth;

  for(int iSplit = 0; iSplit < g_iNumSplits; iSplit++)
  {      
    if(fDistance < g_fSplitPlane[iSplit])
    {
      float4 pos = vTexCoord[iSplit];
      pos.xyz /= pos.w;

      float3 vCubeCoords;
      vCubeCoords = _vConstantOffset[iSplit] + _vPosXMultiplier[iSplit] * pos.x + _vPosYMultiplier[iSplit] * pos.y;

      // border clamping not possible with TextureCubes
      // so we must simply avoid sampling outside the borders
      //if(pos.x > 0 && pos.y > 0 && pos.x < 1 && pos.y < 1)
      if(min(pos.x, pos.y) > 0 && max(pos.x, pos.y) < 1)
        fLightingFactor = g_txShadowMapArray.SampleCmpLevelZero(g_samShadowMapArray, vCubeCoords, pos.z);
      break;
    }
  }
  return fLightingFactor;
}

// calculating tessellation factor. It is either constant or hyperbolic depending on g_UseDynamicLOD switch
float CalculateTessellationFactor(float distance)
{
	return lerp(g_StaticTessFactor,g_DynamicTessFactor*(1/(0.015*distance)),g_UseDynamicLOD);
}

// to avoid vertex swimming while tessellation varies, one can use mipmapping for displacement maps
// it's not always the best choice, but it effificiently suppresses high frequencies at zero cost
float CalculateMIPLevelForDisplacementTextures(float distance)
{
	return log2(128/CalculateTessellationFactor(distance));
}

// primitive simulation of non-uniform atmospheric fog
float3 CalculateFogColor(float3 pixel_to_light_vector, float3 pixel_to_eye_vector)
{
	return lerp(g_AtmosphereDarkColor,g_AtmosphereBrightColor,0.5*dot(pixel_to_light_vector,-pixel_to_eye_vector)+0.5);
}

// constructing the displacement amount and normal for water surface geometry
float4 CombineWaterNormal(float3 world_position)
{
	float4 water_normal=float4(0.0,4.0,0.0,0.0);
	float water_miplevel;
	float distance_to_camera;
	float4 texvalue;
	float texcoord_scale=1.0;
	float height_disturbance_scale=1.0;
	float normal_disturbance_scale=1.0;
	float2 tc;
	float2 variance={1.0,1.0};

	// calculating MIP level for water texture fetches
	distance_to_camera=length(g_CameraPosition-world_position);
	water_miplevel= CalculateMIPLevelForDisplacementTextures(distance_to_camera)/2.0-2.0;
	tc=(world_position.xz*g_WaterBumpTexcoordScale/g_HeightFieldSize);

	// fetching water heightmap
	for(float i=0;i<5;i++)
	{
		texvalue=g_WaterBumpTexture.SampleLevel(SamplerLinearWrap, tc*texcoord_scale+g_WaterBumpTexcoordShift*0.03*variance,water_miplevel).rbga;
		variance.x*=-1.0;
		water_normal.xz+=(2*texvalue.xz-float2(1.0,1.0))*normal_disturbance_scale;
		water_normal.w += (texvalue.w-0.5)*height_disturbance_scale;
		texcoord_scale*=1.4;
		height_disturbance_scale*=0.65;
		normal_disturbance_scale*=0.65;
	}
	water_normal.w*=g_WaterHeightBumpScale;
	return float4(normalize(water_normal.xyz),water_normal.w);
}

// constructing water surface normal for water refraction caustics
float3 CombineSimplifiedWaterNormal(float3 world_position, float mip_level)
{
	float3 water_normal=float3(0.0,4.0,0.0);

	float water_miplevel;
	float distance_to_camera;
	float4 texvalue;
	float texcoord_scale=1.0;
	float normal_disturbance_scale=1.0;
	float2 tc;
	float2 variance={1.0,1.0};

	tc=(world_position.xz*g_WaterBumpTexcoordScale/g_HeightFieldSize);
	
	// need more high frequensy details for caustics, so summing more "octaves"
	for(float i=0;i<8;i++)
	{
		texvalue=g_WaterBumpTexture.SampleLevel(SamplerLinearWrap, tc*texcoord_scale+g_WaterBumpTexcoordShift*0.03*variance,mip_level/*+i*/).rbga;
		variance.x*=-1.0;
		water_normal.xz+=(2*texvalue.xz-float2(1,1))*normal_disturbance_scale;
		texcoord_scale*=1.4;
		normal_disturbance_scale*=0.85;
	}
	return normalize(water_normal);
}

// calculating water refraction caustics intensity
float CalculateWaterCausticIntensity(float3 worldpos)
{

	float distance_to_camera=length(g_CameraPosition-worldpos);

	float2 refraction_disturbance;
	float3 n;
	float m=0.2;
	float cc=0;
	float k=0.15;
	float water_depth=0.5-worldpos.y;

	float3 pixel_to_light_vector=normalize(g_LightPosition-worldpos);

	worldpos.xz-=worldpos.y*pixel_to_light_vector.xz;
	float3 pixel_to_water_surface_vector=pixel_to_light_vector*water_depth;
	float3 refracted_pixel_to_light_vector;

	// tracing approximately refracted rays back to light
	for(float i=-3; i<=3;i+=1)
		for(float j=-3; j<=3;j+=1)
		{
			n=2.0f*g_WaterNormalMapTexture.SampleLevel(SamplerLinearWrap,(worldpos.xz-g_CameraPosition.xz-float2(200.0,200.0)+float2(i*k,j*k)*m*water_depth)/400.0,0).rgb-float3(1.0f,1.0f,1.0f);
			refracted_pixel_to_light_vector=m*(pixel_to_water_surface_vector+float3(i*k,0,j*k))-0.5*float3(n.x,0,n.z);
			cc+=0.05*max(0,pow(max(0,dot(normalize(refracted_pixel_to_light_vector),normalize(pixel_to_light_vector))),500.0f));
		}
	return cc;
}

float GetRefractionDepth(float2 position)
{
	return g_RefractionDepthTextureResolved.SampleLevel(SamplerLinearClamp,position,0).r;
}

float GetConservativeRefractionDepth(float2 position)
{
	float result =      g_RefractionDepthTextureResolved.SampleLevel(SamplerPointClamp,position + 2.0*float2(g_ScreenSizeInv.x,g_ScreenSizeInv.y),0).r;
	result = min(result,g_RefractionDepthTextureResolved.SampleLevel(SamplerPointClamp,position + 2.0*float2(g_ScreenSizeInv.x,-g_ScreenSizeInv.y),0).r);
	result = min(result,g_RefractionDepthTextureResolved.SampleLevel(SamplerPointClamp,position + 2.0*float2(-g_ScreenSizeInv.x,g_ScreenSizeInv.y),0).r);
	result = min(result,g_RefractionDepthTextureResolved.SampleLevel(SamplerPointClamp,position + 2.0*float2(-g_ScreenSizeInv.x,-g_ScreenSizeInv.y),0).r);
	return result;
}

float3 RenderPatchPS(RenderPatchVS_Output In) : SV_Target
{
    float4 SurfaceColor;

    // It is more accurate to calculate average elevation in the pixel shader rather than in the vertex shader
    float Elev = g_tex2DElevationMap.Sample( samLinearClamp, In.HeightMapUV.xy ) * g_fElevationScale;
#   if ENABLE_HEIGHT_MAP_MORPH
        float ParentElev = g_tex2DParentElevMap.Sample(samLinearClamp, In.HeightMapUV.zw );
        Elev = lerp(Elev, ParentElev, In.fMorphCoeff);
#   endif
    float NormalizedElev = (Elev - g_GlobalMinMaxElevation.x) / (g_GlobalMinMaxElevation.y - g_GlobalMinMaxElevation.x);
    //SurfaceColor.rgb = g_tex2DElevationColor.Sample( samLinearClamp, float2(NormalizedElev, 0.5) );
	
	float2 uv = In.TileTexUV;	
	SurfaceColor.rgb = g_tex2DElevationColor.Sample( samLinearWrap, float2(uv.x, uv.y));
		
	float3 Normal; 
    // If uncompressed normal map is used, then normal xy coordinates are stored in "xy" texture comonents.
    // If DXT5 (BC3) compression is used, then x coordiante is stored in "g" texture component and y coordinate is 
    // stored in "a" component
    //Normal.xy = g_tex2DNormalMap.Sample(samLinearClamp, In.NormalMapUV.xy).NORMAL_MAP_COMPONENTS;
	// fetching base heightmap,normal and moving vertices along y axis		
	//float4 base_texvalue;	
	float3 base_normal;
	float3 detail_normal;
	float3 detail_normal_rotated;
	float4 detail_texvalue;
	float detail_height;
	float4 depthmap_scaler;
	float3x3 normal_rotation_matrix;
	float4 layerdef;
	float distance_to_camera;
	float detailmap_miplevel;
	float2 texcoord0to1 = uv;
	//texcoord0to1.y=1-texcoord0to1.y;

	Normal = g_tex2DNormalMap.SampleLevel(samLinearWrap, In.NormalMapUV.xy, 0);

	#if ENABLE_NORMAL_MAP_MORPH
    float2 ParentNormalXY = g_tex2DParentNormalMap.Sample( samLinearClamp, In.NormalMapUV.zw).NORMAL_MAP_COMPONENTS;
    Normal.xy = lerp(Normal.xy, ParentNormalXY.xy, In.fMorphCoeff);
#endif
    Normal.xy = Normal.xy*2 - 1;
    
    // Since compressed normal map is reconstructed with some errors,
    // it is possible that dot(Normal.xy,Normal.xy) > 1. In this case
    // there will be noticeable artifacts. To get better looking results,
    // clamp minimu z value to sqrt(0.1)
    Normal.z = sqrt( max(1 - dot(Normal.xy,Normal.xy), 0.1) );
    Normal = normalize( Normal );

    Normal = Normal.POS_XYZ_SWIZZLE;
		
	base_normal = Normal;
	base_normal.z=-base_normal.z;

	// calculating MIP level for detail texture fetches
	distance_to_camera= In.DistToCam;
	detailmap_miplevel= CalculateMIPLevelForDisplacementTextures(distance_to_camera);//log2(1+distance_to_camera*3000/(g_HeightFieldSize*g_TessFactor));
	// fetching layer definition texture
	layerdef=g_LayerdefTexture.SampleLevel(SamplerLinearWrap, texcoord0to1,0);

	// default detail texture
	detail_texvalue=g_SandBumpTexture.SampleLevel(SamplerLinearWrap, texcoord0to1*g_SandBumpTexcoordScale,detailmap_miplevel).rbga;
	detail_normal=normalize(2*detail_texvalue.xyz-float3(1,0,1));
	detail_height=(detail_texvalue.w-0.5)*g_SandBumpHeightScale;

	// rock detail texture
	detail_texvalue=g_RockBumpTexture.SampleLevel(SamplerLinearWrap, texcoord0to1*g_RockBumpTexcoordScale,detailmap_miplevel).rbga;
	detail_normal=lerp(detail_normal,normalize(2*detail_texvalue.xyz-float3(1,1.4,1)),layerdef.w);
	detail_height=lerp(detail_height,(detail_texvalue.w-0.5)*g_RockBumpHeightScale,layerdef.w);

	
	//calculating base normal rotation matrix
	normal_rotation_matrix[1]=base_normal;
	normal_rotation_matrix[2]=normalize(cross(float3(-1.0,0.0,0.0),normal_rotation_matrix[1]));
	normal_rotation_matrix[0]=normalize(cross(normal_rotation_matrix[2],normal_rotation_matrix[1]));

	//applying base rotation matrix to detail normal
	detail_normal_rotated=mul(detail_normal,normal_rotation_matrix);

	//adding refraction caustics
	float cc=0;
	
	if((g_SkipCausticsCalculation==0) && (g_RenderCaustics>0)) // doing it only for main
	{
		//cc=CalculateWaterCausticIntensity(In.PositionWS.xyz);
	}

		// fading caustics out at distance
	cc*=(200.0/(200.0+distance_to_camera));

	// fading caustics out as we're getting closer to water surface
	cc*=min(1,max(0,-g_WaterHeightBumpScale-In.PositionWS.y));

	depthmap_scaler=float4(1.0,1.0,detail_height,cc);


	texcoord0to1 *= float2(130, 130); //g_DiffuseTexcoordScale;

	float4 color;
	float3 pixel_to_light_vector = normalize(g_LightPosition-In.PositionWS);
	float3 pixel_to_eye_vector = normalize(CameraPosWS-In.PositionWS);
	float3 microbump_normal; 

	// fetching default microbump normal
	microbump_normal = normalize(2*g_SandMicroBumpTexture.Sample(SamplerAnisotropicWrap, texcoord0to1).rbg - float3 (1.0,1.0,1.0));
	microbump_normal = normalize(lerp(microbump_normal,2*g_RockMicroBumpTexture.Sample(SamplerAnisotropicWrap,texcoord0to1).rbg - float3 (1.0,1.0,1.0), layerdef.w));

	//calculating base normal rotation matrix
	normal_rotation_matrix[1]=detail_normal_rotated;
	normal_rotation_matrix[2]=normalize(cross(float3(-1.0,0.0,0.0),normal_rotation_matrix[1]));
	normal_rotation_matrix[0]=normalize(cross(normal_rotation_matrix[2],normal_rotation_matrix[1]));
	microbump_normal=mul(microbump_normal,normal_rotation_matrix);

	// getting diffuse color
	 
	color = clipTexture0.Sample(clipTexSampler, In.CipmapUVs[0]);
	//color = g_SlopeDiffuseTexture.Sample(SamplerAnisotropicWrap,texcoord0to1);	
	color=lerp(color,g_SandDiffuseTexture.Sample(SamplerAnisotropicWrap, texcoord0to1),layerdef.g*layerdef.g);
	color=lerp(color,g_RockDiffuseTexture.Sample(SamplerAnisotropicWrap, texcoord0to1),layerdef.w*layerdef.w);
	color=lerp(color,g_GrassDiffuseTexture.Sample(SamplerAnisotropicWrap, texcoord0to1),layerdef.b);

	// adding per-vertex lighting defined by displacement of vertex 
	color*=0.5+0.5*min(1.0,max(0.0,depthmap_scaler.b/3.0f+0.5f));

	// calculating pixel position in light view space
	//float4 positionLS = mul(float4(In.PositionWS,1),g_LightModelViewProjectionMatrix);
	//positionLS.xyz/=positionLS.w;
	//positionLS.x=(positionLS.x+1)*0.5;
	//positionLS.y=(1-positionLS.y)*0.5;


	//// fetching shadowmap and shading
	//float dsf=0.75f/4096.0f;
	//float shadow_factor=0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic,positionLS.xy,positionLS.z* 0.995f).r;
	//shadow_factor+=0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic,positionLS.xy+float2(dsf,dsf),positionLS.z* 0.995f).r;
	//shadow_factor+=0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic,positionLS.xy+float2(-dsf,dsf),positionLS.z* 0.995f).r;
	//shadow_factor+=0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic,positionLS.xy+float2(dsf,-dsf),positionLS.z* 0.995f).r;
	//shadow_factor+=0.2*g_DepthTexture.SampleCmp(SamplerDepthAnisotropic,positionLS.xy+float2(-dsf,-dsf),positionLS.z* 0.995f).r;
	//color.rgb*=max(0,dot(pixel_to_light_vector,microbump_normal))*shadow_factor+0.2;


	// adding light from the sky
	color.rgb+=(0.0+0.2*max(0,(dot(float3(0,1,0),microbump_normal))))*float3(0.2,0.2,0.3);

	// making all a bit brighter, simultaneously pretending the wet surface is darker than normal;
	//color.rgb*=0.5+0.8*max(0,min(1,In.PositionWS.y*0.5+0.5));



	// applying refraction caustics
	//color.rgb*=(1.0+max(0,0.4+0.6*dot(pixel_to_light_vector,microbump_normal))*depthmap_scaler.a*(0.4+0.6*shadow_factor));

	// applying fog
	//color.rgb=lerp(CalculateFogColor(pixel_to_light_vector,pixel_to_eye_vector).rgb,color.rgb,min(1,exp(-length(CameraPosWS-In.PositionWS)*g_FogDensity)));
	color.a=length(CameraPosWS-In.PositionWS);


	// 
	//SurfaceColor = GetClipmapColor(In.HeightMapUV);
	// GPG 7 Version
	// The base level can always be sampled as there's nothing behind it...
	// so save some math.
    float3 colAccum = clipTexture0.Sample(clipTexSampler, In.CipmapUVs[0]);
	// Grab the rest, fading based on distance from each layer's center.        
	/*float fade = smoothstep(0.4, 0.5, distance(In.CipmapUVs[1], g_clipLayerAndCenter[1].xy));
	float4 curColor = clipTexture1.Sample(clipTexSampler, In.CipmapUVs[1]);
	colAccum = lerp(curColor, colAccum, fade);

	fade = smoothstep(0.4, 0.5, distance(In.CipmapUVs[2], g_clipLayerAndCenter[2].xy));
	curColor = clipTexture2.Sample(clipTexSampler, In.CipmapUVs[2]);
	colAccum = lerp(curColor, colAccum, fade);

	fade = smoothstep(0.4, 0.5, distance(In.CipmapUVs[3], g_clipLayerAndCenter[3].xy));
	curColor = clipTexture3.Sample(clipTexSampler, In.CipmapUVs[3]);
	colAccum = lerp(curColor, colAccum, fade);

	fade = smoothstep(0.4, 0.5, distance(In.CipmapUVs[4], g_clipLayerAndCenter[4].xy));
	curColor = clipTexture4.Sample(clipTexSampler, In.CipmapUVs[4]);
	colAccum = lerp(curColor, colAccum, fade);*/			
	
	SurfaceColor = color; //In.TextureUV[0].xyxy;	
	
	
  


    //float DiffuseIllumination = max(0, dot(Normal.xyz, g_vDirectionOnSun.xyz));

    //float3 lightColor = g_vSunColorAndIntensityAtGround.rgb;
	//return SurfaceColor.rgb*(DiffuseIllumination*lightColor + g_vAmbientLight.rgb);
	//return float4(SurfaceColor.rgb, 1);
	float3 lightColor = GetDeferredLightColor(In, SurfaceColor.rgb, Normal);			
	  
#if ENABLE_CASCASED_SHADOW
	float4 vVisualizeCascadeColor = GetCascadedShadowColor(In.vTexShadow, In.DepthVS, In.vInterpPos);
	lightColor = vVisualizeCascadeColor.rgb * SurfaceColor.rgb;
#else
	//float ShadowColor = GetPSSMShadow(In.DepthVS, In.vTexCoord );
	 float fLightingFactor = 1;
	  float fDistance = In.DepthVS;

	  for(int iSplit = 0; iSplit < g_iNumSplits; iSplit++)
	  {      
		if(fDistance < g_fSplitPlane[iSplit])
		{
		  float4 pos = In.vTexCoord[iSplit];
		  pos.xyz /= pos.w;

		  float3 vCubeCoords;
		  vCubeCoords = _vConstantOffset[iSplit] + _vPosXMultiplier[iSplit] * pos.x + _vPosYMultiplier[iSplit] * pos.y;

		  // border clamping not possible with TextureCubes
		  // so we must simply avoid sampling outside the borders
		  //if(pos.x > 0 && pos.y > 0 && pos.x < 1 && pos.y < 1)
		  if(min(pos.x, pos.y) > 0 && max(pos.x, pos.y) < 1)
			fLightingFactor = g_txShadowMapArray.SampleCmpLevelZero(g_samShadowMapArray, vCubeCoords, pos.z);
		  break;
		}
	 }
    lightColor = saturate(float3(0.25f, 0.25f, 0.25) +lightColor *  fLightingFactor) * SurfaceColor.rgb ;    	  
#endif
	  // Add in the ambient lighting
	//lightColor += float3(0.2f, 0.5f, 1.0f) * (1.0f / 3.14159f) * SurfaceColor.rgb * 0.25f;  	  	
	return lightColor;	
	//return color;
	//return output;
}

float3 RenderWireframePatchPS(RenderPatchVS_Output In) : SV_Target
{
    return float3(0,0,0);
}

float4 RenderSSAODepthPS(RenderPatchVS_Output In) : SV_Target
{
    return In.Depth;
}


//--------------------------------------------------------------------------------------
// Water shaders
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// Shader Inputs/Outputs
//--------------------------------------------------------------------------------------
struct VSIn_Diffuse
{
    float3 position : POSITION;
    float2 texcoord : TEXCOORD;
    float3 normal   : NORMAL;
};

struct PSIn_Diffuse
{
    float4 position     : SV_Position;
    centroid float2 texcoord     : TEXCOORD0;
    centroid float3 normal       : NORMAL;
    centroid float3 positionWS   : TEXCOORD1;
	centroid float4 layerdef		: TEXCOORD2;
	centroid float4 depthmap_scaler: TEXCOORD3;
};

struct PSIn_Quad
{
    float4 position     : SV_Position;
    float2 texcoord     : TEXCOORD0;
};

struct VSIn_Default
{
	float4 position : POSITION;
    float2 texcoord  : TEXCOORD;
};


struct DUMMY
{
	float Dummmy : DUMMY;
};

struct HSIn_Heightfield
{
    float2 origin		 : ORIGIN;
    float2 size			 : SIZE;
	//float2 HeightMapUV	 : HEIGHTMAP_UV;
};


struct PatchData
{
    float Edges[4]  : SV_TessFactor;
    float Inside[2]	: SV_InsideTessFactor;

	float2 origin   : ORIGIN;
    float2 size     : SIZE;
};
//--------------------------------------------------------------------------------------
// Heightfield shaders
//--------------------------------------------------------------------------------------

HSIn_Heightfield PassThroughVS(uint PackedVertexIJ : SV_VertexID, float4 PatchParams : PATCH_PARAMETERS)
{	
    HSIn_Heightfield output;
   
	//int2 UnpackedIJ = UnpackVertexIJ( PackedVertexIJ );
 //   float fFlangeShift = 0;
 //   
 //   // Clamp indices to the allowable range 
 //   UnpackedIJ = clamp( UnpackedIJ.xy, int2(0,0), int2(PATCH_SIZE, PATCH_SIZE));

 //   // Calculate texture UV coordinates
 //   float2 ElevDataTexSize;    
 //   
	//g_tex2DElevationMap.GetDimensions( ElevDataTexSize.x, ElevDataTexSize.y );
 //   
 //   float2 HeightMapUVUnShifted = (float2(UnpackedIJ.xy) + float2(ELEV_DATA_EXTENSION, ELEV_DATA_EXTENSION)) / ElevDataTexSize;
 //   // + float2(0.5,0.5) is necessary to offset the coordinates to the center of the appropriate neight/normal map texel
 //   output.HeightMapUV.xy = HeightMapUVUnShifted + float2(0.5,0.5)/ElevDataTexSize.xy;
		    
//#if ENABLE_HEIGHT_MAP_MORPH || ENABLE_NORMAL_MAP_MORPH
//    float2 ParentElevDataTexSize = 0;
//    g_tex2DParentElevMap.GetDimensions( ParentElevDataTexSize.x, ParentElevDataTexSize.y );
//
//    float2 ParentHeightMapUVUnShifted = (float2(UnpackedIJ.xy + PatchOrderInSiblQuad.xy*PATCH_SIZE)/2.f + float2(ELEV_DATA_EXTENSION, ELEV_DATA_EXTENSION)) / ParentElevDataTexSize;
//
//#   if ENABLE_HEIGHT_MAP_MORPH 
//        Out.HeightMapUV.zw = ParentHeightMapUVUnShifted + float2(0.5,0.5) / ParentElevDataTexSize.xy;
//#   endif
//#endif
	
	output.origin = PatchParams.xy;
    output.size = PatchParams.zw;
    return output;
}


RasterizerState RS_SolidFill//Set by the app; can be biased or not
{
    FILLMODE = Solid;
    CullMode = Back;
    FrontCounterClockwise = true;
};

RasterizerState RS_SolidFill_NoCull
{
    FILLMODE = Solid;
    CullMode = None;
    //AntialiasedLineEnable = true;
};

//RasterizerState RS_Wireframe_NoCull
//{
//    FILLMODE = Wireframe;
//    CullMode = None;
//    //AntialiasedLineEnable = true;
//};

BlendState BS_DisableBlending
{
    BlendEnable[0] = FALSE;
    BlendEnable[1] = FALSE;
    BlendEnable[2] = FALSE;
};

DepthStencilState DSS_EnableDepthTest
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
};

DepthStencilState DSS_DisableDepthTest
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

technique11 RenderPatch_FeatureLevel10
{
    pass PRenderSolidModel
    {
		// Depth Test Fail 
        SetBlendState( BS_DisableBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetRasterizerState( RS_SolidFill );
        SetDepthStencilState( DSS_EnableDepthTest, 0 );

        SetVertexShader( CompileShader(vs_5_0, RenderPatchVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader(ps_5_0, RenderPatchPS() ) );
    }

    pass PRenderWireframeModel
    {
        SetBlendState( BS_DisableBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        //SetRasterizerState(RS_Wireframe_NoCull);
        SetDepthStencilState( DSS_EnableDepthTest, 0 );

        SetVertexShader( CompileShader(vs_5_0, RenderPatchVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader(ps_5_0, RenderWireframePatchPS() ) );
    }

    pass PRenderZOnly
    {
        SetBlendState( BS_DisableBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetRasterizerState(RS_SolidFill_NoCull);
        SetDepthStencilState( DSS_EnableDepthTest, 0 );

        SetVertexShader( CompileShader(vs_5_0, RenderPatchVS() ) );
        SetGeometryShader( NULL );
        //SetPixelShader( NULL );
		SetPixelShader( CompileShader(ps_5_0, RenderSSAODepthPS() ) );
    }
}




float4 g_vTerrainMapPos_PS;
struct GenerateQuadVS_OUTPUT
{
    float4 m_ScreenPos_PS : SV_POSITION;
    float2 m_ElevationMapUV : TEXCOORD0;
};

GenerateQuadVS_OUTPUT GenerateQuadVS( in uint VertexId : SV_VertexID)
{
    float4 DstTextureMinMaxUV = float4(-1,1,1,-1);
    DstTextureMinMaxUV.xy = g_vTerrainMapPos_PS.xy;
    DstTextureMinMaxUV.zw = DstTextureMinMaxUV.xy + g_vTerrainMapPos_PS.zw * float2(1,1);
    float2 ElevDataTexSize;
    g_tex2DElevationMap.GetDimensions( ElevDataTexSize.x, ElevDataTexSize.y );

    float4 SrcElevAreaMinMaxUV = float4(ELEV_DATA_EXTENSION/ElevDataTexSize.x, 
										ELEV_DATA_EXTENSION/ElevDataTexSize.y,
										1-ELEV_DATA_EXTENSION/ElevDataTexSize.x,
										1-ELEV_DATA_EXTENSION/ElevDataTexSize.y);
    
    GenerateQuadVS_OUTPUT Verts[4] = 
    {
        {float4(DstTextureMinMaxUV.xy, 0.5, 1.0), SrcElevAreaMinMaxUV.xy}, 
        {float4(DstTextureMinMaxUV.xw, 0.5, 1.0), SrcElevAreaMinMaxUV.xw},
        {float4(DstTextureMinMaxUV.zy, 0.5, 1.0), SrcElevAreaMinMaxUV.zy},
        {float4(DstTextureMinMaxUV.zw, 0.5, 1.0), SrcElevAreaMinMaxUV.zw}
    };

    return Verts[VertexId];
}


float4 RenderHeigtMapPreviewPS(GenerateQuadVS_OUTPUT In) : SV_TARGET
{
	float fHeight = g_tex2DElevationMap.SampleLevel( samLinearClamp, In.m_ElevationMapUV.xy, 0, int2(0,0) ) * g_fElevationScale;
	fHeight = (fHeight-g_GlobalMinMaxElevation.x)/g_GlobalMinMaxElevation.y;
	return float4(fHeight.xxx, 0.8);
}

//BlendState AlphaBlending
//{
//    BlendEnable[0] = TRUE;
//    RenderTargetWriteMask[0] = 0x0F;
//    BlendOp = ADD;
//    SrcBlend = SRC_ALPHA;
//    DestBlend = INV_SRC_ALPHA;
//    SrcBlendAlpha = ZERO;
//    DestBlendAlpha = INV_SRC_ALPHA;
//};



technique11 RenderHeightMapPreview_FeatureLevel10
{
    pass
    {
        SetDepthStencilState( DSS_DisableDepthTest, 0 );
        SetRasterizerState( RS_SolidFill_NoCull );
        SetBlendState( AlphaBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, GenerateQuadVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, RenderHeigtMapPreviewPS() ) );
    }
}



struct VS_OUTPUT
{
    float4 Pos : SV_POSITION;     // Projection coord
	float4 Color : COLOR;
};

#pragma warning (disable: 3571) // warning X3571: pow(f, e) will not work for negative f


VS_OUTPUT RenderBoundBoxVS( uint id : SV_VertexID,
                            float3 BoundBoxMinXYZ : BOUND_BOX_MIN_XYZ,
                            float3 BoundBoxMaxXYZ : BOUND_BOX_MAX_XYZ,
                            float4 BoundBoxColor  : BOUND_BOX_COLOR )
{
    float4 BoxCorners[8]=
    {
        float4(BoundBoxMinXYZ.x, BoundBoxMinXYZ.y, BoundBoxMinXYZ.z, 1.f),
        float4(BoundBoxMinXYZ.x, BoundBoxMaxXYZ.y, BoundBoxMinXYZ.z, 1.f),
        float4(BoundBoxMaxXYZ.x, BoundBoxMaxXYZ.y, BoundBoxMinXYZ.z, 1.f),
        float4(BoundBoxMaxXYZ.x, BoundBoxMinXYZ.y, BoundBoxMinXYZ.z, 1.f),

        float4(BoundBoxMinXYZ.x, BoundBoxMinXYZ.y, BoundBoxMaxXYZ.z, 1.f),
        float4(BoundBoxMinXYZ.x, BoundBoxMaxXYZ.y, BoundBoxMaxXYZ.z, 1.f),
        float4(BoundBoxMaxXYZ.x, BoundBoxMaxXYZ.y, BoundBoxMaxXYZ.z, 1.f),
        float4(BoundBoxMaxXYZ.x, BoundBoxMinXYZ.y, BoundBoxMaxXYZ.z, 1.f),
    };

    const int RibIndices[12*2] = {0,1, 1,2, 2,3, 3,0,
                                  4,5, 5,6, 6,7, 7,4,
                                  0,4, 1,5, 2,6, 3,7};
    VS_OUTPUT Out;
    Out.Pos = mul( BoxCorners[RibIndices[id]], g_mWorldViewProj );
    Out.Color = pow(BoundBoxColor, 2.2); // gamma correction
    return Out;
}

float4 g_vQuadTreePreviewPos_PS;
float4 g_vScreenPixelSize;
VS_OUTPUT RenderQuadTreeVS( uint id : SV_VertexID,
                            float3 BoundBoxMinXYZ : BOUND_BOX_MIN_XYZ,
                            float3 BoundBoxMaxXYZ : BOUND_BOX_MAX_XYZ,
                            float4 BoundBoxColor  : BOUND_BOX_COLOR )
{
	BoundBoxMinXYZ.z = 1 - BoundBoxMinXYZ.z;
	BoundBoxMaxXYZ.z = 1 - BoundBoxMaxXYZ.z;
	BoundBoxMinXYZ.xz *= g_vQuadTreePreviewPos_PS.zw * float2(1,-1);
	BoundBoxMaxXYZ.xz *= g_vQuadTreePreviewPos_PS.zw * float2(1,-1);
	BoundBoxMinXYZ.xz += g_vQuadTreePreviewPos_PS.xy;
	BoundBoxMaxXYZ.xz += g_vQuadTreePreviewPos_PS.xy;
	
	BoundBoxMinXYZ.xz += g_vScreenPixelSize.xy/2.f * float2(1,-1);
	BoundBoxMaxXYZ.xz -= g_vScreenPixelSize.xy/2.f * float2(1,-1);
    float4 QuadCorners[4]=
    {
        float4(BoundBoxMinXYZ.x, BoundBoxMinXYZ.z, 0.5, 1.f),
        float4(BoundBoxMinXYZ.x, BoundBoxMaxXYZ.z, 0.5, 1.f),
        float4(BoundBoxMaxXYZ.x, BoundBoxMaxXYZ.z, 0.5, 1.f),
        float4(BoundBoxMaxXYZ.x, BoundBoxMinXYZ.z, 0.5, 1.f),
    };

    const int RibIndices[4*2] = {0,1, 1,2, 2,3, 3,0};
    VS_OUTPUT Out;
    Out.Pos = QuadCorners[RibIndices[id]];
    Out.Color = pow(BoundBoxColor, 2.2); // gamma correction
    return Out;
}


float4 RenderBoundBoxPS(VS_OUTPUT In) : SV_TARGET
{
    return In.Color;
}

technique11 RenderBoundBox_FeatureLevel10
{
    pass P0
    {
        SetBlendState( BS_DisableBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetRasterizerState( RS_SolidFill_NoCull );
        SetDepthStencilState( DSS_EnableDepthTest, 0 );

        SetVertexShader( CompileShader( vs_4_0, RenderBoundBoxVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, RenderBoundBoxPS() ) );
    }
}

technique11 RenderQuadTree_FeatureLevel10
{
    pass P0
    {
        SetBlendState( BS_DisableBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetRasterizerState( RS_SolidFill_NoCull );
        SetDepthStencilState( DSS_DisableDepthTest, 0 );

        SetVertexShader( CompileShader( vs_4_0, RenderQuadTreeVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, RenderBoundBoxPS() ) );
    }
}
