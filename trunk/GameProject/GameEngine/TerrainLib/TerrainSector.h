#ifndef __TERRAINSECTOR_H__
#define __TERRAINSECTOR_H__

#include <EngineCore/Math/Vector.h>
#include "TerrainConfig.h"
#include "SectorTile.h"
#include "LargePosition.h"
#include "SectorVisibilityZone.h"
#include "TerrainSectorManager.h"
#include "TerrainSectorMesh.h"

class TerrainSector;
class TerrainVisiblityInfo;
typedef MSmartPtr<TerrainSector> TerrainSectorPtr;
class TerrainSectorManager;

#define WORLDX_2_TILE(_x) (int)(((_x)-m_vSectorOrigin.x)*m_Config.m_vWorld2Tile.x)
#define WORLDY_2_TILE(_y) (int)(((_y)-m_vSectorOrigin.y)*m_Config.m_vWorld2Tile.y)


class TerrainSector : public RefCounter 
{
public:
	TerrainSector(TerrainSectorManager *pManager, const TerrainConfig &config, int iIndexX, int iIndexY);
	~TerrainSector();
		
	TerrainSectorManager* GetSectorManager() const { return (TerrainSectorManager*)m_pManager; }


	inline int GetHeightmapSampleCount() const { return m_iSampleStrideX*(m_Config.m_iHeightSamplesPerSector[1]+2);}

	inline float *GetHeightmapValues() {if (!m_pHeight) LoadHeightmap(); return m_pHeight;}

	inline float *GetHeightmapValuesAt(int x, int y) 
	{
		float *fVal = GetHeightmapValues();
		//VASSERT(x>=0 && x<=m_Config.m_iHeightSamplesPerSector[0]+1); ///< also allow overlapping part
		//VASSERT(y>=0 && y<=m_Config.m_iHeightSamplesPerSector[1]+1);
		return &fVal[y*m_iSampleStrideX+x];
	}

	inline float GetHeightAt(int x, int y) const
	{
		//VASSERT(m_pHeight); ///< make sure GetHeightmapValues() has been called
		//VASSERT(x>=0 && x<=m_Config.m_iHeightSamplesPerSector[0]+1); ///< also allow overlapping part
		//VASSERT(y>=0 && y<=m_Config.m_iHeightSamplesPerSector[1]+1);
		return m_pHeight[y*m_iSampleStrideX+x];
	}

	float GetHeightAtRelPos(const noVec4 &vPos) const;

	virtual void PerformVisibility(TerrainVisibilityCollector& infoComp);


	virtual void OnPositionChanged(const noVec4 &vOldPos,const noVec4 &vNewPos);
	
	
	inline TerrainSectorMeshPageInfo *GetMeshPageInfo(int x, int y) const 
	{
		//VASSERT(x>=0 && y>=0 && x<m_Config.m_iSectorMeshesPerSector[0] && y<m_Config.m_iSectorMeshesPerSector[1]);
		if (m_pMeshPage)
			return &m_pMeshPage[y*m_Config.m_iSectorMeshesPerSector[0]+x];
		return NULL;
	}
	inline int GetGlobalPageIndex(int px, int py)
	{
		int xx = m_iIndexX*m_Config.m_iSectorMeshesPerSector[0]+px;
		int yy = m_iIndexY*m_Config.m_iSectorMeshesPerSector[1]+py;
		return yy*m_Config.m_iOverallMeshPageCount[0] + xx;
	}

	void UpdateHoleMask();

	inline int GetTileIndexX(const LargePosition &vPos) const 
	{
		return (int)(vPos.m_vSectorOfs.x*m_Config.m_vWorld2Tile.x) + m_Config.m_iTilesPerSector[0]*(vPos.m_iSectorX-m_iIndexX);
	}

	inline int GetTileIndexY(const LargePosition &vPos) const 
	{
		return (int)(vPos.m_vSectorOfs.y*m_Config.m_vWorld2Tile.y) + m_Config.m_iTilesPerSector[1]*(vPos.m_iSectorY-m_iIndexY);
	}

	inline void GetTileIndices_Clamped(const LargePosition &vPos, int &x, int &y) const 
	{
		x = GetTileIndexX(vPos);
		if (x<0) x=0; else if (x>=m_Config.m_iTilesPerSector[0]) x=m_Config.m_iTilesPerSector[0]-1;
		y = GetTileIndexY(vPos);
		if (y<0) y=0; else if (y>=m_Config.m_iTilesPerSector[1]) y=m_Config.m_iTilesPerSector[1]-1;
	}

	inline SectorTile*	GetTile(int x, int y) const
	{
		//VASSERT(x>=0 && x<m_Config.m_iTilesPerSector[0]);
		//VASSERT(y>=0 && y<m_Config.m_iTilesPerSector[1]);
		return &m_pTile[y*m_Config.m_iTilesPerSector[0]+x];
	}

	inline SectorTile *GetTileAtSampleIndices(int x,int y) const
	{
		x/=m_Config.m_iHeightSamplesPerTile[0];
		y/=m_Config.m_iHeightSamplesPerTile[1];
		// the height samples must be in range, but clamp at max edge because of extra overlapping values
		return GetTile(__min(x,m_Config.m_iTilesPerSector[0]-1),__min(y,m_Config.m_iTilesPerSector[1]-1));
	}

	inline void GetTexelIndices(const LargePosition &vPos, int &x, int &y, int iSizeX, int iSizeY)
	{
		x = (vPos.m_iSectorX - m_iIndexX) * iSizeX;
		y = (vPos.m_iSectorY - m_iIndexY) * iSizeY;
		x += (int)(vPos.m_vSectorOfs.x * m_Config.m_vInvSectorSize.x * (float)iSizeX);
		y += (int)(vPos.m_vSectorOfs.y * m_Config.m_vInvSectorSize.y * (float)iSizeY);
	}

	float* AllocateHeightMap();
	void LoadHeightmap();
	bool SaveHeightmap();
	bool LoadSectorInformation();
	bool SaveSectorInformation();
	void DisposePerSectorObjects();

	
	virtual bool SaveSector(bool bUseTemp=false);
	virtual void PrepareSector() 
	{
		m_bPrepared=true;
		//EnsureLoaded();
	}

	void UpdateMesh();
	
	//void ComputeSortingKey() {m_iSortingKey= 'TERR';}

	virtual int GetNumPrimitives();
	virtual int GetSupportedStreamMask();

	virtual bool SaveSectorFinal(bool bSaveSnapshot=false);
	void BindSectorShaderTextures();
	float ComputeMaxErrorForLOD(int iLod, int x1,int y1,int x2,int y2);
	void ComputeLODDistanceTable();

	void AssignVisibility(bool bStatus);
	static float Noise2D(int x, int y);

	// Resource Functions
	virtual bool	Reload();
	virtual bool	Unload();

	inline const noVec4& GetSectorOrigin() const {return m_vSectorOrigin;}

	const BoundingBox	GetBoundingBox() { return m_BoundingBox; }

	virtual void			Render();

	virtual void RenderTerrainMesh(TerrainSectorMeshPageInfo* pPage, const TerrainVisibilityCollector &visInfoComp) {}


public:
	const TerrainConfig&	m_Config;
	SectorVisibilityZonePtr m_spSectorZone;

	noVec4		m_vSectorOrigin;
	int m_iIndexX, m_iIndexY;     ///< sector index in the global terrain
	int m_iSampleStrideX;         ///< 2 additional overlapping height values (m_Config.m_iHeightSamplesPerSector+2)
	float *m_pHeight;             ///< (m_iSampleCount[0]+2)*(m_iSampleCount[1]+2)
	float m_fMinHeightValue;      ///< the minimum value in m_pHeight array
	float m_fMaxHeightValue;      ///< the maximum value in m_pHeight array
	float m_fMaxTileHandlingDistance; ///< maximum of m_fMaxDecorationFarClip over all tiles

	// sector mesh
	TerrainSectorMeshBufferPtr m_spMesh;
	TerrainSectorMeshPageInfo* m_pMeshPage;

	TerrainSectorManager*	m_pManager;

	SectorTile* m_pTile;
	
	bool m_bPrepared;  ///< loaded and updated
	bool m_bSectorFileLoaded;
	bool m_bHasAdditionalDecoration; ///< only relevant inside editor
	BoundingBox	m_BoundingBox;
};




#endif