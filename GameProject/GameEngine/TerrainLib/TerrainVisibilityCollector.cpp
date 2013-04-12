#include "stdafx.h"
#include "TerrainVisibilityCollector.h"
#include "Terrain.h"
#include "TerrainSector.h"

TerrainVisibilityCollector::TerrainVisibilityCollector( Terrain* pOwnerTerrain )
{
	m_pTerrain = pOwnerTerrain;
	m_iPendingSectorCount = 0;
	m_bHasVisibleReplacementMeshes = false;

	const TerrainConfig& cfg(pOwnerTerrain->m_Config);

	m_iOverallMeshPageCount = cfg.m_iOverallMeshPageCount[0]*cfg.m_iOverallMeshPageCount[1];
	m_pPageLOD = m_PageLODBuffer;
	memset(m_pPageLOD,-1,m_iOverallMeshPageCount); // flag invisible

	memset(m_pPageRenderRange,0,m_iOverallMeshPageCount * sizeof(SectorMeshRenderRange_t *));
	memset(m_pPageRenderRangeNoHoles,0,m_iOverallMeshPageCount * sizeof(SectorMeshRenderRange_t *));

	m_SectorVisible.AllocateBitfield(cfg.m_iSectorCount[0]*cfg.m_iSectorCount[1]);
	m_SectorReplacementVisible.AllocateBitfield(cfg.m_iSectorCount[0]*cfg.m_iSectorCount[1]);
}

TerrainVisibilityCollector::~TerrainVisibilityCollector()
{

}

void TerrainVisibilityCollector::BeginVisibility()
{
	memset(m_pPageLOD,-1,m_iOverallMeshPageCount); // flag invisible
	m_SectorVisible.Clear();
	m_SectorReplacementVisible.Clear();
	m_bHasVisibleReplacementMeshes = false;

	m_pTerrain->m_SectorManager.BeginVisibilityUpdate(*this);
}

void TerrainVisibilityCollector::EndVisibility()
{
	m_pTerrain->m_SectorManager.EndVisibilityUpdate(*this);
}

void TerrainVisibilityCollector::AddPendingSector( TerrainSector* pSector )
{

}

void TerrainVisibilityCollector::ProcessPendingSectors()
{
	if (!m_iPendingSectorCount)
		return;

	// we have to stop all tasks here, otherwise the IsLoaded flag of sectors assert&crash
	WaitForVisibilityTasks();

	const TerrainConfig &cfg(this->m_pTerrain->m_Config);
}

void TerrainVisibilityCollector::WaitForVisibilityTasks()
{
	


	
}