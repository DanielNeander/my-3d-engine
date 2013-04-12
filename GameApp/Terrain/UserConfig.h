///////////////////////////////////////////////////////////////////////  
//  UserConfig.h
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

#pragma once
#include "GameApp/Terrain/Forest/Forest.h"


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    ///////////////////////////////////////////////////////////////////////  
    //  Structure SSpeciesPlacement

    struct SSpeciesPlacement
    {
                                    SSpeciesPlacement( ) :
                                        m_nQuantity(1)
                                    {
                                        m_afElevationRange[0] = -1e7f;
                                        m_afElevationRange[1] = 1e7f;
                                        m_afSlopeRange[0] = 0.0f;
                                        m_afSlopeRange[1] = 1.0f;
                                        m_afScaleRange[0] = 1.0f;
                                        m_afScaleRange[1] = 1.0f;
                                    }

        CFixedString                m_strFilename;
        st_float32                  m_afElevationRange[2];
        st_float32                  m_afSlopeRange[2];
        st_int32                    m_nQuantity;
        st_float32                  m_afScaleRange[2];
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SGrassPlacement

    struct SGrassPlacement
    {
                                    SGrassPlacement( ) :
                                        m_nIndex(-1),
                                        m_fFrequency(1.0f)
                                    {
                                        m_afElevationRange[0] = -1e7f;
                                        m_afElevationRange[1] = 1e7f;
                                        m_afSlopeRange[0] = 0.0f;
                                        m_afSlopeRange[1] = 1.0f;
                                        m_afWidthRange[0] = 5.5f;
                                        m_afWidthRange[1] = 10.0f;
                                        m_afHeightRange[0] = 3.0f;
                                        m_afHeightRange[1] = 5.0f;
                                    }

        st_int32                    m_nIndex;               // which texture
        st_float32                  m_fFrequency;           // 1.0 = normal
        st_float32                  m_afElevationRange[2];
        st_float32                  m_afSlopeRange[2];
        st_float32                  m_afWidthRange[2];
        st_float32                  m_afHeightRange[2];
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SUserSettings

    struct SUserSettings
    {
                                    SUserSettings( );

        // paths
        CArray<SSpeciesPlacement>   m_aSpecies;
        CArray<CFixedString>        m_aStfFiles;
        CFixedString                m_strExePath;
        CFixedString                m_strShaderLocation;
        CFixedString                m_strTerrainPath;
        CFixedString                m_strCameraSavePath;

        // lighting
        SMaterial                   m_sLightMaterial;
        noVec3                        m_vLightDir;
        st_float32                  m_fGlobalLightScalar;
        st_bool                     m_bSpecularLighting;
        st_bool                     m_bTransmissionLighting;
        st_bool                     m_bDetailLayer;
        st_bool                     m_bDetailNormalMapping;
        st_bool                     m_bAmbientContrast;

        // sun/sky
        noVec3                        m_vSkyColor;
        st_float32                  m_afSkyFogMinMax[2];
        noVec3                        m_vSunColor;
        noVec3                        m_vSunParameters;
        int                         m_anNumStars[2];

        // fog
        st_float32                  m_fFogStart;
        st_float32                  m_fFogEnd;
        noVec3                        m_vFogColor;

        // size
        st_float32                  m_fTreeAreaWidthInFeet;
        st_float32                  m_fVisibility;
        st_float32                  m_afRandomScaleRange[2];
        st_float32                  m_fLodScale;

        // display
        st_int32                    m_nWindowWidth;
        st_int32                    m_nWindowHeight;
        st_bool                     m_bFullscreen;
        st_bool                     m_bFullscreenResOverride;
        st_float32                  m_fFieldOfView;
        st_float32                  m_fNearClip;
        st_int32                    m_nSampleCount;
        st_int32                    m_nMaxAnisotropy;
        st_int32                    m_nShadowRes;    // zero means no shadow map
        st_float32                  m_fGamma;

        // wind
        st_bool                     m_bWindOff;
        st_bool                     m_bFrondRipple;

        // terrain
        st_float32                  m_fTerrainLightScalar;
        st_int32                    m_nTerrainRes;
        st_float32                  m_fTerrainFollowHeight;
        st_float32                  m_fTerrainAmbientOcclusion;
        st_float32                  m_fTerrainTreeAODistance;

        // shadows
        st_int32                    m_nNumShadowMaps;
        st_float32                  m_fShadowSplitScalar;
        st_float32                  m_fShadowFadePercent;
        st_bool                     m_bShowSplitsOnTerrain;
        st_float32                  m_afShadowSplits[c_nMaxNumShadowMaps];
        st_bool                     m_bSmoothShadows;
        st_int32                    m_nShadowUpdateFreq;

        // grass
        CFixedString                m_strGrassTexture;
        st_int32                    m_nNumGrassTextureCols;
        st_int32                    m_nNumGrassTextureRows;
        st_int32                    m_nGrassDensity; // number of overall blades per cell
        st_float32                  m_fGrassLightScalar;
        st_float32                  m_fGrassNormalVariance;
        st_float32                  m_fGrassStartFade;
        st_float32                  m_fGrassEndFade;
        CWind::SParams              m_sGrassWind;
        st_bool                     m_bGrassAlphaTesting;
        st_float32                  m_afGrassTerrainMatchFactors[2]; // [0] = top, [1] = bottom
        CArray<SGrassPlacement>     m_aGrassPlacements;
        st_float32                  m_fTotalPlacementWeight;

        // clouds
#ifdef SPEEDTREE_SIMUL_CLOUDS
        bool                        m_bUseClouds;
        CString                     m_strCloudPath;
#endif

        // rendering
        st_float32                  m_fAlphaTestScalar;
        st_bool                     m_bZPrePass;

        // cell sizes
        st_float32                  m_fGrassCellSize;
        st_float32                  m_fTerrainCellSize;
        st_float32                  m_fForestCellSize;

        // other
        st_bool                     m_bHorzBillboards;
        st_bool                     m_bVerbosePrint;
        st_bool                     m_bDiagnostic;
        st_float32                  m_fTreeCountScalar;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  class CCommandLineParser declaration

    class CCommandLineParser
    {
    public:
            st_bool     Parse(st_int32 argc, char* argv[], SUserSettings& sConfig);

    private:
            st_bool     ParseCommandLineFile(const char* pFilename, const char* pExeName, SUserSettings& sConfig);
    };

} // end namespace SpeedTree

