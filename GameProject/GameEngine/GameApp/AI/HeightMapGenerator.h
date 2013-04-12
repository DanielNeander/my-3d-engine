#pragma once 

class CGenericHeightMap
{
public:
	CGenericHeightMap() {}
	virtual ~CGenericHeightMap() {}

	virtual unsigned int Elevation(unsigned int x, unsigned int y) = 0;
	virtual unsigned int GetSize() = 0;
};

class CCostNormalizer
{
public:
	CCostNormalizer() {}
	virtual ~CCostNormalizer() {}

	virtual float operator()(float value) = 0;
};

class CNoOpNormalizer : public CCostNormalizer
{
	virtual float operator()(float value) { return value; };
};

class CHeightMapGenerator
{
public:
	CHeightMapGenerator	
		(
		CGenericHeightMap *heightMap,
		CCostNormalizer *normalizer
		);
	virtual ~CHeightMapGenerator(void);

	virtual float Elevation(float x, float y) = 0;

protected:
	float NormalizeCost(float cost);
	float GetNeighborCost(int x, int y, int size, float def);

	CGenericHeightMap *m_heightMap;
	CCostNormalizer	*m_normalizer;
};