#pragma once 

#define MAX_CHANELS 3

class JPEG_GPU
{

public:
    
    ID3D11Texture2D*                pTextureDCT;
    ID3D11ShaderResourceView*       pTextureDCTSRV;
    
    ID3D11Texture2D*                pTextureQ;
    ID3D11ShaderResourceView*       pTextureQSRV;

    // Row textures
    ID3D11Texture2D*                pTexture1Row;
    ID3D11ShaderResourceView*       pTexture1RowSRV;
    ID3D11RenderTargetView*         pTexture1RowRTV;
    
    ID3D11Texture2D*                pTexture2Row;
    ID3D11ShaderResourceView*       pTexture2RowSRV;
    ID3D11RenderTargetView*         pTexture2RowRTV;

    // Column textures
    ID3D11Texture2D*                pTexture1Col;
    ID3D11ShaderResourceView*       pTexture1ColSRV;
    ID3D11RenderTargetView*         pTexture1ColRTV;
    
    ID3D11Texture2D*                pTexture2Col;
    ID3D11ShaderResourceView*       pTexture2ColSRV;
    ID3D11RenderTargetView*         pTexture2ColRTV;
    
    // Final texture
    ID3D11Texture2D*                pTextureTarget;
    ID3D11ShaderResourceView*       pTextureTargetSRV;
    ID3D11RenderTargetView*         pTextureTargetRTV;
    
    JPEG_GPU();
	HRESULT Intitialize( ID3D11Device* pd3dDevice, int width, int height, BYTE *pQuantTable );
	void Release();
};


class JPEG_Data
{
    int     imageWidth;
    int     imageHeight;
    int     componentsNum;
    BYTE    quantTables[MAX_CHANELS][64];
    
    int     MCU_per_row;
    int     MCU_blocks_num;
    
    HANDLE  fileHandle;
    HANDLE  mappingHandle;
    char    *pFileData;

    
    // D3D10 Textures are stored here and are used for IDCT on GPU
    JPEG_GPU        TextureData[MAX_CHANELS];
    
public:    
    JPEG_Data();
    HRESULT LoadFile( LPWSTR fileName );
    void    Release();
    int     GetImageWidth() { return imageWidth; }
    int     GetImageHeight() { return imageHeight; }
    HRESULT AllocateTextures( ID3D11Device* pd3dDevice, int stackSize, int border );
	HRESULT UpdateTextureData( ID3D11DeviceContext* pd3dContext, int blocksNum, int blockSize, int *pSrcCorners );
    ID3D11ShaderResourceView *GetFinalTarget( int componentID ) { return TextureData[componentID].pTextureTargetSRV; }    
	void UncompressTextureData( ID3D11DeviceContext* d3dContext);

};


class UpdateInfo
{
    int     blocksNum;      // A total number of blocks reserved for a single update step.
                            // This should be at least StackSize / UpdateBorderSize
    int     curBlock;
    int     blocksAdded;
    int     *pSrcCorners;   // Values used to map data during UpdateSubresource calls
                            // when we prepare data for IDCT pass.
                            // Here we store only the positions of a left upper corner,
                            // because update region size depends on the resource being used.
    int     *pDstCorners;   // Values used to map data during CopySubresourceRegion calls
                            // when we update our clipmap stack texture with uncompressed data.
    
public:
    
    UpdateInfo();
    ~UpdateInfo();
    void    Initialize( int blocks );
    void    AddBlock( int DstBlock[2], int SrcBlock[2] );
    int     GetNewBlocksNumber() { return blocksAdded; }
    int*    GetSrcBlocksPointer() { return pSrcCorners; }
    int*    GetDstBlocksPointer() { return pDstCorners; }
    void    Reset();
    void    Release();
};


class JPEG_Manager
{
    bool        isInitialized;
    int         levelsNum;      // Preprocessed levels number usde for a clipmap stack update.
                                // This should be at least as much as a stack depth
    int         textureSize;    // The biggest size of 0 level.
    int         blockSize;      // Updatable region size for 0 level.
    JPEG_Data   *pJpegData;
    JPEG_Data   *pJpegDataHM;   // Used to store additional height maps values
    UpdateInfo  *pUpdateBlocks;
    ID3D11Texture2D **ppBakedTextures;  // This textures are used at a final pass and they hold
                                        // an uncomprssed data ready for a clipmap stack texture update
    ID3D11RenderTargetView **ppBakedTexturesRTV;
    

public:
    
    JPEG_Manager();
    ~JPEG_Manager();
    HRESULT     Intitialize( int levels, WCHAR fileNames[][MAX_PATH], WCHAR fileNamesHM[][MAX_PATH] );
    void        AddBlock( int level, int srcBlock[2], int dstBlock[2] );
    void        AllocateBlocks( int blocksNum );
	HRESULT AllocateTextures( ID3D11Device* pd3dDevice, int stackSize, int borderSize );
	void Update( ID3D11DeviceContext* pd3dContext, int level, ID3D11Texture2D*& outputStackTex );
	int         GetImageWidth() { return pJpegData[0].GetImageWidth(); }
    int         GetImageHeight() { return pJpegData[0].GetImageHeight(); }
    void        Release();
};

extern JPEG_Manager g_JPEG_Manager;
extern  ID3DX11Effect* g_pEffectJPEG;
