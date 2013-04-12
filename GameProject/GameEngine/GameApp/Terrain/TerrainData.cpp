///////////////////////////////////////////////////////////////////////  
//  TerrainData.cpp
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) CONFIDENTIAL AND PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization, Inc. and
//  may not be copied, disclosed, or exploited except in accordance with 
//  the terms of that agreement.
//
//      Copyright (c) 2003-2009 IDV, Inc.
//      All rights reserved in all media.
//
//      IDV, Inc.
//      http://www.idvinc.com


///////////////////////////////////////////////////////////////////////  
//  Preprocessor
#include "stdafx.h"
#include <cstdio>
#include "TerrainData.h"
#include "TGALoader.h"
#include "Utility.h"
#include "EngineCore/Speedtree/Timer.h"
#include "GameApp/GameApp.h"
#ifdef SPEEDTREE_OPENMP
    #include <omp.h>
#endif
using namespace SpeedTree;

#ifdef _XBOX
    #define SLASH "\\"
#else
    #define SLASH "/"
#endif


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::LoadTerrainData

st_bool CTerrainData::LoadTerrainData(const CFixedString& strDirectory)
{
    st_bool bReturn = false;
    Clear( );

    FILE* pFile = fopen((strDirectory + SLASH + "terrain.txt").c_str( ), "r");
    if (pFile != NULL)
    {
        char strTemp[1024];

        st_bool bContinue = true;
        while (bContinue && !feof(pFile))
        {
            if (fscanf(pFile, "%s", strTemp) > 0)
            {
                if (strcmp(strTemp, "area") == 0)
                {
                    const st_float32 c_fFeetPerKilometer = 3280.839895013f;

                    fscanf(pFile, "%g", &m_fSize);
                    if (m_fSize != 0.0f)
                        m_fSize = sqrt(m_fSize) * c_fFeetPerKilometer;
                }
                else if (strcmp(strTemp, "height_scale") == 0)
                    fscanf(pFile, "%g", &m_fHeightScale);
                else if (strcmp(strTemp, "normalmap_b_scale") == 0)
                    fscanf(pFile, "%g", &m_fNormalMapBScale);
                else if (strcmp(strTemp, "heightmap") == 0)
                {
                    fscanf(pFile, "%s", strTemp);
                    CFixedString strFilename(strTemp);
                    strFilename.erase_all('\"');

                    if (!LoadHeight((strDirectory + SLASH + strFilename).c_str( ), HM_ALL))
                    {
                        Warning("Terrain heightmap [%s] failed to load.\n", strFilename.c_str( ));
                        return false;
                    }
                    ComputeSlope(5.0f);
                    bReturn = true;
                }
                else if (strcmp(strTemp, "texture") == 0)
                {
                    fscanf(pFile, "%s", strTemp);
                    st_float32 fTemp = 0.0f;
                    fscanf(pFile, "%g", &fTemp);

                    if (m_aTextureFilenames.size( ) > 4)
                        Warning("Terrain currently only supports 5 textures.\n");
                    else
                    {
                        CFixedString strFilename(strTemp);
                        strFilename.erase_all('\"');
                        m_aTextureFilenames.push_back(strDirectory + SLASH + strFilename);
                        m_aTextureTiling.push_back(fTemp);
                    }
                }
                else if (strcmp(strTemp, "color") == 0)
                {
                    noVec4 vecColor; // alpha defaults to 1.0
                    fscanf(pFile, "%g%g%g", &vecColor.x, &vecColor.y, &vecColor.z);
                    m_aColors.push_back(vecColor);
                }
                else if (strcmp(strTemp, "ambient") == 0)
                {
                    fscanf(pFile, "%g%g%g%g", &m_sMaterial.m_vAmbient[0], &m_sMaterial.m_vAmbient[1], &m_sMaterial.m_vAmbient[2], &m_sMaterial.m_vAmbient[3]);
                }
                else if (strcmp(strTemp, "diffuse") == 0)
                {
                    fscanf(pFile, "%g%g%g%g", &m_sMaterial.m_vDiffuse[0], &m_sMaterial.m_vDiffuse[1], &m_sMaterial.m_vDiffuse[2], &m_sMaterial.m_vDiffuse[3]);
                }
                else if (strcmp(strTemp, "specular") == 0)
                {
                    fscanf(pFile, "%g%g%g%g", &m_sMaterial.m_vSpecular[0], &m_sMaterial.m_vSpecular[1], &m_sMaterial.m_vSpecular[2], &m_sMaterial.m_vSpecular[3]);
                }
                else if (strcmp(strTemp, "emissive") == 0)
                {
                    fscanf(pFile, "%g%g%g%g", &m_sMaterial.m_vEmissive[0], &m_sMaterial.m_vEmissive[1], &m_sMaterial.m_vEmissive[2], &m_sMaterial.m_vEmissive[3]);
                }
                else if (strcmp(strTemp, "shininess") == 0)
                {
                    fscanf(pFile, "%g", &m_sMaterial.m_fShininess);
                }
                else
                {
                    Warning("Unknown token [%s] in terrain data file.\n", strTemp);
                    bContinue = false;
                }
            }
        }

        fclose(pFile);
    }

    return bReturn;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::LoadHeight

st_bool CTerrainData::LoadHeight(const char* pFilename, st_int32 nChannels)
{
    st_bool bReturn = false;

    m_cHeightData.Clear( );
    m_fMinHeight = FLT_MAX;
    m_fMaxHeight = -FLT_MAX;

    CTGALoader cLoader;
    if (cLoader.Read(pFilename))
    {
        m_cHeightData.m_nWidth = cLoader.GetWidth( );
        m_cHeightData.m_nHeight = cLoader.GetHeight( );
        st_uint32 uiDepth = cLoader.GetDepth( );
        st_uchar* pImageData = cLoader.GetRawData( );

        if (uiDepth < 4)
            nChannels &= ~(HM_ALPHA);
        if (nChannels == 0)
            Error("CTerrainData::LoadHeight requires a 4 channel image (RGBA) when using HM_ALPHA");
        else
        {
            st_uint32 uiSize = m_cHeightData.m_nWidth * m_cHeightData.m_nHeight;
            m_cHeightData.m_pData = st_new_array<st_float32>(uiSize, "CTerrainData::CInterpolationData");
            st_float32* pData = m_cHeightData.m_pData;

            st_float32 fScalar = ((nChannels & HM_BLUE) ? 1.0f : 0.0f) + 
                                 ((nChannels & HM_GREEN) ? 1.0f : 0.0f) +  
                                 ((nChannels & HM_RED) ? 1.0f : 0.0f) + 
                                 ((nChannels & HM_ALPHA) ? 1.0f : 0.0f);
            fScalar = m_fSize * m_fHeightScale / (255.0f * fScalar);

            for (st_uint32 i = 0; i < uiSize; ++i)
            {
                *pData = 0.0f;

                // channels (BGRA)
                if (nChannels & HM_BLUE)
                    *pData += pImageData[0];
                if (nChannels & HM_GREEN)
                    *pData += pImageData[1];
                if (nChannels & HM_RED)
                    *pData += pImageData[2];
                if (nChannels & HM_ALPHA)
                    *pData += pImageData[3];

                *pData *= fScalar;

                if (*pData < m_fMinHeight)
                    m_fMinHeight = *pData;
                if (*pData > m_fMaxHeight)
                    m_fMaxHeight = *pData;

                ++pData;
                pImageData += uiDepth;
            }

            bReturn = true;
        }
    }

    return bReturn;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::LoadNormals

st_bool CTerrainData::LoadNormals(const char* pFilename)
{
    st_bool bReturn = false;

    m_cNormalData.Clear( );

    CTGALoader cLoader;
    if (cLoader.Read(pFilename))
    {
        st_uint32 uiDepth = cLoader.GetDepth( );    
        st_uchar* pImageData = cLoader.GetRawData( );
        m_cNormalData.m_nWidth = cLoader.GetWidth( );
        m_cNormalData.m_nHeight = cLoader.GetHeight( );

        st_uint32 uiSize = m_cNormalData.m_nWidth * m_cNormalData.m_nHeight;
        m_cNormalData.m_pData = st_new_array<noVec3>(uiSize, "CTerrainData::CInterpolationData");
        noVec3* pNormalData = m_cNormalData.m_pData;
        for (st_uint32 i = 0; i < uiSize; ++i)
        {
            pNormalData->Set(pImageData[2], pImageData[1], pImageData[0]);
            ++pNormalData;
            pImageData += uiDepth;
        }

        bReturn = true;
    }

    return bReturn;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::GenerateNormals

void CTerrainData::GenerateNormals(st_float32 fSmoothing)
{
    m_cNormalData.Clear( );

    if (m_cHeightData.m_nWidth > 0 && m_cHeightData.m_nHeight > 0 && m_cHeightData.m_pData != NULL)
    {
        st_int32 nWidth = m_cNormalData.m_nWidth = m_cHeightData.m_nWidth;
        st_int32 nHeight = m_cNormalData.m_nHeight = m_cHeightData.m_nHeight;
        m_cNormalData.m_pData = st_new_array<noVec3>(nWidth * nHeight, "CTerrainData::CInterpolationData");

        st_float32 fUSpacing = 2.0f * m_fSize / st_float32(nWidth);
        st_float32 fVSpacing = 2.0f * m_fSize / st_float32(nHeight);

        for (st_int32 i = 0; i < nWidth; ++i)
        {
            st_int32 iLeft = (i == 0 ? nWidth - 1 : i - 1);
            st_int32 iRight = (i == nWidth - 1 ? 0 : i + 1);

            for (st_int32 j = 0; j < nHeight; ++j)
            {
                st_int32 iTop = (j == 0 ? nHeight - 1 : j - 1);
                st_int32 iBottom = (j == nHeight - 1 ? 0 : j + 1);

                noVec3 vecOne(fUSpacing, 0.0f, m_cHeightData.m_pData[iRight + j * nWidth] - m_cHeightData.m_pData[iLeft + j * nWidth]);
                noVec3 vecTwo(0.0f, fVSpacing, m_cHeightData.m_pData[i + iBottom * nWidth] - m_cHeightData.m_pData[i + iTop * nWidth]);

                m_cNormalData.m_pData[i + j * nWidth] = vecOne.Cross(vecTwo);
                m_cNormalData.m_pData[i + j * nWidth].Normalize( );
            }
        }

        if (fSmoothing > 0.0f)
        {
            noVec3* pSmoothed = st_new_array<noVec3>(nWidth * nHeight, "CTerrainData::noVec3");

            st_int32 nSteps = st_int32(fSmoothing);
            st_float32 fLastInterpolation = fSmoothing - nSteps;
            ++nSteps;
            for (st_int32 nStep = 0; nStep < nSteps; ++nStep)
            {
                // compute smoothed normals
                for (st_int32 i = 0; i < nWidth; ++i)
                {
                    st_int32 iLeft = (i == 0 ? nWidth - 1 : i - 1);
                    st_int32 iRight = (i == nWidth - 1 ? 0 : i + 1);

                    for (st_int32 j = 0; j < nHeight; ++j)
                    {
                        st_int32 iTop = (j == 0 ? nHeight - 1 : j - 1);
                        st_int32 iBottom = (j == nHeight - 1 ? 0 : j + 1);

                        pSmoothed[i + j * nWidth] = (m_cNormalData.m_pData[iRight + j * nWidth] + 
                                                      m_cNormalData.m_pData[iLeft + j * nWidth] + 
                                                      m_cNormalData.m_pData[i + iTop * nWidth] + 
                                                      m_cNormalData.m_pData[i + iBottom * nWidth] +
                                                      m_cNormalData.m_pData[iRight + iTop * nWidth] +
                                                      m_cNormalData.m_pData[iRight + iBottom * nWidth] +
                                                      m_cNormalData.m_pData[iLeft + iTop * nWidth] +
                                                      m_cNormalData.m_pData[iLeft + iBottom * nWidth]);
                        pSmoothed[i + j * nWidth].Normalize( );
                    }
                }

                // interpolate or set
                if (nStep == nSteps - 1)
                {
                    // last step, interpolate
                    for (st_int32 i = 0; i < nWidth; ++i)
                        for (st_int32 j = 0; j < nHeight; ++j)
                        {
                            m_cNormalData.m_pData[i + j * nWidth] = Interpolate(m_cNormalData.m_pData[i + j * nWidth], pSmoothed[i + j * nWidth], fLastInterpolation);
                            m_cNormalData.m_pData[i + j * nWidth].Normalize( );
                        }
                }
                else
                {
                    // full smoothing step, copy everything
                    memcpy(m_cNormalData.m_pData, pSmoothed, nWidth * nHeight * sizeof(noVec3));
                }
            }

            st_delete_array<noVec3>(pSmoothed, "CTerrainData::noVec3");
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::ComputeSlope

void CTerrainData::ComputeSlope(st_float32 fSmoothing)
{
    m_cSlopeData.Clear( );

    if (m_cHeightData.m_nWidth > 0 && m_cHeightData.m_nHeight > 0 && m_cHeightData.m_pData != NULL)
    {
        st_int32 nWidth = m_cSlopeData.m_nWidth = m_cHeightData.m_nWidth;
        st_int32 nHeight = m_cSlopeData.m_nHeight = m_cHeightData.m_nHeight;
        m_cSlopeData.m_pData = st_new_array<st_float32>(nWidth * nHeight, "CTerrainData::CInterpolationData");

        st_float32 fUSpacing = m_fSize / (st_float32)nWidth;
        st_float32 fVSpacing = m_fSize / (st_float32)nHeight;

        for (st_int32 i = 0; i < nWidth; ++i)
        {
            st_int32 iLeft = (i == 0 ? nWidth - 1 : i - 1);
            st_int32 iRight = (i == nWidth - 1 ? 0 : i + 1);

            for (st_int32 j = 0; j < nHeight; ++j)
            {
                st_int32 iTop = (j == 0 ? nHeight - 1 : j - 1);
                st_int32 iBottom = (j == nHeight - 1 ? 0 : j + 1);

                st_float32 fSlope = 0.0f;
                st_float32 fThisHeight = m_cHeightData.m_pData[i + j * nWidth];
                fSlope += atan2(abs(fThisHeight - m_cHeightData.m_pData[iRight + j * nWidth]), fUSpacing);
                fSlope += atan2(abs(fThisHeight - m_cHeightData.m_pData[iLeft + j * nWidth]), fUSpacing);
                fSlope += atan2(abs(fThisHeight - m_cHeightData.m_pData[i + iTop * nWidth]), fVSpacing);
                fSlope += atan2(abs(fThisHeight - m_cHeightData.m_pData[i + iBottom * nWidth]), fVSpacing);

                fSlope *= (0.25f / noMath::HALF_PI);

                if (fSlope > 1.0f)
                    fSlope = 1.0f;
                m_cSlopeData.m_pData[i + j * nWidth] = fSlope;
            }
        }

        if (fSmoothing > 0.0f)
        {
            st_float32* pSmoothed = st_new_array<st_float32>(nWidth * nHeight, "CTerrainData::st_float32");

            st_int32 nSteps = st_int32(fSmoothing);
            st_float32 fLastInterpolation = fSmoothing - nSteps;
            ++nSteps;
            for (st_int32 nStep = 0; nStep < nSteps; ++nStep)
            {
                // compute smoothed normals
                for (st_int32 i = 0; i < nWidth; ++i)
                {
                    st_int32 iLeft = (i == 0 ? nWidth - 1 : i - 1);
                    st_int32 iRight = (i == nWidth - 1 ? 0 : i + 1);

                    for (st_int32 j = 0; j < nHeight; ++j)
                    {
                        st_int32 iTop = (j == 0 ? nHeight - 1 : j - 1);
                        st_int32 iBottom = (j == nHeight - 1 ? 0 : j + 1);

                        pSmoothed[i + j * nWidth] = (m_cSlopeData.m_pData[iRight + j * nWidth] + 
                                                      m_cSlopeData.m_pData[iLeft + j * nWidth] + 
                                                      m_cSlopeData.m_pData[i + iTop * nWidth] + 
                                                      m_cSlopeData.m_pData[i + iBottom * nWidth] +
                                                      m_cSlopeData.m_pData[iRight + iTop * nWidth] +
                                                      m_cSlopeData.m_pData[iRight + iBottom * nWidth] +
                                                      m_cSlopeData.m_pData[iLeft + iTop * nWidth] +
                                                      m_cSlopeData.m_pData[iLeft + iBottom * nWidth]);
                        pSmoothed[i + j * nWidth] *= 0.125f;
                    }
                }

                // interpolate or set
                if (nStep == nSteps - 1)
                {
                    // last step, interpolate
                    for (st_int32 i = 0; i < nWidth; ++i)
                        for (st_int32 j = 0; j < nHeight; ++j)
                            m_cSlopeData.m_pData[i + j * nWidth] = Interpolate(m_cSlopeData.m_pData[i + j * nWidth], pSmoothed[i + j * nWidth], fLastInterpolation);
                }
                else
                {
                    // full smoothing step, copy everything
                    memcpy(m_cSlopeData.m_pData, pSmoothed, nWidth * nHeight * sizeof(st_float32));
                }
            }

            st_delete_array<st_float32>(pSmoothed, "CTerrainData::st_float32");
        }
    }
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::ComputeAmbientOcclusion

void CTerrainData::ComputeAmbientOcclusion(const SUserSettings* pSettings, const CForest* pForest)
{
    m_cAOData.Clear( );
    if (pSettings->m_fTerrainAmbientOcclusion < 0.1f)
        return;

    CTimer cTimer;
    cTimer.Start( );

    // compute tree extents
    CExtents cTreeExtents;
    pForest->ComputeForestExtents(cTreeExtents);
    if (!cTreeExtents.Valid( ))
        return;

    // modify extents so it lines up with terrain cells
    *(const_cast<noVec3*>(&cTreeExtents.Min( ))) = CCoordSys::ConvertToStd((float*)&cTreeExtents.Min( ));
    *(const_cast<noVec3*>(&cTreeExtents.Max( ))) = CCoordSys::ConvertToStd((float*)&cTreeExtents.Max( ));
    cTreeExtents.Order( );  
    cTreeExtents[0] -= pSettings->m_fTerrainTreeAODistance;
    cTreeExtents[1] -= pSettings->m_fTerrainTreeAODistance;
    cTreeExtents[3] += pSettings->m_fTerrainTreeAODistance;
    cTreeExtents[4] += pSettings->m_fTerrainTreeAODistance;
    cTreeExtents[0] /= pSettings->m_fTerrainCellSize;
    cTreeExtents[1] /= pSettings->m_fTerrainCellSize;
    cTreeExtents[3] /= pSettings->m_fTerrainCellSize;
    cTreeExtents[4] /= pSettings->m_fTerrainCellSize;
    cTreeExtents[0] = floor(cTreeExtents[0]);
    cTreeExtents[1] = floor(cTreeExtents[1]);
    cTreeExtents[3] = ceil(cTreeExtents[3]);
    cTreeExtents[4] = ceil(cTreeExtents[4]);
    cTreeExtents[0] *= pSettings->m_fTerrainCellSize;
    cTreeExtents[1] *= pSettings->m_fTerrainCellSize;
    cTreeExtents[3] *= pSettings->m_fTerrainCellSize;
    cTreeExtents[4] *= pSettings->m_fTerrainCellSize;

    // set up dimming data
    m_afAOBounds[0] = cTreeExtents[0];
    m_afAOBounds[1] = cTreeExtents[1];
    m_afAOBounds[2] = cTreeExtents[3] - cTreeExtents[0];
    m_afAOBounds[3] = cTreeExtents[4] - cTreeExtents[1];
    m_cAOData.m_nWidth = st_int32(st_float32(pSettings->m_nTerrainRes) * m_afAOBounds[2] / pSettings->m_fTerrainCellSize);
    m_cAOData.m_nHeight = st_int32(st_float32(pSettings->m_nTerrainRes) * m_afAOBounds[3] / pSettings->m_fTerrainCellSize);
    m_cAOData.m_pData = st_new_array<st_uchar>(m_cAOData.m_nWidth * m_cAOData.m_nHeight, "CTerrainData::CInterpolationData");
    memset(m_cAOData.m_pData, 0xff, m_cAOData.m_nWidth * m_cAOData.m_nHeight);

    // set up precomputed constants
    const st_float32 fWidth = st_float32(m_cAOData.m_nWidth);
    const st_float32 fHeight = st_float32(m_cAOData.m_nHeight);
    const st_float32 fOneOverWidth = 1.0f / fWidth;
    const st_float32 fOneOverHeight = 1.0f / fHeight;
    const st_float32 fOneOverBoundsWidth = 1.0f / m_afAOBounds[2];
    const st_float32 fOneOverBoundsHeight = 1.0f / m_afAOBounds[3];

    // iterate through trees
    const TTreeCellMap& mapCells = pForest->GetTreeCellMap( );
    for (TTreeCellMap::const_iterator iCell = mapCells.begin( ); iCell != mapCells.end( ); ++iCell)
    {
        const CCellInstances& cCellInstances = iCell->second.GetCellInstances( );

        CCellBaseTreeItr iBaseTree = cCellInstances.FirstBaseTree( );
        while (iBaseTree.TreePtr( ))
        {
            const CTree* pBaseTree = iBaseTree.TreePtr( );

            noVec3 vDiagonal = pBaseTree->GetExtents( ).GetDiagonal( );
            vDiagonal.z = 0.0f;
            float fTreeGroundRadius = vDiagonal.Length( ) * 0.5f;

            st_int32 nNumInstances = 0;
            const CInstance* pInstances = cCellInstances.GetInstances(iBaseTree, nNumInstances);
            if (pInstances)
            {
                for (st_int32 nInstance = 0; nInstance < nNumInstances; ++nInstance)
                {
                    const CInstance* pInstance = pInstances + nInstance;

                    // splat underneath this tree
                    const noVec3 vPos = CCoordSys::ConvertToStd((float*)&pInstance->GetPos( ));
                    const st_int32 nU = st_int32(fWidth * (vPos.x - m_afAOBounds[0]) * fOneOverBoundsWidth + 0.5f);
                    const st_int32 nV = st_int32(fHeight * (vPos.y - m_afAOBounds[1]) * fOneOverBoundsHeight + 0.5f);

                    const float fTreeRadius = fTreeGroundRadius * pInstance->GetScale( );
                    float fDistanceScale = 1.0f / (fTreeRadius + pSettings->m_fTerrainTreeAODistance);
                    fDistanceScale *= fDistanceScale;
                    const st_int32 nXSteps = st_int32(fWidth * (pSettings->m_fTerrainTreeAODistance + fTreeRadius) * fOneOverBoundsWidth + 0.5f);
                    const st_int32 nYSteps = st_int32(fHeight * (pSettings->m_fTerrainTreeAODistance + fTreeRadius) * fOneOverBoundsHeight + 0.5f);

                    st_float32 fHeightInfluence = st_min(1.0f, (pInstance->GetScale( ) * pBaseTree->GetExtents( ).Max( )[2]) / (pSettings->m_fTerrainTreeAODistance + fTreeRadius));
                    for (st_int32 nX = nU - nXSteps; nX <= nU + nXSteps; ++nX)
                    {
                        for (st_int32 nY = nV - nYSteps; nY <= nV + nYSteps; ++nY)
                        {
                            const st_float32 fXDiff = m_afAOBounds[2] * st_float32(nX) * fOneOverWidth + m_afAOBounds[0] - vPos.x;
                            const st_float32 fYDiff = m_afAOBounds[3] * st_float32(nY) * fOneOverHeight + m_afAOBounds[1] - vPos.y;
                            st_float32 fDist = fXDiff * fXDiff + fYDiff * fYDiff;
                            fDist *= fDistanceScale;
                            if (fDist < 1.0f)
                            {
                                fDist = Interpolate(1.0f, fDist, fHeightInfluence);
                                st_uchar* pValue = &m_cAOData.m_pData[nX + nY * m_cAOData.m_nWidth];
                                *pValue = st_min(*pValue, st_uchar(fDist * 255.0f + 0.5f));
                            }
                        }
                    }
                }
            }

            ++iBaseTree;
        }
    }

    st_int32 nSize = m_cAOData.m_nWidth * m_cAOData.m_nHeight;
#ifdef SPEEDTREE_OPENMP
    // restrict the # of threads to no more than SPEEDTREE_OPENMP_MAX_THREADS
    int nAvailableThreads = omp_get_max_threads( );
    omp_set_num_threads(st_min(SPEEDTREE_OPENMP_MAX_THREADS, nAvailableThreads));
    #pragma omp parallel for
#endif
    for (st_int32 i = 0; i < nSize; ++i)
    {
        m_cAOData.m_pData[i] = Interpolate<st_uchar>(255, m_cAOData.m_pData[i], pSettings->m_fTerrainAmbientOcclusion);
    }

    // flip these for easy use in GetAmbientOcclusion()
    if (m_afAOBounds[2] != 0.0f)
        m_afAOBounds[2] = 1.0f / m_afAOBounds[2];
    if (m_afAOBounds[3] != 0.0f)
        m_afAOBounds[3] = 1.0f / m_afAOBounds[3];

    cTimer.Stop( );

    //printf("Terrain AO data uses %g Mb\n", m_cAOData.m_nWidth * m_cAOData.m_nHeight / 1024.0f / 1024.0f);
}
