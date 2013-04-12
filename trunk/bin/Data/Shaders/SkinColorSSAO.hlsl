#ifndef MATRIX_PALETTE_SIZE_DEFAULT
#define MATRIX_PALETTE_SIZE_DEFAULT 164
#endif

#include "lib.hlsl"

float4x4    g_mWorld : World;                  // World matrix for object
float4x4	g_mView : View;
float4x4    g_mWorldViewProjection : WorldViewProjection;    // World * View * Projection matrix

cbuffer animationvars
{
    matrix    g_matrices[MATRIX_PALETTE_SIZE_DEFAULT];
	float3      g_instanceColor = float3(1,1,1);
}


//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------

struct A_to_VS
{
    float3 vPos         : POSITION;
    float3 vNormal      : NORMAL;
    float3 vTangent		: TANGENT;
    float3 vBinorm		: BINORMAL;
    float2 vTexCoord0   : TEXCOORD0;
    float4 vBones       : BONES;
    float4 vWeights     : WEIGHTS;
	
};

struct VS_to_PS
{
    float4 pos      : SV_Position;
    float3 color    : COLOR;
    float3 tex      : TEXTURE0;
    float3 norm     : NORMAL;    
    float3 worldPos : WORLDPOS;
};

[Vertex shader]

VS_to_PS main( A_to_VS input )
{
    VS_to_PS output;
    float3 vNormalWorldSpace;
                
    matrix finalMatrix;
    finalMatrix = input.vWeights.x * g_matrices[input.vBones.x];
    finalMatrix += input.vWeights.y * g_matrices[input.vBones.y];
    finalMatrix += input.vWeights.z * g_matrices[input.vBones.z];
    finalMatrix += input.vWeights.w * g_matrices[input.vBones.w];    
    
    float4 vAnimatedPos = mul(float4(input.vPos.xyz,1),finalMatrix);
	//float4 vAnimatedPos = float4(input.vPos.xyz,1);
    float4 vAnimatedNormal = mul(float4(input.vNormal.xyz,0),finalMatrix);
    //float4 vAnimatedTangent = mul(float4(input.vTangent.xyz,0),finalMatrix);
        
    // Transform the position from object space to homogeneous projection space
    output.pos = mul(vAnimatedPos, g_mWorldViewProjection);

    // Transform the normal from object space to world space    
    //output.norm = normalize(mul(vAnimatedNormal.xyz, (float3x3)g_mWorld)); // normal to world space
	float3 ViewSpaceNormals = mul( float4( vAnimatedNormal.xyz, 0.0f ), g_mWorld * g_mView ).xyz;
	output.norm = ViewSpaceNormals * 0.5f + 0.5f;

    //output.tangent = normalize(mul(vAnimatedTangent.xyz, (float3x3)g_mWorld));
    //output.binorm = cross(output.norm,output.tangent);
    
    // Do the position too for lighting
    float4 vWorldAnimatedPos = mul(float4(vAnimatedPos.xyz,1), g_mWorld);
    output.worldPos = float4(vWorldAnimatedPos.xyz,1);
    output.tex.xy = float2(input.vTexCoord0.x, input.vTexCoord0.y); 
    output.tex.z = 0;
    
    output.color = g_instanceColor;
    
    return output;    
}

[Fragment shader]
Texture2D g_txDiffuse       : register( t0 );           ;

SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

Texture2D       AmbientOcclusionBuffer : register( t1 );           



float4 main( VS_to_PS input) : SV_Target
{
    //float4 bumps = g_txNormals.Sample(g_samAniso,input.tex.xyz);    
    //bumps = float4(2*bumps.xyz - float3(1,1,1),0);  
   // float3 lightVec = normalize(g_lightPos - input.worldPos);    
    
    // lighting
    float3 Nn = normalize(input.norm);
    //float3 Tn = normalize(input.tangent);
    //float3 Bn = normalize(input.binorm);
    
    //float3x3 TangentToWorld = float3x3(Tn,Bn,Nn);
    //bumps.xy *= Bumps ; 
    //float3 Nb = mul(bumps.xyz,TangentToWorld);
    //Nb = normalize(Nb);
    
    //float3 Vn = normalize(g_EyePos - input.worldPos.xyz);
    //float3 Ln = normalize(g_lightPos - input.worldPos);    
    //float3 Hn = normalize(Vn + Ln);
	
    float4 txcolor = g_txDiffuse.Sample(g_samLinear,input.tex.xyz) ;
	//if( txcolor.a <= 0.7 )
    //    discard;
	//txcolor.a *= MatDiffuse.a;
	//txcolor.rgb *= MatEmissive.rgb;
	//return txcolor;
	float fOcclusion = AmbientOcclusionBuffer[input.pos.xy];
	
	return( float4( fOcclusion, fOcclusion, fOcclusion, fOcclusion ) );

    //return txcolor * (Diffuse * max(dot(Ln,Nb), 0) + Specular * SimpleCookTorrance( Nb, Ln, Vn, Roughness )) + AmbientScale * AmbiColor ;
}
