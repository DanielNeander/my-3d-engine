///////////////////////////////////////////////////////////////////////  
//  Extents.inl
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement
//
//      Copyright (c) 2003-2009 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      http://www.idvinc.com
//
//  *** Release version 5.0 ***


/////////////////////////////////////////////////////////////////////
// CExtents::CExtents

inline CExtents::CExtents( )
{
    Reset( );
}


/////////////////////////////////////////////////////////////////////
// CExtents::CExtents

inline CExtents::CExtents(const float afExtents[6]) :
    m_cMin(afExtents[0], afExtents[1], afExtents[2]),
    m_cMax(afExtents[3], afExtents[4], afExtents[5])
{
}


/////////////////////////////////////////////////////////////////////
// CExtents::CExtents

inline CExtents::CExtents(const noVec3& cMin, const noVec3& cMax) :
    m_cMin(cMin),
    m_cMax(cMax)
{
}


/////////////////////////////////////////////////////////////////////
// CExtents::~CExtents

inline CExtents::~CExtents( )
{
}


/////////////////////////////////////////////////////////////////////
// CExtents::Reset

inline void CExtents::Reset(void)
{
    m_cMin.Set(FLT_MAX, FLT_MAX, FLT_MAX);
    m_cMax.Set(-FLT_MAX, -FLT_MAX, -FLT_MAX);
}


/////////////////////////////////////////////////////////////////////
// CExtents::SetToZeros

inline void CExtents::SetToZeros(void)
{
    m_cMin.Set(0.0f, 0.0f, 0.0f);
    m_cMax.Set(0.0f, 0.0f, 0.0f);
}


/////////////////////////////////////////////////////////////////////
// CExtents::Order
//
// If mins are not <= max values, then swap them

inline void CExtents::Order(void)
{
    float fTmp;

    if (m_cMin.x > m_cMax.x)
    {
        fTmp = m_cMin.x;
        m_cMin.x = m_cMax.x;
        m_cMax.x = fTmp;
    }

    if (m_cMin.y > m_cMax.y)
    {
        fTmp = m_cMin.y;
        m_cMin.y = m_cMax.y;
        m_cMax.y = fTmp;
    }

    if (m_cMin.z > m_cMax.z)
    {
        fTmp = m_cMin.z;
        m_cMin.z = m_cMax.z;
        m_cMax.z = fTmp;
    }
}


/////////////////////////////////////////////////////////////////////
// CExtents::Valid

inline bool CExtents::Valid(void) const
{
    return (m_cMin.x != FLT_MAX || 
            m_cMin.y != FLT_MAX || 
            m_cMin.z != FLT_MAX || 
            m_cMax.x != -FLT_MAX || 
            m_cMax.y != -FLT_MAX || 
            m_cMax.z != -FLT_MAX);
}


/////////////////////////////////////////////////////////////////////
// CExtents::ExpandAround

inline void CExtents::ExpandAround(const float afPoint[3])
{
    m_cMin.x = (afPoint[0] < m_cMin.x) ? afPoint[0] : m_cMin.x;
    m_cMin.y = (afPoint[1] < m_cMin.y) ? afPoint[1] : m_cMin.y;
    m_cMin.z = (afPoint[2] < m_cMin.z) ? afPoint[2] : m_cMin.z;

    m_cMax.x = (afPoint[0] > m_cMax.x) ? afPoint[0] : m_cMax.x;
    m_cMax.y = (afPoint[1] > m_cMax.y) ? afPoint[1] : m_cMax.y;
    m_cMax.z = (afPoint[2] > m_cMax.z) ? afPoint[2] : m_cMax.z;
}


/////////////////////////////////////////////////////////////////////
// CExtents::ExpandAround

inline void CExtents::ExpandAround(const noVec3& vPoint)
{
    m_cMin.x = (vPoint.x < m_cMin.x) ? vPoint.x : m_cMin.x;
    m_cMin.y = (vPoint.y < m_cMin.y) ? vPoint.y : m_cMin.y;
    m_cMin.z = (vPoint.z < m_cMin.z) ? vPoint.z : m_cMin.z;

    m_cMax.x = (vPoint.x > m_cMax.x) ? vPoint.x : m_cMax.x;
    m_cMax.y = (vPoint.y > m_cMax.y) ? vPoint.y : m_cMax.y;
    m_cMax.z = (vPoint.z > m_cMax.z) ? vPoint.z : m_cMax.z;
}


/////////////////////////////////////////////////////////////////////
// CExtents::ExpandAround

inline void CExtents::ExpandAround(const noVec3& vPoint, float fRadius)
{
    noVec3 vPointMin(vPoint.x - fRadius, vPoint.y - fRadius, vPoint.z - fRadius);
    noVec3 vPointMax(vPoint.x + fRadius, vPoint.y + fRadius, vPoint.z + fRadius);

    m_cMin[0] = (vPointMin.x < m_cMin[0]) ? vPointMin.x : m_cMin[0];
    m_cMin[1] = (vPointMin.y < m_cMin[1]) ? vPointMin.y : m_cMin[1];
    m_cMin[2] = (vPointMin.z < m_cMin[2]) ? vPointMin.z : m_cMin[2];

    m_cMax[0] = (vPointMax.x > m_cMax[0]) ? vPointMax.x : m_cMax[0];
    m_cMax[1] = (vPointMax.y > m_cMax[1]) ? vPointMax.y : m_cMax[1];
    m_cMax[2] = (vPointMax.z > m_cMax[2]) ? vPointMax.z : m_cMax[2];
}


/////////////////////////////////////////////////////////////////////
// CExtents::ExpandAround

inline void CExtents::ExpandAround(const CExtents& cOther)
{
    m_cMin[0] = (cOther.m_cMin[0] < m_cMin[0]) ? cOther.m_cMin[0] : m_cMin[0];
    m_cMin[1] = (cOther.m_cMin[1] < m_cMin[1]) ? cOther.m_cMin[1] : m_cMin[1];
    m_cMin[2] = (cOther.m_cMin[2] < m_cMin[2]) ? cOther.m_cMin[2] : m_cMin[2];

    m_cMax[0] = (cOther.m_cMax[0] > m_cMax[0]) ? cOther.m_cMax[0] : m_cMax[0];
    m_cMax[1] = (cOther.m_cMax[1] > m_cMax[1]) ? cOther.m_cMax[1] : m_cMax[1];
    m_cMax[2] = (cOther.m_cMax[2] > m_cMax[2]) ? cOther.m_cMax[2] : m_cMax[2];
}


/////////////////////////////////////////////////////////////////////
// CExtents::Scale

inline void CExtents::Scale(float fScale)
{
    m_cMin *= fScale;
    m_cMax *= fScale;
}


/////////////////////////////////////////////////////////////////////
// CExtents::Translate

inline void CExtents::Translate(const noVec3& vTranslation)
{
    m_cMin += vTranslation;
    m_cMax += vTranslation;
}


/////////////////////////////////////////////////////////////////////
// CExtents::Rotate

inline void CExtents::Rotate(float fRadians)
{
    noMat3 mRot;
    CCoordSys::RotateUpAxis(mRot, fRadians);

    noVec3 vCorner1 = mRot * m_cMin;
    noVec3 vCorner2 = mRot * m_cMax;
    noVec3 vCorner3 = mRot * noVec3(m_cMin.x, m_cMax.y, m_cMin.z);
    noVec3 vCorner4 = mRot * noVec3(m_cMax.x, m_cMin.y, m_cMin.z);

    Reset( );
    ExpandAround(vCorner1);
    ExpandAround(vCorner2);
    ExpandAround(vCorner3);
    ExpandAround(vCorner4);
}


/////////////////////////////////////////////////////////////////////
// CExtents::ComputeRadiusFromCenter3D

inline float CExtents::ComputeRadiusFromCenter3D(void)
{
    return GetCenter( ).Distance(m_cMin);
}


/////////////////////////////////////////////////////////////////////
// CExtents::ComputeRadiusFromCenter2D

inline float CExtents::ComputeRadiusFromCenter2D(void)
{
    noVec3 vCenter2D = CCoordSys::ConvertToStd(GetCenter( ).x, GetCenter( ).y, GetCenter( ).z);
    vCenter2D.z = 0.0f;
    noVec3 vMin2D = CCoordSys::ConvertToStd(Min( ).x, Min( ).y, Min( ).z);
    vMin2D.z = 0.0f;

    return vCenter2D.Distance(vMin2D);
}


/////////////////////////////////////////////////////////////////////
// CExtents::Min

inline const noVec3& CExtents::Min(void) const
{
    return m_cMin;
}


/////////////////////////////////////////////////////////////////////
// CExtents::Max

inline const noVec3& CExtents::Max(void) const
{
    return m_cMax;
}


/////////////////////////////////////////////////////////////////////
// CExtents::Midpoint

inline float CExtents::Midpoint(unsigned int uiAxis) const
{
    assert(uiAxis < 3);

    return 0.5f * (m_cMin[uiAxis] + m_cMax[uiAxis]);
}


/////////////////////////////////////////////////////////////////////
// CExtents::GetCenter

inline noVec3 CExtents::GetCenter(void) const
{
    return (m_cMin + m_cMax) * 0.5f;
}


/////////////////////////////////////////////////////////////////////
// CExtents::GetDiagonal

inline noVec3 CExtents::GetDiagonal(void) const
{
    return m_cMax - GetCenter( );
}


/////////////////////////////////////////////////////////////////////
// CExtents::operator float*

inline CExtents::operator float*(void)
{
    return &(m_cMin.x);
}


/////////////////////////////////////////////////////////////////////
// CExtents::operator const float*

inline CExtents::operator const float*(void) const
{
    return &(m_cMin.x);
}

