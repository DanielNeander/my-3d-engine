#include "stdafx.h"
#include "QuadMesh.h"
#include <U2_3D/Src/dx9/U2TriListData.h>

IMPLEMENT_RTTI(QuadMesh, U2N2Mesh);

QuadMesh::QuadMesh(U2TriangleData *pData) : U2N2Mesh(pData)
,mNumRows(0)
,mNumCols(0)
{

}


bool QuadMesh::Initialize(DWORD m, DWORD n, float dx)
{
	mNumRows = m;
	mNumCols = n;

	unsigned int uiNumVertices, uiNumFaces, uiNumIndices;
	uiNumVertices = m * n;
	uiNumFaces = (m-1) * (n-1) * 2;
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
	//D3DXVECTOR3* pVert = pData->GetVertices();
	float halfWidth = (n-1)*dx*0.5f;
	float halfDepth = (m-1)*dx*0.5f;

	uint16* pusIdxArray = U2_ALLOC(uint16, uiNumIndices);
	m_spModelData->SetIndices(uiNumFaces, uiNumFaces, pusIdxArray, 0, 1);

	if (!this->CreateEmptyBuffers())
	{
		U2ASSERT(FALSE);
		return false;
	}

	float *vBasePtr = LockVertices();
	U2ASSERT(vBasePtr);

	float du = 1.0f / (n-1);
	float dv = 1.0f / (m-1);
	for(DWORD i = 0; i < m; i++)
	{
		float z = halfDepth - i*dx;
		for(DWORD j = 0; j < n; j++)
		{
			float x = -halfWidth + j*dx;
			D3DXVECTOR3* pVert = pData->GetVertices();
			D3DXVECTOR3* pNorm = pData->GetNormals();
			D3DXVECTOR3* pTangent = pData->GetTangents();
			D3DXVECTOR2* pUV0 = pData->GetTexCoordSet(0);

			pVert[i*n+j] = D3DXVECTOR3(x, 0.0f, z);
			pNorm[i*n+j] = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
			pTangent[i*n+j]	= D3DXVECTOR3(1.0f, 0.0f, 0.0f);
			pUV0[i*n+j] = D3DXVECTOR2(j * du, i * dv);

			*vBasePtr++ = x;	*vBasePtr++ = 0.0f;	*vBasePtr++ = z;
			*vBasePtr++ = 0.0f; *vBasePtr++ = 1.0f; *vBasePtr++ = 0.0f;
			*vBasePtr++ = 1.0f; *vBasePtr++ = 0.0f; *vBasePtr++ = 0.0f;
			*vBasePtr++ = j * du; *vBasePtr++ = i * dv;
		}
	}

	UnlockVertices();	

	pMeshGroup->SetFirstVertex(0);
	pMeshGroup->SetNumVertices(uiNumVertices);
		
	

	

	int ibSize = sizeof(uint16) * uiNumIndices;
	uint16* p = LockIndices();	
	U2ASSERT(p);
	DWORD k = 0;
	// Iterate over each quad and compute indices.	
	for(DWORD i = 0; i < m-1; ++i)
	{
		for(DWORD j = 0; j < n-1; ++j)
		{
			pusIdxArray[k] = p[k] = i*n+j;
			pusIdxArray[k+1] = p[k+1] = i*n+j+1;
			pusIdxArray[k+2] = p[k+2] = (i+1)*n+j;

			pusIdxArray[k+3] = p[k+3] = (i+1)*n+j;
			pusIdxArray[k+4] = p[k+4] = i*n+j+1;
			pusIdxArray[k+5] = p[k+5] = (i+1)*n+j+1;			
			k+=6;
		}
	}
	
	UnlockIndices();
	m_spModelData->SetIndexArray(pusIdxArray);
	pMeshGroup->SetFirstIndex(0);	
	pMeshGroup->SetNumIndices(uiNumIndices);
	pMeshGroup->UpdateMeshState(true);

}
