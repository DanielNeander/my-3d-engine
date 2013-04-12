#pragma once 

#include "TerrainSectorManager.h"
#include "TerrainConfig.h"

class VTerrain;
class VTerrainSector;
typedef MSmartPtr<VTerrain> VTerrainPtr;
class Renderer;

class Terrain 
{
public:
	/// \brief
	///   Enum that is used for the VTerrain::SetLODMetric function
	enum VTerrainLODMode_e
	{
		VLODMODE_NOISE_AND_DISTANCE, ///< LOD metric is based on distance and terrain noise (default)
		VLODMODE_DISTANCE_ONLY,  ///< LOD metric is based on distance only
	};

	Terrain();

	~Terrain();

	void CreateTerrain( Renderer* pRenderer, TerrainConfig *pConfig, const char *szAbsProjectDir, bool bSave=true );

	//virtual bool LoadFromFile(const char *szFolder);

	virtual bool SaveToFile(const char *szAbsFolder=NULL);

	bool SaveConfigFile(const char *szFolder);

	const TerrainConfig& Config() const {return m_Config;}

	virtual void FreeTerrain();

	virtual void OnNewTerrainCreated() {};

	void AddToSceneManager();

	void RemoveFromSceneManager();

	void EnsureSectorRangeLoaded(int iX1=0,int iY1=0, int iCountX=-1,int iCountY=-1);

	TerrainSector* CreateSectorInstance(int iIndexX, int iIndexY);

	/// \brief
	///   Sets the visibility bitmask filter for this terrain to render it into specific contexts
	inline void SetVisibleBitmask(unsigned int iMask) {m_SectorManager.m_iVisibleBitmask=iMask;}

	/// \brief
	///   Returns the current visibility bitmask. See SetVisibleBitmask
	inline unsigned int GetVisibleBitmask() const {return m_SectorManager.m_iVisibleBitmask;}

	void Render();

public:
	inline void SetUniqueID(__int64 iID) {m_iUniqueID=iID;}
	inline __int64 GetUniqueID() const {return m_iUniqueID;}


	bool m_bAddedToSceneManager;
	TerrainConfig m_Config;
	TerrainSectorManager	m_SectorManager;
	__int64	m_iUniqueID;
};