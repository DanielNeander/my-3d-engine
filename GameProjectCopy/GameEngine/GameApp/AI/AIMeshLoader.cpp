#include "stdafx.h"
#include "GameApp/SceneNode.h"
#include "ModelLib/M2Loader.h"
#include "../ObjLoader.h"
#include "AIMeshLoader.h"
#include "../GameApp.h"
#include "../Mesh.h"

rcMeshLoaderObj::rcMeshLoaderObj() :
	m_verts(0),	m_tris(0), m_normals(0), m_vertCount(0), m_triCount(0),
	ntris(0), tris(0), verts(0), nverts(0), numEnt(0)
{

}

rcMeshLoaderObj::~rcMeshLoaderObj()
{
	delete [] m_verts;
	delete [] m_normals;
	delete [] m_tris;
	delete [] tris;
	delete [] verts;
}

bool rcMeshLoaderObj::Load( const char* filename )
{			
	objLoader.Create(GetApp()->GetDevice(), filename);

	const int numNodes = 1;
	size_t *meshVertexCount = new size_t[numNodes];
	size_t *meshIndexCount = new size_t[numNodes];
	noVec3** meshVertices = new noVec3*[numNodes];
	unsigned long **meshIndices = new unsigned long*[numNodes]; 

	for (int i = 0 ; i < numNodes ; i++)
	{
		meshVertices[i] = new noVec3[objLoader.GetVertexCount()];
		meshIndices[i] = new unsigned long[objLoader.GetIndexCount()];

		meshVertexCount[i] = objLoader.GetVertexCount();
		meshIndexCount[i] = objLoader.GetIndexCount();

		//memcpy_s(meshVertices[i], meshVertexCount[i] * sizeof(noVec3), objLoader.GetVertices(), meshVertexCount[i] * sizeof(noVec3));
		for (int x = 0; x < objLoader.GetVertexCount(); ++x)		
			meshVertices[i][x] =  worldTM_ *  objLoader.GetVertices()[x].position ;
			
		
		memcpy_s(meshIndices[i], meshIndexCount[i] * sizeof(DWORD), objLoader.GetIndices(), meshIndexCount[i] * sizeof(DWORD));		

		//total number of verts
		nverts += meshVertexCount[i];
		//total number of indices
		ntris += meshIndexCount[i];
	}

	verts = new float[nverts*3];// *3 as verts holds x,y,&z for each verts in the array
	tris = new int[ntris];// tris in recast is really indices like ogre
	
	//convert index count into tri count
	ntris = ntris/3; //although the tris array are indices the ntris is actual number of triangles, eg. indices/3;
	int vertsIndex = 0;
	int prevVerticiesCount = 0;
	int prevIndexCountTotal = 0;

	for (uint i = 0 ; i < 1 ; i++)
	{
		noVec3 vertexPos;
		for (uint j = 0 ; j < meshVertexCount[i] ; j++)
		{			
			vertexPos = meshVertices[i][j];
			verts[vertsIndex] = vertexPos.x;
			verts[vertsIndex+1] = vertexPos.y;
			verts[vertsIndex+2] = vertexPos.z;
			vertsIndex+=3;
		}

		for (uint j = 0 ; j < meshIndexCount[i] ; j++)
		{
			tris[prevIndexCountTotal+j] = meshIndices[i][j]+prevVerticiesCount;
		}
		prevIndexCountTotal += meshIndexCount[i];
		prevVerticiesCount = meshVertexCount[i];
	}



	for (int i = 0 ; i < numNodes ; i++)
	{
		delete [] meshVertices[i];
		delete [] meshIndices[i];
	}
	delete [] meshVertices;
	delete [] meshVertexCount;
	delete [] meshIndices;
	delete [] meshIndexCount;

	// calculate normals data for Recast - im not 100% sure where this is required
	// but it is used, Ogre handles its own Normal data for rendering, this is not related
	// to Ogre at all ( its also not correct lol )
	// TODO : fix this
	m_normals = new float[ntris*3];
	for (int i = 0; i < ntris*3; i += 3)
	{
		const float* v0 = &verts[tris[i]*3];
		const float* v1 = &verts[tris[i+1]*3];
		const float* v2 = &verts[tris[i+2]*3];
		float e0[3], e1[3];
		for (int j = 0; j < 3; ++j)
		{
			e0[j] = (v1[j] - v0[j]);
			e1[j] = (v2[j] - v0[j]);
		}
		float* n = &m_normals[i];
		n[0] = ((e0[1]*e1[2]) - (e0[2]*e1[1]));
		n[1] = ((e0[2]*e1[0]) - (e0[0]*e1[2]));
		n[2] = ((e0[0]*e1[1]) - (e0[1]*e1[0]));

		float d = sqrtf(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
		if (d > 0)
		{
			d = 1.0f/d;
			n[0] *= d;
			n[1] *= d;
			n[2] *= d;
		}	
	}

	return true;
}

bool rcMeshLoaderObj::LoadTerrain()
{
	return true;
}

bool rcMeshLoaderObj::LoadMesh(Mesh* pMesh)
{
	const int numNodes = 1;
	size_t *meshVertexCount = new size_t[numNodes];
	size_t *meshIndexCount = new size_t[numNodes];
	noVec3** meshVertices = new noVec3*[numNodes];
	unsigned long **meshIndices = new unsigned long*[numNodes]; 

	for (int i = 0 ; i < numNodes ; i++)
	{
		meshVertices[i] = new noVec3[pMesh->m_iNumVertices];
		meshIndices[i] = new unsigned long[pMesh->GetFaceCount() * 3];

		meshVertexCount[i] = pMesh->m_iNumVertices;
		meshIndexCount[i] = pMesh->GetFaceCount() * 3;

		//memcpy_s(meshVertices[i], meshVertexCount[i] * sizeof(noVec3), objLoader.GetVertices(), meshVertexCount[i] * sizeof(noVec3));
		int numElem = pMesh->m_iVertexSize /4;
		for (int x = 0; x < meshVertexCount[i]; ++x)		

			meshVertices[i][x] =  worldTM_ *  noVec3(pMesh->GetVertices()[x * numElem],
			pMesh->GetVertices()[x * numElem+1], pMesh->GetVertices()[x * numElem+2]);

		uint16 i0, i1, i2;
		
		for (int x=0; x < pMesh->GetFaceCount(); x++)
		{
			pMesh->GetTriangleIndices(x, i0, i1, i2);
			meshIndices[i][x*3] = i0;
			meshIndices[i][x*3+1] = i1;
			meshIndices[i][x*3+2] = i2;		
		}
		
		//total number of verts
		nverts += meshVertexCount[i];
		//total number of indices
		ntris += meshIndexCount[i];
	}

	verts = new float[nverts*3];// *3 as verts holds x,y,&z for each verts in the array
	tris = new int[ntris];// tris in recast is really indices like ogre

	//convert index count into tri count
	ntris = ntris/3; //although the tris array are indices the ntris is actual number of triangles, eg. indices/3;
	int vertsIndex = 0;
	int prevVerticiesCount = 0;
	int prevIndexCountTotal = 0;

	for (uint i = 0 ; i < 1 ; i++)
	{
		noVec3 vertexPos;
		for (uint j = 0 ; j < meshVertexCount[i] ; j++)
		{			
			vertexPos = meshVertices[i][j];
			verts[vertsIndex] = vertexPos.x;
			verts[vertsIndex+1] = vertexPos.y;
			verts[vertsIndex+2] = vertexPos.z;
			vertsIndex+=3;
		}

		for (uint j = 0 ; j < meshIndexCount[i] ; j++)
		{
			tris[prevIndexCountTotal+j] = meshIndices[i][j]+prevVerticiesCount;
		}
		prevIndexCountTotal += meshIndexCount[i];
		prevVerticiesCount = meshVertexCount[i];
	}



	for (int i = 0 ; i < numNodes ; i++)
	{
		delete [] meshVertices[i];
		delete [] meshIndices[i];
	}
	delete [] meshVertices;
	delete [] meshVertexCount;
	delete [] meshIndices;
	delete [] meshIndexCount;

	// calculate normals data for Recast - im not 100% sure where this is required
	// but it is used, Ogre handles its own Normal data for rendering, this is not related
	// to Ogre at all ( its also not correct lol )
	// TODO : fix this
	m_normals = new float[ntris*3];
	for (int i = 0; i < ntris*3; i += 3)
	{
		const float* v0 = &verts[tris[i]*3];
		const float* v1 = &verts[tris[i+1]*3];
		const float* v2 = &verts[tris[i+2]*3];
		float e0[3], e1[3];
		for (int j = 0; j < 3; ++j)
		{
			e0[j] = (v1[j] - v0[j]);
			e1[j] = (v2[j] - v0[j]);
		}
		float* n = &m_normals[i];
		n[0] = ((e0[1]*e1[2]) - (e0[2]*e1[1]));
		n[1] = ((e0[2]*e1[0]) - (e0[0]*e1[2]));
		n[2] = ((e0[0]*e1[1]) - (e0[1]*e1[0]));

		float d = sqrtf(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
		if (d > 0)
		{
			d = 1.0f/d;
			n[0] *= d;
			n[1] *= d;
			n[2] *= d;
		}	
	}


	return true;
}

bool rcMeshLoaderObj::CreateBuffers( void )
{
	return true;
}

void rcMeshLoaderObj::Draw( void )
{

}
