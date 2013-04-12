//#define RES_2048

#ifdef RES_2048

#define TREE_MAX_RESOLUTION	2048
#define TREE_MAX_LEVELS		7

// SUM_i=0'(TREE_MAX_LEVELS-1)( 4^i )
#define TREE_NODES			5461

static	const	int		levelOffset[ TREE_MAX_LEVELS ] = { 0, 1,  5, 21,  85,  341, 1365 };
static	const	int		levelNodes [ TREE_MAX_LEVELS ] = { 1, 4, 16, 64, 256, 1024, 4096 };
static			int		levelFree  [ TREE_MAX_LEVELS ];

#else

#define TREE_MAX_RESOLUTION	1024
#define TREE_MAX_LEVELS		6

// SUM_i=0'(TREE_MAX_LEVELS-1)( 4^i )
#define TREE_NODES			1365

static	const	int		levelOffset[ TREE_MAX_LEVELS ] = { 0, 1,  5, 21,  85,  341 };
static	const	int		levelNodes [ TREE_MAX_LEVELS ] = { 1, 4, 16, 64, 256, 1024 };
static			int		levelFree  [ TREE_MAX_LEVELS ];

#endif

#define NODE_FREE		0
#define NODE_OCC		1
#define NODE_CHILD_OCC	2
#define NODE_PARENT_OCC	3

typedef struct _MFLAT_TREE
{
//	TERRAINTILE		*tile[ TREE_NODES ];
	int			free[ TREE_NODES ];
	int			tile[ TREE_NODES ];
}FLAT_TREE;

FLAT_TREE	memTree;
FLAT_TREE	*mt = &memTree;

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

/*
TODO:
hierarchie ausnutzen, auch um die qualit?sabsch?zung zu verbessern...
rekursiv aufziehen, dann kann man auch das "free" leichter implementierne (von wegen
status des parent knoten bestimmen etc.)

*/

void	debugCheckFree()
{
	for ( int l = 0; l < TREE_MAX_LEVELS; l++ )
	{
		int		free = 0;
		int		*freePtr = &mt->free[ levelOffset[ l ] ];

		for ( int n = 0; n < levelNodes[ l ]; n++ )
		{
			if ( freePtr[ n ] == NODE_FREE )
				free++;
		}
		levelFree[ l ] = free;
	}
}

void	debugOut()
{
	char s[4] = { '.', 'X', 'Y', 'x' };
	FILE *f = fopen( "memory.txt", "wt" );

	unsigned char memLayout[ 64 * 64 ];

	int		*freePtr = &mt->free[ levelOffset[ TREE_MAX_LEVELS-1 ] ];

	int		xpos[ 4 ] = { 0, 1, 1, 0 };
	int		ypos[ 4 ] = { 0, 0, 1, 1 };


	for ( int j = 0; j < 64; j++ )
	{
		for ( int i = 0; i < 64; i++ )
		{
			int x = i, y = j;
			memLayout[ x + y * 64 ] = s[ freePtr[ i + j * 64 ] ];
		}
	}

	for ( int j = 0; j < 64; j++ )
	{
		for ( int i = 0; i < 64; i++ )
		{
			fprintf( f, "%c", memLayout[ i + j * 64 ] );
		}
		fprintf( f, "\n" );
	}

	fclose( f );
}

/*void	getBestFreeNode( int level, int *tileIdx, int *quality, int curLevel = 0 )
{
	if ( curLevel == 0 )
	{
		*tileIdx = -1;
		*quality = 0;
	}

	if ( level == curLevel )
	{

	}
}*/

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
/*	*tileIdx = -1;
	*quality = 0;
	for ( int i = 0; i < levelNodes[ level ]; i++ )
	{
		if ( freePtr[ i ] == NODE_FREE )
		{
			if ( *tileIdx == -1 )
				*tileIdx = i; 
		} else
			*quality ++;
	}
*/

	*tileIdx = -1;
	*quality = 0;
	int bestIdx = -1, bestQuality = -1;
//	getBestFreeNode( level, tileIdx, quality );
	// look for position with best quality:
	for ( int i = 0; i < levelNodes[ level ]; i++ )
	if ( freePtr[ i ] == NODE_FREE )
	{
		// estimate quality of this location
		int parentOfs = i / 4;
		int curLevel = level - 1;
		int curQuality = 0;

//		while ( curLevel >= 0 )
		{
			for ( int j = 0; j < 4; j++ )
				if ( mt->free[ levelOffset[ curLevel + 1 ] + parentOfs * 4 + j ] != NODE_FREE )
					curQuality ++;

			curLevel --;
			parentOfs /= 4;
		}

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


/*************

	// if all nodes of this level are free, then parent node is free, too
	// so und das hier m?ste rekursiv passieren... 
	// => rekursive implementation!?
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
**********/




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
	// so und das hier m?ste rekursiv passieren... 
	// => rekursive implementation!?
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



// IN: size of texture tile to allocate
// OUT: tile index & quality measurement
int		treeScan( int size, int *levelIdx, int *tileIdx, int *quality, D3DXVECTOR4 *coordTile )
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
/*	*tileIdx = -1;
	*quality = 0;
	for ( int i = 0; i < levelNodes[ level ]; i++ )
	{
		if ( freePtr[ i ] == NODE_FREE )
		{
			if ( *tileIdx == -1 )
				*tileIdx = i; 
		} else
			*quality ++;
	}
*/

	*tileIdx = -1;
	*quality = 0;
	int bestIdx = -1, bestQuality = -1;
//	getBestFreeNode( level, tileIdx, quality );
	// look for position with best quality:
	for ( int i = 0; i < levelNodes[ level ]; i++ )
	if ( freePtr[ i ] == NODE_FREE )
	{
		// estimate quality of this location
		int parentOfs = i / 4;
		int curLevel = level - 1;
		int curQuality = 0;

//		while ( curLevel >= 0 )
		{
			for ( int j = 0; j < 4; j++ )
				if ( mt->free[ levelOffset[ curLevel + 1 ] + parentOfs * 4 + j ] != NODE_FREE )
					curQuality ++;

			curLevel --;
			parentOfs /= 4;
		}

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


/*************

	// if all nodes of this level are free, then parent node is free, too
	// so und das hier m?ste rekursiv passieren... 
	// => rekursive implementation!?
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
**********/




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

int		treeAllocate( int size, int tileIdx )
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

int		treeFree( int size, int tileIdx )
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
	// so und das hier m?ste rekursiv passieren... 
	// => rekursive implementation!?
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