#include "stdafx.h"
#include "GameApp/Dx11Util.h"
#include "GameApp/Util/EffectUtil.h"
#include "GameApp/GameApp.h"
#include "GameApp/BitWise.h"
#include "GameApp/GraphicsTypes.h"

#include "surface.h"
#include "lod_dx11.h"
#include "ProcTexTerrainRender.h"

//#include "bicubic.h"
#include "quadtree.h"

// resolution of the lookup tables
#define WEIGHT_TEX_RES	128

// choose one of the following options for 16-bit integer, 16-bit float oder 32-bit float (first and last work fine)
//#define BICUBIC_FORMAT		DXGI_FORMAT_R16G16B16A16_UINT
//#define BICUBIC_FORMAT		DXGI_FORMAT_R16G16B16A16_FLOAT
#define BICUBIC_FORMAT		DXGI_FORMAT_R32G32B32A32_FLOAT


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

// max render target size
static	DWORD				RT_SIZE = TREE_MAX_RESOLUTION;


int ProcTexTerrainRender::render_Init()
{
	//
	// load terrain
	//
	heightMapX = heightMapY = 513;
	heightMapXFull = heightMapYFull = 2049;

	FILE *f = fopen( "data/rainier2049.raw", "rb" );
	if ( f == NULL ) return -1;
	HeightMap16Full = new WORD[ heightMapXFull * heightMapYFull ];
	fread( HeightMap16Full, 1, heightMapXFull * heightMapYFull * sizeof( WORD ), f );
	fclose( f );

	// create low res heightmap for triangle mesh
	HeightMap16 = new WORD[ heightMapX * heightMapY ];

	int step = (heightMapXFull-1) / (heightMapX-1);
	for ( int y = 0; y < heightMapY; y++ )
		for ( int x = 0; x < heightMapX; x++ )
		{
			HeightMap16[ x + y * heightMapX ] = 
				HeightMap16Full[ Min( heightMapXFull-1, x * step ) + Min( heightMapYFull-1, y * step ) * heightMapXFull ];
		}

		//
		// create texturing
		// 
		createSurfaceNodes();
		
		CreateRenderTargets();

		Reset();
}

void ProcTexTerrainRender::CreateRenderTargets() {
	ColorRT = new RenderTarget2D;
	Color2RT = new RenderTarget2D;
	HeightCoverageRT = new RenderTarget2D;
	NormalRT = new RenderTarget2D;

	for( int i = 0; i < MAX_LAYER_DEPTH; i++ ) 
		HeightCoverageLayerRT[i] = new RenderTarget2D;

	// atlas textures (+ clear them)
	extern ATLAS atlas[ ATLAS_TEXTURES ];
	for ( int i = 0; i < ATLAS_TEXTURES; i++ )
		atlas[i].texture = new RenderTarget2D;
}

void	ProcTexTerrainRender::Reset() {
	ColorRT->Initialize(D3D11Dev(), RT_SIZE, RT_SIZE, DXGI_FORMAT_B8G8R8A8_UNORM);
	Color2RT->Initialize(D3D11Dev(), RT_SIZE, RT_SIZE, DXGI_FORMAT_B8G8R8A8_UNORM);
	HeightCoverageRT->Initialize(D3D11Dev(), RT_SIZE, RT_SIZE, DXGI_FORMAT_B8G8R8A8_UNORM);
	for( int i = 0; i < MAX_LAYER_DEPTH; i++ ) 
		HeightCoverageLayerRT[i]->Initialize(D3D11Dev(), RT_SIZE, RT_SIZE, DXGI_FORMAT_B8G8R8A8_UNORM);
	NormalRT->Initialize(D3D11Dev(), RT_SIZE, RT_SIZE, DXGI_FORMAT_B8G8R8A8_UNORM, 1, 1, 0, true);

	extern ATLAS atlas[ ATLAS_TEXTURES ];
	ID3D11RenderTargetView* pRTV;
	ID3D11DepthStencilView* pDSV;
	D3D11Context()->OMGetRenderTargets(1, &pRTV, &pDSV);

	for ( int i = 0; i < ATLAS_TEXTURES; i++ ) {
		atlas[i].texture->Initialize(D3D11Dev(), RT_SIZE, RT_SIZE, DXGI_FORMAT_B8G8R8A8_UNORM, 1, 1, 0, true);

		//static const D3D11_VIEWPORT vp2 = { 0,0, (float) RT_SIZE, (float) RT_SIZE, 0.0f, 1.0f };
		//D3D11Context()->RSSetViewports(1, &vp2);		
		//D3D11Context()->OMSetRenderTargets(1, &atlas[i].texture->RTView, NULL);
		D3D11Context()->ClearRenderTargetView(atlas[i].texture->RTView, vec4_zero.ToFloatPtr());

		//ID3D11RenderTargetView* pNULLRT = {NULL};
		//pContext->OMSetRenderTargets(1, &(pNULLRT), NULL);
	}



}

int	ProcTexTerrainRender::render_Quit() {
	SAFE_DELETE( HeightMap16 );

	destroyQLOD();
}

void ProcTexTerrainRender::CreateShaders()
{
	LoadEffectFromFile(D3D11Dev(), "data/shaders/terrascape.fx", &TerrainEffect);
			

	loadTextureFromFile("data/ambient.jpg", "occlusionTexture", D3D11Dev(), TerrainEffect);
	loadTextureFromFile("data/shadow.jpg", "shadowTexture", D3D11Dev(), TerrainEffect);
	loadTextureFromFile("data/noise16.png", "texNoiseMap0", D3D11Dev(), TerrainEffect);
	loadTextureFromFile("data/displace16.png", "texNoiseMap0", D3D11Dev(), TerrainEffect);

	HeightMapTex16 = new D3D11Texture2D;
	HeightMapTex16->Initialize(D3D11Dev(), heightMapXFull-1, heightMapXFull-1, DXGI_FORMAT_R16G16_UNORM);

	WORD *heightMapDouble = new WORD[ (heightMapXFull-1)*(heightMapXFull-1)*2 ];
	int i,j;
	for ( j = 0; j < (heightMapXFull-1); j++ )
		for ( i = 0; i < (heightMapXFull-1); i++ )
		{
			WORD h  = HeightMap16Full[ i +     (heightMapXFull-2-j) * heightMapXFull ];
			WORD h2 = HeightMap16Full[ i + 1 + (heightMapXFull-2-j) * heightMapXFull ];
			heightMapDouble[ (i + j * (heightMapXFull-1)) * 2 + 0 ] = h;
			heightMapDouble[ (i + j * (heightMapXFull-1)) * 2 + 1 ] = h2;
		}

		uint32 pitch;
		WORD*data = (WORD*)HeightMapTex16->Map(D3D11Context(), 0, pitch);
		memcpy(data, heightMapDouble, (heightMapXFull-1)*(heightMapXFull-1)*2 * sizeof(WORD));
		HeightMapTex16->Unmap(D3D11Context(), 0);

		delete heightMapDouble;

		// interpolation look-up textures
		if ( generateBiCubicWeights( D3D11Dev() ) )
			return;

		// create chunk lod quadtree
		createQLOD( D3D11Dev(), heightMapX, HeightMap16, HeightMapTex16 );
}

int ProcTexTerrainRender::generateBiCubicWeights( ID3D11Device* pD3DDevice )
{
	int	i, j;

	BicubicWeight03 = new D3D11Texture2D;
	BicubicWeight03->Initialize(D3D11Dev(),  WEIGHT_TEX_RES, WEIGHT_TEX_RES, BICUBIC_FORMAT);
	BicubicWeight47 = new D3D11Texture2D;
	BicubicWeight47->Initialize(D3D11Dev(),  WEIGHT_TEX_RES, WEIGHT_TEX_RES, BICUBIC_FORMAT);
	BicubicWeight8B = new D3D11Texture2D;
	BicubicWeight8B->Initialize(D3D11Dev(),  WEIGHT_TEX_RES, WEIGHT_TEX_RES, BICUBIC_FORMAT);
	BicubicWeightCF = new D3D11Texture2D;
	BicubicWeightCF->Initialize(D3D11Dev(),  WEIGHT_TEX_RES, WEIGHT_TEX_RES, BICUBIC_FORMAT);


	// !hack!
	noVec4 *rFunc    = new noVec4[ WEIGHT_TEX_RES * WEIGHT_TEX_RES ];
	WORD		*rFunc16  = new WORD       [ WEIGHT_TEX_RES * WEIGHT_TEX_RES * 4 ];
	WORD *rFunc16F = new WORD[ WEIGHT_TEX_RES * WEIGHT_TEX_RES * 4 ];

	for ( int n = -1; n <= 2; n++ )
	{
		for ( j = 0; j < WEIGHT_TEX_RES; j++ )
			for ( i = 0; i < WEIGHT_TEX_RES; i++ )
			{
				float dx = (float)(i+0) / (float)(WEIGHT_TEX_RES), dy = (float)(j+0) / (float)(WEIGHT_TEX_RES);

				noVec4 r;
				// m = -1..+2, n = -1
				r.x = R( -1.0f - dx ) * R( dy - (float)n );
				r.y = R( +0.0f - dx ) * R( dy - (float)n );
				r.z = R( +1.0f - dx ) * R( dy - (float)n );
				r.w = R( +2.0f - dx ) * R( dy - (float)n );

				//D3DXFloat32To16Array( &rFunc16F[ ( i + j * WEIGHT_TEX_RES ) * 4 ], (FLOAT*)&r, 4 );				
				for (int x=0; x < 4; ++x)				
					rFunc16F[ ( i + j * WEIGHT_TEX_RES ) +x] = Bitwise::floatToHalf(r[x]);

				rFunc16[ ( i + j * WEIGHT_TEX_RES ) * 4 + 0 ] = (WORD)(r.x * 65536.0f);
				rFunc16[ ( i + j * WEIGHT_TEX_RES ) * 4 + 1 ] = (WORD)(r.y * 65536.0f);
				rFunc16[ ( i + j * WEIGHT_TEX_RES ) * 4 + 2 ] = (WORD)(r.z * 65536.0f);
				rFunc16[ ( i + j * WEIGHT_TEX_RES ) * 4 + 3 ] = (WORD)(r.w * 65536.0f);

				rFunc[ i + j * WEIGHT_TEX_RES ] = r;
			}
											
#if BICUBIC_FORMAT == DXGI_FORMAT_R32G32B32A32_FLOAT;
			void *src = rFunc;
			int  size = sizeof( float );
#elif BICUBIC_FORMAT == DXGI_FORMAT_R16G16B16A16_FLOAT;
			void *src = rFunc16F;
			int  size = sizeof( WORD );
#elif BICUBIC_FORMAT == DXGI_FORMAT_R16G16B16A16_UINT;
			void *src = rFunc16;
			int  size = sizeof( WORD );
#endif

			D3D11Texture2D* dst;
			switch ( n )
			{
			case -1: dst = BicubicWeight03;	break;
			case +0: dst = BicubicWeight47;	break;
			case +1: dst = BicubicWeight8B;	break;
			case +2: dst = BicubicWeightCF;	break;
			};
									
#if BICUBIC_FORMAT == DXGI_FORMAT_R32G32B32A32_FLOAT;
			RECT srcRect = { 0, 0, WEIGHT_TEX_RES, WEIGHT_TEX_RES };
			uint32 pitch;
			float* mem = (float*) dst->Map(D3D11Context(), 0, pitch);
			memcpy(mem, src, size * WEIGHT_TEX_RES * WEIGHT_TEX_RES * 4);
			dst->Unmap(D3D11Context(), 0);

#elif (BICUBIC_FORMAT == DXGI_FORMAT_R16G16B16A16_FLOAT || BICUBIC_FORMAT == DXGI_FORMAT_R16G16B16A16_UINT)
			RECT srcRect = { 0, 0, WEIGHT_TEX_RES, WEIGHT_TEX_RES };
			uint32 pitch;
			WORD* mem = (WORD*) dst->Map(D3D11Context(), 0, pitch);
			memcpy(mem, src, size * WEIGHT_TEX_RES * WEIGHT_TEX_RES * 4);
			dst->Unmap(D3D11Context(), 0);
#endif 	
	}
	delete rFunc;
	delete rFunc16;
	delete rFunc16F;

	return 0;
}


// a vertex and index buffer is used to batch all texture tiles to be updated.
typedef struct
{
	float x, y, z;
	float coordHMBias1, coordHMBias2,
		coordHMScale1, coordHMScale2,
		coordTileBias1, coordTileBias2,
		coordTileScale1, coordTileScale2,
		scale1, scale2;
}TILEVERTEX;

int nTempQuads = 0;
TILEVERTEX	*quadList;

// adds a quad to the temporary vertex buffer
void	addQuad( TERRAINTILE *tile )
{
	quadList[ nTempQuads * 4 + 0 ].x = 0.0f; quadList[ nTempQuads * 4 + 0 ].y = 0.0f;
	quadList[ nTempQuads * 4 + 1 ].x = 0.0f; quadList[ nTempQuads * 4 + 1 ].y = 1.0f;
	quadList[ nTempQuads * 4 + 2 ].x = 1.0f; quadList[ nTempQuads * 4 + 2 ].y = 0.0f;
	quadList[ nTempQuads * 4 + 3 ].x = 1.0f; quadList[ nTempQuads * 4 + 3 ].y = 1.0f;

	for ( int i = 0; i < 4; i++ )
	{
		quadList[ nTempQuads * 4 + i ].z = (float)tile->tileSize;
		quadList[ nTempQuads * 4 + i ].coordHMBias1    = tile->coordHM[0];
		quadList[ nTempQuads * 4 + i ].coordHMBias2    = tile->coordHM[1];
		quadList[ nTempQuads * 4 + i ].coordHMScale1   = tile->coordHM[2];
		quadList[ nTempQuads * 4 + i ].coordHMScale2   = tile->coordHM[3];
		quadList[ nTempQuads * 4 + i ].coordTileBias1  = tile->coordTile[0];
		quadList[ nTempQuads * 4 + i ].coordTileBias2  = tile->coordTile[1];
		quadList[ nTempQuads * 4 + i ].coordTileScale1 = tile->coordTile[2];
		quadList[ nTempQuads * 4 + i ].coordTileScale2 = tile->coordTile[3];

		float realSizeM = tile->worldSize / (float)(heightMapX-1) / (float)tile->tileSize * 15300.0f * 2.6777777f;
		float heightScale = 65536.0f * 0.1f;
		quadList[ nTempQuads * 4 + i ].scale1 = heightScale / realSizeM;
		quadList[ nTempQuads * 4 + i ].scale2 = realSizeM / heightScale;
	}

	nTempQuads ++;
}

void	ProcTexTerrainRender::addTextureLayerRoot( ID3D11Device* pD3DDevice, 
	ATLAS *atlas,
	PROCTEX_SURFACEPARAM &surface, 		
	ID3D11RenderTargetView*		dstColorRTV,
	ID3D11RenderTargetView*		dstHeightCoverageRTV) {

}

void ProcTexTerrainRender::addTextureLayer( ID3D11Device* pD3DDevice, 
	ATLAS *atlas,
	PROCTEX_SURFACEPARAM &surface, 		
	ID3D11ShaderResourceView*	srcColor,
	ID3D11ShaderResourceView*	srcHeight,
	ID3D11ShaderResourceView*	srcCoverage,
	ID3D11RenderTargetView*		dstColorRTV,
	ID3D11RenderTargetView*		dstHeightCoverageRTV) {

}

#define NO_MORE_ON_THIS_LEVEL	1
static RenderTarget2D* lastHeightCoverage = NULL;

void ProcTexTerrainRender::computeTexturesRecursive( ID3D11Device* pD3DDevice, ATLAS *atlas, PROCTEX_SURFACEPARAM &surface, int depth, 
	RenderTarget2D* currentCoverage, int additionalFlags) {
		
		// if this is our first layer, we do not need to evaluate constraints, but resample the height map
		if ( depth == 0 )
		{
			addTextureLayerRoot( pD3DDevice, atlas, surface, Color2RT->RTView, HeightCoverageLayerRT[ depth ]->RTView );
			lastHeightCoverage = HeightCoverageLayerRT[ depth ];
		}
		// ok, just add another layer
		{
			addTextureLayer( pD3DDevice, atlas, surface,
				Color2RT->SRView, HeightCoverageRT->SRView, HeightCoverageLayerRT[ depth - 1]->SRView, 
				Color2RT->RTView, HeightCoverageLayerRT[ depth ]->RTView );
			lastHeightCoverage = HeightCoverageLayerRT[ depth ];
		}

		// and don't forget child surface nodes
		for ( int i = 0; i < surface.nChilds; i++ )
		{
			int addFlags = 0;

			if ( i == surface.nChilds - 1 )
				if ( additionalFlags & NO_MORE_ON_THIS_LEVEL || depth == 0 )
					addFlags |= NO_MORE_ON_THIS_LEVEL;

			computeTexturesRecursive( pD3DDevice, atlas, *surface.child[ i ], depth + 1, HeightCoverageLayerRT[ depth - 1 ], addFlags );
		}
}

void	ProcTexTerrainRender::computeLightingLayer( ID3D11Device* pD3DDevice, 
	ATLAS *atlas,
	ID3D11ShaderResourceView *srcColor,
	ID3D11ShaderResourceView *srcHeight,
	ID3D11RenderTargetView *dstColor ) {

}


// this method handles the update of all atlas textures
void ProcTexTerrainRender::computeTextures( ID3D11Device* pD3DDevice ) {
	extern ATLAS atlas[ ATLAS_TEXTURES ];

	terrainBaseColor = rootSurfaceNode->spColor;

	for ( int i = 0; i < ATLAS_TEXTURES; i++ )
		if ( atlas[ i ].count > 0 ) 		{
			computeTexturesRecursive( pD3DDevice, &atlas[ i ], *rootSurfaceNode );
			computeLightingLayer( pD3DDevice, &atlas[ i ], Color2RT->SRView
				, lastHeightCoverage->SRView, atlas[ i ].texture->RTView );
		}
}



void ProcTexTerrainRender::Render(float fTime, float fDeltaTime)  {

	HRESULT	hr;

	noVec4 camPos, camDir, camDst;


	TerrainEffect->GetVariableByName("cameraPosition")->AsVector()->SetFloatVector(camPos.ToFloatPtr());
	TerrainEffect->GetVariableByName("cameraDirection")->AsVector()->SetFloatVector(camDir.ToFloatPtr());

	// take another surface description ?
	extern int surfaceSelect, qualitySelect;
	static int lastSurfaceType = 0;
	static int lastQuality     = 5;

	if ( lastSurfaceType != surfaceSelect || lastQuality != qualitySelect )
	{
		lastSurfaceType = surfaceSelect;
		lastQuality     = qualitySelect;
		FlushTextures = 1;
		::createSurfaceNodes();
		rootSurfaceNode = surfaceTypes[ surfaceSelect ];
	}

	// update atlas textures, flush all, if another surface type is selected
	if ( FlushTextures )
		::freeAllTexturesQLOD( 0, 0 );

	::traverseQLOD( D3D11Dev(), camPos, camDst, 0, 0 );
	FlushTextures = 0;

	computeTextures( D3D11Dev() );

	//
	// render terrain
	//

}