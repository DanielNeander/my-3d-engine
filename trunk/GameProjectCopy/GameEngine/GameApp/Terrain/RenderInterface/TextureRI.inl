///////////////////////////////////////////////////////////////////////  
//  TextureRI.inl
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
//  CTextureRI::Load

template<class TTexturePolicy>
inline bool CTextureRI<TTexturePolicy>::Load(const char* pFilename, int nMaxAnisotropy)
{
    if (IsValid( ))
        Unload( );

    if (m_tTexturePolicy.Load(pFilename, nMaxAnisotropy))
        m_strFilename = pFilename;
    else
        CCore::SetError("Failed to load texture [%s]\n", pFilename);

    return IsValid( );
}


///////////////////////////////////////////////////////////////////////  
//  CTextureRI::LoadColor

template<class TTexturePolicy>
inline bool CTextureRI<TTexturePolicy>::LoadColor(unsigned int uiColor)
{
    if (IsValid( ))
        Unload( );

    if (m_tTexturePolicy.LoadColor(uiColor))
        m_strFilename = CString::Format("Color_%x", uiColor).c_str( );

    return IsValid( );
}


///////////////////////////////////////////////////////////////////////  
//  CTextureRI::LoadAlphaNoise

template<class TTexturePolicy>
inline bool CTextureRI<TTexturePolicy>::LoadAlphaNoise(int nWidth, int nHeight)
{
    if (IsValid( ))
        Unload( );

    if (m_tTexturePolicy.LoadAlphaNoise(nWidth, nWidth))
        m_strFilename = CString::Format("Noise_%dx%d", nWidth, nHeight).c_str( );

    return IsValid( );
}


///////////////////////////////////////////////////////////////////////  
//  CTextureRI::LoadSpecularLookup

template<class TTexturePolicy>
inline bool CTextureRI<TTexturePolicy>::LoadSpecularLookup(void)
{
    if (IsValid( ))
        Unload( );

    if (m_tTexturePolicy.LoadSpecularLookup( ))
        m_strFilename = "SpecularLookup";

    return IsValid( );
}


///////////////////////////////////////////////////////////////////////  
//  CTextureRI::LoadCubeNormalizer

template<class TTexturePolicy>
inline bool CTextureRI<TTexturePolicy>::LoadCubeNormalizer(int nResolution)
{
    if (IsValid( ))
        Unload( );

    if (m_tTexturePolicy.LoadCubeNormalizer(nResolution))
        m_strFilename = CString::Format("CubeNormalizer_%d", nResolution).c_str( );

    return IsValid( );
}


///////////////////////////////////////////////////////////////////////  
//  CTextureRI::Unload

template<class TTexturePolicy>
inline bool CTextureRI<TTexturePolicy>::Unload(void)
{
    if (m_tTexturePolicy.Unload( ))
        m_strFilename.clear( );

    return !IsValid( );
}


///////////////////////////////////////////////////////////////////////  
//  CTextureRI::FixedFunctionBind

template<class TTexturePolicy>
inline bool CTextureRI<TTexturePolicy>::FixedFunctionBind(void)
{
    return m_tTexturePolicy.FixedFunctionBind( );
}


///////////////////////////////////////////////////////////////////////  
//  CTextureRI::GetPolicy

template<class TTexturePolicy>
inline const TTexturePolicy& CTextureRI<TTexturePolicy>::GetPolicy(void) const
{
    return m_tTexturePolicy;
}


///////////////////////////////////////////////////////////////////////  
//  CTextureRI::GetFilename

template<class TTexturePolicy>
inline const char* CTextureRI<TTexturePolicy>::GetFilename(void) const
{
    return m_strFilename.c_str( );
}


///////////////////////////////////////////////////////////////////////  
//  CTextureRI::IsValid

template<class TTexturePolicy>
inline bool CTextureRI<TTexturePolicy>::IsValid(void) const
{
    return !m_strFilename.empty( );
}
