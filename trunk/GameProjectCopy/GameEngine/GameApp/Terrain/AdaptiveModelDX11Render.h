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
#pragma once

#include "BlockBasedAdaptiveModel.h"
#include "d3dx11effect.h"
#include "HierarchyArray.h"


// This class renders the adaptive model using DX11 API
class CAdaptiveModelDX11Render : public CBlockBasedAdaptiveModel
{
public:
    enum TEXTURING_MODE
    {
        TM_HEIGHT_BASED = 0,
		TM_INDEX_MASK = 1,
		TM_INDEX_MASK_WITH_NM,
    };
    
    // Patch rendering params
    struct SRenderParams
    {
        TEXTURING_MODE m_TexturingMode;

        // Height map and normal map morph hides popping artifacts 
        // realted to LOD change
        bool m_bEnableHeightMapMorph; 
        bool m_bEnableNormalMapMorph;

        bool m_bAsyncModeWorkaround; // If this flag is true, then all DX resources are
                                     // created in main thread. Otherwise - in working threads
        bool m_bCompressNormalMap;  // Use BC3 compression for normal map
        
        int m_iNormalMapLODBias;

        SRenderParams();
    };

    CAdaptiveModelDX11Render(void);
    ~CAdaptiveModelDX11Render(void);

    // Initializes the object
    HRESULT Init(const SRenderingParams &Params,
                 const SRenderParams &RenderParams,
                 CElevationDataSource *pDataSource,
                 CTriangDataSource *pTriangDataSource);

    // Renders the model
	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext,
                   const D3DXVECTOR3 &vCameraPosition, 
                   const D3DXMATRIX &CameraViewProjMatrix,
                   bool bShowBoundingBoxes,
                   bool bShowPreview,
                   bool bShowWireframeModel,
                   bool bZOnlyPass);
	HRESULT Render(ID3D11DeviceContext* pd3dImmediateContext, const D3DXVECTOR3 &vCameraPosition, const D3DXMATRIX &CameraViewProjMatrix, bool bZOnlyPass);
    // Creates Direct3D11 device resources
    HRESULT OnD3D11CreateDevice( ID3D11Device* pd3dDevice,
                                 ID3D11DeviceContext* pd3dImmediateContext);

    // Releases Direct3D11 device resources
    void OnD3D11DestroyDevice( );

    // Creates swap chain-dependent resources
    HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                     const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext );
    
    // Releases swap chain-dependent resources
    void OnD3D11ReleasingSwapChain( void* pUserContext );

    // Enables or disables sorting patches by distance before rendering them
    void EnablePatchSorting(bool bEnablePatchSorting){m_bSortPatchesByDistance = bEnablePatchSorting;}

    // Enables or disables full resolution triangulation
    void EnableAdaptiveTriangulation(bool bEnableAdaptTriang){m_bEnableAdaptTriang = bEnableAdaptTriang;}

    // Sets parameters of the sun light
    HRESULT SetSunParams(const D3DXVECTOR3 &vDirectionOnSun,
                         const D3DXCOLOR &vSunColor,
                         const D3DXCOLOR &vAmbientLight);
    
    // Enables or disables normal map morphing in a pixel shader
    void EnableHeightMapMorph(bool bEnableMorph);
    
    // Enables or disables height map morphing in a pixel shader
    void EnableNormalMapMorph(bool bEnableMorph);

    // Renders small terrain map
	void RenderTerrainMap(const D3DXVECTOR4 &ScreenPos,
		     		      SPatchRenderingInfo pLevel1Patches[4]);

	void  ComputeTerrainBoundingBox(noVec3& vMin, noVec3& vMax);

	void UpdateTerrain(ID3D11DeviceContext* pd3dImmediateContext, const D3DXVECTOR3 &vCameraPosition, const D3DXMATRIX &CameraViewProjMatrix);

		
	noVec2 GetPatchCornerXY(CTerrainPatch *pDX11Patch);
	float GetPatchScale(CTerrainPatch *pDX11Patch);
	CTerrainPatch* GetCurrPatch();
	
	void SetWaterRSV(ID3D11ShaderResourceView* rsv);
	bool IsInit() const { return m_bInitialized; }

	CComPtr<ID3DX11Effect> m_pRenderEffect11;

private:
    // Creates a terrain patch
    virtual std::auto_ptr<CTerrainPatch> CreatePatch(class CPatchElevationData *pPatchElevData,
                                                     class CRQTTriangulation *pAdaptiveTriangulation)const;

    // Render all patches in the model
    int RenderPatches(const D3DXMATRIX &WorldViewProjMatr,
                      float fScreenSpaceTreshold,
                      bool bZOnlyPass,
                      bool bShowWireframe,
                      SPatchRenderingInfo *pPatchesToRender,
                      size_t NumPatchesToRender);
    
	struct BBoxInstance
	{
		D3DXVECTOR3 vMin;
		D3DXVECTOR3 vMax;
		unsigned long color;
	};

    // Checks if bounding box instance buffer is large enough and
    // creates the buffer, if necessary
    HRESULT CreateBoundBoxInstBuffer(size_t RequiredSize);

    // Renders bounding boxes for all visible patches
    void RenderBoundingBoxes(ID3D11DeviceContext* pd3dImmediateContext,
                             const D3DXMATRIX &CameraViewProjMatrix,
                             bool bShowPreview);

    // Renders bounding boxes for all visible patches
    void RenderQuadTreePreview(ID3D11DeviceContext* pd3dImmediateContext);

    // The method hierarchically traverses the tree and releases all D3D device resources
    void RecursiveDestroyD3D11PatchResources(CPatchQuadTreeNode &PatchNode);
    // The method hierarchically traverses the tree and create all D3D device resources
    void RecursiveCreateD3D11PatchResources(CPatchQuadTreeNode &PatchNode);
    // The method hierarchically traverses the tree and updates all device resources
    void RecursiveUpdateDeviceResources(CPatchQuadTreeNode &PatchNode);
    
    // The method compiles patch rendering effect
    HRESULT CompileRenderPatchEffect(ID3D11Device* pd3dDevice);

    // Creates an index buffer for rendering patches using full resolution triangulation
    HRESULT CreateFullResolutionStripBuffer(int iPatchSize);


    
    SRenderParams m_RenderParams;

    CComPtr<ID3D11Device> m_pd3dDevice11;
    CComPtr<ID3D11DeviceContext> m_pd3dDeviceContext;

    CComPtr<ID3D11ShaderResourceView> m_pElevationColorsSRV;


    // Rendering effect variables
    struct SRenderEffectVars
    {		
        ID3DX11EffectScalarVariable *m_pevPatchXYScale;
        ID3DX11EffectScalarVariable *m_pevFlangeWidth;
        ID3DX11EffectScalarVariable *m_pevMorphCoeff;
        ID3DX11EffectVectorVariable *m_pevPatchLBCornerXY;
        ID3DX11EffectVectorVariable *m_pevPatchOrderInSiblQuad;
        ID3DX11EffectShaderResourceVariable *m_pevElevationMap, *m_pevNormalMap;
        ID3DX11EffectShaderResourceVariable *m_pevParentElevMap, *m_pevParentNormalMap;
		ID3DX11EffectShaderResourceVariable *m_pPyramidMap, *m_pPyramidHeightMap, *m_pStackMap;

        ID3DX11EffectMatrixVariable *m_pevWorldViewProj;
        ID3DX11EffectTechnique *m_pevRenderPatch_FeatureLevel10Tech;
        ID3DX11EffectTechnique *m_pevRenderPatchInstanced_FL10Tech;
        ID3DX11EffectTechnique *m_pevRenderPatchTessellated_FL11Tech;
        ID3DX11EffectTechnique *m_pevRenderPatchTessellatedInstanced_FL11Tech;
		ID3DX11EffectTechnique *m_pevRenderHeightMapPreview_FL10;
		ID3DX11EffectVectorVariable *m_pevTerrainMapPos_PS;

        ID3DX11EffectRasterizerVariable *m_pevRS_SolidFill;
        ID3DX11EffectVectorVariable *m_pevDirOnSun;
        ID3DX11EffectVectorVariable *m_pevSunColor;
        ID3DX11EffectVectorVariable *m_pevAmbientLight;
        ID3DX11EffectScalarVariable *m_pevPatchTexArrayIndex;
        ID3DX11EffectScalarVariable *m_pevParentPatchTexArrayIndex;
        ID3DX11EffectScalarVariable *m_pevTileTextureScale;

        ID3DX11EffectTechnique *m_pRenderBoundBox_FeatureLevel10Tech;
		ID3DX11EffectTechnique *m_pRenderQuadTree_FeatureLevel10Tech;
		ID3DX11EffectTechnique *m_pRenderHeightMapPreview_FL10;
		ID3DX11EffectVectorVariable *m_pQuadTreePreviewPos_PS;
		ID3DX11EffectShaderResourceVariable *m_pElevDataTexture;
		ID3DX11EffectVectorVariable *m_pScreenPixelSize;

		ID3DX11EffectConstantBuffer* m_pLightConstBuffer;
				
    }m_RenderEffectVars;

	//class TextureD3D11*						m_pTextureIndexMap;
	//TextureD3D11*						m_pTextureArray;

    CComPtr<ID3D11Buffer> m_pFullResolutionIndBuffer; // Index buffer for full resolution patch triangulation
    UINT m_uiIndicesInFullResolutionStrip;

    // Rasterizer states for rendering solid and wireframe terrain models
    CComPtr<ID3D11RasterizerState> m_pRSSolidFill, m_pRSSolidFill_Biased;
    
    bool m_bEnableAdaptTriang;

    int m_iBackBufferWidth, m_iBackBufferHeight;

    // Instance buffer for rendering bounding boxes
    CComPtr<ID3D11Buffer> m_pBoundBoxInstBuffer;
    CComPtr<ID3D11InputLayout> m_pBoundBoxInputLayout;
    
    // Flag indicating if patches should be sorted by distance before rendering
    bool m_bSortPatchesByDistance;

	bool m_bInitialized;


private:
    CAdaptiveModelDX11Render(const CAdaptiveModelDX11Render&);
    CAdaptiveModelDX11Render& operator = (const CAdaptiveModelDX11Render&);
	void CreateTextures();
	CTerrainPatch* RecursiveGetPatch(CPatchQuadTreeNode &PatchNode, const float* ray, const float* dir);
	std::auto_ptr<CDX11PatchesCommon> m_pPatchCommon;
};
