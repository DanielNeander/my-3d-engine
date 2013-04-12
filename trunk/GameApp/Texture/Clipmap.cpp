#include "stdafx.h"
#include "Clipmap.h"
#include "GameApp/Dx11Util.h"
#include "GameApp/Util/EffectUtil.h"
#include "GameApp/GameApp.h"
#include "GameApp/BitWise.h"
#include "GameApp/GraphicsTypes.h"
#include "GameApp/TGALoader.h"


#define AssertFatal(cond, msg) if(!(cond)) { printf("Assertion: " msg); OutputDebugStringA("Assertion: " msg); exit(1); }

#define FILE_BLOCK_SIZE 512

#define CLIPMAP_STACK_SIZE_MAX 4096
#define CLIPMAP_STACK_SIZE_MIN 1024
#define MIPMAP_LEVELS_MAX 7

const LPCSTR RENDER_TECHNIQUES[] =
{
	"Trilinear",
	"Anisotropic",
	"ColoredMips",
};

float MipmapColors[MIPMAP_LEVELS_MAX][3] =
{
	{0.5f, 1.0f, 0.0f},
	{0.0f, 1.0f, 1.0f},
	{1.0f, 1.0f, 0.0f},
	{0.0f, 1.0f, 0.0f},
	{0.0f, 0.0f, 1.0f},
	{1.0f, 0.0f, 1.0f},
	{1.0f, 0.0f, 0.0f},
};

JPEG_Manager JPEManager;


Clipmaps::Clipmaps( ID3D11Device* Dev, ID3D11DeviceContext* context )
{
	Device = Dev;
	Context = context;
	ClipmapStackSize = 2048;
	UpdateRegionSize = 64;
	RenderingTechnique = 1;

	PyramidTexture      = NULL; // Texture which is used to store coarse mip levels
	PyramidTextureSRV   = NULL;
	PyramidTextureHM    = NULL; // Height map for pyramid texture
	PyramidTextureHMSRV = NULL;
	StackTexture        = NULL; // Clipmap stack texture
	StackTextureSRV     = NULL;	
}

bool Clipmaps::Init()
{
	LoadShaders();
		
	WCHAR diffuseFile[MAX_PATH], hmFile[MAX_PATH];	
	for ( int i = 0; i < SOURCE_FILES_NUM; ++i)
	{		
		swprintf_s(diffuseFile, MAX_PATH, L"%s%d.jpg",TexnamePrefix.c_str(), i);
		swprintf_s(hmFile, MAX_PATH, L"%shm%d.jpg",TexnamePrefix.c_str(), i);	
		wmemcpy_s(DstMediaPath[i], wcslen(diffuseFile), diffuseFile, wcslen(diffuseFile));		
		DstMediaPath[i][wcslen(diffuseFile)] = L'\0';
		wmemcpy_s(DstMediaPathHM[i], wcslen(hmFile), hmFile, wcslen(hmFile));
		DstMediaPathHM[i][wcslen(hmFile)] = L'\0';
	}

	D3DX11_IMAGE_INFO imageInfo;
	HRESULT hr;
	D3DX11GetImageInfoFromFileW( DstMediaPath[0], NULL, &imageInfo, &hr );

	SourceImageWidth = imageInfo.Width;
	SourceImageHeight = imageInfo.Height;
	
	UINT baseDimension = CLIPMAP_STACK_SIZE_MAX;

	while( baseDimension >= CLIPMAP_STACK_SIZE_MIN )
	{
		StackSizeList.push_back(baseDimension);
		baseDimension >>= 1;
	}
	
	return true;
}

void Clipmaps::Reset()
{

}

void Clipmaps::LoadShaders()
{
	//LoadEffectFromFile( Device, "Data/Shaders/Clipmaps.fx", &ClipmapEffect );	

}

void Clipmaps::Process( int clipmapSize )
{
	ClipmapStackSize = clipmapSize;
	
	
		
	CalculateClipmapParameters();	
	int blocksPerLayer = 0;
	blocksPerLayer = SourceImageWidth / UpdateRegionSize;

	UpdateRegionSize = (int)( 16.0 * pow( 2.0f, StackDepth - 1 ) );

	JPEManager.Intitialize( StackDepth, DstMediaPath, DstMediaPathHM );
	JPEManager.AllocateBlocks( blocksPerLayer );
	JPEManager.AllocateTextures( Device, ClipmapStackSize, UpdateRegionSize );

	CreateClipmapTextures( Device );
	InitStackTexture( Device );

	
	//ClipmapEffect->GetVariableByName( "SphereMeridianSlices" )->AsScalar()->SetInt( SPHERE_MERIDIAN_SLICES_NUM );
	//ClipmapEffect->GetVariableByName( "SphereParallelSlices" )->AsScalar()->SetInt( SPHERE_PARALLEL_SLICES_NUM );
}

//--------------------------------------------------------------------------------------
// Calculate base parameters using defined sizes
//--------------------------------------------------------------------------------------
void Clipmaps::CalculateClipmapParameters()
{
		

	StackDepth = 0;
	int dimensionMax = SourceImageWidth;

	if( SourceImageHeight > SourceImageWidth )
		dimensionMax = SourceImageHeight;

	SourceImageMipsNum = int( log( (double)dimensionMax + 1.0 ) / log( 2.0 ) ) + 1;

	for( int i = 0; i < dimensionMax; ++i )
	{
		if( dimensionMax / pow(2.0f, StackDepth) > ClipmapStackSize)
		{
			StackDepth++;
		}
		else
		{
			break;
		}
	}

	SourceImageMipsSize = new int*[SourceImageMipsNum];

	for( int i = 0; i < SourceImageMipsNum; ++i )
	{
		SourceImageMipsSize[i] = new int[2];

		SourceImageMipsSize[i][0] = SourceImageWidth / (unsigned)pow( 2.0f, i );
		SourceImageMipsSize[i][1] = SourceImageHeight / (unsigned)pow( 2.0f, i );
	}

	StackPosition.x = 0.0f;
	StackPosition.y = 0.5f;

	assert(StackDepth);

	MipDataOffset = new int[StackDepth];
	UpdatePositions = new int*[StackDepth];

	for( int i = 0; i < StackDepth; ++i )
	{
		UpdatePositions[i] = new int[2];

		UpdatePositions[i][0] = 0;
		UpdatePositions[i][1] = 0;
	}

	ClipmapEffect->GetVariableByName( "StackDepth" )->AsScalar()->SetInt( StackDepth );

	D3DXVECTOR2 scaleFactor;
	scaleFactor.x = (float)SourceImageWidth / ClipmapStackSize;
	scaleFactor.y = (float)SourceImageHeight / ClipmapStackSize;
	ClipmapEffect->GetVariableByName( "ScaleFactor" )->AsVector()->SetFloatVectorArray( scaleFactor, 0, 2 );

	int textureSize[2];
	textureSize[0] = SourceImageWidth;
	textureSize[1] = SourceImageHeight;

	ClipmapEffect->GetVariableByName( "TextureSize" )->AsVector()->SetIntVectorArray( (int*)textureSize, 0, 2 );
	ClipmapEffect->GetVariableByName( "MipColors" )->AsVector()->SetFloatVectorArray( (float*)MipmapColors, 0, MIPMAP_LEVELS_MAX * 3 );

}

HRESULT Clipmaps::CreateClipmapTextures( ID3D11Device* pd3dDevice )
{
	HRESULT hr;
	D3DX11_IMAGE_INFO imageInfo;
	D3DX11_IMAGE_LOAD_INFO imageLoadInfo;

	D3DX11GetImageInfoFromFileW( DstMediaPath[StackDepth], NULL, &imageInfo, &hr );

	imageLoadInfo.Width = imageInfo.Width;
	imageLoadInfo.Height = imageInfo.Height;
	imageLoadInfo.Depth = imageInfo.Depth;
	imageLoadInfo.FirstMipLevel = 0;
	imageLoadInfo.MipLevels = SourceImageMipsNum - StackDepth;
	imageLoadInfo.MiscFlags = imageInfo.MiscFlags;
	imageLoadInfo.Format = imageInfo.Format;
	imageLoadInfo.Usage = D3D11_USAGE_DEFAULT;
	imageLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	imageLoadInfo.CpuAccessFlags = 0;
	imageLoadInfo.pSrcInfo = &imageInfo;

	D3DX11CreateTextureFromFileW( pd3dDevice, DstMediaPath[StackDepth], &imageLoadInfo, NULL, (ID3D11Resource**)&PyramidTexture, &hr );

	D3D11_TEXTURE2D_DESC texDesc;
	PyramidTexture->GetDesc( &texDesc );

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
	ZeroMemory( &SRVDesc, sizeof(SRVDesc) );
	SRVDesc.Format = texDesc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = texDesc.MipLevels;
	SRVDesc.Texture2D.MostDetailedMip = 0;

	( pd3dDevice->CreateShaderResourceView( PyramidTexture, &SRVDesc, &PyramidTextureSRV ) );

	D3DX11GetImageInfoFromFileW( DstMediaPathHM[StackDepth], NULL, &imageInfo, &hr );

	imageLoadInfo.Width = imageInfo.Width;
	imageLoadInfo.Height = imageInfo.Height;
	imageLoadInfo.Depth = imageInfo.Depth;
	imageLoadInfo.FirstMipLevel = 0;
	imageLoadInfo.MipLevels = SourceImageMipsNum - StackDepth;
	imageLoadInfo.MiscFlags = imageInfo.MiscFlags;
	imageLoadInfo.Format = imageInfo.Format;
	imageLoadInfo.Usage = D3D11_USAGE_DEFAULT;
	imageLoadInfo.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	imageLoadInfo.CpuAccessFlags = 0;
	imageLoadInfo.pSrcInfo = &imageInfo;

	D3DX11CreateTextureFromFileW( pd3dDevice, DstMediaPathHM[StackDepth], &imageLoadInfo, NULL, (ID3D11Resource**)&PyramidTextureHM, &hr );

	PyramidTexture->GetDesc( &texDesc );
	ZeroMemory( &SRVDesc, sizeof(SRVDesc) );
	SRVDesc.Format = texDesc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture2D.MipLevels = texDesc.MipLevels;
	SRVDesc.Texture2D.MostDetailedMip = 0;

	( pd3dDevice->CreateShaderResourceView( PyramidTextureHM, &SRVDesc, &PyramidTextureHMSRV ) );

	ZeroMemory( &texDesc, sizeof(texDesc) );
	texDesc.ArraySize = StackDepth;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.Width = ClipmapStackSize;
	texDesc.Height = ClipmapStackSize;
	texDesc.MipLevels = 1;
	texDesc.SampleDesc.Count = 1;

	( pd3dDevice->CreateTexture2D(&texDesc, NULL, &StackTexture) );

	ZeroMemory( &SRVDesc, sizeof(SRVDesc) );
	SRVDesc.Format = texDesc.Format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	SRVDesc.Texture2DArray.ArraySize = StackDepth;
	SRVDesc.Texture2DArray.FirstArraySlice = 0;
	SRVDesc.Texture2DArray.MipLevels = 1;
	SRVDesc.Texture2DArray.MostDetailedMip = 0;

	( pd3dDevice->CreateShaderResourceView( StackTexture, &SRVDesc, &StackTextureSRV ) );

	/*ClipmapEffect->GetVariableByName( "PyramidTexture" )->AsShaderResource()->SetResource( PyramidTextureSRV );
	ClipmapEffect->GetVariableByName( "PyramidTextureHM" )->AsShaderResource()->SetResource( PyramidTextureHMSRV );
	ClipmapEffect->GetVariableByName( "StackTexture" )->AsShaderResource()->SetResource( StackTextureSRV );*/

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Perform stack texture updates based on a new viewer's position
//--------------------------------------------------------------------------------------
void Clipmaps::InitStackTexture( ID3D11Device* pd3dDevice )
{
	int mipCornerLU[2];
	int mipCornerRD[2];
	int tileBlockSize = 0;
	int blockIndex[2];
	int subBlockIndex[2];
	int blockCorner[2];

	D3D11_BOX SubResourceBox;
	SubResourceBox.front = 0;
	SubResourceBox.back = 1;

	int srcBlock[2];
	int dstBlock[2];

	for( int i = 0; i < StackDepth; ++i )
	{
		mipCornerLU[0] = int(StackPosition.x * SourceImageMipsSize[i][0] - ClipmapStackSize* 0.5f);
		mipCornerLU[1] = int(StackPosition.y * SourceImageMipsSize[i][1] - ClipmapStackSize* 0.5f);

		mipCornerRD[0] = int(StackPosition.x * SourceImageMipsSize[i][0] + ClipmapStackSize* 0.5f);    
		mipCornerRD[1] = int(StackPosition.y * SourceImageMipsSize[i][1] + ClipmapStackSize* 0.5f);

		tileBlockSize = int(UpdateRegionSize / pow(2.0, i));

		SubResourceBox.top = 0;
		SubResourceBox.bottom = SubResourceBox.top + tileBlockSize;

		if( SourceImageMipsSize[i][1] == ClipmapStackSize )
		{
			UpdatePositions[i][1] = ClipmapStackSize / 2;

			for( int j = mipCornerLU[1] + ClipmapStackSize / 2; j < mipCornerRD[1]; j += tileBlockSize )
			{
				SubResourceBox.left = 0;
				SubResourceBox.right = SubResourceBox.left + tileBlockSize;

				blockIndex[1] = j / FILE_BLOCK_SIZE;
				blockCorner[1] = blockIndex[1] * FILE_BLOCK_SIZE;
				subBlockIndex[1] = ( j - blockCorner[1] ) / tileBlockSize;

				for( int k = mipCornerLU[0]; k < mipCornerRD[0]; k += tileBlockSize )
				{
					int tempBoundary = k;

					if( tempBoundary < 0 )
						tempBoundary += SourceImageMipsSize[i][0];

					blockIndex[0] = tempBoundary / FILE_BLOCK_SIZE;

					blockCorner[0] = blockIndex[0] * FILE_BLOCK_SIZE;
					subBlockIndex[0] = ( tempBoundary - blockCorner[0] ) / tileBlockSize; 

					srcBlock[0] = tempBoundary;
					srcBlock[1] = j;

					dstBlock[0] = SubResourceBox.left;
					dstBlock[1] = SubResourceBox.top;

					JPEManager.AddBlock( i, srcBlock, dstBlock );

					SubResourceBox.left += tileBlockSize;
					SubResourceBox.right += tileBlockSize;
				}

				JPEManager.Update( Context, i, StackTexture );

				SubResourceBox.top += tileBlockSize;
				SubResourceBox.bottom += tileBlockSize;
			}

			SubResourceBox.top = ClipmapStackSize/ 2;
			SubResourceBox.bottom = SubResourceBox.top + tileBlockSize;

			for( int j = mipCornerLU[1]; j < mipCornerRD[1] - ClipmapStackSize* 0.5; j += tileBlockSize )
			{
				SubResourceBox.left = 0;
				SubResourceBox.right = SubResourceBox.left + tileBlockSize;

				blockIndex[1] = j / FILE_BLOCK_SIZE;
				blockCorner[1] = blockIndex[1] * FILE_BLOCK_SIZE;
				subBlockIndex[1] = ( j - blockCorner[1] ) / tileBlockSize;

				for( int k = mipCornerLU[0]; k < mipCornerRD[0]; k += tileBlockSize )
				{
					int tempBoundary = k;

					if( tempBoundary < 0 )
						tempBoundary += SourceImageMipsSize[i][0];

					blockIndex[0] = tempBoundary / FILE_BLOCK_SIZE;

					blockCorner[0] = blockIndex[0] * FILE_BLOCK_SIZE;
					subBlockIndex[0] = ( tempBoundary - blockCorner[0] ) / tileBlockSize; 

					srcBlock[0] = tempBoundary;
					srcBlock[1] = j;

					dstBlock[0] = SubResourceBox.left;
					dstBlock[1] = SubResourceBox.top;

					JPEManager.AddBlock( i, srcBlock, dstBlock );

					SubResourceBox.left += tileBlockSize;
					SubResourceBox.right += tileBlockSize;
				}

				JPEManager.Update( Context, i , StackTexture);

				SubResourceBox.top += tileBlockSize;
				SubResourceBox.bottom += tileBlockSize;
			}
		}
		else
		{
			for( int j = mipCornerLU[1]; j < mipCornerRD[1]; j += tileBlockSize )
			{
				SubResourceBox.left = 0;
				SubResourceBox.right = SubResourceBox.left + tileBlockSize;

				blockIndex[1] = j / FILE_BLOCK_SIZE;
				blockCorner[1] = blockIndex[1] * FILE_BLOCK_SIZE;
				subBlockIndex[1] = ( j - blockCorner[1] ) / tileBlockSize;

				for( int k = mipCornerLU[0]; k < mipCornerRD[0]; k += tileBlockSize )
				{
					int tempBoundary = k;

					if( tempBoundary < 0 )
						tempBoundary += SourceImageMipsSize[i][0];

					blockIndex[0] = tempBoundary / FILE_BLOCK_SIZE;

					blockCorner[0] = blockIndex[0] * FILE_BLOCK_SIZE;
					subBlockIndex[0] = ( tempBoundary - blockCorner[0] ) / tileBlockSize; 

					srcBlock[0] = tempBoundary;
					srcBlock[1] = j;

					dstBlock[0] = SubResourceBox.left;
					dstBlock[1] = SubResourceBox.top;

					JPEManager.AddBlock( i, srcBlock, dstBlock );

					SubResourceBox.left += tileBlockSize;
					SubResourceBox.right += tileBlockSize;
				}

				JPEManager.Update( Context, i , StackTexture);

				SubResourceBox.top += tileBlockSize;
				SubResourceBox.bottom += tileBlockSize;
			}
		}
	}
}

void Clipmaps::UpdateStackTexture( ID3D11Device* pd3dDevice )
{
	D3DXVECTOR2 updateBorder;
	float length = sqrtf( EyePosition.x * EyePosition.x + EyePosition.z * EyePosition.z );
	float posHorizontal;
	float posVertical;

	// Positions are calculated in order to map current viewer's position
	// to a rectangular region of a source image mapped on a spherical mesh.
	// Positions are represented by values clamped to [0 - 1] range in each dimension
	if( EyePosition.z >= 0 )
	{
		if( EyePosition.x <= 0 )
			posHorizontal = atanf( -( EyePosition.x / EyePosition.z ) ) / (noMath::PI * 2);
		else
			posHorizontal = 1.0f - atanf( EyePosition.x / EyePosition.z ) / (noMath::PI * 2);
	}
	else
	{
		posHorizontal = 0.5f - atanf((EyePosition.x / EyePosition.z)) / (noMath::PI * 2);
	}

	posVertical = 0.5f - atanf(EyePosition.y / length) / noMath::PI;

	int srcBlock[2];
	int dstBlock[2];

	// Calculate border sizes to be updated
	updateBorder.x = posHorizontal - StackPosition.x;

	if( updateBorder.x > 0 )
	{
		if( updateBorder.x > 0.5f )
			updateBorder.x = updateBorder.x - 1.0f;
	}
	else
	{
		if( updateBorder.x < -0.5f )
			updateBorder.x += 1.0f; 
	}

	updateBorder.y = posVertical - StackPosition.y;

	int updateBorderSize[2];
	updateBorderSize[0] = int( updateBorder.x * SourceImageWidth );
	updateBorderSize[1] = int( updateBorder.y * SourceImageHeight );

	int tileBlockSize = 0;
	int mipCornerLU[2];
	int mipCornerRD[2];

	D3D11_BOX SubResourceBox;
	SubResourceBox.front = 0;
	SubResourceBox.back = 1;

	unsigned correction = 0;

	// Update square region when we move "Image space Right"
	if( updateBorderSize[0] > UpdateRegionSize )
	{
		// Update all clipmap stack layers one by one
		for( int i = 0; i < StackDepth; ++i )
		{            
			tileBlockSize = UpdateRegionSize / (unsigned)pow(2.0, i);

			// Calculate regions of source image that should be updated in the current stack layer
			mipCornerLU[0] = int(StackPosition.x * SourceImageMipsSize[i][0]) + ClipmapStackSize / 2;
			if( mipCornerLU[0] > int(SourceImageMipsSize[i][0] - tileBlockSize) )
				mipCornerLU[0] -= SourceImageMipsSize[i][0];
			mipCornerRD[0] = mipCornerLU[0] + tileBlockSize;

			mipCornerLU[1] = int(StackPosition.y * SourceImageMipsSize[i][1]) - ClipmapStackSize / 2;
			mipCornerRD[1] = int(StackPosition.y * SourceImageMipsSize[i][1]) + ClipmapStackSize / 2;

			if( mipCornerLU[1] < 0 )
			{
				correction = -mipCornerLU[1];
				mipCornerLU[1] = 0;
			}
			else if( mipCornerLU[1] >= SourceImageMipsSize[i][1] )
			{
				UpdateMipPosition(UpdatePositions[i][0], tileBlockSize);
				continue;
			}

			if( mipCornerRD[1] <= 0 )
			{
				UpdateMipPosition(UpdatePositions[i][0], tileBlockSize);
				continue;
			}
			else if( mipCornerRD[1] > SourceImageMipsSize[i][1] )
				mipCornerRD[1] = SourceImageMipsSize[i][1];

			SubResourceBox.left = UpdatePositions[i][0];
			SubResourceBox.right = SubResourceBox.left + tileBlockSize;
			SubResourceBox.top = UpdatePositions[i][1] + correction;

			for( int j = mipCornerLU[1]; j < mipCornerRD[1]; j += tileBlockSize )
			{
				SubResourceBox.bottom = SubResourceBox.top + tileBlockSize;

				srcBlock[0] = mipCornerLU[0];
				srcBlock[1] = j;

				dstBlock[0] = SubResourceBox.left;
				dstBlock[1] = SubResourceBox.top;

				JPEManager.AddBlock( i, srcBlock, dstBlock );

				SubResourceBox.top += tileBlockSize;

				if( SubResourceBox.top > UINT(ClipmapStackSize - tileBlockSize) )
					SubResourceBox.top = 0;
			}

			JPEManager.Update( Context, i , StackTexture);
			UpdateMipPosition( UpdatePositions[i][0], tileBlockSize );
		}

		StackPosition.x += (float)UpdateRegionSize / SourceImageWidth;
	}

	// Update square region when we move "Image space Left"
	if( updateBorderSize[0] < -UpdateRegionSize )
	{
		for( int i = 0; i < StackDepth; ++i )
		{
			tileBlockSize = UpdateRegionSize / (unsigned)pow(2.0, i);

			UpdateMipPosition(UpdatePositions[i][0], -tileBlockSize);

			mipCornerLU[0] = int(StackPosition.x * SourceImageMipsSize[i][0]) - ClipmapStackSize / 2 - tileBlockSize;
			if( mipCornerLU[0] < 0 )
				mipCornerLU[0] += SourceImageMipsSize[i][0];
			mipCornerRD[0] = mipCornerLU[0] + tileBlockSize;

			mipCornerLU[1] = int(StackPosition.y * SourceImageMipsSize[i][1]) - ClipmapStackSize / 2;
			mipCornerRD[1] = int(StackPosition.y * SourceImageMipsSize[i][1]) + ClipmapStackSize / 2;

			if( mipCornerLU[1] < 0 )
			{
				correction = -mipCornerLU[1];
				mipCornerLU[1] = 0;
			}
			else if( mipCornerLU[1] >= SourceImageMipsSize[i][1] )
				continue;

			if( mipCornerRD[1] <= 0 )
				continue;
			else if( mipCornerRD[1] > SourceImageMipsSize[i][1] )
				mipCornerRD[1] = SourceImageMipsSize[i][1];

			SubResourceBox.left = UpdatePositions[i][0];
			SubResourceBox.right = SubResourceBox.left + tileBlockSize;
			SubResourceBox.top = UpdatePositions[i][1] + correction;

			for( int j = mipCornerLU[1]; j < mipCornerRD[1]; j += tileBlockSize )
			{
				SubResourceBox.bottom = SubResourceBox.top + tileBlockSize;

				srcBlock[0] = mipCornerLU[0];
				srcBlock[1] = j;

				dstBlock[0] = SubResourceBox.left;
				dstBlock[1] = SubResourceBox.top;

				JPEManager.AddBlock( i, srcBlock, dstBlock );

				SubResourceBox.top += tileBlockSize;

				if( SubResourceBox.top > UINT(ClipmapStackSize - tileBlockSize) )
					SubResourceBox.top = 0;
			}

			JPEManager.Update( Context, i , StackTexture);
		}

		StackPosition.x -= (float)UpdateRegionSize / SourceImageWidth;
	}

	if( StackPosition.x < 0.0f )
		StackPosition.x = 1.0f + StackPosition.x;
	else if( StackPosition.x > 1.0f )
		StackPosition.x -= 1.0f;

	// Update square region when we move "Image space Down"
	if( updateBorderSize[1] > UpdateRegionSize )
	{
		for( int i = 0; i < StackDepth; ++i )
		{
			correction = 0;

			tileBlockSize = UpdateRegionSize / (unsigned)pow(2.0, i);

			mipCornerLU[0] = int(StackPosition.x * SourceImageMipsSize[i][0]) - ClipmapStackSize / 2;
			if( mipCornerLU[0] < 0 )
				mipCornerLU[0] += SourceImageMipsSize[i][0];

			int stepsNum = ClipmapStackSize / tileBlockSize;

			mipCornerLU[1] = int(StackPosition.y * SourceImageMipsSize[i][1]) + ClipmapStackSize / 2;
			mipCornerRD[1] = mipCornerLU[1] + tileBlockSize;

			if( mipCornerLU[1] < 0 )
				mipCornerLU[1] = 0;
			else if( mipCornerLU[1] >= SourceImageMipsSize[i][1] )
			{
				UpdateMipPosition(UpdatePositions[i][1], tileBlockSize);
				continue;
			}

			if( mipCornerRD[1] <= 0 )
				continue;
			else if( mipCornerRD[1] > SourceImageMipsSize[i][1] )
				mipCornerRD[1] = SourceImageMipsSize[i][1];

			SubResourceBox.top = UpdatePositions[i][1];
			SubResourceBox.bottom = SubResourceBox.top + tileBlockSize;
			SubResourceBox.left = UpdatePositions[i][0];

			while( SubResourceBox.left > UINT(ClipmapStackSize - tileBlockSize) )
				SubResourceBox.left -= ClipmapStackSize;

			for( int step = 0, j = mipCornerLU[0]; step < stepsNum; step++, j += tileBlockSize )
			{
				if( j == SourceImageMipsSize[i][0] )
					j = 0;

				SubResourceBox.right = SubResourceBox.left + tileBlockSize;

				srcBlock[0] = j;
				srcBlock[1] = mipCornerLU[1];

				dstBlock[0] = SubResourceBox.left;
				dstBlock[1] = SubResourceBox.top;

				JPEManager.AddBlock( i, srcBlock, dstBlock );

				SubResourceBox.left += tileBlockSize;

				if( SubResourceBox.left > UINT( ClipmapStackSize - tileBlockSize ) )
					SubResourceBox.left = 0;
			}

			JPEManager.Update( Context, i , StackTexture);
			UpdateMipPosition( UpdatePositions[i][1], tileBlockSize );
		}

		StackPosition.y += (float)UpdateRegionSize / SourceImageHeight;
	}

	// Update square region when we move "Image space Up"
	if( updateBorderSize[1] < -UpdateRegionSize )
	{
		for( int i = 0; i < StackDepth; ++i )
		{
			correction = 0;

			tileBlockSize = UpdateRegionSize / (unsigned)pow(2.0, i);

			mipCornerLU[0] = int(StackPosition.x * SourceImageMipsSize[i][0]) - ClipmapStackSize / 2;
			if( mipCornerLU[0] < 0 )
				mipCornerLU[0] += SourceImageMipsSize[i][0];

			int stepsNum = ClipmapStackSize / tileBlockSize;

			UpdateMipPosition(UpdatePositions[i][1], -tileBlockSize);

			mipCornerLU[1] = int(StackPosition.y * SourceImageMipsSize[i][1]) - ClipmapStackSize / 2 - tileBlockSize;
			if( mipCornerLU[1] < 0 )
			{
				mipCornerLU[1] += SourceImageMipsSize[i][1];
			}

			mipCornerRD[1] = mipCornerLU[1] + tileBlockSize;

			if( mipCornerRD[1] <= 0 )
				continue;
			else if( mipCornerRD[1] > SourceImageMipsSize[i][1] )
				mipCornerRD[1] = SourceImageMipsSize[i][1];

			SubResourceBox.top = UpdatePositions[i][1];
			SubResourceBox.bottom = SubResourceBox.top + tileBlockSize;
			SubResourceBox.left = UpdatePositions[i][0];

			while( SubResourceBox.left > UINT(ClipmapStackSize - tileBlockSize) )
				SubResourceBox.left -= ClipmapStackSize;

			for( int step = 0, j = mipCornerLU[0]; step < stepsNum; step++, j += tileBlockSize )
			{
				if( j == SourceImageMipsSize[i][0] )
					j = 0;

				SubResourceBox.right = SubResourceBox.left + tileBlockSize;

				srcBlock[0] = j;
				srcBlock[1] = mipCornerLU[1];

				dstBlock[0] = SubResourceBox.left;
				dstBlock[1] = SubResourceBox.top;

				JPEManager.AddBlock( i, srcBlock, dstBlock );

				SubResourceBox.left += tileBlockSize;

				if( SubResourceBox.left > UINT(ClipmapStackSize - tileBlockSize) )
					SubResourceBox.left = 0;
			}

			JPEManager.Update( Context, i , StackTexture);
		}

		StackPosition.y -= (float)UpdateRegionSize / SourceImageHeight;
	}

	if( StackPosition.y < 0.0f )
		StackPosition.y = 1.0f + StackPosition.y;
	else if( StackPosition.y > 1.0f )
		StackPosition.y -= 1.0f;
}

//--------------------------------------------------------------------------------------
// Calculate new positions of updatable regions in each clipmap stack level.
// Positions define the upper left corner of updatable square region
//--------------------------------------------------------------------------------------
void Clipmaps::UpdateMipPosition( int &position, int offset )
{
	position += offset;

	if( offset > 0 )
	{
		if( position > ClipmapStackSize - offset)
			position = 0;
	}
	else
	{
		if( position < 0 )
			position = ClipmapStackSize + offset;
	}    
}


void Clipmaps::UpdateClipMap()
{
	EyePosition = GetApp()->ActiveCam_->GetFrom(); 

	UpdateStackTexture( Device );	
}

void Clipmaps::SetClipmapVars()
{

	ClipmapEffect->GetVariableByName( "g_StackDepth" )->AsScalar()->SetInt( StackDepth );

	D3DXVECTOR2 scaleFactor;
	scaleFactor.x = (float)SourceImageWidth / ClipmapStackSize;
	scaleFactor.y = (float)SourceImageHeight / ClipmapStackSize;
	ClipmapEffect->GetVariableByName( "g_ScaleFactor" )->AsVector()->SetFloatVector( &scaleFactor.x);

	int textureSize[2];
	textureSize[0] = SourceImageWidth;
	textureSize[1] = SourceImageHeight;

	ClipmapEffect->GetVariableByName( "g_TextureSize" )->AsVector()->SetIntVector( (int*)textureSize);
	ClipmapEffect->GetVariableByName( "g_MipColors" )->AsVector()->SetFloatVectorArray( (float*)MipmapColors, 0, MIPMAP_LEVELS_MAX );

	ClipmapEffect->GetVariableByName( "g_StackCenter" )->AsVector()->SetFloatVector( (float*)StackPosition );

	ClipmapEffect->GetVariableByName( "PyramidTexture" )->AsShaderResource()->SetResource( PyramidTextureSRV );
	ClipmapEffect->GetVariableByName( "PyramidTextureHM" )->AsShaderResource()->SetResource( PyramidTextureHMSRV );
	ClipmapEffect->GetVariableByName( "StackTexture" )->AsShaderResource()->SetResource( StackTextureSRV );
}

void Clipmaps::ReleaseTextures()
{
	ClipmapEffect->GetVariableByName( "PyramidTexture" )->AsShaderResource()->SetResource( NULL );
	ClipmapEffect->GetVariableByName( "PyramidTextureHM" )->AsShaderResource()->SetResource( NULL );
	ClipmapEffect->GetVariableByName( "StackTexture" )->AsShaderResource()->SetResource( NULL );

}

bool Clipmap::Init() {
	
	TGAImg img;
	img.Load(SrcTexutreName);	
		
		
	csmSourceImageSize = img.GetWidth();
	csmSourceImageMipCount = noMath::ILog2(csmSourceImageSize); // log2 of csmSourceImageSize

	csmClipMapSize = csmSourceImageSize / 16;
	csmClipMapShift = noMath::ILog2(csmClipMapSize); // log2 of csmClipMapSize;

	// Fill in some default information for all layers.
	for(int i=0; i<csmLayerCount; i++)
	{
		// Set up a centered position.
		mLayer[i].mCenterX = 0.5f;
		mLayer[i].mCenterY = 0.5f; 
		mLayer[i].mToroidalOffset.x = ((csmClipMapSize/2) << i) - csmClipMapSize/2;
		mLayer[i].mToroidalOffset.y = ((csmClipMapSize/2) << i) - csmClipMapSize/2;
		mLayer[i].mScale = float(1<<i);
	}
	
	// Recreate appropriate textures to upload into.	
	for(int i=0; i<csmLayerCount; i++) 	{
		mLayer[i].mTex = new D3D11Texture2D;
		mLayer[i].mTex->Initialize(D3D11Dev(), csmClipMapSize, csmClipMapSize, 
			DXGI_FORMAT_B8G8R8X8_UNORM, D3D11_BIND_SHADER_RESOURCE);
	}
		
	return true;
}

void Clipmap::reset() {
	for(int i=0; i<csmLayerCount; i++)
	{
		if(mLayer[i].mTex)
		{
			SAFE_RELEASE(mLayer[i].mTexSRV);
		}
		
	}
	bindTextures();
}

void Clipmap::alloc( ID3D11Device *device ) {
	// Just to be safe...
	reset();
	
	//bindTextures();
	fillWithTextureData();
}

void Clipmap::bindTextures() {

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVdesc;
	SRVdesc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
	SRVdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVdesc.Texture2D.MipLevels = 1;
	SRVdesc.Texture2D.MostDetailedMip = 0;

	for(int i=0; i<csmLayerCount; i++) 	
		D3D11Dev()->CreateShaderResourceView( mLayer[i].mTex->Texture, &SRVdesc, &mLayer[i].mTexSRV );

	ClipmapEffect->GetVariableByName("clipTexture0")->AsShaderResource()->SetResource(mLayer[0].mTexSRV);
	ClipmapEffect->GetVariableByName("clipTexture1")->AsShaderResource()->SetResource(mLayer[1].mTexSRV);
	ClipmapEffect->GetVariableByName("clipTexture2")->AsShaderResource()->SetResource(mLayer[2].mTexSRV);
	ClipmapEffect->GetVariableByName("clipTexture3")->AsShaderResource()->SetResource(mLayer[3].mTexSRV);
	ClipmapEffect->GetVariableByName("clipTexture4")->AsShaderResource()->SetResource(mLayer[4].mTexSRV);	
}

void Clipmap::updateEffectParams() {
	if (!ClipmapEffect) return;

	HRESULT hr;

	float clipArray[csmLayerCount*3];

	for(int i=0; i<csmLayerCount; i++)
	{
		clipArray[i*3+0] = mLayer[i].mCenterX * mLayer[i].mScale;
		clipArray[i*3+1] = mLayer[i].mCenterY * mLayer[i].mScale;
		clipArray[i*3+2] = mLayer[i].mScale;
	}
	
	ClipmapEffect->GetVariableByName("g_clipLayerAndCenter")->AsVector()->SetFloatVectorArray(clipArray, 0, csmLayerCount);
}

void Clipmap::generateMip32Bit( unsigned char *srcLevel, int srcSize, unsigned char *outLevel )
{
	int r,g,b,a,offset;

	int halfSrcSize = srcSize >> 1;
	for(int x=0; x<halfSrcSize; x++)
	{
		for(int y=0; y<halfSrcSize; y++)
		{
			// Sample the four source pixels and average them to generate
			// out pixel.
			offset = (2*x*srcSize+2*y)*4;
			r = srcLevel[offset + 0];
			g = srcLevel[offset + 1];
			b = srcLevel[offset + 2];
			a = srcLevel[offset + 3];

			offset += 4;
			r += srcLevel[offset + 0];
			g += srcLevel[offset + 1];
			b += srcLevel[offset + 2];
			a += srcLevel[offset + 3];

			offset += srcSize*4;
			r += srcLevel[offset + 0];
			g += srcLevel[offset + 1];
			b += srcLevel[offset + 2];
			a += srcLevel[offset + 3];

			offset += 4;
			r += srcLevel[offset + 0];
			g += srcLevel[offset + 1];
			b += srcLevel[offset + 2];
			a += srcLevel[offset + 3];

			// Now average down.
			r >>= 2;
			g >>= 2;
			b >>= 2;
			a >>= 2;

			// And store
			offset = (x*halfSrcSize+y)*4;
			outLevel[offset+0] = r;
			outLevel[offset+1] = g;
			outLevel[offset+2] = b;
			outLevel[offset+3] = a;
		}
	}
}

void Clipmap::fillSourceData() {

	// Only fill it once.
	if(mRawImage)
		return;

	// Allocate storage for an RGBA image with mips. We also note what
	// the offset from the image base will be for each mip level so that
	// we just have to add the image base offset to the contents of mRawMips.
	int storageSize = 0;
	int storageDim = csmSourceImageSize;
	int mipCount = 0;
	while(storageDim > 1)
	{
		mRawMips[mipCount++] = (unsigned char*)storageSize;
		storageSize += storageDim * storageDim * 4;
		storageDim >>= 1;
	}

	mRawImage = new unsigned char[storageSize];

	for(int i=0; i<csmSourceImageMipCount; i++)
		mRawMips[i] = mRawMips[i] + (int)mRawImage;

#if 0
	// Now walk the array filling with a test pattern.
	unsigned char *ptr = mRawImage;

	for(int i=0; i<csmSourceImageSize; i++)
	{
		for(int j=0; j<csmSourceImageSize; j++)
		{
			// Generate a fill pattern.
			(*ptr++) = unsigned char(i & 0xFF);
			(*ptr++) = unsigned char(j & 0xFF);
			(*ptr++) = unsigned char(((i+j) & 0x0F) << 4);
			(*ptr++) = unsigned char(0);
		}
	}
#else
	TGAImg image;
	int res = image.Load(SrcTexutreName.c_str());
			

	if(res != IMG_OK)
	{
		AssertFatal(false, "Image 'texture.tga' failed to load.");
	}
	
	// We need to flip endianness. :)
	image.BGRtoRGB();

	// This will be 24bpp, so have to swizzle it into our buffer.
	unsigned char *imgBits = image.GetImg();

	unsigned char *outBits = mRawImage;
	unsigned char *outBitsEnd = mRawImage + (csmSourceImageSize * csmSourceImageSize * 4);

	while(outBits != outBitsEnd)
	{
		if (image.GetBPP() == 24) {		
			*(outBits++) = *(imgBits++);
			*(outBits++) = *(imgBits++);
			*(outBits++) = *(imgBits++);
			*(outBits++) = 0;
		}
		else if (image.GetBPP() == 32) {		
			*(outBits++) = *(imgBits++);
			*(outBits++) = *(imgBits++);
			*(outBits++) = *(imgBits++);
			*(outBits++) = *(imgBits++);
		}
	}
#endif
	// Great - the base mip is filled with data but we need to generate
	// the mip levels so we can copy out of them for clipmap updates.

	// Let's do something simple - braindead box filter.
	for(int i=0; i<csmSourceImageMipCount-1; i++)
		generateMip32Bit(mRawMips[i], csmSourceImageSize >> i, mRawMips[i+1]);

	// And we're done.
}

void Clipmap::clipAgainstGrid( const int gridSpacing, const RectI &rect, int *outCount, RectI *outBuffer )
{
	// Check against X grids...
	const int startX = rect.point.x;
	const int endX   = rect.point.x + rect.extent.x;

	const int gridMask = ~(gridSpacing-1);
	const int startGridX = startX & gridMask;
	const int endGridX   = endX   & gridMask;

	RectI buffer[2];
	int rectCount = 0;

	// Check X...
	if(startGridX != endGridX && endX - endGridX > 0)
	{
		// We have a clip! Split against the grid multiple and store.
		rectCount = 2;
		buffer[0].point.x  = startX;
		buffer[0].point.y  = rect.point.y;
		buffer[0].extent.x = endGridX - startX;
		buffer[0].extent.y = rect.extent.y;

		buffer[1].point.x  = endGridX;
		buffer[1].point.y  = rect.point.y;
		buffer[1].extent.x = endX - endGridX;
		buffer[1].extent.y = rect.extent.y;
	}
	else
	{
		// Copy it in.
		rectCount = 1;
		buffer[0] = rect;
	}

	// Now, check Y for the one or two rects we have from above.
	*outCount = 0;
	for(int i=0; i<rectCount; i++)
	{
		// Figure our extent and grid information.
		const int startY = buffer[i].point.y;
		const int endY   = buffer[i].point.y + rect.extent.y;
		const int startGridY = startY & gridMask;
		const int endGridY   = endY   & gridMask;

		if(startGridY != endGridY && endY - endGridY > 0)
		{
			// We have a clip! Split against the grid multiple and store.
			RectI *buffA = outBuffer + *outCount;
			RectI *buffB = buffA + 1;
			(*outCount) += 2;

			buffA->point.x  = buffer[i].point.x;
			buffA->point.y  = startY;
			buffA->extent.x = buffer[i].extent.x;
			buffA->extent.y = endGridY - startY;

			buffB->point.x  = buffer[i].point.x;
			buffB->point.y  = endGridY;
			buffB->extent.x = buffer[i].extent.x;
			buffB->extent.y = endY - endGridY;
		}
		else
		{
			// Copy it in.
			outBuffer[*outCount] = buffer[i];
			(*outCount)++;
		}      
	}
}

void Clipmap::calculateModuloDeltaBounds( const RectI &oldData, const RectI &newData, RectI *outRects, int *outRectCount )
{
	// Sanity checking.
	AssertFatal(oldData.point.x >= 0 && oldData.point.y >= 0 && oldData.isValidRect(),
		"ClipMap::calculateModuloDeltaBounds - negative oldData origin or bad rect!");

	AssertFatal(newData.point.x >= 0 && newData.point.y >= 0 && newData.isValidRect(),
		"ClipMap::calculateModuloDeltaBounds - negative newData origin or bad rect!");

	AssertFatal(newData.extent.x == oldData.extent.x && newData.extent.y == oldData.extent.y, 
		"ClipMap::calculateModuloDeltaBounts - mismatching extents, can only work with matching extents!");

	// Easiest case - if they're the same then do nothing.
	if(oldData.point.x == newData.point.x && oldData.point.y == newData.point.y)
	{
		*outRectCount = 0;
		return;
	}

	// Easy case - if there's no overlap then it's all new!
	if(!oldData.overlaps(newData))
	{
		// Clip out to return buffer, and we're done.
		clipAgainstGrid(csmClipMapSize, newData, outRectCount, outRects);
		return;
	}

	// Calculate some useful values for both X and Y. Delta is used a lot
	// in determining bounds, and the boundary values are important for
	// determining where to start copying new data in.
	const int xDelta = newData.point.x - oldData.point.x;
	const int yDelta = newData.point.y - oldData.point.y;

	const int xBoundary = (oldData.point.x + oldData.extent.x) % csmClipMapSize;
	const int yBoundary = (oldData.point.y + oldData.extent.y) % csmClipMapSize;

	AssertFatal(xBoundary % csmClipMapSize == oldData.point.x % csmClipMapSize, 
		"ClipMap::calculateModuleDeltaBounds - we assume that left and "
		"right of the dataset are identical (ie, it's periodical on size of clipmap!) (x)");

	AssertFatal(yBoundary % csmClipMapSize == oldData.point.y % csmClipMapSize, 
		"ClipMap::calculateModuleDeltaBounds - we assume that left and "
		"right of the dataset are identical (ie, it's periodical on size of clipmap!) (y)");

	// Now, let's build up our rects. We have one rect if we are moving
	// on the X or Y axis, two if both. We dealt with the no-move case
	// previously.
	if(xDelta == 0)
	{
		// Moving on Y! So generate and store clipped results.
		RectI yRect;

		if(yDelta < 0)
		{
			// We need to generate the box from right of old to right of new.
			yRect.point = newData.point;
			yRect.extent.x = csmClipMapSize;
			yRect.extent.y = -yDelta;
		}
		else
		{
			// We need to generate the box from left of old to left of new.
			yRect.point.x = newData.point.x; // Doesn't matter which rect we get this from.
			yRect.point.y = (oldData.point.y + oldData.extent.y);
			yRect.extent.x = csmClipMapSize;
			yRect.extent.y = yDelta;
		}

		// Clip out to return buffer, and we're done.
		clipAgainstGrid(csmClipMapSize, yRect, outRectCount, outRects);

		return;
	}
	else if(yDelta == 0)
	{
		// Moving on X! So generate and store clipped results.
		RectI xRect;

		if(xDelta < 0)
		{
			// We need to generate the box from right of old to right of new.
			xRect.point = newData.point;
			xRect.extent.x = -xDelta;
			xRect.extent.y = csmClipMapSize;
		}
		else
		{
			// We need to generate the box from left of old to left of new.
			xRect.point.x = (oldData.point.x + oldData.extent.x);
			xRect.point.y = newData.point.y; // Doesn't matter which rect we get this from.
			xRect.extent.x = xDelta;
			xRect.extent.y = csmClipMapSize;
		}

		// Clip out to return buffer, and we're done.
		clipAgainstGrid(csmClipMapSize, xRect, outRectCount, outRects);

		return;
	}
	else
	{
		// Both! We have an L shape. So let's do the bulk of it in one rect,
		// and the remainder in the other. We'll choose X as the dominant axis.
		//
		// a-----b---------c   going from e to a.
		// |     |         |
		// |     |         |
		// d-----e---------f   So the dominant rect is abgh and the passive
		// |     |         |   rect is bcef. Obviously depending on delta we
		// |     |         |   have to switch things around a bit.
		// |     |         |          y+ ^
		// |     |         |             |  
		// g-----h---------i   x+->      |

		RectI xRect, yRect;

		if(xDelta < 0)
		{
			// Case in the diagram.
			xRect.point = newData.point;
			xRect.extent.x = -xDelta;
			xRect.extent.y = csmClipMapSize;

			// Set up what of yRect we know, too.
			yRect.point.x = xRect.point.x + xRect.extent.x;
			yRect.extent.x = csmClipMapSize - abs(xDelta); 
		}
		else
		{
			// Opposite of case in diagram!
			xRect.point.x = oldData.point.x + oldData.extent.x;
			xRect.point.y = newData.point.y;
			xRect.extent.x = xDelta;
			xRect.extent.y = csmClipMapSize;

			// Set up what of yRect we know,  too.
			yRect.point.x = (xRect.point.x + xRect.extent.x )- csmClipMapSize;
			yRect.extent.x = csmClipMapSize - xRect.extent.x;
		}

		if(yDelta < 0)
		{
			// Case in the diagram.
			yRect.point.y = newData.point.y;
			yRect.extent.y = -yDelta;
		}
		else
		{
			// Opposite of case in diagram!
			yRect.point.y = oldData.point.y + oldData.extent.y;
			yRect.extent.y = yDelta;
		}

		// Make sure we don't overlap.
		AssertFatal(!yRect.overlaps(xRect), 
			"ClipMap::calculateModuloDeltaBounds - have overlap in result rects!");

		// Ok, now run them through the clipper.
		int firstCount;
		clipAgainstGrid(csmClipMapSize, xRect, &firstCount, outRects);
		clipAgainstGrid(csmClipMapSize, yRect, outRectCount, outRects + firstCount);
		*outRectCount += firstCount;

		// All done!
		return;
	}
}

void Clipmap::uploadToTexture( int level, RectI srcRegion, RectI dstRegion ) 
{
	// Lock dstRegion in the texture.
	D3D11_BOX inR;

	inR.front = 0;
	inR.back = 1;
	
	inR.top = dstRegion.point.x;
	inR.bottom = dstRegion.point.x + dstRegion.extent.x;

	inR.left = dstRegion.point.y;
	inR.right = dstRegion.point.y + dstRegion.extent.y;
		
	// Texture PixelFormat 
	PixelBox pb(mLayer[level].mTex->Width, mLayer[level].mTex->Height, 0, PF_X8R8G8B8);	
	
	Box box(inR.left, inR.top, inR.right, inR.bottom);
	PixelBox subBox = pb.getSubVolume(box);	
	uint32 pitch = subBox.rowPitch * PixelUtil::getNumElemBytes(subBox.format);
#if 0
	// Fill region with a debug color.
	static char debugCol = 0;
	debugCol = rand();
	for(int i=0; i<dstRegion.extent.x; i++)
		memset((char*)lockRect.pBits + i * lockRect.Pitch, debugCol, 4 * dstRegion.extent.y);
#else
	// Fill region with data from the source texture - so first make sure
	// it's loaded!
	fillSourceData();
	// Now, blt some data over. Note we have to do some x/y swaps to get thing
	// to work out properly. Also the mip level we upload from is based on the
	// specified CSE level - so we have to adjust from clip layers into source
	// image mips. This is a row-wise copy - one memcpy per row of data.

	const int selectedMip = csmLayerCount - level - 1;
	const unsigned char *mipLayer = mRawMips[selectedMip];
	unsigned char* srcData = new unsigned char [ dstRegion.extent.x * 4 * dstRegion.extent.y ];
	//unsigned char* srcData = (unsigned char*)Mem_Alloc(( dstRegion.extent.x+1) * pitch * (dstRegion.extent.y) * sizeof(unsigned char));


	for(int i=0; i<dstRegion.extent.x; i++)
		memcpy(srcData + i *  dstRegion.extent.y * 4,
		mipLayer + ((srcRegion.point.x + i) * (csmSourceImageSize>>selectedMip) + srcRegion.point.y) * 4,
		4 * dstRegion.extent.y
		);
#endif	
	D3D11Context()->UpdateSubresource(mLayer[level].mTex->Texture, 0, &inR, srcData, 4 * dstRegion.extent.y, 
		0);	

	SAFE_DELETE(srcData);
	//Mem_Free(srcData);
}

void Clipmap::recenter( float centerX, float centerY )
{
	
      // Ok, we're going to do toroidal updates on each entry of the clipstack
      // (except for the cap, which covers the whole texture), based on this
      // new center point.

      // For each texture...
      for(int i=csmLayerCount-1; i>=0; i--)
      {
         // Calculate new center point for this texture.
         float texelCenterFX = centerX * float(csmClipMapSize) * mLayer[i].mScale;
         float texelCenterFY = centerY * float(csmClipMapSize) * mLayer[i].mScale;

         const int texelMin = csmClipMapSize/2;
         const int texelMax = int(float(csmClipMapSize) * mLayer[i].mScale) - texelMin;

         Point2I texelTopLeft(
            clamp(int(floor(texelCenterFY)), texelMin, texelMax) - texelMin,
            clamp(int(floor(texelCenterFX)), texelMin, texelMax) - texelMin);

         // This + current toroid offset tells us what regions have to be blasted.
         RectI oldData(mLayer[i].mToroidalOffset,  Point2I(csmClipMapSize, csmClipMapSize));
         RectI newData(texelTopLeft,         Point2I(csmClipMapSize, csmClipMapSize));

         // This would be a good place to check if data is available (if you
         // implement paging) and abort if it's not present.

         // Alright, determine the set of data we actually need to upload.
         RectI buffer[8];
         int   rectCount = 0;

         calculateModuloDeltaBounds(oldData, newData, buffer, &rectCount);
         AssertFatal(rectCount < 8, "ClipMap::recenter - got too many rects back!");

         mLayer[i].mToroidalOffset = texelTopLeft;
         mLayer[i].mCenterX = centerX;
         mLayer[i].mCenterY = centerY;

         /*     if(rectCount)
         printf("    issuing %d updates to clipmap level %d (offset=%dx%d)", 
         rectCount, i, texelTopLeft.x, texelTopLeft.y); */

         if(rectCount)
         {
            //printf("layer %x, %d updates", &cse,  rectCount);

            // And GO!
            for(int j=0; j<rectCount; j++)
            {
               AssertFatal(buffer[j].isValidRect(),"ClipMap::recenter - got invalid rect!");

               // Note the rect, so we can then wrap and let the image cache do its thing.
               RectI srcRegion = buffer[j];
               buffer[j].point.x = srcRegion.point.x % csmClipMapSize;
               buffer[j].point.y = srcRegion.point.y % csmClipMapSize;

               AssertFatal(newData.contains(srcRegion), 
                  "ClipMap::recenter - got update buffer outside of expected new data bounds.");

               // Upload new data to the texture.
               uploadToTexture(i, srcRegion, buffer[j]);

               // This is where you'd update the number of texels uploaded.
            }
         }

         // If you were doing texel budgeting, this is where you'd abort on
         // overrun of budget.
      }
}

/// This is called when we want to completely fill every layer; for instance
/// for the first frame or as part of a device reset.
void Clipmap::fillWithTextureData()
{
	// For each texture...
      for(int i=csmLayerCount-1; i>=0; i--)
      {
         // This + current toroid offset tells us what regions have to be blasted.
         RectI desiredData(mLayer[i].mToroidalOffset,  Point2I(csmClipMapSize, csmClipMapSize));

         // Alright, determine the set of data we actually need to upload.
         RectI buffer[8];
         int   rectCount = 0;

         clipAgainstGrid(csmClipMapSize, desiredData, &rectCount, buffer);
         AssertFatal(rectCount < 8, "ClipMap::fillWithTextureData - got too many rects back!");

         /*     if(rectCount)
         printf("    issuing %d updates to clipmap level %d (offset=%dx%d)", 
         rectCount, i, texelTopLeft.x, texelTopLeft.y); */

         if(rectCount)
         {
            //printf("layer %x, %d updates", &cse,  rectCount);

            // And GO!
            for(int j=0; j<rectCount; j++)
            {
               AssertFatal(buffer[j].isValidRect(),"ClipMap::fillWithTextureData - got invalid rect!");

               // Note the rect, so we can then wrap and let the image cache do its thing.
               RectI srcRegion = buffer[j];
               buffer[j].point.x = srcRegion.point.x % csmClipMapSize;
               buffer[j].point.y = srcRegion.point.y % csmClipMapSize;

               // Upload new data to the texture.
               uploadToTexture(i, srcRegion, buffer[j]);

               // This is where you'd update the number of texels uploaded.
            }
         }
      }
}
