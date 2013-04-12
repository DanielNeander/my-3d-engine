// MapGrid.cpp: implementation of the MapGrid class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MapGrid.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MapGrid::MapGrid(const int gridsize)
{
	m_grid = NULL;
	m_grid = new GridCell*[gridsize];
	for(int i = 0 ; i < gridsize; i++)
	{
		m_grid[i] = NULL;
		m_grid[i] = new GridCell[gridsize];
	}

	m_gridsize = gridsize;

	m_startx = m_starty = 0;
	m_endx = m_endy = gridsize - 1;
}

MapGrid::~MapGrid()
{
	if(m_grid != NULL)
	{
		for(int i = 0; i < m_gridsize; i++)
		{
			if(m_grid[i] != NULL)
			{
				delete [] m_grid[i];
				m_grid[i] = NULL;
			}
		}

		delete [] m_grid;
		m_grid = NULL;
	}
}

int MapGrid::getCost (const int x, const int y) const
{ 
	return m_grid[x][y].getCost(); 
}

void MapGrid::setCost(const int x, const int y, const int cost)
{
	m_grid[x][y].setCost(cost);
}

MapGrid::GridCell::GridCell()
{
	m_cost = 1;
}

MapGrid::GridCell::GridCell(const int cost)
{
	m_cost = cost;
}

MapGrid::GridCell::GridCell(const GridCell& copy)
{
	m_cost = copy.m_cost;
}

MapGrid::GridCell &MapGrid::GridCell::operator=(const GridCell& rhs)
{
	m_cost = rhs.m_cost;

	return (*this);
}
