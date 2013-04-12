   //  
  // Cached Procedural Textures (see ShaderX4 for details)
 // (w)(c)2005 Carsten Dachsbacher
//

#pragma once
#ifndef TEXTURE_QUADTREE__H
#define TEXTURE_QUADTREE__H

// choose different atlas sizes here
/*
#define TREE_MAX_RESOLUTION	1024
#define TREE_MAX_LEVELS		6

// SUM_i=0'(TREE_MAX_LEVELS-1)( 4^i )
#define TREE_NODES			1365
*/

#define TREE_MAX_RESOLUTION	512
#define TREE_MAX_LEVELS		5

// SUM_i=0'(TREE_MAX_LEVELS-1)( 4^i )
#define TREE_NODES			341

static	const	int		levelOffset[ 6 ] = { 0, 1,  5, 21,  85,  341 };
static	const	int		levelNodes [ 6 ] = { 1, 4, 16, 64, 256, 1024 };
static			int		levelFree  [ 6 ];


#define NODE_FREE		0
#define NODE_OCC		1
#define NODE_CHILD_OCC	2
#define NODE_PARENT_OCC	3

typedef struct _MFLAT_TREE
{
	int			free[ TREE_NODES ];
	int			tile[ TREE_NODES ];
}FLAT_TREE;

// this class handles the tile placement within atlas textures
class FQTree
{
	private:
		FLAT_TREE	*mt;
	public:
		FQTree()
		{
            mt = new FLAT_TREE;
			ZeroMemory( mt->free, sizeof( int ) * TREE_NODES );
		};

		~FQTree()
		{
			delete mt;
		}

		int	treeScan( int size, int *levelIdx, int *tileIdx, int *quality, D3DXVECTOR4 *coordTile );
		int	treeAllocate( int size, int tileIdx );
		int	treeFree( int size, int tileIdx );
};


#endif