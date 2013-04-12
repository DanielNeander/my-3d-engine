//=================================================================================================
//
//  MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#include "stdafx.h"

#include "GraphicsTypes.h"
#include "Exceptions.h"
#include "Utility.h"



// == RenderTarget2D ==============================================================================

RenderTarget2D::RenderTarget2D() :  Width(0),
                                    Height(0),
                                    Format(DXGI_FORMAT_UNKNOWN),
                                    NumMipLevels(0),
                                    MultiSamples(0),
                                    MSQuality(0),
                                    AutoGenMipMaps(false),
                                    UAView(NULL),
                                    ArraySize(1)
{

}

void RenderTarget2D::Initialize(ID3D11Device* device,
                                UINT width,
                                UINT height,
                                DXGI_FORMAT format,
                                UINT numMipLevels,
                                UINT multiSamples,
                                UINT msQuality,
                                BOOL autoGenMipMaps,
                                BOOL createUAV,
                                UINT arraySize,
                                BOOL cubeMap)
{
	Initialize2(device, width, height, D3D11_BIND_SHADER_RESOURCE|D3D11_BIND_RENDER_TARGET,
		format, numMipLevels, multiSamples, msQuality, autoGenMipMaps, createUAV, arraySize, cubeMap);
};


void RenderTarget2D::Initialize2(ID3D11Device* device,
	UINT width,
	UINT height,
	UINT bindFlags,
	DXGI_FORMAT format,	
	UINT numMipLevels,
	UINT multiSamples,
	UINT msQuality,
	BOOL autoGenMipMaps,
	BOOL createUAV,
	UINT arraySize,
	BOOL cubeMap)
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.ArraySize = arraySize;
	desc.BindFlags = bindFlags;
	if(createUAV)
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

	desc.CPUAccessFlags = 0;
	desc.Format = format;
	desc.MipLevels = numMipLevels;
	desc.MiscFlags = (autoGenMipMaps && numMipLevels > 1) ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
	desc.SampleDesc.Count = multiSamples;
	desc.SampleDesc.Quality = msQuality;
	desc.Usage = D3D11_USAGE_DEFAULT;

	if(cubeMap)
	{
		_ASSERT(arraySize == 6);
		desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	}

	DXCall(device->CreateTexture2D(&desc, NULL, &Texture));

	
	ArraySlices.clear();
	for(UINT i = 0; i < arraySize; ++i)
	{
		ID3D11RenderTargetViewPtr rtView;
		D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
		rtDesc.Format = format;
		if (format == DXGI_FORMAT_R32G32_TYPELESS)
			rtDesc.Format = DXGI_FORMAT_R32G32_FLOAT;	
		if (format == DXGI_FORMAT_R32_TYPELESS)
			rtDesc.Format = DXGI_FORMAT_R32_FLOAT;		

		if (arraySize == 1)
		{
			rtDesc.ViewDimension = multiSamples > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMS : D3D11_RTV_DIMENSION_TEXTURE2D;
			rtDesc.Texture2D.MipSlice = 0;
		}
		else
		{
			rtDesc.ViewDimension = multiSamples > 1 ? D3D11_RTV_DIMENSION_TEXTURE2DMSARRAY : D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			rtDesc.Texture2DArray.ArraySize = 1;
			rtDesc.Texture2DArray.FirstArraySlice = i;
			rtDesc.Texture2DArray.MipSlice = 0;
		}
		DXCall(device->CreateRenderTargetView(Texture, &rtDesc, &rtView));

		ArraySlices.push_back(rtView);
	}

	RTView = ArraySlices[0];
	RTViewPtr = RTView;

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVdesc;
	SRVdesc.Format = format;
	if (format == DXGI_FORMAT_R32G32_TYPELESS)
		SRVdesc.Format = DXGI_FORMAT_R32G32_FLOAT;	
	if (format == DXGI_FORMAT_R32_TYPELESS)
		SRVdesc.Format = DXGI_FORMAT_R32_FLOAT;	// ShaderResourceView 및 RenderTargetView 생성시 필요.
	SRVdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVdesc.Texture2D.MipLevels = 1;
	SRVdesc.Texture2D.MostDetailedMip = 0;
	DXCall(device->CreateShaderResourceView(Texture, &SRVdesc, &SRView));


	Width = width;
	Height = height;
	NumMipLevels = numMipLevels;
	MultiSamples = multiSamples;
	Format = format;
	AutoGenMipMaps = autoGenMipMaps;
	ArraySize = arraySize;
	CubeMap = cubeMap;

	if(createUAV)
		DXCall(device->CreateUnorderedAccessView(Texture, NULL, &UAView));

}

// == DepthStencilBuffer ==========================================================================

DepthStencilBuffer::DepthStencilBuffer() :  Width(0),
                                            Height(0),
                                            MultiSamples(0),
                                            MSQuality(0),
                                            Format(DXGI_FORMAT_UNKNOWN),
                                            ArraySize(1)
{

}

void DepthStencilBuffer::Initialize(ID3D11Device* device,
                                    UINT width,
                                    UINT height,
                                    DXGI_FORMAT format,
                                    BOOL useAsShaderResource,
                                    UINT multiSamples,
                                    UINT msQuality,
                                    UINT arraySize)
{
    UINT bindFlags = D3D11_BIND_DEPTH_STENCIL;
    if (useAsShaderResource)
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;

    DXGI_FORMAT dsTexFormat;
    if (!useAsShaderResource)
        dsTexFormat = format;
    else if (format == DXGI_FORMAT_D16_UNORM)
        dsTexFormat = DXGI_FORMAT_R16_TYPELESS;
    else if(format == DXGI_FORMAT_D24_UNORM_S8_UINT)
        dsTexFormat = DXGI_FORMAT_R24G8_TYPELESS;
    else
        dsTexFormat = DXGI_FORMAT_R32_TYPELESS;

    D3D11_TEXTURE2D_DESC desc;
    desc.Width = width;
    desc.Height = height;
    desc.ArraySize = arraySize;
    desc.BindFlags = bindFlags;
    desc.CPUAccessFlags = 0;
    desc.Format = dsTexFormat;
    desc.MipLevels = 1;
    desc.MiscFlags = 0;
    desc.SampleDesc.Count = multiSamples;
    desc.SampleDesc.Quality = msQuality;
    desc.Usage = D3D11_USAGE_DEFAULT;
    DXCall(device->CreateTexture2D(&desc, NULL, &Texture));

    ArraySlices.clear();
    for (UINT i = 0; i < arraySize; ++i)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        ID3D11DepthStencilViewPtr dsView;
        dsvDesc.Format = format;

        if (arraySize == 1)
        {
            dsvDesc.ViewDimension = multiSamples > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;
        }
        else
        {
            dsvDesc.ViewDimension = multiSamples > 1 ? D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Texture2DArray.ArraySize = 1;
            dsvDesc.Texture2DArray.FirstArraySlice = i;
            dsvDesc.Texture2DArray.MipSlice = 0;
        }

        dsvDesc.Flags = 0;
        DXCall(device->CreateDepthStencilView(Texture, &dsvDesc, &dsView));
        ArraySlices.push_back(dsView);

        if (i == 0)
        {
            // Also create a read-only DSV
            dsvDesc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
            if (format == DXGI_FORMAT_D24_UNORM_S8_UINT || format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
                dsvDesc.Flags |= D3D11_DSV_READ_ONLY_STENCIL;
            DXCall(device->CreateDepthStencilView(Texture, &dsvDesc, &ReadOnlyDSView));
            dsvDesc.Flags = 0;
        }
    }

    DSView = ArraySlices[0];

    if (useAsShaderResource)
    {
        DXGI_FORMAT dsSRVFormat;
        if (format == DXGI_FORMAT_D16_UNORM)
            dsSRVFormat = DXGI_FORMAT_R16_UNORM;
        else if(format == DXGI_FORMAT_D24_UNORM_S8_UINT)
            dsSRVFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS ;
        else
            dsSRVFormat = DXGI_FORMAT_R32_FLOAT;

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = dsSRVFormat;

        if (arraySize == 1)
        {
            srvDesc.ViewDimension = multiSamples > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            srvDesc.Texture2D.MostDetailedMip = 0;
        }
        else
        {
            srvDesc.ViewDimension = multiSamples > 1 ? D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
            srvDesc.Texture2DArray.ArraySize = arraySize;
            srvDesc.Texture2DArray.FirstArraySlice = 0;
            srvDesc.Texture2DArray.MipLevels = 1;
            srvDesc.Texture2DArray.MostDetailedMip = 0;
        }

        DXCall(device->CreateShaderResourceView(Texture, &srvDesc, &SRView));
    }
    else
        SRView = NULL;

    Width = width;
    Height = height;
    MultiSamples = multiSamples;
    Format = format;
    ArraySize = arraySize;
}

// == RWBuffer ====================================================================================

RWBuffer::RWBuffer() : Size(0), Stride(0), NumElements(0), Format(DXGI_FORMAT_UNKNOWN), RawBuffer(FALSE)
{

}

void RWBuffer::Initialize(ID3D11Device* device, DXGI_FORMAT format, UINT stride, UINT numElements, BOOL rawBuffer)
{
    Format = format;
    Size = stride * numElements;
    Stride = stride;
    NumElements = numElements;
    RawBuffer = rawBuffer;

    if(rawBuffer)
    {
        _ASSERT(stride == 4);
        Format = DXGI_FORMAT_R32_TYPELESS;
    }

    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = stride * numElements;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = rawBuffer ? D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS : 0;
    bufferDesc.StructureByteStride = 0;
    DXCall(device->CreateBuffer(&bufferDesc, NULL, &Buffer));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = Format;

    if(rawBuffer)
    {
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
        srvDesc.BufferEx.FirstElement = 0;
        srvDesc.BufferEx.NumElements = numElements;
        srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
    }
    else
    {
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
        srvDesc.Buffer.ElementOffset = 0;
        srvDesc.Buffer.ElementWidth = numElements;
    }

    DXCall(device->CreateShaderResourceView(Buffer, &srvDesc, &SRView));

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    uavDesc.Format = format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = numElements;
    uavDesc.Buffer.Flags = rawBuffer ? D3D11_BUFFER_UAV_FLAG_RAW : 0;

    DXCall(device->CreateUnorderedAccessView(Buffer, &uavDesc, &UAView));
}

// == StructuredBuffer ============================================================================

StructuredBuffer::StructuredBuffer() : Size(0), Stride(0), NumElements(0)
{
}

void StructuredBuffer::Initialize(ID3D11Device* device, UINT stride, UINT numElements, BOOL useAsUAV,
                                    BOOL appendConsume, BOOL useAsDrawIndirect, const void* initData)
{
    Size = stride * numElements;
    Stride = stride;
    NumElements = numElements;

    if(appendConsume || useAsDrawIndirect)
        useAsUAV = true;

    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = stride * numElements;
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.BindFlags |= useAsUAV ? D3D11_BIND_UNORDERED_ACCESS : 0;
    bufferDesc.CPUAccessFlags = 0;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.MiscFlags |= useAsDrawIndirect ? D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS : 0;
    bufferDesc.StructureByteStride = stride;

    D3D11_SUBRESOURCE_DATA subresourceData;
    subresourceData.pSysMem = initData;
    subresourceData.SysMemPitch = 0;
    subresourceData.SysMemSlicePitch = 0;

    DXCall(device->CreateBuffer(&bufferDesc, initData != NULL ? &subresourceData : NULL, &Buffer));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.ElementWidth = numElements;
    DXCall(device->CreateShaderResourceView(Buffer, &srvDesc, &SRView));

    if(useAsUAV)
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        uavDesc.Format = DXGI_FORMAT_UNKNOWN;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
        uavDesc.Buffer.FirstElement = 0;
        uavDesc.Buffer.Flags = appendConsume ? D3D11_BUFFER_UAV_FLAG_APPEND : 0;
        uavDesc.Buffer.NumElements = numElements;
        DXCall(device->CreateUnorderedAccessView(Buffer, &uavDesc, &UAView));
    }
}

void StructuredBuffer::WriteToFile(const WCHAR* path, ID3D11Device* device, ID3D11DeviceContext* context)
{
    _ASSERT(Buffer != NULL);

    // Get the buffer info
    D3D11_BUFFER_DESC desc;
    Buffer->GetDesc(&desc);

    UINT32 useAsUAV = (desc.BindFlags & D3D11_BIND_UNORDERED_ACCESS) ? 1 : 0;
    UINT32 useAsDrawIndirect = (desc.MiscFlags & D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS) ? 1 : 0;

    UINT32 appendConsume = 0;
    if(useAsUAV)
    {
        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        UAView->GetDesc(&uavDesc);
        appendConsume = (uavDesc.Format & D3D11_BUFFER_UAV_FLAG_APPEND) ? 1 : 0;
    }

    // If the exists, delete it
    if(FileExists(path))
        Win32Call(DeleteFileW(path));

    // Create the file
    HANDLE fileHandle = CreateFileW(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if(fileHandle == INVALID_HANDLE_VALUE)
        Win32Call(false);

    // Write the buffer info
    DWORD bytesWritten = 0;
    Win32Call(WriteFile(fileHandle, &Size, 4, &bytesWritten, NULL));
    Win32Call(WriteFile(fileHandle, &Stride, 4, &bytesWritten, NULL));
    Win32Call(WriteFile(fileHandle, &NumElements, 4, &bytesWritten, NULL));
    Win32Call(WriteFile(fileHandle, &useAsUAV, 4, &bytesWritten, NULL));
    Win32Call(WriteFile(fileHandle, &useAsDrawIndirect, 4, &bytesWritten, NULL));
    Win32Call(WriteFile(fileHandle, &appendConsume, 4, &bytesWritten, NULL));

    // Get the buffer data
    StagingBuffer stagingBuffer;
    stagingBuffer.Initialize(device, Size);
    context->CopyResource(stagingBuffer.Buffer, Buffer);
    const void* bufferData= stagingBuffer.Map(context);

    // Write the data to the file
    Win32Call(WriteFile(fileHandle, bufferData, Size, &bytesWritten, NULL));

    // Un-map the staging buffer
    stagingBuffer.Unmap(context);

    // Close the file
    Win32Call(CloseHandle(fileHandle));
}

void StructuredBuffer::ReadFromFile(const WCHAR* path, ID3D11Device* device)
{
    // Open the file
    HANDLE fileHandle = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(fileHandle == INVALID_HANDLE_VALUE)
        Win32Call(false);

    // Read the buffer info
    UINT32 useAsUAV, useAsDrawIndirect, appendConsume;
    DWORD bytesRead = 0;
    Win32Call(ReadFile(fileHandle, &Size, 4, &bytesRead, NULL));
    Win32Call(ReadFile(fileHandle, &Stride, 4, &bytesRead, NULL));
    Win32Call(ReadFile(fileHandle, &NumElements, 4, &bytesRead, NULL));
    Win32Call(ReadFile(fileHandle, &useAsUAV, 4, &bytesRead, NULL));
    Win32Call(ReadFile(fileHandle, &useAsDrawIndirect, 4, &bytesRead, NULL));
    Win32Call(ReadFile(fileHandle, &appendConsume, 4, &bytesRead, NULL));

    // Read the buffer data
    UINT8* bufferData = new UINT8[Size];
    Win32Call(ReadFile(fileHandle, bufferData, Size, &bytesRead, NULL));

    // Close the file
    Win32Call(CloseHandle(fileHandle));

    // Init
    Initialize(device, Stride, NumElements, useAsUAV, appendConsume, useAsDrawIndirect, bufferData);

    // Clean up
    delete [] bufferData;
}

// == StagingBuffer ===============================================================================

StagingBuffer::StagingBuffer() : Size(0)
{
}

void StagingBuffer::Initialize(ID3D11Device* device, UINT size)
{
    Size = size;

    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.ByteWidth = Size;
    bufferDesc.Usage = D3D11_USAGE_STAGING;
    bufferDesc.BindFlags = 0;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    bufferDesc.MiscFlags = 0;
    bufferDesc.StructureByteStride = 0;
    DXCall(device->CreateBuffer(&bufferDesc, NULL, &Buffer));
}

void* StagingBuffer::Map(ID3D11DeviceContext* context)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    DXCall(context->Map(Buffer, 0, D3D11_MAP_READ, 0, &mapped));

    return mapped.pData;
}

void StagingBuffer::Unmap(ID3D11DeviceContext* context)
{
    context->Unmap(Buffer, 0);
}

// == StagingTexture2D ============================================================================

StagingTexture2D::StagingTexture2D() :  Width(0),
                                        Height(0),
                                        Format(DXGI_FORMAT_UNKNOWN),
                                        NumMipLevels(0),
                                        MultiSamples(0),
                                        MSQuality(0),
                                        ArraySize(1)
{
}

void StagingTexture2D::Initialize(ID3D11Device* device,
                                    UINT width,
                                    UINT height,
                                    DXGI_FORMAT format,
                                    UINT numMipLevels,
                                    UINT multiSamples,
                                    UINT msQuality,
                                    UINT arraySize)
{
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = width;
    desc.Height = height;
    desc.ArraySize = arraySize;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.Format = format;
    desc.MipLevels = numMipLevels;
    desc.MiscFlags = 0;
    desc.SampleDesc.Count = multiSamples;
    desc.SampleDesc.Quality = msQuality;
    desc.Usage = D3D11_USAGE_STAGING;
    DXCall(device->CreateTexture2D(&desc, NULL, &Texture));


    Width = width;
    Height = height;
    NumMipLevels = numMipLevels;
    MultiSamples = multiSamples;
	MSQuality = msQuality;
    Format = format;
    ArraySize = arraySize;
};

void* StagingTexture2D::Map(ID3D11DeviceContext* context, UINT32 subResourceIndex, UINT32& pitch)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    DXCall(context->Map(Texture, subResourceIndex, D3D11_MAP_READ, 0, &mapped));
    pitch = mapped.RowPitch;
    return mapped.pData;
}

void StagingTexture2D::Unmap(ID3D11DeviceContext* context, UINT32 subResourceIndex)
{
    context->Unmap(Texture, subResourceIndex);
}


D3D11Texture3D::D3D11Texture3D() 
	: Width(0),
	Height(0),
	Format(DXGI_FORMAT_UNKNOWN),
	NumMipLevels(0),	
	Depth(1)
{

}

void D3D11Texture3D::Initialize( ID3D11Device* device, UINT width, UINT height, UINT depth, DXGI_FORMAT format, UINT numMipLevels /*= 1*/, UINT multiSamples /*= 1*/, UINT msQuality /*= 0*/)
{
	D3D11_TEXTURE3D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.Depth = depth;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Format = format;
	desc.MipLevels = numMipLevels;
	desc.MiscFlags = 0;
	
	desc.Usage = D3D11_USAGE_DYNAMIC; //D3D11_USAGE_STAGING;
	DXCall(device->CreateTexture3D(&desc, NULL, &Texture));
	
	Width = width;
	Height = height;
	NumMipLevels = numMipLevels;	
	Format = format;
	Depth = depth;
}

void* D3D11Texture3D::Map( ID3D11DeviceContext* context, UINT32 subResourceIndex, UINT32& pitch )
{
	D3D11_MAPPED_SUBRESOURCE mapped;
		
	DXCall(context->Map(Texture, subResourceIndex, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
	pitch = mapped.RowPitch;
	return mapped.pData;
}

void D3D11Texture3D::Unmap( ID3D11DeviceContext* context, UINT32 subResourceIndex )
{
	context->Unmap(Texture, subResourceIndex);
}

void D3D11Texture2D::Initialize( ID3D11Device* device, UINT width, UINT height, DXGI_FORMAT format, UINT bindFlag /*= D3D10_BIND_SHADER_RESOURCE*/, UINT numMipLevels /*= 1*/, UINT multiSamples /*= 1*/, UINT msQuality /*= 0*/, UINT arraySize /*= 1*/ )
{
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.ArraySize = arraySize;
	desc.BindFlags = bindFlag;
	desc.CPUAccessFlags = 0;
	desc.Format = format;
	desc.MipLevels = numMipLevels;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = multiSamples;
	desc.SampleDesc.Quality = msQuality;
	desc.Usage =  D3D11_USAGE_DEFAULT;
	DXCall(device->CreateTexture2D(&desc, NULL, &Texture));


	Width = width;
	Height = height;
	NumMipLevels = numMipLevels;
	MultiSamples = multiSamples;
	MSQuality = msQuality;
	Format = format;
	ArraySize = arraySize;
}

D3D11Texture2D::D3D11Texture2D() : Width(0),
	Height(0),
	Format(DXGI_FORMAT_UNKNOWN),
	NumMipLevels(0),
	MultiSamples(0),
	MSQuality(0),
	ArraySize(1)

{

}

void* D3D11Texture2D::Map( ID3D11DeviceContext* context, UINT32 subResourceIndex, UINT32& pitch )
{
	D3D11_MAPPED_SUBRESOURCE mapped;

	DXCall(context->Map(Texture, subResourceIndex, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
	pitch = mapped.RowPitch;
	return mapped.pData;
}

void D3D11Texture2D::Unmap( ID3D11DeviceContext* context, UINT32 subResourceIndex ) {
	context->Unmap(Texture, subResourceIndex);
}

using namespace std;

/*! Constructor

@param pGraphicDevice	Crush Graphic Device version D3D11
@param pd3dDevice		Direct3D11 Device
*/
TextureD3D11::TextureD3D11(ID3D11Device* pd3dDevice)
{
	m_pd3dDevice				= pd3dDevice;
	m_pd3dTexture				= NULL;
	m_pd3dResourceView			= NULL;
}

//! Destructor
TextureD3D11::~TextureD3D11( )
{
	Destroy();
}

//! Destroy
void TextureD3D11::Destroy()
{
	SAFE_RELEASE(m_pd3dTexture);
	SAFE_RELEASE(m_pd3dResourceView);
}



/*! Carga la textura desde un archivo

@param strFileName	Nombre de la textura
@param pTexDesc		Descriptor de la textura a crear (es opcional)
@returns true si la cargo, false si fallo
*/
bool TextureD3D11::CreateFromFile(const string& strFileName)
{
	D3DX11_IMAGE_LOAD_INFO loadInfo;
	loadInfo.Width			= D3DX11_DEFAULT;
	loadInfo.Height			= D3DX11_DEFAULT;
	loadInfo.Depth			= D3DX11_DEFAULT;
	loadInfo.FirstMipLevel	= 0;
	loadInfo.MipLevels		= D3DX11_DEFAULT;
	loadInfo.Usage			= (D3D11_USAGE) D3DX11_DEFAULT;
	loadInfo.BindFlags		= D3D11_BIND_SHADER_RESOURCE;
	loadInfo.CpuAccessFlags = 0;
	loadInfo.MiscFlags		= 0;
	loadInfo.Format			= DXGI_FORMAT_FROM_FILE;
	loadInfo.Filter			= D3DX11_FILTER_TRIANGLE;
	loadInfo.MipFilter		= D3DX11_FILTER_TRIANGLE;
	loadInfo.pSrcInfo		= NULL;

	HRESULT hr = S_OK;

	D3DX11_IMAGE_INFO imgInfo;
	hr = D3DX11GetImageInfoFromFileA(strFileName.data(), NULL, &imgInfo, NULL);
	if(FAILED(hr))
		return false;

	m_dxgiFormat	= imgInfo.Format;

	D3DX11_IMAGE_LOAD_INFO* pLoadInfo = NULL;

	if(imgInfo.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE && imgInfo.ArraySize == 6)
	{
		loadInfo.Width			= imgInfo.Width;
		loadInfo.Height			= imgInfo.Height;
		loadInfo.Depth			= imgInfo.Depth;
		loadInfo.Format			= imgInfo.Format;
		loadInfo.MiscFlags 		= imgInfo.MiscFlags;
		loadInfo.FirstMipLevel	= 0;
		loadInfo.pSrcInfo	= &imgInfo;
		pLoadInfo = &loadInfo;

		// Si quiero un CubeMap y el Array Size no es de 6 entonces la textura no puede ser CubeMap
		if(imgInfo.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE && imgInfo.ArraySize != 6 )
			return false;
	}


	// Load the Texture
	hr = D3DX11CreateShaderResourceViewFromFileA(m_pd3dDevice, strFileName.data(), pLoadInfo, NULL, &m_pd3dResourceView, NULL);
	if(FAILED(hr))
		return false;

	ID3D11Texture2D* pd3dTexture = NULL;
	m_pd3dResourceView->GetResource((ID3D11Resource**)&pd3dTexture);
	if(pd3dTexture)
	{
		pd3dTexture->GetDesc(&m_texDescriptor);
		SAFE_RELEASE(pd3dTexture);
	}
	return true;
}




/*! Create a Texture Render Target

@param rtDesc	Descriptor con las caracteristicas del render target
@param strName	Nombre del recurso
@returns true if ok else false
*/
bool TextureD3D11::Create(const D3D11_TEXTURE2D_DESC& texInfo)
{
	m_texDescriptor	= texInfo;

	if(m_texDescriptor.ArraySize > 1)
		return CreateTextureArray();
	return false;
}

/*! 

@returns true if ok else false
*/
bool TextureD3D11::CreateTextureArray()
{
	HRESULT hr = m_pd3dDevice->CreateTexture2D(&m_texDescriptor, NULL, (ID3D11Texture2D**)&m_pd3dTexture);
	if(FAILED(hr))	
		return false;

	ZeroMemory(&m_srViewDesc, sizeof(m_srViewDesc));

	// Shader Resource View
	m_srViewDesc.Format							= m_texDescriptor.Format;	
	m_srViewDesc.ViewDimension					= D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	m_srViewDesc.Texture2DArray.MipLevels		= m_texDescriptor.MipLevels;
	m_srViewDesc.Texture2DArray.MostDetailedMip	= 0;
	m_srViewDesc.Texture2DArray.ArraySize		= m_texDescriptor.ArraySize;
	m_srViewDesc.Texture2DArray.FirstArraySlice	= 0;

	hr = m_pd3dDevice->CreateShaderResourceView( m_pd3dTexture, &m_srViewDesc, &m_pd3dResourceView );
	if( FAILED(hr) )
		return false;

#ifdef DEBUG
	const char szName[] = "TextureArray";
	m_pd3dResourceView->SetPrivateData( WKPDID_D3DDebugObjectName, sizeof( szName ) - 1, szName );
#endif 
	return true;
}



/*! 

@returns true if ok else false
*/
bool TextureD3D11::LoadTextureOnArray(ID3D11DeviceContext* pd3dContext, int slice, const string& filename)
{
	D3DX11_IMAGE_LOAD_INFO loadInfo;
	ZeroMemory( &loadInfo, sizeof( D3DX11_IMAGE_LOAD_INFO ) );
	loadInfo.Width			= m_texDescriptor.Width;
	loadInfo.Height			= m_texDescriptor.Height;
	loadInfo.Depth			= 1;
	loadInfo.FirstMipLevel	= 0;
	loadInfo.MipLevels		= m_texDescriptor.MipLevels;
	loadInfo.Usage			= D3D11_USAGE_STAGING;
	loadInfo.BindFlags		= 0;
	loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
	loadInfo.MiscFlags		= 0;
	loadInfo.Format			= m_texDescriptor.Format;
	loadInfo.Filter			= D3DX11_FILTER_NONE;
	loadInfo.MipFilter		= D3DX11_FILTER_NONE;

	ID3D11Texture2D* pd3dTexture = NULL;
	HRESULT hr = D3DX11CreateTextureFromFileA(m_pd3dDevice, filename.c_str(), &loadInfo, NULL, (ID3D11Resource**)&pd3dTexture, NULL);
	if(FAILED(hr))
		return false;

	UINT uiMipCount			= m_texDescriptor.MipLevels;

	for(UINT i = 0; i < uiMipCount; i++)
	{
		D3D11_MAPPED_SUBRESOURCE mappedTex2D;

		UINT uiSrcSubResource = D3D11CalcSubresource(i, 0, uiMipCount);

		pd3dContext->Map(pd3dTexture, uiSrcSubResource, D3D11_MAP_READ, 0, &mappedTex2D );

		UINT uiDstSubResource	= D3D11CalcSubresource(i, slice, uiMipCount);
		pd3dContext->UpdateSubresource(m_pd3dTexture, uiDstSubResource, NULL, mappedTex2D.pData, mappedTex2D.RowPitch, 0);

		pd3dContext->Unmap(pd3dTexture, uiSrcSubResource);

	}

	SAFE_RELEASE(pd3dTexture);
	return true;
}

/*! 

@returns true if ok else false
*/
//bool TextureD3D11::UpdateFromDDS(IRenderContext* pContext, const UpdateInfo& ui, Buffer::SP spBufferDDS)
//{
//	RenderContextD3D11*		pRenderContextD3D11 = (RenderContextD3D11*)pContext;
//	ID3D11DeviceContext*	pd3dContext			=  pRenderContextD3D11->GetDeviceContext();
//
//
//	D3DX11_IMAGE_LOAD_INFO loadInfo;
//	ZeroMemory( &loadInfo, sizeof( D3DX10_IMAGE_LOAD_INFO ) );
//	loadInfo.Width			= ui.width;
//	loadInfo.Height			= ui.height;
//	loadInfo.Depth			= D3DX_FROM_FILE;
//	loadInfo.FirstMipLevel	= 0;
//	loadInfo.MipLevels		= m_texDescriptor.GetMipmapCount();
//	loadInfo.Usage			= D3D11_USAGE_STAGING;
//	loadInfo.BindFlags		= 0;
//	loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
//	loadInfo.MiscFlags		= 0;
//	loadInfo.Format			= m_dxgiFormat;
//	loadInfo.Filter			= D3DX11_FILTER_NONE;
//	loadInfo.MipFilter		= D3DX11_FILTER_NONE;
//
//	ID3D11Texture2D* pd3dTexture = NULL;
//	HRESULT hr = D3DX11CreateTextureFromMemory(m_pd3dDevice, spBufferDDS->GetBuffer(), spBufferDDS->GetSize(), &loadInfo, NULL, (ID3D11Resource**)&pd3dTexture, NULL);
//	if(FAILED(hr))
//		return false;
//	
//	UINT uiMipCount			= m_texDescriptor.GetMipmapCount();
//
//	for(UINT i = 0; i < uiMipCount; i++)
//	{
//		D3D11_MAPPED_SUBRESOURCE mappedTex2D;
//
//		UINT uiSrcSubResource = D3D11CalcSubresource(i, 0, uiMipCount);
//
//		pd3dContext->Map(pd3dTexture, uiSrcSubResource, D3D11_MAP_READ, 0, &mappedTex2D );
//
//		UINT uiDstSubResource	= D3D11CalcSubresource(i, ui.slice, uiMipCount);
//		pd3dContext->UpdateSubresource(m_pd3dTexture, uiDstSubResource, NULL, mappedTex2D.pData, mappedTex2D.RowPitch, 0);
//
//		pd3dContext->Unmap(pd3dTexture, uiSrcSubResource);
//
//	}
//
//	_SAFE_RELEASE(pd3dTexture);
//	return true;
//}



/*! 

@returns true if ok else false
*/
bool TextureD3D11::CreateFromRAW(const D3D11_TEXTURE2D_DESC& texInfo, void* pData)
{
	m_texDescriptor	= texInfo;
	m_dxgiFormat	= texInfo.Format;

	// Make Array of subresource data
	D3D11_SUBRESOURCE_DATA	subResourcesData;

	uint32		uiPitch		= texInfo.Width;

	subResourcesData.pSysMem			= pData;
	subResourcesData.SysMemPitch		= uiPitch;
	subResourcesData.SysMemSlicePitch	= 0;

	HRESULT hr = m_pd3dDevice->CreateTexture2D(&m_texDescriptor, &subResourcesData, (ID3D11Texture2D**)&m_pd3dTexture);
	if(FAILED(hr))	
		return false;

	// Create the render target view
	ZeroMemory(&m_srViewDesc, sizeof(m_srViewDesc));

	// Shader Resource View
	m_srViewDesc.Format				= m_texDescriptor.Format;	// Make the shaders see this as R32_FLOAT instead of typeless
	m_srViewDesc.ViewDimension		= D3D11_SRV_DIMENSION_TEXTURE2D;
	m_srViewDesc.Texture2D.MipLevels = 1;
	m_srViewDesc.Texture2D.MostDetailedMip = 0;
	hr = m_pd3dDevice->CreateShaderResourceView( m_pd3dTexture, &m_srViewDesc, &m_pd3dResourceView );
	if( FAILED(hr) )
		return false;

#ifdef DEBUG
	const char szName[] = "TextureFromRAW";
	m_pd3dResourceView->SetPrivateData( WKPDID_D3DDebugObjectName, sizeof( szName ) - 1, szName );
#endif 

	return true;
}