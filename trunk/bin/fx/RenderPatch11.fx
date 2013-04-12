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


#ifndef PATCH_SIZE
#   define PATCH_SIZE 64
#endif

#ifndef ENABLE_HEIGHT_MAP_MORPH
#   define ENABLE_HEIGHT_MAP_MORPH 1
#endif

#ifndef ENABLE_NORMAL_MAP_MORPH
#   define ENABLE_NORMAL_MAP_MORPH 0
#endif

#ifndef NORMAL_MAP_COMPONENTS
#   define NORMAL_MAP_COMPONENTS xy
#endif

// Texturing modes
#define TM_HEIGHT_BASED 0             // Simple height-based texturing mode using 1D look-up table

#ifndef TEXTURING_MODE
#   define TEXTURING_MODE TM_HEIGHT_BASED
#endif

#define POS_XYZ_SWIZZLE xzy


cbuffer cbImmutable
{
    float4 g_GlobalMinMaxElevation;
    bool g_bFullResHWTessellatedTriang = false;
    float g_fElevationScale = 65535.f * 0.1f;
}

cbuffer cbFrameParams 
{
    float g_fScrSpaceErrorThreshold = 1.f;
    matrix g_mWorldViewProj;
};


cbuffer cbLightParams
{
    float4 g_vDirectionOnSun = {0.f, 0.769666f, 0.638446f, 1.f}; ///< Direction on sun
    float4 g_vSunColorAndIntensityAtGround = {0.640682f, 0.591593f, 0.489432f, 100.f}; ///< Sun color
    float4 g_vAmbientLight = {0.191534f, 0.127689f, 0.25f, 0.f}; ///< Ambient light
}

cbuffer cbPatchParams
{
    float g_PatchXYScale;
    float4 g_PatchLBCornerXY;
    float g_fFlangeWidth = 50.f;
    float g_fMorphCoeff = 0.f;
    int2 g_PatchOrderInSiblQuad;
}


#define MAX_ANISOTROPY 16
#define MIP_LEVELS_MAX 7

cbuffer cb1
{
	int2    g_TextureSize;        // Source texture size
    float2  g_StackCenter;        // Stack center position defined by normalized texture coordinates
    int    g_StackDepth;         // Number of layers in a stack
    float2  g_ScaleFactor;        // SourceImageSize / ClipmapStackSize
    float3  g_MipColors[MIP_LEVELS_MAX];
    
    //uint     g_SphereMeridianSlices;
    //uint     g_SphereParallelSlices;
    float   g_ScreenAspectRatio;
	
	
}

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


//--------------------------------------------------------------------------------------
// Calculate a minimum stack level to fetch from
//--------------------------------------------------------------------------------------
int GetMinimumStackLevel(float2 coordinates)
{
    float2 distance;
    
    distance.x = abs( coordinates.x - g_StackCenter.x );
    distance.x = min( distance.x, 1.0 - distance.x );
    
    distance.y = abs( coordinates.y - g_StackCenter.y );
    distance.y = min( distance.y, 1.0 - distance.y );
            
    return max( log2( distance.x * g_ScaleFactor.x * 4.2 ), log2( distance.y * g_ScaleFactor.y * 4.2 ) );
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



	// Clipmap
Texture2D PyramidTexture;
Texture2D PyramidTextureHM;
Texture2DArray StackTexture;
//


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

    fHeight -= fZShift;
    
    VertexCoords.xy = float2(in_VertexIJ) * PatchXYScale;
    VertexCoords.z = fHeight;

    return VertexCoords;
}

struct RenderPatchVS_Output
{
    float4 Pos_PS           : SV_Position; // vertex position in projection space

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
	float fMorphCoeff : MORPH_COEFF;	
};


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
		
		
    Out.fMorphCoeff = fMorphCoeff;
	

         
    return Out;
}

RenderPatchVS_Output RenderPatchVS(uint PackedVertexIJ : SV_VertexID)
{
    return PatchVSFunction(PackedVertexIJ, g_PatchXYScale, g_PatchLBCornerXY, g_fFlangeWidth, g_fMorphCoeff, g_PatchOrderInSiblQuad);
};

float4 GetClipmapColor(float2 HeightMapUV)
{
	float4 output;
	  // Calculate texture coordinates gradients.
	float2 pixelCoord = float2( HeightMapUV.x * g_TextureSize.x, HeightMapUV.y * g_TextureSize.y );
    float2 dx = ddx( pixelCoord );
    float2 dy = ddy( pixelCoord );
    float d = max( length( dx ), length( dy ) );
    
    // Calculate base mip level and fractional blending part for trilinear filtering.
    float mipLevel = max( log2( d ), GetMinimumStackLevel( HeightMapUV ) );
    float blendGlobal = saturate(g_StackDepth - mipLevel);
        
    float4 color0 = PyramidTexture.Sample( samLinearClamp, HeightMapUV  );
    
    // Make early out for cases where we don't need to fetch from clipmap stack
    if( blendGlobal == 0.0 )
    {
        output = color0;
    }
    else
    {
        // This fractional part defines the factor used for blending
        // between two neighbour stack layers
        float blendLayers = modf(mipLevel, mipLevel);
        blendLayers = saturate(blendLayers);
        
        int nextMipLevel = mipLevel + 1;
        nextMipLevel = clamp( nextMipLevel, 0, g_StackDepth - 1 );
        mipLevel = clamp( mipLevel, 0, g_StackDepth - 1 );
            
        // Here we need to perform proper scaling for input texture coordinates.
        // For each layer we multiply input coordinates by g_ScaleFactor / pow( 2, layer ).
        // We add 0.5 to result, because our stack center with coordinates (0.5, 0.5)
        // starts from corner with coordinates (0, 0) of the original image.
        float2 clipTexCoord = HeightMapUV / pow( 2, mipLevel );
        clipTexCoord *= g_ScaleFactor;
        float4 color1 = StackTexture.Sample( samplerStackLinear, float3( clipTexCoord + 0.5, mipLevel ) );
            
        clipTexCoord = HeightMapUV / pow( 2, nextMipLevel );
        clipTexCoord *= g_ScaleFactor;
        float4 color2 = StackTexture.Sample( samplerStackLinear, float3( clipTexCoord + 0.5, nextMipLevel ) );     
      
	    output = lerp( color0, lerp( color1, color2, blendLayers ), blendGlobal );
    }
	return output;
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
	
	float2 uv = In.HeightMapUV.xy;	
	SurfaceColor.rgb = g_tex2DElevationColor.Sample( samLinearClamp, float2(uv.x, uv.y));
	
	
	// 
	//SurfaceColor = GetClipmapColor(In.HeightMapUV);
	

    float3 Normal; 
    // If uncompressed normal map is used, then normal xy coordinates are stored in "xy" texture comonents.
    // If DXT5 (BC3) compression is used, then x coordiante is stored in "g" texture component and y coordinate is 
    // stored in "a" component
    Normal.xy = g_tex2DNormalMap.Sample(samLinearClamp, In.NormalMapUV.xy).NORMAL_MAP_COMPONENTS;

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
    float DiffuseIllumination = max(0, dot(Normal.xyz, g_vDirectionOnSun.xyz));

    float3 lightColor = g_vSunColorAndIntensityAtGround.rgb;
	
		
    return SurfaceColor.rgb*(DiffuseIllumination*lightColor + g_vAmbientLight.rgb);
	//return output;
}

float3 RenderWireframePatchPS(RenderPatchVS_Output In) : SV_Target
{
    return float3(0,0,0);
}








RasterizerState RS_SolidFill;//Set by the app; can be biased or not
//{
//    FILLMODE = Solid;
//    CullMode = Back;
//    FrontCounterClockwise = true;
//};

RasterizerState RS_SolidFill_NoCull
{
    FILLMODE = Solid;
    CullMode = None;
    //AntialiasedLineEnable = true;
};

RasterizerState RS_Wireframe_NoCull
{
    FILLMODE = Wireframe;
    CullMode = None;
    //AntialiasedLineEnable = true;
};

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
        SetRasterizerState(RS_Wireframe_NoCull);
        SetDepthStencilState( DSS_EnableDepthTest, 0 );

        SetVertexShader( CompileShader(vs_4_0, RenderPatchVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader(ps_4_0, RenderWireframePatchPS() ) );
    }

    pass PRenderZOnly
    {
        SetBlendState( BS_DisableBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetRasterizerState(RS_SolidFill_NoCull);
        SetDepthStencilState( DSS_EnableDepthTest, 0 );

        SetVertexShader( CompileShader(vs_4_0, RenderPatchVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( NULL );
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

BlendState AlphaBlending
{
    BlendEnable[0] = TRUE;
    RenderTargetWriteMask[0] = 0x0F;
    BlendOp = ADD;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = INV_SRC_ALPHA;
};



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
