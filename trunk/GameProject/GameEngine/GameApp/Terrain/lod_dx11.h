//  
// Cached Procedural Textures (see ShaderX4 for details)
// (w)(c)2005 Carsten Dachsbacher
//

#pragma once
#ifndef _LOD__H
#define _LOD__H

// a terrain-tile represents a square region of the terrain (original or coarser representation)
struct TerrTileVertex  {
	noVec4 pos;
	noVec2 uv;
};


typedef struct
{
	static float* LockVB( int offset);
	static void UnlockVB( );
	static void* LockIB( );
	static void UnlockIB( );
	static void CreateInputLayout(ID3D11Device* pd3dDevice, ID3DX11EffectTechnique* pTech);	

	bool CreateVertexBuffer(const void* pVertexData, unsigned int uiNumVertices, unsigned int uiVertexSize);
	bool CreateIndexBuffer(const void* pIndexData, unsigned int uiNumIndices);

	noVec3				center,				// the center and AABB of this terrain part
		aabbMin, 
		aabbMax;
	
	//LPD3DXMESH				pMesh;				// mesh for this terrain part
	DWORD					fvfMeshVertex, 
		sizePerVertex;

	static ID3D11InputLayout* m_pInputLayout;
	ID3D11Buffer *m_pVertexBuffer;
	ID3D11Buffer *m_pIndexBuffer;

	unsigned int						m_uiCurrentVertexBufferSize;	// in bytes
	unsigned int						m_uiCurrentIndexBufferSize;	// # of indices
	static bool		is16BitBuffer;

	struct D3D11Texture2D*	pTexture;			// texture containing surface color
	noVec4				coordTile;			// bias + scale: 4 floats defining rectangle of corresponding texture area
	int						tileSize, 
		tileIdx;			// stores tile index (lower 24 bit) and texture atlas (upper 8 bit)
	int						worldSize;			// real size in world space
	int						intersectFrustum;	// guess what

	D3D11Texture2D*	pHeightMapTexture;	// ptr to source height map
	noVec4					coordHM;			// bias + scale to adress corresponding rectangle in the heightmap
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
	class RenderTarget2D*	texture;

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

void	createQLOD( ID3D11Device* pD3DDevice, int heightMapSize, WORD *heightMap16, D3D11Texture2D* heightMapTexture16 );
void traverseQLOD( ID3D11Device* pD3DDevice, noVec4 &camPos, noVec4 &camDst, int node, int level );
void	freeAllTexturesQLOD( int node, int level );
void	renderQLOD( ID3DX11Effect *pEffect );
void	destroyQLOD();

#endif
