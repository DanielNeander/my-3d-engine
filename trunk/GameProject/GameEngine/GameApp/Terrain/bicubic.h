   //  
  // Cached Procedural Textures (see ShaderX4 for details)
 // (w)(c)2005 Carsten Dachsbacher
//

#pragma once
#ifndef BICUBIC__H
#define BICUBIC__H

// resolution of the lookup tables
#define WEIGHT_TEX_RES	128

// choose one of the following options for 16-bit integer, 16-bit float oder 32-bit float (first and last work fine)
//#define BICUBIC_FORMAT	D3DFMT_A16B16G16R16
//#define BICUBIC_FORMAT	D3DFMT_A16B16G16R16F
//#define BICUBIC_FORMAT	D3DFMT_A32B32G32R32F


// computes weights for cubic interpolation of the height map
float R( float x )
{
	float v, t;
	t  = max( 0.0f, x + 2.0f );
	v  = t * t * t;
	t  = max( 0.0f, x + 1.0f );
	v -= 4.0f * t * t * t;
	t  = max( 0.0f, x + 0.0f );
	v += 6.0f * t * t * t;
	t  = max( 0.0f, x - 1.0f );
	v -= 4.0f * t * t * t;
	v /= 6.0f;
	return v;
}

// create & compute the textures
int generateBiCubicWeights( IDirect3DDevice9* pD3DDevice )
{
	int	i, j;

	if ( FAILED( pD3DDevice->CreateTexture(
		WEIGHT_TEX_RES, WEIGHT_TEX_RES, 1, 0, BICUBIC_FORMAT, D3DPOOL_MANAGED, (IDirect3DTexture9**)&bicubicWeight03, NULL ) ) )
		return -1;
	if ( FAILED( pD3DDevice->CreateTexture(
		WEIGHT_TEX_RES, WEIGHT_TEX_RES, 1, 0, BICUBIC_FORMAT, D3DPOOL_MANAGED, (IDirect3DTexture9**)&bicubicWeight47, NULL ) ) )
		return -1;
	if ( FAILED( pD3DDevice->CreateTexture(
		WEIGHT_TEX_RES, WEIGHT_TEX_RES, 1, 0, BICUBIC_FORMAT, D3DPOOL_MANAGED, (IDirect3DTexture9**)&bicubicWeight8B, NULL ) ) )
		return -1;
	if ( FAILED( pD3DDevice->CreateTexture(
		WEIGHT_TEX_RES, WEIGHT_TEX_RES, 1, 0, BICUBIC_FORMAT, D3DPOOL_MANAGED, (IDirect3DTexture9**)&bicubicWeightCF, NULL ) ) )
		return -1;


	// !hack!
	D3DXVECTOR4 *rFunc    = new D3DXVECTOR4[ WEIGHT_TEX_RES * WEIGHT_TEX_RES ];
	WORD		*rFunc16  = new WORD       [ WEIGHT_TEX_RES * WEIGHT_TEX_RES * 4 ];
	D3DXFLOAT16 *rFunc16F = new D3DXFLOAT16[ WEIGHT_TEX_RES * WEIGHT_TEX_RES * 4 ];

	for ( int n = -1; n <= 2; n++ )
	{
		for ( j = 0; j < WEIGHT_TEX_RES; j++ )
			for ( i = 0; i < WEIGHT_TEX_RES; i++ )
			{
				float dx = (float)(i+0) / (float)(WEIGHT_TEX_RES), dy = (float)(j+0) / (float)(WEIGHT_TEX_RES);

				D3DXVECTOR4 r;
				// m = -1..+2, n = -1
				r.x = R( -1.0f - dx ) * R( dy - (float)n );
				r.y = R( +0.0f - dx ) * R( dy - (float)n );
				r.z = R( +1.0f - dx ) * R( dy - (float)n );
				r.w = R( +2.0f - dx ) * R( dy - (float)n );

				D3DXFloat32To16Array( &rFunc16F[ ( i + j * WEIGHT_TEX_RES ) * 4 ], (FLOAT*)&r, 4 );

				rFunc16[ ( i + j * WEIGHT_TEX_RES ) * 4 + 0 ] = (WORD)(r.x * 65536.0f);
				rFunc16[ ( i + j * WEIGHT_TEX_RES ) * 4 + 1 ] = (WORD)(r.y * 65536.0f);
				rFunc16[ ( i + j * WEIGHT_TEX_RES ) * 4 + 2 ] = (WORD)(r.z * 65536.0f);
				rFunc16[ ( i + j * WEIGHT_TEX_RES ) * 4 + 3 ] = (WORD)(r.w * 65536.0f);

				rFunc[ i + j * WEIGHT_TEX_RES ] = r;
			}

#if BICUBIC_FORMAT == D3DFMT_A32B32G32R32F
		void *src = rFunc;
		int  size = sizeof( float );
#elif BICUBIC_FORMAT == D3DFMT_A16B16G16R16F
		void *src = rFunc16F;
		int  size = sizeof( WORD );
#elif BICUBIC_FORMAT == D3DFMT_A16B16G16R16
		void *src = rFunc16;
		int  size = sizeof( WORD );
#endif

		LPDIRECT3DTEXTURE9 dst;
		switch ( n )
		{
		case -1: dst = bicubicWeight03;	break;
		case +0: dst = bicubicWeight47;	break;
		case +1: dst = bicubicWeight8B;	break;
		case +2: dst = bicubicWeightCF;	break;
		}

		LPDIRECT3DSURFACE9	lpSurface;
		dst->GetSurfaceLevel( 0, &lpSurface );
		
		RECT srcRect = { 0, 0, WEIGHT_TEX_RES, WEIGHT_TEX_RES };
		D3DXLoadSurfaceFromMemory( 
			lpSurface, NULL, NULL, src, BICUBIC_FORMAT, WEIGHT_TEX_RES * 4 * size,
			NULL, &srcRect, D3DX_FILTER_NONE, 0 );

		SAFE_RELEASE( lpSurface );
	}

	delete rFunc;
	delete rFunc16;
	delete rFunc16F;

	return 0;
}
#endif

#endif