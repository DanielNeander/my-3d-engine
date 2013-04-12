///////////////////////////////////////////////////////////////////////  
//  Forest.h
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
#include "EngineCore/SpeedTree/Core.h"
#include "EngineCore/SpeedTree/Map.h"
#include "EngineCore/SpeedTree/Set.h"
#include "EngineCore/SpeedTree/String.h"
#include "EngineCore/SpeedTree/Wind.h"


///////////////////////////////////////////////////////////////////////  
//  All SpeedTree SDK classes and variables are under the namespace "SpeedTree"

namespace SpeedTree
{

    ///////////////////////////////////////////////////////////////////////  
    //  Constants

    const int c_nMaxNumShadowMaps = 4;


    ///////////////////////////////////////////////////////////////////////  
    //  Class CInstance

    class ST_STORAGE_CLASS CInstance
    {
    public:
            friend class CCullEngine;

                                        CInstance( ) {}
            virtual                     ~CInstance( ) {}

            // geometry parameter settings
            //
            // these functions must be called on an instance before it is added
            // to the forest
            void                        SetPos(const noVec3& vPos);
            void                        SetScale(float fScale); 
            void                        SetRotation(float fRadians);

            // geometry parameter queries
            const noVec3&                 GetPos(void) const;
            float                  GetScale(void) const;
            float                  GetRotationAngle(void) const;   // in radians
            const float*           GetRotationVector(void) const;  // for optimized shader rotations

            // culling
            const noVec3&                 GetGeometricCenter(void) const;
            float                  GetCullingRadius(void) const;
            bool                     IsCulled(void) const;

            // operators
            bool                     operator<(const CInstance& cIn) const;
            bool                     operator==(const CInstance& cIn) const;

            // internal use
            void                        ComputeCullParameters(const CTree* pBase);

    private:
            noVec3                        m_vPos;                 // default to (0.0f, 0.0f, 0.0f)
            float                  m_fScale;               // 1.0 = no scale
            float                  m_fRotation;            // in radians

            noVec3                        m_vGeometricCenter;     // includes position offset
            float                  m_fCullingRadius;
            float                  m_afRotationVector[3];
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SCellKey

    struct SCellKey
    {
                            SCellKey(int nRow, int nCol) :
                                m_nRow(nRow),
                                m_nCol(nCol)
                            {
                            }

            bool         operator<(const SCellKey& sIn) const    { return (m_nRow == sIn.m_nRow) ? (m_nCol < sIn.m_nCol) : (m_nRow < sIn.m_nRow); }
            bool         operator!=(const SCellKey& sIn) const   { return (m_nRow != sIn.m_nRow || m_nCol != sIn.m_nCol); }

            void            Set(int nRow, int nCol)       { m_nRow = nRow; m_nCol = nCol; }

            int        m_nRow;
            int        m_nCol;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Type definitions

    typedef CArray<CInstance> TInstanceArray;
    typedef CMap<const CTree*, TInstanceArray> TBaseToInstanceMap;
    class CCellInstances;


    ///////////////////////////////////////////////////////////////////////  
    //  Class CCellBaseTreeItr

    class ST_STORAGE_CLASS CCellBaseTreeItr
    {
    public:
                                                        CCellBaseTreeItr( );
                                                        CCellBaseTreeItr(const CCellInstances* pCell, const CTree* pBaseTree);
#ifdef SPEEDTREE_FUTURE_EXPANSION
                                                        CCellBaseTreeItr(const CCellInstances* pCell, int nBaseTreeIndex);
#else
                                                        CCellBaseTreeItr(const CCellInstances* pCell, const TBaseToInstanceMap::const_iterator& iBaseTreeItr);
#endif
                                                        ~CCellBaseTreeItr( );

            const CTree*                                TreePtr(void) const;
#ifdef SPEEDTREE_FUTURE_EXPANSION
            int                                    TreeIndex(void) const;
#else
            const TBaseToInstanceMap::const_iterator&   TreeItr(void) const;
#endif

            CCellBaseTreeItr&                           operator++(void);

    private:
#ifdef SPEEDTREE_FUTURE_EXPANSION
            int                                    m_nBaseTreeItr;
#else
            TBaseToInstanceMap::const_iterator          m_iBaseTreeItr;
#endif

            const CCellInstances*                       m_pCell;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CCellInstances

    class ST_STORAGE_CLASS CCellInstances
#ifndef SPEEDTREE_FUTURE_EXPANSION
        : public TBaseToInstanceMap
#endif
    {
    public:
            friend class CCellBaseTreeItr;

                                CCellInstances( );
                                ~CCellInstances( );

            CCellBaseTreeItr    FirstBaseTree(void) const;

            // base tree management
            int            NumBaseTrees(void) const;

            // after complete, don't use these
            const CTree*        GetBaseTree(int nIndex) const;

            bool             AddBaseTree(const CTree* pBaseTree);
            bool             DeleteBaseTree(const CTree* pBaseTree);
            bool             IsBaseTreeInCell(const CTree* pBaseTree) const;

            // instance management
            int            NumInstances(const CTree* pBaseTree) const;
            bool             AddInstance(const CTree* pBaseTree, const CInstance& cInstance);
            const CInstance*    GetInstances(const CCellBaseTreeItr& iBaseTree, int& nNumInstances) const;
            bool             DeleteInstance(const CTree* pBaseTree, const CInstance& cInstance);
            bool             ClearInstances(const CTree* pBaseTree);

#ifdef SPEEDTREE_FUTURE_EXPANSION
            enum
            {
                MAX_NUM_BASE_TREES = 17,
                MAX_NUM_INSTANCES_PER_CELL = 296
            };

    private:
            int            GetBaseTreeIndex(const CTree* pBaseTree) const;

            // base trees
            int            m_nNumBaseTrees;
            const CTree*        m_apBaseTrees[MAX_NUM_BASE_TREES];
            int            m_anInstanceIndices[MAX_NUM_BASE_TREES];

            // instances
            int            m_anNumInstances[MAX_NUM_BASE_TREES];
            CInstance           m_acInstances[MAX_NUM_INSTANCES_PER_CELL];
            int            m_nTotalNumInstances;
#endif
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CCell

    class ST_STORAGE_CLASS CCell
    {
    public:
                                        CCell( );
            virtual                     ~CCell( );

            bool                     IsNew(void) const;

            // row/col attributes
            int                    Col(void) const;
            int                    Row(void) const;
            void                        SetRowCol(int nRow, int nCol);

            // extents
            const CExtents&             GetExtents(void) const;
            void                        InvalidateExtents(void);
    virtual void                        ComputeExtents(void) { }

            // culling
            const noVec3&                 GetCenter(void) const;
            float                  GetCullingRadius(void) const;

            // update index
            int                    GetUpdateIndex(void) const;
            void                        SetUpdateIndex(int nUpdateIndex);

    protected:
            int                    m_nRow;
            int                    m_nCol;

            int                    m_nUpdateIndex;

            // extents & culling
            CExtents                    m_cExtents;
            noVec3                        m_vCenter;
            float                  m_fCullRadius;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CTreeCell

    class ST_STORAGE_CLASS CTreeCell : public CCell
    {
    public:
                                        CTreeCell( );
            virtual                     ~CTreeCell( );

            // cell contents
            CCellInstances&             GetCellInstances(void);
            const CCellInstances&       GetCellInstances(void) const;

            // extents & culling
            void                        ComputeExtents(void);

            // billboard access
            bool                     AreBillboardsDirty(void) const;
            void                        SetBillboardsDirty(bool bFlag);

            // billboard-related
            struct SRenderInfo
            {
                int                m_nBillboardOffset;
                int                m_nNumBillboards;
            };
            typedef CMap<const CTree*, SRenderInfo> TBaseToRenderInfoMap;

            void*                       GetBillboardVbo(void) const;
            void                        SetBillboardVbo(void* pVbo);
            void                        SetRenderInfo(const CTree* pBaseTree, const SRenderInfo& sInfo);
            const SRenderInfo*          GetBillboardRenderInfo(const CTree* pBaseTree) const;

    private:
            CCellInstances              m_cCellInstances;
            bool                     m_bBillboardsDirty;

            // billboard-related
            void*                       m_pBillboardVbo;
            TBaseToRenderInfoMap        m_mBaseToRenderInfo;
    };



    ///////////////////////////////////////////////////////////////////////  
    //  Class CCellContainer

    template<class TCellType>
    class ST_STORAGE_CLASS CCellContainer : public CMap<SCellKey, TCellType>
    {
    public:
            // internal type definitions
            typedef typename CCellContainer::iterator TCellIterator;
            typedef typename CCellContainer::const_iterator TCellConstIterator;

                                        CCellContainer( );
            virtual                     ~CCellContainer( );

            TCellType*                  GetCellPtrByRowCol_Add(int nRow, int nCol);
            const TCellType*            GetCellPtrByRowCol(int nRow, int nCol) const;
            TCellType*                  GetCellPtrByPos_Add(const noVec3& vPos);
            const TCellType*            GetCellPtrByPos(const noVec3& vPos) const;

            // internal
            TCellIterator               GetCellItrByRowCol_Add(int nRow, int nCol);
            TCellConstIterator          GetCellItrByRowCol(int nRow, int nCol) const;
            TCellIterator               GetCellItrByPos_Add(const noVec3& vPos);
            TCellConstIterator          GetCellItrByPos(const noVec3& vPos) const;
            TCellIterator               Erase(TCellIterator iCell);

            // cell size
            float                  GetCellSize(void) const;
            void                        SetCellSize(float fCellSize);

    private:
            float                  m_fCellSize;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Struct SInstanceLod

    struct ST_STORAGE_CLASS SInstanceLod
    {
            bool                     operator<(const SInstanceLod& sRight) const
            {
                return m_fDistanceFromCameraSquared < sRight.m_fDistanceFromCameraSquared;
            }

            const CInstance*            m_pInstance;
            float                  m_fDistanceFromCameraSquared;
            float                  m_fLod;
            float                  m_afShaderLerps[GEOMETRY_TYPE_NUM_3D_TYPES];
            SLodSnapshot                m_sLodSnapshot;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Type definitions

    typedef CArray<const CTreeCell*>                TTreeCellArray;
    typedef CArray<CTreeCell*>                      TTreeCellArrayNonConst;
    typedef CArray<CTree*>                          TTreeArray;
    typedef CCellContainer<CTreeCell>               TTreeCellMap;
    typedef CArray<SInstanceLod>                    TInstanceLodArray;
    typedef CMap<const CTree*, TInstanceLodArray>   TBaseToInstanceLodMap;
    typedef CArray<void*>                           TBillboardVboArray;
    typedef CMap<void*, CTreeCell*>                 TBillboardVboOwners;

    
    ///////////////////////////////////////////////////////////////////////  
    //  Structure SForestCullResults

    struct ST_STORAGE_CLASS SForestCullResults
    {
            // 3D base trees & instances
            const TTreeArray*           m_pBaseTrees;
            TTreeCellArray              m_aVisibleCells;
            TBaseToInstanceLodMap       m_m3dInstances;

            // billboard/VBO management
            TBillboardVboArray          m_aFreedBillboardVbos;
            TTreeCellArrayNonConst      m_aCellsToPopulateWithBillboards;

            int                    Num3dInstances(const CTree* pBaseTree = NULL) const; // if null, return # of instances for all base trees
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Enumeration EFrustumPlanes

    enum EFrustumPlanes
    {
        NEAR_PLANE, FAR_PLANE, RIGHT_PLANE, LEFT_PLANE, BOTTOM_PLANE, TOP_PLANE, NUM_PLANES
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CView

    class ST_STORAGE_CLASS CView
    {
    public:

                                        CView( );

            // returns true if the values passed in our different from the internal values, false otherwise
            bool                     Set(const noVec3& vCameraPos,
                                            const noMat4& mProjection,
                                            const noMat4& mModelview,
                                            float fNearClip,
                                            float fFarClip);
            void                        SetLodRefPoint(const noVec3& vLodRefPoint) { m_vLodRefPoint = vLodRefPoint; }

            // get parameters set directly
            const noVec3&                 GetCameraPos(void) const;
            const noVec3&                 GetLodRefPoint(void) const;
            const noMat4&               GetProjection(void) const;
            const noMat4&               GetModelview(void) const;
            const noMat4&               GetModelviewNoTranslate(void) const;
            float                  GetNearClip(void) const;
            float                  GetFarClip(void) const;

            // get derived parameters
            const noVec3&                 GetCameraDir(void) const;
            const noMat4&               GetComposite(void) const;
            const noMat4&               GetCompositeNoTranslate(void) const;
            float                  GetCameraAzimuth(void) const;
            float                  GetCameraPitch(void) const;
            const noVec3*                 GetFrustumPoints(void) const;
            const noVec4*                 GetFrustumPlanes(void) const;
            const CExtents&             GetFrustumExtents(void) const;

            // get derived-by-request parameters
            const noMat4&               GetCameraFacingMatrix(void) const;

            // horizontal billboard support
            void                        SetHorzBillboardFadeAngles(float fStart, float fEnd); // in radians
            void                        GetHorzBillboardFadeAngles(float& fStart, float& fEnd) const; // in radians
            float                  GetHorzBillboardFadeValue(void) const; // 0.0 = horz bbs are transparent, 1.0 = horz bb's opaque

    private:
            void                        ComputeCameraFacingMatrix(void);
            void                        ComputeFrustumValues(void);
            void                        ExtractFrustumPlanes(void);

            // parameters are set directly
            noVec3                        m_vCameraPos;
            noVec3                        m_vLodRefPoint;
            bool                     m_bLodRefPointSet;
            noMat4                      m_mProjection;
            noMat4                      m_mModelview;
            float                  m_fNearClip;
            float                  m_fFarClip;

            // derived
            noVec3                        m_vCameraDir;
            noMat4                      m_mComposite;
            noMat4                      m_mModelviewNoTranslate;
            noMat4                      m_mCompositeNoTranslate;
            float                  m_fCameraAzimuth;
            float                  m_fCameraPitch;
            noVec3                        m_avFrustumPoints[8];
            noVec4                        m_avFrustumPlanes[NUM_PLANES];
            CExtents                    m_cFrustumExtents;

            // derived on request
            noMat4                      m_mCameraFacingMatrix;

            // horizontal billboards
            float                  m_fHorzFadeStartAngle;
            float                  m_fHorzFadeEndAngle;
            float                  m_fHorzFadeValue;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CForest

    class ST_STORAGE_CLASS CForest
    {
    public:
                                        CForest( );
    virtual                             ~CForest( );

            // base tree management
    virtual bool                     RegisterTree(CTree* pTree);
    virtual bool                     UnregisterTree(const CTree* pTree);
    virtual bool                     ReplaceTree(const CTree* pOldTree, CTree* pNewTree);
    virtual CTree*                      AllocateTree(void) const;
            bool                     TreeIsRegistered(const CTree* pTree) const;
            const TTreeArray&           GetBaseTrees(void) const;

            // tree instance management
            bool                     AddInstances(const CTree* pInstanceOf, const CInstance* pInstances, int nNumInstances);
            bool                     DeleteInstances(const CTree* pInstanceOf, const CInstance* pInstances, int nNumInstances, bool bDeleteCellIfEmpty = true);
            bool                     ClearInstances(const CTree* pInstanceOf = NULL, bool bDeleteCells = true); // when null, all are cleared
            bool                     ChangeInstance(const CTree* pTree, const CInstance& cOrigInstance, const CInstance& cNewInstance);
            int                    NumInstances(const CTree* pInstanceOf = NULL) const;
            bool                     GetInstances(const CTree* pInstanceOf, TInstanceArray& aInstances) const; // an expensive call

            // extents & limits
            int                    UpdateTreeCellExtents(void);
            void                        ComputeForestExtents(CExtents& cExtents) const {}

            // collision
            bool                     CollisionAdjust(noVec3& vPoint, const SForestCullResults& sResults);

            // culling
    virtual void                        CullAndComputeLOD(const CView& cView, SForestCullResults& sResults, bool bSort = false);
            void                        SetCullCellSize(float fCellSize);
            void                        FrameEnd(void);

            // wind management
            void                        EnableWind(bool bFlag);
            bool                     IsWindEnabled(void) const;
            void                        SetGlobalWindStrength(float fStrength);
            float                  GetGlobalWindStrength(void) const;
            void                        SetGlobalWindDirection(const noVec3& vDir);
            const noVec3&                 GetGlobalWindDirection(void) const;
            void                        AdvanceGlobalWind(void);
            void                        SetWindLeader(const CWind* pLeader);
            const CWind&                GetWindLeader(void) const;
            void                        SetGlobalTime(float fSeconds);
            float                  GetGlobalTime(void) const;

            // shadows & lighting support
            void                        SetLightDir(const noVec3& vLightDir);
            const noVec3&                 GetLightDir(void) const;
            bool                     LightDirChanged(void) const;
            bool                     ComputeLightView(const noVec3& vLightDir, const noVec3 avMainFrustum[8], int nMapIndex, CView& sLightView, float fRearExtension) const { return true; }
            void                        SetCascadedShadowMapDistances(const float afSplits[c_nMaxNumShadowMaps], float fFarClip); // each entry marks the end distance of its respective map
            const float*           GetCascadedShadowMapDistances(void) const;
            void                        SetShadowFadePercentage(float fFade);
            float                  GetShadowFadePercentage(void) const;

            // statistics
            struct SPopulationStats
            {
                                        SPopulationStats( );

                int                m_nNumCells;
                int                m_nNumBaseTrees;
                int                m_nNumInstances;
                float              m_fAverageNumInstancesPerBase;
                int                m_nMaxNumBillboardsPerCell; // same as m_nMaxNumInstancesPerCell
                int                m_nMaxNumInstancesPerCell;  // same as m_nMaxNumBillboardsPerCell
                int                m_nMaxNumInstancesPerCellPerBase;
                float              m_fAverageInstancesPerCell;
                int                m_nMaxNumBillboardImages;
            };
            void                        GetPopulationStats(SPopulationStats& sStats);
            
            // utility
            enum EHint
            {
                HINT_MAX_NUM_BILLBOARD_VBOS,
                HINT_MAX_NUM_BILLBOARDS_PER_CELL,
                HINT_MAX_NUM_COLLISION_TREES
            };

            void                        SetHint(EHint eHint, int nHintValue);

            // internal
            const TTreeCellMap&         GetTreeCellMap(void) const;

    protected:
            void                        RemoveDeletedCells(SForestCullResults& sResults);
            void                        ProcessInactiveCells(SForestCullResults& sResults) const;
            TTreeCellMap::iterator      DeleteTreeCell(const TTreeCellMap::iterator& iCell);

            // base trees
            TTreeArray                  m_aBaseTrees;
            bool                     m_bBaseTreesChanged;

            // instance storage & culling
            TTreeCellMap                m_cTreeCellMap;
            float                  m_fLargestInstanceCullRadius;
            TBillboardVboArray          m_aBillboardVbosFreedSinceLastCull;
            bool                     m_bCellVbosCleared;

            // billboards
            TBillboardVboOwners         m_mBillboardVboOwners;
            int                    m_nUpdateIndex;

            // wind
            bool                     m_bWindEnabled;
            float                  m_fGlobalWindStrength;
            noVec3                        m_vWindDir;
            float                  m_fGlobalTime;
            CWind                       m_cWindLeader;

            // shadows & lighting support
            noVec3                        m_vLightDir;
            bool                     m_bLightDirChanged;
            float                  m_afCascadedShadowMapSplits[c_nMaxNumShadowMaps + 1];
            float                  m_fShadowFadePercentage;

            // user-specified hints
            int                    m_nHintMaxNumBillboardVbos;
            int                    m_nHintMaxNumBillboardsPerCell;
            int                    m_nHintMaxNumCollisionTrees;

            // collision storage
            struct SCollisionSort
            {
                bool                 operator < (const SCollisionSort& sR) const { return (m_fDistance < sR.m_fDistance); }

                float              m_fDistance;
                const CTree*            m_pBaseTree;
                const CInstance*        m_pInstance;
            };
            CArray<SCollisionSort>      m_aCollisionTrees;
    };

    // include inline functions
    #include "Cells.inl"
    #include "Forest.inl"
    #include "CellContainer.inl"
    #include "Instance.inl"

} // end namespace SpeedTree

#include "EngineCore/SpeedTree/ExportEnd.h"
