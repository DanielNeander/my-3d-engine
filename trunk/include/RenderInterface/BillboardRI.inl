///////////////////////////////////////////////////////////////////////  
//  BillboardRI.inl
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
//  CForestRI::RenderBillboards

CForestRI_TemplateList
inline bool CForestRI_Class::RenderBillboards(const SForestCullResults& sVisibleTrees, ERenderPassType eRenderPass, const CView& cView)
{
    bool bSuccess = true;

    if (sVisibleTrees.m_aVisibleCells.empty( ))
        return true;

    // state variables
    st_bool bFormatBound = false;

    // setup vertical billboard technique
    TShaderTechniquePolicy& tBillboardTechnique = PickTechnique(m_atBillboardTechniques, eRenderPass);
    if (tBillboardTechnique.Bind( ))
    {
        // setup render & shader state
        {
            m_tRenderStatePolicy.SetFaceCulling(CULLTYPE_NONE);

            // upload rotation parameters, common to all billboards
            float fAzimuth = cView.GetCameraAzimuth( );
            float fSine = sinf(-fAzimuth);

            bSuccess &= m_tCameraAzimuthTrig.Set4f(fSine, cosf(fAzimuth), -fSine, 0.0f);

            if (eRenderPass == RENDER_PASS_STANDARD)
            {
                bSuccess &= UploadBillboardTangents(fAzimuth);

                // the material shader matrix normally holds a geometry's material or the material multiplied
                // against the light materials; for the billboards, it will only contain the light material
                bSuccess &= m_tMaterial.SetMatrix(m_sRenderInfo.m_sLightMaterial.m_vAmbient);
            }

            bSuccess &= m_tMaterialExtras.Set4f(m_sRenderInfo.m_fGlobalLightScalar, 1.0f, 0.0f, 0.55f);

            // upload horizontal fade value
            bSuccess &= m_tHorzBillboardFade.Set1f(m_sRenderInfo.m_bHorizontalBillboards ? cView.GetHorzBillboardFadeValue( ) : 0.0f);
        }

        if (!m_bCanRenderQuads)
            bSuccess &= m_tBillboardIndexBuffer.BindIndexBuffer( );

        // track rendering statistics
        SRenderStats::SGeometryStats& sStats = m_sRenderStats.m_aGeometries[STATS_CATEGORY_BILLBOARDS][eRenderPass];

        assert(sVisibleTrees.m_pBaseTrees);
        for (st_int32 nBase = 0; nBase < st_int32(sVisibleTrees.m_pBaseTrees->size( )); ++nBase)
        {
            // gather info about the base tree's vertical billboards
            const CTreeRI_t* pBaseTree = (const CTreeRI_t*) sVisibleTrees.m_pBaseTrees->at(nBase);
            assert(pBaseTree);
            const SGeometry* pGeometry = pBaseTree->GetGeometry( );
            assert(pGeometry);
            const SVerticalBillboards& sVertBBs = pGeometry->m_sVertBBs;

            // upload shader constants specific to this base tree
            {
                // LOD range
                const SLodProfile& sLodProfile = pBaseTree->GetLodProfile( );
                bSuccess &= m_tLodProfile.Set4fv(reinterpret_cast<const float*>(&sLodProfile)); // cast for quicker upload

                // upload number of billboards
                bSuccess &= m_tNum360Images.Set1f(float(pGeometry->m_sVertBBs.m_nNumBillboards));

                // upload the vertical billboard texcoords
                st_int32 nNumBillboardsToUpload = sVertBBs.m_nNumBillboards;
                if (nNumBillboardsToUpload > m_sRenderInfo.m_nMaxBillboardImagesByBase)
                {
                    nNumBillboardsToUpload = m_sRenderInfo.m_nMaxBillboardImagesByBase;
                    CCore::SetError("CForestRI::RenderBillboards, tried to upload %d billboards for a base tree, but %d was the max set in CForestRI:SetRenderInfo(); truncating",
                        sVertBBs.m_nNumBillboards, nNumBillboardsToUpload);
                }
                bSuccess &= m_t360BillboardTexCoords.SetArray4f(sVertBBs.m_pTexCoords, nNumBillboardsToUpload);

                // upload the horizontal billboard texcoords if present
                if (pBaseTree->HasGeometryType(GEOMETRY_TYPE_HORIZONTAL_BILLBOARDS))
                    bSuccess &= m_tHorzBillboardTexCoords.Set4fv(pGeometry->m_sHorzBB.m_vTexCoordsShader);

                // upload wind data
                bSuccess &= UploadWindParams(pBaseTree->GetWind( ));
            
                // set the billboard texture
                st_int32 nNumTexturesNewlyBound = UploadMaterial(&pBaseTree->GetMaterials( )->at(sVertBBs.m_nMaterialIndex), true);
                if (nNumTexturesNewlyBound > 0)
                {
                    bSuccess &= tBillboardTechnique.CommitTextures( );
                    sStats.m_uiNumTextureBinds += st_uint32(nNumTexturesNewlyBound);
                }

                tBillboardTechnique.CommitConstants( );
            }

            // run through all of the cells, looking for instances of this base tree
            for (st_int32 nCell = 0; nCell < st_int32(sVisibleTrees.m_aVisibleCells.size( )); ++nCell)
            {
                const CTreeCell* pCell = sVisibleTrees.m_aVisibleCells[nCell];
                assert(pCell);
                const CTreeCell::SRenderInfo* pRenderInfo = pCell->GetBillboardRenderInfo(pBaseTree);
                if (pRenderInfo)
                {
                    TGeometryBufferPolicy* pVbo = (TGeometryBufferPolicy*) pCell->GetBillboardVbo( );
                    assert(pVbo);

                    if (!bFormatBound)
                    {
                        bSuccess &= pVbo->EnableFormat( );
                        bFormatBound = true;
                    }

                    if (pVbo->BindVertexBuffer( ))
                    {
                        ++sStats.m_uiNumVboBinds;

                        if (m_bCanRenderQuads)
                            bSuccess &= pVbo->RenderArrays(PRIMITIVE_QUADS, pRenderInfo->m_nBillboardOffset * 4, pRenderInfo->m_nNumBillboards * 4);
                        else
                        {
                            assert((pRenderInfo->m_nBillboardOffset + pRenderInfo->m_nNumBillboards) * 6 < st_int32(m_tBillboardIndexBuffer.NumIndices( )));
                            bSuccess &= m_tBillboardIndexBuffer.RenderIndexed(PRIMITIVE_TRIANGLES, pRenderInfo->m_nBillboardOffset * 6, pRenderInfo->m_nNumBillboards * 6, (pRenderInfo->m_nBillboardOffset + pRenderInfo->m_nNumBillboards) * 4);
                        }
                        ++sStats.m_uiNumDrawCalls;
                        sStats.m_uiNumTriangles += pRenderInfo->m_nNumBillboards * 2;

                        bSuccess &= pVbo->UnBindVertexBuffer( );
                    }
                    else
                        bSuccess = false;
                }
            }
        }

        if (bFormatBound)
            bSuccess &= TGeometryBufferPolicy::DisableFormat( );
        if (!m_bCanRenderQuads)
            bSuccess &= m_tBillboardIndexBuffer.UnBindIndexBuffer( );

        bSuccess &= tBillboardTechnique.UnBind( );
    }
    else
        bSuccess = false;

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::UploadBillboardTangents

CForestRI_TemplateList
inline st_bool CForestRI_Class::UploadBillboardTangents(float fCameraAzimuth) const
{
    fCameraAzimuth += c_fPi;

    if (CCoordSys::IsLeftHanded( ))
        fCameraAzimuth = c_fPi - fCameraAzimuth;

    Vec3 vBinormal = CCoordSys::UpAxis( );
    Vec3 vNormal = CCoordSys::ConvertFromStd(cosf(fCameraAzimuth), sinf(fCameraAzimuth), 0.0f);
    Vec3 vTangent = vBinormal.Cross(vNormal);

    // negate the tangent if RH/Yup or LH/Zup
    if ((CCoordSys::IsYAxisUp( ) && !CCoordSys::IsLeftHanded( )) ||
        (!CCoordSys::IsYAxisUp( ) && CCoordSys::IsLeftHanded( )))
        vTangent = -vTangent;

    float afBillboardTangents[12] = 
    {
        vNormal.x, vNormal.y, vNormal.z, 0.0f,
        vBinormal.x, vBinormal.y, vBinormal.z, 0.0f,
        vTangent.x, vTangent.y, vTangent.z, 0.0f
    };

    return m_tBillboardTangents.SetArray4f(afBillboardTangents, 3);
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::ResizeBillboardIndexBuffer

CForestRI_TemplateList
inline st_bool CForestRI_Class::ResizeBillboardIndexBuffer(st_int32 nMaxNumBillboards)
{
    st_bool bSuccess = false;

    // index buffer is only necessary if quads can't be rendered
    if (nMaxNumBillboards > 0 &&
        m_tBillboardIndexBuffer.NumIndices( ) < st_uint32(nMaxNumBillboards * 6))
    {
        CArray<st_uint32> aIndices(nMaxNumBillboards * 6);

        st_uint32* pBuffer = &aIndices[0];
        for (st_uint32 i = 0; i < st_uint32(nMaxNumBillboards); ++i)
        {
            st_uint32 uiOffset = i * 4;
            *pBuffer++ = uiOffset + 0;
            *pBuffer++ = uiOffset + 1;
            *pBuffer++ = uiOffset + 2;
            *pBuffer++ = uiOffset + 0;
            *pBuffer++ = uiOffset + 2;
            *pBuffer++ = uiOffset + 3;
        }
        assert(st_uint32(pBuffer - &aIndices[0]) == st_uint32(nMaxNumBillboards * 6));

        // copy buffer contents into index buffer
        bSuccess = m_tBillboardIndexBuffer.ReplaceIndices((const st_byte*) &aIndices[0], st_uint32(aIndices.size( )));
        assert(m_tBillboardIndexBuffer.NumIndices( ) == st_uint32(aIndices.size( )));
    }
    else
        bSuccess = true; // it's okay if it didn't need to resize

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::InitBillboardVboPool

CForestRI_TemplateList
inline st_bool CForestRI_Class::InitBillboardVboPool(void)
{
    st_bool bSuccess = false;

    if (m_nHintMaxNumBillboardVbos > 0)
    {
        m_aBillboardVboPool.resize(m_nHintMaxNumBillboardVbos);
        for (st_int32 i = 0; i < m_nHintMaxNumBillboardVbos; ++i)
        {
            m_aBillboardVboPool[i] = NewBillboardVbo( );
        }

        bSuccess = true;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::NextBillboardVbo

CForestRI_TemplateList
inline TGeometryBufferPolicy* CForestRI_Class::NextBillboardVbo(void)
{
    TGeometryBufferPolicy* pVbo = NULL;

    if (m_aBillboardVboPool.empty( ))
        pVbo = NewBillboardVbo( );
    else
    {
        pVbo = m_aBillboardVboPool.back( );
        m_aBillboardVboPool.pop_back( );
    }

    return pVbo;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::NewBillboardVbo

CForestRI_TemplateList
inline TGeometryBufferPolicy* CForestRI_Class::NewBillboardVbo(void)
{
    TGeometryBufferPolicy* pVbo = new ("Billboard VBO") TGeometryBufferPolicy;
    pVbo->SetVertexFormat(asBillboardVertexAttribs, &m_atBillboardTechniques[SHADER_TYPE_STANDARD], true);

    const st_uint32 c_uiVertsPerBillboard = 4;
    const st_uint32 c_uiNumVeritces = m_nHintMaxNumBillboardsPerCell * c_uiVertsPerBillboard * (m_sRenderInfo.m_bHorizontalBillboards ? 2 : 1);
    CArray<st_byte> aEmpty(pVbo->VertexSize( ) * c_uiNumVeritces);
    pVbo->ReplaceVertices(&aEmpty[0], c_uiNumVeritces);

    return pVbo;
}


///////////////////////////////////////////////////////////////////////
//  CForestRI::PopulateBillboardVbos

CForestRI_TemplateList
inline void CForestRI_Class::PopulateBillboardVbos(TTreeCellArrayNonConst& aCells)
{
    // init; won't resize unless requested size is larger than capacity()
    m_aBillboardPopulationBuffer.resize(aCells.size( ) * m_nHintMaxNumBillboardsPerCell * 4 * 2);

    for (st_int32 nCell = 0; nCell < st_int32(aCells.size( )); ++nCell)
    {
        // this one cell contains a series of base trees which each contain a series of instances
        CTreeCell* pCell = aCells[nCell];
        assert(pCell);
        TGeometryBufferPolicy* pVbo = (TGeometryBufferPolicy*) pCell->GetBillboardVbo( );
        assert(pVbo);

        // get a list of base trees for this cell
        const CCellInstances& cCellInstances = pCell->GetCellInstances( );

        // run through each base tree
        st_int32 nNumTotalBBs = 0;
        SBillboardVertex* pVertex = &m_aBillboardPopulationBuffer[0];
        CCellBaseTreeItr iBaseTree = cCellInstances.FirstBaseTree( );
        while (iBaseTree.TreePtr( ))
        {
            const CTree* pBase = iBaseTree.TreePtr( );
            assert(pBase);
            const st_bool c_bVerticalBillboardsPresent = pBase->HasGeometryType(GEOMETRY_TYPE_VERTICAL_BILLBOARDS);
            const st_bool c_bHorizontalBillboardsPresent = m_sRenderInfo.m_bHorizontalBillboards && pBase->HasGeometryType(GEOMETRY_TYPE_HORIZONTAL_BILLBOARDS);

            // resize population buffer if needed
            st_int32 nNumInstances = 0;
            const CInstance* pInstances = cCellInstances.GetInstances(iBaseTree, nNumInstances);
            const st_int32 c_nNextVertexCount = (nNumTotalBBs + nNumInstances) * 4 * 2;
            if (c_nNextVertexCount > st_int32(m_aBillboardPopulationBuffer.size( )))
            {
                m_aBillboardPopulationBuffer.resize(c_nNextVertexCount);
                pVertex = &m_aBillboardPopulationBuffer[nNumTotalBBs * 4];
            }

            if (c_bVerticalBillboardsPresent || c_bHorizontalBillboardsPresent)
            {
                // gather stats on this base tree's vertical billboard
                const SVerticalBillboards& sVertBBs = pBase->GetGeometry( )->m_sVertBBs;
                const float c_fHalfWidth = 0.5f * sVertBBs.m_fWidth;
                const float c_fNegHalfWidth = -c_fHalfWidth;
                const float c_fTopCoord = sVertBBs.m_fTopCoord;
                const float c_fBottomCoord = sVertBBs.m_fBottomCoord;

                // gather for horizontal billboard
                const SHorizontalBillboard& sHorzBB = pBase->GetGeometry( )->m_sHorzBB;

                // run through all of the base tree's instances
                const SBillboardVertex* c_pBaseVertex = pVertex;
                for (st_int32 nInstance = 0; nInstance < nNumInstances; ++nInstance)
                {
                    const CInstance& cInstance = pInstances[nInstance];

                    // parameters common to all vertices in this billboard
                    const Vec3& c_vPos = cInstance.GetPos( );
                    const st_float32 c_fScale = cInstance.GetScale( );
                    const st_float32 c_fRotationAngle = cInstance.GetRotationAngle( );

                    if (c_bVerticalBillboardsPresent)
                    {
                        // vertex 0
                        pVertex->m_vPosition = c_vPos;
                        pVertex->m_fCornerIndex = 0.0f;
                        pVertex->m_fWidth = c_fHalfWidth;
                        pVertex->m_fHeight = c_fTopCoord;
                        pVertex->m_fScale = c_fScale;
                        pVertex->m_fRotation = c_fRotationAngle;
                        ++pVertex;

                        // vertex 1
                        pVertex->m_vPosition = c_vPos;
                        pVertex->m_fCornerIndex = 1.0f;
                        pVertex->m_fWidth = c_fNegHalfWidth;
                        pVertex->m_fHeight = c_fTopCoord;
                        pVertex->m_fScale = c_fScale;
                        pVertex->m_fRotation = c_fRotationAngle;
                        ++pVertex;

                        // vertex 2
                        pVertex->m_vPosition = c_vPos;
                        pVertex->m_fCornerIndex = 2.0f;
                        pVertex->m_fWidth = c_fNegHalfWidth;
                        pVertex->m_fHeight = c_fBottomCoord;
                        pVertex->m_fScale = c_fScale;
                        pVertex->m_fRotation = c_fRotationAngle;
                        ++pVertex;

                        // vertex 3
                        pVertex->m_vPosition = c_vPos;
                        pVertex->m_fCornerIndex = 3.0f;
                        pVertex->m_fWidth = c_fHalfWidth;
                        pVertex->m_fHeight = c_fBottomCoord;
                        pVertex->m_fScale = c_fScale;
                        pVertex->m_fRotation = c_fRotationAngle;
                        ++pVertex;
                    }

                    if (c_bHorizontalBillboardsPresent)
                    {
                        const Vec3 c_vHorzPos(c_vPos + CCoordSys::UpAxis( ) * sHorzBB.m_avCoords[0].z * c_fScale);

                        // vertex 0
                        pVertex->m_vPosition = c_vHorzPos;
                        pVertex->m_fCornerIndex = 4.0f;
                        pVertex->m_fWidth = sHorzBB.m_avCoords[0].x;
                        pVertex->m_fHeight = sHorzBB.m_avCoords[0].y;
                        pVertex->m_fScale = c_fScale;
                        pVertex->m_fRotation = c_fRotationAngle;
                        ++pVertex;

                        // vertex 1
                        pVertex->m_vPosition = c_vHorzPos;
                        pVertex->m_fCornerIndex = 5.0f;
                        pVertex->m_fWidth = sHorzBB.m_avCoords[1].x;
                        pVertex->m_fHeight = sHorzBB.m_avCoords[1].y;
                        pVertex->m_fScale = c_fScale;
                        pVertex->m_fRotation = c_fRotationAngle;
                        ++pVertex;

                        // vertex 2
                        pVertex->m_vPosition = c_vHorzPos;
                        pVertex->m_fCornerIndex = 6.0f;
                        pVertex->m_fWidth = sHorzBB.m_avCoords[2].x;
                        pVertex->m_fHeight = sHorzBB.m_avCoords[2].y;
                        pVertex->m_fScale = c_fScale;
                        pVertex->m_fRotation = c_fRotationAngle;
                        ++pVertex;

                        // vertex 3
                        pVertex->m_vPosition = c_vHorzPos;
                        pVertex->m_fCornerIndex = 7.0f;
                        pVertex->m_fWidth = sHorzBB.m_avCoords[3].x;
                        pVertex->m_fHeight = sHorzBB.m_avCoords[3].y;
                        pVertex->m_fScale = c_fScale;
                        pVertex->m_fRotation = c_fRotationAngle;
                        ++pVertex;
                    }
                }

                const st_int32 c_nBillboardsAdded = st_int32(pVertex - c_pBaseVertex) / 4;
                nNumTotalBBs += c_nBillboardsAdded;

                // update render info for this base tree
                CTreeCell::SRenderInfo sBillboardRenderInfo;
                sBillboardRenderInfo.m_nNumBillboards = c_nBillboardsAdded;
                sBillboardRenderInfo.m_nBillboardOffset = nNumTotalBBs - c_nBillboardsAdded;
                pCell->SetRenderInfo(pBase, sBillboardRenderInfo);
            }

            ++iBaseTree;
        }

        // populate VBO
        st_uint32 uiNumVertices = st_uint32(pVertex - &m_aBillboardPopulationBuffer[0]);
        pVbo->ReplaceVertices(&m_aBillboardPopulationBuffer[0], uiNumVertices);
    }
}

