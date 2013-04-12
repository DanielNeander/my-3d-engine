///////////////////////////////////////////////////////////////////////
//  Grass.inl
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
//  CGrassCell::CGrassCell

inline CGrassCell::CGrassCell( ) :
    m_pVbo(NULL),
    m_nNumBlades(0),
    m_fDistanceSquaredFromCamera(0.0f)
{
}


///////////////////////////////////////////////////////////////////////
//  CGrassCell::~CGrassCell

inline CGrassCell::~CGrassCell( )
{
#ifndef NDEBUG
    m_pVbo = NULL;
    m_nNumBlades = -1;
#endif
}


///////////////////////////////////////////////////////////////////////
//  CGrassCell::SetExtents

inline void CGrassCell::SetExtents(const CExtents& cExtents)
{
    m_cExtents = cExtents;
    m_vCenter = cExtents.GetCenter( );
}


///////////////////////////////////////////////////////////////////////
//  CGrassCell::GetDistanceSquaredFromCamera

inline float CGrassCell::GetDistanceSquaredFromCamera(void) const
{
    return m_fDistanceSquaredFromCamera;
}


///////////////////////////////////////////////////////////////////////
//  CGrassCell::GetVbo

inline void* CGrassCell::GetVbo(void) const
{
    return m_pVbo;
}


///////////////////////////////////////////////////////////////////////
//  CGrassCell::SetVbo

inline void CGrassCell::SetVbo(void* pVbo)
{
    m_pVbo = pVbo;
}


///////////////////////////////////////////////////////////////////////
//  CGrassCell::GetNumBlades

inline int CGrassCell::GetNumBlades(void) const
{
    return m_nNumBlades;
}


///////////////////////////////////////////////////////////////////////
//  CGrassCell::SetNumBlades

inline void CGrassCell::SetNumBlades(int nNumBlades)
{
    m_nNumBlades = nNumBlades;
}


///////////////////////////////////////////////////////////////////////
//  CGrass::IsEnabled

inline bool CGrass::IsEnabled(void) const
{
    return m_nNumImageCols > 0;
}


///////////////////////////////////////////////////////////////////////
//  CGrass::GetCellSize

inline float CGrass::GetCellSize(void) const
{
    return m_cGrassCellMap.GetCellSize( );
}


///////////////////////////////////////////////////////////////////////
//  CGrass::SetHeightHints

inline void CGrass::SetHeightHints(float fGlobalLowPoint, float fGlobalHighPoint)
{
    m_fGlobalLowPoint = fGlobalLowPoint;
    m_fGlobalHighPoint = fGlobalHighPoint;
}


///////////////////////////////////////////////////////////////////////
//  CGrass::GetWind

inline CWind& CGrass::GetWind(void)
{
    return m_cWind;
}


///////////////////////////////////////////////////////////////////////
//  CGrass::GetWind

inline const CWind& CGrass::GetWind(void) const
{
    return m_cWind;
}


///////////////////////////////////////////////////////////////////////
//  CGrass::SetLodRange

inline void CGrass::SetLodRange(float fStartFade, float fEndFade)
{
    m_fStartFade = fStartFade;
    m_fEndFade = fEndFade;
}


///////////////////////////////////////////////////////////////////////
//  CGrass::GetLodRange

inline void CGrass::GetLodRange(float& fStartFade, float& fEndFade)
{
    fStartFade = m_fStartFade;
    fEndFade = m_fEndFade;
}


///////////////////////////////////////////////////////////////////////
//  CGrass::GetTexture

inline const char* CGrass::GetTexture(void) const
{
    return m_strTexture.c_str( );
}


///////////////////////////////////////////////////////////////////////
//  CGrass::GetNumSubImages

inline int CGrass::GetNumSubImages(void) const
{
    return m_nNumImageCols * m_nNumImageRows;
}


///////////////////////////////////////////////////////////////////////
//  CGrass::GetBladeTexCoords

inline const float* CGrass::GetBladeTexCoords(int nImageIndex, bool bMirror) const
{
    assert(m_aBladeTexCoords.size( ) > size_t(nImageIndex * 8));

    return &m_aBladeTexCoords[nImageIndex * 8 + (bMirror ? 4 : 0)];
}


///////////////////////////////////////////////////////////////////////
//  CGrass::GetBladeTexCoordsUChar

inline const unsigned char* CGrass::GetBladeTexCoordsUChar(int nImageIndex, bool bMirror) const
{
    assert(m_aBladeTexCoords.size( ) > size_t(nImageIndex * 8));

    return &m_aBladeTexCoordsUChar[nImageIndex * 8 + (bMirror ? 4 : 0)];
}

