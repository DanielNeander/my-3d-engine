///////////////////////////////////////////////////////////////////////
//  Terrain.inl
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
//  CTerrainCell::CTerrainCell

inline CTerrainCell::CTerrainCell( ) :
    m_uiIndicesOffset(0),
    m_uiNumIndices(0),
    m_fDistanceFromCamera(0.0f),
    m_nLod(-1),
    m_pVbo(NULL)
{
    assert(IsNew( ));
}


///////////////////////////////////////////////////////////////////////
//  CTerrainCell::~CTerrainCell

inline CTerrainCell::~CTerrainCell( )
{
#ifndef NDEBUG
    m_uiIndicesOffset = 0;
    m_uiNumIndices = 0;
    m_fDistanceFromCamera = 1.0f;
    m_nLod = -1;
    m_pVbo = NULL;
#endif
}


///////////////////////////////////////////////////////////////////////
//  CTerrainCell::GetIndices

inline void CTerrainCell::GetIndices(unsigned int& uiOffset, unsigned int& uiNumIndices) const
{
    uiOffset = m_uiIndicesOffset;
    uiNumIndices = m_uiNumIndices;
}


///////////////////////////////////////////////////////////////////////
//  CTerrainCell::GetLod

inline int CTerrainCell::GetLod(void) const
{
    return m_nLod;
}


///////////////////////////////////////////////////////////////////////
//  CTerrainCell::SetExtents

inline void CTerrainCell::SetExtents(const CExtents& cExtents)
{
    m_cExtents = cExtents;
    m_vCenter = m_cExtents.GetCenter( );
}


///////////////////////////////////////////////////////////////////////
//  CTerrainCell::GetDistanceFromCamera

inline float CTerrainCell::GetDistanceFromCamera(void) const
{
    return m_fDistanceFromCamera;
}


///////////////////////////////////////////////////////////////////////
//  CTerrainCell::GetVbo

inline void* CTerrainCell::GetVbo(void) const
{
    return m_pVbo;
}


///////////////////////////////////////////////////////////////////////
//  CTerrainCell::SetVbo

inline void CTerrainCell::SetVbo(void* pVbo)
{
    m_pVbo = pVbo;
}


///////////////////////////////////////////////////////////////////////
//  CTerrain::IsEnabled

inline bool CTerrain::IsEnabled(void) const
{
    return !m_aMasterLodIndexStrip.empty( );
}


///////////////////////////////////////////////////////////////////////
//  CTerrain::GetNumLods

inline int CTerrain::GetNumLods(void) const
{
    return m_nNumLods;
}


///////////////////////////////////////////////////////////////////////
//  CTerrain::GetMaxTileRes

inline int CTerrain::GetMaxTileRes(void) const
{
    return m_nMaxTileRes;
}


///////////////////////////////////////////////////////////////////////
//  CTerrain::GetCellSize

inline float CTerrain::GetCellSize(void) const
{
    return m_cTerrainCellMap.GetCellSize( );
}


///////////////////////////////////////////////////////////////////////
//  CTerrain::SetHeightHints

inline void CTerrain::SetHeightHints(float fGlobalLowPoint, float fGlobalHighPoint)
{
    m_fGlobalLowPoint = fGlobalLowPoint;
    m_fGlobalHighPoint = fGlobalHighPoint;
}


///////////////////////////////////////////////////////////////////////
//  CTerrain::SetLodRange

inline void CTerrain::SetLodRange(float fNear, float fFar)
{
    m_fNearLodDistance = fNear;
    m_fFarLodDistance = fFar;
}


///////////////////////////////////////////////////////////////////////
//  CTerrain::GetLodRange

inline void CTerrain::GetLodRange(float& fNear, float& fFar) const
{
    fNear = m_fNearLodDistance;
    fFar = m_fFarLodDistance;
}


///////////////////////////////////////////////////////////////////////
//  CTerrain::GetCompositeIndices

inline const CArray<unsigned int>& CTerrain::GetCompositeIndices(void) const
{
    return m_aMasterLodIndexStrip;
}

