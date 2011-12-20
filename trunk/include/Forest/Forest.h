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
#include "Core/ExportBegin.h"
#include "Core/Core.h"
#include "Core/Map.h"
#include "Core/Set.h"
#include "Core/String.h"
#include "Core/Wind.h"


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

                                        CInstance( );
            virtual                     ~CInstance( );

            // geometry parameter settings
            //
            // these functions must be called on an instance before it is added
            // to the forest
            void                        SetPos(const Vec3& vPos);
            void                        SetScale(st_float32 fScale); 
            void                        SetRotation(st_float32 fRadians);

            // geometry parameter queries
            const Vec3&                 GetPos(void) const;
            st_float32                  GetScale(void) const;
            st_float32                  GetRotationAngle(void) const;   // in radians
            const st_float32*           GetRotationVector(void) const;  // for optimized shader rotations

            // culling
            const Vec3&                 GetGeometricCenter(void) const;
            st_float32                  GetCullingRadius(void) const;
            st_bool                     IsCulled(void) const;

            // operators
            st_bool                     operator<(const CInstance& cIn) const;
            st_bool                     operator==(const CInstance& cIn) const;

            // internal use
            void                        ComputeCullParameters(const CTree* pBase);

    private:
            Vec3                        m_vPos;                 // default to (0.0f, 0.0f, 0.0f)
            st_float32                  m_fScale;               // 1.0 = no scale
            st_float32                  m_fRotation;            // in radians

            Vec3                        m_vGeometricCenter;     // includes position offset
            st_float32                  m_fCullingRadius;
            st_float32                  m_afRotationVector[3];
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Structure SCellKey

    struct SCellKey
    {
                            SCellKey(st_int32 nRow, st_int32 nCol) :
                                m_nRow(nRow),
                                m_nCol(nCol)
                            {
                            }

            st_bool         operator<(const SCellKey& sIn) const    { return (m_nRow == sIn.m_nRow) ? (m_nCol < sIn.m_nCol) : (m_nRow < sIn.m_nRow); }
            st_bool         operator!=(const SCellKey& sIn) const   { return (m_nRow != sIn.m_nRow || m_nCol != sIn.m_nCol); }

            void            Set(st_int32 nRow, st_int32 nCol)       { m_nRow = nRow; m_nCol = nCol; }

            st_int32        m_nRow;
            st_int32        m_nCol;
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
                                                        CCellBaseTreeItr(const CCellInstances* pCell, st_int32 nBaseTreeIndex);
#else
                                                        CCellBaseTreeItr(const CCellInstances* pCell, const TBaseToInstanceMap::const_iterator& iBaseTreeItr);
#endif
                                                        ~CCellBaseTreeItr( );

            const CTree*                                TreePtr(void) const;
#ifdef SPEEDTREE_FUTURE_EXPANSION
            st_int32                                    TreeIndex(void) const;
#else
            const TBaseToInstanceMap::const_iterator&   TreeItr(void) const;
#endif

            CCellBaseTreeItr&                           operator++(void);

    private:
#ifdef SPEEDTREE_FUTURE_EXPANSION
            st_int32                                    m_nBaseTreeItr;
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
            st_int32            NumBaseTrees(void) const;

            // after complete, don't use these
            const CTree*        GetBaseTree(st_int32 nIndex) const;

            st_bool             AddBaseTree(const CTree* pBaseTree);
            st_bool             DeleteBaseTree(const CTree* pBaseTree);
            st_bool             IsBaseTreeInCell(const CTree* pBaseTree) const;

            // instance management
            st_int32            NumInstances(const CTree* pBaseTree) const;
            st_bool             AddInstance(const CTree* pBaseTree, const CInstance& cInstance);
            const CInstance*    GetInstances(const CCellBaseTreeItr& iBaseTree, st_int32& nNumInstances) const;
            st_bool             DeleteInstance(const CTree* pBaseTree, const CInstance& cInstance);
            st_bool             ClearInstances(const CTree* pBaseTree);

#ifdef SPEEDTREE_FUTURE_EXPANSION
            enum
            {
                MAX_NUM_BASE_TREES = 17,
                MAX_NUM_INSTANCES_PER_CELL = 296
            };

    private:
            st_int32            GetBaseTreeIndex(const CTree* pBaseTree) const;

            // base trees
            st_int32            m_nNumBaseTrees;
            const CTree*        m_apBaseTrees[MAX_NUM_BASE_TREES];
            st_int32            m_anInstanceIndices[MAX_NUM_BASE_TREES];

            // instances
            st_int32            m_anNumInstances[MAX_NUM_BASE_TREES];
            CInstance           m_acInstances[MAX_NUM_INSTANCES_PER_CELL];
            st_int32            m_nTotalNumInstances;
#endif
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CCell

    class ST_STORAGE_CLASS CCell
    {
    public:
                                        CCell( );
            virtual                     ~CCell( );

            st_bool                     IsNew(void) const;

            // row/col attributes
            st_int32                    Col(void) const;
            st_int32                    Row(void) const;
            void                        SetRowCol(st_int32 nRow, st_int32 nCol);

            // extents
            const CExtents&             GetExtents(void) const;
            void                        InvalidateExtents(void);
    virtual void                        ComputeExtents(void) { }

            // culling
            const Vec3&                 GetCenter(void) const;
            st_float32                  GetCullingRadius(void) const;

            // update index
            st_int32                    GetUpdateIndex(void) const;
            void                        SetUpdateIndex(st_int32 nUpdateIndex);

    protected:
            st_int32                    m_nRow;
            st_int32                    m_nCol;

            st_int32                    m_nUpdateIndex;

            // extents & culling
            CExtents                    m_cExtents;
            Vec3                        m_vCenter;
            st_float32                  m_fCullRadius;
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
            st_bool                     AreBillboardsDirty(void) const;
            void                        SetBillboardsDirty(st_bool bFlag);

            // billboard-related
            struct SRenderInfo
            {
                st_int32                m_nBillboardOffset;
                st_int32                m_nNumBillboards;
            };
            typedef CMap<const CTree*, SRenderInfo> TBaseToRenderInfoMap;

            void*                       GetBillboardVbo(void) const;
            void                        SetBillboardVbo(void* pVbo);
            void                        SetRenderInfo(const CTree* pBaseTree, const SRenderInfo& sInfo);
            const SRenderInfo*          GetBillboardRenderInfo(const CTree* pBaseTree) const;

    private:
            CCellInstances              m_cCellInstances;
            st_bool                     m_bBillboardsDirty;

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

            TCellType*                  GetCellPtrByRowCol_Add(st_int32 nRow, st_int32 nCol);
            const TCellType*            GetCellPtrByRowCol(st_int32 nRow, st_int32 nCol) const;
            TCellType*                  GetCellPtrByPos_Add(const Vec3& vPos);
            const TCellType*            GetCellPtrByPos(const Vec3& vPos) const;

            // internal
            TCellIterator               GetCellItrByRowCol_Add(st_int32 nRow, st_int32 nCol);
            TCellConstIterator          GetCellItrByRowCol(st_int32 nRow, st_int32 nCol) const;
            TCellIterator               GetCellItrByPos_Add(const Vec3& vPos);
            TCellConstIterator          GetCellItrByPos(const Vec3& vPos) const;
            TCellIterator               Erase(TCellIterator iCell);

            // cell size
            st_float32                  GetCellSize(void) const;
            void                        SetCellSize(st_float32 fCellSize);

    private:
            st_float32                  m_fCellSize;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Struct SInstanceLod

    struct ST_STORAGE_CLASS SInstanceLod
    {
            st_bool                     operator<(const SInstanceLod& sRight) const
            {
                return m_fDistanceFromCameraSquared < sRight.m_fDistanceFromCameraSquared;
            }

            const CInstance*            m_pInstance;
            st_float32                  m_fDistanceFromCameraSquared;
            st_float32                  m_fLod;
            st_float32                  m_afShaderLerps[GEOMETRY_TYPE_NUM_3D_TYPES];
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

            st_int32                    Num3dInstances(const CTree* pBaseTree = NULL) const; // if null, return # of instances for all base trees
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
            st_bool                     Set(const Vec3& vCameraPos,
                                            const Mat4x4& mProjection,
                                            const Mat4x4& mModelview,
                                            st_float32 fNearClip,
                                            st_float32 fFarClip);
            void                        SetLodRefPoint(const Vec3& vLodRefPoint);

            // get parameters set directly
            const Vec3&                 GetCameraPos(void) const;
            const Vec3&                 GetLodRefPoint(void) const;
            const Mat4x4&               GetProjection(void) const;
            const Mat4x4&               GetModelview(void) const;
            const Mat4x4&               GetModelviewNoTranslate(void) const;
            st_float32                  GetNearClip(void) const;
            st_float32                  GetFarClip(void) const;

            // get derived parameters
            const Vec3&                 GetCameraDir(void) const;
            const Mat4x4&               GetComposite(void) const;
            const Mat4x4&               GetCompositeNoTranslate(void) const;
            st_float32                  GetCameraAzimuth(void) const;
            st_float32                  GetCameraPitch(void) const;
            const Vec3*                 GetFrustumPoints(void) const;
            const Vec4*                 GetFrustumPlanes(void) const;
            const CExtents&             GetFrustumExtents(void) const;

            // get derived-by-request parameters
            const Mat4x4&               GetCameraFacingMatrix(void) const;

            // horizontal billboard support
            void                        SetHorzBillboardFadeAngles(st_float32 fStart, st_float32 fEnd); // in radians
            void                        GetHorzBillboardFadeAngles(st_float32& fStart, st_float32& fEnd) const; // in radians
            st_float32                  GetHorzBillboardFadeValue(void) const; // 0.0 = horz bbs are transparent, 1.0 = horz bb's opaque

    private:
            void                        ComputeCameraFacingMatrix(void);
            void                        ComputeFrustumValues(void);
            void                        ExtractFrustumPlanes(void);

            // parameters are set directly
            Vec3                        m_vCameraPos;
            Vec3                        m_vLodRefPoint;
            st_bool                     m_bLodRefPointSet;
            Mat4x4                      m_mProjection;
            Mat4x4                      m_mModelview;
            st_float32                  m_fNearClip;
            st_float32                  m_fFarClip;

            // derived
            Vec3                        m_vCameraDir;
            Mat4x4                      m_mComposite;
            Mat4x4                      m_mModelviewNoTranslate;
            Mat4x4                      m_mCompositeNoTranslate;
            st_float32                  m_fCameraAzimuth;
            st_float32                  m_fCameraPitch;
            Vec3                        m_avFrustumPoints[8];
            Vec4                        m_avFrustumPlanes[NUM_PLANES];
            CExtents                    m_cFrustumExtents;

            // derived on request
            Mat4x4                      m_mCameraFacingMatrix;

            // horizontal billboards
            st_float32                  m_fHorzFadeStartAngle;
            st_float32                  m_fHorzFadeEndAngle;
            st_float32                  m_fHorzFadeValue;
    };


    ///////////////////////////////////////////////////////////////////////  
    //  Class CForest

    class ST_STORAGE_CLASS CForest
    {
    public:
                                        CForest( );
    virtual                             ~CForest( );

            // base tree management
    virtual st_bool                     RegisterTree(CTree* pTree);
    virtual st_bool                     UnregisterTree(const CTree* pTree);
    virtual st_bool                     ReplaceTree(const CTree* pOldTree, CTree* pNewTree);
    virtual CTree*                      AllocateTree(void) const;
            st_bool                     TreeIsRegistered(const CTree* pTree) const;
            const TTreeArray&           GetBaseTrees(void) const;

            // tree instance management
            st_bool                     AddInstances(const CTree* pInstanceOf, const CInstance* pInstances, st_int32 nNumInstances);
            st_bool                     DeleteInstances(const CTree* pInstanceOf, const CInstance* pInstances, st_int32 nNumInstances, st_bool bDeleteCellIfEmpty = true);
            st_bool                     ClearInstances(const CTree* pInstanceOf = NULL, st_bool bDeleteCells = true); // when null, all are cleared
            st_bool                     ChangeInstance(const CTree* pTree, const CInstance& cOrigInstance, const CInstance& cNewInstance);
            st_int32                    NumInstances(const CTree* pInstanceOf = NULL) const;
            st_bool                     GetInstances(const CTree* pInstanceOf, TInstanceArray& aInstances) const; // an expensive call

            // extents & limits
            st_int32                    UpdateTreeCellExtents(void);
            void                        ComputeForestExtents(CExtents& cExtents) const;

            // collision
            st_bool                     CollisionAdjust(Vec3& vPoint, const SForestCullResults& sResults);

            // culling
    virtual void                        CullAndComputeLOD(const CView& cView, SForestCullResults& sResults, st_bool bSort = false);
            void                        SetCullCellSize(st_float32 fCellSize);
            void                        FrameEnd(void);

            // wind management
            void                        EnableWind(st_bool bFlag);
            st_bool                     IsWindEnabled(void) const;
            void                        SetGlobalWindStrength(st_float32 fStrength);
            st_float32                  GetGlobalWindStrength(void) const;
            void                        SetGlobalWindDirection(const Vec3& vDir);
            const Vec3&                 GetGlobalWindDirection(void) const;
            void                        AdvanceGlobalWind(void);
            void                        SetWindLeader(const CWind* pLeader);
            const CWind&                GetWindLeader(void) const;
            void                        SetGlobalTime(st_float32 fSeconds);
            st_float32                  GetGlobalTime(void) const;

            // shadows & lighting support
            void                        SetLightDir(const Vec3& vLightDir);
            const Vec3&                 GetLightDir(void) const;
            st_bool                     LightDirChanged(void) const;
            st_bool                     ComputeLightView(const Vec3& vLightDir, const Vec3 avMainFrustum[8], st_int32 nMapIndex, CView& sLightView, st_float32 fRearExtension) const;
            void                        SetCascadedShadowMapDistances(const st_float32 afSplits[c_nMaxNumShadowMaps], st_float32 fFarClip); // each entry marks the end distance of its respective map
            const st_float32*           GetCascadedShadowMapDistances(void) const;
            void                        SetShadowFadePercentage(st_float32 fFade);
            st_float32                  GetShadowFadePercentage(void) const;

            // statistics
            struct SPopulationStats
            {
                                        SPopulationStats( );

                st_int32                m_nNumCells;
                st_int32                m_nNumBaseTrees;
                st_int32                m_nNumInstances;
                st_float32              m_fAverageNumInstancesPerBase;
                st_int32                m_nMaxNumBillboardsPerCell; // same as m_nMaxNumInstancesPerCell
                st_int32                m_nMaxNumInstancesPerCell;  // same as m_nMaxNumBillboardsPerCell
                st_int32                m_nMaxNumInstancesPerCellPerBase;
                st_float32              m_fAverageInstancesPerCell;
                st_int32                m_nMaxNumBillboardImages;
            };
            void                        GetPopulationStats(SPopulationStats& sStats);
            
            // utility
            enum EHint
            {
                HINT_MAX_NUM_BILLBOARD_VBOS,
                HINT_MAX_NUM_BILLBOARDS_PER_CELL,
                HINT_MAX_NUM_COLLISION_TREES
            };

            void                        SetHint(EHint eHint, st_int32 nHintValue);

            // internal
            const TTreeCellMap&         GetTreeCellMap(void) const;

    protected:
            void                        RemoveDeletedCells(SForestCullResults& sResults);
            void                        ProcessInactiveCells(SForestCullResults& sResults) const;
            TTreeCellMap::iterator      DeleteTreeCell(const TTreeCellMap::iterator& iCell);

            // base trees
            TTreeArray                  m_aBaseTrees;
            st_bool                     m_bBaseTreesChanged;

            // instance storage & culling
            TTreeCellMap                m_cTreeCellMap;
            st_float32                  m_fLargestInstanceCullRadius;
            TBillboardVboArray          m_aBillboardVbosFreedSinceLastCull;
            st_bool                     m_bCellVbosCleared;

            // billboards
            TBillboardVboOwners         m_mBillboardVboOwners;
            st_int32                    m_nUpdateIndex;

            // wind
            st_bool                     m_bWindEnabled;
            st_float32                  m_fGlobalWindStrength;
            Vec3                        m_vWindDir;
            st_float32                  m_fGlobalTime;
            CWind                       m_cWindLeader;

            // shadows & lighting support
            Vec3                        m_vLightDir;
            st_bool                     m_bLightDirChanged;
            st_float32                  m_afCascadedShadowMapSplits[c_nMaxNumShadowMaps + 1];
            st_float32                  m_fShadowFadePercentage;

            // user-specified hints
            st_int32                    m_nHintMaxNumBillboardVbos;
            st_int32                    m_nHintMaxNumBillboardsPerCell;
            st_int32                    m_nHintMaxNumCollisionTrees;

            // collision storage
            struct SCollisionSort
            {
                st_bool                 operator < (const SCollisionSort& sR) const { return (m_fDistance < sR.m_fDistance); }

                st_float32              m_fDistance;
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

#include "Core/ExportEnd.h"
