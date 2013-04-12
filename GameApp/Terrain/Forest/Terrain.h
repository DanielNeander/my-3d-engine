///////////////////////////////////////////////////////////////////////  
//  Terrain.h
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
#include "Forest.h"


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    ///////////////////////////////////////////////////////////////////////  
    //  Class CTerrainCell

    class ST_STORAGE_CLASS CTerrainCell : public CCell
    {
    public:
            friend class CTerrain;


                                        CTerrainCell( );
            virtual                     ~CTerrainCell( );

            // geometry
            ST_ILEX void                GetIndices(unsigned int& uiOffset, unsigned int& uiNumIndices) const;
            int                    GetLod(void) const;

            // extents & culling
            void                        SetExtents(const CExtents& cExtents);
            float                  GetDistanceFromCamera(void) const;

            // graphics-related
            void*                       GetVbo(void) const;
            void                        SetVbo(void* pVbo);

    private:
            unsigned int                   m_uiIndicesOffset;
            unsigned int                   m_uiNumIndices;
            float                  m_fDistanceFromCamera;
            int                    m_nLod;
            void*                       m_pVbo;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Type definitions

    typedef CArray<CTerrainCell*>           TTerrainCellArray;
    typedef CCellContainer<CTerrainCell>    TTerrainCellMap;
    typedef CArray<void*>                   TTerrainVboArray;


    ///////////////////////////////////////////////////////////////////////  
    //  Structure STerrainCullResults

    struct ST_STORAGE_CLASS STerrainCullResults
    {
            TTerrainCellArray           m_aCellsToUpdate;
            TTerrainVboArray            m_aFreedVbos;
            TTerrainCellArray           m_aVisibleCells;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CTerrain

    class ST_STORAGE_CLASS CTerrain
    {
    public:
                                        CTerrain( );
    virtual                             ~CTerrain( );

    virtual bool                     Init(int nNumLods, int nMaxTileRes, float fCellSize);
            bool                     IsEnabled(void) const;
            int                    GetNumLods(void) const;
            int                    GetMaxTileRes(void) const;
            float                  GetCellSize(void) const;
            void                        SetHeightHints(float fGlobalLowPoint, float fGlobalHighPoint);

            // LOD
            void                        SetLodRange(float fNear, float fFar);
            void                        GetLodRange(float& fNear, float& fFar) const;
            const CArray<unsigned int>&    GetCompositeIndices(void) const;

    virtual void                        CullAndComputeLOD(const CView& cView, STerrainCullResults& sCullResults);

            // utility
            enum EHint
            {
                HINT_MAX_ACTIVE_TERRAIN_CELLS
            };

            void                        SetHint(EHint eHint, int nHintValue);

    protected:
            void                        ComputeCellLods(const CView& cView, STerrainCullResults& sResults) const;
            void                        RemoveInactiveCells(TTerrainVboArray& aFreedVbos);
            void                        InitLodIndexStrips(void);

            // LOD
            int                    m_nNumLods;                 // the number of discrete LOD stages
            int                    m_nMaxTileRes;              // the highest LOD terrain tile will be a grid mesh of m_nMaxTileRes X m_nMaxTileRes
            float                  m_fNearLodDistance;         // at this distance, the highest LOD cell is used
            float                  m_fFarLodDistance;          // at this distance, the lowest LOD cell is used

            // cell data
            int                    m_nUpdateIndex;             // incremented each time CullAndComputeLOD() is called; used to remove cells that can be deleted
            TTerrainCellMap             m_cTerrainCellMap;          // maps the (row,col) search key to a terrain cell
            float                  m_fGlobalLowPoint;          //
            float                  m_fGlobalHighPoint;         //

            // indices used to control LOD of a given terrain tile
            CArray<unsigned int>           m_aMasterLodIndexStrip;     // contains all of the strips for all LOD/edge combinations
            CArray<CArray<unsigned int> >  m_aStripOffsetsByLod;       // [LOD][0-15] = offset into m_aMasterLodIndexStrip
            CArray<CArray<unsigned int> >  m_aStripLengthByLod;        // [LOD][0-15] = length of strip

            // user-specified hints
            int                    m_nHintMaxActiveTerrainCells;
    };

    // include inline functions
    #include "Terrain.inl"

} // end namespace SpeedTree


#include "EngineCore/SpeedTree/ExportEnd.h"

