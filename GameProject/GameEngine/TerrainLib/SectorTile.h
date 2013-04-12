#pragma once 

#include "TerrainConfig.h"

class TerrainSector;
class TerrainVisibilityInfo;

#define SECTORTILEFLAGS_NONE   0x00000000
#define SECTORTILEFLAGS_HOLE   (1 << 0)

class SectorTile
{
public:
	SectorTile();

	~SectorTile();

	void Init(TerrainSector* pOwner, int iIndexX, int iIndexY);

	void PerformVisibility();

	inline bool IsHole() const {return (m_iTileFlags & SECTORTILEFLAGS_HOLE)>0;}
public:
	TerrainSector* m_pOwner;	
	short m_iIndexX, m_iIndexY; ///< index in the sector
	int m_iTileFlags; ///< internal value 

};

