#include "stdafx.h"

#include "TerrainSector.h"

#include "Terrain.h"

void Terrain::CreateTerrain( Renderer* pRenderer, TerrainConfig *pConfig, const char *szAbsProjectDir, bool bSave/*=true */ )
{
	if (pConfig)
		m_Config = *pConfig;

	const char *szFilename = m_Config.m_sTerrainFolder.c_str();

	FreeTerrain();
	if (!m_Config.IsValid())
	{
		return;
	}

	if (bSave)
	{

	}

	m_SectorManager.InitTerrain(pRenderer);

	if (bSave)
		SaveToFile(szFilename);

	//OnNewTerrainCreated();
}

TerrainSector* Terrain::CreateSectorInstance(int iIndexX, int iIndexY)
{
  return new TerrainSector(&m_SectorManager,m_Config,iIndexX,iIndexY);
}

Terrain::~Terrain()
{
	FreeTerrain();


}

Terrain::Terrain() :m_SectorManager(m_Config)
{
	m_iUniqueID = 0;
	m_SectorManager.m_pTerrain = this;
	m_bAddedToSceneManager = false;

}

void Terrain::FreeTerrain()
{
	m_SectorManager.DeInitTerrain();
}

bool Terrain::SaveToFile( const char *szAbsFolder/*=NULL*/ )
{
	return true;
}

void Terrain::Render()
{
	// render decoration
}
