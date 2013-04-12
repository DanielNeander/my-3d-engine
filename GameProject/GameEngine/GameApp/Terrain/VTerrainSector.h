#pragma once
#include "TerrainLib/TerrainSector.h"

class VTerrainSector : public TerrainSector
{
public:

	virtual void PerformVisibility(TerrainVisibilityCollector& infoComp);
	virtual void Render();

	virtual void RenderTerrainMesh(TerrainSectorMeshPageInfo* pPage, const TerrainVisibilityCollector &visInfoComp);
};