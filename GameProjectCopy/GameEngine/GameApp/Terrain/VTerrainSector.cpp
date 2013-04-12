#include "stdafx.h"
#include "VTerrainSector.h"


void VTerrainSector::Render()
{
	TerrainVisibilityCollector &infoComp(*GetSectorManager()->m_pRenderCollector);

	TerrainSectorManager &sectorManager(*GetSectorManager());
	bool &bLastHasHoles = sectorManager.m_bLastMeshHasHoles;
	assert(sectorManager.m_pRenderCollector);

	// Visible 
	int iStartVertex = 0;
	TerrainSectorMeshPageInfo *pPage = m_pMeshPage;

	for (int iPageY=0;iPageY<m_Config.m_iSectorMeshesPerSector[1];iPageY++)
		for (int iPageX=0;iPageX<m_Config.m_iSectorMeshesPerSector[0];iPageX++,pPage++,iStartVertex+=m_Config.m_iVerticesPerMesh)	
		{
			if (infoComp.m_pPageLOD[pPage->m_iGlobalIndex]==(char)-1) // page not visible
				continue;

			if (pPage->m_bHasHoles)
				 bLastHasHoles = true;
			else if (bLastHasHoles)
				bLastHasHoles = false;
			
			assert(infoComp.m_pPageRenderRange[pPage->m_iGlobalIndex]);

			const SectorMeshRenderRange_t &range(*infoComp.m_pPageRenderRange[pPage->m_iGlobalIndex]);

			//range.m_iFirstPrim, range.m_iPrimCount,
			//	m_Config.m_iVerticesPerMesh
			//Render();
			RenderTerrainMesh(pPage,infoComp);
		}	
}

void VTerrainSector::PerformVisibility( TerrainVisibilityCollector& infoComp )
{
	
	 TerrainVisibilityInfo &info(infoComp.m_VisibilityInfo);

	 // 섹터 보이기 체크 - 나중에 구현 
	 const bool bSectorGeomVisible = IntersectionTest(m_BoundingBox, *info.m_pCamFrustum);
	 float fDistanceToThisSector = m_BoundingBox.GetDistanceXY(info.m_vCamLODPos);
	 const bool bUseReplacement = bSectorGeomVisible && m_Config.m_bSupportsReplacement && fDistanceToThisSector>=m_Config.m_fReplacementDistance;

	 //VMutex &mutex(GetSectorManager()->m_VisibilityMutex); // global visibility mutex for critical operations
	 //mutex.Lock();
	 	 
	 info.m_iVisibleSectorRange[0] = __min(info.m_iVisibleSectorRange[0],m_iIndexX);
	 info.m_iVisibleSectorRange[1] = __min(info.m_iVisibleSectorRange[1],m_iIndexY);
	 info.m_iVisibleSectorRange[2] = __max(info.m_iVisibleSectorRange[2],m_iIndexX);
	 info.m_iVisibleSectorRange[3] = __max(info.m_iVisibleSectorRange[3],m_iIndexY);
	 //mutex.Unlock();

	 // mark this sector as visible
	 if (bSectorGeomVisible)
	 {
		 // 섹터 보이기 체크 - 나중에 구현 
		 //infoComp.m_SectorVisible.SetBit();

		 // mesh pages + LOD
		 //if (bUseReplacement) // far away?
		 if (0)
		 {
			 //infoComp.m_SectorReplacementVisible.SetBit();
			 infoComp.m_bHasVisibleReplacementMeshes = true;
			 // since we don't have pages yet in replacement mode, we use page index directly
			 for (int iPageY=0;iPageY<m_Config.m_iSectorMeshesPerSector[1];iPageY++)
				 for (int iPageX=0;iPageX<m_Config.m_iSectorMeshesPerSector[0];iPageX++)
				 {
					 int iPageIndex = GetGlobalPageIndex(iPageX,iPageY);
					 infoComp.m_pPageLOD[iPageIndex] = PAGE_LOD_FORCEFIXED | m_Config.m_iReplacementLOD;
				 }
		 }
		 else 
		 {
			 TerrainSectorMeshPageInfo *pPage = m_pMeshPage;
			 for (int iPageY=0;iPageY<m_Config.m_iSectorMeshesPerSector[1];iPageY++)
				 for (int iPageX=0;iPageX<m_Config.m_iSectorMeshesPerSector[0];iPageX++,pPage++)
					 pPage->PerformVisibility(infoComp,m_Config);
		 }
	 }

	 // calculate visibility of each tile. 
	 //Since tiles might be larger than geometry at the edge, test for bSectorGeomVisible==false as well
	 // calculate visibility of each tile. Since tiles might be larger than geometry at the edge, test for bSectorGeomVisible==false as well
	 //fDistanceToThisSector *= VTerrainSectorManager::g_fDecorationDistanceInvScaling;
	 //if (fDistanceToThisSector < m_fMaxTileHandlingDistance)
	 {
		 //info.m_bAddDecoration = !m_bHasAdditionalDecoration; // in this mode, only render the custom array, see below

		 //VTerrainSectorDecorationVisibilityMask *pDecoVisMask = NULL;
		 //if (m_Decoration.m_iCount>0)
		 {
			 // the following operation must be thread-safe since it adds something to the list
			/* mutex.Lock();
			 pDecoVisMask = VTerrainSectorDecorationVisibilityMask::GetForCollector(m_pFirstDecoVisInfo,&infoComp,m_Decoration.m_iCount);
			 mutex.Unlock();
			 VASSERT(pDecoVisMask);*/
			 //SectorTile *pTile = m_pTile;
			 //for (int i=0;i<m_Config.m_iTilesPerSectorCount;i++,pTile++)
			 //{
				// //if (pTile->m_iReferencedDecorationCount==0 || !info.m_pCamFrustum->Overlaps(pTile->m_TileBBox))
				////	 continue;
				// pTile->PerformVisibility(infoComp);
			 //}
		 }
	 }

}

void VTerrainSector::RenderTerrainMesh( TerrainSectorMeshPageInfo* pPage, const TerrainVisibilityCollector &visInfoComp )
{
	const SectorMeshRenderRange_t &lod(*visInfoComp.m_pPageRenderRange[pPage->m_iGlobalIndex]);
	//VCompiledTechnique *pTech = m_spMesh->GetDefaultTechnique();
	//const int iPasses = pTech->GetShaderCount();
	//const int iVertexCount = m_Config.m_iVerticesPerMesh; //  m_spMesh->GetVertexCount()
	//for (int iPass=0;iPass<iPasses;iPass++)
	//{
	//	VCompiledShaderPass *pShader = pTech->GetShader(iPass);
	//	BindSectorShaderTextures(pShader,m_spMesh->m_iSamplerClampMask);

	//	// render
	//	Vision::RenderLoopHelper.RenderMeshes(pShader,
	//		VisMeshBuffer_cl::MB_PRIMTYPE_INDEXED_TRILIST,
	//		lod.m_iFirstPrim,lod.m_iPrimCount,iVertexCount);
	//}

}
