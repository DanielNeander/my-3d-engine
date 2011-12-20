///////////////////////////////////////////////////////////////////////
//  Cells.inl
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
//  CCell::CCell

inline CCell::CCell( ) :
    m_nRow(-99999),
    m_nCol(-99999),
    m_nUpdateIndex(-1),
    m_fCullRadius(-1.0f)
{
}


///////////////////////////////////////////////////////////////////////
//  CCell::~CCell

inline CCell::~CCell( )
{
#ifndef NDEBUG
    m_nRow = -99999;
    m_nCol = -99999;
    m_nUpdateIndex = -1;
    m_fCullRadius = -1.0f;
#endif
}


///////////////////////////////////////////////////////////////////////
//  CCell::IsNew

inline st_bool CCell::IsNew(void) const
{
    return !m_cExtents.Valid( );
}


///////////////////////////////////////////////////////////////////////
//  CCell::Col

inline st_int32 CCell::Col(void) const
{
    return m_nCol;
}


///////////////////////////////////////////////////////////////////////
//  CCell::Row

inline st_int32 CCell::Row(void) const
{
    return m_nRow;
}


///////////////////////////////////////////////////////////////////////
//  CCell::GetUpdateIndex

inline st_int32 CCell::GetUpdateIndex(void) const
{
    return m_nUpdateIndex;
}


///////////////////////////////////////////////////////////////////////
//  CCell::SetUpdateIndex

inline void CCell::SetUpdateIndex(st_int32 nUpdateIndex)
{
    m_nUpdateIndex = nUpdateIndex;
}


///////////////////////////////////////////////////////////////////////
//  CCell::InvalidateExtents

inline void CCell::InvalidateExtents(void)
{
    m_cExtents.Reset( );
}


///////////////////////////////////////////////////////////////////////
//  CCell::GetCenter

inline const Vec3& CCell::GetCenter(void) const
{
    return m_vCenter;
}


///////////////////////////////////////////////////////////////////////
//  CCell::GetCullingRadius

inline float CCell::GetCullingRadius(void) const
{
    return m_fCullRadius;
}


///////////////////////////////////////////////////////////////////////
//  CCell::GetExtents

inline const CExtents& CCell::GetExtents(void) const
{
    return m_cExtents;
}


///////////////////////////////////////////////////////////////////////
//  CCell::SetRowCol

inline void CCell::SetRowCol(int nRow, int nCol)
{
    m_nRow = nRow;
    m_nCol = nCol;
}


///////////////////////////////////////////////////////////////////////
//  CTreeCell::CTreeCell

inline CTreeCell::CTreeCell( ) :
    m_bBillboardsDirty(true),
    m_pBillboardVbo(NULL)
{
}


///////////////////////////////////////////////////////////////////////
//  CTreeCell::~CTreeCell

inline CTreeCell::~CTreeCell( )
{
    m_pBillboardVbo = NULL;
}


///////////////////////////////////////////////////////////////////////
//  CTreeCell::GetCellInstances

inline CCellInstances& CTreeCell::GetCellInstances(void)
{
    return m_cCellInstances;
}


///////////////////////////////////////////////////////////////////////
//  CTreeCell::GetCellInstances

inline const CCellInstances& CTreeCell::GetCellInstances(void) const
{
    return m_cCellInstances;
}


///////////////////////////////////////////////////////////////////////
//  CTreeCell::ComputeExtents

inline void CTreeCell::ComputeExtents(void)
{
    m_cExtents.Reset( );

    CCellBaseTreeItr iBaseTree = m_cCellInstances.FirstBaseTree( );
    while (iBaseTree.TreePtr( ))
    {
        st_int32 nNumInstances = 0;
        const CInstance* pInstances = m_cCellInstances.GetInstances(iBaseTree, nNumInstances);

        for (st_int32 i = 0; i < nNumInstances; ++i)
        {
            m_cExtents.ExpandAround(pInstances[i].GetGeometricCenter( ), pInstances[i].GetCullingRadius( ));

            // the cell extents could be made tighter but would result in forests taking
            // a little longer to build
        }

        ++iBaseTree;
    }

    m_vCenter = m_cExtents.GetCenter( );
    m_fCullRadius = m_cExtents.ComputeRadiusFromCenter3D( );
}


///////////////////////////////////////////////////////////////////////
//  CTreeCell::AreBillboardsDirty

inline st_bool CTreeCell::AreBillboardsDirty(void) const
{
    return m_bBillboardsDirty;
}


///////////////////////////////////////////////////////////////////////
//  CTreeCell::SetBillboardsDirty

inline void CTreeCell::SetBillboardsDirty(st_bool bFlag)
{
    m_bBillboardsDirty = bFlag;
}


///////////////////////////////////////////////////////////////////////
//  CTreeCell::GetBillboardVbo

inline void* CTreeCell::GetBillboardVbo(void) const
{
    return m_pBillboardVbo;
}


///////////////////////////////////////////////////////////////////////
//  CTreeCell::SetBillboardVbo

inline void CTreeCell::SetBillboardVbo(void* pVbo)
{
    m_pBillboardVbo = pVbo;
}


///////////////////////////////////////////////////////////////////////
//  CTreeCell::SetRenderInfo

inline void CTreeCell::SetRenderInfo(const CTree* pBaseTree, const SRenderInfo& sInfo)
{
    m_mBaseToRenderInfo[pBaseTree] = sInfo;
}


///////////////////////////////////////////////////////////////////////
//  CTreeCell::GetBillboardRenderInfo

inline const CTreeCell::SRenderInfo* CTreeCell::GetBillboardRenderInfo(const CTree* pBaseTree) const
{
    TBaseToRenderInfoMap::const_iterator iFind = m_mBaseToRenderInfo.find(pBaseTree);
    if (iFind != m_mBaseToRenderInfo.end( ))
        return &iFind->second;

    return NULL;
}

