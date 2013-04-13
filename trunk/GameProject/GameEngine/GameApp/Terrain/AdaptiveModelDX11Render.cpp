// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

#include "stdafx.h"
#include "AdaptiveModelDX11Render.h"
#include "GameApp/Util/EffectUtil.h"
#include "TerrainPatch.h"
#include "RQTTriangulation.h"
#include "GameApp/GameApp.h"
#include "GameApp/Texture/Clipmap.h"
#include "GameApp/GraphicsTypes.h"
#include "GameApp/Dx11Util.h"
#include "GameApp/LIDRenderer.h"
#include "GameApp/DemoSetup.h"
#include "GameApp/PSSMShadowMapDx11.h"
#include "SNBTerrainMaterial.h"

Clipmaps* gClipmap;
Clipmap* gClipMap2;
extern std::vector<mat4> mCropMatrix;
extern std::vector<mat4> mTextureMatrix;


CAdaptiveModelDX11Render::SRenderParams::SRenderParams() : 
    m_TexturingMode(TM_INDEX_MASK),

    m_bEnableHeightMapMorph(false),	// 버그 있음..
    m_bEnableNormalMapMorph(true),

    m_bAsyncModeWorkaround(true),
    m_bCompressNormalMap(true),
    m_iNormalMapLODBias(1)
{
}

CAdaptiveModelDX11Render::CAdaptiveModelDX11Render(void) : 
    m_bSortPatchesByDistance(true),
    m_bEnableAdaptTriang(true),
    m_iBackBufferWidth(1024),
    m_iBackBufferHeight(768),
	m_bInitialized(false)

{
}

CAdaptiveModelDX11Render::~CAdaptiveModelDX11Render(void)
{
	SAFE_DELETE(gClipmap);
	SAFE_DELETE(gClipMap2);
}

HRESULT CAdaptiveModelDX11Render::Init(const SRenderingParams &Params,
                                       const SRenderParams &RenderParams,
                                       CElevationDataSource *pDataSource,
                                       CTriangDataSource *pTriangDataSource)
{
    HRESULT hr;
    
    m_RenderParams =  RenderParams;

    hr = __super::Init(Params, pDataSource, pTriangDataSource);
    CHECK_HR_RET(hr, _T("CBlockBasedAdaptiveModel::Init() failed"));
    
    // Initialize common data for all patches
    m_pPatchCommon.reset( 
        new CDX11PatchesCommon(m_Params.m_fElevationSamplingInterval,
                               m_Params.m_fElevationScale,
                               m_Params.m_iNumLevelsInPatchHierarchy,
                               m_RenderParams.m_bAsyncModeWorkaround,
                               m_RenderParams.m_bCompressNormalMap) );

    // Set required extension for the data source
    m_pDataSource->SetRequiredElevDataBoundaryExtensions( CTerrainPatch::ELEVATION_DATA_BOUNDARY_EXTENSION,
                                                          CTerrainPatch::ELEVATION_DATA_BOUNDARY_EXTENSION,
                                                          CTerrainPatch::ELEVATION_DATA_BOUNDARY_EXTENSION,
                                                          CTerrainPatch::ELEVATION_DATA_BOUNDARY_EXTENSION );

    m_pDataSource->SetHighResDataLODBias( RenderParams.m_iNormalMapLODBias );

    // Height map data returned by the data source will have the following layout:
    //                    ELEVATION_DATA_BOUNDARY_EXTENSION
    //    _______________|_| _
    //   |   ____________   |_ ELEVATION_DATA_BOUNDARY_EXTENSION
    //   |  |            |  |
    //   |  |            |  |
    //   |  |            |  |
    //   |  |            |  |
    //   |  |____________|  |
    //   |__________________|
    //      |<---------->|
    //       m_iPatchSize
	m_bInitialized = true;

    return S_OK;
}

// The method hierarchically traverses the tree and updates all device resources
void CAdaptiveModelDX11Render::RecursiveUpdateDeviceResources(CPatchQuadTreeNode &PatchNode)
{
    if( PatchNode.GetPos().level > 0 )
    {
        // Update resource of the current patch
        HRESULT hr = PatchNode.GetData().pPatch->UpdateDeviceResources();
        if( FAILED(hr) )
            throw std::runtime_error("failed to update patch resources");
    }

    // Get children
    CPatchQuadTreeNode *pDescendantNode[4];
    PatchNode.GetDescendants(pDescendantNode[0], pDescendantNode[1], pDescendantNode[2], pDescendantNode[3]);

    // Process children
    for(int iChild=0; iChild<4; iChild++)
        if( pDescendantNode[iChild] )
            RecursiveUpdateDeviceResources(*pDescendantNode[iChild]);
}

// The method hierarchically traverses the tree and create all D3D device resources
void CAdaptiveModelDX11Render::RecursiveCreateD3D11PatchResources(CPatchQuadTreeNode &PatchNode)
{
    if( PatchNode.GetPos().level > 0 )
    {
        // Create resource for the current patch
        CreatePatchForNode(PatchNode, PatchNode.GetData().m_pElevData.get(), PatchNode.GetData().m_pAdaptiveTriangulation.get());
    }

    // Get children
    CPatchQuadTreeNode *pDescendantNode[4];
    PatchNode.GetDescendants(pDescendantNode[0], pDescendantNode[1], pDescendantNode[2], pDescendantNode[3]);

    // Process children
    for(int iChild=0; iChild<4; iChild++)
        if( pDescendantNode[iChild] )
            RecursiveCreateD3D11PatchResources(*pDescendantNode[iChild]);
}


// The method compiles patch rendering effect
HRESULT CAdaptiveModelDX11Render::CompileRenderPatchEffect(ID3D11Device* pd3dDevice)
{
    HRESULT hr;

    CComPtr<ID3DBlob> pEffectBuffer;

    // Fill up macro definitions depending on rendering parameters
    D3D_SHADER_MACRO DefinedMacroses[64];
    ZeroMemory(DefinedMacroses, sizeof(DefinedMacroses) );
    int iMacrosesDefinedCount = 0;
    
    DefinedMacroses[iMacrosesDefinedCount].Name = "ELEV_DATA_EXTENSION";
    char strElevDataExt[8];
    sprintf_s(strElevDataExt, sizeof(strElevDataExt)/sizeof(strElevDataExt[0]), "%d", CTerrainPatch::ELEVATION_DATA_BOUNDARY_EXTENSION);
    DefinedMacroses[iMacrosesDefinedCount].Definition = strElevDataExt;
    iMacrosesDefinedCount++;
    
    DefinedMacroses[iMacrosesDefinedCount].Name = "ENABLE_HEIGHT_MAP_MORPH";
    DefinedMacroses[iMacrosesDefinedCount].Definition = m_RenderParams.m_bEnableHeightMapMorph ? "1" : "0";
    iMacrosesDefinedCount++;

    DefinedMacroses[iMacrosesDefinedCount].Name = "ENABLE_NORMAL_MAP_MORPH";
    DefinedMacroses[iMacrosesDefinedCount].Definition = m_RenderParams.m_bEnableNormalMapMorph ? "1" : "0";
    iMacrosesDefinedCount++;
    
    char strTexturingMode[8];
    DefinedMacroses[iMacrosesDefinedCount].Name = "TEXTURING_MODE";
    sprintf_s(strTexturingMode, _countof(strTexturingMode), "%d", m_RenderParams.m_TexturingMode);
    DefinedMacroses[iMacrosesDefinedCount].Definition = strTexturingMode;
    iMacrosesDefinedCount++;

    char strPatchSize[8];
    sprintf_s(strPatchSize, sizeof(strPatchSize)/sizeof(strPatchSize[0]), "%d", m_Params.m_iPatchSize);
    DefinedMacroses[iMacrosesDefinedCount].Name = "PATCH_SIZE";
    DefinedMacroses[iMacrosesDefinedCount].Definition = strPatchSize;
    iMacrosesDefinedCount++;

    // If uncompressed normal map is used, then normal xy coordinates are stored in "xy" texture comonents.
    // If DXT5 (BC3) compression is used, then x coordiante is stored in "g" texture component and y coordinate is 
    // stored in "a" component
    DefinedMacroses[iMacrosesDefinedCount].Name = "NORMAL_MAP_COMPONENTS";
    DefinedMacroses[iMacrosesDefinedCount].Definition = m_RenderParams.m_bCompressNormalMap ? "ga" : "xy";
    iMacrosesDefinedCount++;

    DefinedMacroses[iMacrosesDefinedCount].Name = "MaxLights";
	char strMaxLights[8];
	sprintf_s(strMaxLights, _countof(strMaxLights), "%d",  NumLightsGUI::NumLights(0));
    DefinedMacroses[iMacrosesDefinedCount].Definition = strMaxLights;

    // Compile the effect
    hr = CompileShaderFromFile( "Data/Shaders/SNBTerrainPatch.fx", DefinedMacroses, NULL, "fx_5_0", &pEffectBuffer );
    CHECK_HR_RET(hr, _T("Failed to compile the patch effect from file"));

    m_pRenderEffect11.Release();
    hr = D3DX11CreateEffectFromMemory( pEffectBuffer->GetBufferPointer(),
                                        pEffectBuffer->GetBufferSize(),
                                        0,
                                        pd3dDevice,
                                        &m_pRenderEffect11 );
    CHECK_HR_RET(hr, _T("Failed to create the patch effect"));

    // Get effect variables
    GET_EFFECT_VAR( m_pRenderEffect11, "g_PatchXYScale",             SCALAR,          m_RenderEffectVars.m_pevPatchXYScale);
    GET_EFFECT_VAR( m_pRenderEffect11, "g_fFlangeWidth",             SCALAR,          m_RenderEffectVars.m_pevFlangeWidth);
    GET_EFFECT_VAR( m_pRenderEffect11, "g_fMorphCoeff",              SCALAR,          m_RenderEffectVars.m_pevMorphCoeff);
    GET_EFFECT_VAR( m_pRenderEffect11, "g_PatchLBCornerXY",          VECTOR,          m_RenderEffectVars.m_pevPatchLBCornerXY);
    GET_EFFECT_VAR( m_pRenderEffect11, "g_PatchOrderInSiblQuad",     VECTOR,          m_RenderEffectVars.m_pevPatchOrderInSiblQuad);
    GET_EFFECT_VAR( m_pRenderEffect11, "g_tex2DElevationMap",  SHADER_RESOURCE, m_RenderEffectVars.m_pevElevationMap);
    GET_EFFECT_VAR( m_pRenderEffect11, "g_tex2DNormalMap",        SHADER_RESOURCE, m_RenderEffectVars.m_pevNormalMap);
    
    GET_EFFECT_VAR( m_pRenderEffect11, "g_tex2DParentElevMap",  SHADER_RESOURCE, m_RenderEffectVars.m_pevParentElevMap);
    GET_EFFECT_VAR( m_pRenderEffect11, "g_tex2DParentNormalMap",        SHADER_RESOURCE, m_RenderEffectVars.m_pevParentNormalMap);

    GET_EFFECT_VAR( m_pRenderEffect11, "g_mWorldViewProj",           MATRIX,          m_RenderEffectVars.m_pevWorldViewProj);
    GET_EFFECT_VAR( m_pRenderEffect11, "RenderPatch_FeatureLevel10", TECHNIQUE,       m_RenderEffectVars.m_pevRenderPatch_FeatureLevel10Tech);
    GET_EFFECT_VAR( m_pRenderEffect11, "RS_SolidFill",               RASTERIZER,      m_RenderEffectVars.m_pevRS_SolidFill);
    GET_EFFECT_VAR( m_pRenderEffect11, "g_vDirectionOnSun",          VECTOR,          m_RenderEffectVars.m_pevDirOnSun);
    GET_EFFECT_VAR( m_pRenderEffect11, "g_vSunColorAndIntensityAtGround",    VECTOR,  m_RenderEffectVars.m_pevSunColor);
    GET_EFFECT_VAR( m_pRenderEffect11, "g_vAmbientLight",            VECTOR,          m_RenderEffectVars.m_pevAmbientLight);

    GET_EFFECT_VAR( m_pRenderEffect11, "RenderHeightMapPreview_FeatureLevel10", TECHNIQUE, m_RenderEffectVars.m_pevRenderHeightMapPreview_FL10);
    GET_EFFECT_VAR( m_pRenderEffect11, "g_vTerrainMapPos_PS", VECTOR, m_RenderEffectVars.m_pevTerrainMapPos_PS);
	
	   
    // Set global min/max elevation
    ID3DX11EffectVectorVariable *pGloabalMinMaxElevation;
    GET_EFFECT_VAR( m_pRenderEffect11, "g_GlobalMinMaxElevation",            VECTOR, pGloabalMinMaxElevation);
    D3DXVECTOR4 GlobalMinMaxElevation(m_Params.m_fGlobalMinElevation, m_Params.m_fGlobalMaxElevation, 0, 0);
    hr = pGloabalMinMaxElevation->SetFloatVector( &GlobalMinMaxElevation.x );
    CHECK_HR_RET(hr, _T("Failed to set global Min/Max elevation"));    

    // Assign the resource to the effect variable
    ID3DX11EffectShaderResourceVariable *pevElevationColor;
    GET_EFFECT_VAR( m_pRenderEffect11, "g_tex2DElevationColor", SHADER_RESOURCE, pevElevationColor);
    hr = pevElevationColor->SetResource( m_pElevationColorsSRV );

    GET_EFFECT_VAR( m_pRenderEffect11, "RenderBoundBox_FeatureLevel10", TECHNIQUE, m_RenderEffectVars.m_pRenderBoundBox_FeatureLevel10Tech);
    GET_EFFECT_VAR( m_pRenderEffect11, "RenderQuadTree_FeatureLevel10", TECHNIQUE, m_RenderEffectVars.m_pRenderQuadTree_FeatureLevel10Tech);
    GET_EFFECT_VAR( m_pRenderEffect11, "g_vScreenPixelSize", VECTOR, m_RenderEffectVars.m_pScreenPixelSize);
    GET_EFFECT_VAR( m_pRenderEffect11, "g_vQuadTreePreviewPos_PS", VECTOR, m_RenderEffectVars.m_pQuadTreePreviewPos_PS);

	m_RenderEffectVars.m_pLightConstBuffer = m_pRenderEffect11->GetConstantBufferByName("cbLighting")->AsConstantBuffer();

	ID3D11Buffer* buffer;
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.ByteWidth = static_cast<UINT>(sizeof(MeshPSConstants) + (16 - (sizeof(MeshPSConstants) % 16)));
	DXCall(D3D11Dev()->CreateBuffer(&desc, NULL, &buffer));
	m_RenderEffectVars.m_pLightConstBuffer->SetConstantBuffer(buffer);

#if 0
	gClipmap = new Clipmaps(D3D11Dev(), D3D11Context());
	gClipmap->SetTexurePrefix(L"Terrain/terrain_rock");
	gClipmap->Init();
	gClipmap->ClipmapEffect = m_pRenderEffect11;
	gClipmap->Process(2048);

	m_RenderEffectVars.m_pPyramidMap = m_pRenderEffect11->GetVariableByName( "PyramidTexture" )->AsShaderResource();
	m_RenderEffectVars.m_pPyramidHeightMap= m_pRenderEffect11->GetVariableByName( "PyramidTextureHM" )->AsShaderResource();
	m_RenderEffectVars.m_pStackMap = m_pRenderEffect11->GetVariableByName( "StackTexture" )->AsShaderResource();
#else	
	/*gClipMap2 = new Clipmap();
	gClipMap2->ClipmapEffect = m_pRenderEffect11;
	gClipMap2->SrcTexutreName = "Data/diffuseTexture.tga";
	gClipMap2->Init();
	gClipMap2->alloc(D3D11Dev());		*/
#endif

	// 베이스 맵
#if 1
	m_pPatchCommon->m_terrainCommonMat->SplattSrcMap[0].BindShaderMapName = "Data/diffuseTexture.tga";
	
	m_pPatchCommon->m_terrainCommonMat->SplattSrcMap[1].BindShaderMapName = "TerrainTextures/terrain_rock4.dds";
	m_pPatchCommon->m_terrainCommonMat->SplattSrcMap[1].BindShaderVarName = "g_RockDiffuseTexture";

	m_pPatchCommon->m_terrainCommonMat->SplattSrcMap[2].BindShaderMapName = "TerrainTextures/sand_diffuse.dds";
	m_pPatchCommon->m_terrainCommonMat->SplattSrcMap[2].BindShaderVarName = "g_SandDiffuseTexture";
	
	m_pPatchCommon->m_terrainCommonMat->SplattSrcMap[3].BindShaderMapName = "TerrainTextures/terrain_grass.dds";
	m_pPatchCommon->m_terrainCommonMat->SplattSrcMap[3].BindShaderVarName = "g_GrassDiffuseTexture";

	m_pPatchCommon->m_terrainCommonMat->SplattBumpMap[0].BindShaderMapName = "TerrainTextures/rock_bump6.dds";	
	m_pPatchCommon->m_terrainCommonMat->SplattBumpMap[0].BindShaderVarName = "g_RockBumpTexture";
	
	m_pPatchCommon->m_terrainCommonMat->SplattBumpMap[1].BindShaderMapName = "TerrainTextures/rock_bump4.dds";
	m_pPatchCommon->m_terrainCommonMat->SplattBumpMap[1].BindShaderVarName = "g_SandBumpTexture";
	
	m_pPatchCommon->m_terrainCommonMat->SplattBumpMap[2].BindShaderMapName = "TerrainTextures/lichen1_normal.dds";
	m_pPatchCommon->m_terrainCommonMat->SplattBumpMap[2].BindShaderVarName = "g_SandMicroBumpTexture";
	
	m_pPatchCommon->m_terrainCommonMat->SplattBumpMap[3].BindShaderMapName = "TerrainTextures/rock_bump4.dds";
	m_pPatchCommon->m_terrainCommonMat->SplattBumpMap[3].BindShaderVarName = "g_SandBumpTexture";
	
	m_pPatchCommon->m_terrainCommonMat->SlopeMapName.BindShaderMapName = "TerrainTextures/terrain_slope.dds";
	m_pPatchCommon->m_terrainCommonMat->SlopeMapName.BindShaderVarName = "g_SlopeDiffuseTexture";
	
	
	m_pPatchCommon->m_terrainCommonMat->Init(m_pRenderEffect11);
	m_pPatchCommon->m_terrainCommonMat->ReCreateBuffers();
	m_pPatchCommon->m_terrainCommonMat->CommitChanges();

#endif

	CreateTextures();

	//GetShadowMap<PSSMShadowMapDx11>()->CreateConstantBuffer(m_pRenderEffect11);

    return S_OK;
}


void CAdaptiveModelDX11Render::CreateTextures()
{
#if 0
	m_pTextureIndexMap = new TextureD3D11(D3D11Dev());
	m_pTextureArray = new TextureD3D11(D3D11Dev());

	// Load Slices on Texture Array
	vector<std::string> lstMaterials;
	DirectoryFilter dirFilter;

	string path = "textures/materials/";
	dirFilter.Initialize(path, "*.dds", true, false);

	if(dirFilter.Count() == 0)
	{
		path = "textures/terrain_textures/";
		dirFilter.Initialize(path, "*.png", true, false);
	}

	for(DirectoryFilter::Iterator it = dirFilter.Begin(); it != dirFilter.End(); it++)
	{
		lstMaterials.push_back(path + *it);
	}

	uint8 arrayCount = (uint8)lstMaterials.size();

	// Create Index Map
	D3D11_TEXTURE2D_DESC descTextureIndices;
	descTextureIndices.Width				= CElevationDataSource::m_iWidth;//m_pTextureHeightMap->GetDescriptor().Width;
	descTextureIndices.Height				= CElevationDataSource::m_iHeight;//m_pTextureHeightMap->GetDescriptor().Height;
	descTextureIndices.MipLevels			= 1;
	descTextureIndices.ArraySize			= 1;
	descTextureIndices.Format				= DXGI_FORMAT_R8_UNORM;
	descTextureIndices.SampleDesc.Count		= 1;
	descTextureIndices.SampleDesc.Quality	= 0;
	descTextureIndices.Usage				= D3D11_USAGE_IMMUTABLE;
	descTextureIndices.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	descTextureIndices.CPUAccessFlags		= 0;
	descTextureIndices.MiscFlags			= 0;

	int totalIndexCount = descTextureIndices.Width * descTextureIndices.Height;
	uint8* pIndexMapData = new uint8[totalIndexCount];
	memset(pIndexMapData, 0, descTextureIndices.Width * descTextureIndices.Height * sizeof(uint8));

	for(int i = 0; i < 10000; i++)
	{
		uint16 w = 2 + rand() % 4;
		uint16 h = 2 + rand() % 4;

		uint16 posX = rand() % (descTextureIndices.Width-w);
		uint16 posY = rand() % (descTextureIndices.Height-h);

		uint8 materialID = rand() % arrayCount;

		// Draw a Quad 2x2
		for(uint16 y = 0; y < w; y++)
		{
			for(uint16 x = 0; x < h; x++)
			{
				int index = ((posY + y) * descTextureIndices.Width) + posX + x;

				if(index < totalIndexCount)
					pIndexMapData[index] = materialID;
			}
		}
	}

	m_pTextureIndexMap = new TextureD3D11(D3D11Dev());
	//m_pTextureIndexMap->CreateFromRAW(descTextureIndices, pIndexMapData);
	SAFE_DELETE_ARRAY(pIndexMapData);
	
	CreateTextureFromFile(D3D11Dev(), D3D11Context(), "textures/index_map.jpg", &m_pTextureIndexMap->GetResourceView(), true);

	// Create Texture Array
	D3D11_TEXTURE2D_DESC descTextureArray;
	descTextureArray.Width				= 512;
	descTextureArray.Height				= 512;
	descTextureArray.MipLevels			= 8;
	descTextureArray.ArraySize			= arrayCount;
	descTextureArray.Format				= DXGI_FORMAT_R8G8B8A8_UNORM;
	descTextureArray.SampleDesc.Count	= 1;
	descTextureArray.SampleDesc.Quality	= 0;
	descTextureArray.Usage				= D3D11_USAGE_DEFAULT;
	descTextureArray.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	descTextureArray.CPUAccessFlags		= 0;
	descTextureArray.MiscFlags			= 0;

	m_pTextureArray = new TextureD3D11(D3D11Dev());
	m_pTextureArray->Create(descTextureArray);

	for(size_t i = 0; i < lstMaterials.size(); i++)
		m_pTextureArray->LoadTextureOnArray(D3D11Context(), i, lstMaterials[i]);
#endif
}

// This function adds strip of triangles as shown:
//
//                iVerticesAlongXAxis
//           |<------------------------>|
//           *---*---*---*          *---* ---
//           |.' |.' |.' |   ....   |.' |   A
//           *---*---*---*          *---*   |
//                                          |
//           ...........................    | 
//                                          | iVerticesAlongYAxis
//           *---*---*---*          *---*   |
//           |.' |.' |.' |   ....   |.' |   |
//           *---*---*---*          *---*   |
//           |.' |.' |.' |   ....   |.' |   V
//iStartYInd *---*---*---*          *---* ---
//     iStartXInd  
//
// It also can duplicate last and first vertes to stitch a number of strips in one
static
int AddStrip(DWORD* &pIndices, 
             int iNumLevesInQuadTree,
             int iVerticesAlongXAxis, 
             int iVerticesAlongYAxis, 
             int iStartXInd, 
             int iStartYInd, 
             bool bDuplicateFirstIndex, 
             bool bDuplicateLastIndex)
{
    int iNumIndicesInStrip = (iVerticesAlongXAxis*2 + 2) * (iVerticesAlongYAxis-1) - 2 + (bDuplicateFirstIndex ? 1 : 0) + (bDuplicateLastIndex ? 1 : 0);

    if(pIndices == NULL)
        return iNumIndicesInStrip;
    
    DWORD* pCurrIndex = pIndices;

    if(bDuplicateFirstIndex)
        (*pCurrIndex++) = CalculatePackedIndex(iStartXInd, iStartYInd+1, iNumLevesInQuadTree-1, iNumLevesInQuadTree, CTerrainPatch::ELEVATION_DATA_BOUNDARY_EXTENSION);
    
    for(int iY=iStartYInd; iY < iStartYInd + iVerticesAlongYAxis - 1; iY++)
    {
        for(int iX=iStartXInd; iX <= iStartXInd + iVerticesAlongXAxis - 1; iX++)
        {
            (*pCurrIndex++) = CalculatePackedIndex(iX, iY+1, iNumLevesInQuadTree-1, iNumLevesInQuadTree, CTerrainPatch::ELEVATION_DATA_BOUNDARY_EXTENSION);
            (*pCurrIndex++) = CalculatePackedIndex(iX, iY,   iNumLevesInQuadTree-1, iNumLevesInQuadTree, CTerrainPatch::ELEVATION_DATA_BOUNDARY_EXTENSION);
        }

        if(iY < iStartYInd + iVerticesAlongYAxis - 2 )
        {
            (*pCurrIndex++) = *(pCurrIndex-1);
            (*pCurrIndex++) = CalculatePackedIndex(iStartXInd, iY+2, iNumLevesInQuadTree-1, iNumLevesInQuadTree, CTerrainPatch::ELEVATION_DATA_BOUNDARY_EXTENSION);
        }
    }

    if(bDuplicateLastIndex)
        (*pCurrIndex++) = *(pCurrIndex-1);

    assert(pCurrIndex - pIndices == iNumIndicesInStrip);
    
    pIndices = pCurrIndex;

    return iNumIndicesInStrip;
}

// Method creates an index buffer for rendering patches using full resolution triangulation
HRESULT CAdaptiveModelDX11Render::CreateFullResolutionStripBuffer(int iPatchSize)
{
    HRESULT Result;

    // Number of columns in one substrip. 16 is a good value for
    // using post-transform vertex cache
    int iColumnsNumberInSubStrip = 16;
    int iNumSubStrips = (iPatchSize <= iColumnsNumberInSubStrip) ? 1 : (iPatchSize / iColumnsNumberInSubStrip);

    DWORD *pCurrIndex = NULL;
    std::vector<DWORD> IndicesBuffer;
    int iSubStripStartXInd = 0;
    for(int iPass = 0; iPass < 2; iPass++)
    {
        if( iPass == 0 )
            m_uiIndicesInFullResolutionStrip = 0;
        iSubStripStartXInd = -1;
        int iSubStripStartYInd = -1;
        int iVertAlongYAxisInStrip = iPatchSize + 3;
        for(int iSubStripNum = 0; iSubStripNum < iNumSubStrips; iSubStripNum++)
        {
            int iVertAlongXAxisInCurrSubStrip;
            if( iSubStripNum == 0 && iNumSubStrips > 1)
                iVertAlongXAxisInCurrSubStrip = iColumnsNumberInSubStrip + 2;
            else if(iSubStripNum == iNumSubStrips-1)
                iVertAlongXAxisInCurrSubStrip = iPatchSize+2 - iSubStripStartXInd;
            else
                iVertAlongXAxisInCurrSubStrip = iColumnsNumberInSubStrip + 1;

            DWORD uiIndicesInCurrSubStrip = 
                AddStrip(pCurrIndex, m_Params.m_iNumLevelsInPatchHierarchy, iVertAlongXAxisInCurrSubStrip, iVertAlongYAxisInStrip, iSubStripStartXInd, iSubStripStartYInd, (iSubStripNum > 0), (iSubStripNum < iNumSubStrips-1) );
            if( iPass == 0 )
                m_uiIndicesInFullResolutionStrip += uiIndicesInCurrSubStrip;
            
            iSubStripStartXInd += iVertAlongXAxisInCurrSubStrip-1;
        }
        if( iPass == 0 )
        {
            IndicesBuffer.resize( m_uiIndicesInFullResolutionStrip );
            pCurrIndex = &IndicesBuffer[0];
        }
    }

    assert( iSubStripStartXInd == iPatchSize + 1);
    assert( (UINT)(pCurrIndex - &IndicesBuffer[0]) == m_uiIndicesInFullResolutionStrip );

    // Prepare buffer description
    D3D11_BUFFER_DESC IndexBufferDesc;
    ZeroMemory(&IndexBufferDesc, sizeof(IndexBufferDesc));
    IndexBufferDesc.Usage          = D3D11_USAGE_DEFAULT;
    IndexBufferDesc.ByteWidth      = sizeof( DWORD ) * m_uiIndicesInFullResolutionStrip;
    IndexBufferDesc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
    IndexBufferDesc.CPUAccessFlags = 0;
    IndexBufferDesc.MiscFlags      = 0;

    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = &IndicesBuffer[0];
    InitData.SysMemPitch = 0; // This member is used only for 2D and 3D texture resources; it is ignored for the other resource types
    InitData.SysMemSlicePitch = 0; // This member is only used for 3D texture resources; it is ignored for the other resource types. 

    // Create the buffer
    Result = m_pd3dDevice11->CreateBuffer( &IndexBufferDesc, &InitData, &m_pFullResolutionIndBuffer );
    CHECK_HR_RET(Result, _T("Failed to create full triangulation index buffer") )

    return S_OK;
}

CTerrainPatch* CAdaptiveModelDX11Render::GetCurrPatch()
{
	noVec3 ray = GetCamera()->GetFrom();
	noVec3 dir = noVec3(0, -1, 0);	

	OptimalPatchesList::const_iterator endIt = m_OptimalPatchesList.end();
	for( OptimalPatchesList::const_iterator patchIt = m_OptimalPatchesList.begin(); patchIt != endIt; patchIt++ )
	{
		if( patchIt->bIsPatchVisible )
		{
			const SPatchBoundingBox &PatchBoundBox = patchIt->pPatchQuadTreeNode->GetData().BoundBox;		

			noVec3 vMin= noVec3(PatchBoundBox.fMinX, PatchBoundBox.fMinY , PatchBoundBox.fMinZ);
			noVec3 vMax= noVec3(PatchBoundBox.fMaxX, PatchBoundBox.fMaxY , PatchBoundBox.fMaxZ);		 		
			idBounds aabb(vMin, vMax);
			float fDist;
			noVec3 rayOrig(ray[0], ray[1], ray[2]), rayDir(dir[0], dir[1], dir[2]);
			if (aabb.RayIntersection(rayOrig, rayDir, fDist))
				return patchIt->pPatchQuadTreeNode->GetData().pPatch.get();
		}
	}
	return NULL;
	//return RecursiveGetPatch(m_PatchQuadTreeRoot, rayOrig.ToFloatPtr(), rayDir.ToFloatPtr());
}

CTerrainPatch* CAdaptiveModelDX11Render::RecursiveGetPatch( CPatchQuadTreeNode &PatchNode, const float* ray, const float* dir )
{

	if( PatchNode.GetPos().level > 0 )
	{
		// Update resource of the current patch
		 const SPatchBoundingBox &PatchBoundBox = PatchNode.GetData().BoundBox;
		 
		 noVec3 vMin= noVec3(PatchBoundBox.fMinX, PatchBoundBox.fMinY , PatchBoundBox.fMinZ);
		 noVec3 vMax= noVec3(PatchBoundBox.fMaxX, PatchBoundBox.fMaxY , PatchBoundBox.fMaxZ);		 		
		 idBounds aabb(vMin, vMax);
		 float fDist;
		 noVec3 rayOrig(ray[0], ray[1], ray[2]), rayDir(dir[0], dir[1], dir[2]);
		 if (aabb.RayIntersection(rayOrig, rayDir, fDist))
			 return PatchNode.GetData().pPatch.get();
	}

	// Get children
	CPatchQuadTreeNode *pDescendantNode[4];
	PatchNode.GetDescendants(pDescendantNode[0], pDescendantNode[1], pDescendantNode[2], pDescendantNode[3]);

	// Process children
	for(int iChild=0; iChild<4; iChild++)
		if( pDescendantNode[iChild] )
			return RecursiveGetPatch(*pDescendantNode[iChild], ray, dir);

	return NULL;
}



HRESULT CAdaptiveModelDX11Render::OnD3D11CreateDevice( ID3D11Device* pd3dDevice,
                                                       ID3D11DeviceContext* pd3dImmediateContext)
{
    HRESULT hr;
    
    const bool gammaCorrection = true;

    m_pd3dDevice11 = pd3dDevice;
    m_pd3dDeviceContext = pd3dImmediateContext;
    
    // Create elevation color texture shader resource view
    CHAR str[MAX_PATH] = "Terrain/splat3.jpg";//"Terrain/splat1.dds"; //"Media/ElevationColor.bmp";    

    {
        D3DX11_IMAGE_LOAD_INFO loadInfo;
        loadInfo.Usage = D3D11_USAGE_IMMUTABLE;
        loadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        loadInfo.CpuAccessFlags = 0;
        loadInfo.MiscFlags = 0;
        loadInfo.Format = gammaCorrection ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
        loadInfo.Filter = gammaCorrection ? (D3DX11_FILTER_NONE | D3DX11_FILTER_SRGB) : D3DX11_FILTER_NONE;
        hr = D3DX11CreateShaderResourceViewFromFile(pd3dDevice, str, &loadInfo, NULL, &m_pElevationColorsSRV, NULL);
        CHECK_HR_RET(hr, _T("Failed to load ElevationColor.bmp texture"));
    }
    CompileRenderPatchEffect( pd3dDevice );

    // Common objects must be created before other resources in the tree
    hr = m_pPatchCommon->OnD3D11CreateDevice( pd3dDevice,
                                              pd3dImmediateContext,
                                              m_Params.m_iPatchSize );
    CHECK_HR_RET(hr, _T("Failed to create patch common device objects"));

    // Create and udpate all resources in the tree
    RecursiveCreateD3D11PatchResources(m_PatchQuadTreeRoot);
    RecursiveUpdateDeviceResources(m_PatchQuadTreeRoot);

    // Create vertex input layout for bounding box buffer
    const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "BOUND_BOX_MIN_XYZ",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "BOUND_BOX_MAX_XYZ",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "BOUND_BOX_COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, 24, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };

    D3DX11_PASS_SHADER_DESC VsPassDesc;
    D3DX11_EFFECT_SHADER_DESC VsDesc;

    m_RenderEffectVars.m_pRenderBoundBox_FeatureLevel10Tech->GetPassByIndex(0)->GetVertexShaderDesc(&VsPassDesc);
    VsPassDesc.pShaderVariable->GetShaderDesc(VsPassDesc.ShaderIndex, &VsDesc);

    hr = pd3dDevice->CreateInputLayout( layout, ARRAYSIZE( layout ),
                                         VsDesc.pBytecode,
                                         VsDesc.BytecodeLength,
                                         &m_pBoundBoxInputLayout );
    CHECK_HR_RET(hr, _T("Failed to create the input layout for bound box buffer"));

    // Create index buffer for the full resolution triangulation
    hr = CreateFullResolutionStripBuffer(m_Params.m_iPatchSize);
    CHECK_HR_RET(hr, _T("Failed to create full resolution strip buffer"));

    // Create rasterizer state for solid fill mode
    D3D11_RASTERIZER_DESC RSSolidFill = 
    {
        D3D11_FILL_SOLID,
        D3D11_CULL_BACK,
        true, //BOOL FrontCounterClockwise;
        0,// INT DepthBias;
        0,// FLOAT DepthBiasClamp;
        0,// FLOAT SlopeScaledDepthBias;
        false,//BOOL DepthClipEnable;
        false,//BOOL ScissorEnable;
        false,//BOOL MultisampleEnable;
        false,//BOOL AntialiasedLineEnable;
    };
    hr = m_pd3dDevice11->CreateRasterizerState( &RSSolidFill, &m_pRSSolidFill );

    // Create rasterizer state with bias used to render terrain when wireframe is shown
    RSSolidFill.DepthBias = 1;
    RSSolidFill.SlopeScaledDepthBias = 1;
    //RSSolidFill.DepthBiasClamp = 100;
    hr = m_pd3dDevice11->CreateRasterizerState( &RSSolidFill, &m_pRSSolidFill_Biased );

    return S_OK;
}


std::auto_ptr<CTerrainPatch> CAdaptiveModelDX11Render::CreatePatch(class CPatchElevationData *pPatchElevData,
                                                                   class CRQTTriangulation *pAdaptiveTriangulation)const
{
    return std::auto_ptr<CTerrainPatch>(
                new CTerrainPatch(m_pPatchCommon.get(), pPatchElevData, pAdaptiveTriangulation) );
}

// The method hierarchically traverses the tree and releases all D3D device resources
void CAdaptiveModelDX11Render::RecursiveDestroyD3D11PatchResources(CPatchQuadTreeNode &PatchNode)
{
    // Get children
    CPatchQuadTreeNode *pDescendantNode[4];
    PatchNode.GetDescendants(pDescendantNode[0], pDescendantNode[1], pDescendantNode[2], pDescendantNode[3]);

    // Process children
    for(int iChild=0; iChild<4; iChild++)
        if( pDescendantNode[iChild] )
            RecursiveDestroyD3D11PatchResources( *(pDescendantNode[iChild]) );
    
    // Unbind children so they get released
    if( PatchNode.GetData().pPatch.get() )
        PatchNode.GetData().pPatch->BindChildren(NULL, NULL, NULL, NULL);
    // Release all data
    PatchNode.GetData().pPatch.reset();
    PatchNode.GetData().m_pIncreaseLODTask.reset(); // this task contains decompressed children which are not yet in the hierarchy
    PatchNode.GetData().m_pDecreaseLODTask.reset(); // contains pointer to CDX11PatchCache which is accessed from task destructor
}


void CAdaptiveModelDX11Render::OnD3D11DestroyDevice( )
{
    WaitForAsyncTasks();

    RecursiveDestroyD3D11PatchResources(m_PatchQuadTreeRoot);

    m_pElevationColorsSRV.Release();
    m_pRenderEffect11.Release();
    m_pFullResolutionIndBuffer.Release();
    m_pRSSolidFill.Release();
    m_pRSSolidFill_Biased.Release();

    m_pd3dDevice11.Release();
    m_pd3dDeviceContext.Release();

    m_pBoundBoxInstBuffer.Release();
    m_pBoundBoxInputLayout.Release();
    m_pRenderEffect11.Release();
    
	if (m_pPatchCommon.get())
		m_pPatchCommon->OnD3D11DestroyDevice();

    m_pd3dDeviceContext.Release();
    m_pd3dDevice11.Release();
}


HRESULT CAdaptiveModelDX11Render::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                                   const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
    m_iBackBufferWidth = pBackBufferSurfaceDesc->Width;
    m_iBackBufferHeight= pBackBufferSurfaceDesc->Height; 

	m_pRenderEffect11->GetVariableByName( "g_ScreenAspectRatio" )->AsScalar()->SetFloat( GetApp()->GetAspectRatio() );
	if (gClipMap2) {
		gClipMap2->alloc(D3D11Dev());			
	}

    return S_OK;
}

void CAdaptiveModelDX11Render::OnD3D11ReleasingSwapChain( void* pUserContext )
{
}

// This function is used for sorting patches
bool ComparePatchesByDistToCamera( const SPatchRenderingInfo &Patch1Info, const SPatchRenderingInfo &Patch2Info)
{
    return Patch1Info.fDistanceToCamera < Patch2Info.fDistanceToCamera;
}

float  CAdaptiveModelDX11Render::GetPatchScale(CTerrainPatch *pDX11Patch) {
	float fPatchScale = m_Params.m_fElevationSamplingInterval * (float)(1 << ( (m_Params.m_iNumLevelsInPatchHierarchy-1) - pDX11Patch->m_pos.level) );
	return fPatchScale;
}

noVec2 CAdaptiveModelDX11Render::GetPatchCornerXY(CTerrainPatch *pDX11Patch) {

	float fPatchScale = m_Params.m_fElevationSamplingInterval * (float)(1 << ( (m_Params.m_iNumLevelsInPatchHierarchy-1) - pDX11Patch->m_pos.level) );
	noVec2 vPatchLBCorner;
	vPatchLBCorner.x = (float)(pDX11Patch->m_pos.horzOrder * m_Params.m_iPatchSize) * fPatchScale;
	vPatchLBCorner.y = (float)(pDX11Patch->m_pos.vertOrder * m_Params.m_iPatchSize) * fPatchScale;
	return vPatchLBCorner;
}

// Render all patches in the model
int CAdaptiveModelDX11Render::RenderPatches(const D3DXMATRIX &WorldViewProjMatr,
                                            float fScreenSpaceTreshold,
                                            bool bZOnlyPass,
                                            bool bShowWireframe,
                                            SPatchRenderingInfo *pPatchesToRender,
                                            size_t NumPatchesToRender)
{
    HRESULT hr;

    if(NumPatchesToRender == 0) 
        return 0;

    // If the effect is not yet created, compile it
    if( !m_pRenderEffect11 )
    {
        hr = CompileRenderPatchEffect(m_pd3dDevice11);
        if( FAILED(hr) )
            return -1;
    }

    int iTotalTrianglesRendered = 0;
    m_pd3dDeviceContext->IASetInputLayout( NULL );
	
	//gClipmap->UpdateClipMap();
	//gClipMap2->recenter(float(sliderX->GetValue()) / 1024.f, float(sliderY->GetValue()) / 1024.f);
	noVec3 camPos = GetApp()->ActiveCam_->GetFrom();
	//gClipMap2->recenter(camPos.x / (128 * 32) , camPos.z / (128 * 32));
	//gClipMap2->updateEffectParams();

	m_pPatchCommon->m_terrainCommonMat->Update(GetApp()->ddt / 1000.f);
	m_pPatchCommon->m_terrainCommonMat->CommitCommonShaderConstants();
		

    // Go through all patches in the model
	//NumPatchesToRender = max(1, NumPatchesToRender);
    for(size_t iPatchNum = 0; iPatchNum < NumPatchesToRender; iPatchNum++)
    {
        SPatchRenderingInfo &PatchToRenderInfo = pPatchesToRender[iPatchNum];
        CTerrainPatch *pDX11Patch = PatchToRenderInfo.pPatch;
        // To perform morphing, terrain rendering pixel shader calculates 
        // per-pixel normal as a blend of the patch own normal and the parent 
        // patch normal:
        // Normal = lerp(Normal, ParentNormalXY, fMorphCoeff);
        // Thus a parent patch normal map is required
        const CTerrainPatch *pDX11ParentPatch = pDX11Patch->GetParent();
        if( pDX11ParentPatch == NULL )
            pDX11ParentPatch = pDX11Patch;

        float fFlangeWidth = PatchToRenderInfo.fFlangeWidth;
        float fMorphCoeff =  PatchToRenderInfo.fMorphCoeff;

        if(pDX11Patch == NULL)
            continue;


		

        // Set variables
        float fPatchScale = m_Params.m_fElevationSamplingInterval * (float)(1 << ( (m_Params.m_iNumLevelsInPatchHierarchy-1) - pDX11Patch->m_pos.level) );
        m_RenderEffectVars.m_pevPatchXYScale->SetFloat( fPatchScale );

		if (pDX11Patch->m_terrainMat) {
			pDX11Patch->m_terrainMat->TerrainFarRange = m_iPatchSize * fPatchScale;
			pDX11Patch->m_terrainMat->CommitPerInstanceShaderConstants();
		}

        // Flange is used to hide gaps between neighboring patches
         m_RenderEffectVars.m_pevFlangeWidth->SetFloat( fFlangeWidth );
        // Morph coefficient determines morph ratio
         m_RenderEffectVars.m_pevMorphCoeff->SetFloat( fMorphCoeff ) ;

        // Set the patch position in a sibling quad (siblings are patches having common parent)
        int PatchOrderInSiblQuad[] = {pDX11Patch->m_pos.horzOrder&0x01, pDX11Patch->m_pos.vertOrder&0x01, 0,0};
        //      _____ _____
        //     |     |     |
        //     |(0,1)|(1,1)| 2M+1
        //     |_____|_____| 
        //     |     |     |
        //     |(0,0)|(1,0)| 2M
        //     |_____|_____|
        //       2N   2N+1
        //
        m_RenderEffectVars.m_pevPatchOrderInSiblQuad->SetIntVector(PatchOrderInSiblQuad);

        // Get coordinates of the pacth's left bottom corner
        D3DXVECTOR4 vPatchLBCorner;
        vPatchLBCorner.x = (float)(pDX11Patch->m_pos.horzOrder * m_Params.m_iPatchSize) * fPatchScale;
        vPatchLBCorner.y = (float)(pDX11Patch->m_pos.vertOrder * m_Params.m_iPatchSize) * fPatchScale;
        m_RenderEffectVars.m_pevPatchLBCornerXY->SetFloatVector( &vPatchLBCorner.x );


		if (gClipmap) {
			gClipmap->SetClipmapVars();		
			m_RenderEffectVars.m_pPyramidMap->SetResource( gClipmap->PyramidTextureSRV );
			m_RenderEffectVars.m_pPyramidHeightMap->SetResource( gClipmap->PyramidTextureHMSRV );
			m_RenderEffectVars.m_pStackMap->SetResource( gClipmap->StackTextureSRV );
		}

		UINT32 numTilesX = DispatchSize(App::LightTileSize, GetApp()->colorTarget->Width);
		UINT32 numTilesY = DispatchSize(App::LightTileSize, GetApp()->colorTarget->Height);


		// LID start
		D3DXMATRIX mView;
		memcpy(&mView, GetApp()->GetCamera()->getViewMatrix(), SIZE16);		
		m_pRenderEffect11->GetVariableByName("View")->AsMatrix()->SetMatrix((float*)mView);
		XMMATRIX world =XMMatrixIdentity();
		{
			D3DXMATRIX d3dWorld;
			D3DXMatrixIdentity(&d3dWorld);
			m_pRenderEffect11->GetVariableByName("World")->AsMatrix()->SetMatrix((float*)d3dWorld);

		}
		//D3DXVECTOR3 lightDirWS(LIDRenderer::GetSingleton().GetPSConstBuffer()->Data.LightDirWS.x, LIDRenderer::GetSingleton().GetPSConstBuffer()->Data.LightDirWS.y, LIDRenderer::GetSingleton().GetPSConstBuffer()->Data.LightDirWS.z);
		//m_pRenderEffect11->GetVariableByName("LightDirWS")->AsVector()->SetFloatVector(lightDirWS);
		LIDRenderer::GetSingletonPtr()->BeginRender(GetApp()->GetCamera(), D3D11Context(), world, numTilesX, numTilesY);

		m_pRenderEffect11->GetVariableByName("Lights")->AsShaderResource()->SetResource(GetApp()->lightsBuffer->SRView);
		m_pRenderEffect11->GetVariableByName("TileLights")->AsShaderResource()->SetResource(GetApp()->lightIndicesBuffer->SRView);
						
		ID3D11Buffer* buffer;		
		m_RenderEffectVars.m_pLightConstBuffer->GetConstantBuffer(&buffer);
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		DXCall(D3D11Context()->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CopyMemory(mappedResource.pData, &LIDRenderer::GetSingletonPtr()->GetPSConstBuffer()->Data, sizeof(LIDRenderer::GetSingletonPtr()->GetPSConstBuffer()->Data));
		D3D11Context()->Unmap(buffer, 0);
		// LID end
			
		// PSSM
		//m_pRenderEffect11->GetConstantBufferByName("cbPSSM")->AsConstantBuffer()->SetConstantBuffer(GetShadowMap<PSSMShadowMapDx11>()->m_pD3DBuffer);
		m_pRenderEffect11->GetVariableByName("g_mTextureMatrix")->AsMatrix()->SetMatrixArray((float*)GetShadowMap<PSSMShadowMapDx11>()->mPssmConstants.TextureMatrixArray, 0, 4);
		m_pRenderEffect11->GetVariableByName("g_fSplitPlane")->AsScalar()->SetFloatArray((float*)GetShadowMap<PSSMShadowMapDx11>()->mPssmConstants.SplitPlane, 0, 4);
		m_pRenderEffect11->GetVariableByName("g_txShadowMapArray")->AsShaderResource()->SetResource(GetShadowMap<PSSMShadowMapDx11>()->m_pSRV);
						
        // Set height and normal maps
        m_RenderEffectVars.m_pevElevationMap->SetResource( pDX11Patch->GetElevDataSRV() );
        m_RenderEffectVars.m_pevNormalMap->SetResource( pDX11Patch->GetNormalMapSRV() );
        // If morph is enabled, also set parent height and normal maps
        if( m_RenderParams.m_bEnableHeightMapMorph || m_RenderParams.m_bEnableNormalMapMorph  )
            m_RenderEffectVars.m_pevParentElevMap->SetResource( pDX11ParentPatch->GetElevDataSRV() );
        if( m_RenderParams.m_bEnableNormalMapMorph )
            m_RenderEffectVars.m_pevParentNormalMap->SetResource( pDX11ParentPatch->GetNormalMapSRV() );
        // Set transform matrix
        m_RenderEffectVars.m_pevWorldViewProj->SetMatrix( (float*)&WorldViewProjMatr );
        // Set appropriate rasterizer state
        m_RenderEffectVars.m_pevRS_SolidFill->SetRasterizerState(0, bShowWireframe ? m_pRSSolidFill_Biased : m_pRSSolidFill );

        bool bFullResTriangulaton = !m_bEnableAdaptTriang || pDX11Patch->m_pIndexBuffer == NULL;
        // Set index buffer and prim topology
        m_pd3dDeviceContext->IASetIndexBuffer( bFullResTriangulaton ? m_pFullResolutionIndBuffer : pDX11Patch->m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
        m_pd3dDeviceContext->IASetPrimitiveTopology( bFullResTriangulaton ? D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
        // Apply technique pass
        m_RenderEffectVars.m_pevRenderPatch_FeatureLevel10Tech->GetPassByIndex(bZOnlyPass ? 2 : 0)->Apply(0, m_pd3dDeviceContext);
        // Render the patch
        m_pd3dDeviceContext->DrawIndexed( bFullResTriangulaton ? m_uiIndicesInFullResolutionStrip : pDX11Patch->m_uiNumIndicesInAdaptiveTriang, // Number of indices to draw
                                      0, // Index of the first index
                                      0 // Index of the first vertex. 
                                      );
        // Render wireframe model, if necessary
        if( bShowWireframe )
        {
            m_RenderEffectVars.m_pevRenderPatch_FeatureLevel10Tech->GetPassByIndex(1)->Apply(0, m_pd3dDeviceContext);
            
            m_pd3dDeviceContext->DrawIndexed(
                bFullResTriangulaton ? m_uiIndicesInFullResolutionStrip : pDX11Patch->m_uiNumIndicesInAdaptiveTriang, // Number of indices to draw
                0, // Index of the first index
                0 // Index of the first vertex. 
            );
        }

        // Count total number of rendered triangles
        int iTrianglesRendered = bFullResTriangulaton ? m_uiIndicesInFullResolutionStrip-2 : pDX11Patch->m_uiNumIndicesInAdaptiveTriang/3;
        iTotalTrianglesRendered += iTrianglesRendered;
    }
		
	m_pRenderEffect11->GetVariableByName("g_txShadowMapArray")->AsShaderResource()->SetResource(NULL);
	if (gClipmap) {
		m_RenderEffectVars.m_pPyramidMap->SetResource( NULL );
		m_RenderEffectVars.m_pPyramidHeightMap->SetResource( NULL );
		m_RenderEffectVars.m_pStackMap->SetResource( NULL );
	}
    m_RenderEffectVars.m_pevElevationMap->SetResource( NULL );
    m_RenderEffectVars.m_pevNormalMap->SetResource( NULL );
    m_RenderEffectVars.m_pevParentElevMap->SetResource( NULL );
    m_RenderEffectVars.m_pevParentNormalMap->SetResource( NULL );

	//if (!bZOnlyPass)
	//	m_pPatchCommon->m_terrainCommonMat->EndPass_Main();
	
    m_RenderEffectVars.m_pevRenderPatch_FeatureLevel10Tech->GetPassByIndex(0)->Apply(0, m_pd3dDeviceContext);
    
    return iTotalTrianglesRendered;
}

// Method renders terrain mini map
void CAdaptiveModelDX11Render::RenderTerrainMap(const D3DXVECTOR4 &ScreenPos,
                                                SPatchRenderingInfo pLevel1Patches[4])
{
    m_pd3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    // Use 4 patches at the coarsest resolution level to render the map
    for(int iLevel1Patch = 0; iLevel1Patch < 4; iLevel1Patch++)
    {
        if(pLevel1Patches[iLevel1Patch].pPatch)
        {
            CTerrainPatch *pDX11Wrpr = pLevel1Patches[iLevel1Patch].pPatch;
            m_RenderEffectVars.m_pevElevationMap->SetResource( pDX11Wrpr->GetElevDataSRV() );
            // Set patch screen location
            D3DXVECTOR4 ScrPos;
            ScrPos.x = ScreenPos.x + ((iLevel1Patch & 0x01) ? ScreenPos.z/2.f : 0.f);
            ScrPos.y = ScreenPos.y - ScreenPos.w + ((iLevel1Patch >> 0x01) ? ScreenPos.w/2.f : 0.f);
            ScrPos.z = ScreenPos.z/2.f;
            ScrPos.w = ScreenPos.w/2.f;
            m_RenderEffectVars.m_pevTerrainMapPos_PS->SetFloatVector(ScrPos);
            m_RenderEffectVars.m_pevRenderHeightMapPreview_FL10->GetPassByIndex(0)->Apply(0, m_pd3dDeviceContext);
            // Render
            m_pd3dDeviceContext->IASetInputLayout(NULL);
            m_pd3dDeviceContext->Draw(4,0);
        }
    }
}

HRESULT CAdaptiveModelDX11Render::CreateBoundBoxInstBuffer(size_t RequiredSize)
{
    HRESULT hr;

    bool bCreateBoundBoxIntsBuffer = false;
    D3D11_BUFFER_DESC BuffDesc;
    if( m_pBoundBoxInstBuffer == NULL )
    {
        BuffDesc.ByteWidth = static_cast<UINT>(sizeof(BBoxInstance) * max((int)RequiredSize*2, (int)1024));
        bCreateBoundBoxIntsBuffer = true;
    }
    else
    {
        // Ensure the buffer is large enough to store all bounding boxes
        m_pBoundBoxInstBuffer->GetDesc(&BuffDesc);
        if( BuffDesc.ByteWidth < RequiredSize * sizeof(BBoxInstance) )
        {
            // Enlarge buffer if it is necessary
            BuffDesc.ByteWidth = static_cast<UINT>(RequiredSize*2 * sizeof(BBoxInstance));
            bCreateBoundBoxIntsBuffer = true;
        }
    }
    // Create new buffer
    if( bCreateBoundBoxIntsBuffer )
    {
        BuffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        BuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        BuffDesc.MiscFlags = 0;
        BuffDesc.StructureByteStride = 0;
        BuffDesc.Usage = D3D11_USAGE_DYNAMIC;
        m_pBoundBoxInstBuffer.Release();
        m_pd3dDevice11->CreateBuffer( &BuffDesc, NULL, &m_pBoundBoxInstBuffer);
    }

    return S_OK;
}

void CAdaptiveModelDX11Render::ComputeTerrainBoundingBox( noVec3& vMin, noVec3& vMax )
{
	noVec3 vMeshMin;
	noVec3 vMeshMax;

	noVec3 sceneMin(FLT_MAX, FLT_MAX, FLT_MAX);
	noVec3 sceneMax(FLT_MIN, FLT_MIN, FLT_MIN);
	bool bSuccess = false;
	BoundingBox unionBB;
	OptimalPatchesList::const_iterator endIt = m_OptimalPatchesList.end();
	for( OptimalPatchesList::const_iterator patchIt = m_OptimalPatchesList.begin(); patchIt != endIt; patchIt++ )
	{
		if( patchIt->bIsPatchVisible )
		{
			const SPatchBoundingBox &PatchBoundBox = patchIt->pPatchQuadTreeNode->GetData().BoundBox;

			vMeshMin.Set(PatchBoundBox.fMinX, PatchBoundBox.fMinY , PatchBoundBox.fMinZ);

			vMeshMax.Set(PatchBoundBox.fMaxX, PatchBoundBox.fMaxY , PatchBoundBox.fMaxZ);

			noVec3 temp = vMeshMin;
			temp.Min( sceneMin );
			sceneMin = temp;
			temp = vMeshMax;
			temp.Max( sceneMax );		 
			sceneMax = temp;
			bSuccess = true;
		}
	}
	
	if (bSuccess)
	{	
		vMin = sceneMin;
		vMax = sceneMax;	
	}	
}


void CAdaptiveModelDX11Render::RenderBoundingBoxes(ID3D11DeviceContext* pd3dImmediateContext,
                                                   const D3DXMATRIX &CameraViewProjMatrix,
                                                   bool bShowPreview)
{
    HRESULT hr;

    // Assure bounding box instance buffer is large enough to hold all data
    CreateBoundBoxInstBuffer(m_OptimalPatchesList.size());

    // Populate the buffer
    D3D11_MAPPED_SUBRESOURCE MappedData;
    pd3dImmediateContext->Map( m_pBoundBoxInstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedData );
    BBoxInstance *pBoundBox = (BBoxInstance *)MappedData.pData;
    UINT iVisiblePatchNum = 0;
    OptimalPatchesList::const_iterator endIt = m_OptimalPatchesList.end();
    for( OptimalPatchesList::const_iterator patchIt = m_OptimalPatchesList.begin(); patchIt != endIt; patchIt++ )
    {
        if( patchIt->bIsPatchVisible )
        {
            const SPatchBoundingBox &PatchBoundBox = patchIt->pPatchQuadTreeNode->GetData().BoundBox;
            pBoundBox[iVisiblePatchNum].vMin = D3DXVECTOR3(PatchBoundBox.fMinX, PatchBoundBox.fMinY , PatchBoundBox.fMinZ);
            pBoundBox[iVisiblePatchNum].vMax = D3DXVECTOR3(PatchBoundBox.fMaxX, PatchBoundBox.fMaxY , PatchBoundBox.fMaxZ);
            if(patchIt->pPatchQuadTreeNode->GetData().m_pIncreaseLODTask.get())
                pBoundBox[iVisiblePatchNum].color =  0xff00ff00;
            else if( patchIt->pPatchQuadTreeNode->GetAncestor() &&
                        patchIt->pPatchQuadTreeNode->GetAncestor()->GetData().m_pDecreaseLODTask.get() )
                pBoundBox[iVisiblePatchNum].color =  0xffff0000;
            else 
                pBoundBox[iVisiblePatchNum].color = 0xff204ccc;
            iVisiblePatchNum++;
            if( iVisiblePatchNum == m_PatchRenderingInfo.size() )
                break;
        }
    }
    pd3dImmediateContext->Unmap( m_pBoundBoxInstBuffer, 0 );

    // Render bounding boxes using instancing
    UINT offset[1] = { 0 };
    UINT stride[1] = { sizeof(BBoxInstance) };
    ID3D11Buffer* const ppBuffers[1] = { m_pBoundBoxInstBuffer };
    pd3dImmediateContext->IASetVertexBuffers( 0, 1, ppBuffers, stride, offset );
    pd3dImmediateContext->IASetInputLayout( m_pBoundBoxInputLayout );
    pd3dImmediateContext->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_LINELIST );
    m_RenderEffectVars.m_pevWorldViewProj->SetMatrix( (float*)&CameraViewProjMatrix);
    m_RenderEffectVars.m_pRenderBoundBox_FeatureLevel10Tech->GetPassByIndex(0)->Apply(0, pd3dImmediateContext);
    pd3dImmediateContext->DrawInstanced(24, static_cast<UINT>(m_PatchRenderingInfo.size()), 0, 0);

    // Render quad tree preview
    if( bShowPreview )
    {
        RenderQuadTreePreview(pd3dImmediateContext);
    }
}

// Renders bounding boxes for all visible patches
void CAdaptiveModelDX11Render::RenderQuadTreePreview(ID3D11DeviceContext* pd3dImmediateContext)
{
    HRESULT hr;

    // Render quad tree preview
    D3DXVECTOR4 QuadTreePreviewPos_PS;
    QuadTreePreviewPos_PS.x = m_vQuadTreePreviewScrPos.x / m_fViewportWidth;
    QuadTreePreviewPos_PS.y = m_vQuadTreePreviewScrPos.y / m_fViewportHeight;
    QuadTreePreviewPos_PS.z = m_vQuadTreePreviewScrPos.z / m_fViewportWidth;
    QuadTreePreviewPos_PS.w = m_vQuadTreePreviewScrPos.w / m_fViewportHeight;
    QuadTreePreviewPos_PS.x = QuadTreePreviewPos_PS.x*2-1;
    QuadTreePreviewPos_PS.y = 1 - QuadTreePreviewPos_PS.y*2;
    QuadTreePreviewPos_PS.z *= 2.f;
    QuadTreePreviewPos_PS.w *= 2.f;
        
    const CPatchQuadTreeNode *pRoot = m_OptimalPatchesList[0].pPatchQuadTreeNode;
    for(; pRoot->GetAncestor(); pRoot = pRoot->GetAncestor());
    const CPatchQuadTreeNode *pLevel1Patches[4] = {NULL};
    pRoot->GetDescendants(pLevel1Patches[0], pLevel1Patches[1], pLevel1Patches[2], pLevel1Patches[3]);
    if(pLevel1Patches[0] && pLevel1Patches[1] && pLevel1Patches[2] && pLevel1Patches[3])
    {
        SPatchRenderingInfo pLvl1PtchRndrInfo[4];
        for(int i=0; i<4; i++)
            pLvl1PtchRndrInfo[i].pPatch = pLevel1Patches[i]->GetData().pPatch.get();
        // Render small terrain map
        RenderTerrainMap(QuadTreePreviewPos_PS, pLvl1PtchRndrInfo);
    }

    D3D11_MAPPED_SUBRESOURCE MappedData;
    pd3dImmediateContext->Map( m_pBoundBoxInstBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedData );
    BBoxInstance *pBoundBox = (BBoxInstance *)MappedData.pData;
    int iOptimalPatchNum = 0;
    OptimalPatchesList::iterator OptimalPatchesListEndIt = m_OptimalPatchesList.end();
    // Render all patches on the mini map
    for( OptimalPatchesList::iterator patchIt = m_OptimalPatchesList.begin(); patchIt != OptimalPatchesListEndIt; patchIt++ )
    {
        if( patchIt->pPatchQuadTreeNode->GetPos().level == 0 )
            continue;

        int iHorzOrder = patchIt->pPatchQuadTreeNode->GetPos().horzOrder;
        int iVertOrder = patchIt->pPatchQuadTreeNode->GetPos().vertOrder;
        int iLevel = patchIt->pPatchQuadTreeNode->GetPos().level;

        pBoundBox[iOptimalPatchNum].vMin = D3DXVECTOR3( (float) iHorzOrder   /(float)(1<<iLevel), 0, (float)iVertOrder    /(float)(1<<iLevel) );
        pBoundBox[iOptimalPatchNum].vMax = D3DXVECTOR3( (float)(iHorzOrder+1)/(float)(1<<iLevel), 0, (float)(iVertOrder+1)/(float)(1<<iLevel) );
        unsigned long color;

        if( patchIt->bIsPatchVisible )
        {
            color = patchIt->pPatchQuadTreeNode->GetData().m_pIncreaseLODTask.get() ? 0xff00ff00 : 
                (patchIt->pPatchQuadTreeNode->GetData().m_pDecreaseLODTask.get() ? 0xff0000ff : 0xff204ccc);
        }
        else
        {
            color = patchIt->pPatchQuadTreeNode->GetData().m_pIncreaseLODTask.get() ? 0x99009900 : 
                (patchIt->pPatchQuadTreeNode->GetData().m_pDecreaseLODTask.get() ? 0x99000099 : 0x88102666);
        }
            
        pBoundBox[iOptimalPatchNum].color = color;
        iOptimalPatchNum++;
    }
    pd3dImmediateContext->Unmap( m_pBoundBoxInstBuffer, 0 );

    V( m_RenderEffectVars.m_pQuadTreePreviewPos_PS->SetFloatVector( QuadTreePreviewPos_PS ) );

    D3DXVECTOR4 vScreenPixelSize( 1.f / (float)m_fViewportWidth, 1.f / (float)m_fViewportHeight, 0.f, 0.f );
    V( m_RenderEffectVars.m_pScreenPixelSize->SetFloatVector( vScreenPixelSize ) );
        
    m_RenderEffectVars.m_pRenderQuadTree_FeatureLevel10Tech->GetPassByIndex(0)->Apply(0, pd3dImmediateContext);
    pd3dImmediateContext->IASetInputLayout( m_pBoundBoxInputLayout );
    pd3dImmediateContext->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_LINELIST );
    pd3dImmediateContext->DrawInstanced(8, static_cast<UINT>(m_OptimalPatchesList.size()), 0, 0);
}


void CAdaptiveModelDX11Render::UpdateTerrain(ID3D11DeviceContext* pd3dImmediateContext,
	const D3DXVECTOR3 &vCameraPosition, 
	const D3DXMATRIX &CameraViewProjMatrix)
{

	// Extract view frustum planes to determine pacth visibility
	ExtractViewFrustumPlanesFromMatrix(CameraViewProjMatrix, m_CameraViewFrustum);

	// build list of visible patches
	OptimalPatchesList::iterator endIt = m_OptimalPatchesList.end();
	m_PatchRenderingInfo.clear();
	// Go through all pacthes in the current model
	for( OptimalPatchesList::iterator patchIt = m_OptimalPatchesList.begin(); patchIt != endIt; patchIt++ )
	{
		if( patchIt->pPatchQuadTreeNode->GetPos().level == 0 )
			continue;

		const SPatchBoundingBox &PatchBoundBox = patchIt->pPatchQuadTreeNode->GetData().BoundBox;

		AcquireSRWLockShared(&m_srwLock);
		assert( PatchBoundBox.bIsBoxValid );
		AcquireSRWLockShared(&m_srwLock);

		// Determine patch bounding box visibility
		patchIt->bIsPatchVisible = IsBoxVisible(PatchBoundBox);
		if( patchIt->bIsPatchVisible )
		{
			SPatchRenderingInfo CurrPatchInfo;
			CTerrainPatch *pPatch = patchIt->pPatchQuadTreeNode->GetData().pPatch.get();

			// Compute flange width so that its projection onto screen plane is 2*m_Params.m_fScrSpaceErrorBound pixels.
			// We need to multiple the threshold by 2 because approximated model image can deviate from exact model image 
			// by at most m_Params.m_fScrSpaceErrorBound pixels, in BOTH directions (+ and -).
			//
			// Screen space error estimation is calculated according to the following formula:
			// fPatchScrSpaceError = fGuaranteedPatchErrorBound / fDistanceToCamera * m_fViewportStretchConst;
			// By substituting
			// fGuaranteedPatchErrorBound <- fFlangeWidth
			// fPatchScrSpaceError <- m_Params.m_fScrSpaceErrorBound
			// We will get the flange width:
			float fFlangeWidth = 2.f * m_Params.m_fScrSpaceErrorBound * patchIt->pPatchQuadTreeNode->GetData().m_fDistanceToCamera / m_fViewportStretchConst;
			// Flange width must not be less then the patch's approximation error bound as well. (Multiplication by 2
			// is required due to the same reason)
			fFlangeWidth = max( fFlangeWidth, 2.f * patchIt->pPatchQuadTreeNode->GetData().m_fGuaranteedPatchErrorBound ); 

			CurrPatchInfo.pPatch = pPatch;

			CurrPatchInfo.fFlangeWidth = fFlangeWidth;
			CurrPatchInfo.fDistanceToCamera = patchIt->pPatchQuadTreeNode->GetData().m_fDistanceToCamera;

			// Calculate morph coefficient
			float fMorphCoeff = 0.f;
			CPatchQuadTreeNode *pParent = patchIt->pPatchQuadTreeNode->GetAncestor();
			CPatchQuadTreeNode *pSiblings[4] = {NULL};
			if( pParent )
				pParent->GetDescendants(pSiblings[0], pSiblings[1], pSiblings[2], pSiblings[3]);
			// Morphing can be performed only if all siblings are optimal patches
			if( pParent &&
				!pParent->GetData().m_bUpdateRequired && // Morphing can not be performed if parent patch should be updated
				patchIt->pPatchQuadTreeNode->GetPos().level > 1 && // Patches at level 1 are not morphed
				pSiblings[0]->GetData().Label == SPatchQuadTreeNodeData::OPTIMAL_PATCH &&
				pSiblings[1]->GetData().Label == SPatchQuadTreeNodeData::OPTIMAL_PATCH &&
				pSiblings[2]->GetData().Label == SPatchQuadTreeNodeData::OPTIMAL_PATCH &&
				pSiblings[3]->GetData().Label == SPatchQuadTreeNodeData::OPTIMAL_PATCH )
			{
				// Calculate screen space error this patch has at the moment when parent patch is subdivided
				float fGuaranteedPatchErrorBound = patchIt->pPatchQuadTreeNode->GetData().m_fGuaranteedPatchErrorBound;
				float fParentGuaranteedErrorBound = patchIt->pPatchQuadTreeNode->GetAncestor() ? patchIt->pPatchQuadTreeNode->GetAncestor()->GetData().m_fGuaranteedPatchErrorBound : fGuaranteedPatchErrorBound*2.f;
				float fLODSwitchScrError =  m_Params.m_fScrSpaceErrorBound * fGuaranteedPatchErrorBound / fParentGuaranteedErrorBound;
				float fPatchScrSpaceError = patchIt->pPatchQuadTreeNode->GetData().m_fPatchScrSpaceError;
				float MorphInterval = (m_Params.m_fScrSpaceErrorBound-fLODSwitchScrError) * 0.1f;
				fMorphCoeff = ((fLODSwitchScrError+MorphInterval) - fPatchScrSpaceError ) / MorphInterval;
				fMorphCoeff = max(fMorphCoeff, 0.0f);
				fMorphCoeff = min(fMorphCoeff, 1.0f);
			}
			CurrPatchInfo.fMorphCoeff = fMorphCoeff;
			m_PatchRenderingInfo.push_back( CurrPatchInfo );

			//pPatch->m_fFlangeWidth = fFlangeWidth;
			//pPatch->m_fMorphCoeff = fMorphCoeff;
		}
	}
	
	//ComputeTerrainBoundingBox(this->m_AABB.m_vMin, this->m_AABB.m_vMax);	 
	//this->m_AABB.Set(this->m_AABB.m_vMin, this->m_AABB.m_vMax);

	// sort visible patches by distance
	if( m_bSortPatchesByDistance && !m_PatchRenderingInfo.empty() )
	{
		std::sort( &m_PatchRenderingInfo[0], &m_PatchRenderingInfo[0] + m_PatchRenderingInfo.size(), ComparePatchesByDistToCamera );
	}
}

HRESULT CAdaptiveModelDX11Render::Render(ID3D11DeviceContext* pd3dImmediateContext,
	const D3DXVECTOR3 &vCameraPosition, 
	const D3DXMATRIX &CameraViewProjMatrix, bool bZOnlyPass) {

	 m_iTotalTrianglesRendered = 0;

	 bool bShowBoundingBoxes = false;
	 bool bShowPreview = false;
	 bool bShowWireframeModel = false;		 

	 m_pPatchCommon->m_terrainCommonMat->RenderPass_Main();

	 // render visible patches
	 if( !m_PatchRenderingInfo.empty() )
	 {
		 m_iTotalTrianglesRendered = RenderPatches(CameraViewProjMatrix,
			 m_Params.m_fScrSpaceErrorBound,
			 bZOnlyPass,
			 bShowWireframeModel,
			 &m_PatchRenderingInfo[0],
			 m_PatchRenderingInfo.size());
	 }	
	 return S_OK;
}

// Renders the model
HRESULT CAdaptiveModelDX11Render::Render(ID3D11DeviceContext* pd3dImmediateContext,
                                         const D3DXVECTOR3 &vCameraPosition, 
                                         const D3DXMATRIX &CameraViewProjMatrix,
                                         bool bShowBoundingBoxes,
                                         bool bShowPreview,
                                         bool bShowWireframeModel,
                                         bool bZOnlyPass)
{
    m_iTotalTrianglesRendered = 0;

    if( bZOnlyPass )
    {
        bShowBoundingBoxes = false;
        bShowWireframeModel = false;
    }

	if (!bZOnlyPass)
		//m_pPatchCommon->m_terrainCommonMat->RenderPass_Main();

    // render visible patches
    if( !m_PatchRenderingInfo.empty() )
    {
        m_iTotalTrianglesRendered = RenderPatches(CameraViewProjMatrix,
                                                  m_Params.m_fScrSpaceErrorBound,
                                                  bZOnlyPass,
                                                  bShowWireframeModel,
                                                  &m_PatchRenderingInfo[0],
                                                  m_PatchRenderingInfo.size());
    }		

    // Render bounding boxes
    if( bShowBoundingBoxes )
    {
        RenderBoundingBoxes(pd3dImmediateContext, CameraViewProjMatrix, bShowPreview);
    }

    return S_OK;
}


// Sets parameters of the sun light
HRESULT CAdaptiveModelDX11Render::SetSunParams(const D3DXVECTOR3 &vDirectionOnSun,
                                               const D3DXCOLOR &vSunColor,
                                               const D3DXCOLOR &vAmbientLight)
{
    HRESULT hr;
    if( !m_pRenderEffect11 )
    {
        hr = CompileRenderPatchEffect(m_pd3dDevice11);
        CHECK_HR_RET(hr, _T("Failed to compile the effect"));
    }

    D3DXVECTOR4 vDirOnSun4(vDirectionOnSun.x, vDirectionOnSun.y, vDirectionOnSun.z, 1);
    m_RenderEffectVars.m_pevDirOnSun->SetFloatVector( (float*)&vDirectionOnSun.x) ;
    m_RenderEffectVars.m_pevSunColor->SetFloatVector( (float*)&vSunColor.r) ;
    m_RenderEffectVars.m_pevAmbientLight->SetFloatVector( (float*)&vAmbientLight.r) ;
    
    return S_OK;
}

void CAdaptiveModelDX11Render::EnableHeightMapMorph(bool bEnableMorph)
{
    if( bEnableMorph != m_RenderParams.m_bEnableHeightMapMorph )
    {
        m_RenderParams.m_bEnableHeightMapMorph = bEnableMorph;
        m_pRenderEffect11.Release();
    }
}


void CAdaptiveModelDX11Render::EnableNormalMapMorph(bool bEnableMorph)
{
    if( bEnableMorph != m_RenderParams.m_bEnableNormalMapMorph )
    {
        m_RenderParams.m_bEnableNormalMapMorph = bEnableMorph;
        m_pRenderEffect11.Release();
    }
}

void CAdaptiveModelDX11Render::SetWaterRSV( ID3D11ShaderResourceView* rsv )
{
	m_pPatchCommon->m_terrainCommonMat->water_normalmap_resourceSRV = rsv;
}
