///////////////////////////////////////////////////////////////////////  
//  Core.inl
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
//  SLodProfile::SLodProfile

inline SLodProfile::SLodProfile( ) :
    m_fHighDetail3dDistance(300.0f),
    m_fLowDetail3dDistance(1200.0f),
    m_fBillboardStartDistance(1300.0f),
    m_fBillboardFinalDistance(1500.0f)
{
    ComputeDerived( );
    assert(IsValid( ));
}


///////////////////////////////////////////////////////////////////////
//  SLodProfile::IsValid

inline bool SLodProfile::IsValid(void) const
{
    return (m_fHighDetail3dDistance < m_fLowDetail3dDistance) &&
           (m_fBillboardStartDistance < m_fBillboardFinalDistance) &&
           (m_fLowDetail3dDistance < m_fBillboardStartDistance) &&
           (m_f3dRange >= 0.0f) &&
           (m_fBillboardRange >= 0.0f);
}


///////////////////////////////////////////////////////////////////////
//  SLodProfile::ComputeDerived

inline void SLodProfile::ComputeDerived(void)
{
    m_f3dRange = m_fLowDetail3dDistance - m_fHighDetail3dDistance;
    m_fBillboardRange = m_fBillboardFinalDistance - m_fBillboardStartDistance;
}


///////////////////////////////////////////////////////////////////////
//  SLodProfile::Scale

inline void SLodProfile::Scale(float fScale)
{
    m_fHighDetail3dDistance *= fScale;
    m_fLowDetail3dDistance *= fScale;
    m_fBillboardStartDistance *= fScale;
    m_fBillboardFinalDistance *= fScale;

    ComputeDerived( );
}


///////////////////////////////////////////////////////////////////////
//  SLodProfile::Square

inline void SLodProfile::Square(SLodProfile& sSquaredProfile) const
{
    sSquaredProfile.m_fHighDetail3dDistance = m_fHighDetail3dDistance * m_fHighDetail3dDistance;
    sSquaredProfile.m_fLowDetail3dDistance = m_fLowDetail3dDistance * m_fLowDetail3dDistance;
    sSquaredProfile.m_fBillboardStartDistance = m_fBillboardStartDistance * m_fBillboardStartDistance;
    sSquaredProfile.m_fBillboardFinalDistance = m_fBillboardFinalDistance * m_fBillboardFinalDistance;

    sSquaredProfile.ComputeDerived( );
}


///////////////////////////////////////////////////////////////////////
//  SLodSnapshot::SLodSnapshot

inline SLodSnapshot::SLodSnapshot( ) :
    m_nBranchLodIndex(-1),
    m_nFrondLodIndex(-1),
    m_nLeafCardLodIndex(-1),
    m_nLeafMeshLodIndex(-1)
{
}


///////////////////////////////////////////////////////////////////////
//  CCore::GetFilename

inline const char* CCore::GetFilename(void) const
{
    return m_strFilename.c_str( );
}


///////////////////////////////////////////////////////////////////////
//  CCore::GetGeometry

inline const SGeometry* CCore::GetGeometry(void) const
{
    return &m_sGeometry;
}


///////////////////////////////////////////////////////////////////////
//  CCore::HasGeometryType

inline bool CCore::HasGeometryType(EGeometryType eType) const
{
    return m_abGeometryTypesPresent[eType];
}


///////////////////////////////////////////////////////////////////////
//  CCore::GetGeometryTypeName

inline const char* CCore::GetGeometryTypeName(EGeometryType eType)
{
    switch (eType)
    {
    case GEOMETRY_TYPE_BRANCHES:
        return "Branches";
    case GEOMETRY_TYPE_FRONDS:
        return "Fronds";
    case GEOMETRY_TYPE_LEAF_CARDS:
        return "Leaf Cards";
    case GEOMETRY_TYPE_LEAF_MESHES:
        return "Leaf Meshes";
    case GEOMETRY_TYPE_VERTICAL_BILLBOARDS:
        return "Vertical Billboards";
    case GEOMETRY_TYPE_HORIZONTAL_BILLBOARDS:
        return "Horizontal Billboards";
    default:
        return "Unknown";
    };
}


///////////////////////////////////////////////////////////////////////
//  CCore::GetExtents

inline const CExtents& CCore::GetExtents(void) const
{
    return m_cExtents;
}


///////////////////////////////////////////////////////////////////////
//  CCore::GetLodProfile

inline const SLodProfile& CCore::GetLodProfile(void) const
{
    return m_sLodProfile;
}


///////////////////////////////////////////////////////////////////////
//  CCore::GetLodProfileSquared

inline const SLodProfile& CCore::GetLodProfileSquared(void) const
{
    return m_sLodProfileSquared;
}


///////////////////////////////////////////////////////////////////////
//  CCore::SetLodProfile

inline bool CCore::SetLodProfile(const SLodProfile& sLodProfile)
{
    bool bSuccess = false;

    if (sLodProfile.IsValid( ))
    {
        // standard profile
        m_sLodProfile = sLodProfile;
        m_sLodProfile.ComputeDerived( );

        // profile squared
        sLodProfile.Square(m_sLodProfileSquared);

        bSuccess = true;
    }
    else
		assert(false);
    //    CCore::SetError("CCore::SetLodRange, one of the near/start values exceeds its corresponding far/end value");

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CCore::ComputeLodByDistance

inline float CCore::ComputeLodByDistance(float fDistance) const
{
    float fLod = -1.0f;

    if (fDistance < m_sLodProfile.m_fHighDetail3dDistance)
        fLod = 1.0f;
    else if (fDistance < m_sLodProfile.m_fLowDetail3dDistance)
        fLod = 1.0f - (fDistance - m_sLodProfile.m_fHighDetail3dDistance) / m_sLodProfile.m_f3dRange;
    else if (fDistance < m_sLodProfile.m_fBillboardStartDistance)
        fLod = 0.0f;
    else if (fDistance < m_sLodProfile.m_fBillboardFinalDistance)
        fLod = -(fDistance - m_sLodProfile.m_fBillboardStartDistance) / m_sLodProfile.m_fBillboardRange;

    return fLod;
}


///////////////////////////////////////////////////////////////////////
//  CCore::ComputeLodByDistanceSquared

inline float CCore::ComputeLodByDistanceSquared(float fDistance) const
{
    float fLod = -1.0f;

    if (fDistance < m_sLodProfileSquared.m_fHighDetail3dDistance)
        fLod = 1.0f;
    else if (fDistance < m_sLodProfileSquared.m_fLowDetail3dDistance)
        fLod = 1.0f - (fDistance - m_sLodProfileSquared.m_fHighDetail3dDistance) / m_sLodProfileSquared.m_f3dRange;
    else if (fDistance < m_sLodProfileSquared.m_fBillboardStartDistance)
        fLod = 0.0f;
    else if (fDistance < m_sLodProfileSquared.m_fBillboardFinalDistance)
        fLod = -(fDistance - m_sLodProfileSquared.m_fBillboardStartDistance) / m_sLodProfileSquared.m_fBillboardRange;

    return fLod;
}


///////////////////////////////////////////////////////////////////////
//  CCore::ComputeShaderLerp
//
//  optimization possible - opportunities here, especially since it's done 
//  for every geometry type and those geometry types more often than not 
//  have the same # of LODs.  In the SDK, calls to this func are made from
//  ProcessCellInstances() in Forest.cpp.

inline float CCore::ComputeShaderLerp(float fLod, int nNumDiscreteLevels)
{
    if (nNumDiscreteLevels == 0)
        return 1.0f;

    float fLodClamped = max(0.0f, fLod);
    float fSpacing = 1.0f / float(nNumDiscreteLevels);

    float fMod = fLodClamped - int(fLodClamped / fSpacing) * fSpacing;

    if (fLod <= 0.0f)
        return fMod / fSpacing;
    else
        return (fMod == 0.0f) ? 1.0f : fMod / fSpacing;
}


///////////////////////////////////////////////////////////////////////
//  CCore::GetCollisionObjects

inline const SCollisionObject* CCore::GetCollisionObjects(int& nNumObjects) const
{
    nNumObjects = m_nNumCollisionObjects;
    
    return m_pCollisionObjects;
}


///////////////////////////////////////////////////////////////////////
//  CCore::UncompressVec3

inline noVec3 CCore::UncompressVec3(const unsigned char* pCompressedVector)
{
    return noVec3(float(pCompressedVector[0]) / 127.5f - 1.0f,
                float(pCompressedVector[1]) / 127.5f - 1.0f,
                float(pCompressedVector[2]) / 127.5f - 1.0f);
}


///////////////////////////////////////////////////////////////////////
//  CCore::UncompressScalar

inline float CCore::UncompressScalar(unsigned char uiCompressedScalar)
{
    return float(uiCompressedScalar) / 127.0f - 1.0f;
}




