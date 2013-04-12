
cbuffer cbModifiedAreaPlacement
{
    float4 g_vModifiedAreaPlacement = float4(-1,1,1,-1);
    float4 g_vModificationMinMaxUV = float4(0,0,1,1);
    uint g_TexArrayInd;
}

Texture2D<float> g_tex2Displacement;
Texture2D<float4> g_tex2DiffuseModification;

SamplerState samPointBorder0
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = BORDER;
    AddressV = BORDER;
    BorderColor = float4(0,0,0,0);
};

SamplerState samLinearBorder0
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = BORDER;
    AddressV = BORDER;
    BorderColor = float4(0,0,0,0);
};

#ifndef USE_TEXTURE_ARRAY
#   define USE_TEXTURE_ARRAY 0
#endif

struct GenerateQuadVS_OUTPUT
{
    float4 m_ScreenPos_PS : SV_POSITION;
    float2 m_ModifiedRegionUV : TEXCOORD0;
    uint m_uiInstID : INST_ID; // Unused if texture arrays are not used
};

GenerateQuadVS_OUTPUT GenerateQuadVS( in uint VertexId : SV_VertexID, 
                                      in uint InstID : SV_InstanceID)
{
    float4 DstTextureMinMaxUV = g_vModifiedAreaPlacement;
    float4 DisplacementMinMaxUV = g_vModificationMinMaxUV;
    
    GenerateQuadVS_OUTPUT Verts[4] = 
    {
        {float4(DstTextureMinMaxUV.xy, 0.5, 1.0), DisplacementMinMaxUV.xy, InstID}, 
        {float4(DstTextureMinMaxUV.xw, 0.5, 1.0), DisplacementMinMaxUV.xw, InstID},
        {float4(DstTextureMinMaxUV.zy, 0.5, 1.0), DisplacementMinMaxUV.zy, InstID},
        {float4(DstTextureMinMaxUV.zw, 0.5, 1.0), DisplacementMinMaxUV.zw, InstID}
    };

    return Verts[VertexId];
}

#if USE_TEXTURE_ARRAY

struct SPassThroughGS_Output
{
    GenerateQuadVS_OUTPUT VSOutput;
    uint RenderTargetIndex : SV_RenderTargetArrayIndex;
    float m_fTexArrInd : TEX_ARRAY_IND_FLOAT;
};

[maxvertexcount(3)]
void PassThroughGS(triangle GenerateQuadVS_OUTPUT In[3], 
                   inout TriangleStream<SPassThroughGS_Output> triStream )
{
    uint InstID =In[0].m_uiInstID;
    for(int i=0; i<3; i++)
    {
        SPassThroughGS_Output Out;
        Out.VSOutput = In[i];
        Out.RenderTargetIndex = g_TexArrayInd;
        Out.m_fTexArrInd = (float)g_TexArrayInd;
        triStream.Append( Out );
    }
}

#endif

#if USE_TEXTURE_ARRAY

float ModifyElevMapPS(SPassThroughGS_Output In) : SV_TARGET
{
    return g_tex2Displacement.SampleLevel(samPointBorder0, In.VSOutput.m_ModifiedRegionUV, 0);
}

float4 ModifyDiffuseMapPS(SPassThroughGS_Output In) : SV_TARGET
{
    return g_tex2DiffuseModification.SampleLevel(samLinearBorder0, In.VSOutput.m_ModifiedRegionUV, 0);
}

#else

float ModifyElevMapPS(GenerateQuadVS_OUTPUT In) : SV_TARGET
{
    return g_tex2Displacement.SampleLevel(samPointBorder0, In.m_ModifiedRegionUV, 0);
}

float4 ModifyDiffuseMapPS(GenerateQuadVS_OUTPUT In) : SV_TARGET
{
    return g_tex2DiffuseModification.SampleLevel(samLinearBorder0, In.m_ModifiedRegionUV, 0);
}

#endif

DepthStencilState DSS_DisableDepthTest
{
    DepthEnable = FALSE;
    DepthWriteMask = ZERO;
};


RasterizerState RS_SolidFill_NoCull
{
    FILLMODE = Solid;
    CullMode = NONE;
};

// Additive blending
BlendState AdditiveBlending
{
    BlendEnable[0] = TRUE;
    RenderTargetWriteMask[0] = 0x0F;
    BlendOp = ADD;
    SrcBlend = ONE;
    DestBlend = ONE;
};

technique11 ModifyElevMap_FL10
{
    pass
    {
        SetDepthStencilState( DSS_DisableDepthTest, 0 );
        SetRasterizerState( RS_SolidFill_NoCull );
        SetBlendState( AdditiveBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, GenerateQuadVS() ) );
#if USE_TEXTURE_ARRAY
        SetGeometryShader( CompileShader( gs_4_0, PassThroughGS() ) );
#else
        SetGeometryShader( NULL );
#endif
        SetPixelShader( CompileShader( ps_4_0, ModifyElevMapPS() ) );
    }
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

technique11 ModifyDiffuseMap_FL10
{
    pass
    {
        SetDepthStencilState( DSS_DisableDepthTest, 0 );
        SetRasterizerState( RS_SolidFill_NoCull );
        SetBlendState( AlphaBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );

        SetVertexShader( CompileShader( vs_4_0, GenerateQuadVS() ) );
#if USE_TEXTURE_ARRAY
        SetGeometryShader( CompileShader( gs_4_0, PassThroughGS() ) );
#else
        SetGeometryShader( NULL );
#endif
        SetPixelShader( CompileShader( ps_4_0, ModifyDiffuseMapPS() ) );
    }
}
