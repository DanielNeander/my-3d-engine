///////////////////////////////////////////////////////////////////////  
//  Grass.h
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
//
//  *** Release version 5.0 ***


///////////////////////////////////////////////////////////////////////  
//  Preprocessor

#pragma once
#include "EngineCore/SpeedTree/ExportBegin.h"
#include "GameApp/Terrain/Forest/Forest.h"
#include "EngineCore/SpeedTree/Wind.h"


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    ///////////////////////////////////////////////////////////////////////  
    //  Class CGrassCell

    class ST_STORAGE_CLASS CGrassCell : public CCell
    {
    public:
            friend class CGrass;


                                        CGrassCell( );
            virtual                     ~CGrassCell( );

            // extents & culling
            void                        SetExtents(const CExtents& cExtents);
            float                  GetDistanceSquaredFromCamera(void) const;

            // graphics-related
            void*                       GetVbo(void) const;
            void                        SetVbo(void* pVbo);
            int                    GetNumBlades(void) const;
            void                        SetNumBlades(int nNumBlades);

    private:
            void*                       m_pVbo;
            int                    m_nNumBlades;
            float                  m_fDistanceSquaredFromCamera;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Type definitions

    typedef CArray<CGrassCell*>         TGrassCellArray;
    typedef CCellContainer<CGrassCell>  TGrassCellMap;
    typedef CArray<void*>               TGrassVboArray;


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SGrassCullResults

    struct ST_STORAGE_CLASS SGrassCullResults
    {
    public:
            TGrassCellArray             m_aCellsToUpdate;
            TGrassVboArray              m_aFreedVbos;
            TGrassCellArray             m_aVisibleCells;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CTerrain

    class ST_STORAGE_CLASS CGrass
    {
    public:
                                        CGrass( );
    virtual                             ~CGrass( );

    virtual bool                     Init(const char* pTextureFilename, int nImageRows, int nImageCols, float fCellSize);
            bool                     IsEnabled(void) const;
            float                  GetCellSize(void) const;
            void                        SetHeightHints(float fGlobalLowPoint, float fGlobalHighPoint);
            CWind&                      GetWind(void);
            const CWind&                GetWind(void) const;

            // LOD range
            void                        SetLodRange(float fStartFade, float fEndFade);
            void                        GetLodRange(float& fStartFade, float& fEndFade);

            // used during render loop
            void                        AdvanceWind(float fCurrentTime);
    virtual void                        Cull(const CView& cView, SGrassCullResults& sResults);

            // texture-related
            const char*                 GetTexture(void) const;
            int                    GetNumSubImages(void) const;
            const float*           GetBladeTexCoords(int nImageIndex, bool bMirror) const; // four values per blade (min_s, min_t, max_s, max_t)
            const unsigned char*             GetBladeTexCoordsUChar(int nImageIndex, bool bMirror) const; // four values per blade (min_s, min_t, max_s, max_t)

            // utility
            enum EHint
            {
                HINT_MAX_GRASS_BLADES_PER_CELL,
                HINT_MAX_NUM_ACTIVE_CELLS
            };

            void                        SetHint(EHint eHint, int nHintValue);
            int                    GetHint(EHint eHint) const;

    protected:
            void                        RemoveInactiveCells(TGrassVboArray& aFreedVbos);
            void                        BuildTexCoordTable(void);

            // LOD
            float                  m_fStartFade;           // distance from camera where grass blades begin to fade
            float                  m_fEndFade;             // distance from camera where grass blades are completely faded

            // wind
            CWind                       m_cWind;

            // cell data
            int                    m_nUpdateIndex;         // incremented each time CullAndComputeLOD() is called; used to remove cells that can be deleted
            TGrassCellMap               m_cGrassCellMap;        // maps the (row,col) search key to a grass cell
            float                  m_fGlobalLowPoint;      // used to initialize a cull cell's extents before its populated; should be lowest point grass will populate
            float                  m_fGlobalHighPoint;     // used to initialize a cull cell's extents before its populated; should be highest point grass will populate

            // texture data
            CString                     m_strTexture;           // texture filename holding the composite grass map
            int                    m_nNumImageCols;        // number of columns in the composite grass map (assumes evenly sized sub-images)
            int                    m_nNumImageRows;        // number of rows in the composite grass map (assumes evenly sized sub-images)
            CArray<float>          m_aBladeTexCoords;      // the texture coordinates for each sub-image based on the cols & rows
            CArray<unsigned char>            m_aBladeTexCoordsUChar; // the texture coordinates for each sub-image based on the cols & rows

            // user-specified hints
            int                    m_nHintMaxGrassBladesPerCell;
            int                    m_nHintMaxNumVbos;
    };

    // include inline functions
    #include "Grass.inl"

} // end namespace SpeedTree

#include "EngineCore/SpeedTree/ExportEnd.h"

