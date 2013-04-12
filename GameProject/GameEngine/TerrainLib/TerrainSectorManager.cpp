#include "stdafx.h"
#include "TerrainSectorManager.h"
#include "TerrainSector.h"
#include "TerrainSectorMesh.h"
#include "Terrain.h"

void TerrainSectorManager::SetAllowPurging(bool bStatus)
{
	if (bStatus)
	{

	}
	else 
	{

	}
}

void TerrainSectorManager::InitTerrain(Renderer* pRenderer)
{
	m_pRenderer = pRenderer;
	m_Config.IsValid();

	m_iSectorCount = m_Config.m_iSectorCount[0] * m_Config.m_iSectorCount[1];
	m_pSector = new TerrainSectorPtr[m_iSectorCount];

	int iIndex = 0;
	int x,y;
	for (y=0; y < m_Config.m_iSectorCount[1]; ++y)
		for (x=0; x < m_Config.m_iSectorCount[0]; x++, iIndex++)
		{
			m_pSector[iIndex] = m_pTerrain->CreateSectorInstance(x,y);
			m_pSector[iIndex]->Reload();
		}

	m_bGlobalsCBDirty = true;

	while (m_iPickingMeshDetail>0 
		&& (m_Config.m_iHeightSamplesPerSector[0]<(m_Config.m_iTilesPerSector[0]<<m_iPickingMeshDetail))
		|| m_Config.m_iHeightSamplesPerSector[1]<(m_Config.m_iTilesPerSector[1]<<m_iPickingMeshDetail))
		m_iPickingMeshDetail--;	
}

TerrainSectorManager::TerrainSectorManager(TerrainConfig& config)
	:m_Config(config)
	
{
	m_iSectorCount = 0;
	m_pSector = NULL;
	m_iVisibleBitmask = 0xffffffff;

	m_fLODScaling = 1.0f;
	m_bLastMeshHasHoles = false;
	m_pRenderCollector = NULL;
	m_iPickingMeshDetail = 2; // use only every 4th vertex for picking

	memset(m_fSphereConstraintRadius,0,sizeof(m_fSphereConstraintRadius));
	m_bSphereConstraintConstantsDirty = true;

	SetAllowPurging(true);

	//// Init fallback lightgrid colors
	//VVertex3f lightDir(1.f,1.f,1.f);
	//lightDir.Normalize();
	//VLightGrid_cl::GetDirectionalLightColors(m_DecorationLightgridColors,lightDir,VVertex3f(1.f,0.8f,0.8f),VVertex3f(0.2f,0.3f,0.4f));
	//m_bNeedsLightgridUpdate = true;

	//m_bShaderUsesDirectionalLight = false;
	//m_vDirectionalLightColor.SetXYZ(1.f,1.f,1.f);
	//m_vAmbientColor.SetXYZ(0.2f,0.21f,0.24f);
	//m_vLightDirection.SetXYZ(1.f,1.f,-1.f);
	//m_vLightDirection.Normalize();

}

TerrainSectorManager::~TerrainSectorManager()
{

}

void TerrainSectorManager::DeInitTerrain()
{

}



void TerrainSectorManager::EnsureSharedMeshesCreated()
{
	// prepare index buffers
	int iIndexCount = 0;
	int iEstCount = 38 * (m_Config.m_iHeightSamplesPerSector[0] / m_Config.m_iSectorMeshesPerSector[0]) 
		* (m_Config.m_iHeightSamplesPerSector[1] / m_Config.m_iSectorMeshesPerSector[1]);
	DynArray_cl<unsigned short> indices(iEstCount,0);	
	int x, y;

	// highest resolution does not have increased edge res.
	int iCount = CreateLODIndexBuffer(m_Config, 0,0,0,0,0, indices,iIndexCount,NULL);	
	for (int up=0;up<2;up++)
		for (int right=0;right<2;right++)
			for (int btm=0;btm<2;btm++)
				for (int left=0;left<2;left++)
					m_LODInfo[0].m_EdgeInfo[up][right][btm][left].SetRange(iIndexCount/3,iCount/3);
	iIndexCount+=iCount;

	// higher LOD levels have edge resolution permutations
	for (int i=1;i<m_Config.m_iMaxMeshLOD;i++)
	{
		SectorMeshLODLevelInfo_t &info = m_LODInfo[i];
		// edges
		for (int up=0;up<2;up++)
			for (int right=0;right<2;right++)
				for (int btm=0;btm<2;btm++)
					for (int left=0;left<2;left++)
					{
						SectorMeshRenderRange_t &range = info.m_EdgeInfo[up][right][btm][left];
						int iCount = CreateLODIndexBuffer(m_Config, i,up,right,btm,left,indices,iIndexCount,NULL);
						range.SetRange(iIndexCount/3,iCount/3);
						iIndexCount+=iCount;
					}
	}

	// build shared vertex & index buffer (shared for all meshes)
	const int iSamplesX = m_Config.m_iHeightSamplesPerSector[0] / m_Config.m_iSectorMeshesPerSector[0];
	const int iSamplesY = m_Config.m_iHeightSamplesPerSector[1] / m_Config.m_iSectorMeshesPerSector[1];
	const int iVertCount = (iSamplesX+1)*(iSamplesY+1);

	char workdir[MAX_PATH], fullPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, workdir);
	_snprintf_s(fullPath, MAX_PATH, "%s/Data/Shaders/Terrain.shd",workdir);	
	terrainShader = m_pRenderer->addShader("Data/Shaders/Terrain.shd");
	 

	FormatDesc terrainFmt[] = {
		0, TYPE_VERTEX, FORMAT_FLOAT, 2,		
		//0, TYPE_COLOR,  FORMAT_FLOAT, 4,
	};

	if ((terrainVF = m_pRenderer->addVertexFormat(terrainFmt, elementsOf(terrainFmt), terrainShader)) == VF_NONE) 
	{
		assert(false);
		return;
	}

	// vertex XY resp. UV position
	float2 vInvSize(1.f/(float)m_Config.m_iSectorMeshVertexCount[0], 1.f/(float)m_Config.m_iSectorMeshVertexCount[1]);

	float* vertices = new float[iVertCount * sizeof(float2)];
	float *vDest = vertices;
	for (int y=0;y<=iSamplesY;y++)
		for (int x=0;x<=iSamplesX;x++)
		{
			*vDest++ = (float)x*vInvSize.x; 
			*vDest++ = (float)y*vInvSize.y;
		}

	// Double Buffer Check
	if ((terrainVB = m_pRenderer->addVertexBuffer(iVertCount * sizeof(float2), STATIC, vertices)) == VB_NONE)
	{
		assert(false);
		return; 
	}

	if ((terrainIB = m_pRenderer->addIndexBuffer(iIndexCount, sizeof(unsigned short), STATIC, &indices[0])) == IB_NONE) 
	{
		assert(false);
		return;
	}	
}

#define VERTEXINDEX(ix,iy)    ((iy)*iSingleRow+(ix))
#define ADDTRIANGLE(v0,v1,v2) \
{\
	assert((v0)>=0 && (v0)<iVertexCount);\
	assert((v1)>=0 && (v1)<iVertexCount);\
	assert((v2)>=0 && (v2)<iVertexCount);\
	bool bAddTriangle = true;\
	if (pTileHoleMask)\
{\
	int vx = ((v0)%iSingleRow);\
	int vy = ((v0)/iSingleRow);\
	vx = __min(vx,(v1)%iSingleRow);\
	vy = __min(vy,(v1)/iSingleRow);\
	vx = __min(vx,(v2)%iSingleRow);\
	vy = __min(vy,(v2)/iSingleRow);\
	int tx = vx / cfg.m_iHeightSamplesPerTile[0];\
	int ty = vy / cfg.m_iHeightSamplesPerTile[1];\
	bAddTriangle = !pTileHoleMask[ty*cfg.m_iTilesPerSectorMesh[0]+tx];\
}\
	if (bAddTriangle)\
{\
	pDestBuffer[iStartIndex+iIndex+0] = v0;\
	pDestBuffer[iStartIndex+iIndex+1] = v1;\
	pDestBuffer[iStartIndex+iIndex+2] = v2;\
	iIndex += 3;\
}\
}

int TerrainSectorManager::CreateLODIndexBuffer( const TerrainConfig &cfg, int iLOD, int iUp, int iRight, int iBottom, int iLeft, DynArray_cl<unsigned short> &pDestBuffer , int iStartIndex, const bool* pTileHoleMask )
{
	const int iSamplesX = cfg.m_iHeightSamplesPerSector[0] / cfg.m_iSectorMeshesPerSector[0];
	const int iSamplesY = cfg.m_iHeightSamplesPerSector[1] / cfg.m_iSectorMeshesPerSector[1];
	const int iVertexCount = (iSamplesX+1) * (iSamplesY+1);
	const int iStep = 1<<iLOD; // LOD==0 -> use all vertices
	const int iSingleRow = (iSamplesX+1);
	const int iVertexRowStride = iSingleRow*iStep; // num vertices per row
	const int iBorder = (iLOD==0) ? 0 : iStep;
	int iIndex = 0;
	int j,x,y,iLast,iV0,iV1=0;

	// in highest resolution, there are no subdivisions for the edges
	if (iLOD==0)
	{
		assert(iUp==0 && iRight==0 && iBottom==0 && iLeft==0 && "Cannot use increased edge resolution in highest resolution");
	}

	for (y=iBottom*iStep;y<iSamplesY-iUp*iStep;y+=iStep)
	{
		int iRow = y*iSingleRow;
		int iNextRow = iRow+iVertexRowStride;
		for (x=iLeft*iStep;x<iSamplesX-iRight*iStep;x+=iStep)
		{
			ADDTRIANGLE(iRow+x,iRow+x+iStep,iNextRow+x+iStep);
			ADDTRIANGLE(iRow+x,iNextRow+x+iStep,iNextRow+x);
		}
	}

	const int iHalfStep = iStep/2;

	// horizontal border
	iLast = iSamplesX-iBorder;
	for (x=iBorder;x<=iLast;x+=iStep)
	{
		const int iFanStart = (x==iBorder) ? iStep : iHalfStep;
		const int iFanEnd   = (x==iLast) ? iStep : iHalfStep;
		if (iBottom)
		{
			iV0 = VERTEXINDEX(x,iBorder);
			for (j=-iFanStart;j<iFanEnd;j+=iHalfStep)
			{
				iV1 = VERTEXINDEX(x+j,0);
				ADDTRIANGLE(iV0,iV1,iV1+iHalfStep);
			}
			if (x<iLast)
				ADDTRIANGLE(iV0,iV1+iHalfStep,iV0+iStep);
		}
		if (iUp)
		{
			iV0 = VERTEXINDEX(x,iSamplesY-iBorder);
			for (j=-iFanStart;j<iFanEnd;j+=iHalfStep)
			{
				iV1 = VERTEXINDEX(x+j,iSamplesY);
				ADDTRIANGLE(iV0,iV1+iHalfStep,iV1);
			}
			if (x<iLast)
				ADDTRIANGLE(iV0,iV0+iStep,iV1+iHalfStep);
		}
	}

	// vertical border
	iLast = iSamplesY-iBorder;
	for (y=iBorder;y<=iLast;y+=iStep)
	{
		const int iFanStart = (y==iBorder) ? iStep : iHalfStep;
		const int iFanEnd   = (y==iLast) ? iStep : iHalfStep;
		if (iLeft)
		{
			iV0 = VERTEXINDEX(iBorder,y);
			for (j=-iFanStart;j<iFanEnd;j+=iHalfStep)
			{
				iV1 = VERTEXINDEX(0,y+j);
				ADDTRIANGLE(iV1,iV0,iV1+iSingleRow*iHalfStep);
			}
			if (y<iLast)
				ADDTRIANGLE(iV1+iSingleRow*iHalfStep,iV0,iV0+iStep*iSingleRow);
		}

		if (iRight)
		{
			iV0 = VERTEXINDEX(iSamplesX-iBorder,y);
			for (j=-iFanStart;j<iFanEnd;j+=iHalfStep)
			{
				iV1 = VERTEXINDEX(iSamplesX,y+j);
				ADDTRIANGLE(iV1+iSingleRow*iHalfStep,iV0,iV1);
			}
			if (y<iLast)
				ADDTRIANGLE(iV0+iStep*iSingleRow,iV0,iV1+iSingleRow*iHalfStep);
		}
	}

	// corners:
	if (iBottom && !iLeft)
		ADDTRIANGLE(VERTEXINDEX(0,0),VERTEXINDEX(iStep,iStep),VERTEXINDEX(0,iStep))
	else if (!iBottom && iLeft)
	ADDTRIANGLE(VERTEXINDEX(0,0),VERTEXINDEX(iStep,0),VERTEXINDEX(iStep,iStep))

	if (iLeft && !iUp)
		ADDTRIANGLE(VERTEXINDEX(0,iSamplesY),VERTEXINDEX(iStep,iSamplesY-iStep),VERTEXINDEX(iStep,iSamplesY))
	else if (!iLeft && iUp)
	ADDTRIANGLE(VERTEXINDEX(0,iSamplesY),VERTEXINDEX(0,iSamplesY-iStep),VERTEXINDEX(iStep,iSamplesY-iStep))

	if (iRight && !iUp)
		ADDTRIANGLE(VERTEXINDEX(iSamplesX-iStep,iSamplesY),VERTEXINDEX(iSamplesX-iStep,iSamplesY-iStep),VERTEXINDEX(iSamplesX,iSamplesY))
	else if (!iRight && iUp)
	ADDTRIANGLE(VERTEXINDEX(iSamplesX,iSamplesY),VERTEXINDEX(iSamplesX-iStep,iSamplesY-iStep),VERTEXINDEX(iSamplesX,iSamplesY-iStep))

	if (iRight && !iBottom)
		ADDTRIANGLE(VERTEXINDEX(iSamplesX-iStep,0),VERTEXINDEX(iSamplesX,0),VERTEXINDEX(iSamplesX-iStep,iStep))
	else if (!iRight && iBottom)
	ADDTRIANGLE(VERTEXINDEX(iSamplesX-iStep,iStep),VERTEXINDEX(iSamplesX,0),VERTEXINDEX(iSamplesX,iStep))

	return iIndex;
}

TerrainSectorMeshPageInfo* TerrainSectorManager::GetMeshPageInfo( int x, int y )
{
	int sx = x/m_Config.m_iSectorMeshesPerSector[0];
	int sy = y/m_Config.m_iSectorMeshesPerSector[1];
	if (x<0 || y<0 || sx>=m_Config.m_iSectorCount[0] || sy>=m_Config.m_iSectorCount[1])
		return NULL;
	return GetSector(sx,sy)->GetMeshPageInfo(x%m_Config.m_iSectorMeshesPerSector[0],y%m_Config.m_iSectorMeshesPerSector[1]);
}

bool TerrainSectorManager::GetMeshPageInfo( int x, int y, int &iGlobalIndex ) const
{
	if (x<0 || y<0 || x>=m_Config.m_iOverallMeshPageCount[0] || y>=m_Config.m_iOverallMeshPageCount[1])
		return false;

	iGlobalIndex = y*m_Config.m_iOverallMeshPageCount[0]+x;
	return true;
}

SectorTile * TerrainSectorManager::GetTile( int x,int y ) const
{
	int sx = x/m_Config.m_iTilesPerSector[0];
	int sy = y/m_Config.m_iTilesPerSector[1];
	TerrainSector *pSector = GetSector(sx,sy);
	//pSector->EnsureLoaded();
	return pSector->GetTile(x-sx*m_Config.m_iTilesPerSector[0],y-sy*m_Config.m_iTilesPerSector[1]);
}

void TerrainSectorManager::BeginVisibilityUpdate( TerrainVisibilityCollector &infoComp )
{
	  float fFOVX,fFOVY;
	  // Get FOVX, FOVY 
	  //infoComp.m_pCollector->GetLODReferenceRenderContext()->GetFOV(fFOVX,fFOVY);
	  //m_Config.UpdateLODScaling(fFOVX,fFOVY, m_fLODScaling);

	  TerrainVisibilityInfo &info(infoComp.m_VisibilityInfo);
	  //info.Set(m_Config);
	  //info.m_bVisibleInContext = (m_iVisibleBitmask&info.m_iContextFilterMask)>0;
	  //if (!info.m_bVisibleInContext)
	  //  return;

	  // Editor Mode 아니면 한다.
#if 0
	  int x1,y1,x2,y2;
	  info.m_VisibleRangeBox.GetSectorIndices_Clamped(m_Config,x1,y1,x2,y2);
	  for (int y=y1;y<=y2;y++)
		  for (int x=x1;x<=x2;x++)
		  {
			  TerrainSector *pSector = GetSector(x,y);
			  if (pSector->m_bPrepared)
			  {
				 // pSector->UpdateTimeStamp();
				  continue;
			  }
			  
			  //pSector->PreCache();
		  }
#endif

}

void TerrainSectorManager::EndVisibilityUpdate( TerrainVisibilityCollector &infoComp )
{

}
