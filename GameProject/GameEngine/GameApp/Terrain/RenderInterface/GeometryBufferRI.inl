///////////////////////////////////////////////////////////////////////  
//  GeometryBufferRI.inl
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
//  SVertexAttribDesc::SizeOfDataType

inline unsigned int SVertexAttribDesc::SizeOfDataType(void) const
{
    ///////////////////////////////////////////////////////////////////////
    //  Size table
    //
    //  Size in bytes of each of the data types

    static unsigned int auiSizeTable[VERTEX_ATTRIB_TYPE_COUNT] = 
    {
        1, // VERTEX_ATTRIB_TYPE_UNSIGNED_BYTE,
        1, // VERTEX_ATTRIB_TYPE_UNSIGNED_BYTE_NORMALIZED,
        2, // VERTEX_ATTRIB_TYPE_SHORT,
        2, // VERTEX_ATTRIB_TYPE_SHORT_NORMALIZED,
        2, // VERTEX_ATTRIB_TYPE_UNSIGNED_SHORT,
        2, // VERTEX_ATTRIB_TYPE_UNSIGNED_SHORT_NORMALIZED,
        4, // VERTEX_ATTRIB_TYPE_INT,
        4, // VERTEX_ATTRIB_TYPE_INT_NORMALIZED,
        4, // VERTEX_ATTRIB_TYPE_UNSIGNED_INT,
        4, // VERTEX_ATTRIB_TYPE_UNSIGNED_INT_NORMALIZED,
        4, // VERTEX_ATTRIB_TYPE_FLOAT,
        8  // VERTEX_ATTRIB_TYPE_DOUBLE,
    };

    return auiSizeTable[m_eDataType];
}


///////////////////////////////////////////////////////////////////////  
//  SVertexAttribDesc::SizeOfAttrib

inline unsigned int SVertexAttribDesc::SizeOfAttrib(void) const
{
    return m_uiNumElements * SizeOfDataType( );
}


///////////////////////////////////////////////////////////////////////  
//  SVertexAttribDesc::DataTypeName

inline const char* SVertexAttribDesc::DataTypeName(void) const
{
    static const char* aszDataTypeNamesTable[VERTEX_ATTRIB_TYPE_COUNT] =
    {
        "Unsigned Byte",
        "Unsigned Byte (Normalized)",
        "Short",
        "Short (Normalized)",
        "Unsigned Short",
        "Unsigned Short (Normalized)",
        "Integer",
        "Integer (Normalized)",
        "Unsigned Integer",
        "Unsigned Integer (Normalized)",
        "Float",
        "Double"
    };

    return aszDataTypeNamesTable[m_eDataType];
}


///////////////////////////////////////////////////////////////////////  
//  SVertexAttribDesc::SemanticName

inline const char* SVertexAttribDesc::SemanticName(void) const
{
    static const char* aszSemanticNamesTable[VERTEX_ATTRIB_SEMANTIC_COUNT] =
    {
        "Position",
        "Blend Weight",
        "Normal",
        "Diffuse Color",
        "Specular Color",
        "Tessellation Factor",
        "Point Size",
        "Blend Indices",
        "Texture Coords, Layer 0",
        "Texture Coords, Layer 1",
        "Texture Coords, Layer 2",
        "Texture Coords, Layer 3",
        "Texture Coords, Layer 4",
        "Texture Coords, Layer 5",
        "Texture Coords, Layer 6",
        "Texture Coords, Layer 7"
    };

    return aszSemanticNamesTable[m_eSemantic];
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferRI::CGeometryBufferRI

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::CGeometryBufferRI( )
    : m_uiVertexSize(0)
    , m_uiNumVertices(0)
#ifndef NDEBUG
    , m_bFormatEnabled(false)
    , m_bVertexBufferBound(false)
    , m_bIndexBufferBound(false)
#endif
    , m_uiIndexSize(4)
    , m_uiNumIndices(0)
{
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferRI::CGeometryBufferRI

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::~CGeometryBufferRI( )
{
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferRI::SetVertexFormat

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::SetVertexFormat(const SVertexAttribDesc* pAttribDesc, const TShaderTechniqueRI* pTechnique, bool bDynamic)
{
    bool bSuccess = false;

    // compute vertex size
    m_uiVertexSize = 0;
    assert(pAttribDesc);
    const SVertexAttribDesc* pAttrib = pAttribDesc;
    while (pAttrib->m_eSemantic != VERTEX_ATTRIB_SEMANTIC_END)
    {
        m_uiVertexSize += pAttrib->SizeOfAttrib( );
        ++pAttrib;
    }

    if (m_uiVertexSize > 0)
    {
        // graphics API needs to know about the format
        bSuccess = m_tGeometryBufferPolicy.SetVertexFormat(pAttribDesc, pTechnique, bDynamic);
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::AppendVertices

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::AppendVertices(const void* pVertexData, unsigned int uiNumVertices)
{
    bool bSuccess = false;

#ifndef NDEBUG
    if (m_uiVertexSize > 0)
    {
        if (pVertexData && uiNumVertices > 0)
        {
#endif
            size_t szStartIndex = m_aVertexData.size( );
            m_aVertexData.resize(szStartIndex + m_uiVertexSize * uiNumVertices);

            memcpy(&m_aVertexData[szStartIndex], pVertexData, uiNumVertices * m_uiVertexSize);
            m_uiNumVertices += uiNumVertices;

            bSuccess = true;
#ifndef NDEBUG
        }
    }
    else
        CCore::SetError("CGeometryBufferRI::AppendVertices, SetFormat() must be called before AppendVertices");
#endif

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::EndVertices

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::EndVertices(void)
{
    bool bSuccess = false;

#ifndef NDEBUG
    if (m_uiVertexSize > 0 && !m_aVertexData.empty( ))
    {
#endif
        bSuccess = m_tGeometryBufferPolicy.CreateVertexBuffer(&m_aVertexData[0], unsigned int(m_aVertexData.size( ) / m_uiVertexSize), m_uiVertexSize);

        // don't need the client-side copy since the buffer's been created
        m_aVertexData.clear( );
#ifndef NDEBUG
    }
    else
        CCore::SetError("CGeometryBufferRI::EndVertices, SetFormat() and AppendVertices() must be called before EndVertices");
#endif

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::ReplaceVertices

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::ReplaceVertices(const void* pVertexData, unsigned int uiNumVertices)
{
    bool bSuccess = false;

#ifndef NDEBUG
    if (pVertexData && uiNumVertices > 0)
    {
#endif
        // if vertex buffer is in place, replace the buffer contents
        if (m_tGeometryBufferPolicy.VertexBufferIsValid( ))
        {
            bSuccess = m_tGeometryBufferPolicy.ReplaceVertices(pVertexData, uiNumVertices, m_uiVertexSize);
        }
        else // no vertex buffer is in place, must use the append functions
        {
            if (AppendVertices(pVertexData, uiNumVertices))
                bSuccess = EndVertices( );
        }
#ifndef NDEBUG
    }
#endif

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::NumVertices

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline unsigned int CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::NumVertices(void) const
{
    return m_uiNumVertices;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::VertexSize

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline unsigned int CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::VertexSize(void) const
{
    return m_uiVertexSize;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::EnableFormat

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::EnableFormat(void) const
{
    bool bSuccess = false;

#ifndef NDEBUG
    // no need to check IsFormatSet() since VertexBufferIsValid can't return true is the format isn't already set
    if (m_uiVertexSize > 0 && m_tGeometryBufferPolicy.VertexBufferIsValid( ))
    {
#endif

        bSuccess = m_tGeometryBufferPolicy.EnableFormat( );

#ifndef NDEBUG
        m_bFormatEnabled = true;
    }
    else
        CCore::SetError("CGeometryBufferRI::EnableFormat, the vertex buffer has not been setup yet");
#endif

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::DisableFormat

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::DisableFormat(void)
{
    return TGeometryBufferPolicy::DisableFormat( );
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::BindVertexBuffer

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::BindVertexBuffer(void) const
{
    bool bSuccess = false;

#ifndef NDEBUG
    if (m_uiVertexSize > 0 && m_tGeometryBufferPolicy.VertexBufferIsValid( ))
    {
#endif
        bSuccess = m_tGeometryBufferPolicy.BindVertexBuffer(m_uiVertexSize);

#ifndef NDEBUG
        if (bSuccess)
            m_bVertexBufferBound = true;
    }
    else
        CCore::SetError("CGeometryBufferRI::BindVertexBuffer, the vertex buffer has not been setup yet");
#endif

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::UnBindVertexBuffer

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::UnBindVertexBuffer(void)
{
    return TGeometryBufferPolicy::UnBindVertexBuffer( );
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::SetIndexFormat

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::SetIndexFormat(EIndexFormat eFormat)
{
    if (eFormat == INDEX_FORMAT_UNSIGNED_16BIT)
        m_uiIndexSize = 2;
    else 
        m_uiIndexSize = 4;

    return m_tGeometryBufferPolicy.SetIndexFormat(eFormat);
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::AppendIndices

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::AppendIndices(const void* pIndexData, unsigned int uiNumIndices)
{
    bool bSuccess = false;

#ifndef NDEBUG
    if (pIndexData && uiNumIndices > 0)
    {
#endif
        if (m_uiIndexSize > 0)
        {
            // grow the internal representation by the amount requested to append
            size_t szStartIndex = m_aIndexData.size( );
            m_aIndexData.resize(szStartIndex + uiNumIndices * m_uiIndexSize);

            // copy into the array
            memcpy(&m_aIndexData[szStartIndex], pIndexData, uiNumIndices * m_uiIndexSize);

            // m_uiNumIndices is used instead of m_aIndexData.size() because m_aIndexData will
            // be cleared after EndIndices() is called
            m_uiNumIndices += uiNumIndices;

            bSuccess = true;
        }
        else
            CCore::SetError("CGeometryBufferRI::AppendIndices, index buffer format must be set first");
#ifndef NDEBUG
    }
#endif

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::ReplaceIndices

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::ReplaceIndices(const void* pIndexData, unsigned int uiNumIndices)
{
    bool bSuccess = false;

#ifndef NDEBUG
    if (pIndexData && uiNumIndices > 0)
    {
#endif
        // if index buffer is in place, replace the buffer contents
        if (m_tGeometryBufferPolicy.IndexBufferIsValid( ))
        {
            bSuccess = m_tGeometryBufferPolicy.ReplaceIndices(pIndexData, uiNumIndices);
        }
        else // no index buffer is in place, must use the append functions
        {
            if (AppendIndices(pIndexData, uiNumIndices))
                bSuccess = EndIndices( );
        }
#ifndef NDEBUG
    }
#endif

    if (bSuccess)
        m_uiNumIndices = uiNumIndices;

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::EndIndices

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::EndIndices(void)
{
    bool bSuccess = false;

#ifndef NDEBUG
    if (!m_aIndexData.empty( ))
    {
#endif
        m_uiNumIndices = unsigned int(m_aIndexData.size( ) / m_uiIndexSize);
        bSuccess = m_tGeometryBufferPolicy.CreateIndexBuffer(&m_aIndexData[0], m_uiNumIndices);
        
        // don't need the client-side copy since the buffer's been created
        m_aIndexData.clear( );
#ifndef NDEBUG
    }
    else
        CCore::SetError("CGeometryBufferRI::EndIndices, AppendIndices() must be called before EndVertices");
#endif

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::NumIndices

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline unsigned int CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::NumIndices(void) const
{
    return m_uiNumIndices;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::BindIndexBuffer

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::BindIndexBuffer(void) const
{
    bool bSuccess = false;

#ifndef NDEBUG
    if (m_tGeometryBufferPolicy.IndexBufferIsValid( ))
    {
#endif

        bSuccess = m_tGeometryBufferPolicy.BindIndexBuffer( );

#ifndef NDEBUG
        if (bSuccess)
            m_bIndexBufferBound = true;
    }
#endif

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::UnBindIndexBuffer

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::UnBindIndexBuffer(void) const
{
    bool bSuccess = false;

#ifndef NDEBUG
    if (m_tGeometryBufferPolicy.IndexBufferIsValid( ))
    {
#endif

        bSuccess = m_tGeometryBufferPolicy.BindIndexBuffer( );

#ifndef NDEBUG
        if (bSuccess)
            m_bIndexBufferBound = false;
    }
#endif

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::RenderIndexed

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::RenderIndexed(EPrimitiveType ePrimType, unsigned int uiStartIndex, unsigned int uiNumIndices, unsigned int uiNumVerticesOverride) const
{
    return m_tGeometryBufferPolicy.RenderIndexed(ePrimType, uiStartIndex, uiNumIndices, uiNumVerticesOverride);
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::RenderArrays

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::RenderArrays(EPrimitiveType ePrimType, unsigned int uiStartVertex, unsigned int uiNumVertices) const
{
    bool bSuccess = false;

#ifndef NDEBUG
    if (m_bVertexBufferBound)
    {
#endif

        bSuccess = m_tGeometryBufferPolicy.RenderArrays(ePrimType, uiStartVertex, uiNumVertices);

#ifndef NDEBUG
    }
    else
        CCore::SetError("CGeometryBufferRI::RenderArrays, the vertex buffer is not bound");
#endif

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CGeometryBufferRI::IsPrimitiveTypeSupported

template<class TGeometryBufferPolicy, class TShaderTechniqueRI>
inline bool CGeometryBufferRI<TGeometryBufferPolicy, TShaderTechniqueRI>::IsPrimitiveTypeSupported(EPrimitiveType ePrimType)
{
    return TGeometryBufferPolicy::IsPrimitiveTypeSupported(ePrimType);
}

