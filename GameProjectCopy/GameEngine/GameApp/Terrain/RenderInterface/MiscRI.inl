///////////////////////////////////////////////////////////////////////  
//  MiscRI.inl
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


////////////////////////////////////////////////////////////
// CForestRI_Interpolate

template <class T> inline T CForestRI_Interpolate(const T& tStart, const T& tEnd, float fPercent)
{
    return static_cast<T>((tStart + (tEnd - tStart) * fPercent));
}


////////////////////////////////////////////////////////////
// TextureShaderConstantName

inline CString TextureShaderConstantName(const CString& strBaseName)
{
#if defined(SPEEDTREE_OPENGL) || defined(_XBOX)
    return CString("sam") + strBaseName;
#else
    return CString("g_t") + strBaseName;
#endif
}


////////////////////////////////////////////////////////////
// SMaterialRI::Copy

template<class TTexturePolicy>
inline void SMaterialRI<TTexturePolicy>::Copy(const SMaterial& sSource)
{
    *((SMaterial*) this) = sSource;
}


////////////////////////////////////////////////////////////
// CLodRenderInfo::CLodRenderInfo

inline CLodRenderInfo::CLodRenderInfo( ) :
    m_nNumLods(0),
    m_nMaxMaterialId(0)
{
}


////////////////////////////////////////////////////////////
// CLodRenderInfo::InitIndexed

inline void CLodRenderInfo::InitIndexed(const SIndexedTriangles* pLods, int nNumLods)
{
    // determine the highest material id used by all LODs
    m_nMaxMaterialId = 0;
    for (int nLod = 0; nLod < nNumLods; ++nLod)
    {
        const SIndexedTriangles* pLod = pLods + nLod;
        for (int nMaterialGroup = 0; nMaterialGroup < pLod->m_nNumMaterialGroups; ++nMaterialGroup)
        {
            const SDrawCallInfo* pDrawCallInfo = pLod->m_pDrawCallInfo + nMaterialGroup;
            m_nMaxMaterialId = max(m_nMaxMaterialId, pDrawCallInfo->m_nMaterialIndex);
        }
    }

    m_nMaxMaterialId++;
    m_nNumLods = nNumLods;

    SRef sDefault;
    sDefault.m_nMaterialId = -1;
    sDefault.m_nOffset = -1;
    sDefault.m_nLength = -1;

    m_aRefs.resize(m_nNumLods * m_nMaxMaterialId, sDefault);
}


////////////////////////////////////////////////////////////
// CLodRenderInfo::InitLeafCards

inline void CLodRenderInfo::InitLeafCards(const SLeafCards* pLods, int nNumLods)
{
    // determine the highest material id used by all LODs
    m_nMaxMaterialId = 0;
    for (int nLod = 0; nLod < nNumLods; ++nLod)
    {
        const SLeafCards* pLod = pLods + nLod;
        for (int nMaterialGroup = 0; nMaterialGroup < pLod->m_nNumMaterialGroups; ++nMaterialGroup)
        {
            const SDrawCallInfo* pDrawCallInfo = pLod->m_pDrawCallInfo + nMaterialGroup;
            m_nMaxMaterialId = max(m_nMaxMaterialId, pDrawCallInfo->m_nMaterialIndex);
        }
    }

    m_nMaxMaterialId++;
    m_nNumLods = nNumLods;

    SRef sDefault;
    sDefault.m_nMaterialId = -1;
    sDefault.m_nOffset = -1;
    sDefault.m_nLength = -1;

    m_aRefs.resize(m_nNumLods * m_nMaxMaterialId, sDefault);
}

////////////////////////////////////////////////////////////
// CLodRenderInfo::AddRef

inline void CLodRenderInfo::AddRef(int nLod, int nMaterialId, int nOffset, int nLength)
{
    assert(nLod < m_nNumLods);
    assert(nMaterialId < m_nMaxMaterialId);

    SRef& sRef = m_aRefs[nMaterialId * m_nNumLods + nLod];
    sRef.m_nMaterialId = nMaterialId;
    sRef.m_nOffset = nOffset;
    sRef.m_nLength = nLength;

    m_sMaterialsUsed.insert(nMaterialId);
}


////////////////////////////////////////////////////////////
// CLodRenderInfo::IsActive

inline const CLodRenderInfo::SRef* CLodRenderInfo::IsActive(int nLod, int nMaterialId) const
{
    assert(nLod < m_nNumLods);
    assert(nMaterialId < m_nMaxMaterialId);

    const CLodRenderInfo::SRef* pRef = &m_aRefs[nMaterialId * m_nNumLods + nLod];

    if (pRef->m_nMaterialId == -1)
        return NULL;

    return pRef;
}


////////////////////////////////////////////////////////////
// CLodRenderInfo::MaterialRefs

inline const CSet<int>* CLodRenderInfo::MaterialRefs(void) const
{
    return &m_sMaterialsUsed;
}


///////////////////////////////////////////////////////////////////////
//  SRenderStats::SGeometryStats::SGeometryStats

inline SRenderStats::SGeometryStats::SGeometryStats( )
{
    Reset( );
}


///////////////////////////////////////////////////////////////////////
//  SRenderStats::SGeometryStats::Reset

inline void SRenderStats::SGeometryStats::Reset(void)
{
    m_uiNumObjects = 0;
    m_uiNumTextureBinds = 0;
    m_uiNumDrawCalls = 0;
    m_uiNumVboBinds = 0;
    m_uiNumTriangles = 0;
}


///////////////////////////////////////////////////////////////////////
//  SRenderStats::SRenderStats

inline SRenderStats::SRenderStats( )
{
    Reset( );
}


///////////////////////////////////////////////////////////////////////
//  SRenderStats::Reset

inline void SRenderStats::Reset(void)
{
    for (int i = 0; i < STATS_CATEGORY_COUNT; ++i)
        for (int j = 0; j < RENDER_PASS_NUM_TYPES; ++j)
            m_aGeometries[i][j].Reset( );

    m_fCullTime = 0.0f;
}


///////////////////////////////////////////////////////////////////////
//  SRenderStats::GetTotals

inline void SRenderStats::GetTotals(SRenderStats::SGeometryStats& sTotals) const
{
    sTotals.Reset( );

    for (int i = 0; i < STATS_CATEGORY_COUNT; ++i)
    {
        for (int j = 0; j < RENDER_PASS_NUM_TYPES; ++j)
        {
            sTotals.m_uiNumObjects += m_aGeometries[i][j].m_uiNumObjects;
            sTotals.m_uiNumTextureBinds += m_aGeometries[i][j].m_uiNumTextureBinds;
            sTotals.m_uiNumDrawCalls += m_aGeometries[i][j].m_uiNumDrawCalls;
            sTotals.m_uiNumVboBinds += m_aGeometries[i][j].m_uiNumVboBinds;
            sTotals.m_uiNumTriangles += m_aGeometries[i][j].m_uiNumTriangles;
        }
    }
}


///////////////////////////////////////////////////////////////////////
//  SForestRenderInfo::SForestRenderInfo

inline SForestRenderInfo::SForestRenderInfo( ) :
    // general rendering
    m_strShaderFilename(""),
    m_nMaxAnisotropy(0),
    m_bHorizontalBillboards(true),
    m_fAlphaTestScalar(0.57f),
    m_bZPrePass(false),
    m_nMaxBillboardImagesByBase(50),
    // lighting
    m_fGlobalLightScalar(1.0f),
    m_bSpecularLighting(false),
    m_bTransmissionLighting(false),
    m_bDetailLayer(false),
    m_bDetailNormalMapping(false),
    m_bAmbientContrast(false),
    // fog
    m_fFogStartDistance(2500.0f),
    m_fFogEndDistance(5000.0f),
    m_vFogColor(1.0f, 1.0f, 1.0),
    // sky
    m_vSkyColor(0.2f, 0.3f, 0.5f),
    m_fSkyFogMin(-0.5f),
    m_fSkyFogMax(1.0f),
    // sun
    m_vSunColor(1.0f, 1.0f, 0.85f),
    m_fSunSize(0.001f),
    m_fSunSpreadExponent(200.0f),
    m_fSunFogBloom(0.0f),
    // shadows
    m_nNumShadowMaps(3),
    m_nShadowMapResolution(1024),
    m_bSmoothShadows(false),
    m_bShowShadowSplitsOnTerrain(false),
    // wind
    m_bWindEnabled(true),
    m_bFrondRippling(true)
{
}
