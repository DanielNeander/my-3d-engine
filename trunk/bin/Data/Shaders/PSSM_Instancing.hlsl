// PSSM
static const int g_iNumSplits = 4;
int g_iFirstSplit;
int g_iLastSplit;
matrix g_mWorld;
float4x4 g_mViewProj;
matrix g_mCropMatrix[g_iNumSplits];

/////////////////////////////////////////////////////////////////////////////////
//
// Instacing method


struct VS_INPUT_SHADOWMAP_INSTANCING
{
  float4 vPos : POSITION;
  uint iInstance : SV_InstanceID;
};

struct VS_OUTPUT_SHADOWMAP_INSTANCING
{
  float4 vPos : POSITION;
  uint iSplit : TEXTURE0;
};

[Vertex shader]

VS_OUTPUT_SHADOWMAP_INSTANCING main( VS_INPUT_SHADOWMAP_INSTANCING i )
{
  VS_OUTPUT_SHADOWMAP_INSTANCING o = (VS_OUTPUT_SHADOWMAP_INSTANCING)0;
  o.vPos = mul(i.vPos, g_mWorld);
  o.vPos = mul(o.vPos, g_mViewProj);
  // determine split index from instance ID
  o.iSplit = g_iFirstSplit + i.iInstance;
  // transform with split specific projection matrix
  o.vPos = mul( o.vPos, g_mCropMatrix[o.iSplit] );
  return o;
}

[Geometry shader]

struct GS_OUTPUT_SHADOWMAP
{
  float4 vPos : SV_POSITION;
  uint RTIndex : SV_RenderTargetArrayIndex;
};


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