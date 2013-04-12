   //  
  // Cached Procedural Textures (see ShaderX4 for details)
 // (w)(c)2005 Carsten Dachsbacher
//

#include "dxstdafx.h"
#include "quadtree.h"
#include "lod.h"
#include "frustum.h"

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

// recursive (quadtree) generation of the terrain-lod
void	buildQLOD( IDirect3DDevice9* pD3DDevice, int node, int x, int y, int size, int level, WORD *heightMap16, LPDIRECT3DTEXTURE9 heightMapTexture16 )
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
	tile->fvfMeshVertex = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0);
	tile->sizePerVertex = 6 * sizeof( float );

	DWORD creationFlags = D3DXMESH_VB_MANAGED | D3DXMESH_IB_MANAGED;

	//creationFlags = 0;
	if ( quadsPerTile * quadsPerTile * 2 * 3 >= 65535 )
		creationFlags |= D3DXMESH_32BIT;

	float *pData = NULL;

	D3DXCreateMeshFVF(	(quadsPerTile+2) * (quadsPerTile+2) * 2,
						( quadsPerTile + 1+2 ) * ( quadsPerTile + 1+2 ),
						creationFlags,
						tile->fvfMeshVertex,
						pD3DDevice,
						&tile->pMesh );

	HRESULT hr;

	hr = tile->pMesh->LockVertexBuffer( D3DLOCK_DISCARD, (void**)&pData );

	tile->center  = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
	tile->aabbMin = D3DXVECTOR3( +1e37f, +1e37f, +1e37f );
	tile->aabbMax = D3DXVECTOR3( -1e37f, -1e37f, -1e37f );

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
				tile->center += D3DXVECTOR3( xc, pData[ idx * 6 + 1 ], zc );

			D3DXVec3Minimize( &tile->aabbMin, &tile->aabbMin, (D3DXVECTOR3*)&pData[ idx * 6 + 0 ] );
			D3DXVec3Maximize( &tile->aabbMax, &tile->aabbMax, (D3DXVECTOR3*)&pData[ idx * 6 + 0 ] );
		}

	tile->pMesh->UnlockVertexBuffer();

	tile->center /= (float)( ( quadsPerTile + 1 ) * ( quadsPerTile + 1 ) );

	void *pIndexBuffer;
	tile->pMesh->LockIndexBuffer( D3DLOCK_DISCARD, &pIndexBuffer );

	if ( creationFlags & D3DXMESH_32BIT )
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
	tile->pMesh->UnlockIndexBuffer();

	tile->pMesh->OptimizeInplace( D3DXMESHOPT_VERTEXCACHE|D3DXMESHOPT_STRIPREORDER, NULL, NULL, NULL, NULL );

	terrainQT[ node ] = tile;

	buildQLOD( pD3DDevice, qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 0, x,        y,        size/2, level+1, heightMap16, heightMapTexture16 );
	buildQLOD( pD3DDevice, qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 1, x+size/2, y,        size/2, level+1, heightMap16, heightMapTexture16 );
	buildQLOD( pD3DDevice, qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 2, x,        y+size/2, size/2, level+1, heightMap16, heightMapTexture16 );
	buildQLOD( pD3DDevice, qlodOffset[ level+1 ] + (node-qlodOffset[level]) * 4 + 3, x+size/2, y+size/2, size/2, level+1, heightMap16, heightMapTexture16 );
}


//
// create chunk lod quadtree
//
void	createQLOD( IDirect3DDevice9* pD3DDevice, int heightMapSize, WORD *heightMap16, LPDIRECT3DTEXTURE9 heightMapTexture16 )
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
void	renderQLOD( ID3DXEffect *pEffect )
{
	LPDIRECT3DTEXTURE9	curTexture = NULL;
	int					curTileSize = 0;


	for ( int a = 0; a < qlodToRender; a++ )
	{
		int commit = 0;

		TERRAINTILE *tile = terrainQTRender[ a ];

		if ( curTileSize != tile->tileSize )
		{
			curTileSize = tile->tileSize;
			D3DXVECTOR4	rtSize = D3DXVECTOR4( 0.0f, 0.0f, (float)tile->tileSize, 0.0f );
			pEffect->SetVector( "rtSize", &rtSize );
			commit ++;
		}

		if ( curTexture != tile->pTexture )
		{
			curTexture = tile->pTexture;
			pEffect->SetTexture( "atlasTexture", tile->pTexture );
			commit++;
		}
		pEffect->SetVector ( "texCoordBiasScale", &tile->coordTile ); 

		pEffect->CommitChanges();

		tile->pMesh->DrawSubset( 0 );
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

void	traverseQLOD( IDirect3DDevice9* pD3DDevice, D3DXVECTOR4 &camPos, D3DXVECTOR4 &camDst, int node, int level )
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
		D3DXVECTOR3	boxVertex;
		boxVertex.x = ((i>>0)&1) ? tile->aabbMin.x : tile->aabbMax.x;
		boxVertex.y = ((i>>1)&1) ? tile->aabbMin.y : tile->aabbMax.y;
		boxVertex.z = ((i>>2)&1) ? tile->aabbMin.z : tile->aabbMax.z;

		D3DXVECTOR3 toTile = boxVertex - (D3DXVECTOR3)camPos;
		D3DXVECTOR3 camDir = camDst - camPos;
		D3DXVec3Normalize( &camDir, &camDir );

		float distance = D3DXVec3Dot( &camDir, &toTile );

		minDistance = min( minDistance, distance );
		maxDistance = max( maxDistance, distance );
	}

	minDistance = max( 10.0f, minDistance );

	float	tileDistance;
	
	D3DXVECTOR3 toTile = tile->center - (D3DXVECTOR3)camPos;
	tileDistance = max( 10.0f, D3DXVec3Length( &toTile ) );

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
			D3DXVECTOR4	coordTile;
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
 