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

#include "TerrainPatch.h"
#include "ElevationDataSource.h"
#include "RQTTriangulation.h"
#include "PatchCache.h"
#include <gdiplus.h>
#include "GameApp/Util/EffectUtil.h"
#include "DXTCompressorDLL.h"
#include "SNBTerrainMaterial.h"
#include "GameApp/GameApp.h"
#include "AdaptiveModelDX11Render.h"
#include "GameApp/Objects/DecorationSystem.h"

template <class T>
static void PurgeVector(std::vector<T> &v)
{
	std::vector<T> empty;
	v.swap(empty);
}

template <typename PtrType>
PtrType *AlignPointer(PtrType *Ptr)
{
    return reinterpret_cast<PtrType*> ( (reinterpret_cast<size_t>(Ptr) + 15) & (-16) );
}



static float bilinear_interpolation(float fx, float fy, float a, float b, float c, float d)
{
	float s1,s2,s3,s4;
	s1=fx*fy;
	s2=(1-fx)*fy;
	s3=(1-fx)*(1-fy);
	s4=fx*(1-fy);
	return((a*s3+b*s4+c*s1+d*s2));
}

static void CalculateLayerDefMap(const UINT16 *pElevData,     // Height map data pointer
	size_t ElevDataPitch,    // Height map data pitch
	const BYTE *pNormalData, 
	size_t NormalPitch,
	int iNumChannels,
	int terrain_gridpoints,           		
	BYTE *temp_layerdef_map_texture_pixels        // Normal map data pointer	
	) {

		// Compute normal using discrete gradient approximation
		int i,j, ix, iz;
		float rm, mv;
		float x, z;
		for(i=0;i<terrain_layerdef_map_texture_size;i++)
			for(j=0;j<terrain_layerdef_map_texture_size;j++) {
				x=(float)(terrain_gridpoints)*((float)i/(float)terrain_layerdef_map_texture_size);
				z=(float)(terrain_gridpoints)*((float)j/(float)terrain_layerdef_map_texture_size);
				ix=(int)floor(x);
				iz=(int)floor(z);
				rm=bilinear_interpolation(x-ix,z-iz, pElevData[i + j * ElevDataPitch], 	pElevData[ i +1 + j * ElevDataPitch], 
					pElevData[i + (j + 1) * ElevDataPitch], pElevData[i + (j + 1)* ElevDataPitch]);
				
				
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=0;
				temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=0;

				if((rm>terrain_height_underwater_start)&&(rm<=terrain_height_underwater_end))
				{
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=255;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=0;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=0;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=0;
				}

				if((rm>terrain_height_sand_start)&&(rm<=terrain_height_sand_end))
				{
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=0;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=255;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=0;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=0;
				}

				if((rm>terrain_height_grass_start)&&(rm<=terrain_height_grass_end))
				{
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=0;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=0;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=255;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=0;
				}
				int idx;
				if (iNumChannels == 2)
					idx = 1;
				else if( iNumChannels == 4 )
					 idx = 3;				

				mv=bilinear_interpolation(x-ix,z-iz,pNormalData[i + j * NormalPitch+idx] ,
					pNormalData[i +1 + j * NormalPitch+idx], pNormalData[i+1 + (j+1)* NormalPitch+idx], 
					pNormalData[i + (j+1) * NormalPitch+idx]);

				if((mv<terrain_slope_grass_start)&&(rm>terrain_height_sand_end))
				{
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=0;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=0;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=0;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=0;
				}

				if((mv<terrain_slope_rocks_start)&&(rm>terrain_height_rocks_start))
				{
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=0;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=0;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=0;
					temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=255;
				}

			}
}

// This function generates normal map for the 16-bit height map
static void CalculateNormalMap(const UINT16 *pElevData,     // Height map data pointer
                               int iPatchSize,           
                               int iLeftBoundaryExtWidth,   // Height map extensions
                               int iBottomBoundaryExtWidth,
                               int iRightBoundaryExtWidth,
                               int iTopBoundaryExtWidth,
                               size_t ElevDataPitch,    // Height map data pitch
                               BYTE *pNormalMap,        // Normal map data pointer
                               size_t NormalMapPitch,   // Normal map data pitch
                               float fPatchSampleSpacingInterval, // Height map sample spacing interval length
                               float fElevationScale,   // Height map sample scale
                               int iNumChannels         // Num channels in normal map
                               )
{
    // Compute normal using discrete gradient approximation
    for(int iRow = -iBottomBoundaryExtWidth; iRow < iPatchSize + iTopBoundaryExtWidth; iRow++)
    {
        for(int iCol = -iLeftBoundaryExtWidth; iCol < iPatchSize + iRightBoundaryExtWidth; iCol++)
        {
            float fLeftNeighb = pElevData[max((iCol)-1, -iLeftBoundaryExtWidth) + (iRow) * ElevDataPitch];
            float fRightNeighb = pElevData[min((iCol)+1, iPatchSize+iRightBoundaryExtWidth-1) + (iRow) * ElevDataPitch];
            float fBottomNeighb = pElevData[(iCol) + max((iRow)-1, -iBottomBoundaryExtWidth) * ElevDataPitch];
            float fTopNeighb = pElevData[(iCol) + min((iRow)+1, iPatchSize+iTopBoundaryExtWidth-1) * ElevDataPitch];
            D3DXVECTOR3 Grad( (fLeftNeighb-fRightNeighb) * fElevationScale, (fBottomNeighb-fTopNeighb) * fElevationScale, fPatchSampleSpacingInterval*2.f );
            D3DXVECTOR3 vNormal;
            D3DXVec3Normalize(&vNormal, &Grad);
            BYTE Nx = (Min<BYTE>(Max<float>(127.f*vNormal.x+128.f, 0), 255));
            BYTE Ny = (Min<BYTE>(Max<float>(127.f*vNormal.y+128.f, 0), 255));
            BYTE *pComponents = &pNormalMap[( (iCol) + (iRow) * NormalMapPitch)*iNumChannels];
            // Uncompressed normals are stored in a two-channel texture
            // Compressed normal map is stored in a BC3_UNORM format: r,g,b channels store x 
            // component, while alpha channel stores y component:
            pComponents[0] = Nx;
            if( iNumChannels == 2 )
                pComponents[1] = Ny;
            else if( iNumChannels == 4 )
            {
                pComponents[1] = Nx;
                pComponents[2] = Nx;
                pComponents[3] = Ny;
            }             
        }
    }
}

// This function calculates coarset normal map levels by averaging normals
// at the finer resolution level
static void CalculateCoarseNormalMapMIP(const BYTE *pFineNormalMap,
                                        size_t FineNormalMapPitch,
                                        BYTE *pCoarseNormalMap,
                                        size_t CoarseNormalMapPitch,
                                        int iCoarseNormalMapWidth,
                                        int iCoarseNormalMapHeight,
                                        int iNumChannels)
{
    int iAlignedCoarseNormMapWidth  = (iCoarseNormalMapWidth + 3) & (-4);
    int iAlignedCoarseNormMapHeight = (iCoarseNormalMapHeight + 3) & (-4);
    for(int iRow = 0; iRow < iCoarseNormalMapHeight; iRow++)
    {
        for(int iCol = 0; iCol < iCoarseNormalMapWidth; iCol++)
        {
            // Load four normals at the finer resolution
            BYTE Nx[4], Ny[4];                   
            Nx[0] = (&pFineNormalMap[ (iCol*2     +  iRow*2    * FineNormalMapPitch)* iNumChannels]  ) [0];
            Ny[0] = (&pFineNormalMap[ (iCol*2     +  iRow*2    * FineNormalMapPitch)* iNumChannels]  ) [iNumChannels == 2 ? 1 : 3];
            Nx[1] = (&pFineNormalMap[ (iCol*2 + 1 +  iRow*2    * FineNormalMapPitch)* iNumChannels]  ) [0];
            Ny[1] = (&pFineNormalMap[ (iCol*2 + 1 +  iRow*2    * FineNormalMapPitch)* iNumChannels]  ) [iNumChannels == 2 ? 1 : 3];
            Nx[2] = (&pFineNormalMap[ (iCol*2     + (iRow*2+1) * FineNormalMapPitch)* iNumChannels]  ) [0];
            Ny[2] = (&pFineNormalMap[ (iCol*2     + (iRow*2+1) * FineNormalMapPitch)* iNumChannels]  ) [iNumChannels == 2 ? 1 : 3];
            Nx[3] = (&pFineNormalMap[ (iCol*2 + 1 + (iRow*2+1) * FineNormalMapPitch)* iNumChannels]  ) [0];
            Ny[3] = (&pFineNormalMap[ (iCol*2 + 1 + (iRow*2+1) * FineNormalMapPitch)* iNumChannels]  ) [iNumChannels == 2 ? 1 : 3];
            // Average normals                                                     
            D3DXVECTOR3 CoarseNormal( (float)(Nx[0]+Nx[1]+Nx[2]+Nx[3])/4.f/255.f, (float)(Ny[0]+Ny[1]+Ny[2]+Ny[3])/4.f/255.f, 0.f );
            BYTE *pCoarseNormalComponents = (BYTE *)&pCoarseNormalMap[ (iCol + iRow * CoarseNormalMapPitch) * iNumChannels ];
            pCoarseNormalComponents[0] = (BYTE)(Min<BYTE>(Max<float>(255.f*CoarseNormal.x, 0), 255));
            if( iNumChannels == 2 )
                pCoarseNormalComponents[1] = (BYTE)(Min<BYTE>(Max<float>(255.f*CoarseNormal.y, 0), 255));
            else if( iNumChannels == 4 )
            {
                pCoarseNormalComponents[1] = pCoarseNormalComponents[0];
                pCoarseNormalComponents[2] = pCoarseNormalComponents[0];
                pCoarseNormalComponents[3] = (BYTE)(Min<BYTE>(Max<float>(255.f*CoarseNormal.y, 0), 255));
            }
        }
        for(int iCol = iCoarseNormalMapWidth; iCol < iAlignedCoarseNormMapWidth; iCol++)
        {
            for(int iCh=0; iCh<iNumChannels; iCh++)
                pCoarseNormalMap[ (iCol + iRow * CoarseNormalMapPitch)*iNumChannels + iCh] = 
                    pCoarseNormalMap[ ((iCoarseNormalMapWidth-1) + iRow * CoarseNormalMapPitch)*iNumChannels + iCh ];
        }
    }

    for(int iRow = iCoarseNormalMapHeight; iRow < iAlignedCoarseNormMapHeight; iRow++)
    {
        memcpy(&pCoarseNormalMap[iRow * CoarseNormalMapPitch *iNumChannels ],
               &pCoarseNormalMap[(iCoarseNormalMapHeight-1) * CoarseNormalMapPitch *iNumChannels ],
               iAlignedCoarseNormMapWidth * sizeof(pCoarseNormalMap[0])*iNumChannels);
               
    }
}


///////////////////////////////////////////////////////////////////////////////

// This fucntion updates all mip levels of the texture
template <class T>
static void UpdateTextureData(ID3D11DeviceContext* pd3dDeviceContext,
                              ID3D11ShaderResourceView *srv, 
                              T *data)
{
	CComPtr<ID3D11Resource> resource;
	srv->GetResource(&resource);
	D3D11_TEXTURE2D_DESC desc;
	CComQIPtr<ID3D11Texture2D>(resource)->GetDesc(&desc);
	T *currDataPtr = &data[0];
	UINT currMipWidth = desc.Width;
	assert(desc.MipLevels > 0);
	for( UINT mip = 0; mip < desc.MipLevels; ++mip )
	{
		pd3dDeviceContext->UpdateSubresource(resource, mip, NULL, currDataPtr, desc.Width * sizeof(T), 0);
		currDataPtr += mip ? currMipWidth : (desc.Width * desc.Height);
		currMipWidth /= 2;
	}
}

const CTerrainPatch* CTerrainPatch :: GetParent()
{ 
    return m_pParent;
}

void CTerrainPatch :: GetChidlren(const CTerrainPatch* &pLBChild, const CTerrainPatch* &pRBChild, const CTerrainPatch* &pLTChild, const CTerrainPatch* &pRTChild)
{
    pLBChild = m_pChild[0];
    pRBChild = m_pChild[1];
    pLTChild = m_pChild[2];
    pRTChild = m_pChild[3];
}

void CTerrainPatch :: BindChildren(CTerrainPatch* pLBChild, CTerrainPatch* pRBChild, CTerrainPatch* pLTChild, CTerrainPatch* pRTChild)
{
    m_pChild[0] = pLBChild;
    m_pChild[1] = pRBChild; 
    m_pChild[2] = pLTChild;
    m_pChild[3] = pRTChild;
    for(int iChild=0; iChild<4; iChild++)
        if(m_pChild[iChild])
            m_pChild[iChild]->m_pParent = this;
}

void CTerrainPatch::DefineElevDataTexDesc(D3D11_TEXTURE2D_DESC &ElevDataTexDesc)
{
    ElevDataTexDesc.Width = m_iPatchSize + ELEVATION_DATA_BOUNDARY_EXTENSION*2;
    ElevDataTexDesc.Height = m_iPatchSize + ELEVATION_DATA_BOUNDARY_EXTENSION*2;
    ElevDataTexDesc.MipLevels = 1;
    ElevDataTexDesc.ArraySize = 1;
    ElevDataTexDesc.Format = DXGI_FORMAT_R16_UNORM;
    ElevDataTexDesc.SampleDesc.Count = 1;
    ElevDataTexDesc.SampleDesc.Quality = 0;
    ElevDataTexDesc.Usage = D3D11_USAGE_DEFAULT;
    ElevDataTexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    ElevDataTexDesc.CPUAccessFlags = 0;
    ElevDataTexDesc.MiscFlags = 0;
}

void CTerrainPatch::DefineNormalMapDesc(D3D11_TEXTURE2D_DESC &NormalMapDesc)
{
    DefineElevDataTexDesc(NormalMapDesc);
    NormalMapDesc.Width <<= m_iHighResElevDataLODBias;
    NormalMapDesc.Height <<= m_iHighResElevDataLODBias;
    NormalMapDesc.Format = m_pPatchCommon->m_bCompressNormalMap ? DXGI_FORMAT_BC3_UNORM : DXGI_FORMAT_R8G8_UNORM;
    NormalMapDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    NormalMapDesc.MipLevels = NORMAL_MAP_MIPS;
}

CTerrainPatch::CTerrainPatch(const CDX11PatchesCommon *pPatchesCommon,
                             const CPatchElevationData *pPatchElevData,
                             CRQTTriangulation *pAdaptiveTriangulation) : 
    m_pPatchCommon(pPatchesCommon),
    m_bNormalMapIsValid(false),
    m_bElevMapIsValid(false),
    m_fPatchApproxErrorBound(-1.f),
    m_uiNumIndicesInAdaptiveTriang(0),
    m_pPatchElevData(pPatchElevData),
    m_pParent(NULL),
	m_terrainMat(NULL)
{
    memset(m_pChild,0,sizeof(m_pChild));

    HRESULT hr;

    if( pPatchesCommon == NULL || pPatchElevData == NULL )
        return;

    // Get the patch size and location in the tree
    m_iPatchSize = pPatchElevData->GetPatchSize();
    pPatchElevData->GetPos(m_pos);
    // Store patch's approximation error bound
    m_fPatchApproxErrorBound = pPatchElevData->GetApproximationErrorBound() * m_pPatchCommon->m_fElevationScale;

    const UINT16 *pElevData = NULL;
    size_t ElevDataPitch = 0;
    // Get elevation data from the pPatchElevData interface
    m_pPatchElevData->GetDataPtr( pElevData, ElevDataPitch,
                                  ELEVATION_DATA_BOUNDARY_EXTENSION, 
                                  ELEVATION_DATA_BOUNDARY_EXTENSION,
                                  ELEVATION_DATA_BOUNDARY_EXTENSION,
                                  ELEVATION_DATA_BOUNDARY_EXTENSION );

    assert( pElevData != NULL );

	


    const UINT16 *pHighResElevData = NULL;
    size_t HighResElevDataPitch = 0;
    m_iHighResElevDataLODBias = m_pPatchElevData->GetHighResDataLODBias();
    m_pPatchElevData->GetHighResDataPtr( pHighResElevData, HighResElevDataPitch,
                                         ELEVATION_DATA_BOUNDARY_EXTENSION<<m_iHighResElevDataLODBias, 
                                         ELEVATION_DATA_BOUNDARY_EXTENSION<<m_iHighResElevDataLODBias,
                                         ELEVATION_DATA_BOUNDARY_EXTENSION<<m_iHighResElevDataLODBias,
                                         ELEVATION_DATA_BOUNDARY_EXTENSION<<m_iHighResElevDataLODBias );
    assert( pHighResElevData );

	//
	// elevation data
	//

    D3D11_TEXTURE2D_DESC ElevDataTexDesc;
    DefineElevDataTexDesc(ElevDataTexDesc);

	if( !m_pPatchCommon->m_bAsyncModeWorkaround )
	{
        V( CreateElevDataTexture() );
	}

    //
    // normal map
    //
    D3D11_TEXTURE2D_DESC NormalMapDesc;
    DefineNormalMapDesc(NormalMapDesc);

	if( !m_pPatchCommon->m_bAsyncModeWorkaround )
	{
		m_pPatchCommon->m_patchCache->GetPatchTexture(NormalMapDesc, &m_ptex2DNormalMapSRV, &m_ptex2DNormalMapRTV);
		assert(m_ptex2DNormalMapSRV);
	}

    
    // Uncompressed normals are stored in a two-channel texture
    // Compressed normal map is stored in a BC3_UNORM format: r,g,b channels store x 
    // component; a stores y component
    int iNumChannels = m_pPatchCommon->m_bCompressNormalMap ? 4 : 2;
    // Data alignement is necessary for the normal map compression
    int iNormMapDataSize = 0;
    UINT CurrMipHeight = NormalMapDesc.Height;
    UINT CurrMipWidth  = NormalMapDesc.Width;
    // Compute required space amount for all mip levels taking into account alignment and 
    for(int iNormalMapMip = 0; iNormalMapMip < NORMAL_MAP_MIPS; iNormalMapMip++, CurrMipHeight /= 2, CurrMipWidth /= 2)
        iNormMapDataSize += ((CurrMipHeight+3) & (-4)) * ((CurrMipWidth+3) & (-4));
        
    m_NormalMapData.resize( iNormMapDataSize * iNumChannels  + 15 ); // Reserve additional space for 16-byte alignement
    if( m_pPatchCommon->m_bCompressNormalMap )
        m_NormalMapDataBC3.resize( iNormMapDataSize + 15 ); // Reserve additional space for 16-byte alignement
        
    int iAlignedNormMapWidth = (NormalMapDesc.Width + 3) & (-4);

    float fHighResDataSpacingInterval = m_pPatchCommon->m_fElevationSampleSpacing * (float)(1<<(m_pPatchCommon->m_iNumLevelsInPatchHierarchy-1 - m_pos.level - m_iHighResElevDataLODBias ));
    assert( ((NormalMapDesc.Width & 0x03) == 0) && ((NormalMapDesc.Height & 0x03) == 0) );
    // Generate normal map
    CalculateNormalMap( pHighResElevData + (ELEVATION_DATA_BOUNDARY_EXTENSION<<m_iHighResElevDataLODBias) + (ELEVATION_DATA_BOUNDARY_EXTENSION<<m_iHighResElevDataLODBias)*HighResElevDataPitch,
                        m_iPatchSize<<m_iHighResElevDataLODBias,
                        ELEVATION_DATA_BOUNDARY_EXTENSION<<m_iHighResElevDataLODBias, 
                        ELEVATION_DATA_BOUNDARY_EXTENSION<<m_iHighResElevDataLODBias,
                        ELEVATION_DATA_BOUNDARY_EXTENSION<<m_iHighResElevDataLODBias,
                        ELEVATION_DATA_BOUNDARY_EXTENSION<<m_iHighResElevDataLODBias,
                        HighResElevDataPitch,
                        AlignPointer( &m_NormalMapData[0] ) + ( (ELEVATION_DATA_BOUNDARY_EXTENSION<<m_iHighResElevDataLODBias) + (ELEVATION_DATA_BOUNDARY_EXTENSION<<m_iHighResElevDataLODBias)*iAlignedNormMapWidth) * iNumChannels,
                        iAlignedNormMapWidth,
                        fHighResDataSpacingInterval,
                        m_pPatchCommon->m_fElevationScale,
                        iNumChannels);

    // Generate coarse normal map MIP levels
    CurrMipWidth  = NormalMapDesc.Width;
    CurrMipHeight = NormalMapDesc.Height;

    BYTE *pFinerMipLevel = NULL;
    int iFinerMipAlignedWidth = 0;
    BYTE *pCurrMipLevel = AlignPointer( &m_NormalMapData[0] );
    BYTE *pCurrCompressedMipLevel = m_pPatchCommon->m_bCompressNormalMap ? AlignPointer( &m_NormalMapDataBC3[0] ) : NULL;
    for(int iNormalMapMip = 0; iNormalMapMip < NORMAL_MAP_MIPS; iNormalMapMip++)
    {
        int iAlignedMipHeight = (CurrMipHeight + 3) & (-4);
        int iAlignedMipWidth  = (CurrMipWidth  + 3) & (-4);

        if( iNormalMapMip >= 1)
        {
            CalculateCoarseNormalMapMIP(pFinerMipLevel, iFinerMipAlignedWidth,
                                        pCurrMipLevel, iAlignedMipWidth,
                                        CurrMipWidth,
                                        CurrMipHeight,
                                        m_pPatchCommon->m_bCompressNormalMap ? 4 : 2);
        }

        if( m_pPatchCommon->m_bCompressNormalMap )
        {
            // Compress normal map if necessary
            DXTC::CompressImageDXT5SSE2( pCurrMipLevel, pCurrCompressedMipLevel, iAlignedMipWidth, iAlignedMipHeight );
            pCurrCompressedMipLevel += iAlignedMipWidth * iAlignedMipHeight;
        }

        pFinerMipLevel = pCurrMipLevel;
        pCurrMipLevel += iAlignedMipWidth * iAlignedMipHeight * iNumChannels;

        iFinerMipAlignedWidth = iAlignedMipWidth;
        CurrMipWidth/=2;
        CurrMipHeight/=2;
    }

	//if( !m_pPatchCommon->m_bAsyncModeWorkaround )
	{	
	
	}
        
    /*if( m_pPatchCommon->m_bCompressNormalMap )
    {
        PurgeVector( m_NormalMapData );
    }*/

	if( pAdaptiveTriangulation )
	{
		// Generate indices for adaptive triangulation
        // Reserve enough space to hold indices for the full resolution triangulation
        // Maximum number of vertices on each page edge is
        int iMaxVerticesOnEdge = m_iPatchSize+3;
        //
        //         +1                                  +1  +1
        //  left -> *   *   *   *   *   *   *   *   *   *   * <- Right (top) skirt
        //  (bottom)    |<------------------------->|   |
        //  skirt                 m_iPatchSize       Connection
        //                                           with right(top) neighbour    
        //
        // Thus maximum number of indices requred to fullfill iMaxVerticesOnEdge x iMaxVerticesOnEdge 
        // vertex grid is:
        size_t MaxIndices = (iMaxVerticesOnEdge-1) * (iMaxVerticesOnEdge-1) * 2 * 3;
		m_Indices.resize( MaxIndices );
		pAdaptiveTriangulation->GenerateIndices( ELEVATION_DATA_BOUNDARY_EXTENSION, &m_Indices[0], m_uiNumIndicesInAdaptiveTriang );

		char temp[256];
		sprintf(temp, "Active Triangle : %d\n", m_uiNumIndicesInAdaptiveTriang);
		OutputDebugStringA(temp);

		// Create adaptive triangulation of this patch
		if( !m_pPatchCommon->m_bAsyncModeWorkaround )
		{
			hr = CreateIndexBuffer();
			if( FAILED(hr) )return;
		}
	}

}

void CalculateDepthMap( const UINT16 *pElevData, /* Height map data pointer */ size_t ElevDataPitch, /* Height map data pitch */ int terrain_gridpoints, float terrain_geometry_scale, BYTE *depth_shadow_map_texture_pixels /* Normal map data pointer */ )
{

		// Compute normal using discrete gradient approximation

		int i,j, ix, iz;
		float rm, mv;
		float x, z;
		for(i=0;i<terrain_depth_shadow_map_texture_size;i++)
			for(j=0;j<terrain_depth_shadow_map_texture_size;j++) {
				x=(float)(terrain_gridpoints)*((float)i/(float)terrain_depth_shadow_map_texture_size);
				z=(float)(terrain_gridpoints)*((float)j/(float)terrain_depth_shadow_map_texture_size);
				ix=(int)floor(x);
				iz=(int)floor(z);
				rm=bilinear_interpolation(x-ix,z-iz, pElevData[i + j * ElevDataPitch], 	pElevData[ i +1 + j * ElevDataPitch], 
					pElevData[i + (j + 1) * ElevDataPitch], pElevData[i + (j + 1)* ElevDataPitch]) * terrain_geometry_scale;

				if(rm>0)
				{
					depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size+i)*4+0]=0;
					depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size+i)*4+1]=0;
					depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size+i)*4+2]=0;
				}
				else
				{
					float no=(1.0f*255.0f*(rm/(terrain_minheight*terrain_geometry_scale)))-1.0f;
					if(no>255) no=255;
					if(no<0) no=0;
					depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size+i)*4+0]=(byte)no;

					no=(10.0f*255.0f*(rm/(terrain_minheight*terrain_geometry_scale)))-40.0f;
					if(no>255) no=255;
					if(no<0) no=0;
					depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size+i)*4+1]=(byte)no;

					no=(100.0f*255.0f*(rm/(terrain_minheight*terrain_geometry_scale)))-300.0f;
					if(no>255) no=255;
					if(no<0) no=0;
					depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size+i)*4+2]=(byte)no;
				}				
				depth_shadow_map_texture_pixels[(j*terrain_depth_shadow_map_texture_size+i)*4+3]=0;
			}			
}


void CTerrainPatch::SetupMaterial() {
	if (m_terrainMat || m_NormalMapData.empty() || m_pPatchElevData == NULL) return;

	m_terrainMat = new SNBTerrainMaterial;	
	m_terrainMat->Init(m_pPatchCommon->m_terrainCommonMat->Effect);

	D3D11_TEXTURE2D_DESC NormalMapDesc;
	DefineNormalMapDesc(NormalMapDesc);
	int iAlignedNormMapWidth = (NormalMapDesc.Width + 3) & (-4);
	int iNumChannels = m_pPatchCommon->m_bCompressNormalMap ? 4 : 2;

	const UINT16 *pElevData = NULL;
	size_t ElevDataPitch = 0;		
	int i,j,k,l;
	m_pPatchElevData->GetDataPtr(pElevData, ElevDataPitch, 0, 0, 1, 1);


	byte* temp_layerdef_map_texture_pixels=(byte *)Mem_Alloc(terrain_layerdef_map_texture_size*terrain_layerdef_map_texture_size*4);
	byte* layerdef_map_texture_pixels=(byte *)Mem_Alloc(terrain_layerdef_map_texture_size*terrain_layerdef_map_texture_size*4);
	CalculateLayerDefMap(pElevData, ElevDataPitch, AlignPointer( &m_NormalMapData[0] ), iAlignedNormMapWidth, iNumChannels,
		m_iPatchSize, temp_layerdef_map_texture_pixels);

	for(i=0;i<terrain_layerdef_map_texture_size;i++)
		for(j=0;j<terrain_layerdef_map_texture_size;j++)  {
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0];
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1];
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2];
			layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=temp_layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3];
		}

		for(i=2;i<terrain_layerdef_map_texture_size-2;i++)
			for(j=2;j<terrain_layerdef_map_texture_size-2;j++) 				{
				int n1=0;
				int n2=0;
				int n3=0;
				int n4=0;
				for(k=-2;k<3;k++)
					for(l=-2;l<3;l++)
					{
						n1+=temp_layerdef_map_texture_pixels[((j+k)*terrain_layerdef_map_texture_size+i+l)*4+0];
						n2+=temp_layerdef_map_texture_pixels[((j+k)*terrain_layerdef_map_texture_size+i+l)*4+1];
						n3+=temp_layerdef_map_texture_pixels[((j+k)*terrain_layerdef_map_texture_size+i+l)*4+2];
						n4+=temp_layerdef_map_texture_pixels[((j+k)*terrain_layerdef_map_texture_size+i+l)*4+3];
					}
					layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+0]=(byte)(n1/25);
					layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+1]=(byte)(n2/25);
					layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+2]=(byte)(n3/25);
					layerdef_map_texture_pixels[(j*terrain_layerdef_map_texture_size+i)*4+3]=(byte)(n4/25);
			}


			// putting the generated data to textures

			HRESULT result;
			D3D11_SUBRESOURCE_DATA subresource_data;
			D3D11_TEXTURE2D_DESC tex_desc;
			D3D11_SHADER_RESOURCE_VIEW_DESC textureSRV_desc; 

			subresource_data.pSysMem = layerdef_map_texture_pixels;
			subresource_data.SysMemPitch = terrain_layerdef_map_texture_size*4;
			subresource_data.SysMemSlicePitch = 0;

			tex_desc.Width = terrain_layerdef_map_texture_size;
			tex_desc.Height = terrain_layerdef_map_texture_size;
			tex_desc.MipLevels = 1;
			tex_desc.ArraySize = 1;
			tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			tex_desc.SampleDesc.Count = 1; 
			tex_desc.SampleDesc.Quality = 0; 
			tex_desc.Usage = D3D11_USAGE_DEFAULT;
			tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			tex_desc.CPUAccessFlags = 0;
			tex_desc.MiscFlags = 0;


			m_terrainMat->CreateOpacityMap(layerdef_map_texture_pixels);																	

			Mem_Free(layerdef_map_texture_pixels);
			Mem_Free(temp_layerdef_map_texture_pixels);
			

#if 0
			extern CAdaptiveModelDX11Render g_TerrainDX11Render;
			noVec2 thisCornerXY = g_TerrainDX11Render.GetPatchCornerXY(this);
			idStr texName;			
			char path[64];
			texName.snPrintf(path, 64, "Opacity_%d_%d", (int)thisCornerXY.x, (int)thisCornerXY.y);
			texName+=path;
			texName.DefaultPath("Terrain/");
			texName.DefaultFileExtension(".dds");
			D3DX11SaveTextureToFile(D3D11Context(), m_terrainMat->layerdef_texture ,D3DX11_IFF_DDS, texName.c_str());
#endif
			//extern CAdaptiveModelDX11Render g_TerrainDX11Render;
			//float terrain_geometry_scale = g_TerrainDX11Render.GetPatchScale(this);
			// build depthmap			
			//byte * depth_shadow_map_texture_pixels=(byte *)Mem_Alloc(terrain_depth_shadow_map_texture_size*terrain_depth_shadow_map_texture_size*4);
			//CalculateDepthMap(pElevData, ElevDataPitch, m_iPatchSize, terrain_geometry_scale, depth_shadow_map_texture_pixels);			
			//Mem_Free(depth_shadow_map_texture_pixels);
				

			// Remove reference
			if( m_pPatchCommon->m_bCompressNormalMap )
			{
				PurgeVector( m_NormalMapData );
			}
			PurgeVector(m_NormalMapData);
			PurgeVector(m_NormalMapDataBC3);

			//if (m_bElevMapIsValid) m_pPatchElevData = NULL; // Elevation data is not needed anymore

}

// Creates index buffer for storing triangulation indices
HRESULT CTerrainPatch::CreateIndexBuffer()
{
    HRESULT hr;

    // Create index buffer
    D3D11_BUFFER_DESC IndexBufferDesc;
    ZeroMemory(&IndexBufferDesc, sizeof(IndexBufferDesc));
    IndexBufferDesc.Usage          = D3D11_USAGE_DEFAULT;
    IndexBufferDesc.ByteWidth      = sizeof( DWORD ) * m_uiNumIndicesInAdaptiveTriang;
    IndexBufferDesc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
    IndexBufferDesc.CPUAccessFlags = 0;
    IndexBufferDesc.MiscFlags      = 0;

    D3D11_SUBRESOURCE_DATA InitData = 
    {
        &m_Indices[0],
        0, //SysMemPitch - This member is used only for 2D and 3D texture resources; it is ignored for the other resource types
        0  // SysMemSlicePitch - This member is only used for 3D texture resources; it is ignored for the other resource types. 
    };

    hr = m_pPatchCommon->m_pDevice->CreateBuffer( &IndexBufferDesc, &InitData, &m_pIndexBuffer );
    CHECK_HR_RET(hr, _T("Failed to create adaptive triangulation index buffer") )
    
	PurgeVector(m_Indices);

    return S_OK;
}

CTerrainPatch::~CTerrainPatch()
{
	SAFE_DELETE(m_terrainMat);
    m_pPatchCommon->m_patchCache->ReleasePatchTexture(m_ptex2DElevDataSRV.Detach(), m_ptex2DElevDataRTV.Detach());
    m_pPatchCommon->m_patchCache->ReleasePatchTexture(m_ptex2DNormalMapSRV.Detach(), m_ptex2DNormalMapRTV.Detach());
}



HRESULT CTerrainPatch::CreateElevDataTexture()
{
    D3D11_TEXTURE2D_DESC ElevDataTexDesc;
    DefineElevDataTexDesc(ElevDataTexDesc);

    m_pPatchCommon->m_patchCache->GetPatchTexture(ElevDataTexDesc, &m_ptex2DElevDataSRV, &m_ptex2DElevDataRTV);
    assert(m_ptex2DElevDataSRV);
    return S_OK;
}

// Uploads the data from system memory to D3D resources
HRESULT CTerrainPatch::UpdateDeviceResources()
{
	D3D11_TEXTURE2D_DESC ElevDataTexDesc;
    DefineElevDataTexDesc(ElevDataTexDesc);


    if( !m_bElevMapIsValid )
    {
        // Update height map
        const UINT16 *pElevDataPtr = NULL;
        size_t ElevDataPitch = 0;
        m_pPatchElevData->GetDataPtr( pElevDataPtr, ElevDataPitch,
                                      ELEVATION_DATA_BOUNDARY_EXTENSION, 
                                      ELEVATION_DATA_BOUNDARY_EXTENSION,
                                      ELEVATION_DATA_BOUNDARY_EXTENSION,
                                      ELEVATION_DATA_BOUNDARY_EXTENSION );

        if( pElevDataPtr != NULL )
        {
            // In W/A mode we need to create texture from the main thread
			if( m_pPatchCommon->m_bAsyncModeWorkaround )
			{
                CreateElevDataTexture();
			}
			
            // Upload data to the texture
            CComPtr<ID3D11Resource> presElevData;
            m_ptex2DElevDataSRV->GetResource(&presElevData);
            m_pPatchCommon->m_pDeviceContext->UpdateSubresource(presElevData, 0, NULL, pElevDataPtr, (UINT) ElevDataPitch*sizeof(pElevDataPtr[0]), 0);
            
            m_bElevMapIsValid = true;
            //m_pPatchElevData = NULL; // Elevation data is not needed anymore
        }
    }


	// Update all normal map mip levels
    if( !m_bNormalMapIsValid  )
    {
        UINT CurrMipWidth  = 0;
        UINT CurrMipHeight = 0;

        CComPtr<ID3D11Resource> presNormalMap;
		if( m_pPatchCommon->m_bAsyncModeWorkaround )
		{
            D3D11_TEXTURE2D_DESC NormalMapDesc;
            DefineNormalMapDesc(NormalMapDesc);
			m_pPatchCommon->m_patchCache->GetPatchTexture(NormalMapDesc, &m_ptex2DNormalMapSRV, &m_ptex2DNormalMapRTV);
			assert(m_ptex2DNormalMapSRV);
		}

        m_ptex2DNormalMapSRV->GetResource(&presNormalMap);
        D3D11_TEXTURE2D_DESC NormMapDesc;
        CComQIPtr<ID3D11Texture2D>(presNormalMap)->GetDesc(&NormMapDesc);
	    CurrMipWidth = NormMapDesc.Width;
	    CurrMipHeight = NormMapDesc.Height;
        assert( NormMapDesc.MipLevels == NORMAL_MAP_MIPS );

        if(!m_NormalMapData.empty() || !m_NormalMapDataBC3.empty())
        {
            BYTE *pwCurrMipLevel = NULL;
            BYTE *pCurrCompressedMipLevel = NULL;
            if( m_pPatchCommon->m_bCompressNormalMap )
                pCurrCompressedMipLevel = AlignPointer( &m_NormalMapDataBC3[0] );
            else
                pwCurrMipLevel = AlignPointer( &m_NormalMapData[0] );
                        
            int iNumCahnnels = m_pPatchCommon->m_bCompressNormalMap ? 4 : 2;

            for(int iNormalMapMip = 0; iNormalMapMip < NORMAL_MAP_MIPS; iNormalMapMip++)
            {
                int iAlignedMipHeight = (CurrMipHeight + 3) & (-4);
                int iAlignedMipWidth  = (CurrMipWidth + 3) & (-4);

                const void *pSrcMipData = m_pPatchCommon->m_bCompressNormalMap ? (const void*)pCurrCompressedMipLevel : (const void*)pwCurrMipLevel;
                UINT SrcRowPitchInBytes = m_pPatchCommon->m_bCompressNormalMap ? iAlignedMipWidth *4 : iAlignedMipWidth*sizeof(m_NormalMapData[0])*iNumCahnnels;
                m_pPatchCommon->m_pDeviceContext->UpdateSubresource(presNormalMap, iNormalMapMip, NULL, 
                                                                       pSrcMipData, 
                                                                       SrcRowPitchInBytes, 0);

                if( m_pPatchCommon->m_bCompressNormalMap )
                    pCurrCompressedMipLevel += iAlignedMipWidth * iAlignedMipHeight;
                else
                    pwCurrMipLevel += iAlignedMipWidth * iAlignedMipHeight*iNumCahnnels;

                CurrMipWidth/=2;
                CurrMipHeight/=2;
            }
            
	        //PurgeVector(m_NormalMapData);
            //PurgeVector(m_NormalMapDataBC3);
        
		    m_bNormalMapIsValid = true;
        }
    }

    // Create index buffer, if necessary
	if( !m_Indices.empty() && m_pPatchCommon->m_bAsyncModeWorkaround )
	{
#if 0
		const UINT16 *pElevData = NULL;
		size_t ElevDataPitch = 0;
		// Get elevation data from the pPatchElevData interface
		m_pPatchElevData->GetDataPtr( pElevData, ElevDataPitch,
			ELEVATION_DATA_BOUNDARY_EXTENSION, 
			ELEVATION_DATA_BOUNDARY_EXTENSION,
			ELEVATION_DATA_BOUNDARY_EXTENSION,
			ELEVATION_DATA_BOUNDARY_EXTENSION );

		assert( pElevData != NULL );
		// Debug 
		// Indicies Unpack 
		// Triangle vertices
		struct SUnpackedVertex
		{
			int iXInd, iYInd;
		}UnpackedVertices[3] = { {INT_MIN, INT_MIN}, {INT_MIN, INT_MIN}, {INT_MIN, INT_MIN} };

		uint32 uiNumTriangles = m_uiNumIndicesInAdaptiveTriang / 3;

		extern CAdaptiveModelDX11Render g_TerrainDX11Render;
		noVec2 thisCornerXY = g_TerrainDX11Render.GetPatchCornerXY(this);
		float PatchScale =  g_TerrainDX11Render.GetPatchScale(this);
		m_PathVertices.resize(uiNumTriangles * 3);
		int vIdx = 0;
		for(UINT uiTriangleNum = 0; uiTriangleNum < uiNumTriangles; uiTriangleNum++)
		{
			UINT *puiTriangleVertPackedIndices = &m_Indices[0] + uiTriangleNum*3;
			
			bool bIsFlangeTriangle = false;
			for(int iVert=0; iVert < 3; iVert++)
			{
				UINT uiPackedVertInd = puiTriangleVertPackedIndices[iVert];
				int iXInd, iYInd;
				UnpackIndices(uiPackedVertInd, iXInd, iYInd, ELEVATION_DATA_BOUNDARY_EXTENSION);
				if( iXInd < 0 || iYInd < 0 || iXInd > m_iPatchSize || iYInd > m_iPatchSize )
				{
					bIsFlangeTriangle = true;
					break;
				}
			}
			
			// Ommit flange vertices
			if( bIsFlangeTriangle )
				continue;

				D3DXVECTOR3 TriangleVertices[3];
				for(int iVert=0; iVert < 3; iVert++)
				{
					UINT uiPackedVertInd = puiTriangleVertPackedIndices[iVert];
					int iXInd, iYInd;
					UnpackIndices(uiPackedVertInd, iXInd, iYInd, ELEVATION_DATA_BOUNDARY_EXTENSION);

					UnpackedVertices[iVert].iXInd = iXInd * PatchScale;
					UnpackedVertices[iVert].iYInd = iYInd * PatchScale;
					TriangleVertices[iVert] = D3DXVECTOR3( (float)iXInd, (float)iYInd, (float)pElevData[iXInd + iYInd*ElevDataPitch] );
					TriangleVertices[iVert].x += thisCornerXY.x;
					TriangleVertices[iVert].y += thisCornerXY.y;
					m_PathVertices[vIdx++] = TriangleVertices[iVert];					
				}			
			
		}
		
#endif	
		HRESULT hr;
		hr = CreateIndexBuffer();
		CHECK_HR_RET(hr, _T("Failed to create index buffer") );
	}

	if( m_pPatchCommon->m_bAsyncModeWorkaround )
	{
		SetupMaterial();

		/*extern DecorationSystem gDecoSys;
		extern CAdaptiveModelDX11Render g_TerrainDX11Render;
		noVec2 PatchXY = g_TerrainDX11Render.GetPatchCornerXY(this);		
		char name[128];
		idStr::snPrintf(name, 128, "Grass%f_%f", PatchXY.x, PatchXY.y);
		m_pGrass = gDecoSys.CreateNewGrass(name);*/
	}

	return S_OK;
}

ID3D11ShaderResourceView* CTerrainPatch::GetElevDataSRV()const
{
    return m_ptex2DElevDataSRV;
}

ID3D11RenderTargetView* CTerrainPatch::GetElevDataRTV()const
{
    return m_ptex2DElevDataRTV;
}

ID3D11ShaderResourceView* CTerrainPatch::GetNormalMapSRV()const
{
    return m_ptex2DNormalMapSRV;
}

ID3D11RenderTargetView* CTerrainPatch::GetNormalMapRTV()const
{
    return m_ptex2DNormalMapRTV;
}


float CTerrainPatch::GetHeight( const noVec2& pos )
{

	const UINT16 *pElevData = NULL;
	size_t ElevDataPitch = 0;			
	m_pPatchElevData->GetDataPtr(pElevData, ElevDataPitch, 0, 0, ELEVATION_DATA_BOUNDARY_EXTENSION, ELEVATION_DATA_BOUNDARY_EXTENSION);

	extern CAdaptiveModelDX11Render g_TerrainDX11Render;
	extern SRenderingParams g_TerrainRenderParams;

	noVec2 PatchCornerXY = g_TerrainDX11Render.GetPatchCornerXY(this);

	//SPatchBoundingBox terrainBB;
	//g_TerrainDX11Render.GetTerrainBoundingBox(terrainBB);

	//uint32 terrainWidth = terrainBB.fMaxX - terrainBB.fMinX;
	//uint32 terrainHeight = terrainBB.fMaxZ - terrainBB.fMinZ;
	float PatchScale = g_TerrainDX11Render.GetPatchScale(this);

	float fMapX = (pos.x - PatchCornerXY.x) / PatchScale + ELEVATION_DATA_BOUNDARY_EXTENSION;
	float fMapY = (pos.y - PatchCornerXY.y) / PatchScale + ELEVATION_DATA_BOUNDARY_EXTENSION;
	

	// Compute normal using discrete gradient approximation
	extern float g_fElevationScale;
	int i,j, ix, iz;
	float height, parentHeight;
	float x, z;
	int iMapX0, iMapY0;
	int iMapX1, iMapY1;
	
	x= fMapX;
	z= fMapY;
	ix=(int)floor(x);
	iz=(int)floor(z);	
	i = int(x - ix);
	j = int(z - iz);

	iMapX0 = noMath::ClampInt(0, m_iPatchSize + ELEVATION_DATA_BOUNDARY_EXTENSION*2, ix);
	iMapX1 = noMath::ClampInt(0, m_iPatchSize + ELEVATION_DATA_BOUNDARY_EXTENSION*2, ix+1);

	iMapY0 = noMath::ClampInt(0, m_iPatchSize + ELEVATION_DATA_BOUNDARY_EXTENSION*2, iz);
	iMapY1 = noMath::ClampInt(0, m_iPatchSize + ELEVATION_DATA_BOUNDARY_EXTENSION*2, iz+1);

	//rm=bilinear_interpolation(x-ix,z-iz, pElevData[iMapX0 + iMapY0 * ElevDataPitch], 	pElevData[ iMapX1 + iMapY0 * ElevDataPitch], 
	//	pElevData[iMapX0 + (iMapY1) * ElevDataPitch], pElevData[iMapX0 + (iMapY1)* ElevDataPitch]);
	height = pElevData[iMapX0 + iMapY0 * ElevDataPitch];

	const CTerrainPatch* pParent = this->GetParent();
	if (pParent) {
		const UINT16 *pElevData = NULL;
		size_t ElevDataPitch = 0;			
		pParent->GetElevData()->GetDataPtr(pElevData, ElevDataPitch, 0, 0, ELEVATION_DATA_BOUNDARY_EXTENSION, ELEVATION_DATA_BOUNDARY_EXTENSION);

		int PatchOrderInSiblQuad[] = {this->m_pos.horzOrder&0x01, this->m_pos.vertOrder&0x01, 0,0};

		iMapX0 = (iMapX0+ PatchOrderInSiblQuad[0]*m_iPatchSize)/2.f + ELEVATION_DATA_BOUNDARY_EXTENSION * 0.5f;
		iMapY0 = (iMapY0+ PatchOrderInSiblQuad[1]*m_iPatchSize)/2.f +  ELEVATION_DATA_BOUNDARY_EXTENSION * 0.5f;
		parentHeight = pElevData[iMapX0 + iMapY0 * ElevDataPitch];
		height = height + m_fMorphCoeff * (parentHeight - height);
	}
	else {
		int PatchOrderInSiblQuad[] = {this->m_pos.horzOrder&0x01, this->m_pos.vertOrder&0x01, 0,0};

		iMapX0 = (iMapX0+ PatchOrderInSiblQuad[0]*m_iPatchSize)/2.f;
		iMapY0 = (iMapY0+ PatchOrderInSiblQuad[1]*m_iPatchSize)/2.f;
		parentHeight = pElevData[iMapX0 + iMapY0 * ElevDataPitch];
		height = height + m_fMorphCoeff * (parentHeight - height);
	}

	height =height * g_fElevationScale;
	return height;
}



CDX11PatchesCommon::CDX11PatchesCommon(float fElevationSampleSpacing,
                                       float fElevationScale,
                                       int iNumLevelsInPatchHierarchy,
									   bool bAsyncModeWorkaround,
                                       bool bCompressNormalMap) : 
    m_bCompressNormalMap(bCompressNormalMap),
    m_fElevationSampleSpacing(fElevationSampleSpacing),
    m_fElevationScale(fElevationScale),
    m_iNumLevelsInPatchHierarchy(iNumLevelsInPatchHierarchy),
	m_bAsyncModeWorkaround(bAsyncModeWorkaround)
{
	m_terrainCommonMat = new SNBTerrainMaterial;

}

CDX11PatchesCommon::~CDX11PatchesCommon()
{
}

HRESULT CDX11PatchesCommon::OnD3D11CreateDevice( ID3D11Device* pd3dDevice,
                                                 ID3D11DeviceContext* pd3dImmediateContext,
                                                 int iPatchSize )
{
    m_patchCache.reset(new CDX11PatchCache(pd3dDevice));
	m_pDeviceContext = pd3dImmediateContext;
    m_pDevice = pd3dDevice;

    return S_OK;
}


void CDX11PatchesCommon::OnD3D11DestroyDevice()
{
    m_patchCache.reset();
    m_pDeviceContext.Release();
    m_pDevice.Release();
}
