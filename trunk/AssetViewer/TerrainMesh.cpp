#include "stdafx.h"
#include "Terrain.h"
#include "TerrainMesh.h"

#include <U2_3D/Src/dx9/U2TristripData.h>

IMPLEMENT_RTTI(TerrainMesh, U2N2Mesh);

bool TerrainMeshGroup::Render(U2SceneMgr* pSceneMgr, U2RenderContext* pCxt)
{
	TerrainMesh* pMesh = static_cast<TerrainMesh*>(GetMesh());

	pMesh->GetTerrain()->Render(0.0f, 0.0f, 1.0f, 1.0f, pMesh->GetTerrain()->GetMaxLevels(), 1.0f);	

	return true;
}


//-------------------------------------------------------------------------------------------------
TerrainMesh::TerrainMesh(U2TriangleData* pData, Terrain* pTerrain) : U2N2Mesh(pData)
,m_pTerrain(pTerrain),
m_bCreated(false)
{

}

TerrainMesh::~TerrainMesh()
{

}


bool TerrainMesh::Initialize(int iNumRows, int iNumCols, bool bSkirt)
{
	U2TriStripData *pData = NULL;
	nMeshGroup* pMeshGroup = NULL;

	if (!m_bCreated)
	{
		pData = U2_NEW U2TriStripData;	
		//m_spMesh = U2_NEW U2N2Mesh(pData);		
		this->SetModelData(pData);
		this->SetUsages(U2N2Mesh::WriteOnly);
		this->SetVertexComponents(U2N2Mesh::Coord);

		//this->CreateMeshGroups(1);
		//m_meshGroups.Resize(1);
		m_meshGroups.AddElem(U2_NEW TerrainMeshGroup);
		U2Mesh::m_uNumGroups = 1;

		pMeshGroup = static_cast<nMeshGroup*>(GroupPtr(0));
		pMeshGroup->SetMesh(this);
		pMeshGroup->SetModelData(pData);					

		pData->SetStripsCnt(1);		
			
		this->LightOnOff(false);

		U2ASSERT(m_pTerrain->m_spEffect);
		pMeshGroup->AttachEffect(m_pTerrain->m_spEffect);

	


		m_bCreated = true;
	}

	if (!pData)
		return false;

	

	

	unsigned int iNumVertices;

	m_iNumRows = iNumRows;
	m_iNumCols = iNumCols;

	iNumVertices = iNumRows * iNumCols;
	pData->SetVertexCount(iNumVertices, iNumVertices);	

	// Indices
	
	m_bSkirt = bSkirt;

	if( m_bSkirt ) {
		// If we are using a skirt along the edge, 
		// we get an additional border of vertices.
		iNumVertices += 2*iNumRows + 2*iNumCols - 4;
		pData->SetVertexCount(iNumVertices, iNumVertices);			
		if( !InitSkirt())
			return false;
	}
	else 
	{	
		if (!InitNoSkirt())
			return false;
	}

	pData->SetStripLists((uint16*)m_spModelData->GetIndexArray());
	pData->SetStripLengths((uint16*)m_spModelData->GetArrayLengths());

	// Vertex

	pData->CreateVertices();
	D3DXVECTOR3* pVert = pData->GetVertices();

	float *vBasePtr = LockVertices();
	U2ASSERT(vBasePtr);

	float* pPtr = vBasePtr;
	FillVertices(pPtr);

	pPtr = vBasePtr + 3 * (m_iNumRows * m_iNumCols);

	if (m_bSkirt)
		FillSkirtVertices(pPtr);		

	for(uint32 i = 0; i < iNumVertices; i++)
	{
		pVert[i].x = *vBasePtr++;
		pVert[i].y = *vBasePtr++;
		pVert[i].z = *vBasePtr++;
		FILE_LOG(logDEBUG) << pVert[i].x << " " << pVert[i].y << " " << pVert[i].z;
	}

	UnlockVertices();	

	pMeshGroup->SetFirstVertex(0);
	pMeshGroup->SetNumVertices(iNumVertices);
	pMeshGroup->SetFirstIndex(0);	
	pMeshGroup->SetNumIndices(m_spModelData->GetArrayLengths()[0]);

	

	pMeshGroup->UpdateMeshState(false);

	return true;
}

bool TerrainMesh::InitNoSkirt()
{
	
	unsigned short uiNumStrips = static_cast<unsigned short>(m_iNumRows-1);
	unsigned short uiLineStep = static_cast<unsigned short>(m_iNumCols);
	unsigned short uiStartVertex = 0;
	unsigned short uiStepX = 1;
	
	int iNumIndices =  2*uiNumStrips*(m_iNumCols+1) - 2;

	

	uint16 numArrays = 1;
	uint16* pusArrayLengths;
	
	pusArrayLengths = (uint16*)m_spModelData->GetArrayLengths();
	if (pusArrayLengths)
		U2_FREE(pusArrayLengths);
	
	pusArrayLengths = U2_ALLOC(uint16, numArrays);

	// 기본적으로 1개이므로.
	pusArrayLengths[0] = iNumIndices;

	m_spModelData->SetIndices(iNumIndices, iNumIndices, 0, pusArrayLengths, numArrays);

	if (this->CreateEmptyBuffers())
	{
		U2ASSERT(FALSE);
		return false;
	}

	int ibSize = sizeof(uint16) * iNumIndices;
	uint16 *pIdxArray = (uint16*)U2_MALLOC(ibSize);

	uint16* p = LockIndices();
	U2ASSERT(p);

	for (unsigned short j = 0; j < uiNumStrips; j++) { 
		unsigned short uiVert = uiStartVertex;
		for (unsigned short k = 0; k < m_iNumCols; k++) {
			*(p++) = uiVert;			
			*(p++) = uiVert + uiLineStep;			
			uiVert = uiVert + uiStepX;
		}
		uiStartVertex = uiStartVertex + uiLineStep;
		if (j+1 < uiNumStrips) { // add degenerate triangle
			*(p++) = (uiVert-uiStepX)+uiLineStep;			
			*(p++) = uiStartVertex;
			
		}
	}

	memcpy_s(pIdxArray, ibSize, p, ibSize);

	UnlockIndices();

	m_spModelData->SetIndexArray(pIdxArray);

	return true;
}

bool TerrainMesh::InitSkirt()
{
	

	unsigned short uiNumRows = static_cast<unsigned short>(m_iNumRows);
	unsigned short uiNumCols = static_cast<unsigned short>(m_iNumCols);
	unsigned short uiNumStrips = uiNumRows-1;
	unsigned short uiLineStep = uiNumCols;
	unsigned short uiStartVertex = 0;
	unsigned short uiStepX = 1;

	unsigned short uiNumStripIndices = 2*uiNumStrips*(uiNumCols+1) - 2 + 2 + 4*uiNumCols * 2*uiNumStrips + 2*(uiNumStrips-1);

	uint16 numArrays = 1;
	uint16* pusArrayLengths;

	pusArrayLengths = (uint16*)m_spModelData->GetArrayLengths();
	if (pusArrayLengths)
		U2_FREE(pusArrayLengths);

	pusArrayLengths = U2_ALLOC(uint16, numArrays);

	// 기본적으로 1개이므로.
	pusArrayLengths[0] = uiNumStripIndices;

	uint32 maxTris = uiNumStripIndices - 2;
	m_spModelData->SetIndices(maxTris, maxTris, 0, pusArrayLengths, 1);


	if (!this->CreateEmptyBuffers())
	{
		U2ASSERT(FALSE);
		return false;
	}

	int ibSize = sizeof(uint16) * uiNumStripIndices;
	uint16 *pIdxArray = (uint16*)U2_MALLOC(ibSize);

	uint16* iPtr;

	iPtr = LockIndices();
	uint16* p = iPtr;
	U2ASSERT(p);

	// regular grid
	for (unsigned short j = 0; j < uiNumStrips; j++) { 
		unsigned short uiVert = uiStartVertex;
		for (unsigned short k = 0; k < uiNumCols; k++) {
			*(p++) = uiVert;
			*(p++) = uiVert + uiLineStep;
			uiVert = uiVert + uiStepX;
		}
		uiStartVertex = uiStartVertex + uiLineStep;
		if (j+1 < uiNumStrips) { // add degenerate triangle
			*(p++) = (uiVert-uiStepX)+uiLineStep;
			*(p++) = uiStartVertex;
		}
	}

	// add degenerate triangles to start over
	uiStartVertex = uiStartVertex + uiLineStep;
	*(p++) = *(p-1);
	*(p++) = uiStartVertex;

	// bottom border
	unsigned short uiSkirt = uiStartVertex;
	for (unsigned short k = 0; k < uiNumCols; k++) {
		*(p++) = uiSkirt;
		*(p++) = k;
		uiSkirt++;
	}

	// right border
	uiSkirt++;
	unsigned short uiVert = uiNumCols + uiLineStep - 1;
	for (unsigned short j = 0; j < uiNumStrips-1; j++) {
		*(p++) = uiSkirt;
		*(p++) = uiVert;
		uiSkirt += 2;
		uiVert = uiVert + uiLineStep;
	}

	// top border, from right to left
	uiVert = uiNumRows*uiNumCols-1; 
	uiSkirt =  uiVert + 2*uiLineStep + 2*(uiLineStep-2);
	for (unsigned short k = 0; k < uiNumCols; k++) {
		*(p++) = uiSkirt;
		*(p++) = uiVert;
		uiSkirt--;
		uiVert--;
	}

	// left border, from top to bottom
	uiSkirt = uiSkirt - 1;
	uiVert = uiVert - uiLineStep + 1;
	for (unsigned short j = 0; j < uiNumStrips-1; j++) {
		*(p++) = uiSkirt;
		*(p++) = uiVert;
		uiSkirt -= 2;
		uiVert = uiVert - uiLineStep;
	}
	uiSkirt = uiNumRows*uiNumCols;
	*(p++) = uiSkirt;
	*(p++) = uiVert;	

	

	memcpy_s(pIdxArray, ibSize, iPtr, ibSize);

	UnlockIndices();

	m_spModelData->SetIndexArray(pIdxArray);	

	FILE_LOG(logDEBUG) << _T("TriStrip Indices");

	for (int i = 0; i < uiNumStripIndices; i++)
	{
		FILE_LOG(logDEBUG) << "idx: " << i << " " << pIdxArray[i];
	}


	return true;
}

//----------------------------------------------------------------------------------

bool TerrainMesh::FillVertices(float *vPtr)
{
	
	const float fInvScaleX = 1.0f / (m_iNumCols-1.0f);
	const float fInvScaleZ = 1.0f / (m_iNumRows-1.0f);

	D3DXVECTOR3 vScale = m_tmWorld.GetScale();

	float s, t;
	for (int i = 0; i < m_iNumRows; i++) {
		t = i * fInvScaleZ;
		for (int j = 0; j < m_iNumCols; j++) {
			s = j * fInvScaleX;

			*vPtr++ = s * vScale.x;		// x
			*vPtr++ = 1.0f * vScale.y;		// y
			*vPtr++ = t * vScale.z;		// z			
		}
	}

	return true;
}

//----------------------------------------------------------------------------------

bool TerrainMesh::FillSkirtVertices(float *vPtr)
{
	
	const float fInvScaleX = 1.0f / (m_iNumCols-1.0f);
	const float fInvScaleZ = 1.0f / (m_iNumRows-1.0f);

	D3DXVECTOR3 vScale = m_tmWorld.GetScale();

	float s, t;
	t = 0.0f;
	for (int j = 0; j < m_iNumCols; j++) {
		s = j * fInvScaleX;

		*vPtr++ = s * vScale.x;		// x
		*vPtr++ = -1.0f * vScale.y;	// y
		*vPtr++ = t * vScale.z;		// z			
	}

	for (int i = 1; i < m_iNumRows-1; i++) {
		t = i * fInvScaleZ;
		s = 0.0f;
		*vPtr++ = s * vScale.x;		// x
		*vPtr++ = -1.0f * vScale.y;	// y
		*vPtr++ = t * vScale.z;		// z			
		s = 1.0f;
		*vPtr++ = s * vScale.x;		// x
		*vPtr++ = -1.0f * vScale.y;	// y
		*vPtr++ = t * vScale.z;		// z					
	}

	t = 1.0f;
	for (int j = 0; j < m_iNumCols; j++) {
		s = j * fInvScaleX;
		*vPtr++ = s * vScale.x;		// x
		*vPtr++ = -1.0f * vScale.y;	// y
		*vPtr++ = t * vScale.z;		// z			
	}

	return true;
}

void TerrainMesh::GetVisibleSet(U2Culler& culler, bool bNoCull)
{
	U2N2Mesh::GetVisibleSet(culler, bNoCull);
}
