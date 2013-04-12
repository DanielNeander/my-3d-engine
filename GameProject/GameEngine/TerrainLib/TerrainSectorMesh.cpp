#include "stdafx.h"
#include "TerrainSectorMesh.h"
#include "TerrainSector.h"
#include "EngineCore/Math/Frustum.h"
#include "EngineCore/Math/BoundingBox.h"
#include "EngineCore/Math/IntersectionTests.h"


void TerrainSectorMeshPageInfo::Init( TerrainSector* pOwner, int x, int y, bool bFirstTime )
{
	const TerrainConfig &cfg = pOwner->m_Config;
	m_iGlobalIndex = pOwner->GetGlobalPageIndex(x,y);
	float fInvMeshCountX = 1.f / (float)cfg.m_iSectorMeshesPerSector[0];
	float fInvMeshCountY = 1.f / (float)cfg.m_iSectorMeshesPerSector[1];
	float fSizeX = cfg.m_vSectorSize.x*fInvMeshCountX;
	float fSizeY = cfg.m_vSectorSize.y*fInvMeshCountY;	
	m_AbsBoundingBox = pOwner->GetBoundingBox();
	noVec4 vOrigin = pOwner->GetSectorOrigin();	
	if (bFirstTime)
		m_AbsBoundingBox.m_vMin.z = m_AbsBoundingBox.m_vMax.z = vOrigin.z;
	m_AbsBoundingBox.m_vMin.x = vOrigin.x + (float)x*fSizeX;
	m_AbsBoundingBox.m_vMin.y = vOrigin.y + (float)y*fSizeY;
	m_AbsBoundingBox.m_vMax.x = m_AbsBoundingBox.m_vMin.x + fSizeX;
	m_AbsBoundingBox.m_vMax.y = m_AbsBoundingBox.m_vMin.y + fSizeY;
}

void TerrainSectorMeshPageInfo::PerformVisibility( TerrainVisibilityCollector &infoComp, const TerrainConfig &cfg )
{
#if 1
	TerrainVisibilityInfo &info(infoComp.m_VisibilityInfo);
	//info.m_pCamFrustum->CalculateAABB();
	if (m_bAllHoles || !IntersectionTest(m_AbsBoundingBox, *info.m_pCamFrustum))
		return;

	// calculate LOD value
	float fDistance = m_AbsBoundingBox.GetDistance(info.m_vCamLODPos);
	float fThreshold = fDistance * cfg.m_fSectorDistLODMult;
	const int iMaxLod = m_bHasHoles ? (cfg.m_iMaxMeshLODHoles) : (cfg.m_iMaxMeshLOD-1);

	if (cfg.m_bDistanceBasedLODOnly)
	{
		int iLOD = (int)(fThreshold+0.5f);
		infoComp.m_pPageLOD[m_iGlobalIndex] = __min(iLOD,iMaxLod);
	}
	else
	{

		infoComp.m_pPageLOD[m_iGlobalIndex] = 0; // >=0 makes it visible at the same time

		char &iCurrentLOD = infoComp.m_pPageLOD[m_iGlobalIndex];

		while (fThreshold>=m_fLODThreshold[iCurrentLOD] && iCurrentLOD<iMaxLod)
			iCurrentLOD++;
	}
#endif
}

void TerrainSectorMeshPageInfo::UpdateHoleMask( TerrainSector* pOwner, int px, int py )
{
	const TerrainConfig &cfg = pOwner->m_Config;
	// check for holes:
	m_bHasHoles = false;
	m_bAllHoles = true;
	bool bHoleArray[4096];
	int iIndex=0;
	for (int ty=0;ty<cfg.m_iTilesPerSectorMesh[1];ty++)
		for (int tx=0;tx<cfg.m_iTilesPerSectorMesh[0];tx++,iIndex++)
		{
			SectorTile *pTile = pOwner->GetTile(px*cfg.m_iTilesPerSectorMesh[0]+tx, py*cfg.m_iTilesPerSectorMesh[1]+ty);
			bHoleArray[iIndex] = pTile->IsHole();
			if (bHoleArray[iIndex])
				m_bHasHoles = true;
			else
				m_bAllHoles = false;
		}

		if (!m_bHasHoles || m_bAllHoles)
		{
			//m_spHoleIndexBuffer = NULL;
			return;
		}

		// rebuild index buffer
		int iIndexCount = 0;
		int iEstCount = 38 * (cfg.m_iHeightSamplesPerSector[0] / cfg.m_iSectorMeshesPerSector[0]) * (cfg.m_iHeightSamplesPerSector[1] / cfg.m_iSectorMeshesPerSector[1]);
		DynArray_cl<unsigned short> indices(iEstCount, 0);

		// highest resolution does not have increased edge res.
		int iCount = TerrainSectorManager::CreateLODIndexBuffer(cfg, 0,0,0,0,0,indices,iIndexCount,bHoleArray);
		for (int up=0;up<2;up++)
			for (int right=0;right<2;right++)
				for (int btm=0;btm<2;btm++)
					for (int left=0;left<2;left++)
						m_HoleLODInfo[0].m_EdgeInfo[up][right][btm][left].SetRange(iIndexCount/3,iCount/3);
		iIndexCount+=iCount;

		// higher LOD levels have edge resolution permutations
		for (int i=1;i<=cfg.m_iMaxMeshLODHoles;i++)
		{
			SectorMeshLODLevelInfo_t &info = m_HoleLODInfo[i];
			// edges
			for (int up=0;up<2;up++)
				for (int right=0;right<2;right++)
					for (int btm=0;btm<2;btm++)
						for (int left=0;left<2;left++)
						{
							SectorMeshRenderRange_t &range = info.m_EdgeInfo[up][right][btm][left];
							int iCount = TerrainSectorManager::CreateLODIndexBuffer(cfg, i,up,right,btm,left,indices,iIndexCount,bHoleArray);
							range.SetRange(iIndexCount/3,iCount/3);
							iIndexCount+=iCount;
						}
		}

		assert(iIndexCount>0);
}
