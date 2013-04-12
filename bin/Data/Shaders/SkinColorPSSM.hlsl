#ifndef MATRIX_PALETTE_SIZE_DEFAULT
#define MATRIX_PALETTE_SIZE_DEFAULT 164
#endif

#include "lib.hlsl"

static const int g_iNumSplits = 4;

float3 g_vLightDir;
float3 g_vLightColor;
float3 g_vAmbient;

int g_iFirstSplit;
int g_iLastSplit;
matrix g_mViewProj;
matrix g_mCropMatrix[g_iNumSplits];

float4x4    g_mWorld : World;                  // World matrix for object
float4x4	g_mView : View;
float4x4    g_mWorldViewProjection : WorldViewProjection;    // World * View * Projection matrix


float3 g_localEye;
float3 g_localLightPos;

matrix g_mTextureMatrix[g_iNumSplits];

TextureCube g_txShadowMapArray;
// for use with SampleCmpLevelZero
SamplerComparisonState g_samShadowMapArray
{
  ComparisonFunc = Less;
  Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
  AddressU = Clamp;
  AddressV = Clamp;
  AddressW = Clamp;
};


float g_fSplitPlane[g_iNumSplits];

cbuffer animationvars
{
    matrix    g_matrices[MATRIX_PALETTE_SIZE_DEFAULT];
	float3      g_instanceColor = float3(1,1,1);
}

Texture2D g_txDiffuse       : DIFFUSE;

SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

Texture2D	g_txBump;

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------

struct A_to_VS
{
    float3 vPos         : POSITION;
    float3 vNormal      : NORMAL;
    float3 vTangent     : TANGENT;
	float3 vBinorm		: BINORMAL;
    float2 vTexCoord0   : TEXCOORD0;
    float4 vBones       : BONES;
    float4 vWeights     : WEIGHTS;
#ifdef PRT_LIGHT
	float3 prt0			: COLOR0;
	float3 prt1			: COLOR1;
	float3 prt2			: COLOR2;
#endif
};

struct VS_to_PS
{
    float4 pos      : SV_Position;    
	float4 vTexCoord[g_iNumSplits + 1] : TEXCOORD;
    float3 lightVec      : TEXCOORD6;
    float3 modelLightVec : TEXCOORD7;
    float3 halfVec       : TEXCOORD8;
    float3 eyePos        : TEXCOORD9;	
	float3 vLighting : COLOR0;	
	float3 vColor : COLOR1;   	
	float3 norm     : NORMAL;    
    float3 worldPos : WORLDPOS;

	
};

[Vertex shader]

float3 GouradShading(float3 vNormal)
{
  return g_vLightColor * saturate(dot(-g_vLightDir, normalize(mul(vNormal, (float3x3)g_mWorld))));
}


VS_to_PS main( A_to_VS input )
{
    VS_to_PS output = (VS_to_PS)0;
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
    output.norm = normalize(mul(vAnimatedNormal.xyz, (float3x3)g_mWorld)); // normal to world space
    //output.tangent = normalize(mul(vAnimatedTangent.xyz, (float3x3)g_mWorld));
    //output.binorm = cross(output.norm,output.tangent);
    
    // Do the position too for lighting
    float4 vWorldAnimatedPos = mul(float4(vAnimatedPos.xyz,1), g_mWorld);
    output.worldPos = float4(vWorldAnimatedPos.xyz,1);
    //output.tex.xy = float2(input.vTexCoord0.x, input.vTexCoord0.y); 
    //output.tex.z = 0;
	// store view space position
	output.vTexCoord[0].xy = float2(input.vTexCoord0.x, input.vTexCoord0.y); 
	output.vTexCoord[0].z = mul(output.worldPos, g_mView);

	// coordinates for shadow maps
	[unroll] for(int i=0;i<g_iNumSplits;i++)
	{
		output.vTexCoord[i+1] = mul(output.worldPos, g_mTextureMatrix[i]);
	}
    
	vsLight2(float4(input.vPos, 1), input.vNormal, input.vTangent, input.vBinorm, g_localEye, g_localLightPos, output.lightVec, output.modelLightVec, output.halfVec, output.eyePos);		
    output.vLighting = GouradShading(input.vNormal);
    
    return output;    
}

[Fragment shader]


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

float4 main( VS_to_PS input) : SV_Target
{

  float fLightingFactor = 1;
  float fDistance = input.vTexCoord[0].z;

  for(int iSplit = 0; iSplit < g_iNumSplits; iSplit++)
  {      
    if(fDistance < g_fSplitPlane[iSplit])
    {
      float4 pos = input.vTexCoord[iSplit + 1];
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
    float4 diffColor = g_txDiffuse.Sample(g_samLinear,input.vTexCoord[0].xy) ;	
	// alpha test
	//clip( diffColor.a < 0.1f ? -1 : 1 );	


	float2 uvOffset = float2(0.0f, 0.0f);
	/*if (BumpScale != 0.0f)
	{
        uvOffset = ParallaxUv(psIn.uv0, BumpSampler, psIn.eyePos);
    }*/
	//float3 tangentSurfaceNormal = (g_txBump.Sample(g_samLinear, input.vTexCoord[0].xy + uvOffset).rgb * 2.0f) - 1.0f;
		
	float4 vColor=diffColor;
	vColor.rgb = vColor * saturate(g_vAmbient.xyz + input.vLighting.xyz*fLightingFactor);
	// Deferred 렌더링시 버그 있음..
	//color4 vColor = psLight(diffColor, tangentSurfaceNormal, input.lightVec, input.modelLightVec, input.halfVec, fLightingFactor);
	return vColor;
	
    //return txcolor * (Diffuse * max(dot(Ln,Nb), 0) + Specular * SimpleCookTorrance( Nb, Ln, Vn, Roughness )) + AmbientScale * AmbiColor ;
}
