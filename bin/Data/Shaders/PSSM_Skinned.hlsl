// PSSM
static const int g_iNumSplits = 4;
int 	 g_iFirstSplit;
int		 g_iLastSplit;
float4x4 g_mViewProj;
matrix 	 g_mCropMatrix[g_iNumSplits];

cbuffer animationvars
{
    matrix    g_matrices[MATRIX_PALETTE_SIZE_DEFAULT];
	float3      g_instanceColor = float3(1,1,1);
}

struct VS_INPUT_SHADOWMAP
{
  float4 vPos : POSITION;
  float4 vBones       : BONES;
  float4 vWeights     : WEIGHTS;
};

struct VS_OUTPUT_SHADOWMAP
{
  float4 vPos : SV_POSITION;
};

struct GS_OUTPUT_SHADOWMAP
{
  float4 vPos : SV_POSITION;
  uint RTIndex : SV_RenderTargetArrayIndex;
};

[Vertex shader]
/////////////////////////////////////////////////////////////////////////////////
//
// GS Cloning method

VS_OUTPUT_SHADOWMAP main( VS_INPUT_SHADOWMAP input )
{
	matrix finalMatrix;
    finalMatrix = input.vWeights.x * g_matrices[input.vBones.x];
    finalMatrix += input.vWeights.y * g_matrices[input.vBones.y];
    finalMatrix += input.vWeights.z * g_matrices[input.vBones.z];
    finalMatrix += input.vWeights.w * g_matrices[input.vBones.w];    
	
	float4 vAnimatedPos = mul(float4(input.vPos.xyz,1),finalMatrix);
	VS_OUTPUT_SHADOWMAP o = (VS_OUTPUT_SHADOWMAP)0;
	o.vPos = mul(vAnimatedPos, g_mWorld);
	o.vPos = mul(o.vPos, g_mViewProj);
	return o;
}

[Geometry shader]

[maxvertexcount(12)]
void main( triangle VS_OUTPUT_SHADOWMAP In[3], inout TriangleStream<GS_OUTPUT_SHADOWMAP> TriStream )
{
  // for each split
  for(int iSplit = g_iFirstSplit; iSplit <= g_iLastSplit; iSplit++)
  {
    GS_OUTPUT_SHADOWMAP Out;
    Out.RTIndex = iSplit;
    // for each vertex
    [unroll] for(int iVertex = 0; iVertex < 3; iVertex++)
    {
      // transform with split specific projection matrix
      Out.vPos = mul( In[iVertex].vPos, g_mCropMatrix[iSplit] );
      // append vertex to stream
      TriStream.Append(Out);
    }
    // mark end of triangle
    TriStream.RestartStrip();
  }
}
