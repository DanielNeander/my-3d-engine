//=================================================================================================
//
//  MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#pragma once


#include "InterfacePointers.h"
#include "Utility.h"
#include "PixelFormat.h"

struct RenderTarget2D
{
    ID3D11Texture2DPtr Texture;
    ID3D11RenderTargetViewPtr RTView;
	ID3D11RenderTargetView*		RTViewPtr;
    ID3D11ShaderResourceViewPtr SRView;	
    ID3D11UnorderedAccessViewPtr UAView;
    UINT Width;
    UINT Height;
    UINT NumMipLevels;
    UINT MultiSamples;
    UINT MSQuality;
    DXGI_FORMAT Format;
    BOOL AutoGenMipMaps;
    UINT ArraySize;
    BOOL CubeMap;
    std::vector<ID3D11RenderTargetViewPtr> ArraySlices;

    RenderTarget2D();

  void Initialize(      ID3D11Device* device,
                        UINT width,
                        UINT height,
                        DXGI_FORMAT format,
                        UINT numMipLevels = 1,
                        UINT multiSamples = 1,
                        UINT msQuality = 0,
                        BOOL autoGenMipMaps = false,
                        BOOL createUAV = false,
                        UINT arraySize = 1,
                        BOOL cubeMap = false);

  void Initialize2(ID3D11Device* device,
	  UINT width,
	  UINT height,
	  UINT bindFlags,
	  DXGI_FORMAT format,	  
	  UINT numMipLevels = 1,
	  UINT multiSamples = 1,
	  UINT msQuality = 0,
	  BOOL autoGenMipMaps = false,
	  BOOL createUAV = false,
	  UINT arraySize = 1,
	  BOOL cubeMap = false);
};

struct DepthStencilBuffer
{
    ID3D11Texture2DPtr Texture;
    ID3D11DepthStencilViewPtr DSView;
    ID3D11DepthStencilViewPtr ReadOnlyDSView;
    ID3D11ShaderResourceViewPtr SRView;
    UINT Width;
    UINT Height;
    UINT MultiSamples;
    UINT MSQuality;
    DXGI_FORMAT Format;
    UINT ArraySize;
    std::vector<ID3D11DepthStencilViewPtr> ArraySlices;

    DepthStencilBuffer();

    void Initialize(    ID3D11Device* device,
                        UINT width,
                        UINT height,
                        DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT,
                        BOOL useAsShaderResource = false,
                        UINT multiSamples = 1,
                        UINT msQuality = 0,
                        UINT arraySize = 1);
};

struct RWBuffer
{
    ID3D11BufferPtr Buffer;
    ID3D11ShaderResourceViewPtr SRView;
    ID3D11UnorderedAccessViewPtr UAView;
    UINT Size;
    UINT Stride;
    UINT NumElements;
    BOOL RawBuffer;
    DXGI_FORMAT Format;

    RWBuffer();

    void Initialize(ID3D11Device* device, DXGI_FORMAT format, UINT stride, UINT numElements, BOOL rawBuffer = FALSE);
};

struct StructuredBuffer
{
    ID3D11BufferPtr Buffer;
    ID3D11ShaderResourceViewPtr SRView;
    ID3D11UnorderedAccessViewPtr UAView;
    UINT Size;
    UINT Stride;
    UINT NumElements;

    StructuredBuffer();

    void Initialize(ID3D11Device* device, UINT stride, UINT numElements, BOOL useAsUAV = false,
                            BOOL appendConsume = false, BOOL useAsDrawIndirect = false, const void* initData = NULL);

  void WriteToFile(const WCHAR* path, ID3D11Device* device, ID3D11DeviceContext* context);
  void ReadFromFile(const WCHAR* path, ID3D11Device* device);
};

struct StagingBuffer
{
    ID3D11BufferPtr Buffer;
    UINT Size;

    StagingBuffer();

    void Initialize(ID3D11Device* device, UINT size);
    void* Map(ID3D11DeviceContext* context);
    void Unmap(ID3D11DeviceContext* context);
};



struct StagingTexture2D
{
    ID3D11Texture2DPtr Texture;
    UINT Width;
    UINT Height;
    UINT NumMipLevels;
    UINT MultiSamples;
    UINT MSQuality;
    DXGI_FORMAT Format;
    UINT ArraySize;

    StagingTexture2D();

    void Initialize(    ID3D11Device* device,
                        UINT width,
                        UINT height,
                        DXGI_FORMAT format,
                        UINT numMipLevels = 1,
                        UINT multiSamples = 1,
                        UINT msQuality = 0,
                        UINT arraySize = 1);

    void* Map(ID3D11DeviceContext* context, UINT32 subResourceIndex, UINT32& pitch);
    void Unmap(ID3D11DeviceContext* context, UINT32 subResourceIndex);
};

struct D3D11Texture2D
{
	ID3D11Texture2DPtr Texture;
	UINT Width;
	UINT Height;
	UINT NumMipLevels;
	UINT MultiSamples;
	UINT MSQuality;
	DXGI_FORMAT Format;
	UINT ArraySize;

	D3D11Texture2D();

	void Initialize(    ID3D11Device* device,
		UINT width,
		UINT height,		
		DXGI_FORMAT format,
		UINT bindFlag = D3D10_BIND_SHADER_RESOURCE,
		UINT numMipLevels = 1,
		UINT multiSamples = 1,
		UINT msQuality = 0,
		UINT arraySize = 1);

	void* Map(ID3D11DeviceContext* context, UINT32 subResourceIndex, UINT32& pitch);
	void Unmap(ID3D11DeviceContext* context, UINT32 subResourceIndex);
};

struct D3D11Texture3D
{
	ID3D11Texture3DPtr Texture;
	UINT Width;
	UINT Height;
	UINT Depth;
	UINT NumMipLevels;	
	DXGI_FORMAT Format;

	PixelBox	CurrentLock;

	D3D11Texture3D();

	void Initialize(    ID3D11Device* device,
		UINT width,
		UINT height,
		UINT Depth,
		DXGI_FORMAT format,
		UINT numMipLevels = 1,
		UINT multiSamples = 1,
		UINT msQuality = 0
		);

	void* Map(ID3D11DeviceContext* context, UINT32 subResourceIndex, UINT32& pitch);
	void Unmap(ID3D11DeviceContext* context, UINT32 subResourceIndex);
};


template<typename T> class ConstantBuffer
{
public:

    T Data;

protected:

    ID3D11BufferPtr buffer;
    bool initialized;

public:

    ConstantBuffer() : initialized(false)
    {
        ZeroMemory(&Data, sizeof(T));
    }

    ID3D11Buffer* Buffer() const
    {
        return buffer;
    }

    void Initialize(ID3D11Device* device)
    {
        D3D11_BUFFER_DESC desc;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        desc.MiscFlags = 0;
        desc.ByteWidth = static_cast<UINT>(sizeof(T) + (16 - (sizeof(T) % 16)));

        DXCall(device->CreateBuffer(&desc, NULL, &buffer));

        initialized = true;
    }

    void ApplyChanges(ID3D11DeviceContext* deviceContext)
    {
        _ASSERT(initialized);

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DXCall(deviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
        CopyMemory(mappedResource.pData, &Data, sizeof(T));
        deviceContext->Unmap(buffer, 0);
    }

    void SetVS(ID3D11DeviceContext* deviceContext, UINT slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->VSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetPS(ID3D11DeviceContext* deviceContext, UINT slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->PSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetGS(ID3D11DeviceContext* deviceContext, UINT slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->GSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetHS(ID3D11DeviceContext* deviceContext, UINT slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->HSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetDS(ID3D11DeviceContext* deviceContext, UINT slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->DSSetConstantBuffers(slot, 1, bufferArray);
    }

    void SetCS(ID3D11DeviceContext* deviceContext, UINT slot) const
    {
        _ASSERT(initialized);

        ID3D11Buffer* bufferArray[1];
        bufferArray[0] = buffer;
        deviceContext->CSSetConstantBuffers(slot, 1, bufferArray);
    }
};

// For aligning to float4 boundaries
#define Float4Align __declspec(align(16))

class PIXEvent
{
public:

    PIXEvent(const WCHAR* markerName)
    {
        int retVal = D3DPERF_BeginEvent(0xFFFFFFFF, markerName);
        //_ASSERT(retVal >= 0);
    }

    ~PIXEvent()
    {
       int retVal = D3DPERF_EndEvent();
       //_ASSERT(retVal >= 0);
    }
};

using namespace std;

struct UpdateArea
{
	uint32	x, y, z;
	uint32	width, height, depth;
	uint32	slice, mipmap;

	UpdateArea():x(0), y(0), z(0), width(0), height(0), depth(0), slice(0), mipmap(0){}
};

//-----------------------------------------------------------------------------
//-| Desc: 
//-|	   
//-----------------------------------------------------------------------------
class TextureD3D11 
{
protected: // Attributos Basicos de Win32
	ID3D11Device*					m_pd3dDevice;
	ID3D11Resource*					m_pd3dTexture;
	ID3D11ShaderResourceView*		m_pd3dResourceView; 
	DXGI_FORMAT						m_dxgiFormat;
	D3D11_TEXTURE2D_DESC			m_texDescriptor;
	D3D11_SHADER_RESOURCE_VIEW_DESC	m_srViewDesc;	// Desc del Shader Resource (para texture binding)

protected: // Metodos

	bool CreateTextureArray();
public:
	// Constructores
	TextureD3D11(ID3D11Device* pd3dDevice);
	// Destructor
	~TextureD3D11();

	const D3D11_TEXTURE2D_DESC&	GetDescriptor() const	{ return m_texDescriptor;			}

	//! Create for render targets & dynamic textures
	bool Create(const D3D11_TEXTURE2D_DESC& texInfo);

	//! Create From File
	bool CreateFromFile(const string& strFileName);

	//! Create From Raw
	bool CreateFromRAW(const D3D11_TEXTURE2D_DESC& texInfo, void* pData);

	// Update the content of a dynamic texture. Buffer is a DDS (not raw Data
	bool UpdateFromFile(ID3D11DeviceContext* pd3dContext, const UpdateArea& ui, void* pBuffer, uint32 size);

	bool LoadTextureOnArray(ID3D11DeviceContext* pd3dContext, int slice, const string& filename);

	void Destroy();

	ID3D11Resource*						GetTextureResource()	{ return m_pd3dTexture;			}
	ID3D11ShaderResourceView*&			GetResourceView()		{ return m_pd3dResourceView;	}
	FORCEINLINE const DXGI_FORMAT&		GetFormat() const		{ return m_dxgiFormat;			}

};
