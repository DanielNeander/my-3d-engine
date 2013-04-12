///////////////////////////////////////////////////////////////////////  
//  ShaderRI.inl
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
//  CShaderTechniqueRI::CShaderTechniqueRI

template<class TShaderTechniquePolicy>
inline CShaderTechniqueRI<TShaderTechniquePolicy>::CShaderTechniqueRI( )
{
}


///////////////////////////////////////////////////////////////////////
//  CShaderTechniqueRI::~CShaderTechniqueRI

template<class TShaderTechniquePolicy>
inline CShaderTechniqueRI<TShaderTechniquePolicy>::~CShaderTechniqueRI( )
{
}


///////////////////////////////////////////////////////////////////////
//  CShaderTechniqueRI::Bind

template<class TShaderTechniquePolicy>
inline bool CShaderTechniqueRI<TShaderTechniquePolicy>::Bind(unsigned int uiPass)
{
    bool bSuccess = false;

#ifndef NDEBUG
    if (IsValid( ))
    {
#endif
        bSuccess = m_tShaderTechniquePolicy.Bind(uiPass);
#ifndef NDEBUG
    }
#endif

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CShaderTechniqueRI::UnBind

template<class TShaderTechniquePolicy>
inline bool CShaderTechniqueRI<TShaderTechniquePolicy>::UnBind(void)
{
    bool bSuccess = false;

#ifndef NDEBUG
    if (IsValid( ))
    {
#endif
        bSuccess = m_tShaderTechniquePolicy.UnBind( );
#ifndef NDEBUG
    }
#endif

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CShaderTechniqueRI::CommitConstants

template<class TShaderTechniquePolicy>
inline bool CShaderTechniqueRI<TShaderTechniquePolicy>::CommitConstants(void)
{
    bool bSuccess = false;

#ifndef NDEBUG
    if (IsValid( ))
    {
#endif
        bSuccess = m_tShaderTechniquePolicy.CommitConstants( );
#ifndef NDEBUG
    }
#endif

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CShaderTechniqueRI::CommitTextures

template<class TShaderTechniquePolicy>
inline bool CShaderTechniqueRI<TShaderTechniquePolicy>::CommitTextures(void)
{
    bool bSuccess = false;

#ifndef NDEBUG
    if (IsValid( ))
    {
#endif
        bSuccess = m_tShaderTechniquePolicy.CommitTextures( );
#ifndef NDEBUG
    }
#endif

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CShaderTechniqueRI::IsValid

template<class TShaderTechniquePolicy>
inline bool CShaderTechniqueRI<TShaderTechniquePolicy>::IsValid(void) const
{
    return m_tShaderTechniquePolicy.IsValid( );
}


///////////////////////////////////////////////////////////////////////
//  CShaderTechniqueRI::GetName

template<class TShaderTechniquePolicy>
inline const char* CShaderTechniqueRI<TShaderTechniquePolicy>::GetName(void) const
{
    return m_strName.c_str( );
}


///////////////////////////////////////////////////////////////////////
//  CShaderTechniqueRI::SetName

template<class TShaderTechniquePolicy>
inline void CShaderTechniqueRI<TShaderTechniquePolicy>::SetName(const char* pName)
{
    assert(pName);
    m_strName = pName;
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::CShaderConstantRI

template<class TShaderConstantPolicy>
inline CShaderConstantRI<TShaderConstantPolicy>::CShaderConstantRI( )
{
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::~CShaderConstantRI

template<class TShaderConstantPolicy>
inline CShaderConstantRI<TShaderConstantPolicy>::~CShaderConstantRI( )
{
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::Set1f

template<class TShaderConstantPolicy>
inline bool CShaderConstantRI<TShaderConstantPolicy>::Set1f(float x) const
{
    return m_tShaderConstantPolicy.Set1f(x);
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::Set2f

template<class TShaderConstantPolicy>
inline bool CShaderConstantRI<TShaderConstantPolicy>::Set2f(float x, float y) const
{
    return m_tShaderConstantPolicy.Set2f(x, y);
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::Set2fv

template<class TShaderConstantPolicy>
inline bool CShaderConstantRI<TShaderConstantPolicy>::Set2fv(const float afValues[2]) const
{
    return m_tShaderConstantPolicy.Set2fv(afValues);
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::Set3f

template<class TShaderConstantPolicy>
inline bool CShaderConstantRI<TShaderConstantPolicy>::Set3f(float x, float y, float z) const
{
    return m_tShaderConstantPolicy.Set3f(x, y, z);
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::Set3fv

template<class TShaderConstantPolicy>
inline bool CShaderConstantRI<TShaderConstantPolicy>::Set3fv(const float afValues[3]) const
{
    return m_tShaderConstantPolicy.Set3fv(afValues);
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::Set3fvPlus1f

template<class TShaderConstantPolicy>
inline bool CShaderConstantRI<TShaderConstantPolicy>::Set3fvPlus1f(const float afValues[3], float w) const
{
    return m_tShaderConstantPolicy.Set3fvPlus1f(afValues, w);
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::Set4f

template<class TShaderConstantPolicy>
inline bool CShaderConstantRI<TShaderConstantPolicy>::Set4f(float x, float y, float z, float w) const
{
    return m_tShaderConstantPolicy.Set4f(x, y, z, w);
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::Set4fv

template<class TShaderConstantPolicy>
inline bool CShaderConstantRI<TShaderConstantPolicy>::Set4fv(const float afValues[4]) const
{
    return m_tShaderConstantPolicy.Set4fv(afValues);
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::SetArray4f

template<class TShaderConstantPolicy>
inline bool CShaderConstantRI<TShaderConstantPolicy>::SetArray4f(const float* pArray, int nSize, int nOffset) const
{
    return m_tShaderConstantPolicy.SetArray4f(pArray, nSize, nOffset);
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::SetMatrix

template<class TShaderConstantPolicy>
inline bool CShaderConstantRI<TShaderConstantPolicy>::SetMatrix(const float afMatrix[16]) const
{
    return m_tShaderConstantPolicy.SetMatrix(afMatrix);
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::SetMatrixTranspose

template<class TShaderConstantPolicy>
inline bool CShaderConstantRI<TShaderConstantPolicy>::SetMatrixTranspose(const float afMatrix[16]) const
{
    return m_tShaderConstantPolicy.SetMatrix(afMatrix);
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::SetMatrixIndex

template<class TShaderConstantPolicy>
inline bool CShaderConstantRI<TShaderConstantPolicy>::SetMatrixIndex(const float afMatrix[16], unsigned int uiIndex) const
{
    return m_tShaderConstantPolicy.SetMatrixIndex(afMatrix, uiIndex);
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::SetMatrix4x4Array

template<class TShaderConstantPolicy>
inline bool CShaderConstantRI<TShaderConstantPolicy>::SetMatrix4x4Array(const float* pMatrixArray, unsigned int uiOffset, unsigned int uiNumMatrices) const
{
    return m_tShaderConstantPolicy.SetMatrix4x4Array(pMatrixArray, uiOffset, uiNumMatrices);
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::IsValid

template<class TShaderConstantPolicy>
inline bool CShaderConstantRI<TShaderConstantPolicy>::IsValid(void) const
{
    return m_tShaderConstantPolicy.IsValid( );
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::GetName

template<class TShaderConstantPolicy>
inline const char* CShaderConstantRI<TShaderConstantPolicy>::GetName(void) const
{
    return m_strName.c_str( );
}


///////////////////////////////////////////////////////////////////////
//  CShaderConstantRI::SetName

template<class TShaderConstantPolicy>
inline void CShaderConstantRI<TShaderConstantPolicy>::SetName(const char* pName)
{
    assert(pName);
    m_strName = pName;
}


///////////////////////////////////////////////////////////////////////
//  CShaderLoaderRI::CShaderLoaderRI

template<class TShaderLoaderPolicy, class TShaderTechniqueRI, class TShaderConstantRI>
inline CShaderLoaderRI<TShaderLoaderPolicy, TShaderTechniqueRI, TShaderConstantRI>::CShaderLoaderRI( )
{
}


///////////////////////////////////////////////////////////////////////
//  CShaderLoaderRI::~CShaderLoaderRI

template<class TShaderLoaderPolicy, class TShaderTechniqueRI, class TShaderConstantRI>
inline CShaderLoaderRI<TShaderLoaderPolicy, TShaderTechniqueRI, TShaderConstantRI>::~CShaderLoaderRI( )
{
}


///////////////////////////////////////////////////////////////////////
//  CShaderLoaderRI::Load

template<class TShaderLoaderPolicy, class TShaderTechniqueRI, class TShaderConstantRI>
inline bool CShaderLoaderRI<TShaderLoaderPolicy, TShaderTechniqueRI, TShaderConstantRI>::Load(const char* pFilename, const CArray<CString>& aDefines)
{
    bool bSuccess = false;

    // aDefines array must have an even number of entries (the define name and then the value)
    if (aDefines.size( ) % 2 == 0)
    {
        bSuccess = m_tShaderLoaderPolicy.Load(pFilename, aDefines);
        if (!bSuccess)
            CCore::SetError("Shader compilation error, %s\n", GetError( ));
    }
    else
        CCore::SetError("CShaderLoaderRI::Load, array of defines must have an even number of elements");

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CShaderLoaderRI::GetError

template<class TShaderLoaderPolicy, class TShaderTechniqueRI, class TShaderConstantRI>
inline const char* CShaderLoaderRI<TShaderLoaderPolicy, TShaderTechniqueRI, TShaderConstantRI>::GetError(void) const
{
    return m_tShaderLoaderPolicy.GetError( );
}


///////////////////////////////////////////////////////////////////////
//  CShaderLoaderRI::GetTechnique

template<class TShaderLoaderPolicy, class TShaderTechniqueRI, class TShaderConstantRI>
inline bool CShaderLoaderRI<TShaderLoaderPolicy, TShaderTechniqueRI, TShaderConstantRI>::GetTechnique(const char* pName, TShaderTechniqueRI& tTechnique) const
{
    tTechnique.SetName(pName);

    bool bSuccess = m_tShaderLoaderPolicy.GetShader(pName, tTechnique);

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CShaderLoaderRI::GetConstant

template<class TShaderLoaderPolicy, class TShaderTechniqueRI, class TShaderConstantRI>
inline bool CShaderLoaderRI<TShaderLoaderPolicy, TShaderTechniqueRI, TShaderConstantRI>::GetConstant(const char* pName, TShaderConstantRI& tConstant) const
{
    assert(pName);
    tConstant.SetName(pName);

    bool bSuccess = m_tShaderLoaderPolicy.GetConstant(pName, tConstant);

    if (!bSuccess)
        //CCore::SetError("Failed to find shader variable [%s]\n", pName);
		assert(false);

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CShaderLoaderRI::Release

template<class TShaderLoaderPolicy, class TShaderTechniqueRI, class TShaderConstantRI>
inline void CShaderLoaderRI<TShaderLoaderPolicy, TShaderTechniqueRI, TShaderConstantRI>::Release(void)
{
    m_tShaderLoaderPolicy.Release( );
}


///////////////////////////////////////////////////////////////////////
//  CShaderLoaderRI::OnResetDevice

template<class TShaderLoaderPolicy, class TShaderTechniqueRI, class TShaderConstantRI>
inline void CShaderLoaderRI<TShaderLoaderPolicy, TShaderTechniqueRI, TShaderConstantRI>::OnResetDevice(void)
{
    m_tShaderLoaderPolicy.OnResetDevice( );
}


///////////////////////////////////////////////////////////////////////
//  CShaderLoaderRI::OnLostDevice

template<class TShaderLoaderPolicy, class TShaderTechniqueRI, class TShaderConstantRI>
inline void CShaderLoaderRI<TShaderLoaderPolicy, TShaderTechniqueRI, TShaderConstantRI>::OnLostDevice(void)
{
    m_tShaderLoaderPolicy.OnLostDevice( );
}
