///////////////////////////////////////////////////////////////////////  
//  ForestRI.inl
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
//  CForestRI::CForestRI

CForestRI_TemplateList
inline CForestRI_Class::CForestRI( ) :
    m_eTextureAlphaMethod(TRANS_TEXTURE_ALPHA_TESTING),
    m_bTexturingEnabled(true),
    m_bCanRenderQuads(TGeometryBufferPolicy::IsPrimitiveTypeSupported(PRIMITIVE_QUADS))
{
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::~CForestRI

CForestRI_TemplateList
inline CForestRI_Class::~CForestRI( )
{
    // destroy billboard VBOs assigned to any of the tree cells
    for (TTreeCellMap::iterator iMap = m_cTreeCellMap.begin( ); iMap != m_cTreeCellMap.end( ); ++iMap)
    {
        CTreeCell* pCell = &iMap->second;
        assert(pCell);
        TGeometryBufferPolicy* pVbo = (TGeometryBufferPolicy*) pCell->GetBillboardVbo( );

        if (pVbo)
        {
            st_delete<TGeometryBufferPolicy>(pVbo, "Billboard VBO");
            pCell->SetBillboardVbo(NULL);
        }
    }

    // destroy the billboard VBO pool
    for (int i = 0; i < int(m_aBillboardVboPool.size( )); ++i)
    {
        st_delete<TGeometryBufferPolicy>(m_aBillboardVboPool[i], "Billboard VBO");
    }
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::RegisterTree

CForestRI_TemplateList
inline bool CForestRI_Class::RegisterTree(CTreeRI_t* pTree, const char* pTexturePath)
{
    bool bSuccess = false;

    if (CForest::RegisterTree(pTree))
    {
        // setup resource search paths
        CArray<CFixedString> aSearchPaths;
        if (pTree->GetFilename( ))
            aSearchPaths.push_back(CFixedString(pTree->GetFilename( )).Path( ));
        if (pTexturePath)
            aSearchPaths.push_back(pTexturePath);

        const TShaderTechniquePolicy* atTechniques[ ] = 
        { 
            &m_atBranchTechniques[SHADER_TYPE_STANDARD],
            &m_atFrondTechniques[SHADER_TYPE_STANDARD],
            &m_atLeafCardTechniques[SHADER_TYPE_STANDARD],
            &m_atLeafMeshTechniques[SHADER_TYPE_STANDARD] 
        };

        // load the tree's textures, build vertex and index buffers, etc.
        if (!pTree->InitGraphics(aSearchPaths, m_bCanRenderQuads, atTechniques, m_sRenderInfo.m_nMaxAnisotropy))
            CCore::SetError("CForestRI::RegisterTree, InitGraphics failed");
        else
            bSuccess = true;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::ReplaceTree

CForestRI_TemplateList
inline bool CForestRI_Class::ReplaceTree(const CTreeRI_t* pOldTree, CTreeRI_t* pNewTree, const char* pTexturePath)
{
    bool bSuccess = false;

    if (CForest::ReplaceTree(pOldTree, pNewTree))
    {
        if (!pNewTree->GraphicsAreInitialized( ))
        {
            // setup resource search paths
            CArray<CFixedString> aSearchPaths;
            if (pNewTree->GetFilename( ))
                aSearchPaths.push_back(CFixedString(pNewTree->GetFilename( )).Path( ));
            if (pTexturePath)
                aSearchPaths.push_back(pTexturePath);

            const TShaderTechniquePolicy* atTechniques[ ] = 
            { 
                &m_atBranchTechniques[SHADER_TYPE_STANDARD],
                &m_atFrondTechniques[SHADER_TYPE_STANDARD],
                &m_atLeafCardTechniques[SHADER_TYPE_STANDARD],
                &m_atLeafMeshTechniques[SHADER_TYPE_STANDARD] 
            };

            // load the tree's textures, build vertex and index buffers, etc.
            if (!pNewTree->InitGraphics(aSearchPaths, m_bCanRenderQuads, atTechniques, m_sRenderInfo.m_nMaxAnisotropy))
                CCore::SetError("CForestRI::ReplaceTree, InitGraphics failed");
            else
                bSuccess = true;
        }
        else
            bSuccess = true;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::AllocateTree

CForestRI_TemplateList
inline CTree* CForestRI_Class::AllocateTree(void) const
{
    return new ("CTree") CTreeRI_t;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::CullAndComputeLOD

CForestRI_TemplateList
inline void CForestRI_Class::CullAndComputeLOD(const CView& cView, SForestCullResults& sResults, bool bSort)
{
    CForest::CullAndComputeLOD(cView, sResults, bSort);

    // add the newly-available billboard VBOs to the list of available VBOs
    int i = 0;
	#ifdef SPEEDTREE_OPENMP
        #pragma omp critical
    #endif
    {
	    for (i = 0; i < int(sResults.m_aFreedBillboardVbos.size( )); ++i)
		    m_aBillboardVboPool.push_back((TGeometryBufferPolicy*) sResults.m_aFreedBillboardVbos[i]);
	}
	sResults.m_aFreedBillboardVbos.clear( );

    // assign VBOs to the list of newly-visible cells
    for (i = 0; i < int(sResults.m_aCellsToPopulateWithBillboards.size( )); ++i)
    {
        CTreeCell* pCell = sResults.m_aCellsToPopulateWithBillboards[i];
        assert(pCell);
        
        if (!pCell->GetBillboardVbo( ))
            pCell->SetBillboardVbo((void*) NextBillboardVbo( ));
    }

    // populate new billboard buffers
    PopulateBillboardVbos(sResults.m_aCellsToPopulateWithBillboards);
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::SetRenderInfo

CForestRI_TemplateList
inline void CForestRI_Class::SetRenderInfo(const SForestRenderInfo& sInfo)
{
    m_sRenderInfo = sInfo;

    EnableWind(m_sRenderInfo.m_bWindEnabled);
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::GetRenderInfo

CForestRI_TemplateList
inline const SForestRenderInfo& CForestRI_Class::GetRenderInfo(void) const
{
    return m_sRenderInfo;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::InitGraphics

CForestRI_TemplateList
inline bool CForestRI_Class::InitGraphics(void)
{
    bool bSuccess = false;

    // load all of the forest shaders & shader constants
    if (LoadShaders( ))
    {
        bSuccess = true;

        // when textures fail to load, we use very small fallback textures like white and black
        InitFallbackTextures( );

        // cascaded shadow maps
        if (ShadowsAreEnabled( ))
        {
            for (int nShadowMap = 0; nShadowMap < m_sRenderInfo.m_nNumShadowMaps; ++nShadowMap)
            {
                if (!m_atShadowBuffers[nShadowMap].Create(m_sRenderInfo.m_nShadowMapResolution, m_sRenderInfo.m_nShadowMapResolution))
                {
                    CCore::SetError("Failed to create %dx%d shadow buffer #%d of %d", m_sRenderInfo.m_nShadowMapResolution, m_sRenderInfo.m_nShadowMapResolution, nShadowMap, m_sRenderInfo.m_nNumShadowMaps);
                    bSuccess = false;
                }
            }
        }

        // prep billboard VBO pool
        bSuccess &= InitBillboardVboPool( );

        // init overlays to show cascaded shadow maps; return value ignored since this is diagnostic only
        (void) InitOverlays( );
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::StartRender

CForestRI_TemplateList
inline bool CForestRI_Class::StartRender(void)
{
    bool bSuccess = false;

    // upload global shader constants shared by all geometry, all passes; these values
    // are view-independent; view-dependent variables are uploaded with CForestRI::UploadViewShaderParameters
    {
        bSuccess = true;

        // shadow parameters
        if (ShadowsAreEnabled( ))
        {
            // when uploading the split distances, make sure the unused slots are ones, else the
            // shader technique we're using to efficiently pick the shadow map index will not work
            float afUploadedSplits[c_nMaxNumShadowMaps] = { 1.0f, 1.0f, 1.0f, 1.0f };
            const float* c_fShadowSplits = GetCascadedShadowMapDistances( );
            assert(m_sRenderInfo.m_nNumShadowMaps <= c_nMaxNumShadowMaps);
            for (int i = 0; i < m_sRenderInfo.m_nNumShadowMaps; ++i)
                afUploadedSplits[i] = c_fShadowSplits[i + 1];
            bSuccess &= m_tShadowSplitDistances.Set4fv(afUploadedSplits);

            // upload shadow fade parameters
            assert(m_sRenderInfo.m_nNumShadowMaps > 0);
            const float c_fShadowEndDist = afUploadedSplits[m_sRenderInfo.m_nNumShadowMaps - 1];
            float fSimpleFadePercent = m_fShadowFadePercentage * c_fShadowEndDist;
            float fTerrainAmbientOcclusionParam = 1.0f / (fSimpleFadePercent - afUploadedSplits[m_sRenderInfo.m_nNumShadowMaps - 1]); 
            float fShadowMapParam = 1.0f / (afUploadedSplits[m_sRenderInfo.m_nNumShadowMaps - 1] - fSimpleFadePercent); 
            bSuccess &= m_tShadowFadeParams.Set3f(fSimpleFadePercent, fTerrainAmbientOcclusionParam, fShadowMapParam);
        }

        // fog
        bSuccess &= m_tFogParams.Set4f(m_sRenderInfo.m_fFogEndDistance, m_sRenderInfo.m_fFogEndDistance - m_sRenderInfo.m_fFogStartDistance, -m_sRenderInfo.m_fSkyFogMin, -1.0f / (m_sRenderInfo.m_fSkyFogMax - m_sRenderInfo.m_fSkyFogMin));
        bSuccess &= m_tFogColor.Set3fv(m_sRenderInfo.m_vFogColor.TofloatPtr());

        // sky
        bSuccess &= m_tSkyColor.Set3fv(m_sRenderInfo.m_vSkyColor.TofloatPtr());
        bSuccess &= m_tSunColor.Set3fv(m_sRenderInfo.m_vSunColor.TofloatPtr());
        bSuccess &= m_tSunParams.Set3f(m_sRenderInfo.m_fSunSize, m_sRenderInfo.m_fSunSpreadExponent, m_sRenderInfo.m_fSunFogBloom);

        // user specifies light direction like DX/OpenGL, but the shader takes a negated value
		noVec3 mLightDir = -GetLightDir( );
        bSuccess &= m_tLightDir.Set3fv(mLightDir.TofloatPtr()); 

        // billboard index buffer is needed if quads aren't supported
        if (!m_bCanRenderQuads)
            bSuccess &= ResizeBillboardIndexBuffer(m_nHintMaxNumBillboardsPerCell * 2);
    }

    if (!bSuccess)
        CCore::SetError("CForestRI::StartRender() failed to set one or more shader constants");

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::EndRender

CForestRI_TemplateList
inline bool CForestRI_Class::EndRender(void)
{
    CForest::FrameEnd( );

    return true;
}


////////////////////////////////////////////////////////////
//  CForestRI::UploadViewShaderParameters
//
//  These shader parameters are all dependent on the current view, so expect
//  this function to be called multiple times in a multipass render.

CForestRI_TemplateList
inline bool CForestRI_Class::UploadViewShaderParameters(const CView& cView) const
{
    bool bSuccess = true;

    // update camera angles
    bSuccess &= m_tCameraAngles.Set2f(cView.GetCameraAzimuth( ), cView.GetCameraPitch( ));

    // upload other global shader variables
#ifdef SPEEDTREE_WORLD_TRANSLATE
    bSuccess &= m_tProjModelviewMatrix.SetMatrix(cView.GetCompositeNoTranslate( ));
#else
    bSuccess &= m_tProjModelviewMatrix.SetMatrix(cView.GetComposite( ).TofloatPtr());
#endif
    bSuccess &= m_tCameraPosition.Set3fv(cView.GetCameraPos( ).TofloatPtr());
    bSuccess &= m_tCameraDirection.Set3fv(cView.GetCameraDir( ).TofloatPtr());

    bSuccess &= m_tFarClip.Set1f(cView.GetFarClip( ));

    if (!bSuccess)
        CCore::SetError("CForestRI::UploadViewShaderParameters() failed to upload one or more shader constants");

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::BeginShadowMap

CForestRI_TemplateList
inline bool CForestRI_Class::BeginShadowMap(int nShadowMap, const CView& cLightView)
{
    bool bSuccess = false;

    if (ShadowsAreEnabled( ))
    {
        assert(nShadowMap >= 0 && nShadowMap < m_sRenderInfo.m_nNumShadowMaps);

        if (m_atShadowBuffers[nShadowMap].SetAsTarget( ))
        {
            bSuccess = true;

            // shader constants
            bSuccess &= m_tLightViewMatricesArray.SetMatrixIndex(cLightView.GetComposite( ), nShadowMap);

            // render state changes for rendering into shadow map
            m_tRenderStatePolicy.SetColorMask(false, false, false, false);

            // only need to clear the depth buffer when rendering into the shadow map
            m_tRenderStatePolicy.ClearScreen(false, true);

            if (!bSuccess)
                CCore::SetError("CForestRI::BeginShadowMap failed to upload one or more shader constants");
        }
        else
            CCore::SetError("CForestRI::BeginShadowMap, SetAsTarget() failed");
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::EndShadowMap

CForestRI_TemplateList
inline bool CForestRI_Class::EndShadowMap(int nShadowMap)
{
    bool bSuccess = false;

    if (ShadowsAreEnabled( ))
    {
        m_tRenderStatePolicy.SetPolygonOffset(0.0f, 0.0f);

        assert(nShadowMap >= 0 && nShadowMap < m_sRenderInfo.m_nNumShadowMaps);
        if (m_atShadowBuffers[nShadowMap].ReleaseAsTarget( ))
        {
            m_tRenderStatePolicy.SetColorMask(true, true, true, true);

            if (m_atShadowBuffers[nShadowMap].BindAsTexture( ) &&
                m_atShadowMaps[nShadowMap].SetTexture(m_atShadowBuffers[nShadowMap]))
            {
                bSuccess = true;
            }
            else
                CCore::SetError("CForestRI::EndShadowMap, Failed to bind shadow map as texture");
        }
        else
            CCore::SetError("CForestRI::EndShadowMap, ReleaseAsTarget() failed");
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::ShadowsAreEnabled

CForestRI_TemplateList
inline bool CForestRI_Class::ShadowsAreEnabled(void) const
{
    return (m_sRenderInfo.m_nShadowMapResolution > 0);
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::GetShadowDepthTexture

CForestRI_TemplateList
inline TDepthTexturePolicy* CForestRI_Class::GetShadowDepthTexture(int nShadowMap)
{
    return m_atShadowBuffers + nShadowMap;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::RenderBranches

CForestRI_TemplateList
inline bool CForestRI_Class::RenderBranches(const SForestCullResults& sVisibleTrees, ERenderPassType eRenderPass)
{
    bool bSuccess = false;

    // use the right shader, based on the type of rendering pass we're making (main, depth, shadow, etc.)
    TShaderTechniquePolicy& tRenderTechnique = PickTechnique(m_atBranchTechniques, eRenderPass);

    // bind the branch technique one time for rendering all of the 3D branches in the scene
    if (tRenderTechnique.Bind( ))
    {
        bSuccess = true;

        // set face culling mode based on the coordinate system and render pass type
        if (eRenderPass == RENDER_PASS_SHADOW)
            m_tRenderStatePolicy.SetFaceCulling(CCoordSys::IsLeftHanded( ) ? CULLTYPE_BACK : CULLTYPE_FRONT);
        else
            m_tRenderStatePolicy.SetFaceCulling(CCoordSys::IsLeftHanded( ) ? CULLTYPE_FRONT : CULLTYPE_BACK);

        // it's possible that this particular view has 3D instances in it but they have no branch geometry due to
        // their type or LOD state; in the interest of efficiency, no geometry buffer state change will occur until
        // this bool is true
        bool bGeometryFormatSet = false;

        // run through the 3D trees, in order of type (minimize state changes)
        for (TBaseToInstanceLodMap::const_iterator iBaseTree = sVisibleTrees.m_m3dInstances.begin( ); iBaseTree != sVisibleTrees.m_m3dInstances.end( ); ++iBaseTree)
        {
            const CTreeRI_t* pBaseTree = (CTreeRI_t*) iBaseTree->first;

            if (pBaseTree->HasGeometryType(GEOMETRY_TYPE_BRANCHES))
            {
                // get the list of 3D instances of this base type
                const TInstanceLodArray& aInstances = iBaseTree->second;
                int nNumInstances = int(aInstances.size( ));
                if (nNumInstances > 0)
                {
                    SRenderStats::SGeometryStats& sStats = m_sRenderStats.m_aGeometries[STATS_CATEGORY_BRANCHES][eRenderPass];

                    // get the geometry buffer associate with this species (SRT file)
                    const TGeometryBufferPolicy* pGeometryBuffer = pBaseTree->GetGeometryBuffer(GEOMETRY_TYPE_BRANCHES);
                    assert(pGeometryBuffer);

                    // at this point, we know we have to draw some branch geometry, so let's set the geometry buffer
                    // format, but only do it once (bGeometryVisible prevents it from happening again)
                    if (!bGeometryFormatSet)
                    {
                        bSuccess &= pGeometryBuffer->EnableFormat( );
                        bGeometryFormatSet = true;
                    }

                    // bind the vertex & index buffers for this base tree
                    bSuccess &= pGeometryBuffer->BindVertexBuffer( );
                    bSuccess &= pGeometryBuffer->BindIndexBuffer( );
                    ++sStats.m_uiNumVboBinds;

                    // if wind is enabled, upload the wind parameters
                    UploadBaseTreeParameters(pBaseTree);

                    const CLodRenderInfo* pLodRenderInfo = pBaseTree->GetLodRenderInfo(GEOMETRY_TYPE_BRANCHES);
                    assert(pLodRenderInfo);

                    const CSet<int>* pMaterialsUsed = pLodRenderInfo->MaterialRefs( );
                    assert(pMaterialsUsed);

                    for (CSet<int>::const_iterator iMaterial = pMaterialsUsed->begin( ); iMaterial != pMaterialsUsed->end( ); ++iMaterial)
                    {
                        const int nMaterialId = *iMaterial;

                        // upload the material associated with the current material id
                        int nNumTexturesBoundDuringUpload = UploadMaterial(&pBaseTree->GetMaterials( )->at(nMaterialId));

                        if (nNumTexturesBoundDuringUpload > 0)
                        {
                            bSuccess &= tRenderTechnique.CommitTextures( );
                        }

                        for (int nInstance = 0; nInstance < nNumInstances; ++nInstance)
                        {
                            const SInstanceLod* pInstanceLod = &(aInstances.at(nInstance));
                            const SLodSnapshot& sLod = pInstanceLod->m_sLodSnapshot;
                            if (sLod.m_nBranchLodIndex > -1)
                            {
                                const CInstance* pInstance = pInstanceLod->m_pInstance;
                                assert(pInstance);

                                // does this instance, at its current LOD, use the currently bound material?
                                const CLodRenderInfo::SRef* pRef = pLodRenderInfo->IsActive(sLod.m_nBranchLodIndex, nMaterialId);
                                if (pRef)
                                {
                                    // set instance's position & scale
                                    bSuccess &= m_tTreePosAndScale.Set3fvPlus1f(pInstance->GetPos( ), pInstance->GetScale( ));

                                    // set instance's rotation & LOD info
                                    const float* pRotationVector = pInstance->GetRotationVector( );
                                    bSuccess &= m_tTreeRotation.Set4f(pRotationVector[0], pRotationVector[1], pInstanceLod->m_afShaderLerps[GEOMETRY_TYPE_BRANCHES], pInstanceLod->m_fLod);

                                    // make sure all of the variables are updated
                                    bSuccess &= tRenderTechnique.CommitConstants( );

                                    // render the indexed triangle list
                                    bSuccess &= pGeometryBuffer->RenderIndexed(PRIMITIVE_TRIANGLES, pRef->m_nOffset, pRef->m_nLength);

                                    ++sStats.m_uiNumDrawCalls;
                                    sStats.m_uiNumTriangles += pRef->m_nLength / 3;
                                }
                            }
                        }
                    }
                    
                    sStats.m_uiNumObjects += unsigned int(nNumInstances);

                    bSuccess &= pGeometryBuffer->UnBindVertexBuffer( );
                    bSuccess &= pGeometryBuffer->UnBindIndexBuffer( );
                }
            }
        }

        if (bGeometryFormatSet)
            bSuccess &= TGeometryBufferPolicy::DisableFormat( );

        bSuccess &= tRenderTechnique.UnBind( );
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::RenderFronds

CForestRI_TemplateList
inline bool CForestRI_Class::RenderFronds(const SForestCullResults& sVisibleTrees, ERenderPassType eRenderPass)
{
    bool bSuccess = false;

    // use the right shader, based on the type of rendering pass we're making (main, depth, shadow, etc.)
    TShaderTechniquePolicy& tRenderTechnique = PickTechnique(m_atFrondTechniques, eRenderPass);

    // bind the frond technique one time for rendering all of the 3D fronds in the scene
    if (tRenderTechnique.Bind( ))
    {
        bSuccess = true;

        // we can always see both sides of the fronds, so no face culling
        m_tRenderStatePolicy.SetFaceCulling(CULLTYPE_NONE);

        // it's possible that this particular view has 3D instances in it but they have no frond geometry due to
        // their type or LOD state; in the interest of efficiency, no geometry buffer state change will occur until
        // this bool is true
        bool bGeometryFormatSet = false;

        // run through the 3D trees, in order of type (minimize state changes)
        for (TBaseToInstanceLodMap::const_iterator iBaseTree = sVisibleTrees.m_m3dInstances.begin( ); iBaseTree != sVisibleTrees.m_m3dInstances.end( ); ++iBaseTree)
        {
            const CTreeRI_t* pBaseTree = (CTreeRI_t*) iBaseTree->first;

            if (pBaseTree->HasGeometryType(GEOMETRY_TYPE_FRONDS))
            {
                // provide material & index offset/length information
                //const SDrawCallInfo* pDrawCallInfo = pBaseTree->GetDrawCallInfo(GEOMETRY_TYPE_FRONDS);

                // get the list of 3D instances of this base type
                const TInstanceLodArray& aInstances = iBaseTree->second;
                int nNumInstances = int(aInstances.size( ));
                if (nNumInstances > 0)
                {
                    SRenderStats::SGeometryStats& sStats = m_sRenderStats.m_aGeometries[STATS_CATEGORY_FRONDS][eRenderPass];

                    // get the geometry buffer associate with this species (SRT file)
                    const TGeometryBufferPolicy* pGeometryBuffer = pBaseTree->GetGeometryBuffer(GEOMETRY_TYPE_FRONDS);
                    assert(pGeometryBuffer);

                    // at this point, we know we have to draw some frond geometry, so let's set the geometry buffer
                    // format, but only do it once (bGeometryVisible prevents it from happening again)
                    if (!bGeometryFormatSet)
                    {
                        bSuccess &= pGeometryBuffer->EnableFormat( );
                        bGeometryFormatSet = true;
                    }

                    // bind the vertex & index buffers for this base tree
                    bSuccess &= pGeometryBuffer->BindVertexBuffer( );
                    bSuccess &= pGeometryBuffer->BindIndexBuffer( );
                    ++sStats.m_uiNumVboBinds;

                    // if wind is enabled, upload the wind parameters
                    UploadBaseTreeParameters(pBaseTree);

                    const CLodRenderInfo* pLodRenderInfo = pBaseTree->GetLodRenderInfo(GEOMETRY_TYPE_FRONDS);
                    assert(pLodRenderInfo);

                    const CSet<int>* pMaterialsUsed = pLodRenderInfo->MaterialRefs( );
                    assert(pMaterialsUsed);

                    for (CSet<int>::const_iterator iMaterial = pMaterialsUsed->begin( ); iMaterial != pMaterialsUsed->end( ); ++iMaterial)
                    {
                        const int nMaterialId = *iMaterial;

                        // upload the material associated with the current material id
                        int nNumTexturesBoundDuringUpload = UploadMaterial(&pBaseTree->GetMaterials( )->at(nMaterialId));
                        if (nNumTexturesBoundDuringUpload > 0)
                        {
                            bSuccess &= tRenderTechnique.CommitTextures( );
                        }

                        for (int nInstance = 0; nInstance < nNumInstances; ++nInstance)
                        {
                            const SInstanceLod* pInstanceLod = &(aInstances.at(nInstance));
                            const SLodSnapshot& sLod = pInstanceLod->m_sLodSnapshot;
                            if (sLod.m_nFrondLodIndex > -1)
                            {
                                const CInstance* pInstance = pInstanceLod->m_pInstance;
                                assert(pInstance);

                                // does this instance, at its current LOD, use the currently bound material?
                                const CLodRenderInfo::SRef* pRef = pLodRenderInfo->IsActive(sLod.m_nFrondLodIndex, nMaterialId);
                                if (pRef)
                                {
                                    // set instance's position & scale
                                    bSuccess &= m_tTreePosAndScale.Set3fvPlus1f(pInstance->GetPos( ), pInstance->GetScale( ));

                                    // set instance's rotation & LOD info
                                    const float* pRotationVector = pInstance->GetRotationVector( );
                                    bSuccess &= m_tTreeRotation.Set4f(pRotationVector[0], pRotationVector[1], pInstanceLod->m_afShaderLerps[GEOMETRY_TYPE_FRONDS], pInstanceLod->m_fLod);

                                    // make sure all of the variables are updated
                                    bSuccess &= tRenderTechnique.CommitConstants( );

                                    // render the indexed triangle list
                                    bSuccess &= pGeometryBuffer->RenderIndexed(PRIMITIVE_TRIANGLES, pRef->m_nOffset, pRef->m_nLength);

                                    ++sStats.m_uiNumDrawCalls;
                                    sStats.m_uiNumTriangles += pRef->m_nLength / 3;
                                }
                            }
                        }
                    }

                    sStats.m_uiNumObjects += unsigned int(nNumInstances);

                    bSuccess &= pGeometryBuffer->UnBindVertexBuffer( );
                    bSuccess &= pGeometryBuffer->UnBindIndexBuffer( );
                }
            }
        }

        if (bGeometryFormatSet)
            bSuccess &= TGeometryBufferPolicy::DisableFormat( );

        bSuccess &= tRenderTechnique.UnBind( );
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::RenderLeafMeshes

CForestRI_TemplateList
inline bool CForestRI_Class::RenderLeafMeshes(const SForestCullResults& sVisibleTrees, ERenderPassType eRenderPass)
{
    bool bSuccess = false;

    // use the right shader, based on the type of rendering pass we're making (main, depth, shadow, etc.)
    TShaderTechniquePolicy& tRenderTechnique = PickTechnique(m_atLeafMeshTechniques, eRenderPass);

    // bind the leaf mesh technique one time for rendering all of the 3D leaf meshes in the scene
    if (tRenderTechnique.Bind( ))
    {
        bSuccess = true;

        // leaf meshes aren't convex objects, so face culling is out
        m_tRenderStatePolicy.SetFaceCulling(CULLTYPE_NONE);

        // it's possible that this particular view has 3D instances in it but they have no leaf mesh geometry due to
        // their type or LOD state; in the interest of efficiency, no geometry buffer state change will occur until
        // this bool is true
        bool bGeometryFormatSet = false;

        // run through the 3D trees, in order of type (minimize state changes)
        for (TBaseToInstanceLodMap::const_iterator iBaseTree = sVisibleTrees.m_m3dInstances.begin( ); iBaseTree != sVisibleTrees.m_m3dInstances.end( ); ++iBaseTree)
        {
            const CTreeRI_t* pBaseTree = (CTreeRI_t*) iBaseTree->first;

            if (pBaseTree->HasGeometryType(GEOMETRY_TYPE_LEAF_MESHES))
            {
                // get the list of 3D instances of this base type
                const TInstanceLodArray& aInstances = iBaseTree->second;
                int nNumInstances = int(aInstances.size( ));
                if (nNumInstances > 0)
                {
                    SRenderStats::SGeometryStats& sStats = m_sRenderStats.m_aGeometries[STATS_CATEGORY_LEAF_MESHES][eRenderPass];

                    // get the geometry buffer associate with this species (SRT file)
                    const TGeometryBufferPolicy* pGeometryBuffer = pBaseTree->GetGeometryBuffer(GEOMETRY_TYPE_LEAF_MESHES);
                    assert(pGeometryBuffer);

                    // at this point, we know we have to draw some leaf mesh geometry, so let's set the geometry buffer
                    // format, but only do it once (bGeometryVisible prevents it from happening again)
                    if (!bGeometryFormatSet)
                    {
                        bSuccess &= pGeometryBuffer->EnableFormat( );
                        bGeometryFormatSet = true;
                    }

                    // bind the vertex & index buffers for this base tree
                    bSuccess &= pGeometryBuffer->BindVertexBuffer( );
                    bSuccess &= pGeometryBuffer->BindIndexBuffer( );
                    ++sStats.m_uiNumVboBinds;

                    // if wind is enabled, upload the wind parameters
                    UploadBaseTreeParameters(pBaseTree);

                    const CLodRenderInfo* pLodRenderInfo = pBaseTree->GetLodRenderInfo(GEOMETRY_TYPE_LEAF_MESHES);
                    assert(pLodRenderInfo);

                    const CSet<int>* pMaterialsUsed = pLodRenderInfo->MaterialRefs( );
                    assert(pMaterialsUsed);

                    for (CSet<int>::const_iterator iMaterial = pMaterialsUsed->begin( ); iMaterial != pMaterialsUsed->end( ); ++iMaterial)
                    {
                        const int nMaterialId = *iMaterial;

                        // upload the material associated with the current material id
                        int nNumTexturesBoundDuringUpload = UploadMaterial(&pBaseTree->GetMaterials( )->at(nMaterialId));
                        if (nNumTexturesBoundDuringUpload > 0)
                        {
                            bSuccess &= tRenderTechnique.CommitTextures( );
                        }

                        for (int nInstance = 0; nInstance < nNumInstances; ++nInstance)
                        {
                            const SInstanceLod* pInstanceLod = &(aInstances.at(nInstance));

                            const SLodSnapshot& sLod = pInstanceLod->m_sLodSnapshot;
                            if (sLod.m_nLeafMeshLodIndex > -1)
                            {
                                const CInstance* pInstance = pInstanceLod->m_pInstance;
                                assert(pInstance);

                                // does this instance, at its current LOD, use the currently bound material?
                                const CLodRenderInfo::SRef* pRef = pLodRenderInfo->IsActive(sLod.m_nLeafMeshLodIndex, nMaterialId);
                                if (pRef)
                                {
                                    // set instance's position & scale
                                    bSuccess &= m_tTreePosAndScale.Set3fvPlus1f(pInstance->GetPos( ), pInstance->GetScale( ));

                                    // set instance's rotation & LOD info
                                    const float* pRotationVector = pInstance->GetRotationVector( );
                                    bSuccess &= m_tTreeRotation.Set4f(pRotationVector[0], pRotationVector[1], pInstanceLod->m_afShaderLerps[GEOMETRY_TYPE_LEAF_MESHES], pInstanceLod->m_fLod);

                                    // make sure all of the variables are updated
                                    bSuccess &= tRenderTechnique.CommitConstants( );

                                    // render the indexed triangle list
                                    bSuccess &= pGeometryBuffer->RenderIndexed(PRIMITIVE_TRIANGLES, pRef->m_nOffset, pRef->m_nLength);

                                    ++sStats.m_uiNumDrawCalls;
                                    sStats.m_uiNumTriangles += pRef->m_nLength / 3;
                                }
                            }
                        }
                    }

                    sStats.m_uiNumObjects += unsigned int(nNumInstances);

                    bSuccess &= pGeometryBuffer->UnBindVertexBuffer( );
                    bSuccess &= pGeometryBuffer->UnBindIndexBuffer( );
                }
            }
        }

        if (bGeometryFormatSet)
            bSuccess &= TGeometryBufferPolicy::DisableFormat( );

        bSuccess &= tRenderTechnique.UnBind( );
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::RenderLeafCards

CForestRI_TemplateList
inline bool CForestRI_Class::RenderLeafCards(const SForestCullResults& sVisibleTrees, ERenderPassType eRenderPass, const CView& cView)
{
    bool bSuccess = false;

    // use the right shader, based on the type of rendering pass we're making (main, depth, shadow, etc.)
    TShaderTechniquePolicy& tRenderTechnique = PickTechnique(m_atLeafCardTechniques, eRenderPass);

    // bind the leaf card technique one time for rendering all of the 3D leaf cards in the scene
    if (tRenderTechnique.Bind( ))
    {
        bSuccess = true;

        // since the cards face the camera, checking for backfacing triangles is a waste
        m_tRenderStatePolicy.SetFaceCulling(CULLTYPE_NONE);

        // upload a single matrix used by the leaf card shader to billboard every leaf card in the forest
        bSuccess &= m_tCameraFacingMatrix.SetMatrix(cView.GetCameraFacingMatrix( ));

        // it's possible that this particular view has 3D instances in it but they have no leaf card geometry due to
        // their type or LOD state; in the interest of efficiency, no geometry buffer state change will occur until
        // this bool is true
        bool bGeometryFormatSet = false;

        // run through the 3D trees, in order of type (minimize state changes)
        for (TBaseToInstanceLodMap::const_iterator iBaseTree = sVisibleTrees.m_m3dInstances.begin( ); iBaseTree != sVisibleTrees.m_m3dInstances.end( ); ++iBaseTree)
        {
            const CTreeRI_t* pBaseTree = (CTreeRI_t*) iBaseTree->first;
            assert(pBaseTree);

            if (pBaseTree->HasGeometryType(GEOMETRY_TYPE_LEAF_CARDS))
            {
                // how the cards will be rendered
                const bool c_bRenderAsQuads = pBaseTree->CardsAreQuads( );

                // get the list of 3D instances of this base type
                const TInstanceLodArray& aInstances = iBaseTree->second;
                int nNumInstances = int(aInstances.size( ));
                if (nNumInstances > 0)
                {
                    SRenderStats::SGeometryStats& sStats = m_sRenderStats.m_aGeometries[STATS_CATEGORY_LEAF_CARDS][eRenderPass];

                    // get the geometry buffer associate with this species (SRT file)
                    const TGeometryBufferPolicy* pGeometryBuffer = pBaseTree->GetGeometryBuffer(GEOMETRY_TYPE_LEAF_CARDS);
                    assert(pGeometryBuffer);

                    // at this point, we know we have to draw some leaf card geometry, so let's set the geometry buffer
                    // format, but only do it once (bGeometryVisible prevents it from happening again)
                    if (!bGeometryFormatSet)
                    {
                        bSuccess &= pGeometryBuffer->EnableFormat( );
                        bGeometryFormatSet = true;
                    }

                    // bind the vertex & index buffers for this base tree
                    bSuccess &= pGeometryBuffer->BindVertexBuffer( );
                    ++sStats.m_uiNumVboBinds;

                    // if wind is enabled, upload the wind parameters
                    UploadBaseTreeParameters(pBaseTree);

                    const CLodRenderInfo* pLodRenderInfo = pBaseTree->GetLodRenderInfo(GEOMETRY_TYPE_LEAF_CARDS);
                    assert(pLodRenderInfo);

                    const CSet<int>* pMaterialsUsed = pLodRenderInfo->MaterialRefs( );
                    assert(pMaterialsUsed);

                    for (CSet<int>::const_iterator iMaterial = pMaterialsUsed->begin( ); iMaterial != pMaterialsUsed->end( ); ++iMaterial)
                    {
                        const int nMaterialId = *iMaterial;

                        // upload the material associated with the current material id
                        int nNumTexturesBoundDuringUpload = UploadMaterial(&pBaseTree->GetMaterials( )->at(nMaterialId));
                        if (nNumTexturesBoundDuringUpload > 0)
                        {
                            bSuccess &= tRenderTechnique.CommitTextures( );
                        }

                        for (int nInstance = 0; nInstance < nNumInstances; ++nInstance)
                        {
                            const SInstanceLod* pInstanceLod = &(aInstances.at(nInstance));
                            const SLodSnapshot& sLod = pInstanceLod->m_sLodSnapshot;
                            if (sLod.m_nLeafCardLodIndex > -1)
                            {
                                const CInstance* pInstance = pInstanceLod->m_pInstance;
                                assert(pInstance);

                                // does this instance, at its current LOD, use the currently bound material?
                                const CLodRenderInfo::SRef* pRef = pLodRenderInfo->IsActive(sLod.m_nLeafCardLodIndex, nMaterialId);
                                if (pRef)
                                {
                                    // set instance's position & scale
                                    bSuccess &= m_tTreePosAndScale.Set3fvPlus1f(pInstance->GetPos( ), pInstance->GetScale( ));

                                    // set instance's rotation & LOD info
                                    const float* pRotationVector = pInstance->GetRotationVector( );
                                    bSuccess &= m_tTreeRotation.Set4f(pRotationVector[0], pRotationVector[1], pInstanceLod->m_afShaderLerps[GEOMETRY_TYPE_LEAF_CARDS], pInstanceLod->m_fLod);

                                    // make sure all of the variables are updated
                                    bSuccess &= tRenderTechnique.CommitConstants( );

                                    // render the indexed triangle list
                                    if (c_bRenderAsQuads)
                                        bSuccess &= pGeometryBuffer->RenderArrays(PRIMITIVE_QUADS, pRef->m_nOffset, pRef->m_nLength);
                                    else
                                        bSuccess &= pGeometryBuffer->RenderArrays(PRIMITIVE_TRIANGLES, pRef->m_nOffset, pRef->m_nLength);

                                    ++sStats.m_uiNumDrawCalls;
                                    sStats.m_uiNumTriangles += pRef->m_nLength / 3;
                                }
                            }
                        }
                    }

                    sStats.m_uiNumObjects += unsigned int(nNumInstances);

                    bSuccess &= pGeometryBuffer->UnBindVertexBuffer( );
                }
            }
        }

        if (bGeometryFormatSet)
            bSuccess &= TGeometryBufferPolicy::DisableFormat( );

        bSuccess &= tRenderTechnique.UnBind( );
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::EnableTexturing

CForestRI_TemplateList
inline void CForestRI_Class::EnableTexturing(bool bFlag)
{
    m_bTexturingEnabled = bFlag;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::TransparentTextureModeNotify

CForestRI_TemplateList
inline void CForestRI_Class::TransparentTextureModeNotify(ETextureAlphaRenderMode eMode)
{
    m_eTextureAlphaMethod = eMode;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::GetRenderStats

CForestRI_TemplateList
inline SRenderStats& CForestRI_Class::GetRenderStats(void)
{
    return m_sRenderStats;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::GetShaderLoader

CForestRI_TemplateList
inline TShaderLoaderPolicy* CForestRI_Class::GetShaderLoader(void)
{
    return &m_tShaderLoader;
}


////////////////////////////////////////////////////////////
// CForestRI::ShaderNameByPass

CForestRI_TemplateList
inline CFixedString CForestRI_Class::ShaderNameByPass(const CFixedString& strBaseName, EShaderType eShaderType)
{
    switch (eShaderType)
    {
    case SHADER_TYPE_DEPTH_ONLY:
        return strBaseName + "_Depth";
    case SHADER_TYPE_SHADOW:
        return strBaseName + "_Shadow";
    default:
        // do nothing since other shader type enum values do not require a suffix
        break;
    }

    return strBaseName;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::LoadShaders
//
//  Shader load failure is considered fatal.  LoadShaders() will only return
//  true upon complete success of loading all shaders & shader constants.

CForestRI_TemplateList
inline bool CForestRI_Class::LoadShaders(void)
{
    bool bSuccess = false;

    if (!m_sRenderInfo.m_strShaderFilename.empty( ))
    {
        CArray<CString> aDefines;

        // upload hints about the graphics API
#ifdef SPEEDTREE_OPENGL
        aDefines.push_back("SPEEDTREE_OPENGL");
#endif
#if defined(SPEEDTREE_DIRECTX9) || defined(_XBOX)
        aDefines.push_back("SPEEDTREE_DIRECTX9");
#endif
#ifdef SPEEDTREE_DIRECTX10
        aDefines.push_back("SPEEDTREE_DIRECTX10");
#endif
        aDefines.push_back("");

        // upload hints about the platform
#ifdef _XBOX
        aDefines.push_back("_XBOX");
        aDefines.push_back("");
        aDefines.push_back("SPEEDTREE_XBOX_360");
        aDefines.push_back("");
#endif
#ifdef __CELLOS_LV2__
        aDefines.push_back("SPEEDTREE_PS3");
        aDefines.push_back("");
#endif

        // upload information about the user's coordinate system
        if (CCoordSys::IsLeftHanded( ))
            aDefines.push_back("SPEEDTREE_LHCS");
        else
            aDefines.push_back("SPEEDTREE_RHCS");
        aDefines.push_back("");
        if (fabs(CCoordSys::UpAxis( ).y) > 0.0f)
            aDefines.push_back("SPEEDTREE_Y_UP");
        else
            aDefines.push_back("SPEEDTREE_Z_UP");
        aDefines.push_back("");

#ifdef SPEEDTREE_WORLD_TRANSLATE
        aDefines.push_back("SPEEDTREE_WORLD_TRANSLATE");
        aDefines.push_back("");
#endif

        // shadow map resolution
        if (ShadowsAreEnabled( ))
        {
            aDefines.push_back("SPEEDTREE_SHADOW_MAP_RES");
            aDefines.push_back(CString::Format("%d", m_sRenderInfo.m_nShadowMapResolution));

            aDefines.push_back("SPEEDTREE_NUM_SHADOW_MAPS");
            aDefines.push_back(CString::Format("%d", m_sRenderInfo.m_nNumShadowMaps));
        }

        aDefines.push_back("SPEEDTREE_SMOOTH_SHADOWS");
        aDefines.push_back(m_sRenderInfo.m_bSmoothShadows ? "true" : "false");

        // max billboard images
        aDefines.push_back("SPEEDTREE_MAX_BILLBOARD_IMAGES");
        aDefines.push_back(CString::Format("%d", m_sRenderInfo.m_nMaxBillboardImagesByBase));

        if (m_sRenderInfo.m_bZPrePass)
        {
            aDefines.push_back("SPEEDTREE_Z_PREPASS");
            aDefines.push_back("");
        }

        if (IsWindEnabled( ))
        {
            aDefines.push_back("SPEEDTREE_DIRECTIONAL_WIND");
            aDefines.push_back("");
        }

        if (m_sRenderInfo.m_bFrondRippling)
        {
            aDefines.push_back("SPEEDTREE_FROND_RIPPLING");
            aDefines.push_back("");
        }

        if (m_sRenderInfo.m_bShowShadowSplitsOnTerrain)
        {
            aDefines.push_back("SPEEDTREE_TERRAIN_SHOW_SPLITS");
            aDefines.push_back("");
        }

        if (m_sRenderInfo.m_nMaxAnisotropy > 0)
        {
            aDefines.push_back("SPEEDTREE_MAX_ANISOTROPY");
            aDefines.push_back(CString::Format("%d", m_sRenderInfo.m_nMaxAnisotropy));
        }

        if (m_eTextureAlphaMethod == TRANS_TEXTURE_ALPHA_TESTING)
        {
            aDefines.push_back("SPEEDTREE_ALPHA_TESTING");
            aDefines.push_back("");
        }

        if (m_sRenderInfo.m_bHorizontalBillboards)
        {
            aDefines.push_back("SPEEDTREE_HORZ_BBS_ACTIVE");
            aDefines.push_back("");
        }

        // lighting
        {
            aDefines.push_back("SPEEDTREE_SPECULAR_LIGHTING");
            aDefines.push_back(m_sRenderInfo.m_bSpecularLighting ? "true" : "false");

            aDefines.push_back("SPEEDTREE_TRANSMISSION_LIGHTING");
            aDefines.push_back(m_sRenderInfo.m_bTransmissionLighting ? "true" : "false");

            aDefines.push_back("SPEEDTREE_AMBIENT_CONTRAST");
            aDefines.push_back(m_sRenderInfo.m_bAmbientContrast ? "true" : "false");

            aDefines.push_back("SPEEDTREE_DETAIL_TEXTURE");
            aDefines.push_back(m_sRenderInfo.m_bDetailLayer ? "true" : "false");

            aDefines.push_back("SPEEDTREE_DETAIL_NORMAL_MAPPING");
            aDefines.push_back(m_sRenderInfo.m_bDetailNormalMapping ? "true" : "false");
        }

        if (m_tShaderLoader.Load(m_sRenderInfo.m_strShaderFilename.c_str( ), aDefines))
        {
            if (QueryUniformVariables( )) // if any shader constants aren't found, we abort
            {
                // return values are ignored; bad loads are trapped by querying IsValid() on each shader below
                bSuccess = true;
                for (int nShaderType = int(SHADER_TYPE_STANDARD); nShaderType < int(SHADER_TYPE_NUMTYPES); ++nShaderType)
                {
                    // branches
                    CFixedString strShaderName = ShaderNameByPass("Branches", EShaderType(nShaderType));
                    (void) m_tShaderLoader.GetTechnique(strShaderName.c_str( ), m_atBranchTechniques[nShaderType]);

                    // fronds
                    strShaderName = ShaderNameByPass("Fronds", EShaderType(nShaderType));
                    (void) m_tShaderLoader.GetTechnique(strShaderName.c_str( ), m_atFrondTechniques[nShaderType]);

                    // leaf cards
                    strShaderName = ShaderNameByPass("LeafCards", EShaderType(nShaderType));
                    (void) m_tShaderLoader.GetTechnique(strShaderName.c_str( ), m_atLeafCardTechniques[nShaderType]);

                    // leaf meshes
                    strShaderName = ShaderNameByPass("LeafMeshes", EShaderType(nShaderType));
                    (void) m_tShaderLoader.GetTechnique(strShaderName.c_str( ), m_atLeafMeshTechniques[nShaderType]);

                    // billboards
                    strShaderName = ShaderNameByPass("Billboards", EShaderType(nShaderType));
                    (void) m_tShaderLoader.GetTechnique(strShaderName.c_str( ), m_atBillboardTechniques[nShaderType]);

                    bSuccess &= m_atBranchTechniques[nShaderType].IsValid( ) &&
                                m_atFrondTechniques[nShaderType].IsValid( ) &&
                                m_atLeafCardTechniques[nShaderType].IsValid( ) &&
                                m_atLeafMeshTechniques[nShaderType].IsValid( ) &&
                                m_atBillboardTechniques[nShaderType].IsValid( );
                }

                // overlay
                bSuccess &= SetupOverlayShader( );
            }
        }
        else
            CCore::SetError("CForestRI::LoadShaders, shader file [%s] failed to load", m_sRenderInfo.m_strShaderFilename.c_str( ));
    }
    else
        CCore::SetError("CForestRI::LoadShaders, no shader file has been set");

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::QueryUniformVariables

CForestRI_TemplateList
inline bool CForestRI_Class::QueryUniformVariables(void)
{
    bool bSuccess = false;

    // main global shader constants, shared by more than one shader
    bSuccess = m_tShaderLoader.GetConstant("g_mModelViewProj", m_tProjModelviewMatrix);
    bSuccess &= m_tShaderLoader.GetConstant("g_mCameraFacingMatrix", m_tCameraFacingMatrix);
    bSuccess &= m_tShaderLoader.GetConstant("g_vTreePosAndScale", m_tTreePosAndScale);
    bSuccess &= m_tShaderLoader.GetConstant("g_vTreeRotation", m_tTreeRotation);
    bSuccess &= m_tShaderLoader.GetConstant("g_vLodProfile", m_tLodProfile);
    bSuccess &= m_tShaderLoader.GetConstant("g_mMaterial", m_tMaterial);
    bSuccess &= m_tShaderLoader.GetConstant("g_vMaterialExtras", m_tMaterialExtras);
    bSuccess &= m_tShaderLoader.GetConstant("g_vMaterialTransmission", m_tMaterialTransmission);
    bSuccess &= m_tShaderLoader.GetConstant("g_vLightDir", m_tLightDir);
    bSuccess &= m_tShaderLoader.GetConstant("g_vFogParams", m_tFogParams);
    bSuccess &= m_tShaderLoader.GetConstant("g_vFogColor", m_tFogColor);
    bSuccess &= m_tShaderLoader.GetConstant("g_vCameraAngles", m_tCameraAngles);
    bSuccess &= m_tShaderLoader.GetConstant("g_vCameraPosition", m_tCameraPosition);
    bSuccess &= m_tShaderLoader.GetConstant("g_vCameraDirection", m_tCameraDirection);
    bSuccess &= m_tShaderLoader.GetConstant("g_fFarClip", m_tFarClip);

    // wind
    if (IsWindEnabled( ))
    {
        bSuccess &= m_tShaderLoader.GetConstant("g_vWindDir", m_tWindDir);
        bSuccess &= m_tShaderLoader.GetConstant("g_vWindTimes", m_tWindTimes);
        bSuccess &= m_tShaderLoader.GetConstant("g_vWindDistances", m_tWindDistances);
        bSuccess &= m_tShaderLoader.GetConstant("g_vWindLeaves", m_tWindLeaves);
        bSuccess &= m_tShaderLoader.GetConstant("g_vWindFrondRipple", m_tWindFrondRipple);
        bSuccess &= m_tShaderLoader.GetConstant("g_vWindGust", m_tWindGust);
        bSuccess &= m_tShaderLoader.GetConstant("g_vWindGustHints", m_tWindGustHints);
    }

    // billboards
    bSuccess &= m_tShaderLoader.GetConstant("g_fNum360Images", m_tNum360Images);
    bSuccess &= m_tShaderLoader.GetConstant("g_vCameraAzimuthTrig", m_tCameraAzimuthTrig);
    bSuccess &= m_tShaderLoader.GetConstant("g_av360TexCoords", m_t360BillboardTexCoords);
    bSuccess &= m_tShaderLoader.GetConstant("g_avBillboardTangents", m_tBillboardTangents);
    bSuccess &= m_tShaderLoader.GetConstant("g_fHorzBillboardFade", m_tHorzBillboardFade);
    bSuccess &= m_tShaderLoader.GetConstant("g_vHorzBillboardTexCoords", m_tHorzBillboardTexCoords);

    // sky
    bSuccess &= m_tShaderLoader.GetConstant("g_vSkyColor", m_tSkyColor);
    bSuccess &= m_tShaderLoader.GetConstant("g_vSunColor", m_tSunColor);
    bSuccess &= m_tShaderLoader.GetConstant("g_vSunParams", m_tSunParams);

    // shadows
    if (ShadowsAreEnabled( ))
    {
        bSuccess &= m_tShaderLoader.GetConstant("g_amLightViewProjs", m_tLightViewMatricesArray);
        bSuccess &= m_tShaderLoader.GetConstant("g_vSplitDistances", m_tShadowSplitDistances);
        bSuccess &= m_tShaderLoader.GetConstant("g_vShadowFadeParams", m_tShadowFadeParams);

        // shadow maps
        bSuccess &= m_tShaderLoader.GetConstant(TextureShaderConstantName("ShadowMap0").c_str( ), m_atShadowMaps[0]);
        bSuccess &= m_tShaderLoader.GetConstant(TextureShaderConstantName("ShadowMap1").c_str( ), m_atShadowMaps[1]);
        bSuccess &= m_tShaderLoader.GetConstant(TextureShaderConstantName("ShadowMap2").c_str( ), m_atShadowMaps[2]);
        bSuccess &= m_tShaderLoader.GetConstant(TextureShaderConstantName("ShadowMap3").c_str( ), m_atShadowMaps[3]);
    }

    // textures
    bSuccess &= m_tShaderLoader.GetConstant(TextureShaderConstantName("Diffuse").c_str( ), m_atTextures[TL_DIFFUSE]);
    bSuccess &= m_tShaderLoader.GetConstant(TextureShaderConstantName("Normal").c_str( ), m_atTextures[TL_NORMAL]);
    bSuccess &= m_tShaderLoader.GetConstant(TextureShaderConstantName("Specular").c_str( ), m_atTextures[TL_SPECULAR]);
    bSuccess &= m_tShaderLoader.GetConstant(TextureShaderConstantName("DetailDiffuse").c_str( ), m_atTextures[TL_DETAIL_DIFFUSE]);
    bSuccess &= m_tShaderLoader.GetConstant(TextureShaderConstantName("DetailNormal").c_str( ), m_atTextures[TL_DETAIL_NORMAL]);

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::PickTechnique

CForestRI_TemplateList
inline TShaderTechniquePolicy& CForestRI_Class::PickTechnique(TShaderTechniquePolicy atAvailableTechniques[SHADER_TYPE_NUMTYPES], ERenderPassType eRenderPass)
{
    EShaderType eType = SHADER_TYPE_STANDARD;

    switch (eRenderPass)
    {
    case RENDER_PASS_DEPTH_ONLY:
        eType = SHADER_TYPE_DEPTH_ONLY;
        break;
    case RENDER_PASS_SHADOW:
        eType = SHADER_TYPE_SHADOW;
        break;
    default:
        // do nothing since eType defaults to SHADER_TYPE_STANDARD
        break;
    }

    // some of these different shader types might not exist or have loaded successfully, so fall back to
    // the standard shader
    if (!atAvailableTechniques[eType].IsValid( ))
        eType = SHADER_TYPE_STANDARD;

    return atAvailableTechniques[eType];
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::UploadMaterial

CForestRI_TemplateList
inline int CForestRI_Class::UploadMaterial(const SMaterialRI_t* pMaterial, bool bTextureOnly)
{
    assert(pMaterial);

    int nNumTexturesBound = 0;

    // upload color components of material
    if (!bTextureOnly)
    {
        const noVec4 c_avModifiedMaterial[4] = 
        {
            // ambient
            pMaterial->m_vAmbient * m_sRenderInfo.m_sLightMaterial.m_vAmbient,
            // diffuse
            pMaterial->m_vDiffuse * m_sRenderInfo.m_sLightMaterial.m_vDiffuse * pMaterial->m_fLightScalar * m_sRenderInfo.m_fGlobalLightScalar,
            // specular
            pMaterial->m_vSpecular * m_sRenderInfo.m_sLightMaterial.m_vSpecular,
            // emissive
            pMaterial->m_vEmissive * 3.0f, // not multiplied against light since it's the transmission component
        };
        (void) m_tMaterial.SetMatrix(c_avModifiedMaterial[0]); // uploads all 16 float values at once
        (void) m_tMaterialTransmission.Set2f(pMaterial->m_fTransmissionShadow, pMaterial->m_fTransmissionViewDependence);

        // alpha scalar must be adjusted depending on the transparent render mode:
        //  - if alpha-to-coverage, no adjustment made since that's the mode the value was tuned with in the modeler
        //  - if alpha testing, lower the scalar to allow fewer pixels through, mimicking the look of a2c.
        float fAlphaScalar = (m_eTextureAlphaMethod == TRANS_TEXTURE_ALPHA_TESTING) ? pMaterial->m_fAlphaScalar * m_sRenderInfo.m_fAlphaTestScalar : pMaterial->m_fAlphaScalar;
        (void) m_tMaterialExtras.Set4f(pMaterial->m_fLightScalar * m_sRenderInfo.m_fGlobalLightScalar, fAlphaScalar, pMaterial->m_fShininess, 1.0f - pMaterial->m_fAmbientContrast);
    }

    // upload textures
    for (int nLayer = 0; nLayer < TL_NUM_TEX_LAYERS; ++nLayer)
    {
        const CTextureRI<TTexturePolicy>& cTextureObject = pMaterial->m_acTextureObjects[nLayer];

        if (!m_bTexturingEnabled || !cTextureObject.IsValid( ))
        {
            (void) m_atTextures[nLayer].SetTexture(m_atFallbackTextures[nLayer]);
        }
        else
        {
            (void) m_atTextures[nLayer].SetTexture(cTextureObject.m_tTexturePolicy);
            ++nNumTexturesBound;
        }
    }

    return nNumTexturesBound;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::UploadBaseTreeParameters

CForestRI_TemplateList
inline bool CForestRI_Class::UploadBaseTreeParameters(const CTreeRI_t* pBaseTree) const
{
    bool bSuccess = true;

    // upload the LOD range so the shaders can adjust LOD 
    const SLodProfile& sLodProfile = pBaseTree->GetLodProfile( );
    bSuccess &= m_tLodProfile.Set4fv(reinterpret_cast<const float*>(&sLodProfile)); // cast for quicker upload

    // wind parameters
    bSuccess &= UploadWindParams(pBaseTree->GetWind( ));

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::UploadWindParams

CForestRI_TemplateList
inline bool CForestRI_Class::UploadWindParams(const CWind& cWind) const
{
    bool bSuccess = false;

    if (IsWindEnabled( ))
    {
        // get updated shader values for this wind object
        const float* pShaderValues = cWind.GetShaderValues( );
        assert(pShaderValues);

        bSuccess = true;
        bSuccess &= m_tWindDir.Set3fv(pShaderValues + CWind::SH_WIND_DIR_X);
        bSuccess &= m_tWindTimes.Set4fv(pShaderValues + CWind::SH_TIME_PRIMARY);
        bSuccess &= m_tWindDistances.Set4fv(pShaderValues + CWind::SH_DIST_PRIMARY);
        bSuccess &= m_tWindLeaves.Set3fv(pShaderValues + CWind::SH_DIST_LEAVES);
        bSuccess &= m_tWindFrondRipple.Set3fv(pShaderValues + CWind::SH_DIST_FROND_RIPPLE);
        bSuccess &= m_tWindGust.Set3fv(pShaderValues + CWind::SH_STRENGTH_COMBINED);
        bSuccess &= m_tWindGustHints.Set3fv(pShaderValues + CWind::SH_HEIGHT_OFFSET);
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::DisableTransparentTextures

CForestRI_TemplateList
inline void CForestRI_Class::DisableTransparentTextures(void)
{
    m_tRenderStatePolicy.SetBlending(false);
    m_tRenderStatePolicy.SetAlphaTesting(false);
    m_tRenderStatePolicy.SetAlphaToCoverage(false);
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::InitFallbackTextures

CForestRI_TemplateList
inline void CForestRI_Class::InitFallbackTextures(void)
{
    for (int i = 0; i < TL_NUM_TEX_LAYERS; ++i)
    {
        TTexturePolicy& tTex = m_atFallbackTextures[i];

        switch (i)
        {
        case TL_DIFFUSE:
        case TL_SPECULAR:
            tTex.LoadColor(0xffffffff); 
            break;
        case TL_NORMAL:
        case TL_DETAIL_NORMAL:
            tTex.LoadColor(0x7f7fffff);
            break;
        case TL_DETAIL_DIFFUSE:
            tTex.LoadColor(0x00000000);
            break;
        }
    }
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::OnResetDevice

CForestRI_TemplateList
inline void CForestRI_Class::OnResetDevice(void)
{
    m_tShaderLoader.OnResetDevice( );
    m_atShadowBuffers[0].OnResetDevice( );
    m_atShadowBuffers[1].OnResetDevice( );
    m_atShadowBuffers[2].OnResetDevice( );
    m_atShadowBuffers[3].OnResetDevice( );
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::OnLostDevice

CForestRI_TemplateList
inline void CForestRI_Class::OnLostDevice(void)
{
    m_tShaderLoader.OnLostDevice( );
    m_atShadowBuffers[0].OnLostDevice( );
    m_atShadowBuffers[1].OnLostDevice( );
    m_atShadowBuffers[2].OnLostDevice( );
    m_atShadowBuffers[3].OnLostDevice( );
}


