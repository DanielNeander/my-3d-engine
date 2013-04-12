#ifndef __TERRAINSECTORMESH_H__
#define __TERRAINSECTORMESH_H__

#include "TerrainConfig.h"
#include "TerrainSectorManager.h"
#include "EngineCore/Math/BoundingBox.h"

class TerrainSector;
class TerrainVisiblityInfo;
struct SectorMeshRenderRange_t;

struct TerrainVertexLowRes_t
{
public:
	noVec3 vPos;

private:
};

class TerrainSectorMeshBuffer : public RefCounter 
{
public:
	TerrainSectorMeshBuffer()
	{
		m_pOwnerSector = NULL;
		m_iSamplerClampMask = 0;
		m_iMeshStreamMask = 0;
		m_iPixelShaderRegCount = 0;
		m_iLightmapSamplerIndex[0] = m_iLightmapSamplerIndex[1] = m_iLightmapSamplerIndex[2] = m_iLightmapSamplerIndex[3] = -1;
	}

	inline void ResetDetailTextures()
	{
		m_iSamplerClampMask = 0;
		m_iPixelShaderRegCount = 0;
	}

	inline void SetLightmapTexture(int iSampler, /*TextureObject *pTex,*/ int iSubIndex)
	{
		//SetChannelTexture(pTex,iSampler);
		//m_iLightmapSamplerIndex[iSubIndex] = iSampler;
	}
	
	VertexBufferID heightVB;
	

	TerrainSector* m_pOwnerSector;

	unsigned int m_iMeshStreamMask;
	unsigned int m_iSamplerClampMask; ///< each bit represents a sampler intex to clamp

	int m_iPixelShaderRegCount;
	signed char m_iLightmapSamplerIndex[4]; ///< for all 4 lightmap types this is the index where the lightmap is stored.
};

class TerrainSectorMeshPageInfo
{
public:
	TerrainSectorMeshPageInfo()
	{
		//    m_pRenderRange = m_pRenderRangeNoHoles = NULL;
		memset(m_fLODThreshold,0,sizeof(m_fLODThreshold));
		m_bHasHoles = m_bAllHoles = false;
		m_iGlobalIndex = -1;
	}

	void Init(TerrainSector* pOwner, int x, int y, bool bFirstTime);
	void PerformVisibility(TerrainVisibilityCollector &infoComp, const TerrainConfig &cfg);
	void UpdateHoleMask(TerrainSector* pOwner, int px, int py);


	int m_iGlobalIndex; ///< the global index in the whole terrain
	BoundingBox	m_AbsBoundingBox;
	float m_fLODThreshold[MAX_MESH_LOD-1];
	bool m_bHasHoles,m_bAllHoles;

	SectorMeshLODLevelInfo_t	m_HoleLODInfo[MAX_MESH_LOD];
};

typedef MSmartPtr<TerrainSectorMeshBuffer>	TerrainSectorMeshBufferPtr;

#endif