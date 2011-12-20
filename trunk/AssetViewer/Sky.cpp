#include "stdafx.h"
#include "Sky.h"


CSky::CSky(U2TriangleData *pData)
:U2N2Mesh(pData)
{
	mPlanetRadius = 0.0f;
	mAtmosphereRadius = 0.0f;
	mHTileFactor = 1.0f;
	mVTileFactor = 1.0f;
}

// Initialize
HRESULT CSky::Init(		  int divisions,
						  float planetRadius,
						  float atmosphereRadius,
						  float hTilefactor,
						  float vTilefactor)
{
	int divs = divisions;
	if (divisions < 1) 
		divs = 1;

	if (divisions > 256) 
		divs = 256; //64k verts is the max size for a VB 

	mPlanetRadius = planetRadius;
	mAtmosphereRadius = atmosphereRadius;
	mHTileFactor = hTilefactor;
	mVTileFactor = vTilefactor;

	unsigned int uiNumVertices, uiNumFaces, uiNumIndices;
	uiNumVertices = (divs + 1) * (divs + 1);
	uiNumFaces = divs * divs * 2;
	uiNumIndices = uiNumFaces * 3;

	U2TriListData *pData = NULL;
	nMeshGroup* pMeshGroup = NULL;

	pData = U2_NEW U2TriListData;	
	//m_spMesh = U2_NEW U2N2Mesh(pData);		
	this->SetModelData(pData);
	this->SetUsages(U2N2Mesh::WriteOnly);
	this->SetVertexComponents(U2N2Mesh::Coord | U2N2Mesh::Normal |
		U2N2Mesh::Tangent | U2N2Mesh::Uv0);

	//this->CreateMeshGroups(1);
	//m_meshGroups.Resize(1);
	m_meshGroups.AddElem(U2_NEW nMeshGroup);
	U2Mesh::m_uNumGroups = 1;

	pMeshGroup = static_cast<nMeshGroup*>(GroupPtr(0));
	pMeshGroup->SetMesh(this);
	pMeshGroup->SetModelData(pData);	


	// 저장하지 않음...
	pData->SetVertexCount(uiNumVertices, uiNumVertices);
	pData->CreateVertices();
	pData->CreateNormals(false);
	pData->CreateTangents(false);
	pData->CreateTexCoords(1);

	if (!this->CreateEmptyBuffers())
	{
		U2ASSERT(FALSE);
		return false;
	}

	float *vBasePtr = LockVertices();
	U2ASSERT(vBasePtr);

	if (SetupVertices(vBasePtr) != S_OK)
	{
		U2ASSERT(false);
		return E_FAIL;
	}

	UnlockVertices();	

	if (SetupIndices(uiNumFaces, uiNumIndices) != S_OK)
	{
		U2ASSERT(false);
		return E_FAIL;
	}


	pMeshGroup->SetFirstVertex(0);
	pMeshGroup->SetNumVertices(uiNumVertices);
	pMeshGroup->SetFirstIndex(0);	
	pMeshGroup->SetNumIndices(uiNumIndices);
}

HRESULT CSky::SetupVertices(float *pVertices)
{
	if(pVertices == NULL) return E_FAIL;

	//since we don't store the number of divisions in the plane, we can just get it back
	int num_divisions = (int)sqrt((float)(GetActiveTriangleCount() * 3)/6);

	float plane_size = 2.0f * (float)sqrt((mAtmosphereRadius*mAtmosphereRadius) - 
		(mPlanetRadius*mPlanetRadius));

	float delta = plane_size/(float)num_divisions;
	float tex_delta = 2.0f/(float)num_divisions;
	float x_dist   = 0.0f;
	float z_dist   = 0.0f;
	float x_height = 0.0f;
	float z_height = 0.0f;
	float height = 0.0f;

	int count = 0;

	

	for (int i=0;i<=num_divisions;i++)
	{
		for (int j=0;j<=num_divisions;j++)
		{
			x_dist = (-0.5f * plane_size) + ((float)j*delta);
			z_dist = (-0.5f * plane_size) + ((float)i*delta);

			x_height = (x_dist*x_dist) / mAtmosphereRadius;
			z_height = (z_dist*z_dist) / mAtmosphereRadius;
			height = x_height + z_height;

			D3DXVECTOR3* pVert = GetMeshData()->GetVertices();
			D3DXVECTOR3* pNorm = GetMeshData()->GetNormals();			
			D3DXVECTOR2* pUV0 = GetMeshData()->GetTexCoordSet(0);

			pVert[i*num_divisions+j] = D3DXVECTOR3(x_dist, -height, z_dist);
			pNorm[i*num_divisions+j] = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
			pUV0[i*num_divisions+j] = D3DXVECTOR2(mHTileFactor*((float)j * tex_delta*0.5f),
				mVTileFactor*(1.0f - (float)i * tex_delta*0.5f));

	
			*pVertices++ = x_dist; *pVertices++ = 0.0f - height; *pVertices++ = z_dist; 
			*pVertices++ = 0.0f; *pVertices++ = -1.0f; *pVertices++ = 0.0f; 
			*pVertices++ = pUV0[i*num_divisions+j].x; *pVertices++ = 
				pUV0[i*num_divisions+j].y;  
		}
	}

	return S_OK;

}

HRESULT CSky::SetupIndices(uint32 uiNumFaces, uint32 uiNumIndices)
{

	uint16* pusIdxArray = U2_ALLOC(uint16, uiNumIndices);
	m_spModelData->SetIndices(uiNumFaces, uiNumFaces, pusIdxArray, 0, 1);

	int num_divisions = (int)sqrt((float)(GetActiveTriangleCount() * 3)/6);

	int ibSize = sizeof(uint16) * uiNumIndices;
	uint16* p = LockIndices();	
	U2ASSERT(p);
	DWORD k = 0;
	// Iterate over each quad and compute indices.	
	for(DWORD i = 0; i < num_divisions; ++i)
	{
		for(DWORD j = 0; j < num_divisions; ++j)
		{
			int startvert = (i*(num_divisions+1) + j);

			pusIdxArray[k] = p[k] = startvert;
			pusIdxArray[k+1] = p[k+1] = startvert+1;
			pusIdxArray[k+2] = p[k+2] = startvert + num_divisions+1;

			pusIdxArray[k+3] = p[k+3] = startvert + 1;
			pusIdxArray[k+4] = p[k+4] = startvert + num_divisions + 2;
			pusIdxArray[k+5] = p[k+5] = startvert + num_divisions + 1;
			k+=6;
		}
	}

	UnlockIndices();
	m_spModelData->SetIndexArray(pusIdxArray);	
	return S_OK;
}
