#include "stdafx.h"
#include "GameApp/GameApp.h"
#include <GameApp/Util/EffectUtil.h>
#include "JPEG_Preprocessor.h"
#include "JPEG_Decoder.h"
#include <string>

#include "Clipmap.h"

ID3DX11Effect* g_pEffectJPEG;
////////////////////////////////////////////////////////////////////////////////////
// JPEG_Data class
////////////////////////////////////////////////////////////////////////////////////

JPEG_Data::JPEG_Data()
{
    fileHandle = NULL;
    mappingHandle = NULL;
}

HRESULT JPEG_Data::LoadFile( LPWSTR fileName )
{
    Pjpeg_decoder_file_stream Pinput_stream = new jpeg_decoder_file_stream();

    if( !Pinput_stream->open(fileName) )
    {
        return S_FALSE;
    }
    
    Pjpeg_decoder Pd = new jpeg_decoder(Pinput_stream);
    Pd->begin();

    std::wstring temporaryName = std::wstring(fileName) + L"_UNCOMPRESSED";
    
    fileHandle = CreateFileW( temporaryName.c_str(),
                            GENERIC_ALL,
                            NULL,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_FLAG_RANDOM_ACCESS | FILE_FLAG_DELETE_ON_CLOSE,
                            NULL );
                            
    if( !fileHandle )
    {
        return S_FALSE;
    }
    
    Pd->dump_DCT_data( fileHandle );
    
    mappingHandle = CreateFileMapping(
                    fileHandle,
                    NULL,
                    PAGE_READONLY,
                    0,
                    0,
                    NULL );

    pFileData = (char*)MapViewOfFile(
                    mappingHandle,
                    FILE_MAP_READ,
                    0,
                    0,
                    0 );
    
    imageWidth = Pd->get_width();
    imageHeight = Pd->get_height();
    
    componentsNum = Pd->getScanComponentsNum();
    
    static float scaleFactor[8] = { 1.0f, 1.387039845f, 1.306562965f, 1.175875602f,
                                    1.0f, 0.785694958f, 0.541196100f, 0.275899379f };
    
    for( int i = 0; i < componentsNum; i++ )
    {
        QUANT_TYPE *pTable = Pd->getQuantizationTable( i );

        for( int row = 0; row < 8; row++ )
        {
            for( int col = 0; col < 8; col++ )
            {
                quantTables[i][row * 8 + col] = BYTE(pTable[row * 8 + col] * scaleFactor[row] * scaleFactor[col]);
            }
        } 
    
        MCU_per_row = Pd->get_mcus_per_row();
    }
        
    MCU_blocks_num = Pd->get_blocks_per_mcu();
    
    delete Pd;
    delete Pinput_stream;
    
    return S_OK;
}


HRESULT JPEG_Data::AllocateTextures( ID3D11Device* pd3dDevice, int stackSize, int border )
{
    HRESULT hr;
    
    if( componentsNum == 1 )
    {
        TextureData[0].Intitialize( pd3dDevice, stackSize, border, quantTables[0] );
            
        return S_OK;
    }
    else if( componentsNum == 3 )
    {   
        ( TextureData[0].Intitialize( pd3dDevice, stackSize, border, quantTables[0] ) );
        ( TextureData[1].Intitialize( pd3dDevice, stackSize / 2, border / 2, quantTables[1] ) );
        ( TextureData[2].Intitialize( pd3dDevice, stackSize / 2, border / 2, quantTables[2] ) );
        
        return S_OK;
    }
    
    return S_FALSE;
}


// Prepare texture data for IDCT pass
HRESULT JPEG_Data::UpdateTextureData( ID3D11DeviceContext* pd3dContext, int blocksNum, int blockSize, int *pSrcCorners )
{
    int MCU_SIZE = 8;
    int MCU_BLOCK_SIZE = 64;
    
    int MCU_ROW;
    int MCU_COLUMN;

    if( componentsNum == 3 )
    {
        MCU_SIZE = 16;
        MCU_BLOCK_SIZE = 384;
    }
        
    int MCU_NUM_X = blockSize / MCU_SIZE;
    int MCU_NUM_Y = blockSize / MCU_SIZE;
    
    int updatesNum = 0;
    
    if( componentsNum == 1 )
    {
        D3D11_BOX box;

        box.front = 0;
        box.back = 1;
                
        for( int i = 0; i < blocksNum; i++ )
        {
            MCU_ROW = pSrcCorners[ i * 2 + 1 ] / MCU_SIZE;
            MCU_COLUMN = pSrcCorners[ i * 2 ] / MCU_SIZE;
            
            box.top = 0;
            box.bottom = 8;
            
            for( int j = MCU_ROW; j < MCU_ROW + MCU_NUM_Y; j++ )
            {
                box.left = ( i * MCU_NUM_X ) * 8;
                box.right = box.left + 8;
                
                for( int k = MCU_COLUMN; k < MCU_COLUMN + MCU_NUM_X; k++ )
                {
                    int offset = MCU_BLOCK_SIZE * ( j * MCU_per_row  + k );
                    pd3dContext->UpdateSubresource( TextureData[0].pTextureDCT, 0, &box, pFileData + offset, 8, 0 );
                    
                    box.left += 8;
                    box.right += 8;
                }
                
                box.top += 8;
                box.bottom += 8;
            }
        }   
    }
    else if( componentsNum == 3 )
    {
        D3D11_BOX box[2];

        box[0].front = box[1].front = 0;
        box[0].back = box[1].back = 1;
        
        int corner[2];
        
        for( int i = 0; i < blocksNum; i++ )
        {
            MCU_ROW = pSrcCorners[ i * 2 + 1 ] / MCU_SIZE;
            MCU_COLUMN = pSrcCorners[ i * 2 ] / MCU_SIZE;
            
            box[1].top = 0;
            box[1].bottom = 8;
            
            corner[1] = 0;
            
            for( int j = MCU_ROW; j < MCU_ROW + MCU_NUM_Y; j++ )
            {
                corner[0] = i * MCU_NUM_X * 16;
                
                box[1].left = i * MCU_NUM_X * 8;
                box[1].right = box[1].left + 8;
                                
                for( int k = MCU_COLUMN; k < MCU_COLUMN + MCU_NUM_X; k++ )
                {
                    int offset = MCU_BLOCK_SIZE * ( j * MCU_per_row + k );
                    
                    box[0].top = corner[1];
                    box[0].bottom = corner[1] + 8;
                    
                    box[0].left = corner[0];
                    box[0].right = corner[0] + 8;
                    
                    pd3dContext->UpdateSubresource( TextureData[0].pTextureDCT, 0, &box[0], pFileData + offset +   0, 8, 0 );
                    
                    box[0].left += 8;
                    box[0].right += 8;
                   
                    pd3dContext->UpdateSubresource( TextureData[0].pTextureDCT, 0, &box[0], pFileData + offset +  64, 8, 0 );
                    
                    box[0].top += 8;
                    box[0].bottom += 8;
                    
                    box[0].left = corner[0];
                    box[0].right = corner[0] + 8;
                    
                    pd3dContext->UpdateSubresource( TextureData[0].pTextureDCT, 0, &box[0], pFileData + offset + 128, 8, 0 );
                    
                    box[0].left += 8;
                    box[0].right += 8;
                    
                    pd3dContext->UpdateSubresource( TextureData[0].pTextureDCT, 0, &box[0], pFileData + offset + 192, 8, 0 );
                    
                    pd3dContext->UpdateSubresource( TextureData[1].pTextureDCT, 0, &box[1], pFileData + offset + 256, 8, 0 );
                    pd3dContext->UpdateSubresource( TextureData[2].pTextureDCT, 0, &box[1], pFileData + offset + 320, 8, 0 );
                    updatesNum++;
                    
                    box[1].left += 8;
                    box[1].right += 8;
                    
                    corner[0] += 16;
                }
                
                box[1].top += 8;
                box[1].bottom += 8;
                
                corner[1] += 16;
            }
        }   
    }
    
    return S_OK;
}


// Perform IDCT on a single texture chanel 
void JPEG_Data::UncompressTextureData( ID3D11DeviceContext* d3dContext)
{
    D3D11_VIEWPORT customViewports[2];
    
    customViewports[0].TopLeftX = 0;
    customViewports[0].TopLeftY = 0;
    customViewports[0].MinDepth = 0.0f;
    customViewports[0].MaxDepth = 1.0f;
    
    ID3D11RenderTargetView *Views[2];
    
    ID3DX11EffectTechnique *technique = g_pEffectJPEG->GetTechniqueByName( "JPEG_Decompression" );
    
    static ID3D11Buffer *pBuffers[] = {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
    static unsigned pStrides[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    static unsigned pOffsets[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        
    d3dContext->IASetInputLayout( NULL );
    d3dContext->IASetIndexBuffer( NULL, DXGI_FORMAT_R32_UINT, 0 );
    d3dContext->IASetVertexBuffers(0, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT, pBuffers, pStrides, pOffsets );
    d3dContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_POINTLIST );
    
    D3D11_TEXTURE2D_DESC desc;
    
    ID3DX11EffectScalarVariable *pRowScale = g_pEffectJPEG->GetVariableByName( "g_RowScale" )->AsScalar();
    ID3DX11EffectScalarVariable *pColScale = g_pEffectJPEG->GetVariableByName( "g_ColScale" )->AsScalar();
    
    ID3D11ShaderResourceView *ppSRV[2];
    
    for( int i = 0; i < componentsNum; i++ )
    {
        TextureData[i].pTextureTarget->GetDesc( &desc );
        pRowScale->SetFloat( desc.Width / 8.0f );
        pColScale->SetFloat( desc.Height / 8.0f );
                
        /////////////////////////////////////////////////////////////////////////////
        // Pass1
        /////////////////////////////////////////////////////////////////////////////
        
        technique->GetPassByIndex( 0 )->Apply( 0, d3dContext );
        
        Views[0] = TextureData[i].pTexture1RowRTV;
        Views[1] = TextureData[i].pTexture2RowRTV;
        
        TextureData[i].pTexture1Row->GetDesc( &desc );
        customViewports[0].Width = desc.Width;
        customViewports[0].Height = desc.Height;
        customViewports[1] = customViewports[0];
        
        d3dContext->RSSetViewports( 2, customViewports );
        d3dContext->OMSetRenderTargets( 2, Views, NULL );

        D3D11_TEXTURE2D_DESC desc;
        TextureData[i].pTextureQ->GetDesc( &desc );
                
        ppSRV[0] =  TextureData[i].pTextureDCTSRV;
        ppSRV[1] =  TextureData[i].pTextureQSRV;
        d3dContext->PSSetShaderResources( 0, 2, ppSRV );
        
        d3dContext->Draw( 1, 0 );
        
        /////////////////////////////////////////////////////////////////////////////
        // Pass2
        /////////////////////////////////////////////////////////////////////////////
        
        technique->GetPassByIndex( 1 )->Apply( 0, d3dContext);
        
        Views[0] = TextureData[i].pTextureTargetRTV;
        Views[1] = NULL;
        
        TextureData[i].pTextureTarget->GetDesc( &desc );
        customViewports[0].Width = desc.Width;
        customViewports[0].Height = desc.Height;
        
        d3dContext->RSSetViewports( 1, customViewports );
        d3dContext->OMSetRenderTargets( 2, Views, NULL );
                
        ppSRV[0] =  TextureData[i].pTexture1RowSRV;
        ppSRV[1] =  TextureData[i].pTexture2RowSRV;
        d3dContext->PSSetShaderResources( 0, 2, ppSRV );
        
        d3dContext->Draw( 1, 0 );
        
        /////////////////////////////////////////////////////////////////////////////
        // Pass3
        /////////////////////////////////////////////////////////////////////////////
        
        technique->GetPassByIndex( 2 )->Apply( 0, d3dContext );
        
        Views[0] = TextureData[i].pTexture1ColRTV;
        Views[1] = TextureData[i].pTexture2ColRTV;
        
        TextureData[i].pTexture1Col->GetDesc( &desc );
        customViewports[0].Width = desc.Width;
        customViewports[0].Height = desc.Height;
        customViewports[1] = customViewports[0];
                
        d3dContext->RSSetViewports( 2, customViewports );
        d3dContext->OMSetRenderTargets( 2, Views, NULL );
                
        ppSRV[0] =  TextureData[i].pTextureTargetSRV;
        ppSRV[1] =  NULL;
        d3dContext->PSSetShaderResources( 0, 2, ppSRV );
        
        d3dContext->Draw( 1, 0 );
         
        /////////////////////////////////////////////////////////////////////////////
        // Pass4
        /////////////////////////////////////////////////////////////////////////////

        technique->GetPassByIndex( 3 )->Apply( 0, d3dContext );
        
        Views[0] = TextureData[i].pTextureTargetRTV;
        Views[1] = NULL;
        
        TextureData[i].pTextureTarget->GetDesc( &desc );
        customViewports[0].Width = desc.Width;
        customViewports[0].Height = desc.Height;

        d3dContext->RSSetViewports( 1, customViewports );
        d3dContext->OMSetRenderTargets( 2, Views, NULL );
        
        ppSRV[0] =  TextureData[i].pTexture1ColSRV;
        ppSRV[1] =  TextureData[i].pTexture2ColSRV;
        d3dContext->PSSetShaderResources( 0, 2, ppSRV );
                
        d3dContext->Draw( 1, 0 );
    }
}


void JPEG_Data::Release()
{
    if( mappingHandle )
    {
        CloseHandle( mappingHandle );
        mappingHandle = NULL;
    }
    
    if( fileHandle )
    {
        CloseHandle( fileHandle );
        fileHandle = NULL;
    }
    
    pFileData = NULL;
    
    for( int i =0; i < componentsNum; i++ )
    {
        TextureData[i].Release();
    }
}


////////////////////////////////////////////////////////////////////////////////////
// UpdateInfo class
////////////////////////////////////////////////////////////////////////////////////
UpdateInfo::UpdateInfo()
{
    blocksNum = 0;
    curBlock = 0;
    blocksAdded = 0;
    pSrcCorners = NULL;
    pDstCorners = NULL;
}

UpdateInfo::~UpdateInfo()
{
    Release();
}

void UpdateInfo::Initialize( int blocks )
{
    Release();
    
    blocksNum = blocks;
    
    pSrcCorners = new int[ blocksNum * 2 ];
    pDstCorners = new int[ blocksNum * 2 ];
    
    Reset();
}

void UpdateInfo::Release()
{
    if( pSrcCorners )
    {
        delete [] pSrcCorners;
        pSrcCorners = NULL;   
    }
    
    if( pDstCorners )
    {
        delete [] pDstCorners;
        pDstCorners = NULL;   
    }
    
    blocksNum = 0;
    curBlock = 0;
    blocksAdded = 0;
}

void UpdateInfo::Reset()
{
    curBlock = 0;
    blocksAdded = 0;
}

void UpdateInfo::AddBlock( int srcBlock[2], int dstBlock[2] )
{
    pSrcCorners[ 2 * curBlock ] = srcBlock[0];
    pSrcCorners[ 2 * curBlock + 1 ] = srcBlock[1];
    
    pDstCorners[ 2 * curBlock ] = dstBlock[0];
    pDstCorners[ 2 * curBlock + 1 ] = dstBlock[1];
    
    blocksAdded++;
    curBlock++;
    
    curBlock %= blocksNum;
}


////////////////////////////////////////////////////////////////////////////////////
// JPEG_Manager class
////////////////////////////////////////////////////////////////////////////////////
JPEG_Manager::JPEG_Manager()
{
    isInitialized = false;
    levelsNum = 0;
    pJpegData = NULL;
    pUpdateBlocks = NULL;
    ppBakedTextures = NULL;	
}
    
JPEG_Manager::~JPEG_Manager()
{
    Release();
}

HRESULT JPEG_Manager::Intitialize( int levels, WCHAR fileNames[][MAX_PATH], WCHAR fileNamesHM[][MAX_PATH] )
{   
    HRESULT hr = S_OK;
    
    if( !isInitialized )
    {
        if( !levels )
            return S_FALSE;
            
        levelsNum = levels;
        
        pJpegData = new JPEG_Data[levelsNum];
        pUpdateBlocks = new UpdateInfo[levelsNum];
        
        if( fileNamesHM )
        {
            pJpegDataHM = new JPEG_Data[levelsNum];
        }
        
        for( int i = 0; i < levelsNum; i++ )
        {
            ( pJpegData[i].LoadFile( fileNames[i] ) );
            
            if( fileNamesHM )
                ( pJpegDataHM[i].LoadFile( fileNamesHM[i] ) );
        }
        
        isInitialized = true;
    }
    
    return hr;
}


void JPEG_Manager::AllocateBlocks( int blocksNum )
{
    for( int i = 0; i < levelsNum; i++ )
    {
        pUpdateBlocks[i].Initialize( int( blocksNum * pow( 2.0, i ) ) );
    }    
}


HRESULT JPEG_Manager::AllocateTextures( ID3D11Device* pd3dDevice, int stackSize, int borderSize )
{
    HRESULT hr;
    int border = borderSize;
    
    LoadEffectFromFile( pd3dDevice, "Data/Shaders/JPEG_Preprocessor.fx", &g_pEffectJPEG );
    
    textureSize = stackSize;
    blockSize = borderSize;
    
    ppBakedTextures = new ID3D11Texture2D*[levelsNum];
    ppBakedTexturesRTV = new ID3D11RenderTargetView*[levelsNum];
    
    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory( &texDesc, sizeof( texDesc ) );
    texDesc.ArraySize = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.MipLevels = 1;
    texDesc.SampleDesc.Count = 1;
    
    D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
    ZeroMemory( &RTVDesc, sizeof( RTVDesc ) );
    RTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    RTVDesc.Texture2D.MipSlice = 0;
    
    for( int i = 0; i < levelsNum; i++ )
    {		
        ( ( pJpegData[i].AllocateTextures( pd3dDevice, stackSize, border ) ) );
        ( ( pJpegDataHM[i].AllocateTextures( pd3dDevice, stackSize, border ) ) );
        
        texDesc.Width = stackSize;
        texDesc.Height = border;
        
        ( pd3dDevice->CreateTexture2D( &texDesc, NULL, &ppBakedTextures[i] ) );
        ( pd3dDevice->CreateRenderTargetView( ppBakedTextures[i], &RTVDesc, &ppBakedTexturesRTV[i] ) );
        
        border >>= 1;
    }
    
    return S_OK;
}


void JPEG_Manager::Release()
{
    SAFE_RELEASE( g_pEffectJPEG );
    
    if( pJpegData )
    {
        for( int i = 0; i < levelsNum; i++ )
            pJpegData[i].Release();
            
        delete [] pJpegData;
        pJpegData = NULL;
    }
    
    if( pUpdateBlocks )
    {
        for( int i = 0; i < levelsNum; i++ )
            pUpdateBlocks[i].Release();
            
        delete [] pUpdateBlocks;
        pUpdateBlocks = NULL;
    }
    
    if( pJpegDataHM )
    {
        for( int i = 0; i < levelsNum; i++ )
            pJpegDataHM[i].Release();
            
        delete [] pJpegData;
        pJpegDataHM = NULL;
    }
    
    if( ppBakedTextures )
    {
        for( int i = 0; i < levelsNum; i++ )
        {
            SAFE_RELEASE( ppBakedTextures[i] );
            SAFE_RELEASE( ppBakedTexturesRTV[i] );
        }
        
        delete [] ppBakedTextures;
        ppBakedTextures = NULL;
        
        delete [] ppBakedTexturesRTV;
        ppBakedTexturesRTV = NULL;
    }
    
    isInitialized = false;
    levelsNum = 0;
}


void JPEG_Manager::AddBlock( int level, int srcBlock[2], int dstBlock[2] )
{
    if( level < levelsNum )
    {
        pUpdateBlocks[level].AddBlock( srcBlock, dstBlock );
    }
}


// Update one level of a clipmap stack texture with a new data covered by update border size
// We update a single ( StackSize * ( UpdateBorderSize >> level ) ) block at once
void JPEG_Manager::Update( ID3D11DeviceContext* pd3dContext, int level, ID3D11Texture2D*& outputStackTex )
{
    ID3D11RenderTargetView *cashedRTV;
    ID3D11DepthStencilView *cashedDSV;
    D3D11_VIEWPORT cashedViewport;
    UINT numViewports = 1;
        
    // Read current states to restore them after image block processing 
    pd3dContext->OMGetRenderTargets( 1, &cashedRTV, &cashedDSV );
    pd3dContext->RSGetViewports( &numViewports, &cashedViewport );
	
	if( level < levelsNum )
    {
        int blocksNum = pUpdateBlocks[level].GetNewBlocksNumber();
        
		
        // Process diffuse texture
        pJpegData[level].UpdateTextureData( pd3dContext, blocksNum, blockSize >> level, pUpdateBlocks[level].GetSrcBlocksPointer() );
        pJpegData[level].UncompressTextureData( pd3dContext );
                
        // Process heightmap texture
        // Here we use update blocks from the diffuse texture because they should be the same		
        pJpegDataHM[level].UpdateTextureData( pd3dContext, blocksNum, blockSize >> level, pUpdateBlocks[level].GetSrcBlocksPointer() );
        pJpegDataHM[level].UncompressTextureData( pd3dContext );
        pUpdateBlocks[level].Reset();
                
        D3D11_VIEWPORT customViewport;
    
        customViewport.TopLeftX = 0;
        customViewport.TopLeftY = 0;
        customViewport.MinDepth = 0.0f;
        customViewport.MaxDepth = 1.0f;
                
        D3D11_TEXTURE2D_DESC desc;
        ppBakedTextures[level]->GetDesc( &desc );
        
        customViewport.Width = desc.Width;
        customViewport.Height = desc.Height;
        
        ID3DX11EffectTechnique *technique = g_pEffectJPEG->GetTechniqueByName( "JPEG_Decompression" );
        
        pd3dContext->OMSetRenderTargets( 1, &ppBakedTexturesRTV[level], NULL );
        pd3dContext->RSSetViewports( 1, &customViewport );
        technique->GetPassByIndex( 4 )->Apply( 0, pd3dContext );

        ID3D11ShaderResourceView *ppSRV[4];
        
        ppSRV[0] = pJpegData[level].GetFinalTarget( 0 );
        ppSRV[1] = pJpegData[level].GetFinalTarget( 1 );
        ppSRV[2] = pJpegData[level].GetFinalTarget( 2 );
        
        if( pJpegDataHM )
            ppSRV[3] = pJpegDataHM[level].GetFinalTarget( 0 );
        else
            ppSRV[3] = NULL; 
        
        pd3dContext->PSSetShaderResources( 0, 4, ppSRV );
        // Combine decoded components into a single R8G8B8A8 texture
        pd3dContext->Draw( 1, 0 );

        int *pDstBlocks = pUpdateBlocks[level].GetDstBlocksPointer();
        int levelBlockSize = blockSize >> level;
        
        D3D11_BOX srcBox;
		srcBox.front = 0;
        srcBox.back = 1;
        srcBox.top = 0;
        srcBox.bottom = levelBlockSize;
        srcBox.left = 0;
        srcBox.right = levelBlockSize;

        // Copy fully unpacked data to the clipmap stack texture
        for( int i = 0; i < blocksNum; i++ )
        {
            pd3dContext->CopySubresourceRegion( outputStackTex, level, pDstBlocks[i * 2], pDstBlocks[i * 2 + 1], 0, ppBakedTextures[level], 0, &srcBox );
            
            srcBox.left += levelBlockSize;
            srcBox.right += levelBlockSize;
        }
    }

	ID3D11RenderTargetView *ppRTV[2];
    ppRTV[0] = cashedRTV;
    ppRTV[1] = NULL;
        
    pd3dContext->OMSetRenderTargets( 1, ppRTV, cashedDSV );
    pd3dContext->RSSetViewports( 1, &cashedViewport );

	SAFE_RELEASE( cashedRTV );
    SAFE_RELEASE( cashedDSV );
}


////////////////////////////////////////////////////////////////////////////////////
// JPEG_GPU class
////////////////////////////////////////////////////////////////////////////////////

JPEG_GPU::JPEG_GPU()
{
    pTextureDCT = NULL;
    pTextureDCTSRV = NULL;
    pTextureQ = NULL;
    pTextureQSRV = NULL;
    pTexture1Row = NULL;
    pTexture1RowSRV = NULL;
    pTexture1RowRTV = NULL;
    pTexture2Row = NULL;
    pTexture2RowSRV = NULL;
    pTexture2RowRTV = NULL;
    pTexture1Col = NULL;
    pTexture1ColSRV = NULL;
    pTexture1ColRTV = NULL;
    pTexture2Col = NULL;
    pTexture2ColSRV = NULL;
    pTexture2ColRTV = NULL;
    pTextureTarget = NULL;
    pTextureTargetSRV = NULL;
    pTextureTargetRTV = NULL;
}


HRESULT JPEG_GPU::Intitialize( ID3D11Device* pd3dDevice, int width, int height, BYTE *pQuantTable )
{
    HRESULT hr;
    
    D3D11_TEXTURE2D_DESC texDesc;
    ZeroMemory( &texDesc, sizeof( texDesc ) );
    texDesc.ArraySize = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.Format = DXGI_FORMAT_R8_SNORM; // FORMAT 1
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.MipLevels = 1;
    texDesc.SampleDesc.Count = 1;
    ( pd3dDevice->CreateTexture2D( &texDesc, NULL, &pTextureDCT ) );  
    
    D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    ZeroMemory( &SRVDesc, sizeof( SRVDesc ) );
    SRVDesc.Format = texDesc.Format;
    SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    SRVDesc.Texture2D.MipLevels = 1;
    SRVDesc.Texture2D.MostDetailedMip = 0;
    ( pd3dDevice->CreateShaderResourceView( pTextureDCT, &SRVDesc, &pTextureDCTSRV ) );
    
    texDesc.Format = DXGI_FORMAT_R8_UNORM; // FORMAT 2
    texDesc.Width = 8;
    texDesc.Height = 8;
    
    D3D11_SUBRESOURCE_DATA qData;
    qData.pSysMem = pQuantTable;
    qData.SysMemPitch = 8;
    
    ( pd3dDevice->CreateTexture2D( &texDesc, &qData, &pTextureQ ) );
    
    SRVDesc.Format = texDesc.Format;
    ( pd3dDevice->CreateShaderResourceView( pTextureQ, &SRVDesc, &pTextureQSRV ) );
    
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // FORMAT 3
    texDesc.Width = width / 8;
    texDesc.Height = height;
    ( pd3dDevice->CreateTexture2D( &texDesc, NULL, &pTexture1Row ) );
    ( pd3dDevice->CreateTexture2D( &texDesc, NULL, &pTexture2Row ) );
    
    texDesc.Width = width;
    texDesc.Height = height / 8;
    ( pd3dDevice->CreateTexture2D( &texDesc, NULL, &pTexture1Col ) ); 
    ( pd3dDevice->CreateTexture2D( &texDesc, NULL, &pTexture2Col ) );
    
    SRVDesc.Format = texDesc.Format;
    ( pd3dDevice->CreateShaderResourceView( pTexture1Row, &SRVDesc, &pTexture1RowSRV ) );
    ( pd3dDevice->CreateShaderResourceView( pTexture2Row, &SRVDesc, &pTexture2RowSRV ) );
    ( pd3dDevice->CreateShaderResourceView( pTexture1Col, &SRVDesc, &pTexture1ColSRV ) );
    ( pd3dDevice->CreateShaderResourceView( pTexture2Col, &SRVDesc, &pTexture2ColSRV ) );
        
    D3D11_RENDER_TARGET_VIEW_DESC RTVDesc;
    ZeroMemory( &RTVDesc, sizeof( RTVDesc ) );
    RTVDesc.Format = texDesc.Format;
    RTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    RTVDesc.Texture2D.MipSlice = 0;
    
    ( pd3dDevice->CreateRenderTargetView( pTexture1Row, &RTVDesc, &pTexture1RowRTV ) );
    ( pd3dDevice->CreateRenderTargetView( pTexture2Row, &RTVDesc, &pTexture2RowRTV ) );
    ( pd3dDevice->CreateRenderTargetView( pTexture1Col, &RTVDesc, &pTexture1ColRTV ) );
    ( pd3dDevice->CreateRenderTargetView( pTexture2Col, &RTVDesc, &pTexture2ColRTV ) );
        
    texDesc.Format = DXGI_FORMAT_R16_FLOAT; // FORMAT 4
    texDesc.Width = width;
    texDesc.Height = height;
    ( pd3dDevice->CreateTexture2D( &texDesc, NULL, &pTextureTarget ) ); 
    
    SRVDesc.Format = texDesc.Format;
    ( pd3dDevice->CreateShaderResourceView( pTextureTarget, &SRVDesc, &pTextureTargetSRV ) );
    
    RTVDesc.Format = texDesc.Format;
    ( pd3dDevice->CreateRenderTargetView( pTextureTarget, &RTVDesc, &pTextureTargetRTV ) );
    
    return S_OK;
}


void JPEG_GPU::Release()
{
    SAFE_RELEASE( pTextureDCT );
    SAFE_RELEASE( pTextureDCTSRV );
    SAFE_RELEASE( pTextureQ );
    SAFE_RELEASE( pTextureQSRV );
    SAFE_RELEASE( pTexture1Row );
    SAFE_RELEASE( pTexture1RowSRV );
    SAFE_RELEASE( pTexture1RowRTV );
    SAFE_RELEASE( pTexture2Row );
    SAFE_RELEASE( pTexture2RowSRV );
    SAFE_RELEASE( pTexture2RowRTV );
    SAFE_RELEASE( pTexture1Col );
    SAFE_RELEASE( pTexture1ColSRV );
    SAFE_RELEASE( pTexture1ColRTV );
    SAFE_RELEASE( pTexture2Col );
    SAFE_RELEASE( pTexture2ColSRV );
    SAFE_RELEASE( pTexture2ColRTV );
    SAFE_RELEASE( pTextureTarget );
    SAFE_RELEASE( pTextureTargetSRV );
    SAFE_RELEASE( pTextureTargetRTV );
}