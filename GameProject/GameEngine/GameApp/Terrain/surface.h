   //  
  // Cached Procedural Textures (see ShaderX4 for details)
 // (w)(c)2005 Carsten Dachsbacher
//

#pragma once
#ifndef _SURFACE__H
#define _SURFACE__H

// this structure holds our surface parameters
typedef struct _PROCTEX_SURFACEPARAM
{
	char	name[ 256 ];

	// appearance Parameters
	DWORD	spRandomSeed1;
	float	spBumpiness;

	//float	spScaleBump;
	noVec4	spColor;

	// Distribution Parameters
	DWORD	spRandomSeed2;
	float	spCoverage,
			spFractalNoise,
			spMimicTerrain,
			spAltitudeMin,
			spAltitudeMax,
			spAltitudeFuzzinessMin,
			spAltitudeFuzzinessMax,
			spSlopeMin,
			spSlopeMax,
			spSlopeFuzzinessMin, 
			spSlopeFuzzinessMax, 
			spBluriness;

	float	flagAltLimitMin, 
			flagAltLimitMax;
	float	flagSlopeLimitMin, 
			flagSlopeLimitMax;

	// temporary parameters
	int		nodeID;
	float	tpAC1, tpAC2, tpAM;
	float	tpSC1, tpSC2, tpSM;

	noVec4	tpAScaleBias, tpSScaleBias;
	noVec4 noisePan;
	
	int		nChilds;
	_PROCTEX_SURFACEPARAM *child[ 8 ];
}PROCTEX_SURFACEPARAM;

PROCTEX_SURFACEPARAM	*rootSurfaceNode;
PROCTEX_SURFACEPARAM	*surfaceTypes[ 2 ];
noVec4					terrainBaseColor;

PROCTEX_SURFACEPARAM	*newSurfaceNode()
{
	PROCTEX_SURFACEPARAM *node = new PROCTEX_SURFACEPARAM;

	node->nChilds = 0;

	node->spRandomSeed1 = 0;
	node->spRandomSeed2 = 0;
	node->spBumpiness   = 0.0f;

	node->spColor		= noVec4( 0.0f, 0.0f, 0.0f, 0.0f );
	node->noisePan		= noVec4( rand() / 32768.0f, rand() / 32768.0f, 0.0f, 0.0f );

	// Distribution Parameters
	node->spCoverage		  = 
	node->spFractalNoise	  = 
	node->spMimicTerrain	  = 
	node->spAltitudeMin		  =
	node->spAltitudeMax		  =
	node->spAltitudeFuzzinessMin =
	node->spAltitudeFuzzinessMax =
	node->spSlopeMin		  =
	node->spSlopeMax		  =
	node->spSlopeFuzzinessMin =
	node->spSlopeFuzzinessMax =
	node->spBluriness         = 0.0f;

	return node;
}

// the following methods are used to read Terragen surface maps

void	readTERMChunk( FILE *f, PROCTEX_SURFACEPARAM *chunk, int chunkSize = 0, int level = 0 );
void	readDENSChunk( FILE *f, PROCTEX_SURFACEPARAM *chunk, int chunkSize = 0, int level = 0 );

void	readSRFLChunk( FILE *f, PROCTEX_SURFACEPARAM *chunk, int chunkSize = 0, int level = 0 )
{
	DWORD	id, length;
	char	buf[ 256 ];

	do {

		fread( &id,     1, sizeof( DWORD ), f );
		fread( &length, 1, sizeof( DWORD ), f );
		if ( length & 3 ) length = ( length & ~3 ) + 4;
		((char*)&id)[0] = toupper( ((char*)&id)[0] );
		((char*)&id)[1] = toupper( ((char*)&id)[1] );
		((char*)&id)[2] = toupper( ((char*)&id)[2] );
		((char*)&id)[3] = toupper( ((char*)&id)[3] );

		chunkSize -= 8 + length;

		switch ( id )
		{
			// SRFL: child layer
			case 0x4c465253:
				if ( level == 0 )
				{
					readSRFLChunk( f, chunk, length, level + 1 );
					return;
				} else
				{
					chunk->child[ chunk->nChilds ] = newSurfaceNode();
					readSRFLChunk( f, chunk->child[ chunk->nChilds ], length, level + 1 );
					chunk->nChilds ++;
				}
				break;
			// NAME: name chunk
			case 0x454d414e:
				fread( buf, 1, length, f );
				strncpy( chunk->name, buf, 255 );
				break;
			// TERM: term chunk
			case 0x4d524554:
				readTERMChunk( f, chunk, length, level );
				break;
			// DENS: dens chunk
			case 0x534e4544:
				readDENSChunk( f, chunk, length, level );
				break;
			default:
				fseek( f, length, SEEK_CUR );
				break;
		};

	} while ( chunkSize > 0 );
}


void	readTERMChunk( FILE *f, PROCTEX_SURFACEPARAM *chunk, int chunkSize, int level )
{
	DWORD	id, length;
	WORD	r, g, b;

	do {

		fread( &id,     1, sizeof( DWORD ), f );
		fread( &length, 1, sizeof( DWORD ), f );
		if ( length & 3 ) length = ( length & ~3 ) + 4;
		((char*)&id)[0] = toupper( ((char*)&id)[0] );
		((char*)&id)[1] = toupper( ((char*)&id)[1] );
		((char*)&id)[2] = toupper( ((char*)&id)[2] );
		((char*)&id)[3] = toupper( ((char*)&id)[3] );

		chunkSize -= 8 + length;

		switch ( id )
		{
			// DIFC: diffuse color chunk
			case 0x43464944:
				fread( &r, 1, sizeof( WORD ), f );
				fread( &g, 1, sizeof( WORD ), f );
				fread( &b, 1, sizeof( WORD ), f );
				fseek( f, length-3*sizeof( WORD ), SEEK_CUR );

				chunk->spColor = noVec4( (float)r, (float)g, (float)b, 0.0f ) / 256.0f;
				break;

			// BMAM: bump map amount chunk
			case 0x4d414d42:
				fread( &chunk->spBumpiness, 1, sizeof( float ), f );
				break;

			// MTER: mimic terrain chunk
			case 0x5245544d:
				fread( &chunk->spMimicTerrain, 1, sizeof( float ), f );
				break;

			// BMTX: bump map texture settings chunk
			case 0x58544d42:
				fseek( f, length, SEEK_CUR );
				break;

			default:
				fseek( f, length, SEEK_CUR );
				break;
		};

	} while ( chunkSize > 0 );
}


void	readDENSChunk( FILE *f, PROCTEX_SURFACEPARAM *chunk, int chunkSize, int level )
{
	DWORD	id, length;

	do {

		fread( &id,     1, sizeof( DWORD ), f );
		fread( &length, 1, sizeof( DWORD ), f );
		if ( length & 3 ) length = ( length & ~3 ) + 4;
		((char*)&id)[0] = toupper( ((char*)&id)[0] );
		((char*)&id)[1] = toupper( ((char*)&id)[1] );
		((char*)&id)[2] = toupper( ((char*)&id)[2] );
		((char*)&id)[3] = toupper( ((char*)&id)[3] );

		chunkSize -= 8 + length;

		switch ( id )
		{
			// VARI: fractal noise chunk
			case 0x49524156:
				fread( &chunk->spFractalNoise, 1, sizeof( float ), f );
				break;

			// COVR: coverage chunk
			case 0x52564f43:
				fread( &chunk->spCoverage, 1, sizeof( float ), f );
				break;

			// ALTE: altitude effects chunk
			case 0x45544c41:
				fread( &chunk->flagAltLimitMin, 1, sizeof( float ), f );
				fread( &chunk->flagAltLimitMax, 1, sizeof( float ), f );
				break;

			// ALTL: altitude limits chunk
			case 0x4c544c41:
				fread( &chunk->spAltitudeMin, 1, sizeof( float ), f );
				fread( &chunk->spAltitudeMax, 1, sizeof( float ), f );
				break;

			// ALTF: altitude fuzziness chunk
			case 0x46544c41:
				fread( &chunk->spAltitudeFuzzinessMin, 1, sizeof( float ), f );
				fread( &chunk->spAltitudeFuzzinessMax, 1, sizeof( float ), f );
				break;

			// SLPE: slope effects chunk
			case 0x45504c53:
				fread( &chunk->flagSlopeLimitMin, 1, sizeof( float ), f );
				fread( &chunk->flagSlopeLimitMax, 1, sizeof( float ), f );
				break;

			// SLPL: slope limits chunk
			case 0x4c504c53:
				fread( &chunk->spSlopeMin, 1, sizeof( float ), f );
				fread( &chunk->spSlopeMax, 1, sizeof( float ), f );
				break;

			// SLPF: slope fuzziness chunk
			case 0x46504c53:
				fread( &chunk->spSlopeFuzzinessMin, 1, sizeof( float ), f );
				fread( &chunk->spSlopeFuzzinessMax, 1, sizeof( float ), f );
				break;

			// SMSZ: smoothing size chunk
			case 0x5a534d53:
				fread( &chunk->spBluriness, 1, sizeof( float ), f );
				break;


			// VART: variation texture settings chunk
			case 0x54524156:
				fseek( f, length, SEEK_CUR );
				break;
			// DNSP: plugin list chunk
			case 0x444e5350:
				fseek( f, length, SEEK_CUR );
				break;

			default:
				fseek( f, length, SEEK_CUR );
				break;
		};

	} while ( chunkSize > 0 );
}

// compute shader-friendly texturing constraints
void	ptPrepareMaterialNode( PROCTEX_SURFACEPARAM *sp )
{
	float m, n;

	extern float terrainMaxElevation, terrainExtend;

	sp->spAltitudeMax = sp->spAltitudeMax * 30.0f / terrainMaxElevation;
	sp->spAltitudeMin = sp->spAltitudeMin * 30.0f / terrainMaxElevation;
	sp->spAltitudeFuzzinessMin = sp->spAltitudeFuzzinessMin * 30.0f / terrainMaxElevation;
	sp->spAltitudeFuzzinessMax = sp->spAltitudeFuzzinessMax * 30.0f / terrainMaxElevation;

	sp->spAltitudeFuzzinessMin *= 1.0f;
	sp->spAltitudeFuzzinessMax *= 1.0f;
	sp->spSlopeFuzzinessMin *= 1.0f;
	sp->spSlopeFuzzinessMax *= 1.0f;

	if ( sp->flagAltLimitMax < 0.5f )
		sp->spAltitudeMax = 2.0f;
	if ( sp->flagAltLimitMin < 0.5f )
		sp->spAltitudeMin = -1.0f;
	if ( sp->flagSlopeLimitMax < 0.5f )
		sp->spSlopeMax = 100.0f;
	if ( sp->flagSlopeLimitMin < 0.5f )
		sp->spSlopeMin = -1.0f;


	// Altitude

	// right side
	m = -1.0f / ( 2.0f * sp->spAltitudeFuzzinessMax );
	n = -m * ( sp->spAltitudeMax + sp->spAltitudeFuzzinessMax );

	sp->tpAScaleBias.x = m;
	sp->tpAScaleBias.y = n;

	// left side
	m = +1.0f / ( 2.0f * sp->spAltitudeFuzzinessMin );
	n = -m * ( sp->spAltitudeMin - sp->spAltitudeFuzzinessMin );

	sp->tpAScaleBias.z = m;
	sp->tpAScaleBias.w = n;


	// Slope

	// right side
	m = -1.0f / ( 2.0f * sp->spSlopeFuzzinessMax*0.5f );
	n = -m * ( sp->spSlopeMax + sp->spSlopeFuzzinessMax );

	sp->tpSScaleBias.x = m;
	sp->tpSScaleBias.y = n;

	// left side
	m = +1.0f / ( 2.0f * sp->spSlopeFuzzinessMin*0.5f );
	n = -m * ( sp->spSlopeMin - sp->spSlopeFuzzinessMin );

	sp->tpSScaleBias.z = m;
	sp->tpSScaleBias.w = n;
}

void	prepareSurfaceNodes( PROCTEX_SURFACEPARAM *node )
{
	ptPrepareMaterialNode( node );
	for ( int i = 0; i < node->nChilds; i++ )
		prepareSurfaceNodes( node->child[ i ] );
}

void	loadTGSurfaceMap( char *filename, PROCTEX_SURFACEPARAM **root )
{
	*root = newSurfaceNode();
	
	FILE *f = fopen( filename, "rb" );

	if ( f != NULL )
	{
		// Header: TERRAGEN & SURFMAP2
		fseek( f, 16, SEEK_SET );

		readSRFLChunk( f, *root );
		fclose( f );

		prepareSurfaceNodes( *root );
	}
}

void	createSurfaceNodes()
{
	loadTGSurfaceMap( "./data/demo1.srf", &surfaceTypes[ 0 ] );
	loadTGSurfaceMap( "./data/demo2.srf", &surfaceTypes[ 1 ] );
		
	rootSurfaceNode = surfaceTypes[ 0 ];
}

#endif

