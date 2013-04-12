//=================================================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

cbuffer QuadObject
{
    static const float2 QuadVertices[4] =
    {
        {-1.0, -1.0},
        { 1.0, -1.0},
        {-1.0,  1.0},
        { 1.0,  1.0}
    };

    static const float2 QuadTexCoordinates[4] =
    {
        {0.0, 1.0},
        {1.0, 1.0},
        {0.0, 0.0},
        {1.0, 0.0}
    };

}


//=================================================================================================
// Input/Output structs
//=================================================================================================

struct VSInput
{
    float4 PositionCS : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct VSOutput
{
    float4 PositionCS : SV_Position;
    float2 TexCoord : TEXCOORD;
};

//=================================================================================================
// Vertex Shader
//=================================================================================================
VSOutput QuadVS(in VSInput input)
{
    VSOutput output;

    // Just pass it along
    output.PositionCS = input.PositionCS;
    output.TexCoord = input.TexCoord;

    return output;
}


//--------------------------------------------------------------------------------------
// Fullscreen shaders
//--------------------------------------------------------------------------------------
Texture2D g_MainTexture;
SamplerState SamplerLinearWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

float	    g_MainBufferSizeMultiplier;

struct PSIn_Quad
{
    float4 position     : SV_Position;
    float2 texcoord     : TEXCOORD0;
};

PSIn_Quad FullScreenQuadVS(uint VertexId: SV_VertexID)
{
    PSIn_Quad output;

	output.position = float4(QuadVertices[VertexId],0,1);
    output.texcoord = QuadTexCoordinates[VertexId];
    
    return output;
}

float4 MainToBackBufferPS(PSIn_Quad input) : SV_Target
{
	float4 color;
	//color.rgb = g_MainTexture.SampleLevel(SamplerLinearWrap,float2((input.texcoord.x-0.5)/g_MainBufferSizeMultiplier+0.5f,(input.texcoord.y-0.5)/g_MainBufferSizeMultiplier+0.5f),0).rgb;
	color.rgb = g_MainTexture.SampleLevel(SamplerLinearWrap,float2((input.texcoord.x-0.5)/1.0f+0.5f,(input.texcoord.y-0.5)/1.0f+0.5f),0).rgb;
	color.a=0;
	return color;
}