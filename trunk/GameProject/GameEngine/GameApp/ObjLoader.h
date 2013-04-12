//--------------------------------------------------------------------------------------
// File: MeshLoader10.h
//
// Wrapper class for ID3DX10Mesh interface. Handles loading mesh data from an .obj file
// and resource management for material textures.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#ifndef _MESHLOADER10_H_
#define _MESHLOADER10_H_
#pragma once

class Mesh;

#define ERROR_RESOURCE_VALUE 1

template<typename TYPE> BOOL IsErrorResource( TYPE data )
{
	if( ( TYPE )ERROR_RESOURCE_VALUE == data )
		return TRUE;
	return FALSE;
}

// Vertex format
struct VERTEX
{
	noVec3 position;
	noVec3 normal;	
	D3DXVECTOR2 texcoord;	
};


// Used for a hashtable vertex cache when creating the mesh from a .obj file
struct CacheEntry
{
	UINT index;
	CacheEntry* pNext;
};


// Material properties per mesh subset
struct Material
{
	WCHAR   strName[MAX_PATH];

	noVec3 vAmbient;
	noVec3 vDiffuse;
	noVec3 vSpecular;

	int nShininess;
	float fAlpha;

	bool bSpecular;

	WCHAR   strTexture[MAX_PATH];
	ID3D11ShaderResourceView* pTextureRV10;
	ID3DX11EffectTechnique*  pTechnique;
};


class CMeshLoader10 : public SceneNode
{
public:
	CMeshLoader10();
	~CMeshLoader10();

	HRESULT Create( ID3D11Device* pd3dDevice, const CHAR* strFilename );
	void    Destroy();


	UINT    GetNumMaterials() const
	{
		return m_Materials.GetSize();
	}
	Material* GetMaterial( UINT iMaterial )
	{
		return m_Materials.GetAt( iMaterial );
	}

	UINT GetNumSubsets()
	{
		return m_NumAttribTableEntries;
	}
	/*Material* GetSubsetMaterial( UINT iSubset )
	{
		return m_Materials.GetAt( m_pAttribTable[iSubset].AttribId );
	}*/

	Mesh* GetMesh()
	{
		return m_pMesh;
	}
	CHAR* GetMediaDirectory()
	{
		return m_strMediaDir;
	}

	const VERTEX* GetVertices() const { return m_Vertices.GetData(); }
	VERTEX* GetVertices() { return m_Vertices.GetData(); }
	const DWORD* GetIndices()	{ return m_Indices.GetData(); }
	int GetVertexCount() { return m_Vertices.GetSize(); }
	int GetIndexCount() { return m_Indices.GetSize(); }

	RenderData rd;

private:

	HRESULT LoadGeometryFromOBJ( const CHAR* strFilename );
	HRESULT LoadMaterialsFromMTL( const WCHAR* strFileName );
	void    InitMaterial( Material* pMaterial );

	DWORD   AddVertex( UINT hash, VERTEX* pVertex );
	void    DeleteCache();

	ID3D11Device* m_pd3dDevice;    // Direct3D Device object associated with this mesh
	Mesh* m_pMesh;         // Encapsulated D3DX Mesh

	CGrowableArray <CacheEntry*> m_VertexCache;   // Hashtable cache for locating duplicate vertices
	CGrowableArray <VERTEX> m_Vertices;      // Filled and copied to the vertex buffer
	CGrowableArray <DWORD> m_Indices;       // Filled and copied to the index buffer
	CGrowableArray <DWORD> m_Attributes;    // Filled and copied to the attribute buffer
	CGrowableArray <Material*> m_Materials;     // Holds material properties per subset

	UINT        m_NumAttribTableEntries;
//	D3DX10_ATTRIBUTE_RANGE *m_pAttribTable;

	CHAR   m_strMediaDir[ MAX_PATH ];               // Directory where the mesh was found
};

#endif // _MESHLOADER_H_

