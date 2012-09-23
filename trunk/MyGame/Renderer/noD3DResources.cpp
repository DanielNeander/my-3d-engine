#include "DXUT.h"
#include "noD3DResources.h"


///////////////// #defines /////////////////
#define IS_VERTEXSHADER_FUNCTION(pFunc) (((*pFunc) >> 16) == 0xFFFE)
#define IS_PIXELSHADER_FUNCTION(pFunc) (((*pFunc) >> 16) == 0xFFFF)

///////////////// noGeometryBuffer Class Implementation /////////////////
noGeometryBuffer::noGeometryBuffer() :
m_dwIndicesCount(0),
	m_dwVerticesCount(0),
	m_dwVertexSize(0),
	m_pVertices(NULL),
	m_pIndices(NULL),
	m_pVB(NULL),
	m_pIB(NULL)
{
}

noGeometryBuffer::~noGeometryBuffer()
{
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pIB);

	SAFE_DELETE_ARRAY(m_pVertices);
	SAFE_DELETE_ARRAY(m_pIndices);
}

HRESULT noGeometryBuffer::CreateVertexBuffer(DWORD dwVertexSize,DWORD dwVerticesCount,const void *pVertices)
{
	if (!dwVertexSize)
	{
		SXTRACE("noGeometryBuffer ERROR: Invalid vertex size");
		return E_INVALIDARG;
	}
	if (!dwVerticesCount)
	{
		SXTRACE("noGeometryBuffer ERROR: Invalid vertices count");
		return E_INVALIDARG;
	}

	void *pNewVB = new BYTE[dwVertexSize*dwVerticesCount];
	if (!pNewVB)
		return E_OUTOFMEMORY;

	// Get rid of the old buffer
	SAFE_RELEASE(m_pVB);
	SAFE_DELETE_ARRAY(m_pVertices);
	m_pVertices = pNewVB;

	m_dwVertexSize = dwVertexSize;
	m_dwVerticesCount = dwVerticesCount;

	if (pVertices)
		memcpy(m_pVertices,pVertices,dwVertexSize*dwVerticesCount);

	return S_OK;
}

HRESULT noGeometryBuffer::CreateIndexBuffer(DWORD dwIndicesCount,const WORD* pIndices)
{
	if (!dwIndicesCount)
	{
		SXTRACE("noGeometryBuffer ERROR: Invalid indices count");
		return E_INVALIDARG;
	}

	WORD *pNewIB = new WORD[dwIndicesCount];
	if (!pNewIB)
		return E_OUTOFMEMORY;

	// Get rid of the old buffer
	SAFE_RELEASE(m_pIB);
	SAFE_DELETE_ARRAY(m_pIndices);
	m_pIndices = pNewIB;

	m_dwIndicesCount = dwIndicesCount;

	if (pIndices)
		memcpy(m_pIndices,pIndices,sizeof(WORD)*dwIndicesCount);

	return S_OK;
}

HRESULT noGeometryBuffer::OnDeviceLoss(void)
{
	SAFE_RELEASE(m_pVB);
	SAFE_RELEASE(m_pIB);
	return S_OK;
}

HRESULT noGeometryBuffer::OnDeviceRelease(void)
{
	return OnDeviceLoss();
}

HRESULT noGeometryBuffer::OnDeviceRestore(void)
{
	PDIRECT3DDEVICE9 pDevice = m_owner->GetD3DDevice();
	HRESULT hRetval = S_OK;

	// Get this device's VP
	DWORD dwUsageVP = pDevice->GetSoftwareVertexProcessing()?D3DUSAGE_SOFTWAREPROCESSING:0;

	// Create vertex buffer?
	if (m_pVertices && !m_pVB)
	{
		hRetval = pDevice->CreateVertexBuffer(
			m_dwVerticesCount*m_dwVertexSize,	// Length
			dwUsageVP|D3DUSAGE_WRITEONLY,	// Usage
			0,	// FVF
			D3DPOOL_DEFAULT,	// Pool
			&m_pVB,NULL);
		if (FAILED(hRetval))
			return hRetval;

		// Lock'n'load
		void *pData = NULL;
		hRetval = m_pVB->Lock(0,m_dwVertexSize*m_dwVerticesCount,&pData,0);
		if (FAILED(hRetval))
		{
			SAFE_RELEASE(m_pVB);
			return hRetval;
		}
		memcpy(pData,m_pVertices,m_dwVertexSize*m_dwVerticesCount);	// Single yummy memcpy
		m_pVB->Unlock();
	}

	if (!m_pIndices || m_pIB)
		return S_OK;	// Non-indexed, or already valid

	hRetval = pDevice->CreateIndexBuffer(
		m_dwIndicesCount*sizeof(WORD),	// Length
		dwUsageVP|D3DUSAGE_WRITEONLY,	// Usage
		D3DFMT_INDEX16,	// Format
		D3DPOOL_DEFAULT,	// Pool
		&m_pIB,NULL);
	if (FAILED(hRetval))
		return hRetval;

	// Lock'n'load
	void *pData = NULL;
	hRetval = m_pIB->Lock(0,sizeof(WORD)*m_dwIndicesCount,&pData,0);
	if (FAILED(hRetval))
	{
		SAFE_RELEASE(m_pIB);
		return hRetval;
	}
	memcpy(pData,m_pIndices,sizeof(WORD)*m_dwIndicesCount);	// Single yummy memcpy
	m_pIB->Unlock();

	return hRetval;
}


///////////////// noTexture2D Class Implementation /////////////////
noTexture2D::noTexture2D() : m_pTexture(NULL)
{
	memset(m_tszFileName,0,sizeof(m_tszFileName));
}

noTexture2D::~noTexture2D()
{
	SAFE_RELEASE(m_pTexture);
}

HRESULT noTexture2D::SetTextureFile(PCTSTR pszFileName)
{
	if (!pszFileName || !pszFileName[0])
	{
		SXTRACE("noTexture2D ERROR: Invalid file name");
		return E_INVALIDARG;
	}

	// Get rid of the old texture
	SAFE_RELEASE(m_pTexture);
	_tcscpy(m_tszFileName,pszFileName);
	return S_OK;
}

HRESULT noTexture2D::OnDeviceRelease(void)
{
	SAFE_RELEASE(m_pTexture);
	return S_OK;
}

HRESULT noTexture2D::OnDeviceRestore(void)
{
	if (m_pTexture || !m_tszFileName[0])
		return S_OK;	// Not released, or invalid file name

	LPDIRECT3DDEVICE9 pDevice = m_owner->GetD3DDevice();
	return D3DXCreateTextureFromFile(pDevice,m_tszFileName,&m_pTexture);
}


///////////////// noShader Class Implementation /////////////////
noHLSLShader::noHLSLShader() :
m_pShaderFunction(NULL),
	m_pVertexShader(NULL),
	m_pPixelShader(NULL)
{
}

noHLSLShader::~noHLSLShader()
{
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
	SAFE_DELETE_ARRAY(m_pShaderFunction);
}

HRESULT noHLSLShader::SetShaderFile(PCTSTR pszFileName)
{
	if (!pszFileName || !pszFileName[0])
	{
		SXTRACE("noShader ERROR: Invalid file name");
		return E_INVALIDARG;
	}

	LPD3DXBUFFER pShaderBuffer = NULL;
	HRESULT hRetval = D3DXAssembleShaderFromFile(pszFileName,NULL,NULL,0,&pShaderBuffer,NULL);
	if (FAILED(hRetval))
		return hRetval;

	DWORD *pShaderFunction = (DWORD*)(new BYTE[pShaderBuffer->GetBufferSize()]);
	if (!pShaderFunction)
	{
		SAFE_RELEASE(pShaderBuffer);
		return E_OUTOFMEMORY;
	}

	SAFE_DELETE_ARRAY(m_pShaderFunction);
	m_pShaderFunction = pShaderFunction;

	memcpy(m_pShaderFunction,pShaderBuffer->GetBufferPointer(),pShaderBuffer->GetBufferSize());

	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
	return S_OK;
}

HRESULT noHLSLShader::OnDeviceRelease(void)
{
	SAFE_RELEASE(m_pVertexShader);
	SAFE_RELEASE(m_pPixelShader);
	return S_OK;
}

HRESULT noHLSLShader::OnDeviceRestore(void)
{
	if (m_pVertexShader || m_pPixelShader || !m_pShaderFunction)
		return S_OK;	// Not released, or not created yet

	LPDIRECT3DDEVICE9 pDevice = m_owner->GetD3DDevice();

	// Create the shader... Determine its type
	HRESULT hRetval = E_INVALIDARG;
	if (IS_VERTEXSHADER_FUNCTION(m_pShaderFunction))
		hRetval = pDevice->CreateVertexShader(m_pShaderFunction,&m_pVertexShader);
	else if (IS_PIXELSHADER_FUNCTION(m_pShaderFunction))
		hRetval = pDevice->CreatePixelShader(m_pShaderFunction,&m_pPixelShader);
	if (FAILED(hRetval))
		return hRetval;

	return S_OK;
}
