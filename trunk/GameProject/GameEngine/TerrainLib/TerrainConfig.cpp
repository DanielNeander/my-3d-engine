#include "stdafx.h"
#include "TerrainConfig.h"

void TerrainConfig::Finalize()
{
	m_iSectorMeshesPerSector[0] = __max(m_iHeightSamplesPerSector[0]/TERRAIN_SECTORMESH_IDEAL_SIZE,1);
	m_iSectorMeshesPerSector[1] = __max(m_iHeightSamplesPerSector[1]/TERRAIN_SECTORMESH_IDEAL_SIZE,1);
	m_iSectorMeshCount = m_iSectorMeshesPerSector[0]*m_iSectorMeshesPerSector[1];
	m_iSectorMeshVertexCount[0] = m_iHeightSamplesPerSector[0]/m_iSectorMeshesPerSector[0];
	m_iSectorMeshVertexCount[1] = m_iHeightSamplesPerSector[1]/m_iSectorMeshesPerSector[1];
	m_iVerticesPerMesh = (m_iSectorMeshVertexCount[0]+1)*(m_iSectorMeshVertexCount[0]+1);
	m_iHeightSamplesPerTile[0] = m_iHeightSamplesPerSector[0]/m_iTilesPerSector[0];
	m_iHeightSamplesPerTile[1] = m_iHeightSamplesPerSector[1]/m_iTilesPerSector[1];
	m_iOverallHeightSampleCount[0] = m_iHeightSamplesPerSector[0]*m_iSectorCount[0];
	m_iOverallHeightSampleCount[1] = m_iHeightSamplesPerSector[1]*m_iSectorCount[1];
	m_iTilesPerSectorMesh[0] = m_iTilesPerSector[0] / m_iSectorMeshesPerSector[0];
	m_iTilesPerSectorMesh[1] = m_iTilesPerSector[1] / m_iSectorMeshesPerSector[1];
	m_iOverallMeshPageCount[0] = m_iSectorCount[0]*m_iSectorMeshesPerSector[0];
	m_iOverallMeshPageCount[1] = m_iSectorCount[1]*m_iSectorMeshesPerSector[1];
	m_iTilesPerSectorCount = m_iTilesPerSector[0] * m_iTilesPerSector[1];

	// additional initialisation
	m_vInvSectorSize.Set(1.f/m_vSectorSize.x,1.f/m_vSectorSize.y);

	m_vWorld2Sample.x = (float)m_iHeightSamplesPerSector[0] / m_vSectorSize.x;
	m_vWorld2Sample.y = (float)m_iHeightSamplesPerSector[1] / m_vSectorSize.y;
	m_vSampleSpacing.x = 1.f/m_vWorld2Sample.x;
	m_vSampleSpacing.y = 1.f/m_vWorld2Sample.y; 
	m_vTileSize.x = m_vSectorSize.x/(float)m_iTilesPerSector[0];
	m_vTileSize.y = m_vSectorSize.y/(float)m_iTilesPerSector[1];
	m_vWorld2Tile.x = (float)m_iTilesPerSector[0]/m_vSectorSize.x;
	m_vWorld2Tile.y = (float)m_iTilesPerSector[1]/m_vSectorSize.y;

	m_vWorld2DensityMap.x = (float)m_iDensityMapSamplesPerSector[0] / m_vSectorSize.x;
	m_vWorld2DensityMap.y = (float)m_iDensityMapSamplesPerSector[1] / m_vSectorSize.y;
	m_vDensitySampleSpacing.x = 1.f/m_vWorld2DensityMap.x;
	m_vDensitySampleSpacing.y = 1.f/m_vWorld2DensityMap.y;
	m_vNormalGradientSpacing = m_vSampleSpacing*1.f;

	m_iMaxMeshLOD = m_iMaxMeshLODHoles = 0;
	int iEdge = __min(m_iSectorMeshVertexCount[0],m_iSectorMeshVertexCount[1]);
	int iTile = __min(m_iHeightSamplesPerTile[0],m_iHeightSamplesPerTile[1]);

	while (iEdge>2 && m_iMaxMeshLOD<MAX_MESH_LOD) // smallest is a 4x4 tile
	{
		m_iMaxMeshLOD++;
		if ((1<<m_iMaxMeshLOD)<=iTile)
			m_iMaxMeshLODHoles = m_iMaxMeshLOD;
		iEdge/=2;
	}

	//m_iReplacementLOD = __min(m_iReplacementLOD,m_iMaxMeshLODHoles);
	//m_iSamplesPerReplacementMesh[0] = m_iSamplesPerReplacementMesh[1] = 0;

	//if (m_iReplacementLOD>0)
	//{
	//	m_iSamplesPerReplacementMesh[0] = m_iHeightSamplesPerSector[0] >> m_iReplacementLOD;
	//	m_iSamplesPerReplacementMesh[1] = m_iHeightSamplesPerSector[1] >> m_iReplacementLOD;
	//	while (m_iSamplesPerReplacementMesh[0]>128 || m_iSamplesPerReplacementMesh[1]>128)
	//	{
	//		m_iReplacementLOD++;
	//		m_iSamplesPerReplacementMesh[0] /= 2;
	//		m_iSamplesPerReplacementMesh[1] /= 2;
	//	}
	//}

	//m_bSupportsReplacement = m_iReplacementLOD>0 && !Vision::Editor.IsInEditor();

	//// calc optimal replacement mesh distance
	//if (m_bSupportsReplacement && m_fReplacementDistance<0.f)
	//	m_fReplacementDistance = __max(m_vSectorSize.x,m_vSectorSize.y) * ((float)m_iReplacementLOD+0.1f);

	m_fSectorDistLODMult = 0.f;
	m_bFinalized = true;
}
