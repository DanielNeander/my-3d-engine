#include "stdafx.h"
#include "EngineCore/Util/GameLog.h"
#include "Dx11Util.h"
#include "GameApp.h"
#include <GameApp/Util/EffectUtil.h>

#include "Terrain/SkyBox11.h"
#include "Terrain/Stars.h"
#include "Terrain/TileRing.h"


bool g_CheckForCracks = true;		// Set to true for more obvious cracks when debugging.

CSkybox11 g_Skybox;
bool g_ResetTerrain = false;

const int N_QUERIES = 5;
ID3D11Device* g_pD3D11Device = NULL;
ID3D11Query* g_PipelineQueries[N_QUERIES];
ID3D11Query* g_FreePipelineQueries[N_QUERIES];
UINT64 g_PrimitivesRendered = 0;

ID3DX11Effect* g_pTerrainEffect = NULL;
ID3DX11Effect* g_pDeformEffect = NULL;
ID3D11Buffer* g_TileTriStripIB = NULL;
ID3D11Buffer* g_TileQuadListIB = NULL;

ID3DX11EffectVectorVariable* g_pEyePosVar = NULL;
ID3DX11EffectVectorVariable* g_pViewDirVar = NULL;
ID3DX11EffectVectorVariable* g_pFractalOctavesTVar = NULL;
ID3DX11EffectVectorVariable* g_pFractalOctavesDVar = NULL;
ID3DX11EffectVectorVariable* g_pUVOffsetTVar = NULL;
ID3DX11EffectVectorVariable* g_pUVOffsetDVar = NULL;
ID3DX11EffectScalarVariable* g_pTriSizeVar = NULL;
ID3DX11EffectScalarVariable* g_pTileSizeVar = NULL;
ID3DX11EffectScalarVariable* g_DebugShowPatchesVar = NULL;
ID3DX11EffectScalarVariable* g_DebugShowTilesVar = NULL;
ID3DX11EffectVectorVariable* g_pViewportVar = NULL;
ID3DX11EffectScalarVariable* g_WireAlphaVar = NULL;
ID3DX11EffectScalarVariable* g_WireWidthVar = NULL;
ID3DX11EffectScalarVariable* g_DetailNoiseVar = NULL;
ID3DX11EffectVectorVariable* g_DetailUVVar = NULL;
ID3DX11EffectScalarVariable* g_SampleSpacingVar = NULL;

ID3DX11EffectShaderResourceVariable* g_HeightMapVar = NULL;
ID3DX11EffectShaderResourceVariable* g_GradientMapVar = NULL;
ID3DX11EffectShaderResourceVariable* g_InputTexVar = NULL;
D3D11_VIEWPORT g_BackBufferVP;

ID3DX11EffectTechnique* g_pTesselationTechnique = NULL;
ID3DX11EffectTechnique* g_pInitializationTechnique = NULL;
ID3DX11EffectTechnique* g_pGradientTechnique = NULL;

ID3D11ShaderResourceView* g_pHeightMapSRV = NULL;
ID3D11RenderTargetView*   g_pHeightMapRTV = NULL;
ID3D11ShaderResourceView* g_pGradientMapSRV = NULL;
ID3D11RenderTargetView*   g_pGradientMapRTV = NULL;


const int MAX_OCTAVES = 15;
const int g_DefaultRidgeOctaves = 3;			// This many ridge octaves is good for the moon - rather jagged.
const int g_DefaultfBmOctaves = 3;
const int g_DefaultTexTwistOctaves = 1;
const int g_DefaultDetailNoiseScale = 20;
int g_RidgeOctaves     = g_DefaultRidgeOctaves;
int g_fBmOctaves       = g_DefaultfBmOctaves;
int g_TexTwistOctaves  = g_DefaultTexTwistOctaves;
int g_DetailNoiseScale = g_DefaultDetailNoiseScale;

int g_PatchCount = 0;
bool g_HwTessellation = true;
int g_tessellatedTriWidth = 6;	// pixels on a triangle edge

const float CLIP_NEAR = 1, CLIP_FAR = 20000;
D3DXVECTOR2 g_ScreenSize(1920,1200);

const int COARSE_HEIGHT_MAP_SIZE = 1024;
const float WORLD_SCALE = 400;
const int VTX_PER_TILE_EDGE = 9;				// overlap => -2
const int TRI_STRIP_INDEX_COUNT = (VTX_PER_TILE_EDGE-1) * (2 * VTX_PER_TILE_EDGE + 2);
const int QUAD_LIST_INDEX_COUNT = (VTX_PER_TILE_EDGE-1) * (VTX_PER_TILE_EDGE-1) * 4;
const int MAX_RINGS = 10;
int g_nRings = 0;

TileRing* g_pTileRings[MAX_RINGS];
float SNAP_GRID_SIZE = 0;


HRESULT LoadSkyboxFromFile(LPCSTR file, ID3D11Device* pd3dDevice)
{

	HRESULT hr;
		
	D3DX11_IMAGE_INFO SrcInfo;
	hr = D3DX11GetImageInfoFromFile(file, NULL, &SrcInfo, NULL);

	D3DX11_IMAGE_LOAD_INFO texLoadInfo;
	texLoadInfo.Width          = SrcInfo.Width;
	texLoadInfo.Height         = SrcInfo.Height;
	texLoadInfo.Depth          = SrcInfo.Depth;
	texLoadInfo.FirstMipLevel  = 0;
	texLoadInfo.MipLevels      = SrcInfo.MipLevels;
	texLoadInfo.Usage          = D3D11_USAGE_DEFAULT;
	texLoadInfo.BindFlags      = D3D11_BIND_SHADER_RESOURCE;
	texLoadInfo.CpuAccessFlags = 0;
	texLoadInfo.MiscFlags      = SrcInfo.MiscFlags;
	texLoadInfo.Format         = SrcInfo.Format;
	texLoadInfo.Filter         = D3DX11_FILTER_TRIANGLE;
	texLoadInfo.MipFilter      = D3DX11_FILTER_TRIANGLE;
	texLoadInfo.pSrcInfo       = &SrcInfo;
	ID3D11Resource *pRes = NULL;

	D3DX11CreateTextureFromFile(pd3dDevice, file, &texLoadInfo, NULL, &pRes, NULL);
	if (pRes)
	{
		ID3D11Texture2D* texture;

		pRes->QueryInterface(__uuidof(ID3D11Texture2D), (LPVOID*)&texture);
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
		ZeroMemory(&SRVDesc, sizeof(SRVDesc));
		SRVDesc.Format = texLoadInfo.Format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = texLoadInfo.MipLevels;

		ID3D11ShaderResourceView* textureRview;
		pd3dDevice->CreateShaderResourceView(texture, &SRVDesc, &textureRview);

		g_Skybox.OnD3D11CreateDevice(pd3dDevice, 1, texture, textureRview);

		// Sky box class holds references.
		//SAFE_RELEASE(texture);
		//SAFE_RELEASE(textureRview);
	}

	SAFE_RELEASE(pRes);
	return S_OK;
}

static void CreateTileTriangleIB(ID3D11Device* pd3dDevice)
{
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));

	int index = 0;
	unsigned long* pIndices = new unsigned long[TRI_STRIP_INDEX_COUNT];

	// Using the same patch-corner vertices as for h/w tessellaiton, tessellate them into 2 tris each.
	// Create the usual zig-zag pattern of stripped triangles for a regular gridded terrain tile.
	for (int y = 0; y < VTX_PER_TILE_EDGE-1; ++y)
	{
		const int rowStart = y * VTX_PER_TILE_EDGE;

		for (int x = 0; x < VTX_PER_TILE_EDGE; ++x)
		{
			pIndices[index++] = rowStart + x;
			pIndices[index++] = rowStart + x + VTX_PER_TILE_EDGE;
		}

		// Repeat the last one on this row and the first on the next to join strips with degenerates.
		pIndices[index] = pIndices[index-1];
		++index;
		pIndices[index++] = rowStart + VTX_PER_TILE_EDGE;
	}
	assert(index == TRI_STRIP_INDEX_COUNT);

	initData.pSysMem = pIndices;
	D3D11_BUFFER_DESC iBufferDesc =
	{
		sizeof(unsigned long) * TRI_STRIP_INDEX_COUNT,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_INDEX_BUFFER,
		0,
		0
	};

	HRESULT hr;
	V(pd3dDevice->CreateBuffer(&iBufferDesc, &initData, &g_TileTriStripIB));
	delete[] pIndices;
}

static void CreateTileQuadListIB(ID3D11Device* pd3dDevice)
{
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));

	int index = 0;
	unsigned long* pIndices = new unsigned long[QUAD_LIST_INDEX_COUNT];

	// The IB describes one tile of NxN patches.
	// Four vertices per quad, with VTX_PER_TILE_EDGE-1 quads per tile edge.
	for (int y = 0; y < VTX_PER_TILE_EDGE-1; ++y)
	{
		const int rowStart = y * VTX_PER_TILE_EDGE;

		for (int x = 0; x < VTX_PER_TILE_EDGE-1; ++x)
		{
			pIndices[index++] = rowStart + x;
			pIndices[index++] = rowStart + x + VTX_PER_TILE_EDGE;
			pIndices[index++] = rowStart + x + VTX_PER_TILE_EDGE + 1;
			pIndices[index++] = rowStart + x + 1;
		}
	}
	assert(index == QUAD_LIST_INDEX_COUNT);

	initData.pSysMem = pIndices;
	D3D11_BUFFER_DESC iBufferDesc =
	{
		sizeof(unsigned long) * QUAD_LIST_INDEX_COUNT,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_INDEX_BUFFER,
		0,
		0
	};

	HRESULT hr;
	V(pd3dDevice->CreateBuffer(&iBufferDesc, &initData, &g_TileQuadListIB));
	delete[] pIndices;
}

static void CreateQueries(ID3D11Device* pd3dDevice)
{
	D3D11_QUERY_DESC desc;
	desc.MiscFlags = 0;
	desc.Query = D3D11_QUERY_PIPELINE_STATISTICS;

	for (int i=0; i!=N_QUERIES; ++i)
	{
		g_PipelineQueries[i] = NULL;
		pd3dDevice->CreateQuery(&desc, &g_PipelineQueries[i]);
		g_FreePipelineQueries[i] = g_PipelineQueries[i];			// All start on free list.
	}
}


static HRESULT CreateShaders(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext)
{
	HRESULT hr;

	CHAR path[MAX_PATH] = "Data/Shaders/TerrainTessellation/TerrainTessellation.fx";
	ID3DXBuffer* pShader = NULL;	

	const DWORD flags = 0;
	ID3DXBuffer* pErrors = NULL;
	/*if (D3DXCompileShaderFromFile(path, NULL, NULL, NULL, "fx_5_0", flags, &pShader, &pErrors, NULL) != S_OK)
	{
		const char* pTxt = (char *)pErrors->GetBufferPointer();
		MessageBoxA(NULL, pTxt, "Compilation errors", MB_OK);
		return hr;
	}

	if (D3DX11CreateEffectFromMemory(pShader->GetBufferPointer(), pShader->GetBufferSize(), 0, pd3dDevice, &g_pTerrainEffect) != S_OK)
	{
		MessageBoxA(NULL, "Failed to create terrain effect", "Effect load error", MB_OK);
		return hr;
	}*/
	if (FAILED(LoadEffectFromFile(pd3dDevice, path, &g_pTerrainEffect)))
	{
		MessageBoxA(NULL, "Failed to create terrain effect", "Effect load error", MB_OK);
		return hr;
	}
	// Obtain techniques
	g_pTesselationTechnique = g_pTerrainEffect->GetTechniqueByName("TesselationTechnique");

	// Obtain miscellaneous variables
	g_HeightMapVar = g_pTerrainEffect->GetVariableByName("g_CoarseHeightMap")->AsShaderResource();
	g_GradientMapVar = g_pTerrainEffect->GetVariableByName("g_CoarseGradientMap")->AsShaderResource();

	g_pEyePosVar  = g_pTerrainEffect->GetVariableByName("g_EyePos")->AsVector();
	g_pViewDirVar = g_pTerrainEffect->GetVariableByName("g_ViewDir")->AsVector();
	g_pFractalOctavesTVar = g_pTerrainEffect->GetVariableByName("g_FractalOctaves")->AsVector();
	g_pUVOffsetTVar = g_pTerrainEffect->GetVariableByName("g_TextureWorldOffset")->AsVector();
	g_pViewportVar = g_pTerrainEffect->GetVariableByName( "Viewport" )->AsVector();
	g_pTriSizeVar = g_pTerrainEffect->GetVariableByName("g_tessellatedTriWidth")->AsScalar();
	g_pTileSizeVar = g_pTerrainEffect->GetVariableByName("g_tileSize")->AsScalar();
	g_DebugShowPatchesVar = g_pTerrainEffect->GetVariableByName("g_DebugShowPatches")->AsScalar();
	g_DebugShowTilesVar = g_pTerrainEffect->GetVariableByName("g_DebugShowTiles")->AsScalar();
	g_WireAlphaVar = g_pTerrainEffect->GetVariableByName("g_WireAlpha")->AsScalar();
	g_WireWidthVar = g_pTerrainEffect->GetVariableByName("g_WireWidth")->AsScalar();
	g_DetailNoiseVar = g_pTerrainEffect->GetVariableByName("g_DetailNoiseScale")->AsScalar();
	g_DetailUVVar = g_pTerrainEffect->GetVariableByName("g_DetailUVScale")->AsVector();
	g_SampleSpacingVar = g_pTerrainEffect->GetVariableByName("g_CoarseSampleSpacing")->AsScalar();

	loadTextureFromFile("Data/TerrainTessellation/LunarSurface1.dds",     "g_TerrainColourTexture1", pd3dDevice, g_pTerrainEffect);
	loadTextureFromFile("Data/TerrainTessellation/LunarMicroDetail1.dds", "g_TerrainColourTexture2", pd3dDevice, g_pTerrainEffect);
	loadTextureFromFile("Data/TerrainTessellation/GaussianNoise256.jpg", "g_NoiseTexture",   pd3dDevice, g_pTerrainEffect);
	loadTextureFromFile("Data/TerrainTessellation/fBm5Octaves.dds",      "g_DetailNoiseTexture",     pd3dDevice, g_pTerrainEffect);
	loadTextureFromFile("Data/TerrainTessellation/fBm5OctavesGrad.dds",  "g_DetailNoiseGradTexture", pd3dDevice, g_pTerrainEffect);

	return S_OK;

}

static HRESULT CreateAmplifiedHeights(ID3D11Device* pd3dDevice)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width              = COARSE_HEIGHT_MAP_SIZE;
	desc.Height             = COARSE_HEIGHT_MAP_SIZE;
	desc.MipLevels          = 1;
	desc.ArraySize          = 1;
	desc.Format             = DXGI_FORMAT_R32_FLOAT;
	desc.SampleDesc.Count   = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage              = D3D11_USAGE_DEFAULT;
	desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags     = 0;
	desc.MiscFlags          = 0;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory(&SRVDesc, sizeof(SRVDesc));
	SRVDesc.Format                    = desc.Format;
	SRVDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels       = 1;
	SRVDesc.Texture2D.MostDetailedMip = 0;

	D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
	RTVDesc.Format             = desc.Format;
	RTVDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
	RTVDesc.Texture2D.MipSlice = 0;

	// No initial data here - it's initialized by deformation.
	ID3D11Texture2D* pTex = NULL;
	HRESULT hr = pd3dDevice->CreateTexture2D(&desc, NULL, &pTex);

	if (SUCCEEDED(hr))
	{
		HRESULT hr = S_OK;
		pd3dDevice->CreateShaderResourceView(pTex, &SRVDesc, &g_pHeightMapSRV);
		pd3dDevice->CreateRenderTargetView(pTex,   &RTVDesc, &g_pHeightMapRTV);

		SAFE_RELEASE(pTex);
	}

	desc.Format = DXGI_FORMAT_R16G16_FLOAT;

	// No initial data here - it's initialized by deformation.
	hr = pd3dDevice->CreateTexture2D(&desc, NULL, &pTex);

	if (SUCCEEDED(hr))
	{
		HRESULT hr = S_OK;
		SRVDesc.Format = RTVDesc.Format = desc.Format;
		pd3dDevice->CreateShaderResourceView(pTex, &SRVDesc, &g_pGradientMapSRV);
		pd3dDevice->CreateRenderTargetView(pTex,   &RTVDesc, &g_pGradientMapRTV);

		SAFE_RELEASE(pTex);
	}

	return hr;
}

static HRESULT CreateDeformEffect(ID3D11Device* pd3dDevice)
{
	HRESULT hr;

	CHAR path[MAX_PATH] = "Data/Shaders/TerrainTessellation/Deformation.fx";
	ID3DXBuffer* pShader = NULL;
	//if (FAILED (hr = DXUTFindDXSDKMediaFileCch(path, MAX_PATH, L"Deformation.fx"))) {
	//	MessageBox (NULL, L"Deformation.fx cannot be found.", L"Error", MB_OK);
	//	return hr;
	//}

	ID3DXBuffer* pErrors = NULL;
	////if (D3DXCompileShaderFromFile(path, NULL, NULL, NULL, "fx_5_0", 0, &pShader, &pErrors, NULL) != S_OK)	
	//{
	//	MessageBoxA(NULL, (char *)pErrors->GetBufferPointer(), "Compilation errors", MB_OK);
	//	return hr;
	//}

	//if (D3DX11CreateEffectFromMemory(pShader->GetBufferPointer(), pShader->GetBufferSize(), 0, pd3dDevice, &g_pDeformEffect) != S_OK)
	//{
	//	MessageBoxA(NULL, "Failed to create deformation effect", "Effect load error", MB_OK);
	//	return hr;
	//}
	LoadEffectFromFile(pd3dDevice, path, &g_pDeformEffect);

	// Obtain techniques
	g_pInitializationTechnique = g_pDeformEffect->GetTechniqueByName("InitializationTechnique");
	g_pGradientTechnique       = g_pDeformEffect->GetTechniqueByName("GradientTechnique");

	// Obtain miscellaneous variables
	g_pFractalOctavesDVar = g_pDeformEffect->GetVariableByName("g_FractalOctaves")->AsVector();
	g_pUVOffsetDVar = g_pDeformEffect->GetVariableByName("g_TextureWorldOffset")->AsVector();
	g_InputTexVar = g_pDeformEffect->GetVariableByName("g_InputTexture")->AsShaderResource();

	loadTextureFromFile("Data/TerrainTessellation/GaussianNoise256.jpg", "g_NoiseTexture", pd3dDevice, g_pDeformEffect);

	return S_OK;
}

static void CreateMeshes(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext)
{
	// This is a whole fraction of the max tessellation, i.e., 64/N.  The intent is that 
	// the height field scrolls through the terrain mesh in multiples of the polygon spacing.
	// So polygon vertices mostly stay fixed relative to the displacement map and this reduces
	// scintillation.  Without snapping, it scintillates badly.  Additionally, we make the
	// snap size equal to one patch width, purely to stop the patches dancing around like crazy.
	// The non-debug rendering works fine either way, but crazy flickering of the debug patches 
	// makes understanding much harder.
	const int PATCHES_PER_TILE_EDGE = VTX_PER_TILE_EDGE-1;
	SNAP_GRID_SIZE = WORLD_SCALE * g_pTileRings[g_nRings-1]->tileSize() / PATCHES_PER_TILE_EDGE;

	TileRing::CreateInputLayout(pd3dDevice, g_pTesselationTechnique);
	CreateTileTriangleIB(pd3dDevice);
	CreateTileQuadListIB(pd3dDevice);
	//CreateQueries(pd3dDevice);



	LoadSkyboxFromFile("Data/TerrainTessellation/MilkyWayCube.dds", pd3dDevice);
	ReleaseStars();
	CreateStars(pd3dDevice);

}


void App::LoadTessellatedTerrain()
{
	//D3DXVECTOR3 vEye(786.1f,  -86.5f,  62.2f);
	//D3DXVECTOR3  vAt(786.3f, -130.0f, 244.1f);
	D3DXVECTOR3 vEye(18595.1f,  200.4f,  -16347.6f);
	D3DXVECTOR3  vAt(18596.1f, 200.315f, -16348.f);

	ActiveCam_->setFrom((float*)vEye);
	ActiveCam_->setTo((float*)vAt);
	ActiveCam_->setFOV(RAD2DEG(noMath::PI / 3));
	ActiveCam_->setNear(CLIP_NEAR);
	ActiveCam_->setFar(CLIP_FAR);
	ActiveCam_->SetAspect(GetAspectRatio());
	ActiveCam_->computeModelView();	
	ActiveCam_->ComputeProjection();
	
	g_ResetTerrain = true;
	ReadStars();
	
	CreateAmplifiedHeights(device);
	
	CreateShaders(device, context);
	CreateDeformEffect(device);

	// This array defines the outer width of each successive ring.
	int widths[] = { 0, 16, 16, 16, 16 };
	g_nRings = sizeof(widths) / sizeof(widths[0]) - 1;		// widths[0] doesn't define a ring hence -1
	assert(g_nRings <= MAX_RINGS);

	float tileWidth = 0.125f;
	for (int i=0; i!=g_nRings && i!=MAX_RINGS; ++i)
	{
		g_pTileRings[i] = new TileRing(device, widths[i]/2, widths[i+1], tileWidth);
		tileWidth *= 2.0f;
	}
	CreateMeshes(device, context);

	OnSizeTerrain();
}

static void FullScreenPass(ID3D11DeviceContext* pContext, ID3DX11EffectVectorVariable* minVar, ID3DX11EffectVectorVariable* maxVar, ID3DX11EffectTechnique* pTech)
{
	// All of clip space:
	D3DXVECTOR3 areaMin(-1, -1, 0), areaMax(1, 1, 0);

	minVar->SetFloatVector(areaMin);
	maxVar->SetFloatVector(areaMax);
	pTech->GetPassByIndex(0)->Apply(0, pContext);
	pContext->Draw(4, 0);
}

static void InitializeHeights(ID3D11DeviceContext* pContext)
{
	ID3DX11EffectVectorVariable* deformMinVar = g_pDeformEffect->GetVariableByName("g_DeformMin")->AsVector();
	ID3DX11EffectVectorVariable* deformMaxVar = g_pDeformEffect->GetVariableByName("g_DeformMax")->AsVector();

	// This viewport is the wrong size for the texture.  But I've tweaked the noise frequencies etc to match.
	// So keep it like this for now.  TBD: tidy up.
	static const D3D11_VIEWPORT vp1 = { 0,0, (float) COARSE_HEIGHT_MAP_SIZE, (float) COARSE_HEIGHT_MAP_SIZE, 0.0f, 1.0f };
	pContext->RSSetViewports(1, &vp1);
	pContext->IASetInputLayout(NULL);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		
	pContext->OMSetRenderTargets(1, &g_pHeightMapRTV, NULL);

	FullScreenPass(pContext, deformMinVar, deformMaxVar, g_pInitializationTechnique);

	static const D3D11_VIEWPORT vp2 = { 0,0, (float) COARSE_HEIGHT_MAP_SIZE, (float) COARSE_HEIGHT_MAP_SIZE, 0.0f, 1.0f };
	pContext->RSSetViewports(1, &vp2);
	g_InputTexVar->SetResource(g_pHeightMapSRV);
	pContext->OMSetRenderTargets(1, &g_pGradientMapRTV, NULL);
	FullScreenPass(pContext, deformMinVar, deformMaxVar, g_pGradientTechnique);

	ID3D11RenderTargetView* pNULLRT = {NULL};
	pContext->OMSetRenderTargets(1, &(pNULLRT), NULL);
}

static void SetUVOffset(ID3DX11EffectVectorVariable* pVar)
{
	D3DXVECTOR3 eye;
	GetApp()->ActiveCam_->getFrom(eye);
	eye.y = 0;
	if (SNAP_GRID_SIZE > 0)
	{
		eye.x = floorf(eye.x / SNAP_GRID_SIZE) * SNAP_GRID_SIZE;
		eye.z = floorf(eye.z / SNAP_GRID_SIZE) * SNAP_GRID_SIZE;
	}
	eye /= WORLD_SCALE;
	eye.z *= -1;
	LOG_INFO << "UVOffset : "  << eye.x << " " << eye.y << " " << eye.z;
	pVar->SetFloatVector(eye);
}

void DeformInitTerrain(ID3D11DeviceContext* pContext)
{
	// Reset this so that it's not simultaneously resource and RT.
	g_HeightMapVar->SetResource(NULL);
	g_GradientMapVar->SetResource(NULL);
	g_pTesselationTechnique->GetPassByName("ShadedTriStrip")->Apply(0, pContext);

	int octaves[3] = { g_RidgeOctaves, g_fBmOctaves, g_TexTwistOctaves };
	g_pFractalOctavesDVar->SetIntVector(octaves);

	SetUVOffset(g_pUVOffsetDVar);
	InitializeHeights(pContext);
}



void SetViewport(ID3D11DeviceContext* pContext, const D3D11_VIEWPORT& vp)
{
	// Solid wireframe needs the vp in a GS constant.  Set both.
	pContext->RSSetViewports(1, &vp);

	float viewportf[4];
	viewportf[0] = (float) vp.Width;
	viewportf[1] = (float) vp.Height;
	viewportf[2] = (float) vp.TopLeftX;
	viewportf[3] = (float) vp.TopLeftY;
	g_pViewportVar->SetFloatVector(viewportf);
}

void SetMatrices(BaseCamera* pCam, const D3DXMATRIX& mProj)
{
	D3DXMATRIX mView;
	memcpy(&mView, pCam->getViewMatrix(), sizeof(float) * 16);

	// Set matrices
	D3DXMATRIX mWorld, mScale, mTrans;
	D3DXMatrixScaling(&mScale, WORLD_SCALE, WORLD_SCALE, WORLD_SCALE);

	// We keep the eye centered in the middle of the tile rings.  The height map scrolls in the horizontal 
	// plane instead.
	D3DXVECTOR3 eye;
	pCam->getFrom(&eye[0]);
	float snappedX = eye.x, snappedZ = eye.z;
	if (SNAP_GRID_SIZE > 0)
	{
		snappedX = floorf(snappedX / SNAP_GRID_SIZE) * SNAP_GRID_SIZE;
		snappedZ = floorf(snappedZ / SNAP_GRID_SIZE) * SNAP_GRID_SIZE;
	}
	const float dx = eye.x - snappedX;
	const float dz = eye.z - snappedZ;
	snappedX = eye.x - 2*dx;				// Why the 2x?  I'm confused.  But it works.
	snappedZ = eye.z - 2*dz;
	D3DXMatrixTranslation(&mTrans, snappedX, 0, snappedZ);
	D3DXMatrixMultiply(&mWorld, &mScale, &mTrans);

	ID3DX11EffectMatrixVariable* pmWorldViewProj    = g_pTerrainEffect->GetVariableByName("g_WorldViewProj")->AsMatrix();
	ID3DX11EffectMatrixVariable* pmProj             = g_pTerrainEffect->GetVariableByName("g_Proj")->AsMatrix();
	ID3DX11EffectMatrixVariable* pmWorldViewProjLOD = g_pTerrainEffect->GetVariableByName("g_WorldViewProjLOD")->AsMatrix();
	ID3DX11EffectMatrixVariable* pmWorldViewLOD     = g_pTerrainEffect->GetVariableByName("g_WorldViewLOD")->AsMatrix();
	assert(pmProj->IsValid());
	assert(pmWorldViewProj->IsValid());
	assert(pmWorldViewProjLOD->IsValid());
	assert(pmWorldViewLOD->IsValid());

	D3DXMATRIX mWorldView = mWorld * mView;
	D3DXMATRIX mWorldViewProj = mWorldView * mProj;
	pmWorldViewProj->SetMatrix((float*) &mWorldViewProj);
	pmProj->SetMatrix((float*) &mProj);

	// For LOD calculations, we always use the master camera's view matrix.
	D3DXMATRIX mWorldViewLOD = mWorld * mView;
	D3DXMATRIX mWorldViewProjLOD = mWorldViewLOD * mProj;
	pmWorldViewProjLOD->SetMatrix((float*) &mWorldViewProjLOD);
	pmWorldViewLOD->SetMatrix((float*) &mWorldViewLOD);

	// Due to the snapping tricks, the centre of projection moves by a small amount in the range ([0,2*dx],[0,2*dz])
	// relative to the terrain.  For frustum culling, we need this eye position.
	D3DXVECTOR3 cullingEye = eye;
	cullingEye.x -= snappedX;
	cullingEye.z -= snappedZ;
	g_pEyePosVar->SetFloatVector(cullingEye);
	g_pViewDirVar->SetFloatVector((float*)&mView._31);
}

void RenderTerrain(ID3D11DeviceContext* pContext, const D3DXMATRIX& mProj, const D3D11_VIEWPORT& vp, const char* passOverride=NULL)
{
	
	SetMatrices(GetApp()->ActiveCam_, mProj);

	g_HwTessellation = false;
	

	if (g_HwTessellation)
	{
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
		pContext->IASetIndexBuffer(g_TileQuadListIB, DXGI_FORMAT_R32_UINT, 0);
	}
	else
	{
		pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		pContext->IASetIndexBuffer(g_TileTriStripIB, DXGI_FORMAT_R32_UINT, 0);
	}

	const char* passName = "ShadedTriStrip";

	const bool wire = true;
	if (wire)
		passName = "Wireframe";
	if (g_HwTessellation)
	{
		if (wire)
			passName = "HwTessellatedWireframe";
		else
			passName = "HwTessellated";
	}
	if (passOverride != NULL)
		passName = passOverride;

	ID3DX11EffectPass* pPass = g_pTesselationTechnique->GetPassByName(passName);
	if (!pPass)
		return;		// Shouldn't happen unless the FX file is broken (like wrong pass name).

	SetViewport(pContext, vp);

	for (int i=0; i!=g_nRings ; ++i)
	{
		const TileRing* pRing = g_pTileRings[i];
		pRing->SetRenderingState(pContext);

		g_HeightMapVar->SetResource(g_pHeightMapSRV);
		g_GradientMapVar->SetResource(g_pGradientMapSRV);
		g_pTileSizeVar->SetFloat(pRing->tileSize());

		// Need to apply the pass after setting its vars.
		pPass->Apply(0, pContext);

		// Instancing is used: one tiles is one instance and the index buffer describes all the 
		// NxN patches within one tile.
		const int nIndices = (g_HwTessellation)? QUAD_LIST_INDEX_COUNT: TRI_STRIP_INDEX_COUNT;
		pContext->DrawIndexedInstanced(nIndices, pRing->nTiles(), 0, 0, 0);
	}
}

ID3D11Query* FindFreeQuery()
{
	for (int i=0; i!=N_QUERIES; ++i)
	{
		if (g_FreePipelineQueries[i])
		{
			ID3D11Query* pResult = g_FreePipelineQueries[i];
			g_FreePipelineQueries[i] = NULL;
			return pResult;
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------------------
// Render the scene using the D3D11 device
//--------------------------------------------------------------------------------------
void RenderToBackBuffer(Renderer* renderer, double fTime, float fElapsedTime, void* pUserContext)
{
	ID3D11DeviceContext* pContext =  GetApp()->GetContext();
	
	// Something's wrong in the shader and the tri size is out by a factor of 2.  Why?!?
	g_pTriSizeVar->SetInt(2 * g_tessellatedTriWidth);

	const bool debugDrawPatches = false;
	g_DebugShowPatchesVar->SetBool(debugDrawPatches);

	const bool debugDrawTiles = false;
	g_DebugShowTilesVar->SetBool(debugDrawTiles);

	const float wireAlpha = 0.01f * (float)80;

	// Below 1.0, we fade the lines out with blending; above 1, we increase line thickness.
	if (wireAlpha < 1)
	{
		g_WireAlphaVar->SetFloat(wireAlpha);
		g_WireWidthVar->SetFloat(1);
	}
	else
	{
		g_WireAlphaVar->SetFloat(1);
		g_WireWidthVar->SetFloat(wireAlpha);
	}

	g_DetailNoiseVar->SetFloat(0.001f * (float)g_DetailNoiseScale);

	float samplesapcing = WORLD_SCALE * g_pTileRings[g_nRings-1]->outerWidth() / (float) COARSE_HEIGHT_MAP_SIZE;
	g_SampleSpacingVar->SetFloat(samplesapcing);

	// If the settings dialog is being shown, then render it instead of rendering the app's scene		

	BaseCamera* pCam = GetApp()->ActiveCam_;
	D3DXMATRIX mProj;
	D3DXMATRIX mView;

	const unsigned int size16 = sizeof(float) * 16;		 
	memcpy(&mView, pCam->getViewMatrix(), size16);
	memcpy(&mProj, pCam->getProjectionMatrix(), size16);


	SetViewport(pContext, g_BackBufferVP);

	D3DXMATRIX mViewCopy = mView;
	mViewCopy._41 = mViewCopy._42 = mViewCopy._43 = 0;
	D3DXMATRIX mWVP = StarWorldMatrix() * mViewCopy * mProj;
	if (!g_CheckForCracks)
		g_Skybox.D3D11Render(&mWVP, pContext);

	RenderStars(pContext, mViewCopy, mProj, g_ScreenSize);

	int vec[3] = { g_RidgeOctaves, g_fBmOctaves, g_TexTwistOctaves };
	g_pFractalOctavesTVar->SetIntVector(vec);

	// I'm still trying to figure out if the detail scale can be derived from any combo of ridge + twist.
	// I don't think this works well (nor does ridge+twist+fBm).  By contrast the relationship with fBm is
	// straightforward.  The -4 is a fudge factor that accounts for the frequency of the coarsest ocatve
	// in the pre-rendered detail map.
	const float DETAIL_UV_SCALE = powf(2.0f, std::max(g_RidgeOctaves, g_TexTwistOctaves) + g_fBmOctaves - 4.0f);
	g_DetailUVVar->SetFloatVector(D3DXVECTOR2(DETAIL_UV_SCALE, 1.0f/DETAIL_UV_SCALE));

	SetUVOffset(g_pUVOffsetTVar);

	/*ID3D11Query* pFreeQuery = FindFreeQuery();
	if (pFreeQuery)
		pContext->Begin(pFreeQuery);*/
		
	RenderTerrain(pContext, mProj, g_BackBufferVP);

	//if (pFreeQuery)
	//	pContext->End(pFreeQuery);

	//for (int i=0; i!=N_QUERIES; ++i)
	//{
	//	if (!g_FreePipelineQueries[i] && g_PipelineQueries[i])	// in use & exists
	//	{
	//		D3D11_QUERY_DATA_PIPELINE_STATISTICS stats;
	//		if (S_OK == pContext->GetData(g_PipelineQueries[i], &stats, sizeof(stats), D3D11_ASYNC_GETDATA_DONOTFLUSH))
	//		{
	//			g_PrimitivesRendered = stats.CInvocations;
	//			g_FreePipelineQueries[i] = g_PipelineQueries[i];	// Put back on free list.
	//		}
	//	}
	//}		
}

void App::RenderTerrain2( bool bReset, float fTime, float fElapsedTime )
{
	g_ResetTerrain = bReset;

	if (g_ResetTerrain)
	{
		g_ResetTerrain = false;

		DeformInitTerrain(context);
	}

	renderer->changeToMainFramebuffer();	

	ID3D11DeviceContext* pContext =  GetApp()->GetContext();

	ID3D11RenderTargetView* pBackBufferRTV; 
	ID3D11DepthStencilView* pBackBufferDSV;

	pContext->OMGetRenderTargets(1, &pBackBufferRTV, &pBackBufferDSV);

	// Clear render target and the depth stencil 
	float ClearColor[4] = { 1.0f, 0.0f, 1.0f, 1.0f };					// Purple to better spot terrain cracks (disable sky cube).
	//float ClearColor[4] = { 0.465f, 0.725f, 0.0f, 1.0f };				// NV green for colour-consistent illustrations.
	pContext->ClearRenderTargetView(pBackBufferRTV, ClearColor);					
	pContext->ClearDepthStencilView(pBackBufferDSV, D3D11_CLEAR_DEPTH, 1.0, 0);	//  ¸®»çÀÌÂ¡½Ã Error!
	pContext->OMSetRenderTargets(1, &pBackBufferRTV, pBackBufferDSV);
	RenderToBackBuffer(renderer, fTime, fElapsedTime, NULL);

	
}

void App::ReleaseTerrain2()
{
	g_Skybox.OnD3D11ReleasingSwapChain();
}


void App::OnSizeTerrain()
{
	if (!device) return;
	if (!g_pTerrainEffect) return;

	HRESULT hr;
	ID3D11Texture2D* pBackBuffer;
	hr = swapChain->GetBuffer( 0, __uuidof( *pBackBuffer ), ( LPVOID* )&pBackBuffer );
	DXGI_SURFACE_DESC pBBufferSurfaceDesc;
	ZeroMemory( &pBBufferSurfaceDesc, sizeof( DXGI_SURFACE_DESC ) );
	if( SUCCEEDED( hr ) )
	{
		D3D11_TEXTURE2D_DESC TexDesc;
		pBackBuffer->GetDesc( &TexDesc );
		pBBufferSurfaceDesc.Width = ( UINT )TexDesc.Width;
		pBBufferSurfaceDesc.Height = ( UINT )TexDesc.Height;
		pBBufferSurfaceDesc.Format = TexDesc.Format;
		pBBufferSurfaceDesc.SampleDesc = TexDesc.SampleDesc;
		SAFE_RELEASE( pBackBuffer );
	}

	ID3DX11EffectVectorVariable* pScreenSize = g_pTerrainEffect->GetVariableByName("g_screenSize")->AsVector();
	if (pScreenSize)
	{
		D3DXVECTOR2 v((float) pBBufferSurfaceDesc.Width, (float) pBBufferSurfaceDesc.Height);		
		pScreenSize->SetFloatVector(v);
	}

	g_Skybox.OnD3D11ResizedSwapChain(&pBBufferSurfaceDesc);

	unsigned int n = 1;
	context->RSGetViewports(&n, &g_BackBufferVP);

	g_ScreenSize = D3DXVECTOR2((float) pBBufferSurfaceDesc.Width, (float) pBBufferSurfaceDesc.Height);
	float aspectRatio = g_ScreenSize.x / g_ScreenSize.y;

	ActiveCam_->setFOV(RAD2DEG(noMath::PI / 3));
	ActiveCam_->setNear(CLIP_NEAR);
	ActiveCam_->setFar(CLIP_FAR);
	ActiveCam_->SetAspect(aspectRatio);
	ActiveCam_->ComputeProjection();
	
}


void App::DestoryTerrainRes()
{
	g_Skybox.OnD3D11DestroyDevice();
	ReleaseStars();
	TileRing::ReleaseInputLayout();

	for (int i=0; i!=g_nRings; ++i)
	{
		delete g_pTileRings[i];
		g_pTileRings[i] = NULL;
	}

	SAFE_RELEASE(g_pTerrainEffect);
	SAFE_RELEASE(g_pDeformEffect);

	SAFE_RELEASE(g_pHeightMapSRV);
	SAFE_RELEASE(g_pHeightMapRTV);
	SAFE_RELEASE(g_pGradientMapSRV);
	SAFE_RELEASE(g_pGradientMapRTV);

	SAFE_RELEASE(g_TileTriStripIB);
	SAFE_RELEASE(g_TileQuadListIB);

	for (int i=0; i!=N_QUERIES; ++i)
		SAFE_RELEASE(g_PipelineQueries[i]);

}