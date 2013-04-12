#include "stdafx.h"
#include "SectorTile.h"

SectorTile::SectorTile()
{
	m_iTileFlags = SECTORTILEFLAGS_NONE;
}

SectorTile::~SectorTile()
{

}

void SectorTile::Init( TerrainSector* pOwner, int iIndexX, int iIndexY )
{
	m_pOwner = pOwner;
	m_iIndexX = iIndexX;
	m_iIndexY = iIndexY;	
}

void SectorTile::PerformVisibility()
{

}
