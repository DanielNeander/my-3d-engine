///////////////////////////////////////////////////////////////////////  
//  TreeRI.inl
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
//  Constants

const st_float32 c_fUncompressWindScalar = 10.0f;


///////////////////////////////////////////////////////////////////////
//  CTreeRI::CTreeRI

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline CTreeRI_t::CTreeRI( ) :
    m_bCardsAreQuads(true),
    m_pSearchPaths(NULL),
    m_bGraphicsInitialized(false)
{
}


///////////////////////////////////////////////////////////////////////
//  CTreeRI::~CTreeRI

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline CTreeRI_t::~CTreeRI( )
{
}


///////////////////////////////////////////////////////////////////////
//  CTreeRI::InitGraphics

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline st_bool CTreeRI_t::InitGraphics(const CArray<CFixedString>& aSearchPaths, st_bool bQuadsSupported, const TShaderTechniquePolicy** pTechniques, st_int32 nMaxAnisotropy)
{
    st_bool bSuccess = false;

    if (!m_bGraphicsInitialized)
    {
        // can't proceed if the geometry doesn't exist
        const SGeometry* pGeometry = GetGeometry( );
        if (pGeometry)
        {
            bSuccess = InitMaterials(pGeometry, aSearchPaths, nMaxAnisotropy);
            bSuccess &= InitBranches(pGeometry, pTechniques[GEOMETRY_TYPE_BRANCHES]);
            bSuccess &= InitFronds(pGeometry, pTechniques[GEOMETRY_TYPE_FRONDS]);
            bSuccess &= InitLeafCards(pGeometry, bQuadsSupported, pTechniques[GEOMETRY_TYPE_LEAF_CARDS]);
            bSuccess &= InitLeafMeshes(pGeometry, pTechniques[GEOMETRY_TYPE_LEAF_MESHES]);

            if (!bSuccess)
                CCore::SetError("CTreeRI::InitGraphics, failed to initialize material and/or geometry");
        }
        else
            CCore::SetError("CTreeRI::InitGraphics, no geometry present (CCore::DeleteGeometry called too early?");
        
        m_bGraphicsInitialized = bSuccess;
    }
    else
    {
        CCore::SetError("CTreeRI::InitGraphics called redundantly");
        bSuccess = true;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CTreeRI::GraphicsAreInitialized

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline st_bool CTreeRI_t::GraphicsAreInitialized(void) const
{
    return m_bGraphicsInitialized;
}


///////////////////////////////////////////////////////////////////////
//  CTreeRI::GetGeometryBuffer

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline const TGeometryBufferPolicy* CTreeRI_t::GetGeometryBuffer(EGeometryType eType) const
{
    assert(eType >= GEOMETRY_TYPE_BRANCHES && eType <= GEOMETRY_TYPE_LEAF_MESHES);

    return m_atGeometryBuffers + eType;
}


///////////////////////////////////////////////////////////////////////
//  CTreeRI::GetMaterials

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline const CArray<SMaterialRI_t>* CTreeRI_t::GetMaterials(void) const
{
    return &m_aMaterials;
}


///////////////////////////////////////////////////////////////////////
//  CTreeRI::GetLodRenderInfo

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline const CLodRenderInfo* CTreeRI_t::GetLodRenderInfo(EGeometryType eType) const
{
    return m_acRenderInfo + eType;
}


///////////////////////////////////////////////////////////////////////
//  CTreeRI::CardsAreQuads

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline st_bool CTreeRI_t::CardsAreQuads(void) const
{
    return m_bCardsAreQuads;
}


///////////////////////////////////////////////////////////////////////
//  CTreeRI::InitMaterials

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline st_bool CTreeRI_t::InitMaterials(const SGeometry* pGeometry, const CArray<CFixedString>& aSearchPaths, int nMaxAnisotropy)
{
    assert(pGeometry);
    st_bool bSuccess = true;

    // check to see if any materials are stored
    if (pGeometry->m_nNumMaterials > 0)
    {
        // keep copy of the materials in CTreeRI that will be modified to hold the full path names of the textures
        // (determined after the path search), not just the original filename
        m_aMaterials.resize(pGeometry->m_nNumMaterials);

        // run through each material, initializing its textures
        for (st_int32 nMaterial = 0; nMaterial < pGeometry->m_nNumMaterials; ++nMaterial)
        {
            assert(pGeometry->m_pMaterials);
            m_aMaterials[nMaterial].Copy(pGeometry->m_pMaterials[nMaterial]);
            SMaterialRI_t* pMaterial = &m_aMaterials[nMaterial];

            // each material has TL_NUM_TEX_LAYERS layers; initialize each if present
            for (st_int32 nTexture = 0; nTexture < TL_NUM_TEX_LAYERS; ++nTexture)
            {
                const char* pFilename = pMaterial->m_astrTextureFilenames[nTexture].c_str( );
                if (pFilename && strlen(pFilename) > 0)
                {
                    // look for the texture at each location in the search path
                    st_bool bFound = false;
                    for (st_int32 nSearchPath = 0; nSearchPath < st_int32(aSearchPaths.size( )) && !bFound; ++nSearchPath)
                    {
                        CFixedString strSearchLocation = aSearchPaths.at(nSearchPath) + CFixedString(pFilename).NoPath( );

                        // if the Load() call succeeds, the texture was found
                        if (pMaterial->m_acTextureObjects[nTexture].Load(strSearchLocation.c_str( ), nMaxAnisotropy))
                        {
                            // replace the original no-path filename with one containing the full path
                            pMaterial->m_astrTextureFilenames[nTexture] = strSearchLocation;
                            printf("  [material %d, tex layer %d] - [%s]\n", nMaterial, nTexture, strSearchLocation.c_str( ));

                            bFound = true;
                            break;
                        }

                        bSuccess &= bFound;
                    }
                }
            }
        }
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  struct SBranchVertex

struct SBranchVertex
{
    st_float32  m_afCoord[3], m_fDiffuseTexCoordS;      // position
    st_float32  m_afNormal[3];                          // normal
    st_float32  m_afTangent[3], m_fDiffuseTexCoordT;    // texcoord0
    st_float32  m_afBinormal[3], m_fDetailTexCoordS;    // texcoord1
    st_float32  m_fAmbOcc, m_fWindScalar;               // texcoord2
    st_float32  m_afLodCoord[3], m_fDetailTexCoordT;    // texcoord3
    st_float32  m_afWindData[4];                        // texcoord4
};


///////////////////////////////////////////////////////////////////////
//  CTreeRI::InitBranches

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline st_bool CTreeRI_t::InitBranches(const SGeometry* pGeometry, const TShaderTechniquePolicy* pTechnique)
{
    assert(pGeometry);
    st_bool bSuccess = true;

    // setup some aliases
    const st_int32 c_nNumLods = pGeometry->m_nNumBranchLods;
    const SIndexedTriangles* pLods = pGeometry->m_pBranchLods;
    TGeometryBufferPolicy* pGeometryBuffer = m_atGeometryBuffers + GEOMETRY_TYPE_BRANCHES;

    if (c_nNumLods > 0 && pLods) // may not have branch geometry in this model
    {
        // setup vertex structure
        SVertexAttribDesc asBranchVertexFormat[ ] =
        {
            { VERTEX_ATTRIB_SEMANTIC_POS, VERTEX_ATTRIB_TYPE_FLOAT, 4 },        // pos.xyz + diffuse_tex_s
            { VERTEX_ATTRIB_SEMANTIC_NORMAL, VERTEX_ATTRIB_TYPE_FLOAT, 3 },     // normal.xyz
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD0, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // tangent.xyz + diffuse_tex_t
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD1, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // binormal.xyz + detail_tex_s
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD2, VERTEX_ATTRIB_TYPE_FLOAT, 2 },  // x = ambient occlusion, y = wind scalar
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD3, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // LOD position.xyz + detail_tex_t
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD4, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // wind data.xyz
            VERTEX_ATTRIB_END( )
        };

        if (pGeometryBuffer->SetVertexFormat(asBranchVertexFormat, pTechnique) &&
            pGeometryBuffer->SetIndexFormat(INDEX_FORMAT_UNSIGNED_32BIT))
        {
            // fill out vertex and index buffers
            m_acRenderInfo[GEOMETRY_TYPE_BRANCHES].InitIndexed(pLods, c_nNumLods);
            for (st_int32 nLod = 0; nLod < c_nNumLods; ++nLod)
            {
                const SIndexedTriangles* pLod = pLods + nLod;

                // an LOD must have these values before the render path will work
                if (pLod->HasGeometry( ))
                {
                    assert(pLod->m_pNormals && pLod->m_pTexCoordsDiffuse);

                    // initialize index buffer
                    bSuccess &= InitIndexBuffer(nLod, pLod, pGeometryBuffer, m_acRenderInfo[GEOMETRY_TYPE_BRANCHES]);

                    // fill out vertex buffer
                    CArray<SBranchVertex> aVertices(pLod->m_nNumVertices);
                    SBranchVertex* pVertex = &aVertices[0];
                    for (st_int32 nVertex = 0; nVertex < pLod->m_nNumVertices; ++nVertex)
                    {
                        // coords
                        memcpy(pVertex->m_afCoord, pLod->m_pCoords + nVertex * 3, sizeof(pVertex->m_afCoord));

                        // normal
                        const st_uint8* pCompressedNormal = pLod->m_pNormals + nVertex * 3;
                        memcpy(pVertex->m_afNormal, CCore::UncompressVec3(pCompressedNormal), sizeof(pVertex->m_afNormal));

                        // diffuse texcoords
                        pVertex->m_fDiffuseTexCoordS = pLod->m_pTexCoordsDiffuse[nVertex * 2];
                        pVertex->m_fDiffuseTexCoordT = pLod->m_pTexCoordsDiffuse[nVertex * 2 + 1];

                        // detail texcoords
                        if (pLod->m_pTexCoordsDetail)
                        {
                            pVertex->m_fDetailTexCoordS = pLod->m_pTexCoordsDetail[nVertex * 2];
                            pVertex->m_fDetailTexCoordT = pLod->m_pTexCoordsDetail[nVertex * 2 + 1];
                        }
                        else
                            pVertex->m_fDetailTexCoordS = pVertex->m_fDetailTexCoordT = 0.0f;

                        // tangent
                        const st_uint8* pCompressedTangent = pLod->m_pTangents + nVertex * 3;
                        memcpy(pVertex->m_afTangent, CCore::UncompressVec3(pCompressedTangent), sizeof(pVertex->m_afTangent));

                        // binormal
                        const st_uint8* pCompressedBinormal = pLod->m_pBinormals + nVertex * 3;
                        memcpy(pVertex->m_afBinormal, CCore::UncompressVec3(pCompressedBinormal), sizeof(pVertex->m_afBinormal));

                        // ambient occlusion
                        pVertex->m_fAmbOcc = pLod->m_pAmbientOcclusionValues ? CCore::UncompressScalar(pLod->m_pAmbientOcclusionValues[nVertex]) : 1.0f;

                        // LOD value
                        memcpy(pVertex->m_afLodCoord, pLod->m_pLodCoords ? pLod->m_pLodCoords + nVertex * 3 : pLod->m_pCoords + nVertex * 3, sizeof(pVertex->m_afLodCoord));

                        // wind data
                        if (pLod->m_pWindData)
                        {
                            pVertex->m_afWindData[0] = pLod->m_fWindDataMagnitude * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 0]);
                            pVertex->m_afWindData[1] = pLod->m_fWindDataMagnitude * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 1]);
                            pVertex->m_afWindData[2] = pLod->m_fWindDataMagnitude * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 2]);
                            pVertex->m_afWindData[3] = c_fPi * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 3]);
                            pVertex->m_fWindScalar = c_fUncompressWindScalar * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 4]);
                        }
                        else
                        {
                            pVertex->m_afWindData[0] = pVertex->m_afWindData[1] = pVertex->m_afWindData[2] = pVertex->m_afWindData[3] = 0.0f;
                            pVertex->m_fWindScalar = 1.0f;
                        }

                        ++pVertex;
                    }

                    bSuccess &= pGeometryBuffer->AppendVertices(&aVertices[0], pLod->m_nNumVertices);
                }
            }
        }

        bSuccess &= pGeometryBuffer->EndVertices( );
        bSuccess &= pGeometryBuffer->EndIndices( );
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  struct SFrondVertex

struct SFrondVertex
{
    st_float32  m_afCoord[3], m_fDiffuseTexCoordS;              // position
    st_float32  m_afNormal[3];                                  // normal
    st_float32  m_afTangent[3], m_fDiffuseTexCoordT;            // texcoord0
    st_float32  m_afBinormal[3], m_fDetailTexCoordS;            // texcoord1
    st_float32  m_afFrondRipple[2], m_fAmbOcc, m_fWindScalar;   // texcoord2
    st_float32  m_afLodCoord[3], m_fDetailTexCoordT;            // texcoord3
    st_float32  m_afWindData[4];                                // texcoord4
};


///////////////////////////////////////////////////////////////////////
//  CTreeRI::InitFronds

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline st_bool CTreeRI_t::InitFronds(const SGeometry* pGeometry, const TShaderTechniquePolicy* pTechnique)
{
    assert(pGeometry);
    st_bool bSuccess = true;

    // setup some aliases
    const st_int32 c_nNumLods = pGeometry->m_nNumFrondLods;
    const SIndexedTriangles* pLods = pGeometry->m_pFrondLods;
    TGeometryBufferPolicy* pGeometryBuffer = m_atGeometryBuffers + GEOMETRY_TYPE_FRONDS;

    if (c_nNumLods > 0 && pLods) // may not have frond geometry in this model
    {
        // setup vertex structure
        SVertexAttribDesc asFrondVertexFormat[ ] =
        {
            { VERTEX_ATTRIB_SEMANTIC_POS, VERTEX_ATTRIB_TYPE_FLOAT, 4 },        // pos + diffuse_tex_s
            { VERTEX_ATTRIB_SEMANTIC_NORMAL, VERTEX_ATTRIB_TYPE_FLOAT, 3 },     // normal
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD0, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // tangent + diffuse_tex_t
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD1, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // binormal + detail_tex_s
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD2, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // xy = frond ripple, z = ambient occlusion, w = wind scalar
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD3, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // LOD position + detail_tex_t
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD4, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // wind data
            VERTEX_ATTRIB_END( )
        };

        if (pGeometryBuffer->SetVertexFormat(asFrondVertexFormat, pTechnique) &&
            pGeometryBuffer->SetIndexFormat(INDEX_FORMAT_UNSIGNED_32BIT))
        {
            // fill out vertex and index buffers
            m_acRenderInfo[GEOMETRY_TYPE_FRONDS].InitIndexed(pLods, c_nNumLods);
            for (st_int32 nLod = 0; nLod < c_nNumLods; ++nLod)
            {
                const SIndexedTriangles* pLod = pLods + nLod;

                // an LOD must have these values before the render path will work
                if (pLod->HasGeometry( ))
                {
                    assert(pLod->m_pNormals && pLod->m_pTexCoordsDiffuse);

                    // initialize index buffer
                    bSuccess &= InitIndexBuffer(nLod, pLod, pGeometryBuffer, m_acRenderInfo[GEOMETRY_TYPE_FRONDS]);

                    // fill out vertex buffer
                    CArray<SFrondVertex> aVertices(pLod->m_nNumVertices);
                    SFrondVertex* pVertex = &aVertices[0];
                    for (st_int32 nVertex = 0; nVertex < pLod->m_nNumVertices; ++nVertex)
                    {
                        // coords
                        memcpy(pVertex->m_afCoord, pLod->m_pCoords + nVertex * 3, sizeof(pVertex->m_afCoord));

                        // normal
                        const st_uint8* pCompressedNormal = pLod->m_pNormals + nVertex * 3;
                        memcpy(pVertex->m_afNormal, CCore::UncompressVec3(pCompressedNormal), sizeof(pVertex->m_afNormal));

                        // diffuse texcoords
                        pVertex->m_fDiffuseTexCoordS = pLod->m_pTexCoordsDiffuse[nVertex * 2];
                        pVertex->m_fDiffuseTexCoordT = pLod->m_pTexCoordsDiffuse[nVertex * 2 + 1];

                        // detail texcoords
                        if (pLod->m_pTexCoordsDetail)
                        {
                            pVertex->m_fDetailTexCoordS = pLod->m_pTexCoordsDetail[nVertex * 2];
                            pVertex->m_fDetailTexCoordT = pLod->m_pTexCoordsDetail[nVertex * 2 + 1];
                        }
                        else
                            pVertex->m_fDetailTexCoordS = pVertex->m_fDetailTexCoordT = 0.0f;

                        // tangent
                        const st_uint8* pCompressedTangent = pLod->m_pTangents + nVertex * 3;
                        memcpy(pVertex->m_afTangent, CCore::UncompressVec3(pCompressedTangent), sizeof(pVertex->m_afTangent));

                        // binormal
                        const st_uint8* pCompressedBinormal = pLod->m_pBinormals + nVertex * 3;
                        memcpy(pVertex->m_afBinormal, CCore::UncompressVec3(pCompressedBinormal), sizeof(pVertex->m_afBinormal));

                        // frond ripple / ambient occlusion
                        pVertex->m_afFrondRipple[0] = pLod->m_pFrondRipple[nVertex * 2 + 0];
                        pVertex->m_afFrondRipple[1] = pLod->m_pFrondRipple[nVertex * 2 + 1];
                        pVertex->m_fAmbOcc = pLod->m_pAmbientOcclusionValues ? CCore::UncompressScalar(pLod->m_pAmbientOcclusionValues[nVertex]) : 1.0f;

                        // LOD value
                        memcpy(pVertex->m_afLodCoord, pLod->m_pLodCoords ? pLod->m_pLodCoords + nVertex * 3 : pLod->m_pCoords + nVertex * 3, sizeof(pVertex->m_afLodCoord));

                        // wind data
                        if (pLod->m_pWindData)
                        {
                            pVertex->m_afWindData[0] = pLod->m_fWindDataMagnitude * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 0]);
                            pVertex->m_afWindData[1] = pLod->m_fWindDataMagnitude * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 1]);
                            pVertex->m_afWindData[2] = pLod->m_fWindDataMagnitude * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 2]);
                            pVertex->m_afWindData[3] = c_fPi * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 3]);
                            pVertex->m_fWindScalar = c_fUncompressWindScalar * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 4]);
                        }
                        else
                        {
                            pVertex->m_afWindData[0] = pVertex->m_afWindData[1] = pVertex->m_afWindData[2] = pVertex->m_afWindData[3] = 0.0f;
                            pVertex->m_fWindScalar = 1.0f;
                        }

                        ++pVertex;
                    }

                    bSuccess &= pGeometryBuffer->AppendVertices(&aVertices[0], pLod->m_nNumVertices);
                }
            }
        }

        bSuccess &= pGeometryBuffer->EndVertices( );
        bSuccess &= pGeometryBuffer->EndIndices( );
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  struct SLeafCardVertex

struct SLeafCardVertex
{
    st_float32  m_afCardCenter[3];                                      // position
    st_float32  m_afNormal[3];                                          // normal
    st_float32  m_afDiffuseTexCoords[2], m_afLodScales[2];              // texcoord0
    st_float32  m_afCardCorner[2], m_fShadowOffset, m_fPlanarOffset;    // texcoord1
    st_float32  m_afTangent[3], m_fAmbOcc;                              // texcoord2
    st_float32  m_afBinormal[3], m_fWindScalar;                         // texcoord3
    st_float32  m_afWindData[4];                                        // texcoord4
};


///////////////////////////////////////////////////////////////////////
//  CTreeRI::InitLeafCards

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline st_bool CTreeRI_t::InitLeafCards(const SGeometry* pGeometry, st_bool bQuadsSupported, const TShaderTechniquePolicy* pTechnique)
{
    assert(pGeometry);
    st_bool bSuccess = true;

    // retain this for rendering code later
    m_bCardsAreQuads = bQuadsSupported;

    // setup some aliases
    const st_int32 c_nNumLods = pGeometry->m_nNumLeafCardLods;
    const SLeafCards* pLods = pGeometry->m_pLeafCardLods;
    TGeometryBufferPolicy* pGeometryBuffer = m_atGeometryBuffers + GEOMETRY_TYPE_LEAF_CARDS;

    if (c_nNumLods > 0 && pLods) // may not have leaf card geometry in this model
    {
        // setup vertex structure
        SVertexAttribDesc asLeafCardVertexFormat[ ] =
        {
            { VERTEX_ATTRIB_SEMANTIC_POS, VERTEX_ATTRIB_TYPE_FLOAT, 3 },        // xyz = pos
            { VERTEX_ATTRIB_SEMANTIC_NORMAL, VERTEX_ATTRIB_TYPE_FLOAT, 3 },     // xyz = normal
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD0, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // xy = diffuse texcoords, z = lod_width, w = lod_height
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD1, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // xy = card corner, z = shadow offset, w = planar offset
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD2, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // xyz = tangent, w = dimming
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD3, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // xyz = binormal, w = wind scalar
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD4, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // xyzw = wind data
            VERTEX_ATTRIB_END( )
        };

        if (pGeometryBuffer->SetVertexFormat(asLeafCardVertexFormat, pTechnique))
        {
            // if quads aren't supported by the renderer (e.g. DX9), we'll have to use triangle lists
            const st_int32 c_nVertsPerCard = bQuadsSupported ? 4 : 6;
            const st_int32 c_anVertTableForQuads[4] = { 0, 1, 2, 3 };
            const st_int32 c_anVertTableForTriLists[6] = { 0, 1, 2, 0, 2, 3 };
            const st_int32* c_pVertTable = bQuadsSupported ? c_anVertTableForQuads : c_anVertTableForTriLists;

            // fill out a single vertex buffer to accommodate all LODs (avoids buffer state changes during render)
            m_acRenderInfo[GEOMETRY_TYPE_LEAF_CARDS].InitLeafCards(pLods, c_nNumLods);
            for (st_int32 nLod = 0; nLod < c_nNumLods; ++nLod)
            {
                const SLeafCards* pLod = pLods + nLod;

                // initialize index buffer
                bSuccess &= InitLeafCardRenderInfo(nLod, pLod);

                // an LOD must have these values before the render path will work
                if (pLod->HasGeometry( ))
                {
                    assert(pLod->m_pNormals && pLod->m_pTexCoordsDiffuse);

                    // copy this LOD into the vertex buffer
                    CArray<SLeafCardVertex> aVertices(pLod->m_nTotalNumCards * c_nVertsPerCard);
                    SLeafCardVertex* pVertex = &aVertices[0];
                    for (st_int32 nCard = 0; nCard < pLod->m_nTotalNumCards; ++nCard)
                    {
                        const float c_afOffsets[4][2] =  { { 0.5f, 0.5f }, { -0.5f, 0.5f }, { -0.5f, -0.5f }, { 0.5f, -0.5f } };

                        // setup card values used by each vertex
                        const st_float32* c_pDimensons = pLod->m_pDimensions + nCard * 2;
                        const st_float32* c_pPivotPoint = pLod->m_pPivotPoints + nCard * 2;
                        Vec3 vPivot(c_pPivotPoint[0] * c_pDimensons[0], c_pPivotPoint[1] * c_pDimensons[1], 0.0f);

                        // determine the largest distance from the pivot point; useful for the shadow offset value
                        st_float32 fMaxDistanceFromPivotPoint = 0.0f;
                        for (st_int32 nCorner = 0; nCorner < 4; ++nCorner)
                        {
                            Vec3 vCorner(c_afOffsets[nCorner][0] * c_pDimensons[0], c_afOffsets[nCorner][1] * c_pDimensons[1], 0.0f);
                            fMaxDistanceFromPivotPoint = st_max(fMaxDistanceFromPivotPoint, vCorner.Distance(vPivot));
                        }

                        // add the c_nVertsPerCard for this single card
                        for (st_int32 nVertex = 0; nVertex < c_nVertsPerCard; ++nVertex)
                        {
                            st_int32 nCorner = c_pVertTable[nVertex];

                            // position
                            memcpy(pVertex->m_afCardCenter, pLod->m_pPositions + nCard * 3, sizeof(pVertex->m_afCardCenter));

                            // normal
                            Vec3 vNormal = CCore::UncompressVec3(pLod->m_pNormals + nCard * 12 + nCorner * 3);
                            memcpy(pVertex->m_afNormal, vNormal, sizeof(pVertex->m_afNormal));

                            // texcoord0
                            const st_float32* pTexCoords = pLod->m_pTexCoordsDiffuse + nCard * 8 + nCorner * 2;
                            pVertex->m_afDiffuseTexCoords[0] = pTexCoords[0];
                            pVertex->m_afDiffuseTexCoords[1] = pTexCoords[1];

                            const st_float32* pLodScales = pLod->m_pLodScales + nCard * 2;
                            pVertex->m_afLodScales[0] = pLod->m_pLodScales ? pLodScales[0] : 1.0f;
                            pVertex->m_afLodScales[1] = pLod->m_pLodScales ? pLodScales[1] : 1.0f;

                            // texcoord1
                            if (CCoordSys::IsYAxisUp( ))
                            {
                                pVertex->m_afCardCorner[0] = (c_afOffsets[nCorner][1] - c_pPivotPoint[1]) * c_pDimensons[1];
                                pVertex->m_afCardCorner[1] = (c_afOffsets[nCorner][0] - c_pPivotPoint[0]) * c_pDimensons[0];
                            }
                            else
                            {
                                pVertex->m_afCardCorner[0] = (c_afOffsets[nCorner][0] - c_pPivotPoint[0]) * c_pDimensons[0];
                                pVertex->m_afCardCorner[1] = (c_afOffsets[nCorner][1] - c_pPivotPoint[1]) * c_pDimensons[1];
                            }
                            pVertex->m_fShadowOffset = fMaxDistanceFromPivotPoint;
                            pVertex->m_fPlanarOffset = pLod->m_pLeafCardOffsets ? pLod->m_pLeafCardOffsets[nCard * 4 + nCorner] : 0.0f;

                            // texcoord2
                            Vec3 vTangent = CCore::UncompressVec3(pLod->m_pTangents + nCard * 12 + nCorner * 3);
                            memcpy(pVertex->m_afTangent, vTangent, sizeof(pVertex->m_afTangent));
                            pVertex->m_fAmbOcc = CCore::UncompressScalar(pLod->m_pAmbientOcclusionValues[nCard]);

                            // texcoord3
                            Vec3 vBinormal = CCore::UncompressVec3(pLod->m_pBinormals + nCard * 12 + nCorner * 3);
                            memcpy(pVertex->m_afBinormal, vBinormal, sizeof(pVertex->m_afBinormal));

                            // texcoord4
                            if (pLod->m_pWindData)
                            {
                                pVertex->m_afWindData[0] = pLod->m_fWindDataMagnitude * CCore::UncompressScalar(pLod->m_pWindData[nCard * 5 + 0]);
                                pVertex->m_afWindData[1] = pLod->m_fWindDataMagnitude * CCore::UncompressScalar(pLod->m_pWindData[nCard * 5 + 1]);
                                pVertex->m_afWindData[2] = pLod->m_fWindDataMagnitude * CCore::UncompressScalar(pLod->m_pWindData[nCard * 5 + 2]);
                                pVertex->m_afWindData[3] = c_fPi * CCore::UncompressScalar(pLod->m_pWindData[nCard * 5 + 3]);
                                pVertex->m_fWindScalar = c_fUncompressWindScalar * CCore::UncompressScalar(pLod->m_pWindData[nCard * 5 + 4]);
                            }
                            else
                            {
                                pVertex->m_afWindData[0] = pVertex->m_afWindData[1] = pVertex->m_afWindData[2] = pVertex->m_afWindData[3] = 1.0f;
                                pVertex->m_fWindScalar = 1.0f;
                            }

                            ++pVertex;
                        }
                    }

                    bSuccess &= pGeometryBuffer->AppendVertices(&aVertices[0], st_uint32(aVertices.size( )));
                }
            }
        }

        bSuccess &= pGeometryBuffer->EndVertices( );
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  struct SLeafMeshVertex

struct SLeafMeshVertex
{
    st_float32  m_afCoord[3], m_fDiffuseTexCoordS;      // position
    st_float32  m_afNormal[3];                          // normal
    st_float32  m_afTangent[3], m_fDiffuseTexCoordT;    // texcoord0
    st_float32  m_afBinormal[3], m_fDetailTexCoordS;    // texcoord1
    st_float32  m_fAmbOcc, m_fWindScalar;               // texcoord2
    st_float32  m_afLodCoord[3], m_fDetailTexCoordT;    // texcoord3
    st_float32  m_afWindData[4];                        // texcoord4
};


///////////////////////////////////////////////////////////////////////
//  CTreeRI::InitLeafMeshes

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline st_bool CTreeRI_t::InitLeafMeshes(const SGeometry* pGeometry, const TShaderTechniquePolicy* pTechnique)
{
    assert(pGeometry);
    st_bool bSuccess = true;

    // setup some aliases
    const st_int32 c_nNumLods = pGeometry->m_nNumLeafMeshLods;
    const SIndexedTriangles* pLods = pGeometry->m_pLeafMeshLods;
    TGeometryBufferPolicy* pGeometryBuffer = m_atGeometryBuffers + GEOMETRY_TYPE_LEAF_MESHES;

    if (c_nNumLods > 0 && pLods) // may not have leaf mesh geometry in this model
    {
        // setup vertex structure
        SVertexAttribDesc asLeafMeshVertexFormat[ ] =
        {
            { VERTEX_ATTRIB_SEMANTIC_POS, VERTEX_ATTRIB_TYPE_FLOAT, 4 },        // pos.xyz + diffuse_tex_s
            { VERTEX_ATTRIB_SEMANTIC_NORMAL, VERTEX_ATTRIB_TYPE_FLOAT, 3 },     // normal.xyz
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD0, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // tangent.xyz + diffuse_tex_t
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD1, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // binormal.xyz + detail_tex_s
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD2, VERTEX_ATTRIB_TYPE_FLOAT, 2 },  // x = ambient occlusion, y = wind scalar
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD3, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // LOD position.xyz + detail_tex_t
            { VERTEX_ATTRIB_SEMANTIC_TEXCOORD4, VERTEX_ATTRIB_TYPE_FLOAT, 4 },  // wind data.xyz
            VERTEX_ATTRIB_END( )
        };

        if (pGeometryBuffer->SetVertexFormat(asLeafMeshVertexFormat, pTechnique) &&
            pGeometryBuffer->SetIndexFormat(INDEX_FORMAT_UNSIGNED_32BIT))
        {
            // fill out vertex and index buffers
            m_acRenderInfo[GEOMETRY_TYPE_LEAF_MESHES].InitIndexed(pLods, c_nNumLods);
            for (st_int32 nLod = 0; nLod < c_nNumLods; ++nLod)
            {
                const SIndexedTriangles* pLod = pLods + nLod;

                // an LOD must have these values before the render path will work
                if (pLod->HasGeometry( ))
                {
                    assert(pLod->m_pNormals && pLod->m_pTexCoordsDiffuse);

                    // initialize index buffer
                    bSuccess &= InitIndexBuffer(nLod, pLod, pGeometryBuffer, m_acRenderInfo[GEOMETRY_TYPE_LEAF_MESHES]);

                    // fill out vertex buffer
                    CArray<SLeafMeshVertex> aVertices(pLod->m_nNumVertices);
                    SLeafMeshVertex* pVertex = &aVertices[0];
                    for (st_int32 nVertex = 0; nVertex < pLod->m_nNumVertices; ++nVertex)
                    {
                        // coords
                        memcpy(pVertex->m_afCoord, pLod->m_pCoords + nVertex * 3, sizeof(pVertex->m_afCoord));

                        // normal
                        const st_uint8* pCompressedNormal = pLod->m_pNormals + nVertex * 3;
                        memcpy(pVertex->m_afNormal, CCore::UncompressVec3(pCompressedNormal), sizeof(pVertex->m_afNormal));

                        // diffuse texcoords
                        pVertex->m_fDiffuseTexCoordS = pLod->m_pTexCoordsDiffuse[nVertex * 2];
                        pVertex->m_fDiffuseTexCoordT = pLod->m_pTexCoordsDiffuse[nVertex * 2 + 1];

                        // detail texcoords
                        if (pLod->m_pTexCoordsDetail)
                        {
                            pVertex->m_fDetailTexCoordS = pLod->m_pTexCoordsDetail[nVertex * 2];
                            pVertex->m_fDetailTexCoordT = pLod->m_pTexCoordsDetail[nVertex * 2 + 1];
                        }
                        else
                            pVertex->m_fDetailTexCoordS = pVertex->m_fDetailTexCoordT = 0.0f;

                        // tangent
                        const st_uint8* pCompressedTangent = pLod->m_pTangents + nVertex * 3;
                        memcpy(pVertex->m_afTangent, CCore::UncompressVec3(pCompressedTangent), sizeof(pVertex->m_afTangent));

                        // binormal
                        const st_uint8* pCompressedBinormal = pLod->m_pBinormals + nVertex * 3;
                        memcpy(pVertex->m_afBinormal, CCore::UncompressVec3(pCompressedBinormal), sizeof(pVertex->m_afBinormal));

                        // ambient occlusion
                        pVertex->m_fAmbOcc = pLod->m_pAmbientOcclusionValues ? CCore::UncompressScalar(pLod->m_pAmbientOcclusionValues[nVertex]) : 1.0f;

                        // LOD value
                        memcpy(pVertex->m_afLodCoord, pLod->m_pLodCoords ? pLod->m_pLodCoords + nVertex * 3 : pLod->m_pCoords + nVertex * 3, sizeof(pVertex->m_afLodCoord));

                        // wind data
                        if (pLod->m_pWindData)
                        {
                            pVertex->m_afWindData[0] = pLod->m_fWindDataMagnitude * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 0]);
                            pVertex->m_afWindData[1] = pLod->m_fWindDataMagnitude * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 1]);
                            pVertex->m_afWindData[2] = pLod->m_fWindDataMagnitude * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 2]);
                            pVertex->m_afWindData[3] = c_fPi * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 3]);
                            pVertex->m_fWindScalar = c_fUncompressWindScalar * CCore::UncompressScalar(pLod->m_pWindData[nVertex * 5 + 4]);
                        }
                        else
                        {
                            pVertex->m_afWindData[0] = pVertex->m_afWindData[1] = pVertex->m_afWindData[2] = pVertex->m_afWindData[3] = 0.0f;
                            pVertex->m_fWindScalar = 1.0f;
                        }

                        ++pVertex;
                    }

                    pGeometryBuffer->AppendVertices(&aVertices[0], pLod->m_nNumVertices);
                }
            }
        }

        bSuccess &= pGeometryBuffer->EndVertices( );
        bSuccess &= pGeometryBuffer->EndIndices( );
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CTreeRI::InitIndexBuffer
//
//  In addition to building a single index buffer that will store indices
//  for all LODs and materials in this tree model, this function will
//  also determine the draw offsets, indices, and material indices in
//  m_asDrawCallInfo.

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline st_bool CTreeRI_t::InitIndexBuffer(st_int32 nLod, const SIndexedTriangles* pLod, TGeometryBufferPolicy* pGeometryBuffer, CLodRenderInfo& cLodRenderInfo)
{
    assert(pLod);
    assert(pGeometryBuffer);

    st_bool bSuccess = false;

    // need to copy indices into an intermediate buffer because we're combining all of the LODs into a 
    // single index buffer, necessitating index offsets

    // figure out how large the overall buffer is
    st_int32 nBufferSize = 0;
    for (st_int32 nMaterialGroup = 0; nMaterialGroup < pLod->m_nNumMaterialGroups; ++nMaterialGroup)
    {
        const SDrawCallInfo* pInfo = pLod->m_pDrawCallInfo + nMaterialGroup;
        nBufferSize += pInfo->m_nLength;
    }
    assert(nBufferSize > 0);
    CArray<st_uint32> aIndices32;
    aIndices32.reserve(nBufferSize);

    // fill out the index buffer and record other rendering hints
    for (st_int32 nMaterialGroup = 0; nMaterialGroup < pLod->m_nNumMaterialGroups; ++nMaterialGroup)
    {
        // source draw information (raw data taken from SRT file)
        const SDrawCallInfo* pSrcDrawInfo = pLod->m_pDrawCallInfo + nMaterialGroup;

        // add reference for quick LOD/material access during render loop
        cLodRenderInfo.AddRef(nLod, pSrcDrawInfo->m_nMaterialIndex, pSrcDrawInfo->m_nOffset + pGeometryBuffer->NumIndices( ), pSrcDrawInfo->m_nLength);

        // collect indices for a single draw call (for pLod/nMaterialGroup)
        if (pLod->m_pTriangleIndices32)
        {
            for (st_int32 i = pSrcDrawInfo->m_nOffset; i < pSrcDrawInfo->m_nOffset + pSrcDrawInfo->m_nLength; ++i)
                aIndices32.push_back(pLod->m_pTriangleIndices32[i] + pGeometryBuffer->NumVertices( ));
        }
        else
        {
            for (st_int32 i = pSrcDrawInfo->m_nOffset; i < pSrcDrawInfo->m_nOffset + pSrcDrawInfo->m_nLength; ++i)
                aIndices32.push_back(st_uint32(pLod->m_pTriangleIndices16[i]) + pGeometryBuffer->NumVertices( ));
        }
    }

    // finally, copy the array of offset indices into the geometry buffer
    if (!aIndices32.empty( ))
        bSuccess = pGeometryBuffer->AppendIndices((const st_byte*) &aIndices32[0], st_uint32(aIndices32.size( )));
    
    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CTreeRI::InitLeafCardRenderInfo

template<class TTexturePolicy, class TGeometryBufferPolicy, class TShaderTechniquePolicy>
inline st_bool CTreeRI_t::InitLeafCardRenderInfo(st_int32 nLod, const SLeafCards* pLod)
{
    assert(nLod > -1);
    assert(pLod);

    CLodRenderInfo* pLodRenderInfo = m_acRenderInfo + GEOMETRY_TYPE_LEAF_CARDS;
    const TGeometryBufferPolicy* pGeometryBuffer = m_atGeometryBuffers + GEOMETRY_TYPE_LEAF_CARDS;
    const st_int32 c_nVertsPerCard = CardsAreQuads( ) ? 4 : 6;

    // fill out the index buffer and record other rendering hints
    for (st_int32 nMaterialGroup = 0; nMaterialGroup < pLod->m_nNumMaterialGroups; ++nMaterialGroup)
    {
        // source draw information (raw data taken from SRT file); draw call info for cards is a little different
        // than for indexed triangles; instead of vertex counts, offset and length count the number of cards since
        // vertex count varies depending on the quads to render quads
        const SDrawCallInfo* pSrcDrawInfo = pLod->m_pDrawCallInfo + nMaterialGroup;

        // add reference for quick LOD/material access during render loop
        pLodRenderInfo->AddRef(nLod, pSrcDrawInfo->m_nMaterialIndex, (pSrcDrawInfo->m_nOffset * c_nVertsPerCard) + pGeometryBuffer->NumVertices( ), pSrcDrawInfo->m_nLength * c_nVertsPerCard);
    }

    return true;
}
