//=================================================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

//=================================================================================================
// Resources
//=================================================================================================

// Inputs
Texture2DArray<float4> InputTexture : register(t0);

// Outputs
RWTexture2DArray<float4> OutputTexture : register(u0);

//=================================================================================================
// Entry point
//=================================================================================================
[numthreads(TGSize_, 1, 1)]
void DecodeTextureCS(uint3 GroupID : SV_GroupID, uint3 GroupThreadID : SV_GroupThreadID)
{
	const uint3 texelIdx = uint3(GroupThreadID.x + GroupID.x * TGSize_, GroupID.y, GroupID.z);	
	OutputTexture[texelIdx] = InputTexture[texelIdx];
}