   //  
  // Cached Procedural Textures (see ShaderX4 for details)
 // (w)(c)2005 Carsten Dachsbacher
//

#include "stdafx.h"
#include "quadtree.h"
#include "lod_dx11.h"
#include "frustum.h"
#include "GameApp/GameApp.h"

int		heightMapX,			// resolution of the height map used as geometry
		heightMapY,
		heightMapXFull,		// full resolution of the height map (stored as texture)
		heightMapYFull;

// the maximum (virtual) texture resolution of the terrain is:
// [(heightMapX/quadsPerTile)*MAX_TILE_TEXTURE_SIZE]?

// the terrain is subdivided into smaller chunks. the smallest size is quadsPerTile*quadsPerTile heixels
int		quadsPerTile = 8;

// scaling of the input terrain
static	float	hmEdgeLength = 0.018f;
float	terrainMaxElevation = 1000.0f*2.0f, terrainExtend = 7650.0f * 2.0f;

// this is the data for all terrain chunks
TERRAINTILE	**terrainQT = NULL;
TERRAINTILE	**terrainQTRender = NULL; // and a list of the chunks to be rendered
int			qlodOffset[ 16 ], qlodNode[ 16 ], qlodNodes, qlodMaxLevel, qlodToRender;

// the texture atlas
ATLAS	atlas[ ATLAS_TEXTURES ];


bool TERRAINTILE::is16BitBuffer = true;

ID3D11InputLayout* TERRAINTILE::m_pInputLayout;

void TERRAINTILE::CreateInputLayout(ID3D11Device* pd3dDevice, ID3DX11EffectTechnique* pTech) {
	HRESULT hr;

	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 8,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },		
	};
	const size_t N_ELEMS = sizeof(layout) / sizeof(D3D11_INPUT_ELEMENT_DESC);

	assert(pTech != NULL);	// Load this first.
	D3DX11_PASS_DESC passDesc;
	pTech->GetPassByIndex(0)->GetDesc(&passDesc);

	hr = pd3dDevice->CreateInputLayout(layout, N_ELEMS, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_pInputLayout);

	if (FAILED(hr)) assert(false);

}

bool TERRAINTILE::CreateVertexBuffer(const void* pVertexData, unsigned int uiNumVertices, unsigned int uiVertexSize)
{
	bool bSuccess = false;

	if (uiNumVertices > 0 && uiVertexSize > 0)
	{
		assert(GetApp()->GetDevice());

		D3D11_BUFFER_DESC sBufferDesc;
		sBufferDesc.ByteWidth = uiNumVertices * uiVertexSize;
		sBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		sBufferDesc.MiscFlags = 0;
		//if (m_bDynamicVertexBuffer)
		{
			sBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			sBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		/*else
		{
			sBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			sBufferDesc.CPUAccessFlags = 0;
		}*/

		D3D11_SUBRESOURCE_DATA sInitData;
		if (pVertexData)
		{
			sInitData.pSysMem = pVertexData;

			if (GetApp()->GetDevice()->CreateBuffer(&sBufferDesc, &sInitData, &m_pVertexBuffer) == S_OK)
			{
				m_uiCurrentVertexBufferSize = uiNumVertices * uiVertexSize;
				bSuccess = true;
			}
		}
		else 
			if (GetApp()->GetDevice()->CreateBuffer(&sBufferDesc, NULL, &m_pVertexBuffer) == S_OK)
			{
				m_uiCurrentVertexBufferSize = uiNumVertices * uiVertexSize;
				bSuccess = true;
			}

	}

	return bSuccess;
}

bool TERRAINTILE::CreateIndexBuffer(const void* pIndexData, unsigned int uiNumIndices)
{
	bool bSuccess = false;

	//assert(pIndexData);
	assert(uiNumIndices > 0);
	assert(D3D11Dev());

	D3D11_BUFFER_DESC sBufferDesc;
	sBufferDesc.ByteWidth = uiNumIndices * 2;	// 32Bit
	sBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	sBufferDesc.MiscFlags = 0;
	if (pIndexData && uiNumIndices > 0)
	{	
		sBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		sBufferDesc.CPUAccessFlags = 0;
	}
	else if (uiNumIndices > 0) {
		sBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		sBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	if (pIndexData) {
		D3D11_SUBRESOURCE_DATA sInitData;
		sInitData.pSysMem = pIndexData;

		if (D3D11Dev( )->CreateBuffer(&sBufferDesc, &sInitData, &m_pIndexBuffer) == S_OK)
		{
			//FindMaxIndex(pIndexData, uiNumIndices);
			m_uiCurrentIndexBufferSize = uiNumIndices;
			bSuccess = true;
		}
	}
	else {
		if (D3D11Dev( )->CreateBuffer(&sBufferDesc, NULL, &m_pIndexBuffer) == S_OK) 		{			
			m_uiCurrentIndexBufferSize = uiNumIndices;
			bSuccess = true;
		}
	}
	
	return bSuccess;
}

float* TERRAINTILE::LockVB( int offset )
{

	D3D11_MAPPED_SUBRESOURCE ms;	
	D3D11_MAP flag = D3D11_MAP_WRITE_DISCARD;
	D3D11Context()->Map(m_pVertexBuffer, 0, flag, 0, &ms);	

	return (float*)(ms.pData) + offset;
}

void TERRAINTILE::UnlockVB( )
{
	D3D11Context()->Unmap(m_pVertexBuffer, 0);
}

void* TERRAINTILE::LockIB(  )
{

	D3D11_MAPPED_SUBRESOURCE ms;	
	D3D11_MAP flag = D3D11_MAP_WRITE_DISCARD;
	D3D11Context()->Map(m_pIndexBuffer, 0, flag, 0, &ms);	

	return ms.pData;
}

void NavDebugDraw::UnlockIB( )
{
	D3D11Context()->Unmap(m_pIndexBuffer, 0);
}

// recursive (quadtree) generation of the terrain-lod
void	buildQLOD( ID3D11Device* pD3DDevice, int node, int x, int y, int size, int level, WORD *heightMap16, D3D11Texture2D* heightMapTexture16 )
{
	int	i, j;

	if ( level >= qlodMaxLevel ) return;

	TERRAINTILE *tile = new TERRAINTILE;

	//
	// setup texture stuff
	//
	tile->pHeightMapTexture = heightMapTexture16;
	tile->coordHM.x = (float)x / (float)heightMapX;
	tile->coordHM.y = (float)y / (float)heightMapX;
	tile->coordHM.z = (float)size / (float)heightMapX;
	tile->coordHM.w = (float)size / (float)heightMapX;
	tile->tileSize  = 0;
	tile->tileIdx   = -1;
	tile->worldSize = size;

	// use this, to take full geometric resolution everytime and everywhere
	//quadsPerTile = size;

	//
	// create input geometry
	//	
	tile->sizePerVertex = 6 * sizeof( float );
	
	//creationFlags = 0;
	if ( quadsPerTile * quadsPerTile * 2 * 3 >= 65535 )
		TERRAINTILE::is16BitBuffer = false;

	float *pData = NULL;

	if (!tile->CreateVertexBuffer(NULL, ( quadsPerTile + 1+2 ) * ( quadsPerTile + 1+2 ), tile->sizePerVertex))
		assert(false);

	if (!tile->CreateIndexBuffer(NULL, (quadsPerTile+2) * (quadsPerTile+2) * 2) * 3 * 2))
		assert(false);
		
	HRESULT hr;
	pData = tile->LockVB( 0 );

	tile->center  = noVec3( 0.0f, 0.0f, 0.0f );
	tile->aabbMin = noVec3( +1e37f, +1e37f, +1e37f );
	tile->aabbMax = noVec3( -1e37f, -1e37f, -1e37f );

	int	step = size / quadsPerTile;

	for ( int jj = -1; jj < quadsPerTile + 1+1; jj++ )
		for ( int ii = -1; ii < quadsPerTile + 1+1; ii++ )
		{
			int idx = (ii+1) + (jj+1) * ( quadsPerTile + 1+2 );

			if ( idx >= ( quadsPerTile + 1+2 ) * ( quadsPerTile + 1+2 ) )
				idx = idx;

			int i = max( 0, min( quadsPerTile, ii ) );
			int j = max( 0, min( quadsPerTile, jj ) );

			float xc = (float)( ( i * step + x ) - heightMapX / 2 ) / (float)(heightMapX) * terrainExtend;
			float zc = (float)( ( j * step + y ) - heightMapX / 2 ) / (float)(heightMapX) * terrainExtend;

			pData[ idx * 6 + 0 ] = xc;
			pData[ idx * 6 + 1 ] = (float)heightMap16[ min( heightMapX - 1, ( i * step + x ) ) + 
												   	   min( heightMapX - 1, ( j * step + y ) ) * heightMapX ] * 0.1f / 2.677777f;//65535.0f * terrainMaxElevation;

			pData[ idx * 6 + 2 ] = zc;

			pData[ idx * 6 + 3 ] = (float)i / (float)( quadsPerTile );
			pData[ idx * 6 + 4 ] = (float)j / (float)( quadsPerTile );

			// poor mans chunk-lod skirt ;-)
			if ( ii != i || j != jj )
			{
				pData[ idx * 6 + 1 ] -= terrainMaxElevation * (float)size / (float)heightMapX * 0.25f;
				pData[ idx * 6 + 5 ] -= terrainMaxElevation * (float)size / (float)heightMapX * 0.25f;
			}

			if ( ii == i && j == jj )
				tile->center += noVec3( xc, pData[ idx * 6 + 1 ], zc );

			tile->aabbMin.Min( (noVec3*)&pData[ idx * 6 + 0 ] );
			tile->aabbMax.Max( (noVec3*)&pData[ idx * 6 + 0 ] );
		}

	tile->UnlockVB();

	tile->center /= (float)( ( quadsPerTile + 1 ) * ( quadsPerTile + 1 ) );

	void *pIndexBuffer;
	pIndexBuffer = tile->LockIB(  );

	if ( TERRAINTILE::is16BitBuffer == false)
	{
		DWORD *pIndex = (DWORD*)pIndexBuffer;

		for ( j = 0; j < quadsPerTile+2; j++ )
			for ( i = 0; i < quadsPerTile+2; i++ )
			{
				*( pIndex++ ) = i     + j     * ( quadsPerTile + 3 );
				*( pIndex++ ) = i     + (j+1) * ( quadsPerTile + 3 );
				*( pIndex++ ) = (i+1) + j     * ( quadsPerTile + 3 );
				*( pIndex++ ) = (i+1) + j     * ( quadsPerTile + 3 );
				*( pIndex++ ) = i     + (j+1) * ( quadsPerTile + 3 );
				*( pIndex++ ) = (i+1) + (j+1) * ( quadsPerTile + 3 );
			}
	} else
	{
		WORD *pIndex = (WORD*)pIndexBuffer;

		for ( j = 0; j < quadsPerTile+2; j++ )
			for ( i = 0; i < quadsPerTile+2; i++ )
			{
				*( pIndex++ ) = i     + j     * ( quadsPerTile + 3 );
				*( pIndex++ ) = i     + (j+1) * ( quadsPerTile + 3 );
				*( pIndex++ ) = (i+1) + j     * ( quadsPerTile + 3 );
				*( pIndex++ ) = (i+1) + j     * ( quadsPerTile + 3 );
				*( pIndex++ ) = i     + (j+1) * ( quadsPerTile + 3 );
				*( pIndex++ ) = (i+1) + (j+1) * ( quadsPerTile + 3 );
			}
	}
	tile->UnlockIB();
	
	terrainQT[ node ] = tile;

	buildQLOD( pD3DDevice, qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 0, x,        y,        size/2, level+1, heightMap16, heightMapTexture16 );
	buildQLOD( pD3DDevice, qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 1, x+size/2, y,        size/2, level+1, heightMap16, heightMapTexture16 );
	buildQLOD( pD3DDevice, qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 2, x,        y+size/2, size/2, level+1, heightMap16, heightMapTexture16 );
	buildQLOD( pD3DDevice, qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 3, x+size/2, y+size/2, size/2, level+1, heightMap16, heightMapTexture16 );
}


//
// create chunk lod quadtree
//
void	createQLOD( ID3D11Device* pD3DDevice, int heightMapSize, WORD *heightMap16, D3D11Texture2D* heightMapTexture16 )
{
	int i;

	qlodMaxLevel = (int)( logf( (float)heightMapX / (float)quadsPerTile ) / logf( 2.0f ) ) + 1;
	int	curOfs = 0, curNode = 1;
	qlodNode[ 0 ] = 1;
	qlodNodes = 1;
	qlodOffset[ 0 ] = 0;
	for ( i = 1; i < qlodMaxLevel; i++ )
	{
        qlodOffset[ i ] = qlodOffset[ i - 1 ] + qlodNode[ i - 1 ];	
		qlodNode[ i ] = qlodNode[ i - 1 ] * 4;
		qlodNodes += qlodNode[ i ];
	}

	terrainQT = new TERRAINTILE*[ qlodNodes ];
	terrainQTRender = new TERRAINTILE*[ qlodNodes ];
	qlodToRender = 0;

	buildQLOD( pD3DDevice, 0, 0, 0, heightMapSize-1, 0, heightMap16, heightMapTexture16 );

	for ( int i = 0; i < ATLAS_TEXTURES; i++ )
		atlas[ i ].layout = new FQTree();
}

//
// free memory
//
void	destroyQLOD()
{
	freeAllTexturesQLOD( 0, 0 );

	for ( int i = 0; i < qlodNodes; i++ )
	{
		SAFE_RELEASE( terrainQT[ i ]->pMesh );
		SAFE_DELETE ( terrainQT[ i ] );
	}

	for ( int i = 0; i < ATLAS_TEXTURES; i++ )
		SAFE_DELETE( atlas[ i ].layout );
}

//
// render all marked chunks (contained in terrainQTRender)
//
void	renderQLOD( ID3DX11Effect *pEffect )
{
	ID3D11ShaderResourceView*	curTexture = NULL;
	int					curTileSize = 0;


	for ( int a = 0; a < qlodToRender; a++ )
	{
		int commit = 0;

		TERRAINTILE *tile = terrainQTRender[ a ];

		if ( curTileSize != tile->tileSize )
		{
			curTileSize = tile->tileSize;
			noVec4	rtSize = noVec4( 0.0f, 0.0f, (float)tile->tileSize, 0.0f );
			pEffect->SetVector( "rtSize", &rtSize );
			commit ++;
		}

		if ( curTexture != tile->pTexture )
		{
			curTexture = tile->pTexture;
			//pEffect->GetVariableByName( "atlasTexture")->AsShaderResource()->SetResource(tile->pTexture );
			commit++;
		}
		pEffect->GetVariableByName( "texCoordBiasScale")->AsVector()->SetFloatVector(&tile->coordTile ); 

		
	}
}

//
// frees the texture tiles of a terrain chunk (and its sub-chunks)
//
void	freeAllTexturesQLOD( int node, int level )
{
	if ( level >= qlodMaxLevel ) return;

	TERRAINTILE *tile = terrainQT[ node ];

	if ( tile->tileIdx != - 1 )
	{
		int atlasIdx = tile->tileIdx >> 24;

		atlas[ atlasIdx ].layout->treeFree( tile->tileSize, tile->tileIdx & 0xffffff );

		tile->tileIdx  = -1;
		tile->tileSize = 0;
		tile->pTexture = NULL;
	}

	freeAllTexturesQLOD( qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 0, level+1 );
	freeAllTexturesQLOD( qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 1, level+1 );
	freeAllTexturesQLOD( qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 2, level+1 );
	freeAllTexturesQLOD( qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 3, level+1 );
}


// this functions determines, which terrain chunks are intersecting the view frustum and determines
// the required geometric and texture resolution
static int updateAmount;

void	traverseQLOD( ID3D11Device* pD3DDevice, noVec4 &camPos, noVec4 &camDst, int node, int level )
{
	if ( level >= qlodMaxLevel ) return;

	if ( level == 0 ) 
	{
		updateAmount = 0;

		qlodToRender = 0;
		for ( int i = 0; i < ATLAS_TEXTURES; i++ )
			atlas[ i ].count = 0;
	}

	TERRAINTILE *tile = terrainQT[ node ];

	// frustum test
	tile->intersectFrustum = boxIntersectsFrustum( &tile->aabbMin, &tile->aabbMax );

	if ( !tile->intersectFrustum )
	{
		freeAllTexturesQLOD( node, level );
		return;
	}

	//
	// compute tile size
	//

	// at first min and max distance of the bounding box of a chunk
	float	minDistance = +1e37f,
			maxDistance = -1e37f;

	for ( int i = 0; i < 8; i++ )
	{
		noVec3	boxVertex;
		boxVertex.x = ((i>>0)&1) ? tile->aabbMin.x : tile->aabbMax.x;
		boxVertex.y = ((i>>1)&1) ? tile->aabbMin.y : tile->aabbMax.y;
		boxVertex.z = ((i>>2)&1) ? tile->aabbMin.z : tile->aabbMax.z;

		noVec3 toTile = boxVertex - (noVec3)camPos;
		noVec3 camDir = camDst - camPos;
		camDir.Normalize();

		float distance = camDir * toTile;

		minDistance = min( minDistance, distance );
		maxDistance = max( maxDistance, distance );
	}

	minDistance = max( 10.0f, minDistance );

	float	tileDistance;
	
	noVec3 toTile = tile->center - (noVec3)camPos;
	tileDistance = max( 10.0f, toTile.Length() );

	float	worldSpaceSize  = (float)tile->worldSize / (float)heightMapX * terrainExtend;
	float	screenSpaceSize = worldSpaceSize / tileDistance;

	// get power of two texture size
	extern int qualitySelect;
	int		tileSize = (int)( screenSpaceSize * 32.0f * 32.0f * (float)(qualitySelect+1) * 0.1f );
	tileSize = 1 << ( (int)ceilf( ( logf( (float)tileSize ) / logf( 2.0f ) + 0.1f*0 ) ) );

	tileSize = max( ATLAS_MINIMUM_TILE_SIZE, tileSize );

	// this is NO correct estimation of a screen space error - more likely a guess for the upper bound of it ;-)
	float	maxScreenSpaceError = 0.25f * terrainExtend * (float)tile->worldSize / (float)quadsPerTile / minDistance * 512.0f / (float)(heightMapX-1.0f);

	if ( level == qlodMaxLevel - 1 )
		tileSize = min( MAX_TILE_TEXTURE_SIZE, tileSize );

	if ( ( maxScreenSpaceError <= 2.5f && tileSize <= 128 ) || level == qlodMaxLevel - 1 )
	{
		// ok... assign texture atlas space:

		// this strategy assumes, that we get the texture atlas space we need...
		if ( tileSize != tile->tileSize )
		{
			noVec4	coordTile;
			int tileIdx, quality, texLevel;

			// uncomment this, to restrict the amount of texels generated per frame (no real use, but nice to watch)
/*			if ( updateAmount != 0 && updateAmount + tileSize * tileSize > 64*64 )
				goto addTile2List;
			updateAmount += tileSize * tileSize;
*/
			// free texture tile space (size has changed)
			freeAllTexturesQLOD( node, level );

		nextTryForNode:

			// determine, in what atlas texture we want to put this tile (according to its size)
			int tileLevel = (int)( logf( (float)tileSize ) / logf( 2.0f ) );
			int tileAtlas = max( 0, min( ATLAS_TEXTURES-1, (tileLevel - 5)*ATLAS_TEXTURES/6 ) );

			if ( atlas[ tileAtlas ].layout->treeScan( tileSize, &texLevel, &tileIdx, &quality, &coordTile ) == -1 )
			{
				// we could not place it, where we wanted to
				tileIdx = -1;

				// let's try neighbor atlas textures
				int tileAtlasTargeted = tileAtlas;
				for ( int test = 1; test < ATLAS_TEXTURES; test ++ )
				{
					tileAtlas = tileAtlasTargeted - test;
					if ( tileAtlas >= 0 )
						if ( atlas[ tileAtlas ].layout->treeScan( tileSize, &texLevel, &tileIdx, &quality, &coordTile ) != -1 )
							goto foundSpaceForNode;
					tileAtlas = tileAtlasTargeted + test;
					if ( tileAtlas < ATLAS_TEXTURES )
						if ( atlas[ tileAtlas ].layout->treeScan( tileSize, &texLevel, &tileIdx, &quality, &coordTile ) != -1 )
							goto foundSpaceForNode;
				}
			} 

			// we could not place this tile in any atlas texture... try a smaller tile size
			if ( tileIdx == -1 && tileSize > ATLAS_MINIMUM_TILE_SIZE )
			{
				tileSize >>= 1;
				goto nextTryForNode;
			}

		foundSpaceForNode:;
			if ( tileIdx != - 1 )
			{
				// ok, got some space!
				atlas[ tileAtlas ].layout->treeAllocate( tileSize, tileIdx );

				tile->coordTile = coordTile;
				tile->tileIdx   = tileIdx | ( tileAtlas << 24 );
				tile->tileSize  = tileSize;

				atlas[ tileAtlas ].assign[ atlas[ tileAtlas ].count ++ ] = tile;
				tile->pTexture  = atlas[ tileAtlas ].texture;
			}

		}
addTile2List:
		// yes, this tile will be rendered!
		if ( tile->tileIdx != -1 )
			terrainQTRender[ qlodToRender ++ ] = tile;
	} else
	{
		traverseQLOD( pD3DDevice, camPos, camDst, qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 0, level+1 );
		traverseQLOD( pD3DDevice, camPos, camDst, qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 1, level+1 );
		traverseQLOD( pD3DDevice, camPos, camDst, qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 2, level+1 );
		traverseQLOD( pD3DDevice, camPos, camDst, qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 3, level+1 );
	}
}



