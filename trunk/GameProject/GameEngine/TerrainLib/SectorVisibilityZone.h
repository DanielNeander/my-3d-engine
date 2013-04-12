#pragma once 
#include "TerrainConfig.h"

class Terrain;
class TerrainSector;
class TerrainVisibilityInfo;
class TerrainVisibilityCollector;

class SectorVisibilityZone : public RefCounter
{
public:
	SectorVisibilityZone(TerrainSector* pSector);

	SectorVisibilityZone();


	void OnVisible(TerrainVisibilityCollector* pCollector);


	
	//void OnAddedToSceneManager();

	//void OnRemovedFromSceneManager();

	//void OnFinishVisibilityBuilt();

	Terrain *m_pTerrain;       ///< owner terrain
	TerrainSector *m_pSector;  ///< owner sector
};



typedef MSmartPtr<SectorVisibilityZone> SectorVisibilityZonePtr;

