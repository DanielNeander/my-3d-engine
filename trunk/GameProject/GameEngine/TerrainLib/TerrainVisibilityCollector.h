#pragma once 

#include "TerrainConfig.h"
#include "LargePosition.h"
#include "BitField.h"
#include "EngineCore/Math/BoundingBox.h"

class Terrain;
class TerrainSector;
struct SectorMeshRenderRange_t;
class Frustum;

class TerrainVisibilityInfo
{
public:

	// information
	LargePosition m_vCamPos;
	noVec3 m_vCamVisPos, m_vCamLODPos;
	//VisPlane_cl m_CameraPlane;	
	const Frustum *m_pCamFrustum;

	BoundingBox   m_CamBBox;
	LargeBoundingBox m_VisibleRangeBox; ///< same as m_CamBBox
	short m_iVisibleSectorRange[4]; ///< min/max index of visible sectors

	bool m_bVisibleInContext;
	unsigned int m_iContextFilterMask;

	// filled in by sectors:
	bool m_bAddDecoration;
	int m_iVisibleDecorationCount, m_iEstimatedDecorationCount;
	//DynArray_cl<VTerrainDecorationInstance *>m_VisibleDecoration;

	float m_fMaxViewDistance;
};

class TerrainVisibilityCollector
{
public:
	TerrainVisibilityCollector(Terrain* pTerrain);
	virtual ~TerrainVisibilityCollector();

	void BeginVisibility();
	void EndVisibility();

	void AddPendingSector(TerrainSector* pSector);
	void ProcessPendingSectors();

	bool IsSectorVisible(int iSectorIndex) const { }
	void WaitForVisibilityTasks();


	Terrain *m_pTerrain;
	LargePosition m_vCamPos;
	noVec4		m_vCamVisPos, m_vCamLODPos;
	// Plane
	// Frustum
	// BoundingBox
	LargeBoundingBox m_VisibleRangeBox; ///< same as m_CamBBox
	short m_iVisibleSectorRange[4]; ///< min/max index of visible sectors

	TerrainVisibilityInfo m_VisibilityInfo;
	

	int m_iOverallMeshPageCount;
	char *m_pPageLOD;
	SectorMeshRenderRange_t **m_pPageRenderRange;
	SectorMeshRenderRange_t **m_pPageRenderRangeNoHoles;
	bool m_bHasVisibleReplacementMeshes;

	TBitfield<16*16> m_SectorVisible;
	TBitfield<16*16> m_SectorReplacementVisible;

	int m_iPendingSectorCount;
	TerrainSector*	m_pPendingSector;
	
private:
	char m_PageLODBuffer[16*16];
	SectorMeshRenderRange_t* m_PageRangeBuffer[16*16];
	SectorMeshRenderRange_t* m_PageRangeNoHoleBuffer[16*16];
};


