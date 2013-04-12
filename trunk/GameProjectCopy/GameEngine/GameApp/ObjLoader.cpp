#include "stdafx.h"
#include "EngineCore/Util/Array.h"

#include "Mesh.h"
#include <ModelLib/M2Loader.h>
#include "SceneNode.h"

#include "ObjLoader.h"
#include "GameApp.h"

#include <fstream>
using namespace std;
#pragma warning(default: 4995)




// Define the input layout
const D3D10_INPUT_ELEMENT_DESC layout_CMeshLoader10[] =
{
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D10_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D10_INPUT_PER_VERTEX_DATA, 0 },
} ;
UINT numElements_layout_CMeshLoader10 = sizeof( layout_CMeshLoader10 ) / sizeof( layout_CMeshLoader10[0] );


//--------------------------------------------------------------------------------------
CMeshLoader10::CMeshLoader10()
{
	m_pd3dDevice = NULL;
	m_pMesh = NULL;

	m_NumAttribTableEntries = 0;
	//m_pAttribTable = NULL;

	ZeroMemory( m_strMediaDir, sizeof( m_strMediaDir ) );
}


//--------------------------------------------------------------------------------------
CMeshLoader10::~CMeshLoader10()
{
	Destroy();
}


//--------------------------------------------------------------------------------------
void CMeshLoader10::Destroy()
{
	for ( int iMaterial = 0; iMaterial < m_Materials.GetSize(); ++iMaterial )
	{
		Material *pMaterial = m_Materials.GetAt( iMaterial );

		if ( pMaterial->pTextureRV10 && !IsErrorResource(pMaterial->pTextureRV10) )
		{
			ID3D11Resource* pRes = NULL;

			pMaterial->pTextureRV10->GetResource( &pRes );
			SAFE_RELEASE( pRes );
			SAFE_RELEASE( pRes );   // do this twice, because GetResource adds a ref

			SAFE_RELEASE( pMaterial->pTextureRV10 );
		}

		SAFE_DELETE( pMaterial );
	}

	m_Materials.RemoveAll();
	m_Vertices.RemoveAll();
	m_Indices.RemoveAll();
	m_Attributes.RemoveAll();

	//SAFE_DELETE_ARRAY( m_pAttribTable );
	m_NumAttribTableEntries = 0;

	//SAFE_RELEASE( m_pMesh );
	m_pd3dDevice = NULL;
}


//--------------------------------------------------------------------------------------
HRESULT CMeshLoader10::Create( ID3D11Device* pd3dDevice, const CHAR* strFilename )
{
	

	HRESULT hr;
	WCHAR str[ MAX_PATH ] = {0};

	// Start clean
	Destroy();

	// Store the device pointer
	m_pd3dDevice = pd3dDevice;

	// Load the vertex buffer, index buffer, and subset information from a file. In this case, 
	// an .obj file was chosen for simplicity, but it's meant to illustrate that ID3DXMesh objects
	// can be filled from any mesh file format once the necessary data is extracted from file.
	( LoadGeometryFromOBJ( strFilename ) );

	// Set the current directory based on where the mesh was found
	CHAR strOldDir[MAX_PATH] = {0};
		
	//WideCharToMultiByte( CP_ACP, 0, strFilename, -1, strOldDir, MAX_PATH, NULL, FALSE );
	_tcscpy_s( strOldDir, MAX_PATH - 1, strFilename );
	GetCurrentDirectory( MAX_PATH, strOldDir );
	SetCurrentDirectory( m_strMediaDir );    

	// Load material textures
	for ( int iMaterial = 0; iMaterial < m_Materials.GetSize(); ++iMaterial )
	{
		Material *pMaterial = m_Materials.GetAt( iMaterial );
		if ( pMaterial->strTexture[0] )
		{            
			pMaterial->pTextureRV10 = (ID3D11ShaderResourceView*)ERROR_RESOURCE_VALUE;
			
		}
	}

	// Restore the original current directory
	SetCurrentDirectory( strOldDir );

	// Create the encapsulated mesh
//	ID3DX10Mesh *pMesh = NULL;
/*

	( D3DX10CreateMesh( pd3dDevice,
		layout_CMeshLoader10,
		numElements_layout_CMeshLoader10,
		layout_CMeshLoader10[0].SemanticName,
		m_Vertices.GetSize(),
		m_Indices.GetSize() / 3,
		D3DX10_MESH_32_BIT,
		&pMesh ) );*/

	//// Set the vertex data
	//pMesh->SetVertexData( 0, (void*)m_Vertices.GetData() );
	//m_Vertices.RemoveAll();

	//// Set the index data
	//pMesh->SetIndexData( (void*)m_Indices.GetData(), m_Indices.GetSize() );
	//m_Indices.RemoveAll();

	//// Set the attribute data
	//pMesh->SetAttributeData( (UINT*)m_Attributes.GetData() );
	//m_Attributes.RemoveAll();

	//// Reorder the vertices according to subset and optimize the mesh for this graphics 
	//// card's vertex cache. When rendering the mesh's triangle list the vertices will 
	//// cache hit more often so it won't have to re-execute the vertex shader.
	//V( pMesh->GenerateAdjacencyAndPointReps( 1e-6f ) );
	//V( pMesh->Optimize( D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE, NULL, NULL ) );

	//pMesh->GetAttributeTable( NULL, &m_NumAttribTableEntries );
	//m_pAttribTable = new D3DX10_ATTRIBUTE_RANGE[m_NumAttribTableEntries];
	//pMesh->GetAttributeTable( m_pAttribTable, &m_NumAttribTableEntries );

	//V( pMesh->CommitToDevice() );

	//m_pMesh = pMesh;
	FormatDesc vbFmt[] = {
		0, TYPE_VERTEX, FORMAT_FLOAT,  3,
		0, TYPE_NORMAL, FORMAT_FLOAT,   3,
		0, TYPE_TANGENT, FORMAT_FLOAT, 3,
		0, TYPE_BINORMAL,FORMAT_FLOAT, 3,
		0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,				
	};

	rd.shader = gColor;

	rd.vf = gRenderer->addVertexFormat(vbFmt, elementsOf(vbFmt), rd.shader);

	rd.vb = gRenderer->addVertexBuffer(sizeof(VERTEX) * m_Vertices.GetSize(), STATIC, m_Vertices.GetData());
	rd.ib = gRenderer->addIndexBuffer(m_Indices.GetSize(), sizeof(DWORD), STATIC, m_Indices.GetData());



	rd.startIndex = 0;
	rd.endIndex = m_Indices.GetSize();
	rd.vertexStart = 0;
	rd.vertexEnd = m_Vertices.GetSize();
	//rd.cull = gRenderer->addRasterizerState(CULL_BACK);
	rd.depthMode = gRenderer->addDepthState(true, true);
	

	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CMeshLoader10::LoadGeometryFromOBJ( const CHAR* strFileName )
{
	WCHAR strMaterialFilename[MAX_PATH] = {0};	
	char str[MAX_PATH];
	HRESULT hr;

	// Find the file
	//WideCharToMultiByte( CP_ACP, 0, strFileName, -1, str, MAX_PATH, NULL, NULL );

	// Store the directory where the mesh was found
	_tcscpy_s( m_strMediaDir, MAX_PATH - 1, strFileName );
	CHAR* pch = _tcsrchr( m_strMediaDir, '\\' );
	if( pch )
		*pch = NULL;

	// Create temporary storage for the input data. Once the data has been loaded into
	// a reasonable format we can create a D3DXMesh object and load it with the mesh data.
	CGrowableArray <noVec3> Positions;
	CGrowableArray <D3DXVECTOR2> TexCoords;
	CGrowableArray <noVec3> Normals;

	// The first subset uses the default material
	Material* pMaterial = new Material();
	if( pMaterial == NULL )
		return E_OUTOFMEMORY;

	InitMaterial( pMaterial );
	wcscpy_s( pMaterial->strName, MAX_PATH - 1, L"default" );
	m_Materials.Add( pMaterial );

	DWORD dwCurSubset = 0;

	// File input
	WCHAR strCommand[256] = {0};
	wifstream InFile( strFileName );
	if( !InFile )
		return DXTRACE_ERR( "wifstream::open", E_FAIL );

	for(; ; )
	{
		InFile >> strCommand;
		if( !InFile )
			break;

		if( 0 == wcscmp( strCommand, L"#" ) )
		{
			// Comment
		}
		else if( 0 == wcscmp( strCommand, L"v" ) )
		{
			// Vertex Position
			float x, y, z;
			InFile >> x >> y >> z;
			Positions.Add( localTM_ * noVec3( x, y, -z ) );
		}
		else if( 0 == wcscmp( strCommand, L"vt" ) )
		{
			// Vertex TexCoord
			float u, v;
			InFile >> u >> v;
			TexCoords.Add( D3DXVECTOR2( u, v ) );
		}
		else if( 0 == wcscmp( strCommand, L"vn" ) )
		{
			// Vertex Normal
			float x, y, z;
			InFile >> x >> y >> z;
			Normals.Add( noVec3( x, y, z ) );
		}
		else if( 0 == wcscmp( strCommand, L"f" ) )
		{
			// Face
			UINT iPosition, iTexCoord, iNormal;
			VERTEX vertex;

			for( UINT iFace = 0; iFace < 3; iFace++ )
			{
				ZeroMemory( &vertex, sizeof( VERTEX ) );

				// OBJ format uses 1-based arrays
				InFile >> iPosition;
				vertex.position = Positions[ iPosition - 1 ];
				
				if( '/' == InFile.peek() )
				{
					InFile.ignore();

					if( '/' != InFile.peek() )
					{
						// Optional texture coordinate
						InFile >> iTexCoord;
						vertex.texcoord = TexCoords[ iTexCoord - 1 ];
					}

					if( '/' == InFile.peek() )
					{
						InFile.ignore();

						// Optional vertex normal
						InFile >> iNormal;
						vertex.normal = Normals[ iNormal - 1 ];
					}
				}

				// If a duplicate vertex doesn't exist, add this vertex to the Vertices
				// list. Store the index in the Indices array. The Vertices and Indices
				// lists will eventually become the Vertex Buffer and Index Buffer for
				// the mesh.
				DWORD index = AddVertex( iPosition, &vertex );
				if ( index == (DWORD)-1 )
					return E_OUTOFMEMORY;

				m_Indices.Add( index );
			}
			m_Attributes.Add( dwCurSubset );
		}
		else if( 0 == wcscmp( strCommand, L"mtllib" ) )
		{
			// Material library
			InFile >> strMaterialFilename;
		}
		else if( 0 == wcscmp( strCommand, L"usemtl" ) )
		{
			// Material
			WCHAR strName[MAX_PATH] = {0};
			InFile >> strName;

			bool bFound = false;
			for( int iMaterial = 0; iMaterial < m_Materials.GetSize(); iMaterial++ )
			{
				Material* pCurMaterial = m_Materials.GetAt( iMaterial );
				if( 0 == wcscmp( pCurMaterial->strName, strName ) )
				{
					bFound = true;
					dwCurSubset = iMaterial;
					break;
				}
			}

			if( !bFound )
			{
				pMaterial = new Material();
				if( pMaterial == NULL )
					return E_OUTOFMEMORY;

				dwCurSubset = m_Materials.GetSize();

				InitMaterial( pMaterial );
				wcscpy_s( pMaterial->strName, MAX_PATH - 1, strName );

				m_Materials.Add( pMaterial );
			}
		}
		else
		{
			// Unimplemented or unrecognized command
		}

		InFile.ignore( 1000, '\n' );
	}

	// Cleanup
	InFile.close();
	DeleteCache();

	// If an associated material file was found, read that in as well.
	if( strMaterialFilename[0] )
	{
		( LoadMaterialsFromMTL( strMaterialFilename ) );
	}

	return S_OK;
}


//--------------------------------------------------------------------------------------
DWORD CMeshLoader10::AddVertex( UINT hash, VERTEX* pVertex )
{
	// If this vertex doesn't already exist in the Vertices list, create a new entry.
	// Add the index of the vertex to the Indices list.
	bool bFoundInList = false;
	DWORD index = 0;

	// Since it's very slow to check every element in the vertex list, a hashtable stores
	// vertex indices according to the vertex position's index as reported by the OBJ file
	if( ( UINT )m_VertexCache.GetSize() > hash )
	{
		CacheEntry* pEntry = m_VertexCache.GetAt( hash );
		while( pEntry != NULL )
		{
			VERTEX* pCacheVertex = m_Vertices.GetData() + pEntry->index;

			// If this vertex is identical to the vertex already in the list, simply
			// point the index buffer to the existing vertex
			if( 0 == memcmp( pVertex, pCacheVertex, sizeof( noVec3) ) )
			{
				bFoundInList = true;
				index = pEntry->index;
				break;
			}

			pEntry = pEntry->pNext;
		}
	}

	// Vertex was not found in the list. Create a new entry, both within the Vertices list
	// and also within the hashtable cache
	if( !bFoundInList )
	{
		// Add to the Vertices list
		index = m_Vertices.GetSize();
		m_Vertices.Add( *pVertex );

		// Add this to the hashtable
		CacheEntry* pNewEntry = new CacheEntry;
		if( pNewEntry == NULL )
			return (DWORD)-1;

		pNewEntry->index = index;
		pNewEntry->pNext = NULL;

		// Grow the cache if needed
		while( ( UINT )m_VertexCache.GetSize() <= hash )
		{
			m_VertexCache.Add( NULL );
		}

		// Add to the end of the linked list
		CacheEntry* pCurEntry = m_VertexCache.GetAt( hash );
		if( pCurEntry == NULL )
		{
			// This is the head element
			m_VertexCache.SetAt( hash, pNewEntry );
		}
		else
		{
			// Find the tail
			while( pCurEntry->pNext != NULL )
			{
				pCurEntry = pCurEntry->pNext;
			}

			pCurEntry->pNext = pNewEntry;
		}
	}

	return index;
}


//--------------------------------------------------------------------------------------
void CMeshLoader10::DeleteCache()
{
	// Iterate through all the elements in the cache and subsequent linked lists
	for( int i = 0; i < m_VertexCache.GetSize(); i++ )
	{
		CacheEntry* pEntry = m_VertexCache.GetAt( i );
		while( pEntry != NULL )
		{
			CacheEntry* pNext = pEntry->pNext;
			SAFE_DELETE( pEntry );
			pEntry = pNext;
		}
	}

	m_VertexCache.RemoveAll();
}


//--------------------------------------------------------------------------------------
HRESULT CMeshLoader10::LoadMaterialsFromMTL( const WCHAR* strFileName )
{
	HRESULT hr;

	// Set the current directory based on where the mesh was found
	CHAR strOldDir[MAX_PATH] = {0};	
	WideCharToMultiByte( CP_ACP, 0, strFileName, -1, strOldDir, MAX_PATH, NULL, FALSE );	

	GetCurrentDirectory( MAX_PATH, strOldDir );
	SetCurrentDirectory( m_strMediaDir );

	// Find the file
	char cstrPath[MAX_PATH];

	WideCharToMultiByte( CP_ACP, 0, strFileName, -1, cstrPath, MAX_PATH, NULL, NULL );

	// File input
	WCHAR strCommand[256] = {0};
	wifstream InFile( cstrPath );
	if( !InFile )
		return DXTRACE_ERR( "wifstream::open", E_FAIL );

	// Restore the original current directory
	SetCurrentDirectory( strOldDir );

	Material* pMaterial = NULL;

	for(; ; )
	{
		InFile >> strCommand;
		if( !InFile )
			break;

		if( 0 == wcscmp( strCommand, L"newmtl" ) )
		{
			// Switching active materials
			WCHAR strName[MAX_PATH] = {0};
			InFile >> strName;

			pMaterial = NULL;
			for( int i = 0; i < m_Materials.GetSize(); i++ )
			{
				Material* pCurMaterial = m_Materials.GetAt( i );
				if( 0 == wcscmp( pCurMaterial->strName, strName ) )
				{
					pMaterial = pCurMaterial;
					break;
				}
			}
		}

		// The rest of the commands rely on an active material
		if( pMaterial == NULL )
			continue;

		if( 0 == wcscmp( strCommand, L"#" ) )
		{
			// Comment
		}
		else if( 0 == wcscmp( strCommand, L"Ka" ) )
		{
			// Ambient color
			float r, g, b;
			InFile >> r >> g >> b;
			pMaterial->vAmbient = noVec3( r, g, b );
		}
		else if( 0 == wcscmp( strCommand, L"Kd" ) )
		{
			// Diffuse color
			float r, g, b;
			InFile >> r >> g >> b;
			pMaterial->vDiffuse = noVec3( r, g, b );
		}
		else if( 0 == wcscmp( strCommand, L"Ks" ) )
		{
			// Specular color
			float r, g, b;
			InFile >> r >> g >> b;
			pMaterial->vSpecular = noVec3( r, g, b );
		}
		else if( 0 == wcscmp( strCommand, L"d" ) ||
			0 == wcscmp( strCommand, L"Tr" ) )
		{
			// Alpha
			InFile >> pMaterial->fAlpha;
		}
		else if( 0 == wcscmp( strCommand, L"Ns" ) )
		{
			// Shininess
			int nShininess;
			InFile >> nShininess;
			pMaterial->nShininess = nShininess;
		}
		else if( 0 == wcscmp( strCommand, L"illum" ) )
		{
			// Specular on/off
			int illumination;
			InFile >> illumination;
			pMaterial->bSpecular = ( illumination == 2 );
		}
		else if( 0 == wcscmp( strCommand, L"map_Kd" ) )
		{
			// Texture
			InFile >> pMaterial->strTexture;
		}

		else
		{
			// Unimplemented or unrecognized command
		}

		InFile.ignore( 1000, L'\n' );
	}

	InFile.close();

	return S_OK;
}


//--------------------------------------------------------------------------------------
void CMeshLoader10::InitMaterial( Material* pMaterial )
{
	ZeroMemory( pMaterial, sizeof( Material ) );

	pMaterial->vAmbient = noVec3( 0.2f, 0.2f, 0.2f );
	pMaterial->vDiffuse = noVec3( 0.8f, 0.8f, 0.8f );
	pMaterial->vSpecular = noVec3( 1.0f, 1.0f, 1.0f );
	pMaterial->nShininess = 0;
	pMaterial->fAlpha = 1.0f;
	pMaterial->bSpecular = false;
	pMaterial->pTextureRV10 = NULL;
}

