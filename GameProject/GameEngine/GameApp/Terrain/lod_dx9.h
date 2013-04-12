   //  
  // Cached Procedural Textures (see ShaderX4 for details)
 // (w)(c)2005 Carsten Dachsbacher
//

#pragma once
#ifndef _LOD__H
#define _LOD__H

// a terrain-tile represents a square region of the terrain (original or coarser representation)
typedef struct
{
	D3DXVECTOR3				center,				// the center and AABB of this terrain part
							aabbMin, 
							aabbMax;

	LPD3DXMESH				pMesh;				// mesh for this terrain part
	DWORD					fvfMeshVertex, 
							sizePerVertex;

	LPDIRECT3DTEXTURE9		pTexture;			// texture containing surface color
	D3DXVECTOR4				coordTile;			// bias + scale: 4 floats defining rectangle of corresponding texture area
	int						tileSize, 
							tileIdx;			// stores tile index (lower 24 bit) and texture atlas (upper 8 bit)
	int						worldSize;			// real size in world space
	int						intersectFrustum;	// guess what
	
	LPDIRECT3DTEXTURE9		pHeightMapTexture;	// ptr to source height map
	D3DXVECTOR4				coordHM;			// bias + scale to adress corresponding rectangle in the heightmap
}TERRAINTILE;

// number of atlas textures (each 512?
#define ATLAS_TEXTURES	32

#define	MAX_TILE_TEXTURE_SIZE	512
#define ATLAS_MINIMUM_TILE_SIZE	32

// this is, what we store for each atlas texture
class FQTree;
typedef struct
{
	// quadtree for managing the tile placement
	FQTree				*layout;
	// its d3d texture
	LPDIRECT3DTEXTURE9	texture;
	
	// pointers to the terrain tiles stored
	TERRAINTILE			*assign[ (MAX_TILE_TEXTURE_SIZE/ATLAS_MINIMUM_TILE_SIZE)*(MAX_TILE_TEXTURE_SIZE/ATLAS_MINIMUM_TILE_SIZE) ];
	// number of tiles
	DWORD				count;
}ATLAS;

// see lod.cpp
extern int	heightMapX, 
			heightMapY,
			heightMapXFull,
			heightMapYFull,
			quadsPerTile;

void	createQLOD( IDirect3DDevice9* pD3DDevice, int heightMapSize, WORD *heightMap16, LPDIRECT3DTEXTURE9 heightMapTexture16 );
void	traverseQLOD( IDirect3DDevice9* pD3DDevice, D3DXVECTOR4 &camPos, D3DXVECTOR4 &camDst, int node, int level );
void	freeAllTexturesQLOD( int node, int level );
void	renderQLOD( ID3DXEffect *pEffect );
void	destroyQLOD();

#endif