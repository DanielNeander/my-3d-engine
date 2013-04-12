#include "stdafx.h"
#include "mapgridheightmap.h"
#include "mapgrid.h"

CMapGridHeightMap::CMapGridHeightMap(MapGrid *mapGrid) :
	m_mapGrid(mapGrid)
{
}

CMapGridHeightMap::~CMapGridHeightMap(void)
{
	m_mapGrid = NULL;
}

unsigned int CMapGridHeightMap::Elevation(unsigned int x, unsigned int y)
{
	if (m_mapGrid)
		return m_mapGrid->getCost(x, y);
	return 0;
}

unsigned int CMapGridHeightMap::GetSize()
{
	if (m_mapGrid)
		return m_mapGrid->getGridSize();
	return 0;
}
