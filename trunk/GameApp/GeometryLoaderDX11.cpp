#include "stdafx.h"

//--------------------------------------------------------------------------------
// GeometryLoaderDX11
//
// Copyright (C) 2003-2009 Jason Zink. All rights reserved.
//--------------------------------------------------------------------------------
#include "GeometryLoaderDX11.h"
#include "VertexElementDX11.h"
#include "ms3dspec.h"
#include <EngineCore/Math/Vector.h>
#include <fstream>
#include <sstream>
//--------------------------------------------------------------------------------
GeometryLoaderDX11::GeometryLoaderDX11( )
{
}
//--------------------------------------------------------------------------------
GeometryDX11* GeometryLoaderDX11::loadMS3DFile2( std::string filename )
{
	// Temporary Milkshape structures
	unsigned short usVertexCount = 0;
	unsigned short usTriangleCount = 0;
	unsigned short usGroupCount = 0;
	unsigned short usMaterialCount = 0;
	MS3DVertex* pMS3DVertices = NULL;
	MS3DTriangle* pMS3DTriangles = NULL;
	MS3DGroup* pMS3DGroups = NULL;
	MS3DMaterial* pMS3DMaterials = NULL;

	int i;
	std::ifstream fin;
	MS3DHeader header;

	// Open the file and read the MS3D header data
	fin.open( filename.c_str(),std::ios::binary );
	fin.read((char*)(&(header.id)), sizeof(header.id));
	fin.read((char*)(&(header.version)), sizeof(header.version));
	if (header.version!=3 && header.version!=4)
		return NULL;

	// Load all the vertices
	fin.read((char*)(&usVertexCount), sizeof(unsigned short));
	pMS3DVertices = new MS3DVertex[usVertexCount];
	for (i=0; i < usVertexCount; i++)
	{
		fin.read((char*)&(pMS3DVertices[i].flags), sizeof(unsigned char));
		fin.read((char*)&(pMS3DVertices[i].vertex[0]), sizeof(float));
		fin.read((char*)&(pMS3DVertices[i].vertex[1]), sizeof(float));
		fin.read((char*)&(pMS3DVertices[i].vertex[2]), sizeof(float));
		fin.read((char*)&(pMS3DVertices[i].boneId), sizeof(char));
		fin.read((char*)&(pMS3DVertices[i].referenceCount), sizeof(unsigned char));
	}

	// Load all the triangle indices
	fin.read((char*)(&usTriangleCount), sizeof(unsigned short));
	pMS3DTriangles = new MS3DTriangle[usTriangleCount];
	for (i=0; i < usTriangleCount; i++)
	{
		fin.read((char*)&(pMS3DTriangles[i].flags),sizeof(unsigned short));
		fin.read((char*)&(pMS3DTriangles[i].vertexIndices[0]), sizeof(unsigned short)); //3*sizeof(unsigned short));
		fin.read((char*)&(pMS3DTriangles[i].vertexIndices[1]), sizeof(unsigned short)); //3*sizeof(unsigned short));
		fin.read((char*)&(pMS3DTriangles[i].vertexIndices[2]), sizeof(unsigned short)); //3*sizeof(unsigned short));
		fin.read((char*)&(pMS3DTriangles[i].vertexNormals[0]), 3*sizeof(float));
		fin.read((char*)&(pMS3DTriangles[i].vertexNormals[1]), 3*sizeof(float));
		fin.read((char*)&(pMS3DTriangles[i].vertexNormals[2]), 3*sizeof(float));
		fin.read((char*)&(pMS3DTriangles[i].s), 3*sizeof(float));
		fin.read((char*)&(pMS3DTriangles[i].t), 3*sizeof(float));
		fin.read((char*)&(pMS3DTriangles[i].smoothingGroup), sizeof(unsigned char));
		fin.read((char*)&(pMS3DTriangles[i].groupIndex), sizeof(unsigned char));
	}

	// Load all the group information
	fin.read((char*)(&usGroupCount), sizeof(unsigned short));
	pMS3DGroups = new MS3DGroup[usGroupCount];
	for (i=0; i < usGroupCount; i++)
	{
		fin.read((char*)&(pMS3DGroups[i].flags), sizeof(unsigned char));
		fin.read((char*)&(pMS3DGroups[i].name), sizeof(char[32]));
		fin.read((char*)&(pMS3DGroups[i].numtriangles), sizeof(unsigned short));
		unsigned short triCount = pMS3DGroups[i].numtriangles;
		pMS3DGroups[i].triangleIndices = new unsigned short[triCount];
		fin.read((char*)(pMS3DGroups[i].triangleIndices), sizeof(unsigned short) * triCount);
		fin.read((char*)&(pMS3DGroups[i].materialIndex), sizeof(char));
	}

	// Load all the material information
	fin.read((char*)(&usMaterialCount),sizeof(unsigned short));
	pMS3DMaterials = new MS3DMaterial[usMaterialCount];
	for (i=0; i < usMaterialCount; i++)
	{
		fin.read((char*)&(pMS3DMaterials[i].name), sizeof(char[32]));
		fin.read((char*)&(pMS3DMaterials[i].ambient), 4 * sizeof(float));
		fin.read((char*)&(pMS3DMaterials[i].diffuse), 4 * sizeof(float));
		fin.read((char*)&(pMS3DMaterials[i].specular), 4 * sizeof(float));
		fin.read((char*)&(pMS3DMaterials[i].emissive), 4 * sizeof(float));
		fin.read((char*)&(pMS3DMaterials[i].shininess), sizeof(float));
		fin.read((char*)&(pMS3DMaterials[i].transparency), sizeof(float));
		fin.read((char*)&(pMS3DMaterials[i].mode), sizeof(char));
		fin.read((char*)&(pMS3DMaterials[i].texture), sizeof(char[128]));
		fin.read((char*)&(pMS3DMaterials[i].alphamap), sizeof(char[128]));
	}

	// Close the file (remaining file data unused)
	fin.close();


	// create the vertex element streams
	VertexElementDX11* pPositions = new VertexElementDX11( 3, usTriangleCount*3 );
	pPositions->m_SemanticName = "POSITION";
	pPositions->m_uiSemanticIndex = 0;
	pPositions->m_Format = DXGI_FORMAT_R32G32B32_FLOAT;
	pPositions->m_uiInputSlot = 0;
	pPositions->m_uiAlignedByteOffset = 0;
	pPositions->m_InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	pPositions->m_uiInstanceDataStepRate = 0;

	VertexElementDX11* pTexcoords = new VertexElementDX11( 2, usTriangleCount*3 );
	pTexcoords->m_SemanticName = "TEXCOORDS";
	pTexcoords->m_uiSemanticIndex = 0;
	pTexcoords->m_Format = DXGI_FORMAT_R32G32_FLOAT;
	pTexcoords->m_uiInputSlot = 0;
	pTexcoords->m_uiAlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	pTexcoords->m_InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	pTexcoords->m_uiInstanceDataStepRate = 0;

	VertexElementDX11* pNormals = new VertexElementDX11( 3, usTriangleCount*3 );
	pNormals->m_SemanticName = "NORMAL";
	pNormals->m_uiSemanticIndex = 0;
	pNormals->m_Format = DXGI_FORMAT_R32G32B32_FLOAT;
	pNormals->m_uiInputSlot = 0;
	pNormals->m_uiAlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	pNormals->m_InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	pNormals->m_uiInstanceDataStepRate = 0;

	noVec3* pPos = (noVec3*)((*pPositions)[0]);
	noVec3* pNrm = (noVec3*)((*pNormals)[0]);
	noVec2* pTex = (noVec2*)((*pTexcoords)[0]);

	GeometryDX11* pMesh = new GeometryDX11();

	//for ( int i = 0; i < usVertexCount; i++ )
	//{
	//	pPos[i].x = pMS3DVertices[i].vertex[0];
	//	pPos[i].y = pMS3DVertices[i].vertex[1];
	//	pPos[i].z = -pMS3DVertices[i].vertex[2];
	//	pNrm[i].MakeZero();
	//}

	CTriangleIndices face;

	for ( int i = 0; i < usTriangleCount; i++ )
	{

		face.P1() = 3*i+0;
		face.P2() = 3*i+2;
		face.P3() = 3*i+1;

		pPos[3*i+0].x = pMS3DVertices[pMS3DTriangles[i].vertexIndices[0]].vertex[0];
		pPos[3*i+0].y = pMS3DVertices[pMS3DTriangles[i].vertexIndices[0]].vertex[1];
		pPos[3*i+0].z = -pMS3DVertices[pMS3DTriangles[i].vertexIndices[0]].vertex[2];
		pPos[3*i+1].x = pMS3DVertices[pMS3DTriangles[i].vertexIndices[1]].vertex[0];
		pPos[3*i+1].y = pMS3DVertices[pMS3DTriangles[i].vertexIndices[1]].vertex[1];
		pPos[3*i+1].z = -pMS3DVertices[pMS3DTriangles[i].vertexIndices[1]].vertex[2];
		pPos[3*i+2].x = pMS3DVertices[pMS3DTriangles[i].vertexIndices[2]].vertex[0];
		pPos[3*i+2].y = pMS3DVertices[pMS3DTriangles[i].vertexIndices[2]].vertex[1];
		pPos[3*i+2].z = -pMS3DVertices[pMS3DTriangles[i].vertexIndices[2]].vertex[2];

		pNrm[3*i+0].x = pMS3DTriangles[i].vertexNormals[0][0];
		pNrm[3*i+0].y = pMS3DTriangles[i].vertexNormals[0][1];
		pNrm[3*i+0].z = -pMS3DTriangles[i].vertexNormals[0][2];
		pNrm[3*i+1].x = pMS3DTriangles[i].vertexNormals[1][0];
		pNrm[3*i+1].y = pMS3DTriangles[i].vertexNormals[1][1];
		pNrm[3*i+1].z = -pMS3DTriangles[i].vertexNormals[1][2];
		pNrm[3*i+2].x = pMS3DTriangles[i].vertexNormals[2][0];
		pNrm[3*i+2].y = pMS3DTriangles[i].vertexNormals[2][1];
		pNrm[3*i+2].z = -pMS3DTriangles[i].vertexNormals[2][2];

		pTex[3*i+0].x = pMS3DTriangles[i].s[0];
		pTex[3*i+0].y = pMS3DTriangles[i].t[0];
		pTex[3*i+1].x = pMS3DTriangles[i].s[1];
		pTex[3*i+1].y = pMS3DTriangles[i].t[1];
		pTex[3*i+2].x = pMS3DTriangles[i].s[2];
		pTex[3*i+2].y = pMS3DTriangles[i].t[2];

		pMesh->AddFace( face );
	}

	for ( int i = 0; i < usVertexCount; i++ )
	{
		pNrm[i].Normalize();
	}

	pMesh->AddElement( pPositions );
	pMesh->AddElement( pTexcoords );
	pMesh->AddElement( pNormals );

	// Delete temporary materials
	if (pMS3DMaterials != NULL)
	{
		delete[] pMS3DMaterials;
		pMS3DMaterials = NULL;
	}

	// Delete temporary groups and their indices
	if (pMS3DGroups != NULL)
	{
		for (i = 0; i < usGroupCount; i++)
		{
			if (pMS3DGroups[i].triangleIndices != NULL)
			{
				delete[] pMS3DGroups[i].triangleIndices;
				pMS3DGroups[i].triangleIndices = NULL;
			}
		}
		delete[] pMS3DGroups;
		pMS3DGroups = NULL;
	}

	// Delete temporary triangles
	if (pMS3DTriangles != NULL)
	{
		delete[] pMS3DTriangles;
		pMS3DTriangles = NULL;
	}

	// Delete temporary vertices
	if (pMS3DVertices != NULL)
	{
        delete[] pMS3DVertices;
		pMS3DVertices = NULL;
	}

	return( pMesh );
}
//--------------------------------------------------------------------------------