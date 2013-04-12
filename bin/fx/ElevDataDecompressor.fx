
cbuffer cbLevelParamsParams
{
    float g_fReconstructionPrecision;
}

#ifndef USE_TEXTURE_ARRAY
#   define USE_TEXTURE_ARRAY 0
#endif

#if USE_TEXTURE_ARRAY
    Texture2DArray<float> g_tex2DElevationMapArr;
    uint g_uiAncestorHeightMapArrInd = 0;
    
    cbuffer cbPatchParams
    {
        float4 g_AncestorRefinedHeightMapUVRange[4];
        float4 g_DescendantInterpRfnmtLabelsIJRange[4];
        uint g_uiDescendantHeightMapArrInd[4];
    }

#else
    Texture2D<float> g_tex2DAncestorHeightMap;

    cbuffer cbPatchParams
    {
        float4 g_AncestorRefinedHeightMapUVRange;
        float4 g_DescendantInterpRfnmtLabelsIJRange;
    }

#endif

Texture2D<uint> g_tex2DAncestorHeightMapRfnmtLabels;
Texture2D<float> g_tex2DAncestorRefinedHeightMap;
Texture2D<int> g_tex2DDescendantInterpRfnmtLabels;

SamplerState samPointClamp
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

SamplerState samLinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = CLAMP;
    AddressV = CLAMP;
};

int QuantizeValue(float fVal, float fQuantizer)
{
    return (int)floor( (fVal + fQuantizer) / (2.f * fQuantizer) );
}

float DequantizeValue(int iQuantizedVal, float fQuantizer)
{
    return (float)iQuantizedVal * (2.f * fQuantizer); 
}

struct RefineAncestorHeightMapVS_OUTPUT
{
    float4 m_ScreenPos_PS : SV_POSITION;
    float2 m_AncestorHeightMapUV : TEXCOORD0; 
    float2 m_RfnmtLabelIJ : IJ_INDICES;
};

RefineAncestorHeightMapVS_OUTPUT RefineAncestorHeightMapVS( in uint VertexId : SV_VertexID )
{
    float2 AncestorHeightMapTexSize = 0;
#if USE_TEXTURE_ARRAY
    float Elems;
    g_tex2DElevationMapArr.GetDimensions(AncestorHeightMapTexSize.x, AncestorHeightMapTexSize.y, Elems);
#else
    g_tex2DAncestorHeightMap.GetDimensions(AncestorHeightMapTexSize.x, AncestorHeightMapTexSize.y);
#endif

    float4 DstTextureMinMaxUV = float4(-1,1,1,-1);
    float4 SrcElevAreaMinMaxUV = float4(0,0,1,1);
    
    // Important: when indices are interpolated to texel center, they get +0.5 shift. This is essential
    // for eliminating rounding errors.
    // If inidces are set such that they are not shifted after interpolation, i.e.:
    // SrcIJIndices = float4(-0.5, -0.5, AncestorHeightMapTexSize.x-0.5, AncestorHeightMapTexSize.y-0.5 );
    // then artifacts caused by rounding errors arise
    //   ||                             ||
    //   ||_______                ______||
    //   ||       |              |      ||
    //   ||   X   |              |   X  ||
    //   ||_______|     .....    |______||
    //   ||   |                      |  ||
    //   ||   |                      |  ||
    //    0  0.5  1                  |  AncestorHeightMapTexSize.x
    //                               AncestorHeightMapTexSize.x-0.5
    float4 SrcIJIndices = float4(0.f, 0.f, AncestorHeightMapTexSize.x, AncestorHeightMapTexSize.y );

    RefineAncestorHeightMapVS_OUTPUT Verts[4] = 
    {
        {float4(DstTextureMinMaxUV.xy, 0.5, 1.0), SrcElevAreaMinMaxUV.xy, SrcIJIndices.xy}, 
        {float4(DstTextureMinMaxUV.xw, 0.5, 1.0), SrcElevAreaMinMaxUV.xw, SrcIJIndices.xw},
        {float4(DstTextureMinMaxUV.zy, 0.5, 1.0), SrcElevAreaMinMaxUV.zy, SrcIJIndices.zy},
        {float4(DstTextureMinMaxUV.zw, 0.5, 1.0), SrcElevAreaMinMaxUV.zw, SrcIJIndices.zw}
    };

    return Verts[VertexId];
}

float RefineAncestorHeightMapPS(RefineAncestorHeightMapVS_OUTPUT In) : SV_TARGET
{
#if USE_TEXTURE_ARRAY
//#   define GET_ELEV(Offset) g_tex2DElevationMapArr.Sample(samPointClamp, float3(ElevationMapUV.xy, g_fElevDataTexArrayIndex), Offset)
    float fCoarseElevation = g_tex2DElevationMapArr.SampleLevel(samPointClamp, float3(In.m_AncestorHeightMapUV.xy, (float)g_uiAncestorHeightMapArrInd), 0);
#else
    float fCoarseElevation = g_tex2DAncestorHeightMap.SampleLevel(samPointClamp, In.m_AncestorHeightMapUV.xy, 0);
#endif
    uint iRefinementLabel = g_tex2DAncestorHeightMapRfnmtLabels.Load( int3(In.m_RfnmtLabelIJ.xy, 0) );

    int iCoarserLevelQuantizedElev = QuantizeValue(fCoarseElevation, g_fReconstructionPrecision*2.f);
    int iFinerLevelQuantizedElev = iCoarserLevelQuantizedElev*2 + iRefinementLabel - 1;
    float fRefinedPatchElev = DequantizeValue( iFinerLevelQuantizedElev, g_fReconstructionPrecision );
    
    return fRefinedPatchElev;
}



struct ComputeDescendantHeightMapsVS_OUTPUT
{
    float4 m_ScreenPos_PS : SV_POSITION;
    float2 m_AncestorRefinedHeightMapUV : TEXCOORD0; 
    float2 m_RfnmtLabelsIJ : IJ_INDICES;
    uint m_ChildID : CHILD_ID;
};

ComputeDescendantHeightMapsVS_OUTPUT InterpolateRefineDescendantHeightMapVS( in uint VertexId : SV_VertexID,
                                                                             in uint ChildID : SV_InstanceID)
{
    float4 DstTextureMinMaxUV = float4(-1,1,1,-1);
#if USE_TEXTURE_ARRAY
    float4 SrcElevAreaMinMaxUV = g_AncestorRefinedHeightMapUVRange[ChildID];//float4(0,0,1,1);
    float4 SrcIJIndices = g_DescendantInterpRfnmtLabelsIJRange[ChildID];
#else
    float4 SrcElevAreaMinMaxUV = g_AncestorRefinedHeightMapUVRange;//float4(0,0,1,1);
    float4 SrcIJIndices = g_DescendantInterpRfnmtLabelsIJRange;
#endif

    ComputeDescendantHeightMapsVS_OUTPUT Verts[4] = 
    {
        {float4(DstTextureMinMaxUV.xy, 0.5, 1.0), SrcElevAreaMinMaxUV.xy, SrcIJIndices.xy, ChildID}, 
        {float4(DstTextureMinMaxUV.xw, 0.5, 1.0), SrcElevAreaMinMaxUV.xw, SrcIJIndices.xw, ChildID},
        {float4(DstTextureMinMaxUV.zy, 0.5, 1.0), SrcElevAreaMinMaxUV.zy, SrcIJIndices.zy, ChildID},
        {float4(DstTextureMinMaxUV.zw, 0.5, 1.0), SrcElevAreaMinMaxUV.zw, SrcIJIndices.zw, ChildID}
    };

    return Verts[VertexId];
}

#if USE_TEXTURE_ARRAY

struct SSlectRenderTargetGS_Output
{
    float4 m_ScreenPos_PS : SV_POSITION;
    float2 m_AncestorRefinedHeightMapUV : TEXCOORD0; 
    float2 m_RfnmtLabelsIJ : IJ_INDICES;
    uint RenderTargetIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3)]
void SlectRenderTargetGS(triangle ComputeDescendantHeightMapsVS_OUTPUT In[3], inout TriangleStream<SSlectRenderTargetGS_Output> triStream)
{
    for(int i=0; i<3; i++)
    {
        SSlectRenderTargetGS_Output Out;
        Out.m_ScreenPos_PS = In[i].m_ScreenPos_PS;
        Out.m_AncestorRefinedHeightMapUV = In[i].m_AncestorRefinedHeightMapUV;
        Out.m_RfnmtLabelsIJ = In[i].m_RfnmtLabelsIJ;
        Out.RenderTargetIndex = g_uiDescendantHeightMapArrInd[In[0].m_ChildID];
        triStream.Append( Out );
    }
}
#endif

float InterpolateRefineDescendantHeightMapPS(
#if USE_TEXTURE_ARRAY
                                      SSlectRenderTargetGS_Output In
#else
                                      ComputeDescendantHeightMapsVS_OUTPUT In
#endif
                                      ) : SV_TARGET
{
    float fInterpolatedElev = g_tex2DAncestorRefinedHeightMap.SampleLevel(samLinearClamp, In.m_AncestorRefinedHeightMapUV.xy, 0);
    int iQuantizedResidual = g_tex2DDescendantInterpRfnmtLabels.Load( int3(In.m_RfnmtLabelsIJ.xy, 0) );

    int iQuantizedInterpolatedElev = QuantizeValue( fInterpolatedElev, g_fReconstructionPrecision );
    float fReconstructedChildElev = DequantizeValue(iQuantizedInterpolatedElev + iQuantizedResidual, g_fReconstructionPrecision );

    return fReconstructedChildElev;
}



shared RasterizerState RS_SolidFill_NoCull
{
    FILLMODE = Solid;
    CullMode = NONE;
};

shared DepthStencilState DSS_DisableDepthTest
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};

// Blend state disabling blending
BlendState NoBlending
{
    BlendEnable[0] = FALSE;
    BlendEnable[1] = FALSE;
    BlendEnable[2] = FALSE;
};

technique11 DecompressHeightMap_FL10
{
    pass PRefineAncestorHeightMap
    {
        SetDepthStencilState( DSS_DisableDepthTest, 0 );
        SetRasterizerState( RS_SolidFill_NoCull );
        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, RefineAncestorHeightMapVS() ) );
        SetHullShader(NULL);
        SetDomainShader(NULL);
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, RefineAncestorHeightMapPS() ) );
    }

    pass PInterpolateRefineDescendantHeightMap
    {
        SetDepthStencilState( DSS_DisableDepthTest, 0 );
        SetRasterizerState( RS_SolidFill_NoCull );
        SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, InterpolateRefineDescendantHeightMapVS() ) );
        SetHullShader(NULL);
        SetDomainShader(NULL);
#if USE_TEXTURE_ARRAY
        SetGeometryShader( CompileShader( gs_4_0, SlectRenderTargetGS() ) );
#else
        SetGeometryShader( NULL );
#endif
        SetPixelShader( CompileShader( ps_4_0, InterpolateRefineDescendantHeightMapPS() ) );
    }
}
