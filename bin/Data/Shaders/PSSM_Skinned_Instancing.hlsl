// PSSM
static const int g_iNumSplits = 4;
int g_iFirstSplit;
int g_iLastSplit;
matrix g_mWorld;
float4x4 g_mViewProj;
matrix g_mCropMatrix[g_iNumSplits];

#ifndef MATRIX_PALETTE_SIZE_DEFAULT
	#define MATRIX_PALETTE_SIZE_DEFAULT 164
#endif

cbuffer animationvars
{
    matrix    g_matrices[MATRIX_PALETTE_SIZE_DEFAULT];	
}
/////////////////////////////////////////////////////////////////////////////////
//
// Instacing method

struct VS_INPUT_SHADOWMAP_INSTANCING
{
  float3 vPos		  : POSITION;	// float4 형이면 Vertex Shader linkage 에러 뜸..
  float3 vNormal      : NORMAL;
  float3 vTangent     : TANGENT;
  float3 vBinorm	  : BINORMAL;
  float2 vTexCoord0   : TEXCOORD0;
  float4 vBones		  : BONES;  
  float4 vWeights	  : WEIGHTS;
  uint iInstance	  : SV_InstanceID;  
};

struct VS_OUTPUT_SHADOWMAP_INSTANCING
{
  float4 vPos : POSITION;  
  uint iSplit : TEXTURE0;
};

struct GS_OUTPUT_SHADOWMAP
{
  float4 vPos : SV_POSITION;
  uint RTIndex : SV_RenderTargetArrayIndex;
};

[Vertex shader]

VS_OUTPUT_SHADOWMAP_INSTANCING main( VS_INPUT_SHADOWMAP_INSTANCING input )
{
  VS_OUTPUT_SHADOWMAP_INSTANCING o = (VS_OUTPUT_SHADOWMAP_INSTANCING)0;	

	matrix finalMatrix;
    finalMatrix = input.vWeights.x * g_matrices[input.vBones.x];
    finalMatrix += input.vWeights.y * g_matrices[input.vBones.y];
    finalMatrix += input.vWeights.z * g_matrices[input.vBones.z];
    finalMatrix += input.vWeights.w * g_matrices[input.vBones.w];    
	
	float4 vAnimatedPos = mul(float4(input.vPos,1),finalMatrix);
  
	o.vPos = mul(vAnimatedPos, g_mWorld);
	 o.vPos = mul(o.vPos, g_mViewProj);
  // determine split index from instance ID
	o.iSplit = g_iFirstSplit + input.iInstance;
  // transform with split specific projection matrix
	o.vPos = mul( o.vPos, g_mCropMatrix[o.iSplit] );
  return o;
}

[Geometry shader]


[maxvertexcount(3)]
void main( triangle VS_OUTPUT_SHADOWMAP_INSTANCING In[3], inout TriangleStream<GS_OUTPUT_SHADOWMAP> TriStream )
{
  GS_OUTPUT_SHADOWMAP Out;
  // set render target index
  Out.RTIndex = In[0].iSplit;
  // pass vertices through
  Out.vPos = In[0].vPos;
  TriStream.Append(Out);
  Out.vPos = In[1].vPos;
  TriStream.Append(Out);
  Out.vPos = In[2].vPos;
  TriStream.Append(Out);
  TriStream.RestartStrip();
}