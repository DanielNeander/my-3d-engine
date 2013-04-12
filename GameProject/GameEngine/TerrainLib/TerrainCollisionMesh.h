#pragma once 

#include "TerrainConfig.h"

class TerrainSector;

class TerrainCollisionMeshManager
{
public:
	
};

class SectorCollisionMeshes 
{
public:
	SectorCollisionMeshes(TerrainSector* pOwnerSector);
	virtual ~SectorCollisionMeshes();


protected:

	short m_iCountX,m_iCountY;        ///< Same as number of tiles in a sector.
};