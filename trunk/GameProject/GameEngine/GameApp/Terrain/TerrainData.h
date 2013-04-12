///////////////////////////////////////////////////////////////////////  
//  TerrainData.h
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
#include "EngineCore/Speedtree/Core.h"
#include "EngineCore/Speedtree/String.h"
#include "GameApp/Terrain/Forest/Forest.h"
#include "UserConfig.h"
#include "Utility.h"


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    ///////////////////////////////////////////////////////////////////////  
    //  Channels to use when loading

    #define HM_RED      (1 << 0)
    #define HM_GREEN    (1 << 1)
    #define HM_BLUE (1 << 2)
    #define HM_ALPHA    (1 << 3)
    #define HM_ALL      HM_RED | HM_GREEN | HM_BLUE | HM_ALPHA


    ///////////////////////////////////////////////////////////////////////  
    //  Class CTerrainData
    //
    //  This class serves as merely an example way to feed data into the
    //  terrain engine housed in the SpeedTree SDK. CTerrainData is not
    //  part of the SpeedTree SDK.  Your application is free to feed data 
    //  to the terrain engine in any way.
    //
    //  The ambient occlusion component is a good example of how this class
    //  is for more illustrative purposes.  It serves to demonstrate how
    //  effective darkening the terrain vertices can be in visually grounding
    //  billboards in the distance.  Our example implementation has not been
    //  optimized for memory or speed.  Normally these values would be baked
    //  in with a world builder instead of generated on the fly.

    class CTerrainData
    {
    public:
                                            CTerrainData( );
    virtual                                 ~CTerrainData( );           

            // clear all internal data
            void                            Clear(void);

            // load a terrain data directory. it must include a terrain.txt file
            st_bool                         LoadTerrainData(const CFixedString& strDirectory);
            st_bool                         IsLoaded(void) const;
            const CArray<CFixedString>&     GetTextureFilenames(void) const;
            const CArray<st_float32>&       GetTextureTiling(void) const;
            const CArray<noVec4>&             GetColors(void) const;

            // the uniform size of the area to which the heightmap will be applied
            // these values are used when generating normals and when converting (x, y) on height lookups
            void                            SetSize(st_float32 fSize)           { m_fSize = fSize; }
            st_float32                      GetSize(void) const                 { return m_fSize; }

            // the height scalar, useful when the heightmap and size it's applied to aren't uniformly scaled.
            // also can set/get the scale on the b channel of the normal map to increase/decrease roughness
            void                            SetHeightScale(st_float32 fHeightScale, st_float32 fNormalMapBScale);
            st_float32                      GetNormalMapBScale(void) const      { return m_fNormalMapBScale; }

            // loaded terrain material
            const SMaterial&                GetMaterial(void) const             { return m_sMaterial; }

            // load individual TGA texture maps
            st_bool                         LoadHeight(const char* pFilename, int iChannels = HM_ALPHA);
            st_bool                         LoadNormals(const char* pFilename); // assumes normal in RGB
            
            // generate normals for the currently loaded height data
            void                            GenerateNormals(st_float32 fSmoothing = 0.0f);

            // compute the slope for the currently loaded height data
            void                            ComputeSlope(st_float32 fSmoothing = 0.0f);

            // queries
            void                            GetHeightRange(st_float32& fMin, st_float32& fMax) const;
            st_float32                      GetHeight(st_float32 fXPos, st_float32 fYPos) const;
            noVec3                            GetNormal(st_float32 fXPos, st_float32 fYPos) const;
            st_float32                      GetSlope(st_float32 fXPos, st_float32 fYPos) const;
            st_float32                      GetSmoothHeight(st_float32 fXPos, st_float32 fYPos, st_float32 fDistance) const;
            void                            ReportParameters(noVec3 vPos, st_float32 fDistance);

            // tree query, returns false if placement fails placement parameters
            st_bool                         GetHeightForPlacementParameters(st_float32& fHeight, st_float32 fXPos, st_float32 fYPos, 
                                                                            st_float32 fMinHeight, st_float32 fMaxHeight,
                                                                            st_float32 fMinSlope = 0.0f, st_float32 fMaxSlope = 1.0f) const;

            // tree dimming
            void                            ComputeAmbientOcclusion(const SUserSettings* pSettings, const CForest* pForest);
            st_float32                      GetAmbientOcclusion(st_float32 x, st_float32 y) const;

    private:
            template<class T>
            class CInterpolationData
            {
            public:
                                            CInterpolationData( ) : 
                                                m_nWidth(0), 
                                                m_nHeight(0), 
                                                m_pData(NULL) 
                                            { 
                                            }

                void                        Clear(void);
                T                           NearestNeighbor(st_float32 fU, st_float32 fV) const;
                T                           BilinearInterpolation(st_float32 fU, st_float32 fV) const;
                T                           Smooth(st_float32 fU, st_float32 fV, st_float32 fDistance) const;
                st_bool                     IsPresent(void) const;

                st_int32                    m_nWidth;
                st_int32                    m_nHeight;
                T*                          m_pData;
            };

            st_float32                      m_fSize;
            st_float32                      m_fHeightScale;
            st_float32                      m_fNormalMapBScale;
            st_float32                      m_fMinHeight;
            st_float32                      m_fMaxHeight;
            st_float32                      m_afAOBounds[4];

            SMaterial                       m_sMaterial;
            CArray<CFixedString>            m_aTextureFilenames;
            CArray<st_float32>              m_aTextureTiling;
            CArray<noVec4>                    m_aColors;

            CInterpolationData<st_float32>  m_cHeightData;
            CInterpolationData<noVec3>        m_cNormalData;
            CInterpolationData<st_float32>  m_cSlopeData;
            CInterpolationData<st_uchar>    m_cAOData;
    };

    // include inline functions
    #include "TerrainData.inl"

} // end namespace SpeedTree


