#include "stdafx.h"

#include "EngineCore/Math/Frustum.h"
#include "EngineCore/Math/BoundingBox.h"
#include "EngineCore/Math/IntersectionTests.h"
#include "TerrainSector.h"
#include "TerrainSectorManager.h"
#include "Terrain.h"

TerrainSector::TerrainSector( TerrainSectorManager *pManager, const TerrainConfig &config, int iIndexX, int iIndexY )
	:m_Config(config)
{	
	m_pManager = pManager;
	//m_vSectorOrigin = config.GetSectorOrigin(iIndexX,iIndexY);	
	m_iIndexX = iIndexX;
	m_iIndexY = iIndexY;
	char szBuffer[128];
	sprintf(szBuffer,"Sector_%02i_%02i",m_iIndexX,m_iIndexY);
	//SetFilename(szBuffer);
	m_fMinHeightValue = 0.f;
	m_fMaxHeightValue = 0.f;
	m_fMaxTileHandlingDistance = 0.f;
	m_pHeight = NULL;
	m_pTile = NULL;
	m_bPrepared = m_bSectorFileLoaded = false;
	m_pMeshPage = NULL;

	m_iSampleStrideX = config.m_iHeightSamplesPerSector[0]+2;
	//bbox.m_vMin.z = -10.f;
	//bbox.m_vMax.z = 10.f;

	//m_iPerSectorObjectCount = 0;
}

TerrainSector::~TerrainSector()
{

}

float TerrainSector::GetHeightAtRelPos( const noVec4 &vPos ) const
{
	((TerrainSector *)this)->GetHeightmapValues();
	float fPosX = vPos.x * m_Config.m_vWorld2Sample.x;
	float fPosY = vPos.y * m_Config.m_vWorld2Sample.y;
	int x = (int)fPosX;
	int y = (int)fPosY;
	float h00 = GetHeightAt(x,y);
	float h01 = GetHeightAt(x,y+1);
	float h10 = GetHeightAt(x+1,y);
	float h11 = GetHeightAt(x+1,y+1);

	float fx1 = fmodf(fPosX,1.f);
	float fy1 = fmodf(fPosY,1.f);
	float fx0 = 1.f-fx1;
	float fy0 = 1.f-fy1;
	return (h00*fx0+h10*fx1)*fy0 + (h01*fx0+h11*fx1)*fy1;
}

void TerrainSector::UpdateHoleMask()
{
	TerrainSectorMeshPageInfo *pPage = m_pMeshPage;
	for (int y=0;y<m_Config.m_iSectorMeshesPerSector[1];y++)
		for (int x=0;x<m_Config.m_iSectorMeshesPerSector[0];x++,pPage++)
			pPage->UpdateHoleMask(this,x,y);
}

float* TerrainSector::AllocateHeightMap()
{
	if (!m_pHeight)
	{
		// allocate height values
		const int iSampleCount = GetHeightmapSampleCount();
		m_pHeight = new float[iSampleCount];
	}
	return m_pHeight;
}

void TerrainSector::LoadHeightmap()
{
	if (m_pHeight==NULL)
		AllocateHeightMap();
	const int iSampleCount = GetHeightmapSampleCount();
	//VASSERT(iSampleCount>0 && m_pHeight);
	bool bLoaded = false;
	char szFilename[4096];

	// Load HeightMap

	memset(m_pHeight,0,iSampleCount*sizeof(float));

	// recalc min/max. Note that the extra border must not be considered
	m_fMaxHeightValue = m_fMinHeightValue = m_pHeight[0];
	for (int y=0;y<=m_Config.m_iHeightSamplesPerSector[1];y++)
	{
		float *pHeight = &m_pHeight[y*m_iSampleStrideX];
		for (int x=0;x<=m_Config.m_iHeightSamplesPerSector[0];x++)
		{
			m_fMaxHeightValue = __max(m_fMaxHeightValue, pHeight[x]);
			m_fMinHeightValue = __min(m_fMinHeightValue, pHeight[x]);
		}
	}

	// update bounding box:
	/*VisBoundingBox_cl bbox(false);
	GetDefaultSectorBoundingBox(bbox);
	SetSectorBoundingBox(bbox);*/

}

bool TerrainSector::SaveHeightmap()
{
	return true;
}
//
//bool TerrainSector::LoadSectorInformation()
//{
//
//}
//
//bool TerrainSector::SaveSectorInformation()
//{
//
//}

void TerrainSector::DisposePerSectorObjects()
{

}

bool TerrainSector::SaveSector( bool bUseTemp/*=false*/ )
{
	m_Config.m_bUseTempFolder = bUseTemp; // temp or final directory?
	bool bSuccess = true;
	//bSuccess &= SaveSectorInformation();
	bSuccess &= SaveHeightmap();

	//NB. If you add anything here, you also have to add it to VEditableTerrainSector::SaveSector
	return bSuccess;
}

struct TerrainVertex_t
{
	float fHeight;        // position z value
};

void TerrainSector::UpdateMesh()
{
	GetHeightmapValues();
	//VASSERT(m_spMesh && m_pHeight);
	const TerrainConfig& config = m_Config;
	const int iMeshSamplesX = m_Config.m_iHeightSamplesPerSector[0]/config.m_iSectorMeshesPerSector[0];
	const int iMeshSamplesY = m_Config.m_iHeightSamplesPerSector[1]/config.m_iSectorMeshesPerSector[1];
	int iReqVertCount = (iMeshSamplesX+1)*(iMeshSamplesY+1) * m_Config.m_iSectorMeshCount;

	int x,y;

	TerrainVertex_t* pVert = new TerrainVertex_t[iReqVertCount*sizeof(TerrainVertex_t)];
	memset(pVert, 0, iReqVertCount*sizeof(TerrainVertex_t));

	
	// generate vertices
	m_fMinHeightValue = m_pHeight[0];
	m_fMaxHeightValue = m_pHeight[0];
	const float fHOfs = m_Config.m_vTerrainPos.z;

	TerrainSectorMeshPageInfo *pPage = m_pMeshPage;
	for (int iPageY=0;iPageY<config.m_iSectorMeshesPerSector[1];iPageY++)
		for (int iPageX=0;iPageX<config.m_iSectorMeshesPerSector[0];iPageX++,pPage++)
		{
			//pPage->m_AbsBoundingBox.m_vMin.z = 10000000.f;
			//pPage->m_AbsBoundingBox.m_vMax.z = -10000000.f;
			for (y=0;y<=iMeshSamplesY;y++)
			{
				float *pHVal = &m_pHeight[m_iSampleStrideX*(y+iPageY*iMeshSamplesY) + iPageX*iMeshSamplesX];
				for (x=0;x<=iMeshSamplesX;x++, pVert++, pHVal++)
				{
					float h = *pHVal + fHOfs;
					pVert->fHeight = h;
					m_fMinHeightValue = __min(m_fMinHeightValue,*pHVal); // use h here?
					m_fMaxHeightValue = __max(m_fMaxHeightValue,*pHVal);
					//pPage->m_AbsBoundingBox.m_vMin.z = __min(pPage->m_AbsBoundingBox.m_vMin.z,h);
					//pPage->m_AbsBoundingBox.m_vMax.z = __max(pPage->m_AbsBoundingBox.m_vMax.z,h);
				}
			}
		}

	m_spMesh->heightVB = m_pManager->m_pRenderer->addVertexBuffer(iReqVertCount, STATIC, pVert);
	
	ComputeLODDistanceTable();
	
}

int TerrainSector::GetNumPrimitives()
{
	return GetSectorManager()->m_LODInfo[0].m_EdgeInfo[0][0][0][0].m_iPrimCount;
}

int TerrainSector::GetSupportedStreamMask()
{
	int iMask = 0;	
	return iMask;
}

bool TerrainSector::SaveSectorFinal( bool bSaveSnapshot/*=false*/ )
{
	return true;
}

void TerrainSector::BindSectorShaderTextures()
{

}

float TerrainSector::ComputeMaxErrorForLOD( int iLod, int x1,int y1,int x2,int y2 )
{	
	//VASSERT(iLod>0);
	const int iStep = 1<<iLod;
	const int iHalfStep = iStep/2;
	float fMax = 0.f;
	float d;
	for (int y=y1;y<y2;y+=iStep)
		for (int x=x1;x<x2;x+=iStep)
		{
			float fH = GetHeightAt(x,y);
			float fHdx = GetHeightAt(x+iStep,y);
			float fHdy = GetHeightAt(x,y+iStep);
			d = fabsf(GetHeightAt(x+iHalfStep,y)-((fH+fHdx)*0.5f)); // error on dx edge
			fMax = __max(fMax,d);
			d = fabsf(GetHeightAt(x,y+iHalfStep)-((fH+fHdy)*0.5f)); // error on dy edge
			fMax = __max(fMax,d);
			d = fabsf(GetHeightAt(x+iHalfStep,y+iHalfStep)-((fHdx+fHdy)*0.5f)); // error on diagonal edge
			fMax = __max(fMax,d);
		}
		return fMax;

}

void TerrainSector::ComputeLODDistanceTable()
{
	TerrainSectorMeshPageInfo *pPage = m_pMeshPage;
	for (int iPageY=0;iPageY<m_Config.m_iSectorMeshesPerSector[1];iPageY++)
		for (int iPageX=0;iPageX<m_Config.m_iSectorMeshesPerSector[0];iPageX++,pPage++)
			for (int i=1;i<m_Config.m_iMaxMeshLOD;i++)
			{
				int x1 = m_Config.m_iSectorMeshVertexCount[0]*iPageX;
				int y1 = m_Config.m_iSectorMeshVertexCount[1]*iPageY;
				float fError = ComputeMaxErrorForLOD(i,x1,y1,x1+m_Config.m_iSectorMeshVertexCount[0],y1+m_Config.m_iSectorMeshVertexCount[1]);
				pPage->m_fLODThreshold[i-1] = fError;
			}
}

//#define ADDREMOVENEIGHBOR(iIX,iIY) \
//	if ((iIX)>=0 && (iIY)>=0 && (iIX)<m_Config.m_iSectorCount[0] && (iIY)<m_Config.m_iSectorCount[1])\
//{\
//	VTerrainSector *pOther = pSectors->GetSector(iIX,iIY);\
//	bool bConnected = m_spSectorZone->IsConnected(pOther->m_spSectorZone);\
//	if (bStatus && !bConnected)\
//	m_spSectorZone->AddVisibilityZone(pOther->m_spSectorZone);\
//	else if (!bStatus && bConnected)\
//	m_spSectorZone->RemoveVisibilityZone(pOther->m_spSectorZone);\
//}

void TerrainSector::AssignVisibility( bool bStatus )
{
	assert(!bStatus || m_spSectorZone);

	// remove from old
	//IVisSceneManager_cl *pSM = Vision::GetSceneManager();
	//VASSERT(pSM);

	/*if (m_pVisibilityZone)
		m_pVisibilityZone->RemoveGeometryInstance(this);
	VASSERT(m_pVisibilityZone==NULL);*/
	if (!m_spSectorZone)
		return;

	// assign to new
	//if (!bStatus && m_spSectorZone->GetIndex()>=0)
	//	pSM->RemoveVisibilityZone(m_spSectorZone);
	//else if (bStatus)
	{
		//m_spSectorZone->AddStaticGeometryInstance(this);
		//if (m_spSectorZone->GetIndex()<0)
		//	pSM->AddVisibilityZone(m_spSectorZone);
	}

	/*TerrainSectorManager *pSectors = GetSectorManager();
	ADDREMOVENEIGHBOR(m_iIndexX-1,m_iIndexY);
	ADDREMOVENEIGHBOR(m_iIndexX+1,m_iIndexY);
	ADDREMOVENEIGHBOR(m_iIndexX,m_iIndexY-1);
	ADDREMOVENEIGHBOR(m_iIndexX,m_iIndexY+1);*/


}

float TerrainSector::Noise2D( int x, int y )
{
	double fx = fmod((double)x*0.001,(double)3.1415927f * 2.0f);
	double fy = fmod((double)y*0.001,(double)3.1415927f * 2.0f);
	return (float)(sin(fx*4.0)*cos(fy*5.0) + 0.5 * sin(fx*9.0)*cos(fy*11.0) + 0.25 * sin(fx*17.0)*cos(fy*19.0)  + 0.13 * sin(fx*31.0)*cos(fy*33.0));
}

bool TerrainSector::Reload()
{
	int x,y;

	// Delete 

	assert(m_pTile == NULL);
	m_pTile = new SectorTile[m_Config.m_iTilesPerSectorCount];
	SectorTile *pTile = m_pTile;

	//VisBoundingBox_cl bbox = GetBoundingBox();
	for (y=0;y<m_Config.m_iTilesPerSector[1];y++)
		for (x=0;x<m_Config.m_iTilesPerSector[0];x++,pTile++)
		{
			//GetDefaultTileBoundingBox(bbox,x,y);
			pTile->Init(this,x,y);
		}

	//VASSERT(m_pMeshPage==NULL);
	m_pMeshPage = new TerrainSectorMeshPageInfo[m_Config.m_iSectorMeshCount];
	TerrainSectorMeshPageInfo *pPage = m_pMeshPage;
	for (y=0;y<m_Config.m_iSectorMeshesPerSector[1];y++)
		for (x=0;x<m_Config.m_iSectorMeshesPerSector[0];x++,pPage++)
			pPage->Init(this,x,y,true);

	LoadHeightmap();

	if (!m_spMesh)
	{
		m_spMesh = new TerrainSectorMeshBuffer();
		m_spMesh->m_pOwnerSector = this;		
	}

	// load normal map texture

	UpdateMesh();
	m_bPrepared = true;

	// make sure everything is inflated properly
	//SetSectorBoundingBox(GetBoundingBox(),true);

	return true;
}

bool TerrainSector::Unload()
{
	return true;
}

void TerrainSector::OnPositionChanged( const noVec4 &vOldPos,const noVec4 &vNewPos )
{
	m_vSectorOrigin =  m_Config.GetSectorOrigin(m_iIndexX,m_iIndexY);

	noVec4 vDelta = vNewPos-vOldPos;			

	TerrainSectorMeshPageInfo *pPage = m_pMeshPage;
	if (pPage)
	{
		for (int iPageY=0;iPageY<m_Config.m_iSectorMeshesPerSector[1];iPageY++)
			for (int iPageX=0;iPageX<m_Config.m_iSectorMeshesPerSector[0];iPageX++,pPage++)
				pPage->Init(this,iPageX,iPageY,false);
	}
}

void TerrainSector::Render()
{
	TerrainVisibilityCollector &infoComp(*GetSectorManager()->m_pRenderCollector);

	TerrainSectorManager &sectorManager(*GetSectorManager());
	bool &bLastHasHoles = sectorManager.m_bLastMeshHasHoles;
	assert(sectorManager.m_pRenderCollector);

	// Visible 
	int iStartVertex = 0;
	TerrainSectorMeshPageInfo *pPage = m_pMeshPage;

	for (int iPageY=0;iPageY<m_Config.m_iSectorMeshesPerSector[1];iPageY++)
		for (int iPageX=0;iPageX<m_Config.m_iSectorMeshesPerSector[0];iPageX++,pPage++,iStartVertex+=m_Config.m_iVerticesPerMesh)	
		{
			//if (infoComp.m_pPageLOD[pPage->m_iGlobalIndex]==(char)-1) // page not visible
			//	continue;
			const SectorMeshRenderRange_t &range(*infoComp.m_pPageRenderRange[pPage->m_iGlobalIndex]);

		}	
}

void TerrainSector::PerformVisibility( TerrainVisibilityCollector& infoComp )
{
}


