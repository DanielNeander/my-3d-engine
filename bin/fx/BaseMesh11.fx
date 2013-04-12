

cbuffer cbFrameParams 
{
    matrix g_mWorldViewProj;
    float4 g_PatchHeightMapUVRegion;
    float4 g_ClearPos;
    float3 g_ClearNormal;
}
cbuffer cbImmutable
{
    float g_tmpScale = 100000.f;
}

cbuffer cbConstants
{
    uint g_uiTexArrayIndex;
}

SamplerState samLinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};


struct RenderBaseMeshVS_Output
{
    float4 Pos_PS           : SV_Position; // vertex position in projection space
    float3 Pos_WS           : PositionWS;
    float3 Normal_WS        : NormalWS;
};

RenderBaseMeshVS_Output RenderBaseMeshVS(in float3 PosWS : POSITION,
                                         in float3 Normal : NORMAL,
                                         in float2 TexUV : TEXCOORD)
{
    RenderBaseMeshVS_Output Out = (RenderBaseMeshVS_Output)0;

    PosWS *= g_tmpScale;
    Out.Pos_PS = mul( float4(PosWS.xyz, 1), g_mWorldViewProj );

    return Out;
}

float3 RenderBaseMeshPS(RenderBaseMeshVS_Output In) : SV_Target
{
    return float3(In.Normal_WS);
};







RenderBaseMeshVS_Output RenderPosAndNormMapVS(in float3 PosWS : POSITION,
                                         in float3 Normal : NORMAL,
                                         in float2 TexUV : TEXCOORD)
{
    RenderBaseMeshVS_Output Out;

    PosWS *= g_tmpScale;

    TexUV.y = 1-TexUV.y;
    float2 ScrPos = (TexUV.xy-g_PatchHeightMapUVRegion.xy) / g_PatchHeightMapUVRegion.zw;
    ScrPos.x = ScrPos.x*2 - 1;
    ScrPos.y = 1 - ScrPos.y*2;

    
    Out.Pos_PS = float4(ScrPos.xy, 0, 1);
    Out.Pos_WS = PosWS;
    Out.Normal_WS = Normal;

    return Out;
}

void RenderPosAndNormMapPS(RenderBaseMeshVS_Output In,
                           out float4 Position : SV_Target0,
                           out float3 Normal   : SV_Target1) 
{
    Position = float4(In.Pos_WS, 0); // The last component is used to mark empty areas
    Normal = normalize(In.Normal_WS);
};


struct SPassThroughGS_Output
{
    RenderBaseMeshVS_Output VSOutput;
    uint RenderTargetIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(3)]
void PassThroughGS(triangle RenderBaseMeshVS_Output In[3], inout TriangleStream<SPassThroughGS_Output> triStream)
{
    for(int i=0; i<3; i++)
    {
        SPassThroughGS_Output Out;
        Out.VSOutput = In[i];
        Out.RenderTargetIndex = g_uiTexArrayIndex;
        triStream.Append( Out );
    }
}


RenderBaseMeshVS_Output ClearTexArraySliceVS( in uint VertexId : SV_VertexID )
{
    RenderBaseMeshVS_Output Out;

    float4 PosPSMinMaxXY = float4(-1,1,1,-1);
    float4 SrcElevAreaMinMaxUV = float4(0,0,1,1);
    
    float4 PosPS[4] = 
    {
        {float4(PosPSMinMaxXY.xy, 0.5, 1.0)}, 
        {float4(PosPSMinMaxXY.xw, 0.5, 1.0)},
        {float4(PosPSMinMaxXY.zy, 0.5, 1.0)},
        {float4(PosPSMinMaxXY.zw, 0.5, 1.0)}
    };

    Out.Pos_PS = PosPS[VertexId];
    Out.Pos_WS = g_ClearPos.xyz;
    Out.Normal_WS = g_ClearNormal;

    return Out;
}

void ClearPosAndNormMapPS(RenderBaseMeshVS_Output In,
                          out float4 Position : SV_Target0,
                          out float3 Normal   : SV_Target1) 
{
    Position = g_ClearPos; // The last component is used to mark empty areas
    Normal = g_ClearNormal;
};

RasterizerState RS_Wireframe_NoCull
{
    FILLMODE = Wireframe;
    CullMode = None;
    //AntialiasedLineEnable = true;
};

RasterizerState RS_SolidFill//Set by the app; can be biased or not
{
    FILLMODE = Solid;
    CullMode = None;
    //FrontCounterClockwise = true;
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

technique11 RenderBaseMesh_FeatureLevel10
{
    pass PRenderSolidModel
    {
        SetBlendState( BS_DisableBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetRasterizerState( RS_SolidFill );
        SetDepthStencilState( DSS_EnableDepthTest, 0 );

        SetVertexShader( CompileShader(vs_4_0, RenderBaseMeshVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader(ps_4_0, RenderBaseMeshPS() ) );
    }

    pass PRenderWireframeModel
    {
        SetBlendState( BS_DisableBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetRasterizerState(RS_Wireframe_NoCull);
        SetDepthStencilState( DSS_EnableDepthTest, 0 );

        SetVertexShader( CompileShader(vs_4_0, RenderBaseMeshVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader(ps_4_0, RenderBaseMeshPS() ) );
    }
}


technique11 RenderPosAndNormMap_FL10
{
    pass P0
    {
        SetBlendState( BS_DisableBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetRasterizerState( RS_SolidFill );
        SetDepthStencilState( DSS_DisableDepthTest, 0 );

        SetVertexShader( CompileShader(vs_4_0, RenderPosAndNormMapVS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader(ps_4_0, RenderPosAndNormMapPS() ) );
    }

    pass PRenderToTexArray
    {
        SetBlendState( BS_DisableBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetRasterizerState( RS_SolidFill );
        SetDepthStencilState( DSS_DisableDepthTest, 0 );

        SetVertexShader( CompileShader(vs_4_0, RenderPosAndNormMapVS() ) );
        // NOTE: it is more efficient to render to single texture and then call UpdateSubresourceRegion()
        SetGeometryShader( CompileShader(gs_4_0, PassThroughGS() ) );
        SetPixelShader( CompileShader(ps_4_0, RenderPosAndNormMapPS() ) );
    }

    pass PClearTexArraySlice
    {
        SetBlendState( BS_DisableBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetRasterizerState( RS_SolidFill );
        SetDepthStencilState( DSS_DisableDepthTest, 0 );

        SetVertexShader( CompileShader(vs_4_0, ClearTexArraySliceVS() ) );
        SetGeometryShader( CompileShader(gs_4_0, PassThroughGS() ) );
        SetPixelShader( CompileShader(ps_4_0, ClearPosAndNormMapPS() ) );
    }
}

