///////////////////////////////////////////////////////////////////////  
//  SpeedTreeForest.inl
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
//  CView::CView

inline CView::CView( ) :
    m_bLodRefPointSet(false),
    m_fNearClip(1.0f),
    m_fFarClip(100.0f),
    m_fCameraAzimuth(0.0f),
    m_fCameraPitch(0.0f),
    m_fHorzFadeStartAngle(DEG2RAD(30.0f)),
    m_fHorzFadeEndAngle(DEG2RAD(60.0f))
{
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetCameraPos

inline const noVec3& CView::GetCameraPos(void) const
{
    return m_vCameraPos;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetLodRefPoint

inline const noVec3& CView::GetLodRefPoint(void) const
{
    return m_bLodRefPointSet ? m_vLodRefPoint : m_vCameraPos;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetProjection

inline const noMat4& CView::GetProjection(void) const
{
    return m_mProjection;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetModelview

inline const noMat4& CView::GetModelview(void) const
{
    return m_mModelview;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetModelviewNoTranslate

inline const noMat4& CView::GetModelviewNoTranslate(void) const
{
    return m_mModelviewNoTranslate;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetNearClip

inline float CView::GetNearClip(void) const
{
    return m_fNearClip;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetFarClip

inline float CView::GetFarClip(void) const
{
    return m_fFarClip;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetCameraDir

inline const noVec3& CView::GetCameraDir(void) const
{
    return m_vCameraDir;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetComposite

inline const noMat4& CView::GetComposite(void) const
{
    return m_mComposite;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetCompositeNoTranslate

inline const noMat4& CView::GetCompositeNoTranslate(void) const
{
    return m_mCompositeNoTranslate;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetCameraAzimuth

inline float CView::GetCameraAzimuth(void) const
{
    return m_fCameraAzimuth;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetCameraPitch

inline float CView::GetCameraPitch(void) const
{
    return m_fCameraPitch;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetFrustumPoints

inline const noVec3* CView::GetFrustumPoints(void) const
{
    return m_avFrustumPoints;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetFrustumPlanes

inline const noVec4* CView::GetFrustumPlanes(void) const
{
    return m_avFrustumPlanes;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetFrustumExtents

inline const CExtents& CView::GetFrustumExtents(void) const
{
    return m_cFrustumExtents;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetCameraFacingMatrix

inline const noMat4& CView::GetCameraFacingMatrix(void) const
{
    return m_mCameraFacingMatrix;
}


///////////////////////////////////////////////////////////////////////  
//  CView::SetHorzBillboardFadeAngles

inline void CView::SetHorzBillboardFadeAngles(float fStart, float fEnd)
{
    m_fHorzFadeStartAngle = fStart;
    m_fHorzFadeEndAngle = fEnd;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetHorzBillboardFadeAngles

inline void CView::GetHorzBillboardFadeAngles(float& fStart, float& fEnd) const
{
    fStart = m_fHorzFadeStartAngle;
    fEnd = m_fHorzFadeEndAngle;
}


///////////////////////////////////////////////////////////////////////  
//  CView::GetHorzBillboardFadeValue

inline float CView::GetHorzBillboardFadeValue(void) const
{
    return m_fHorzFadeValue;
}


///////////////////////////////////////////////////////////////////////  
//  CForest::GetBaseTrees

inline const TTreeArray& CForest::GetBaseTrees(void) const
{
    return m_aBaseTrees;
}


///////////////////////////////////////////////////////////////////////  
//  CForest::GetTreeCellMap

inline const TTreeCellMap& CForest::GetTreeCellMap(void) const
{
    return m_cTreeCellMap;
}


///////////////////////////////////////////////////////////////////////
//  CForest::SetCullCellSize

inline void CForest::SetCullCellSize(float fCellSize)
{
    m_cTreeCellMap.SetCellSize(fCellSize);
}


///////////////////////////////////////////////////////////////////////
//  CForest::FrameEnd

inline void CForest::FrameEnd(void)
{
    m_aBillboardVbosFreedSinceLastCull.resize(0);
    ++m_nUpdateIndex;
    m_bLightDirChanged = false;
    m_bBaseTreesChanged = false;
}


///////////////////////////////////////////////////////////////////////
//  CForest::EnableWind

inline void CForest::EnableWind(bool bFlag)
{
    m_bWindEnabled = bFlag;
}


///////////////////////////////////////////////////////////////////////
//  CForest::IsWindEnabled

inline bool CForest::IsWindEnabled(void) const
{
    return m_bWindEnabled;
}


///////////////////////////////////////////////////////////////////////
//  CForest::SetGlobalWindStrength

inline void CForest::SetGlobalWindStrength(float fStrength)
{
    m_fGlobalWindStrength = min(1.0f, max(fStrength, 0.0f));

    // update all of the base trees
    for (unsigned int i = 0; i < m_aBaseTrees.size( ); ++i)
    {
        assert(m_aBaseTrees[i]);
        m_aBaseTrees[i]->GetWind( ).SetStrength(m_fGlobalWindStrength);
    }

    // update main wind object
    m_cWindLeader.SetStrength(m_fGlobalWindStrength);
}


///////////////////////////////////////////////////////////////////////
//  CForest::GetGlobalWindStrength

inline float CForest::GetGlobalWindStrength(void) const
{
    return m_fGlobalWindStrength;
}


///////////////////////////////////////////////////////////////////////
//  CForest::SetGlobalWindDirection

inline void CForest::SetGlobalWindDirection(const noVec3& vDir)
{
    m_vWindDir = vDir;

    // update all of the base trees
    for (unsigned int i = 0; i < m_aBaseTrees.size( ); ++i)
    {
        assert(m_aBaseTrees[i]);
        m_aBaseTrees[i]->GetWind( ).SetDirection(m_vWindDir.TofloatPtr());
    }

    // update main wind object
    m_cWindLeader.SetDirection(m_vWindDir.TofloatPtr());
}


///////////////////////////////////////////////////////////////////////
//  CForest::GetGlobalWindDirection

inline const noVec3& CForest::GetGlobalWindDirection(void) const
{
    return m_vWindDir;
}


///////////////////////////////////////////////////////////////////////  
//  CForest::AdvanceGlobalWind

inline void CForest::AdvanceGlobalWind(void)
{
    // update main wind object
    m_cWindLeader.Advance(m_bWindEnabled, m_fGlobalTime);

    // update all of the base trees
    for (unsigned int i = 0; i < m_aBaseTrees.size( ); ++i)
    {
        assert(m_aBaseTrees[i]);
        m_aBaseTrees[i]->GetWind( ).Advance(m_bWindEnabled, m_fGlobalTime);
    }
}


///////////////////////////////////////////////////////////////////////  
//  CForest::SetWindLeader

inline void CForest::SetWindLeader(const CWind* pLeader)
{
    assert(pLeader);

    m_cWindLeader = *pLeader;

    // update all of the base trees
    for (unsigned int i = 0; i < m_aBaseTrees.size( ); ++i)
    {
        assert(m_aBaseTrees[i]);
        m_aBaseTrees[i]->GetWind( ).SetWindLeader(&m_cWindLeader);
    }

    m_cWindLeader.SetWindLeader(NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CForest::GetWindLeader

inline const CWind& CForest::GetWindLeader(void) const
{
    return m_cWindLeader;
}


///////////////////////////////////////////////////////////////////////  
//  CForest::SetGlobalTime

inline void CForest::SetGlobalTime(float fSeconds)
{
    m_fGlobalTime = fSeconds;
}


///////////////////////////////////////////////////////////////////////  
//  CForest::GetGlobalTime

inline float CForest::GetGlobalTime(void) const
{
    return m_fGlobalTime;
}


///////////////////////////////////////////////////////////////////////  
//  CForest::SetLightDir

inline void CForest::SetLightDir(const noVec3& vLightDir)
{
    if (m_vLightDir != vLightDir)
    {
        m_vLightDir = vLightDir;
        m_bLightDirChanged = true;
    }
}


///////////////////////////////////////////////////////////////////////  
//  CForest::GetLightDir

inline const noVec3& CForest::GetLightDir(void) const
{
    return m_vLightDir;
}


///////////////////////////////////////////////////////////////////////  
//  CForest::LightDirChanged

inline bool CForest::LightDirChanged(void) const
{
    return m_bLightDirChanged;
}


///////////////////////////////////////////////////////////////////////  
//  CForest::SetCascadedShadowMapDistances

inline void CForest::SetCascadedShadowMapDistances(const float afSplits[c_nMaxNumShadowMaps], float fFarClip)
{
    for (int i = 0; i < c_nMaxNumShadowMaps; ++i)
        m_afCascadedShadowMapSplits[i + 1] = afSplits[i] / fFarClip;

    m_bLightDirChanged = true;
}


///////////////////////////////////////////////////////////////////////  
//  CForest::GetCascadedShadowMapDistances

inline const float* CForest::GetCascadedShadowMapDistances(void) const
{
    return m_afCascadedShadowMapSplits;
}


///////////////////////////////////////////////////////////////////////  
//  CForest::SetShadowFadePercentage

inline void CForest::SetShadowFadePercentage(float fFade)
{
    m_fShadowFadePercentage = fFade;
}


///////////////////////////////////////////////////////////////////////  
//  CForest::GetShadowFadePercentage

inline float CForest::GetShadowFadePercentage(void) const
{
    return m_fShadowFadePercentage;
}


///////////////////////////////////////////////////////////////////////  
//  CForest::SPopulationStats::SPopulationStats

inline CForest::SPopulationStats::SPopulationStats( ) :
    m_nNumCells(-1),
    m_nNumBaseTrees(-1),
    m_nNumInstances(-1),
    m_fAverageNumInstancesPerBase(-1.0f),
    m_nMaxNumBillboardsPerCell(-1),
    m_nMaxNumInstancesPerCell(-1),
    m_nMaxNumInstancesPerCellPerBase(-1),
    m_fAverageInstancesPerCell(-1.0f),
    m_nMaxNumBillboardImages(-1)
{
}



