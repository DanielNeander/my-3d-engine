///////////////////////////////////////////////////////////////////////  
//  TerrainRI.inl
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
//  STerrainRenderInfo::STerrainRenderInfo

inline STerrainRenderInfo::STerrainRenderInfo( ) :
    m_fNormalMapBlueScale(1.0f)
{
    for (int nSplatLayer = 0; nSplatLayer < c_nNumTerrainSplatLayers; ++nSplatLayer)
        m_afSplatTileValues[nSplatLayer] = 1.0f;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::CTerrainRI

CTerrainRI_TemplateList
inline CTerrainRI_Class::CTerrainRI( ) :
    m_nMaxAnisotropy(0),
    m_pShaderLoader(NULL),
    m_pVertexFormat(NULL),
    m_bTexturingEnabled(true)
{
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::~CTerrainRI

CTerrainRI_TemplateList
inline CTerrainRI_Class::~CTerrainRI( )
{
    // destroy VBOs assigned to the terrain cells
    for (TTerrainCellMap::iterator iMap = m_cTerrainCellMap.begin( ); iMap != m_cTerrainCellMap.end( ); ++iMap)
    {
        CTerrainCell* pCell = &iMap->second;
        assert(pCell);
        TGeometryBufferPolicy* pVbo = (TGeometryBufferPolicy*) pCell->GetVbo( );
        assert(pVbo);

        st_delete<TGeometryBufferPolicy>(pVbo, "Terrain VBO");
    }

    // destroy VBO pool
    for (int i = 0; i < int(m_aVboPool.size( )); ++i)
    {
        st_delete<TGeometryBufferPolicy>(m_aVboPool[i], "Terrain VBO");
    }
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::SetShaderLoader

CTerrainRI_TemplateList
inline void CTerrainRI_Class::SetShaderLoader(TShaderLoaderPolicy* pShaderLoader)
{
    m_pShaderLoader = pShaderLoader;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::SetRenderInfo

CTerrainRI_TemplateList
inline void CTerrainRI_Class::SetRenderInfo(const STerrainRenderInfo& sInfo)
{
    m_sRenderInfo = sInfo;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::GetRenderInfo

CTerrainRI_TemplateList
inline const STerrainRenderInfo& CTerrainRI_Class::GetRenderInfo(void) const
{
    return m_sRenderInfo;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::SetMaxAnisotropy

CTerrainRI_TemplateList
inline void CTerrainRI_Class::SetMaxAnisotropy(int nMaxAnisotropy)
{
    m_nMaxAnisotropy = nMaxAnisotropy;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::Init

CTerrainRI_TemplateList
inline bool CTerrainRI_Class::Init(int nNumLods, int nMaxTileRes, float fCellSize, const SVertexAttribDesc* pVertexFormat)
{
    bool bSuccess = false;

    // load textures
    bSuccess = m_tNormalMapTexture.Load(m_sRenderInfo.m_strNormalMap.c_str( ), m_nMaxAnisotropy);
    bSuccess &= m_tSplatMapTexture.Load(m_sRenderInfo.m_strSplatMap.c_str( ), m_nMaxAnisotropy);
    for (int nSplatLayer = 0; nSplatLayer < c_nNumTerrainSplatLayers; ++nSplatLayer)
        bSuccess &= m_atSplatTextureLayers[nSplatLayer].Load(m_sRenderInfo.m_astrSplatLayers[nSplatLayer].c_str( ), m_nMaxAnisotropy);

    // load white texture (for use when texturing is 'disabled')
    if (!m_tWhiteTexture.IsValid( ))
        (void) m_tWhiteTexture.LoadColor(0xffffffff);

    // shader
    if (!ShaderIsLoaded( ))
        bSuccess &= InitShader( );

    // vbo, allocated ahead of time
    if (m_aVboPool.empty( ) && m_cTerrainCellMap.empty( ))
    {
        m_pVertexFormat = pVertexFormat;
        bSuccess &= InitVboPool( );
    }

    // if cells exist, then this is a subsequent Init() call and we need to destroy the terrain cells,
    // saving off their respective VBOs in the process
    if (!m_cTerrainCellMap.empty( ))
    {
        for (TTerrainCellMap::iterator iMap = m_cTerrainCellMap.begin( ); iMap != m_cTerrainCellMap.end( ); ++iMap)
        {
            CTerrainCell* pCell = &iMap->second;
            assert(pCell);
            TGeometryBufferPolicy* pVbo = (TGeometryBufferPolicy*) pCell->GetVbo( );
            assert(pVbo);

            m_aVboPool.push_back(pVbo);
        }
    }

    // init graphics-independent portion from Forest library
    bSuccess &= CTerrain::Init(nNumLods, nMaxTileRes, fCellSize);

    // initialize the single index buffer that all terrain tiles will share
    bSuccess &= InitIndexBuffer( );

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::Render

CTerrainRI_TemplateList
inline bool CTerrainRI_Class::Render(const STerrainCullResults& sCullResults, ERenderPassType eRenderPass, const SMaterial& sLighting, SRenderStats* pMainStats)
{
    bool bSuccess = true;

    // if stats are passed in, pick the appropriate geometry group to fill out
    SRenderStats::SGeometryStats* pStats = pMainStats ? &pMainStats->m_aGeometries[STATS_CATEGORY_TERRAIN][eRenderPass] : NULL;

    // first and foremost, see if any of the terrain is visible
    if (!sCullResults.m_aVisibleCells.empty( ))
    {
        TShaderTechniquePolicy& tTechnique = CForestRI_Class::PickTechnique(m_atTechniques, eRenderPass);
        if (BeginRender(tTechnique, sLighting))
        {
            bSuccess &= tTechnique.CommitConstants( );

            // enable the master index buffer
            bSuccess &= m_tIndexBuffer.BindIndexBuffer( );

            // enable the vertex format
            TGeometryBufferPolicy* pFirstVbo = (TGeometryBufferPolicy*) sCullResults.m_aVisibleCells[0]->GetVbo( );
            assert(pFirstVbo);
            bSuccess &= pFirstVbo->EnableFormat( );

            for (int nCell = 0; nCell < int(sCullResults.m_aVisibleCells.size( )); ++nCell)
            {
                const CTerrainCell* pCell = sCullResults.m_aVisibleCells[nCell];
                assert(pCell);
                TGeometryBufferPolicy* pVbo = (TGeometryBufferPolicy*) pCell->GetVbo( );
                assert(pVbo);

                if (pVbo->BindVertexBuffer( ))
                {
                    // query offset into master index buffer
                    unsigned int uiOffset = 0, uiNumIndices = 0;
                    pCell->GetIndices(uiOffset, uiNumIndices);

                    // render the tile at the correct LOD and transitional borders
                    m_tIndexBuffer.RenderIndexed(PRIMITIVE_TRIANGLE_STRIP, uiOffset, uiNumIndices);

                    if (pStats)
                    {
                        pStats->m_uiNumVboBinds++;
                        pStats->m_uiNumDrawCalls++;
                        pStats->m_uiNumTriangles += uiNumIndices - 2;
                    }

                    bSuccess &= pVbo->UnBindVertexBuffer( );
                }
                else
                {
                    CCore::SetError("CTerrainRI::Render, BindVertexBuffer() failed");
                    bSuccess = false;
                }
            }

            // with the render loop complete, unbind the index buffer & disable the vbo vertex format
            bSuccess &= m_tIndexBuffer.UnBindIndexBuffer( );
            bSuccess &= TGeometryBufferPolicy::DisableFormat( );

            if (bSuccess && pStats)
            {
                pStats->m_uiNumObjects++;
                pStats->m_uiNumTextureBinds += c_nNumTerrainSplatLayers;
            }
        }
        else
        {
            CCore::SetError("CTerrainRI::Render, BeginRender() failed");
            bSuccess = false;
        }

        // final state changes, if any
        bSuccess &= EndRender(tTechnique);
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::CullAndComputeLOD

CTerrainRI_TemplateList
inline void CTerrainRI_Class::CullAndComputeLOD(const CView& cView, STerrainCullResults& sCullResults)
{
    // determine which cells are visible and which need to be populated
    CTerrain::CullAndComputeLOD(cView, sCullResults);

    // add the newly-available VBOs to the list of available VBOs
    int i = 0;
    for (i = 0; i < int(sCullResults.m_aFreedVbos.size( )); ++i)
        m_aVboPool.push_back((TGeometryBufferPolicy*) sCullResults.m_aFreedVbos[i]);

    // assign VBOs to the newly-created cells
    for (i = 0; i < int(sCullResults.m_aCellsToUpdate.size( )); ++i)
        sCullResults.m_aCellsToUpdate[i]->SetVbo((void*) NextVbo( ));
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::EnableTexturing

CTerrainRI_TemplateList
inline void CTerrainRI_Class::EnableTexturing(bool bFlag)
{
    m_bTexturingEnabled = bFlag;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::InitIndexBuffer

CTerrainRI_TemplateList
inline bool CTerrainRI_Class::InitIndexBuffer(void)
{
    bool bSuccess = false;

    const CArray<unsigned int>& aCompositeIndices = GetCompositeIndices( );
    if (!aCompositeIndices.empty( ))
    {
        m_tIndexBuffer.SetIndexFormat(INDEX_FORMAT_UNSIGNED_32BIT);
        bSuccess = m_tIndexBuffer.ReplaceIndices((const unsigned char*) &aCompositeIndices[0], unsigned int(aCompositeIndices.size( )));
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::InitShader

CTerrainRI_TemplateList
inline bool CTerrainRI_Class::InitShader(void)
{
    bool bSuccess = false;

    if (m_pShaderLoader)
    {
        bSuccess = true;

        for (int nShaderType = int(CForestRI_Class::SHADER_TYPE_STANDARD); nShaderType < int(CForestRI_Class::SHADER_TYPE_NUMTYPES); ++nShaderType)
        {
            CFixedString strShaderName = CForestRI_Class::ShaderNameByPass("Terrain", typename CForestRI_Class::EShaderType(nShaderType));
            bSuccess &= m_pShaderLoader->GetTechnique(strShaderName.c_str( ), m_atTechniques[nShaderType]);
        }

        // query terrain-specific uniform variables
        bSuccess &= m_pShaderLoader->GetConstant(TextureShaderConstantName("TerrainNormalMap").c_str( ), m_tNormalMapTextureVar);
        bSuccess &= m_pShaderLoader->GetConstant(TextureShaderConstantName("TerrainSplatMap").c_str( ), m_tSplatMapTextureVar);
        bSuccess &= m_pShaderLoader->GetConstant(TextureShaderConstantName("TerrainSplatLayer0").c_str( ), m_atSplatTextureLayersVars[0]);
        bSuccess &= m_pShaderLoader->GetConstant(TextureShaderConstantName("TerrainSplatLayer1").c_str( ), m_atSplatTextureLayersVars[1]);
        bSuccess &= m_pShaderLoader->GetConstant(TextureShaderConstantName("TerrainSplatLayer2").c_str( ), m_atSplatTextureLayersVars[2]);
        bSuccess &= m_pShaderLoader->GetConstant("g_vSplatMapTiles", m_tSplatMapTiles);
        bSuccess &= m_pShaderLoader->GetConstant("g_mMaterial", m_tLightAndMaterial);
    }
    else
        CCore::SetError("CTerrainRI::InitShader, no shader loader was passed in from CForestRI");

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::ShaderIsLoaded

CTerrainRI_TemplateList
inline bool CTerrainRI_Class::ShaderIsLoaded(void) const
{
    // assume that if a shader constant is loaded that the shader has already been loaded
    return m_tSplatMapTiles.IsValid( );
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::BeginRender

CTerrainRI_TemplateList
inline bool CTerrainRI_Class::BeginRender(TShaderTechniquePolicy& tTechnique, const SMaterial& sLighting)
{
    bool bSuccess = false;

    // set render states
    m_tRenderStatePolicy.SetAlphaTesting(false);
    m_tRenderStatePolicy.SetFaceCulling(CCoordSys::IsLeftHanded( ) ? CULLTYPE_BACK : CULLTYPE_FRONT);

    // bind the technique
    if (tTechnique.Bind( ))
    {
        // upload the textures
        bSuccess = SetTexture(m_tNormalMapTextureVar, m_tNormalMapTexture);
        bSuccess &= SetTexture(m_tSplatMapTextureVar, m_tSplatMapTexture);
        for (int nSplatLayer = 0; nSplatLayer < c_nNumTerrainSplatLayers; ++nSplatLayer)
            bSuccess &= SetTexture(m_atSplatTextureLayersVars[nSplatLayer], m_atSplatTextureLayers[nSplatLayer]);

        // set shader variables
        {
            // material composed of terrain colors * light colors
            const noVec4 c_avModifiedMaterial[4] = 
            {
                // ambient
                m_sRenderInfo.m_sMaterial.m_vAmbient.Multiply(sLighting.m_vAmbient),
                // diffuse
                (m_sRenderInfo.m_sMaterial.m_vDiffuse.Multiply(sLighting.m_vDiffuse)) * (m_sRenderInfo.m_sMaterial.m_fLightScalar),
                // specular
                m_sRenderInfo.m_sMaterial.m_vSpecular.Multiply(sLighting.m_vSpecular),
                // emissive
                m_sRenderInfo.m_sMaterial.m_vEmissive.Multiply(sLighting.m_vEmissive),
            };
            bSuccess &= m_tLightAndMaterial.SetMatrix(c_avModifiedMaterial[0].TofloatPtr());

            // upload extra terrain data (usually tiling parameters)
            bSuccess &= m_tSplatMapTiles.Set4fv(reinterpret_cast<float*>(&m_sRenderInfo.m_afSplatTileValues));
        }
    }
    else
        CCore::SetError("CTerrainRI::BeginRender, tTechnique.Bind() failed");

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::EndRender

CTerrainRI_TemplateList
inline bool CTerrainRI_Class::EndRender(TShaderTechniquePolicy& tTechnique)
{
    bool bSuccess = false;

    bSuccess = tTechnique.UnBind( );

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::SetTexture

CTerrainRI_TemplateList
inline bool CTerrainRI_Class::SetTexture(const TShaderConstantPolicy& tTextureVar, const TTexturePolicy& tTextureMap) const
{
    bool bSuccess = false;

    if (m_bTexturingEnabled && tTextureVar.IsValid( ) && tTextureMap.IsValid( ))
        bSuccess = tTextureVar.SetTexture(tTextureMap);
    else
        bSuccess = tTextureVar.SetTexture(m_tWhiteTexture);

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::InitVboPool

CTerrainRI_TemplateList
inline bool CTerrainRI_Class::InitVboPool(void)
{
    bool bSuccess = false;

    if (m_nHintMaxActiveTerrainCells > 0)
    {
        m_aVboPool.resize(m_nHintMaxActiveTerrainCells);
        for (int i = 0; i < m_nHintMaxActiveTerrainCells; ++i)
        {
            m_aVboPool[i] = NewVbo( );
        }

        bSuccess = true;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::NextVbo

CTerrainRI_TemplateList
inline TGeometryBufferPolicy* CTerrainRI_Class::NextVbo(void)
{
    TGeometryBufferPolicy* pVbo = NULL;

    if (m_aVboPool.empty( ))
        pVbo = NewVbo( );
    else
    {
        pVbo = m_aVboPool.back( );
        m_aVboPool.pop_back( );
    }

    return pVbo;
}


///////////////////////////////////////////////////////////////////////  
//  CTerrainRI::NewVbo

CTerrainRI_TemplateList
inline TGeometryBufferPolicy* CTerrainRI_Class::NewVbo(void) const
{
    TGeometryBufferPolicy* pVbo = new ("Terrain VBO") TGeometryBufferPolicy;

    assert(m_pVertexFormat);
    pVbo->SetVertexFormat(m_pVertexFormat, &m_atTechniques[CForestRI_Class::SHADER_TYPE_STANDARD], true);

    return pVbo;
}


