#pragma once 

// maximum depth of surface layer tree!
#define MAX_LAYER_DEPTH		4

class ProcTexTerrainRender {

public:
	int		render_Init();
	int		render_Quit();

	void	CreateShaders();
	int generateBiCubicWeights( ID3D11Device* pD3DDevice );
	void Reset();
	void Render(float fTime, float fDeltaTime);
	void computeTextures( ID3D11Device* pD3DDevice );
	void addTextureLayer( ID3D11Device* pD3DDevice, ATLAS *atlas, PROCTEX_SURFACEPARAM &surface, ID3D11ShaderResourceView* srcColor, ID3D11ShaderResourceView* srcHeight, ID3D11ShaderResourceView* srcCoverage, ID3D11RenderTargetView* dstColorRTV, ID3D11RenderTargetView* dstHeightCoverageRTV);
	void addTextureLayerRoot( ID3D11Device* pD3DDevice, ATLAS *atlas, PROCTEX_SURFACEPARAM &surface, ID3D11RenderTargetView* dstColorRTV, ID3D11RenderTargetView* dstHeightCoverageRTV);
	void computeTexturesRecursive( ID3D11Device* pD3DDevice, ATLAS *atlas, PROCTEX_SURFACEPARAM &surface, int depth = 0, RenderTarget2D *currentCoverage = NULL, int additionalFlags = 0);
	void CreateRenderTargets();
	void computeLightingLayer( ID3D11Device* pD3DDevice, ATLAS *atlas, ID3D11ShaderResourceView *srcColor, ID3D11ShaderResourceView *srcHeight, ID3D11RenderTargetView *dstColor );
	int		FlushTextures;
	WORD				*HeightMap16;
	WORD				*HeightMap16Full;
	D3D11Texture2D		*HeightMapTex16;
	ID3D11ShaderResourceView*	HeightMapTex16SRV;

	D3D11Texture2D		*NoiseTex;
	ID3D11ShaderResourceView*	NoiseTexSRV;

	D3D11Texture2D		*DisplacementTex;
	ID3D11ShaderResourceView*	DisplacementTexSRV;

	D3D11Texture2D		*BicubicWeight03, *BicubicWeight47, *BicubicWeight8B, *BicubicWeightCF;
	ID3D11ShaderResourceView		*BicubicWeight03SRV, *BicubicWeight37SRV, *BicubicWeight88SRV, *BicubicWeightCFSRV;

	D3D11Texture2D		*ShadowTex;
	ID3D11ShaderResourceView		*ShadowSRV;

	D3D11Texture2D		*OcculsionTex;
	ID3D11ShaderResourceView		*OcculsionSRV;

	RenderTarget2D*	ColorRT;
	RenderTarget2D*	Color2RT;
	RenderTarget2D*	NormalRT;
	RenderTarget2D*	HeightCoverageRT;
	RenderTarget2D* HeightCoverageLayerRT[ MAX_LAYER_DEPTH ];
	

	ID3D11BufferPtr QuadVB;
	ID3D11BufferPtr QuadIB;
	ID3D11VertexShaderPtr QuadVS;
	ID3D11InputLayoutPtr QuadInputLayout;

	ID3DX11Effect* TerrainEffect;
};


