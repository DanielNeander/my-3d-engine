   //  
  // Cached Procedural Textures (see ShaderX4 for details)
 // (w)(c)2005 Carsten Dachsbacher
//

#include "stdafx.h"
#include "quadtree.h"

// scan the atlas texture for free regions meeting the size requirements
// IN: size of texture tile to allocate
// OUT: tile index & quality measurement
int		FQTree::treeScan( int size, int *levelIdx, int *tileIdx, int *quality, D3DXVECTOR4 *coordTile )
{
	// determine on which level the memory chunk need to be placed
	if ( size > TREE_MAX_RESOLUTION || size == 0 )
		return -1;

	// compute level of memory quad tree
	int level = (int)( logf( (float)(TREE_MAX_RESOLUTION / size) ) / logf( 2.0f ) );
	*levelIdx = level;

	if ( level >= TREE_MAX_LEVELS )
		return -1;

	int		*freePtr = &mt->free[ levelOffset[ level ] ];
	int		*tilePtr = &mt->tile[ levelOffset[ level ] ];

	// look for free memory tile and estimate quality
	// simple quality estimate: the more neighbour tiles are filled, the better!
    // simple variante
	*tileIdx = -1;
	*quality = 0;
	int bestIdx = -1, bestQuality = -1;

	// look for position with best quality:
	for ( int i = 0; i < levelNodes[ level ]; i++ )
	if ( freePtr[ i ] == NODE_FREE )
	{
		// estimate quality of this location
		int parentOfs = i / 4;
		int curLevel = level - 1;
		int curQuality = 0;

		for ( int j = 0; j < 4; j++ )
			if ( mt->free[ levelOffset[ curLevel + 1 ] + parentOfs * 4 + j ] != NODE_FREE )
				curQuality ++;

		curLevel --;
		parentOfs /= 4;

		if ( curQuality > bestQuality )
		{
			bestQuality = curQuality;
			bestIdx     = i;
		}
	}

	if ( bestIdx != -1 )
	{
		*tileIdx = bestIdx;
		*quality = bestQuality;
	}

	if ( *tileIdx == - 1)
		return -1;

	//
	// determine tile position in texture
	//
	*coordTile = D3DXVECTOR4( 0.0f, 0.0f, 1.0f, 1.0f );

	int idx = *tileIdx;

	for ( int curLevel = level - 1; curLevel >= 0; curLevel -- )
	{
		*coordTile *= 0.5f;

		if ( (idx & 3) == 1 || (idx & 3) == 2 )
			coordTile->x += 0.5f;
		if ( (idx & 3) == 2 || (idx & 3) == 3 )
			coordTile->y += 0.5f;

		idx >>= 2;
	}


	return 1;
}

// allocate a part of the atlas texture (its index and thus location was determined by using 'treeScan')
int		FQTree::treeAllocate( int size, int tileIdx )
{
	int		i, j;
	// determine on which level the memory chunk need to be placed
	if ( size > TREE_MAX_RESOLUTION || size == 0 )
		return -1;

	// compute level of memory quad tree
	int level = (int)( logf( (float)(TREE_MAX_RESOLUTION / size) ) / logf( 2.0f ) );

	if ( level >= TREE_MAX_LEVELS )
		return -1;

	int		*freePtr = &mt->free[ levelOffset[ level ] ];
	int		*tilePtr = &mt->tile[ levelOffset[ level ] ];

	// occupy node, children and parent nodes
	freePtr[ tileIdx ] = NODE_OCC;

	int nodes = 4, ofs = tileIdx * 4;
	for ( i = level + 1; i < TREE_MAX_LEVELS; i++ )
	{
		for ( j = 0; j < nodes; j++ )
			mt->free[ levelOffset[ i ] + ofs + j ] = NODE_PARENT_OCC;
		nodes *= 4;
		ofs   *= 4;
	}

	ofs = tileIdx / 4;
	for ( i = level - 1; i >= 0; i--, ofs /= 4 )
	{
		mt->free[ levelOffset[ i ] + ofs ] = NODE_CHILD_OCC;
	}

	return 1;
}

// free previously allocated texture tiles
int		FQTree::treeFree( int size, int tileIdx )
{
	int		i, j;
	// determine on which level the memory chunk need to be placed
	if ( size > TREE_MAX_RESOLUTION || size == 0 )
		return -1;

	// compute level of memory quad tree
	int level = (int)( logf( (float)(TREE_MAX_RESOLUTION / size) ) / logf( 2.0f ) );

	if ( level >= TREE_MAX_LEVELS )
		return -1;

	int		*freePtr = &mt->free[ levelOffset[ level ] ];
	int		*tilePtr = &mt->tile[ levelOffset[ level ] ];

	// free node itself
	freePtr[ tileIdx ] = NODE_FREE;

	// free children
	int nodes = 4, ofs = tileIdx * 4;
	for ( i = level + 1; i < TREE_MAX_LEVELS; i++ )
	{
		for ( j = 0; j < nodes; j++ )
			mt->free[ levelOffset[ i ] + ofs + j ] = NODE_FREE;
		nodes *= 4;
		ofs   *= 4;
	}

	// if all nodes of this level are free, then parent node is free, too
	int parentOfs = tileIdx / 4;
	int curLevel = level - 1;

	while ( curLevel >= 0 )
	{
		int childNodeOcc = 0;
		for ( i = 0; i < 4; i++ )
			if ( mt->free[ levelOffset[ curLevel + 1 ] + parentOfs * 4 + i ] != NODE_FREE )
				childNodeOcc ++;

		if ( childNodeOcc == 0 )
			mt->free[ levelOffset[ curLevel ] + parentOfs ] = NODE_FREE;

		curLevel --;
		parentOfs /= 4;
	}

	return 1;
}
