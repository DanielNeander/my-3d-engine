///////////////////////////////////////////////////////////////////////  
//  Instance.inl
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
//  CInstance::SetPos

inline void CInstance::SetPos(const noVec3& vPos)
{
    m_vPos = vPos;
}


///////////////////////////////////////////////////////////////////////  
//  CInstance::SetScale

inline void CInstance::SetScale(float fScale)
{
    m_fScale = fScale;
}


///////////////////////////////////////////////////////////////////////  
//  CInstance::SetRotation

inline void CInstance::SetRotation(float fRadians)
{
    m_fRotation = fRadians;

    // can't use a negative rotation value (billboard shader optimization)
    if (fRadians < 0.0f)
        fRadians = fmod(fRadians, noMath::TWO_PI) + noMath::TWO_PI;

    m_fRotation = fRadians;
    m_afRotationVector[0] = sinf(fRadians);    
    m_afRotationVector[1] = cosf(fRadians);    
    m_afRotationVector[2] = -m_afRotationVector[0];
}


///////////////////////////////////////////////////////////////////////  
//  CInstance::GetPos

inline const noVec3& CInstance::GetPos(void) const
{
    return m_vPos;
}


///////////////////////////////////////////////////////////////////////  
//  CInstance::GetScale

inline float CInstance::GetScale(void) const
{
    return m_fScale;
}


///////////////////////////////////////////////////////////////////////  
//  CInstance::GetRotationAngle

inline float CInstance::GetRotationAngle(void) const
{
    return m_fRotation;
}


///////////////////////////////////////////////////////////////////////  
//  CInstance::GetRotationVector

inline const float* CInstance::GetRotationVector(void) const
{
    return m_afRotationVector;
}


///////////////////////////////////////////////////////////////////////  
//  CInstance::GetGeometricCenter

inline const noVec3& CInstance::GetGeometricCenter(void) const
{
    return m_vGeometricCenter;
}


///////////////////////////////////////////////////////////////////////  
//  CInstance::GetCullingRadius

inline float CInstance::GetCullingRadius(void) const
{
    return m_fCullingRadius;
}


///////////////////////////////////////////////////////////////////////
//  CInstance::operator<

inline bool CInstance::operator<(const CInstance& cIn) const
{
    if (m_vPos == cIn.m_vPos)
    {
        if (m_fScale == cIn.m_fScale)
            return m_fRotation < cIn.m_fRotation;
        else
            return m_fScale < cIn.m_fScale;
    }
    else
        return m_vPos < cIn.m_vPos;
}


///////////////////////////////////////////////////////////////////////
//  CInstance::operator==

inline bool CInstance::operator==(const CInstance& cIn) const
{
    return (m_vPos == cIn.m_vPos &&
            m_fScale == cIn.m_fScale &&
            m_fRotation == cIn.m_fRotation);
}

