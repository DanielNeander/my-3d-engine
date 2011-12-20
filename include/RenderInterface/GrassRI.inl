///////////////////////////////////////////////////////////////////////  
//  GrassRI.inl
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) CONFIDENTIAL AND PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization, Inc. and
//  may not be copied, disclosed, or exploited except in accordance with 
//  the terms of thats agreement.
//
//      Copyright (c) 2003-2009 IDV, Inc.
//      All rights reserved in all media.
//
//      IDV, Inc.
//      http://www.idvinc.com


///////////////////////////////////////////////////////////////////////  
//  SGrassRenderInfo::SGrassRenderInfo

inline SGrassRenderInfo::SGrassRenderInfo( ) :
    m_fTerrainMatchFactorTop(0.7f),
    m_fTerrainMatchFactorBottom(1.0)
{
    for (st_int32 nSplatLayer = 0; nSplatLayer < c_nNumTerrainSplatLayers; ++nSplatLayer)
        m_avTerrainSplatColors[nSplatLayer].Set(1.0f, 1.0f, 1.0f, 1.0f);
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::CGrassRI

CGrassRI_TemplateList
inline CGrassRI_Class::CGrassRI( ) :
    m_pShaderLoader(NULL),
    m_pVertexFormat(NULL),
    m_bTexturingEnabled(true)
{
    m_tIndexBuffer.SetIndexFormat(INDEX_FORMAT_UNSIGNED_32BIT);
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::~CGrassRI

CGrassRI_TemplateList
inline CGrassRI_Class::~CGrassRI( )
{
    // destroy VBOs assigned to the Grass cells
    for (TGrassCellMap::iterator iMap = m_cGrassCellMap.begin( ); iMap != m_cGrassCellMap.end( ); ++iMap)
    {
        CGrassCell* pCell = &iMap->second;
        assert(pCell);
        TGeometryBufferPolicy* pVbo = (TGeometryBufferPolicy*) pCell->GetVbo( );
        assert(pVbo);

        st_delete<TGeometryBufferPolicy>(pVbo, "Grass VBO");
    }

    // destroy VBO pool
    for (st_int32 i = 0; i < st_int32(m_aVboPool.size( )); ++i)
    {
        st_delete<TGeometryBufferPolicy>(m_aVboPool[i], "Grass VBO");
    }
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::SetShaderLoader

CGrassRI_TemplateList
inline void CGrassRI_Class::SetShaderLoader(TShaderLoaderPolicy* pShaderLoader)
{
    m_pShaderLoader = pShaderLoader;
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::SetRenderInfo

CGrassRI_TemplateList
inline void CGrassRI_Class::SetRenderInfo(const SGrassRenderInfo& sInfo)
{
    m_sRenderInfo = sInfo;
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::GetRenderInfo

CGrassRI_TemplateList
inline const SGrassRenderInfo& CGrassRI_Class::GetRenderInfo(void) const
{
    return m_sRenderInfo;
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::Init
//
//  Init() may be called more than once in an application's lifetime; it may
//  be used to set new render parameters for level levels, forests, etc.
//
//  Init() does assume that the vertex format doesn't change in subsequent calls.

CGrassRI_TemplateList
inline st_bool CGrassRI_Class::Init(const char* pTextureFilename, st_int32 nImageRows, st_int32 nImageCols, st_float32 fCellSize, const SVertexAttribDesc* pVertexFormat)
{
    st_bool bSuccess = false;

    assert(pVertexFormat);

    // load the grass texture
    bSuccess = m_tCompositeTexture.Load(pTextureFilename);

    // don't load if it was loaded previously
    if (!m_tWhiteTexture.IsValid( ))
        (void) m_tWhiteTexture.LoadColor(0xffffffff);

    // shader
    if (!ShaderIsLoaded( ))
        bSuccess &= InitShader( );

    // vbos, allocated up front
    if (m_aVboPool.empty( ) && m_cGrassCellMap.empty( ))
    {
        m_pVertexFormat = pVertexFormat;
        bSuccess &= InitVboPool( );
        if (!TGeometryBufferPolicy::IsPrimitiveTypeSupported(PRIMITIVE_QUADS))
            bSuccess &= ResizeIndexBuffer(m_nHintMaxGrassBladesPerCell);
    }

    // if cells exist, then this is a subsequent Init() call and we need to destroy the grass cells, 
    // saving off their respective VBOs in the process
    if (!m_cGrassCellMap.empty( ))
    {
        for (TGrassCellMap::iterator iMap = m_cGrassCellMap.begin( ); iMap != m_cGrassCellMap.end( ); ++iMap)
        {
            CGrassCell* pCell = &iMap->second;
            assert(pCell);
            TGeometryBufferPolicy* pVbo = (TGeometryBufferPolicy*) pCell->GetVbo( );
            assert(pVbo);

            m_aVboPool.push_back(pVbo);
        }
    }

    bSuccess &= CGrass::Init(pTextureFilename, nImageRows, nImageCols, fCellSize);

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::Render

CGrassRI_TemplateList
inline st_bool CGrassRI_Class::Render(const SGrassCullResults& sCullResults, ERenderPassType eRenderPass, const SMaterial& sLighting, SRenderStats* pMainStats)
{
    st_bool bSuccess = true;

    // if stats are passed in, pick the appropriate geometry group to fill out
    SRenderStats::SGeometryStats* pStats = pMainStats ? &pMainStats->m_aGeometries[STATS_CATEGORY_GRASS][eRenderPass] : NULL;

    // first and foremost, see if any of the grass is visible
    if (!sCullResults.m_aVisibleCells.empty( ))
    {
        TShaderTechniquePolicy& tTechnique = m_tTechnique;
        if (BeginRender(tTechnique, sLighting))
        {
            bSuccess &= tTechnique.CommitConstants( );

            if (pStats)
                pStats->m_uiNumTextureBinds++;

            // enable the vertex format
            TGeometryBufferPolicy* pFirstVbo = (TGeometryBufferPolicy*) sCullResults.m_aVisibleCells[0]->GetVbo( );
            assert(pFirstVbo);
            bSuccess &= pFirstVbo->EnableFormat( );

            // determine how the grass will be rendered
            const st_bool c_bCanRenderQuads = TGeometryBufferPolicy::IsPrimitiveTypeSupported(PRIMITIVE_QUADS);
            if (!c_bCanRenderQuads)
                bSuccess &= m_tIndexBuffer.BindIndexBuffer( );

            // render the individual grass cells
            for (st_int32 nCell = 0; nCell < st_int32(sCullResults.m_aVisibleCells.size( )); ++nCell)
            {
                const CGrassCell* pCell = sCullResults.m_aVisibleCells[nCell];
                assert(pCell);
                TGeometryBufferPolicy* pVbo = (TGeometryBufferPolicy*) pCell->GetVbo( );
                assert(pVbo);

                if (pVbo->BindVertexBuffer( ))
                {
                    if (c_bCanRenderQuads)
                        bSuccess &= pVbo->RenderArrays(PRIMITIVE_QUADS, 0, pCell->GetNumBlades( ) * 4);
                    else
                    {
                        const st_uint32 c_uiNumVerts = pCell->GetNumBlades( ) * 6;
                        if (c_uiNumVerts > m_tIndexBuffer.NumIndices( ))
                        {
                            CCore::SetError("CGrassRI::Render(), exceeded preset limit of %d grass blades per cell, now %d; use CGrass::SetHint() to adjust",
                                m_tIndexBuffer.NumIndices( ) / 6, pCell->GetNumBlades( ));

                            bSuccess &= m_tIndexBuffer.UnBindIndexBuffer( );
                            bSuccess &= ResizeIndexBuffer(pCell->GetNumBlades( ));
                            bSuccess &= m_tIndexBuffer.BindIndexBuffer( );
                        }
                        bSuccess &= m_tIndexBuffer.RenderIndexed(PRIMITIVE_TRIANGLES, 0, c_uiNumVerts);
                    }

                    if (pStats)
                    {
                        pStats->m_uiNumVboBinds++;
                        pStats->m_uiNumDrawCalls++;
                        pStats->m_uiNumTriangles += pCell->GetNumBlades( ) * 2;
                    }

                    bSuccess &= pVbo->UnBindVertexBuffer( );
                }
                else
                {
                    CCore::SetError("CGrassRI::Render, BindVertexBuffer() failed");
                    bSuccess = false;
                }
            }

            bSuccess &= pFirstVbo->DisableFormat( );
            bSuccess &= EndRender(tTechnique);
            if (!c_bCanRenderQuads)
                bSuccess &= m_tIndexBuffer.UnBindIndexBuffer( );

            if (bSuccess && pStats)
            {
                pStats->m_uiNumObjects++;
            }
        }
        else
        {
            CCore::SetError("CGrassRI::Render, BeginRender() failed");
            bSuccess = false;
        }
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::Cull

CGrassRI_TemplateList
inline void CGrassRI_Class::Cull(const CView& cView, SGrassCullResults& sCullResults)
{
    // determine which cells are visible and which need to be populated
    CGrass::Cull(cView, sCullResults);

    // add the newly-available VBOs to the list of available VBOs
    st_int32 i = 0;
    for (i = 0; i < st_int32(sCullResults.m_aFreedVbos.size( )); ++i)
        m_aVboPool.push_back((TGeometryBufferPolicy*) sCullResults.m_aFreedVbos[i]);

    // assign VBOs to the newly-created cells
    for (i = 0; i < st_int32(sCullResults.m_aCellsToUpdate.size( )); ++i)
        sCullResults.m_aCellsToUpdate[i]->SetVbo((void*) NextVbo( ));

    // compute new grass unit billboard since the view has presumably changed
    ComputeUnitBillboard(cView);
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::EnableTexturing

CGrassRI_TemplateList
inline void CGrassRI_Class::EnableTexturing(st_bool bFlag)
{
    m_bTexturingEnabled = bFlag;
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::BeginRender

CGrassRI_TemplateList
inline st_bool CGrassRI_Class::BeginRender(TShaderTechniquePolicy& tTechnique, const SMaterial& sLighting)
{
    st_bool bSuccess = false;

    // set render states
    m_tRenderStatePolicy.SetFaceCulling(CULLTYPE_NONE);

    if (tTechnique.Bind( ))
    {
        bSuccess = true;

        // upload the texture
        if (m_bTexturingEnabled && m_tCompositeTexture.IsValid( ))
            bSuccess &= m_tCompositeTextureVar.SetTexture(m_tCompositeTexture);
        else
            (void) m_tCompositeTextureVar.SetTexture(m_tWhiteTexture);

        // set shader variables
        bSuccess &= m_tUnitBillboard.SetMatrix(m_mCameraFacingUnitBillboard);
        bSuccess &= m_tRenderInfo.Set4f(m_fStartFade, m_fEndFade - m_fStartFade, m_sRenderInfo.m_fTerrainMatchFactorTop, m_sRenderInfo.m_fTerrainMatchFactorBottom);
        bSuccess &= m_tTerrainSplatColors.SetArray4f(m_sRenderInfo.m_avTerrainSplatColors[0], 3);

        // material composed of grass colors * light colors
        const Vec4 c_avModifiedMaterial[4] = 
        {
            // ambient
            m_sRenderInfo.m_sMaterial.m_vAmbient * sLighting.m_vAmbient,
            // diffuse
            m_sRenderInfo.m_sMaterial.m_vDiffuse * sLighting.m_vDiffuse * m_sRenderInfo.m_sMaterial.m_fLightScalar,
            // specular
            m_sRenderInfo.m_sMaterial.m_vSpecular * sLighting.m_vSpecular,
            // emissive
            m_sRenderInfo.m_sMaterial.m_vEmissive * sLighting.m_vEmissive,
        };
        bSuccess &= m_tLightAndMaterial.SetMatrix(c_avModifiedMaterial[0]);
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::EndRender

CGrassRI_TemplateList
inline st_bool CGrassRI_Class::EndRender(TShaderTechniquePolicy& tTechnique)
{
    bool bSuccess = false;

    bSuccess = tTechnique.UnBind( );

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::ComputeUnitBillboard

CGrassRI_TemplateList
inline void CGrassRI_Class::ComputeUnitBillboard(const CView& cView)
{
    // setup initial billboard coords for default coordinate system
    Mat4x4 mUnitBillboard(0.0f, 0.5f, 1.0f, 0.0f,
                          0.0f, -0.5f, 1.0f, 0.0f,
                          0.0f, -0.5f, 0.0f, 0.0f,
                          0.0f, 0.5f, 0.0f, 0.0f);

    // convert unit billboard to current coordinate system
    if (!CCoordSys::IsDefaultCoordSys( ))
    {
        for (int i = 0; i < 4; ++i)
        {
            Vec3* pCoord = reinterpret_cast<Vec3*>(mUnitBillboard.m_afSingle + i * 4);
            *pCoord = CCoordSys::ConvertFromStd(*pCoord);
        }
    }

    // build and transpose a Mat4x4 type for operations later
    mUnitBillboard.Transpose(mUnitBillboard);

    // compute pitch and azimuth
    float fAzimuth = -cView.GetCameraAzimuth( );
    float fPitch = cView.GetCameraPitch( );

    // alternate coordinate system adjustment
    if (!CCoordSys::IsLeftHanded( ) && CCoordSys::IsYAxisUp( ))
        fPitch = -fPitch;

    m_mCameraFacingUnitBillboard.SetIdentity( );
    CCoordSys::RotateOutAxis(m_mCameraFacingUnitBillboard, 0.75f * fPitch);
    CCoordSys::RotateUpAxis(m_mCameraFacingUnitBillboard, fAzimuth);
    m_mCameraFacingUnitBillboard = m_mCameraFacingUnitBillboard * mUnitBillboard;
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::InitVboPool

CGrassRI_TemplateList
inline st_bool CGrassRI_Class::InitVboPool(void)
{
    st_bool bSuccess = false;

    if (m_nHintMaxNumVbos > 0)
    {
        m_aVboPool.resize(m_nHintMaxNumVbos);
        for (st_int32 i = 0; i < m_nHintMaxNumVbos; ++i)
        {
            m_aVboPool[i] = NewVbo( );
        }

        bSuccess = true;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::NextVbo

CGrassRI_TemplateList
inline TGeometryBufferPolicy* CGrassRI_Class::NextVbo(void)
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
//  CGrassRI::ResizeIndexBuffer

CGrassRI_TemplateList
inline st_bool CGrassRI_Class::ResizeIndexBuffer(st_int32 nNumGrassBlades)
{
    st_bool bSuccess = false;

    // index buffer is only necessary if quads can't be rendered
    if (nNumGrassBlades > 0 &&
        m_tIndexBuffer.NumIndices( ) < st_uint32(nNumGrassBlades * 6))
    {
        CArray<st_uint32> aIndices(nNumGrassBlades * 6);

        st_uint32* pBuffer = &aIndices[0];
        for (st_uint32 i = 0; i < st_uint32(nNumGrassBlades); ++i)
        {
            st_uint32 uiOffset = i * 4;
            *pBuffer++ = uiOffset + 0;
            *pBuffer++ = uiOffset + 1;
            *pBuffer++ = uiOffset + 2;
            *pBuffer++ = uiOffset + 0;
            *pBuffer++ = uiOffset + 2;
            *pBuffer++ = uiOffset + 3;
        }
        assert(st_uint32(pBuffer - &aIndices[0]) == st_uint32(nNumGrassBlades * 6));

        // copy buffer contents into index buffer
        bSuccess = m_tIndexBuffer.ReplaceIndices((const st_byte*) &aIndices[0], st_uint32(aIndices.size( )));
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::NewVbo

CGrassRI_TemplateList
inline TGeometryBufferPolicy* CGrassRI_Class::NewVbo(void)
{
    TGeometryBufferPolicy* pVbo = new ("Grass VBO") TGeometryBufferPolicy;

    assert(m_pVertexFormat);
    pVbo->SetVertexFormat(m_pVertexFormat, &m_tTechnique, true);

    // go ahead and allocate the VBO so it doesn't happen after app's init time
    assert(m_nHintMaxGrassBladesPerCell > 0);

    const st_int32 c_nVertsPerBlade = 4;
    CArray<st_byte> aEmpty(m_nHintMaxGrassBladesPerCell * c_nVertsPerBlade * pVbo->VertexSize( ));
    pVbo->ReplaceVertices(&aEmpty[0], m_nHintMaxGrassBladesPerCell * c_nVertsPerBlade);

    return pVbo;
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::InitShader

CGrassRI_TemplateList
inline st_bool CGrassRI_Class::InitShader(void)
{
    bool bSuccess = false;

    if (m_pShaderLoader)
    {
        bSuccess = m_pShaderLoader->GetTechnique("Grass", m_tTechnique);
        if (bSuccess)
        {
            // query grass-specific uniform variables
            bSuccess &= m_pShaderLoader->GetConstant(TextureShaderConstantName("GrassBlades").c_str( ), m_tCompositeTextureVar);
            bSuccess &= m_pShaderLoader->GetConstant("g_mGrassUnitBillboard", m_tUnitBillboard);
            bSuccess &= m_pShaderLoader->GetConstant("g_avGrassSplatColors", m_tTerrainSplatColors);
            bSuccess &= m_pShaderLoader->GetConstant("g_vGrassInfo", m_tRenderInfo);
            bSuccess &= m_pShaderLoader->GetConstant("g_mMaterial", m_tLightAndMaterial);
        }
        else
            CCore::SetError("CGrassRI::InitShader, technique 'Grass' failed to load");
    }
    else
        CCore::SetError("CGrassRI::InitShader, no shader loader was passed in from CForestRI");

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGrassRI::ShaderIsLoaded

CGrassRI_TemplateList
inline st_bool CGrassRI_Class::ShaderIsLoaded(void) const
{
    // assume that if a shader constant is loaded that the shader has already been loaded
    return m_tUnitBillboard.IsValid( );
}


