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

#include "DynamicQuadTreeNode.h"

class CDX11PatchCache;

// Class implementing common data for all patches in the quad tree
class CDX11PatchesCommon
{
public:
    CDX11PatchesCommon(float fElevationSampleSpacing = 0.f,
                       float fElevationScale = 0.f,
                       int iNumLevelsInPatchHierarchy = 0,
					   bool bAsyncModeWorkaround = true,
                       bool bCompressNormalMap = false);
    ~CDX11PatchesCommon();

    // Creates Direct3D11 device resources
    HRESULT OnD3D11CreateDevice( ID3D11Device* pd3dDevice,
                                 ID3D11DeviceContext* pd3dImmediateContext,
                                 int iPatchSize );
    
    // Releases Direct3D11 device resources
    void OnD3D11DestroyDevice( );

	class SNBTerrainMaterial*	m_terrainCommonMat;
private:
    friend class CTerrainPatch;
    friend class CDX11TriangulatedPatch;
    
    std::auto_ptr<CDX11PatchCache> m_patchCache; // Resource cache
    CComPtr<ID3D11DeviceContext> m_pDeviceContext;
    CComPtr<ID3D11Device> m_pDevice;

    bool m_bCompressNormalMap;
    float m_fElevationSampleSpacing, m_fElevationScale;
    int m_iNumLevelsInPatchHierarchy;
	bool m_bAsyncModeWorkaround;


	
};

class CTerrainPatch
{
public:
    CTerrainPatch(const CDX11PatchesCommon *pPatchCommon,
                  const class CPatchElevationData *pPatchElevData,
                  class CRQTTriangulation *pAdaptiveTriangulation);

    ~CTerrainPatch();

    // Gets pointer to parent patch
    const CTerrainPatch* GetParent();
    // Gets pointers to children patches
    void GetChidlren(const CTerrainPatch* &ppLBChild, const CTerrainPatch* &ppRBChild, const CTerrainPatch* &ppLTChild, const CTerrainPatch* &ppRTChild);
    // Binds pointers to children patches
    void BindChildren(CTerrainPatch* pLBChild, CTerrainPatch* pRBChild, CTerrainPatch* pLTChild, CTerrainPatch* pRTChild);

    void SetElevMapValid(bool bElevMapIsValid){m_bElevMapIsValid = bElevMapIsValid;}
    bool IsElevMapValid() const { return m_bElevMapIsValid; }
	bool IsNormalMapValid() const { return m_bNormalMapIsValid; }
    float GetApproximationErrorBound() const{return m_fPatchApproxErrorBound;}

    // Uploads the data from system memory to D3D resources
	HRESULT UpdateDeviceResources();

    // Extension of the height map texture
    enum {ELEVATION_DATA_BOUNDARY_EXTENSION = 2};

    // Number of mip levels in a normal map
    enum {NORMAL_MAP_MIPS = 4};

    ID3D11ShaderResourceView* GetElevDataSRV()const;
    ID3D11RenderTargetView*   GetElevDataRTV()const;
    ID3D11ShaderResourceView* GetNormalMapSRV()const;
    ID3D11RenderTargetView*   GetNormalMapRTV()const;

	class SNBTerrainMaterial*	GetMat() const { return m_terrainMat; }

	class BBGrassPatch*&		GetGrass() { return m_pGrass; }		    

	const CPatchElevationData*	GetElevData() const { return m_pPatchElevData; }


	float GetHeight(const noVec2& pos);
protected:

    int m_iPatchSize;
    
    // Location in a quad tree
    SQuadTreeNodeLocation m_pos;

    // Patch world space approximation error bound
    float m_fPatchApproxErrorBound;
    
    // Pointer to patch elevation data
    const CPatchElevationData *m_pPatchElevData;

    // Pointer to patch common data
    const CDX11PatchesCommon *m_pPatchCommon;

	void SetupMaterial();

	float m_fFlangeWidth;
	float m_fMorphCoeff;
private:
    friend class CAdaptiveModelDX11Render;
    friend class CDX11PatchesCommon;

    HRESULT CreateElevDataTexture();
    void DefineElevDataTexDesc(D3D11_TEXTURE2D_DESC &ElevDataTexDesc);
    void DefineNormalMapDesc(D3D11_TEXTURE2D_DESC &NormalMapDesc);

    // Flag indicating if height map has already been loaded to GPU
    bool m_bElevMapIsValid;
    
    // Flag indicating if normal map has already been computed and loaded to GPU
    bool m_bNormalMapIsValid;

    CComPtr<ID3D11ShaderResourceView> m_ptex2DElevDataSRV;
    CComPtr<ID3D11RenderTargetView> m_ptex2DElevDataRTV;
    CComPtr<ID3D11ShaderResourceView> m_ptex2DNormalMapSRV;
    CComPtr<ID3D11RenderTargetView> m_ptex2DNormalMapRTV;

	CComPtr<ID3D11ShaderResourceView> m_ptex2DOpacityMapSRV;
	CComPtr<ID3D11RenderTargetView> m_ptex2DOpacityMapRTV;

    std::vector<BYTE> m_NormalMapData;// Normal map stores only x,y components (1 byte/component). 
                                      // z component is calculated in the shader as sqrt(1 - x^2 - y^2)
    std::vector<BYTE> m_NormalMapDataBC3; // Compressed normal map data

	std::vector<D3DXVECTOR3>	m_PathVertices;
   
    HRESULT CreateIndexBuffer();
	
    // Num indices in patch adaptive triangulation
    UINT m_uiNumIndicesInAdaptiveTriang;
    // Adaptive triangulation index buffer
    CComPtr<ID3D11Buffer> m_pIndexBuffer;
    // Adaptive triangulation indices.
    // Note that these are in fact packed quad tree vertex locations
	std::vector<UINT> m_Indices;

    // Not using CComPtr to avoid cyclic links
    CTerrainPatch *m_pParent;
    CTerrainPatch *m_pChild[4];
    int m_iHighResElevDataLODBias;

	SNBTerrainMaterial*	m_terrainMat;
	BBGrassPatch*	m_pGrass;

    CTerrainPatch();
    CTerrainPatch(const CTerrainPatch&); // no copy
    CTerrainPatch& operator = (const CTerrainPatch&);
};
