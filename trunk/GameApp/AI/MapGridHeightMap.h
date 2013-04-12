#pragma once
#include "heightmapgenerator.h"

class MapGrid;

class CMapGridHeightMap :
	public CGenericHeightMap
{
public:
	CMapGridHeightMap(MapGrid *mapGrid);
	virtual ~CMapGridHeightMap(void);

	virtual unsigned int Elevation(unsigned int x, unsigned int y);
	virtual unsigned int GetSize();
protected:
	MapGrid *m_mapGrid;
};

class CMapGridNormalizer : public CCostNormalizer
{
public:
	CMapGridNormalizer
		(
		float minCost, 
		float maxCost, 
		float maxCostNormalizedCost, 
		float blockedCostNormalizedCost
		) :
	m_minCost(minCost),
		m_maxCost(maxCost),
		m_maxCostNormalizedCost(maxCostNormalizedCost),
		m_blockedNormalizedCost(blockedCostNormalizedCost)
	{ }
	virtual ~CMapGridNormalizer(void) {}

	virtual float operator()(float cost)
	{
		float normalizedCost = m_blockedNormalizedCost;

		if (cost <= m_maxCost)
		{
			normalizedCost = (cost - m_minCost) /  (m_maxCost - m_minCost);
			normalizedCost *= m_maxCostNormalizedCost;
		}

		return normalizedCost;
	}

protected:
	float m_minCost;
	float m_maxCost;
	float m_maxCostNormalizedCost;
	float m_blockedNormalizedCost;
};


