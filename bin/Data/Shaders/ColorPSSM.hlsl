#include "lib.hlsl"

static const int g_iNumSplits = 4;


float3 g_vLightDir;
float3 g_vLightColor;
float3 g_vAmbient;

static const float3      g_instanceColor = float3(1.0f,1.0f,1.0f);

float g_fSplitPlane[g_iNumSplits];
struct A_to_VS
{
    float4 vPos         : POSITION;
    float3 vNormal      : NORMAL;
    float3 vTangent     : TANGENT;
	float3 vBinorm		: BINORMAL;
	float2 texcoord		: TEXCOORD;
    //float4 vColor   : COLOR;    
};

struct VS_to_PS
{
    float4 pos      : SV_Position;    
	float4 vTexCoord[g_iNumSplits + 1] : TEXCOORD;
	float3 vLighting : COLOR0;	
	float3 vColor : COLOR1;   	
};


matrix g_mViewProj;
float4x4    g_mWorld : World;                  // World matrix for object
float4x4	g_mView : View;
float4x4    g_mWorldViewProjection : WorldViewProjection;    // World * View * Projection matrix


Texture2D g_txDiffuse       : DIFFUSE;

SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};



[Vertex shader]

matrix g_mTextureMatrix[g_iNumSplits];

float3 GouradShading(float3 vNormal)
{
  return g_vLightColor * saturate(dot(-g_vLightDir, normalize(mul(vNormal, (float3x3)g_mWorld))));
}

VS_to_PS main(A_to_VS In)
{
  VS_to_PS o = (VS_to_PS)0;
  // calculate world position
  float4 vPosWorld = mul(In.vPos, g_mWorld);
  // transform vertex
  o.pos = mul(vPosWorld, g_mViewProj);
  
  // store view space position
  o.vTexCoord[0].xy = In.texcoord;
  o.vTexCoord[0].z = mul(vPosWorld, g_mView);

  // coordinates for shadow maps
  [unroll] for(int i=0;i<g_iNumSplits;i++)
  {
    o.vTexCoord[i+1] = mul(vPosWorld, g_mTextureMatrix[i]);
  }

  // calculate per vertex lighting
  o.vLighting = GouradShading(In.vNormal);  
  return o;
}

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------

[Fragment shader]


TextureCube g_txShadowMapArray;

// for use with SampleCmpLevelZero
SamplerComparisonState g_samShadowMapArray
{
  ComparisonFunc = Less;
  Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
};



//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
#line 99
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
	
    
	float4 txcolor = g_txDiffuse.Sample(g_samLinear,input.vTexCoord[0].xy) ;	
	float4 vColor = txcolor; //float4(0.6, 0.8, 0.7, 1.0f);
	//vColor.rgb = input.vColor * saturate(g_vAmbient.xyz + input.vLighting.xyz*fLightingFactor);
	vColor.rgb = vColor * saturate(g_vAmbient.xyz + input.vLighting.xyz*fLightingFactor);	
	return vColor;
	
}
