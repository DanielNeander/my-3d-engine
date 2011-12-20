#include "stdafx.h"
#include "PolygonUtil.h"
#include "Vertex.h"

void GenerateGrid(unsigned short numVertRows, unsigned short numVertCols, 
				 float dx, float dz, const D3DXVECTOR3& vCenter,
				 std::vector<D3DXVECTOR3>& verts, std::vector<DWORD>& indices)
{

	unsigned short numVertices = numVertRows*numVertCols;
	unsigned short numCellRows = numVertRows-1;
	unsigned short numCellCols = numVertCols-1;

	unsigned short numTris = numCellRows*numCellCols*2;

	float width = (float)numCellCols * dx;
	float depth = (float)numCellRows * dz;

	//===========================================
	// Build vertices.

	// We first build the grid geometry centered about the origin and on
	// the xz-plane, row-by-row and in a top-down fashion.  We then translate
	// the grid vertices so that they are centered about the specified 
	// parameter 'center'.

	verts.resize( numVertices );

	// Offsets to translate grid from quadrant 4 to center of 
	// coordinate system.
	float xOffset = -width * 0.5f; 
	float zOffset =  depth * 0.5f;

	int k = 0;
	for(float i = 0; i < numVertRows; ++i)
	{
		for(float j = 0; j < numVertCols; ++j)
		{
			// Negate the depth coordinate to put in quadrant four.  
			// Then offset to center about coordinate system.
			verts[k].x =  j * dx + xOffset;
			verts[k].z = -i * dz + zOffset;
			verts[k].y =  0.0f;

			// Translate so that the center of the grid is at the
			// specified 'center' parameter.
			D3DXMATRIX T;
			D3DXMatrixTranslation(&T, vCenter.x, vCenter.y, vCenter.z);
			D3DXVec3TransformCoord(&verts[k], &verts[k], &T);

			++k; // Next vertex
		}
	}

	//===========================================
	// Build indices.

	indices.resize(numTris * 3);

	// Generate indices for each quad.
	k = 0;
	for(DWORD i = 0; i < (DWORD)numCellRows; ++i)
	{
		for(DWORD j = 0; j < (DWORD)numCellCols; ++j)
		{
			indices[k]     =   i   * numVertCols + j;
			indices[k + 1] =   i   * numVertCols + j + 1;
			indices[k + 2] = (i+1) * numVertCols + j;

			indices[k + 3] = (i+1) * numVertCols + j;
			indices[k + 4] =   i   * numVertCols + j + 1;
			indices[k + 5] = (i+1) * numVertCols + j + 1;

			// next quad
			k += 6;
		}
	}

}void LoadXFile(
				const std::string& filename, 
				ID3DXMesh** meshOut,
				std::vector<Mtrl>& mtrls, 
				std::vector<IDirect3DTexture9*>& texs)
{
	// Step 1: Load the .x file from file into a system memory mesh.

	ID3DXMesh* meshSys      = 0;
	ID3DXBuffer* adjBuffer  = 0;
	ID3DXBuffer* mtrlBuffer = 0;
	DWORD numMtrls          = 0;

	HR(D3DXLoadMeshFromX(filename.c_str(), D3DXMESH_SYSTEMMEM, gd3dDevice,
		&adjBuffer,	&mtrlBuffer, 0, &numMtrls, &meshSys));


	// Step 2: Find out if the mesh already has normal info?

	D3DVERTEXELEMENT9 elems[MAX_FVF_DECL_SIZE];
	HR(meshSys->GetDeclaration(elems));

	bool hasNormals = false;
	D3DVERTEXELEMENT9 term = D3DDECL_END();
	for(int i = 0; i < MAX_FVF_DECL_SIZE; ++i)
	{
		// Did we reach D3DDECL_END() {0xFF,0,D3DDECLTYPE_UNUSED, 0,0,0}?
		if(elems[i].Stream == 0xff )
			break;

		if( elems[i].Type == D3DDECLTYPE_FLOAT3 &&
			elems[i].Usage == D3DDECLUSAGE_NORMAL &&
			elems[i].UsageIndex == 0 )
		{
			hasNormals = true;
			break;
		}
	}


	// Step 3: Change vertex format to VertexPNT.

	D3DVERTEXELEMENT9 elements[64];
	UINT numElements = 0;
	VertexPNT::Decl->GetDeclaration(elements, &numElements);

	ID3DXMesh* temp = 0;
	HR(meshSys->CloneMesh(D3DXMESH_SYSTEMMEM, 
		elements, gd3dDevice, &temp));
	ReleaseCOM(meshSys);
	meshSys = temp;


	// Step 4: If the mesh did not have normals, generate them.

	if( hasNormals == false)
		HR(D3DXComputeNormals(meshSys, 0));


	// Step 5: Optimize the mesh.

	HR(meshSys->Optimize(D3DXMESH_MANAGED | 
		D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, 
		(DWORD*)adjBuffer->GetBufferPointer(), 0, 0, 0, meshOut));
	ReleaseCOM(meshSys); // Done w/ system mesh.
	ReleaseCOM(adjBuffer); // Done with buffer.

	// Step 6: Extract the materials and load the textures.

	if( mtrlBuffer != 0 && numMtrls != 0 )
	{
		D3DXMATERIAL* d3dxmtrls = (D3DXMATERIAL*)mtrlBuffer->GetBufferPointer();

		for(DWORD i = 0; i < numMtrls; ++i)
		{
			// Save the ith material.  Note that the MatD3D property does not have an ambient
			// value set when its loaded, so just set it to the diffuse value.
			Mtrl m;
			m.ambient   = d3dxmtrls[i].MatD3D.Diffuse;
			m.diffuse   = d3dxmtrls[i].MatD3D.Diffuse;
			m.spec      = d3dxmtrls[i].MatD3D.Specular;
			m.specPower = d3dxmtrls[i].MatD3D.Power;
			mtrls.push_back( m );

			// Check if the ith material has an associative texture
			if( d3dxmtrls[i].pTextureFilename != 0 )
			{
				// Yes, load the texture for the ith subset
				IDirect3DTexture9* tex = 0;
				char* texFN = d3dxmtrls[i].pTextureFilename;
				HR(D3DXCreateTextureFromFile(gd3dDevice, texFN, &tex));

				// Save the loaded texture
				texs.push_back( tex );
			}
			else
			{
				// No texture for the ith subset
				texs.push_back( 0 );
			}
		}
	}
	ReleaseCOM(mtrlBuffer); // done w/ buffer
}

U2N2Mesh* ConvertXToMesh(ID3DXMesh* pIn, IDirect3DDevice9* pd3dDevice)
{


	//1. 정점 포맷 확인 
	D3DVERTEXELEMENT9 declaration[MAX_FVF_DECL_SIZE];
	pIn->GetDeclaration(declaration);

	LPDIRECT3DVERTEXDECLARATION9 pVertDecl = 0;
	HRESULT hr = pd3dDevice->CreateVertexDeclaration(declaration, 
		&pVertDecl);

	U2TriListData *pData = NULL;
	nMeshGroup* pMeshGroup = NULL;

	pData = U2_NEW U2TriListData;	
	U2N2Mesh* pOut = U2_NEW U2N2Mesh(pData);
	pOut->LightOnOff(false);
	pOut->SetModelData(pData);
	pOut->SetUsages(U2N2Mesh::WriteOnly);
	pData->SetVertexDeclaration(pVertDecl);

	pOut->CreateMeshGroups(1);	
	pMeshGroup = static_cast<nMeshGroup*>(pOut->GroupPtr(0));
	pMeshGroup->SetMesh(pOut);
	pMeshGroup->SetModelData(pData);	

	//2. 정점 데이터 복사
	// Create our model VB
	int numMeshVerts = pIn->GetNumVertices();
	
	pData->SetVertexCount(numMeshVerts, numMeshVerts);
	pOut->SetVertexStride(pIn->GetNumBytesPerVertex());

	
	int bufferSize = numMeshVerts*D3DXGetDeclVertexSize( declaration, 0 );
	
	void *vBasePtr = pOut->LockVertices();
	// Fill the vertex buffer with data from the mesh object
	LPVOID pMeshData;
	pIn->LockVertexBuffer(0,&pMeshData);
	vBasePtr = pMeshData;	
	pOut->UnlockVertices();	
	V(pIn->UnlockVertexBuffer());

	pMeshGroup->SetFirstVertex(0);
	pMeshGroup->SetNumVertices(numMeshVerts);

	//3. 인덱스 데이터 복사
	// Now for the IB.  This is silly. Gotta basically memcpy out the IB to replicate it.  
	LPDIRECT3DINDEXBUFFER9* ppIB;
	V(pIn->GetIndexBuffer(ppIB));
	D3DINDEXBUFFER_DESC ibDesc;
	V((*ppIB)->GetDesc(&ibDesc));
	(*ppIB)->Release();

	int numMeshIndices = ibDesc.Format==D3DFMT_INDEX16?(ibDesc.Size/2):(ibDesc.Size/4);
	int numPolysPerModel = numMeshIndices/3;

	int indexBufferSize = ibDesc.Format==D3DFMT_INDEX16?(numMeshIndices*2):(numMeshIndices*4);
	
	uint16* pusIdxArray = U2_ALLOC(uint16, numMeshIndices);
	pData->SetIndices(numPolysPerModel, numPolysPerModel, pusIdxArray, 0, 1);
	
	pOut->CreateEmptyBuffers();


	LPVOID pIBDataSrc;	
	V(pIn->LockIndexBuffer(D3DLOCK_READONLY,&pIBDataSrc));	
	uint16* pIBDataDst = pOut->LockIndices();			
	memcpy(pIBDataDst, pIBDataSrc, indexBufferSize);
	V(pIn->UnlockIndexBuffer());
	pOut->UnlockIndices();

	SAFE_RELEASE(pIn);
}