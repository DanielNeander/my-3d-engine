#pragma once 
#ifndef __TERRAIN_CONFIG_H__
#define __TERRAIN_CONFIG_H__



#define MAX_MESH_LOD 8

#define TERRAIN_COLLISIONMESHES_SECTORLOCALSPACE
#undef TERRAIN_COLLISIONMESHES_SECTORLOCALSPACE

#define TERRAIN_SECTORMESH_IDEAL_SIZE	128

class LargePosition;
class LargeBoundingBox;



class TerrainConfig 
{
public:
	
	inline TerrainConfig() { m_bFinalized = false; InitDefaults(); }

	std::string m_sTerrainFolder;
	std::string m_sAbsTerrainDir;
	mutable bool m_bUseTempFolder;
	bool		m_bSortTextureChannels;
	bool		m_bDistanceBasedLODOnly;

	int			m_iSectorCount[2];
	noVec2		m_vSectorSize;
	noVec3		m_vTerrainPos;

	int			m_iHeightSamplesPerSector[2];
	int			m_iTilesPerSector[2];
	int			m_iDensityMapSamplesPerSector[2];
	int			m_iDefaultWeightmapResolution[2];
	
	noVec2		m_vBaseTextureTiling;
	std::string	m_sBaseTextureFile;

	// Replacement meshes
	int m_iReplacementLOD;            ///< future use
	float m_fReplacementDistance;     ///< future use

	float		m_fPurgeInterval;
	float		m_fVisibilityHeightOverTerrain;


	inline void InitDefaults()
	{
		m_bSortTextureChannels = false;
		m_bUseTempFolder = false;
		m_sTerrainFolder = "TerrainScene";
		m_iSectorCount[0] = m_iSectorCount[1] = 16;
		m_fViewDistance = 100000.f;
		m_iHeightSamplesPerSector[0] = m_iHeightSamplesPerSector[1] = 128; ///< ideal size
		m_vSectorSize.Set(10000.f,10000.f);
		m_iTilesPerSector[0] = m_iTilesPerSector[1] = 16;
		m_iDensityMapSamplesPerSector[0] = m_iDensityMapSamplesPerSector[1] = 32;
		m_vTerrainPos.Set(0.f,0.f,0.f);
		m_fPurgeInterval = 100.f;
		m_iDefaultWeightmapResolution[0] = m_iDefaultWeightmapResolution[1] = 512;
		m_fVisibilityHeightOverTerrain = 2000.f;		
		m_iReplacementLOD = -1;
		m_fReplacementDistance = -1.f;
		m_bDistanceBasedLODOnly = false;

		Finalize();
	}

	 void Finalize();

	inline bool IsValid() const 
	{
		if (m_iSectorCount[0]<=0 || m_iSectorCount[1]<=0) return false;
		if (m_iSectorCount[0]>1024 || m_iSectorCount[1]>1024) return false; ///< sanity check
		if (m_iHeightSamplesPerSector[0]<=4 || m_iHeightSamplesPerSector[1]<=4) return false;
		if (m_iTilesPerSector[0]<=0 || m_iTilesPerSector[1]<=0) return false;
		if (m_iTilesPerSector[0]>m_iHeightSamplesPerSector[0] || m_iTilesPerSector[1]>m_iHeightSamplesPerSector[1]) return false;
		if (m_fViewDistance<1.f) return false;
		if (m_fVisibilityHeightOverTerrain<0.f) return false;
		return true;		
	}
	
	inline int GetSectorIndexAtSamplePosX(int iHMSampleX) const {return iHMSampleX/m_iHeightSamplesPerSector[0];}
		
	inline int GetSectorIndexAtSamplePosY(int iHMSampleY) const {return iHMSampleY/m_iHeightSamplesPerSector[1];}

	inline int GetSectorSampleOfsX(int iHMSampleX) const {return iHMSampleX%m_iHeightSamplesPerSector[0];}

	inline int GetSectorSampleOfsY(int iHMSampleY) const {return iHMSampleY%m_iHeightSamplesPerSector[1];}

	inline int GetSectorIndexAtTilePosX(int iTileX) const {return iTileX/m_iTilesPerSector[0];}

	inline int GetSectorIndexAtTilePosY(int iTileY) const {return iTileY/m_iTilesPerSector[1];}

	inline void ClampSectorIndices(int &x,int &y) const
	{
		if (x<0) x=0; else if (x>=m_iSectorCount[0]) x=m_iSectorCount[0]-1;
		if (y<0) y=0; else if (y>=m_iSectorCount[1]) y=m_iSectorCount[1]-1;
	}

	inline void ClampSectorRectangle(int &x1,int &y1,int &x2,int &y2) const
	{
		if (x1<0) x1=0;
		if (y1<0) y1=0;
		if (x2>=m_iSectorCount[0]) x2=m_iSectorCount[0]-1;
		if (y2>=m_iSectorCount[1]) y2=m_iSectorCount[1]-1;
	}

	inline void GetSectorIndices_Clamped(int iHMSampleX, int iHMSampleY, int &x,int &y) const
	{
		x = GetSectorIndexAtSamplePosX(iHMSampleX);
		if (x<0) x=0; else if (x>=m_iSectorCount[0]) x=m_iSectorCount[0]-1;
		y = GetSectorIndexAtSamplePosY(iHMSampleY);
		if (y<0) y=0; else if (y>=m_iSectorCount[1]) y=m_iSectorCount[1]-1;
	}

	inline const noVec4 GetSectorOrigin(int iIndexX, int iIndexY) const {return noVec4(m_vSectorSize.x*(float)iIndexX+m_vTerrainPos.x,m_vSectorSize.y*(float)iIndexY+m_vTerrainPos.y,m_vTerrainPos.z, 1.0f);}



public:
	float m_fViewDistance;            ///< maximum view distance, updated per frame
	int m_iSectorMeshesPerSector[2];  ///< samples per sector / 128 (ideal mesh size)
	int m_iSectorMeshVertexCount[2];  ///< 128 (ideal mesh size)
	int m_iSectorMeshCount,m_iVerticesPerMesh;
	int m_iHeightSamplesPerTile[2];   ///< number of height map samples per tile, m_iHeightSamplesPerSector/m_iTilesPerSector
	int m_iOverallHeightSampleCount[2]; ///< overall number of heightsamples in both directions
	int m_iOverallMeshPageCount[2];     ///< overall number of pages in both directions
	int m_iSamplesPerReplacementMesh[2]; ///< number of heightmap samples per low res replacement mesh
	int m_iTilesPerSectorMesh[2];       ///< number of tile subdivisions for each sector mesh
	int m_iTilesPerSectorCount;       ///< m_iTilesPerSector[0] * m_iTilesPerSector[1]
	noVec2 m_vInvSectorSize;       ///< inverse of terrain sector size
	noVec2 m_vWorld2Sample;        ///< scales a world position to height sample index inside a sector
	noVec2 m_vTileSize;            ///< size of a single tile (sectorsize/tiles per sector)
	noVec2 m_vWorld2Tile;          ///< scales a world position to tile index inside a sector (1.0/m_vTileSize)
	noVec2 m_vWorld2DensityMap;    ///< scales a world position to sample index of the density map
	noVec2 m_vSampleSpacing;       ///< x-y spacing between two height map samples
	noVec2 m_vDensitySampleSpacing;// x-y spacing between two samples on the density map
	noVec2 m_vNormalGradientSpacing; ///< recommended x-y spacing for making the cross product
	int m_iMaxMeshLOD;                ///< the usable max (<= MAX_MESH_LOD)
	int m_iMaxMeshLODHoles;           ///< the maximum LOD for sectors that have holes
	float m_fSectorDistLODMult;       ///< multiplier that maps sector distance to LOD level value
	bool m_bFinalized;                ///< future use
	bool m_bSupportsReplacement;      ///< future use







};

#endif