///////////////////////////////////////////////////////////////////////  
//  TerrainData.inl
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
//  CTerrainData::CTerrainData

inline CTerrainData::CTerrainData(void) :
    m_fSize(1.0f),
    m_fHeightScale(0.1f),
    m_fNormalMapBScale(1.0f),
    m_fMinHeight(FLT_MAX),
    m_fMaxHeight(-FLT_MAX)
{
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::~CTerrainData

inline CTerrainData::~CTerrainData(void)
{
    Clear( );
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::Clear

inline void CTerrainData::Clear(void)
{
    m_sMaterial.m_vAmbient.Set(0.5f, 0.5f, 0.5f, 1.0f);
    m_sMaterial.m_vDiffuse.Set(1.0f, 1.0f, 1.0f, 1.0f);
    m_sMaterial.m_vSpecular.Set(0.0f, 0.0f, 0.0f, 1.0f);
    m_sMaterial.m_vEmissive.Set(0.0f, 0.0f, 0.0f, 1.0f);

    m_sMaterial.m_fShininess = 30.0f;

    m_fMinHeight = FLT_MAX;
    m_fMaxHeight = -FLT_MAX;
    
    m_cHeightData.Clear( );
    m_cNormalData.Clear( );
    m_cSlopeData.Clear( );
    m_cAOData.Clear( );
    m_aTextureFilenames.clear( );
    m_aTextureTiling.clear( );
    m_aColors.clear( );
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::IsLoaded

inline st_bool CTerrainData::IsLoaded(void) const
{
    return m_cHeightData.IsPresent( );
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::GetTextureFilenames

inline const CArray<CFixedString>& CTerrainData::GetTextureFilenames(void) const
{
    return m_aTextureFilenames;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::GetTextureTiling

inline const CArray<st_float32>& CTerrainData::GetTextureTiling(void) const
{
    return m_aTextureTiling;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::GetColors

inline const CArray<noVec4>& CTerrainData::GetColors(void) const
{
    return m_aColors;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::SetHeightScale

inline void CTerrainData::SetHeightScale(st_float32 fHeightScale, st_float32 fNormalMapBScale)
{
    m_fHeightScale = fHeightScale;
    m_fNormalMapBScale = fNormalMapBScale;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::GetHeightRange

inline void CTerrainData::GetHeightRange(st_float32& fMin, st_float32& fMax) const
{
    fMin = m_fMinHeight;
    fMax = m_fMaxHeight;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::GetHeight

inline st_float32 CTerrainData::GetHeight(st_float32 fXPos, st_float32 fYPos) const
{
    return m_cHeightData.BilinearInterpolation(fXPos / m_fSize, fYPos / m_fSize);
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::GetSmoothHeight

inline st_float32 CTerrainData::GetSmoothHeight(st_float32 fXPos, st_float32 fYPos, st_float32 fDistance) const
{
    return m_cHeightData.Smooth(fXPos / m_fSize, fYPos / m_fSize, fDistance / m_fSize);
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::GetNormal

inline noVec3 CTerrainData::GetNormal(st_float32 fXPos, st_float32 fYPos) const
{
    noVec3 vecReturn = m_cNormalData.BilinearInterpolation(fXPos / m_fSize, fYPos / m_fSize);
    if (vecReturn.LengthSqr( ) == 0.0f)
        vecReturn.Set(0.0f, 0.0f, 1.0f);
    else
        vecReturn.Normalize( );

    return vecReturn;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::GetSlope

inline st_float32 CTerrainData::GetSlope(st_float32 fXPos, st_float32 fYPos) const
{
    return m_cSlopeData.BilinearInterpolation(fXPos / m_fSize, fYPos / m_fSize);
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::ReportParameters

inline void CTerrainData::ReportParameters(noVec3 vPos, st_float32 fDistance)
{
    st_float32 fSlope = m_cSlopeData.Smooth(vPos.x / m_fSize, vPos.y / m_fSize, fDistance / m_fSize);
    st_float32 fHeight = m_cHeightData.Smooth(vPos.x / m_fSize, vPos.y / m_fSize, fDistance / m_fSize);
    
    printf("Terrain parameters for (%g, %g):  height=%g  slope=%g\n", vPos.x, vPos.y, fHeight, fSlope);
}



///////////////////////////////////////////////////////////////////////  
//  CTerrainData::GetHeightForPlacementParameters

inline st_bool CTerrainData::GetHeightForPlacementParameters(st_float32& fHeight, 
                                                             st_float32 fXPos, 
                                                             st_float32 fYPos, 
                                                             st_float32 fMinHeight, 
                                                             st_float32 fMaxHeight,
                                                             st_float32 fMinSlope, 
                                                             st_float32 fMaxSlope) const
{
    st_bool bReturn = false;

    fXPos /= m_fSize;
    fYPos /= m_fSize;

    st_float32 fSlope = m_cSlopeData.BilinearInterpolation(fXPos, fYPos);
    if (fSlope >= fMinSlope && fSlope <= fMaxSlope)
    {
        fHeight = m_cHeightData.BilinearInterpolation(fXPos, fYPos);
        if (fHeight >= fMinHeight && fHeight <= fMaxHeight)
            bReturn = true;
    }

    return bReturn;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::GetAmbientOcclusion

inline st_float32 CTerrainData::GetAmbientOcclusion(st_float32 x, st_float32 y) const
{
    if (m_cAOData.m_pData)
    {
        const st_float32 fU = (x - m_afAOBounds[0]) * m_afAOBounds[2];
        const st_float32 fV = (y - m_afAOBounds[1]) * m_afAOBounds[3];
        if (fU > 0.0f && fU < 1.0f && fV > 0.0f && fV < 1.0f)
        {
            st_uchar ucReturn = m_cAOData.NearestNeighbor(fU, fV);
            return st_float32(ucReturn) * 0.0039215686f; // convert to 0-1
        }
    }

    return 1.0f;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::CInterpolationData::Clear

template <class T>
inline void CTerrainData::CInterpolationData<T>::Clear(void)
{
    if (m_pData != NULL)
    {
        st_delete_array<T>(m_pData, "CTerrainData::CInterpolationData");
        m_pData = NULL;
    }

    m_nWidth = 0;
    m_nHeight = 0;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::CInterpolationData::NearestNeighbor

template <class T>
inline T CTerrainData::CInterpolationData<T>::NearestNeighbor(st_float32 fU, st_float32 fV) const
{
    assert(m_nWidth > 0);
    assert(m_nHeight > 0);
    assert(m_pData != NULL);

    const st_int32 nU = st_int32(fU * m_nWidth + 0.5f);
    const st_int32 nV = st_int32(fV * m_nHeight + 0.5f);
    return m_pData[nU + nV * m_nWidth];
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::CInterpolationData::BilinearInterpolation

template <class T>
inline T CTerrainData::CInterpolationData<T>::BilinearInterpolation(st_float32 fU, st_float32 fV) const
{
    assert(m_nWidth > 0);
    assert(m_nHeight > 0);
    assert(m_pData != NULL);

    st_int32 nUint = st_int32(fU);
    fU -= nUint;
    if (fU < 0.0f)
        fU += 1.0f;
    if (fU >= 1.0f)
        fU = 0.0f;

    st_int32 nVint = st_int32(fV);
    fV -= nVint;
    if (fV < 0.0f)
        fV += 1.0f;
    if (fV >= 1.0f)
        fV = 0.0f;

    fU *= (st_float32)m_nWidth;
    fV *= (st_float32)m_nHeight;

    st_int32 nLowerX = st_uint32(fU);
    st_int32 nLowerY = st_uint32(fV);
    st_int32 nHigherX = nLowerX + 1;
    st_int32 nHigherY = nLowerY + 1;

    if (nHigherX >= m_nWidth)
        nHigherX = 0;
    if (nHigherY >= m_nHeight)
        nHigherY = 0;

    T t1 = m_pData[nLowerX + nLowerY * m_nWidth];
    T t2 = m_pData[nHigherX + nLowerY * m_nWidth];
    T t3 = m_pData[nLowerX + nHigherY * m_nWidth];
    T t4 = m_pData[nHigherX + nHigherY * m_nWidth];

    st_float32 fPercentageX = fU - st_float32(nLowerX);
    t1 = Interpolate(t1, t2, fPercentageX);
    t2 = Interpolate(t3, t4, fPercentageX);

    st_float32 fPercentageY = fV - st_float32(nLowerY);
    return Interpolate(t1, t2, fPercentageY);
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::CInterpolationData::Smooth

template <class T>
inline T CTerrainData::CInterpolationData<T>::Smooth(st_float32 fU, st_float32 fV, st_float32 fDistance) const
{
    T tReturn;
    memset(&tReturn, 0, sizeof(T));

    if (fDistance <= 0.0f)
        tReturn = BilinearInterpolation(fU, fV);
    else
    {
        const st_float32 afTestPoints[9][2] = 
        {
            {  0.0f * fDistance,   0.0f * fDistance },
            {  0.8f * fDistance,   0.0f * fDistance },
            { -0.8f * fDistance,   0.0f * fDistance },
            {  0.0f * fDistance,   0.8f * fDistance },
            {  0.0f * fDistance,  -0.8f * fDistance },
            {  0.25f * fDistance,  0.25f * fDistance },
            {  0.25f * fDistance, -0.25f * fDistance },
            { -0.25f * fDistance,  0.25f * fDistance },
            { -0.25f * fDistance, -0.25f * fDistance }
        };

        st_float32 fTotalWeight = 0.0f;
        for (st_uint32 i = 0; i < 9; ++i)
        {
            const st_float32* pTestPoint = afTestPoints[i];
            st_float32 fWeight = (1.0f - sqrt((pTestPoint[0] * pTestPoint[0]) + (pTestPoint[1] * pTestPoint[1])));
            fTotalWeight += fWeight;
            tReturn += fWeight * BilinearInterpolation(fU + pTestPoint[0], fV + pTestPoint[1]);
        }   
    
        tReturn /= fTotalWeight;
    }

    return tReturn;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainData::CInterpolationData::IsPresent

template <class T>
inline st_bool CTerrainData::CInterpolationData<T>::IsPresent(void) const
{
    return (m_pData != NULL);
}


