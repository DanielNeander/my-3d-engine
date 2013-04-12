///////////////////////////////////////////////////////////////////////
//  GeometryBuffer.inl
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) CONFIDENTIAL AND PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization, Inc. and
//  may not be copied, disclosed, or exploited except in accordance with 
//  the terms of that agreement.
//
//      Copyright (c) 2003-2009 IDV, Inc.
//      All rights reserved in all media.
//
//      IDV, Inc.
//      http://www.idvinc.com


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::CGeometryBufferDirectX10

inline CGeometryBufferDirectX10::CGeometryBufferDirectX10( ) :
	m_pVertexLayout(NULL),
	m_pVertexBuffer(NULL),
	m_pIndexBuffer(NULL),
	m_eIndexFormat(INDEX_FORMAT_UNSIGNED_32BIT),
	m_uiMaxIndex(0),
	m_uiCurrentVertexBufferSize(0),
	m_uiCurrentIndexBufferSize(0),
	m_bDynamicVertexBuffer(false)
{
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::~CGeometryBufferDirectX10

inline CGeometryBufferDirectX10::~CGeometryBufferDirectX10( )
{
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release( );
		m_pVertexBuffer = NULL;
	}

	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release( );
		m_pIndexBuffer = NULL;
	}

	if (m_pVertexLayout)
	{
		m_pVertexLayout->Release( );
		m_pVertexLayout = NULL;
	}

#ifdef _DEBUG
	m_uiMaxIndex = 0;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::CreateVertexBuffer

inline bool CGeometryBufferDirectX10::CreateVertexBuffer(const void* pVertexData, unsigned int uiNumVertices, unsigned int uiVertexSize)
{
	bool bSuccess = false;

	if (pVertexData && uiNumVertices > 0 && uiVertexSize > 0)
	{
		assert(DX11::Device( ));

		D3D11_BUFFER_DESC sBufferDesc;
		sBufferDesc.ByteWidth = uiNumVertices * uiVertexSize;
		sBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		sBufferDesc.MiscFlags = 0;
		if (m_bDynamicVertexBuffer)
		{
			sBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			sBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		else
		{
			sBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			sBufferDesc.CPUAccessFlags = 0;
		}

		D3D11_SUBRESOURCE_DATA sInitData;
		sInitData.pSysMem = pVertexData;

		if (DX11::Device( )->CreateBuffer(&sBufferDesc, &sInitData, &m_pVertexBuffer) == S_OK)
		{
			m_uiCurrentVertexBufferSize = uiNumVertices * uiVertexSize;
			bSuccess = true;
		}
	}

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::ReplaceVertices

inline bool CGeometryBufferDirectX10::ReplaceVertices(const void* pVertexData, unsigned int uiNumVertices, unsigned int uiVertexSize)
{
	bool bSuccess = false;

	// should have been taken care of in CGeometryBufferRI::ReplaceVertices, but be sure
	assert(pVertexData);
	assert(uiNumVertices > 0);
	assert(VertexBufferIsValid( ));

	// if incoming data is same size or smaller as current, just write over the current buffer
	const unsigned int c_uiSizeInBytes = uiNumVertices * uiVertexSize;
	if (m_bDynamicVertexBuffer && c_uiSizeInBytes <= m_uiCurrentVertexBufferSize)
	{
		// lock and fill the buffer
		D3D11_MAPPED_SUBRESOURCE MappedResource;		
		if (DX11::Context()->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource ) == S_OK)
		{
			memcpy(MappedResource.pData, pVertexData, c_uiSizeInBytes);
			DX11::Context()->Unmap(m_pVertexBuffer, 0);

			bSuccess = true;
		}
	}
	// if the incoming data is larger, we have to create a new buffer
	else
	{
		// destroy current buffer
		m_pVertexBuffer->Release( );
		m_pVertexBuffer = NULL;

		// create another
		bSuccess = CreateVertexBuffer(pVertexData, uiNumVertices, uiVertexSize);
	}

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::VertexBufferIsValid

inline bool CGeometryBufferDirectX10::VertexBufferIsValid(void) const
{
	return (m_pVertexBuffer != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::EnableFormat

inline bool CGeometryBufferDirectX10::EnableFormat(void) const
{
	assert(DX11::Device( ));

	DX11::Context( )->IASetInputLayout(m_pVertexLayout);

	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::DisableFormat

inline bool CGeometryBufferDirectX10::DisableFormat(void)
{
	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::BindVertexBuffer

inline bool CGeometryBufferDirectX10::BindVertexBuffer(int nVertexSize) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (VertexBufferIsValid( ) && IsFormatSet( ))
	{
#endif
		assert(DX11::Context( ));

		unsigned int uiStride = nVertexSize;
		unsigned int uiOffset = 0;
		DX11::Context( )->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &uiStride, &uiOffset);
		bSuccess = true;

#ifdef _DEBUG
	}
	else
		CCore::SetError("CGeometryBufferDirectX10::BindVertexBuffer, vertex buffer is not valid, cannot bind");
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::UnBindVertexBuffer

inline bool CGeometryBufferDirectX10::UnBindVertexBuffer(void)
{
	assert(DX11::Device( ));

	//ID3D11Buffer* aNullBuffers[] = { NULL };
	//UINT uiNULL = 0;
	//DX10::Device( )->IASetVertexBuffers(0, 1, aNullBuffers, &uiNULL, &uiNULL);

	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::SetIndexFormat

inline bool CGeometryBufferDirectX10::SetIndexFormat(EIndexFormat eFormat)
{
	m_eIndexFormat = eFormat;

	return (m_eIndexFormat == INDEX_FORMAT_UNSIGNED_16BIT || m_eIndexFormat == INDEX_FORMAT_UNSIGNED_32BIT);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::CreateIndexBuffer

inline bool CGeometryBufferDirectX10::CreateIndexBuffer(const void* pIndexData, unsigned int uiNumIndices)
{
	bool bSuccess = false;

	assert(pIndexData);
	assert(uiNumIndices > 0);
	assert(DX11::Device( ));

	if (pIndexData && uiNumIndices > 0)
	{
		D3D11_BUFFER_DESC sBufferDesc;
		sBufferDesc.ByteWidth = uiNumIndices * IndexSize( );
		sBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		sBufferDesc.MiscFlags = 0;
		sBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		sBufferDesc.CPUAccessFlags = 0;
		//sBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		//sBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA sInitData;
		sInitData.pSysMem = pIndexData;

		if (DX11::Device( )->CreateBuffer(&sBufferDesc, &sInitData, &m_pIndexBuffer) == S_OK)
		{
			FindMaxIndex(pIndexData, uiNumIndices);
			m_uiCurrentIndexBufferSize = uiNumIndices;
			bSuccess = true;
		}
	}			

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::ReplaceIndices

inline bool CGeometryBufferDirectX10::ReplaceIndices(const void* pIndexData, unsigned int uiNumIndices)
{
	bool bSuccess = false;

	// should have been taken care of in CGeometryBufferRI::ReplaceVertices, but be sure
	assert(pIndexData);
	assert(uiNumIndices > 0);
	assert(IndexBufferIsValid( ));

	// if incoming data is same size or smaller as current, just write over the current buffer
	if (uiNumIndices <= m_uiCurrentIndexBufferSize)
	{
		// lock and fill the buffer
		//void* pIndexBufferContents = NULL;
		D3D11_MAPPED_SUBRESOURCE MappedResource;		
		if (DX11::Context()	->Map(m_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource) == S_OK)
		{
			memcpy(MappedResource.pData, pIndexData, uiNumIndices * IndexSize( ));
			DX11::Context()->Unmap(m_pIndexBuffer, 0);

			FindMaxIndex(pIndexData, uiNumIndices);
			bSuccess = true;
		}
	}
	// if the incoming data is larger, we have to create a new buffer
	else
	{
		// destroy current buffer
		m_pIndexBuffer->Release( );
		m_pIndexBuffer = NULL;

		// create another
		bSuccess = CreateIndexBuffer(pIndexData, uiNumIndices);
	}

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::IndexBufferIsValid

inline bool CGeometryBufferDirectX10::IndexBufferIsValid(void) const
{
	return (m_pIndexBuffer != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::BindIndexBuffer

inline bool CGeometryBufferDirectX10::BindIndexBuffer(void) const
{
	assert(DX11::Device( ));

	// CGeometryBufferRI class checks if the index buffer is valid before calling
	// BindIndexBuffer; no need to check twice
	DX11::Context()->IASetIndexBuffer(m_pIndexBuffer, 
										(m_eIndexFormat == INDEX_FORMAT_UNSIGNED_16BIT ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT), 
										0);

	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::UnBindIndexBuffer

inline bool CGeometryBufferDirectX10::UnBindIndexBuffer(void) const
{
	assert(DX11::Device( ));

	//DX10::Device( )->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, 0);

	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::RenderIndexed

inline bool CGeometryBufferDirectX10::RenderIndexed(EPrimitiveType ePrimType, unsigned int uiStartIndex, unsigned int uiNumIndices, unsigned int /*uiNumVerticesOverride*/) const
{
	bool bSuccess = false;

	if (uiNumIndices > 0)
	{
		bool bValidPrimitiveType = false;

		D3D11_PRIMITIVE_TOPOLOGY eDirectX10Primitive = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		switch (ePrimType)
		{
		case PRIMITIVE_TRIANGLE_STRIP:
			eDirectX10Primitive = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			bValidPrimitiveType = true;
			break;
		case PRIMITIVE_TRIANGLES:
			eDirectX10Primitive = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			bValidPrimitiveType = true;
			break;
		case PRIMITIVE_POINTS:
			//eDirectX10Primitive = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST
			CCore::SetError("CGeometryBufferDirectX10::RenderIndexed, PRIMITIVE_POINTS type is not supported under the DirectX10 render interface");
			break;
		case PRIMITIVE_LINE_STRIP:
			//eDirectX10Primitive = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
			CCore::SetError("CGeometryBufferDirectX10::RenderIndexed, PRIMITIVE_LINE_STRIP type is not supported under the DirectX10 render interface");
			break;
		case PRIMITIVE_LINES:
			//eDirectX10Primitive = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			CCore::SetError("CGeometryBufferDirectX10::RenderIndexed, PRIMITIVE_LINES type is not supported under the DirectX10 render interface");
			break;
		case PRIMITIVE_TRIANGLE_FAN:
			CCore::SetError("CGeometryBufferDirectX10::RenderIndexed, PRIMITIVE_TRIANGLE_FAN type is not supported under the DirectX10 render interface");
			break;
		case PRIMITIVE_QUAD_STRIP:
			CCore::SetError("CGeometryBufferDirectX10::RenderIndexed, PRIMITIVE_QUAD_STRIP type is not supported under the DirectX10 render interface");
			break;
		case PRIMITIVE_QUADS:
			CCore::SetError("CGeometryBufferDirectX10::RenderIndexed, PRIMITIVE_QUADS type is not supported under the DirectX10 render interface");
			break;
		case PRIMITIVE_LINE_LOOP:
			CCore::SetError("CGeometryBufferDirectX10::RenderIndexed, PRIMITIVE_LINE_LOOP type is not supported under the DirectX10 render interface");
			break;
		default:
			assert(false);
		}

		if (bValidPrimitiveType)
		{
			assert(DX11::Context( ));
			DX11::Context( )->IASetPrimitiveTopology(eDirectX10Primitive);
			DX11::Context( )->DrawIndexed(uiNumIndices, uiStartIndex, 0);
			bSuccess = true;
		}
	}

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::RenderArrays

inline bool CGeometryBufferDirectX10::RenderArrays(EPrimitiveType ePrimType, unsigned int uiStartVertex, unsigned int uiNumVertices) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (VertexBufferIsValid( ))
	{
#endif
		bool bValidPrimitiveType = false;

		D3D11_PRIMITIVE_TOPOLOGY eDirectX10Primitive = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		switch (ePrimType)
		{
		case PRIMITIVE_TRIANGLE_STRIP:
			eDirectX10Primitive = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			bValidPrimitiveType = true;
			break;
		case PRIMITIVE_TRIANGLES:
			eDirectX10Primitive = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			bValidPrimitiveType = true;
			break;
		case PRIMITIVE_LINE_STRIP:
			eDirectX10Primitive = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
			bValidPrimitiveType = true;
			break;
		case PRIMITIVE_LINES:
			eDirectX10Primitive = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
			bValidPrimitiveType = true;
			break;
		case PRIMITIVE_POINTS:
			eDirectX10Primitive = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
			bValidPrimitiveType = true;
			break;
		case PRIMITIVE_TRIANGLE_FAN:
			CCore::SetError("CGeometryBufferDirectX10::RenderArrays, PRIMITIVE_TRIANGLE_FAN type is not supported under the DirectX10 render interface");
			break;
		case PRIMITIVE_QUAD_STRIP:
			CCore::SetError("CGeometryBufferDirectX10::RenderArrays, PRIMITIVE_QUAD_STRIP type is not supported under the DirectX10 render interface");
			break;
		case PRIMITIVE_QUADS:
			CCore::SetError("CGeometryBufferDirectX10::RenderArrays, PRIMITIVE_QUADS type is not supported under the DirectX10 render interface");
			break;
		case PRIMITIVE_LINE_LOOP:
			CCore::SetError("CGeometryBufferDirectX10::RenderArrays, PRIMITIVE_LINE_LOOP type is not supported under the DirectX10 render interface");
			break;
		default:
			assert(false);
		}

		if (bValidPrimitiveType)
		{
			assert(DX11::Context( ));
			DX11::Context( )->IASetPrimitiveTopology(eDirectX10Primitive);
			DX11::Context()->Draw(uiNumVertices, uiStartVertex);
			bSuccess = true;
		}

#ifdef _DEBUG
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::IsPrimitiveTypeSupported

inline bool CGeometryBufferDirectX10::IsPrimitiveTypeSupported(EPrimitiveType ePrimType)
{
	return (ePrimType == PRIMITIVE_TRIANGLE_STRIP ||
			ePrimType == PRIMITIVE_TRIANGLES);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::IsFormatSet

inline bool CGeometryBufferDirectX10::IsFormatSet(void) const
{
	return (m_pVertexLayout != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::IndexSize

inline unsigned int CGeometryBufferDirectX10::IndexSize(void) const
{
	return (m_eIndexFormat == INDEX_FORMAT_UNSIGNED_16BIT) ? 2 : 4;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::FindMaxIndex

inline void CGeometryBufferDirectX10::FindMaxIndex(const void* pIndexData, unsigned int uiNumIndices)
{
	m_uiMaxIndex = 0;
	if (m_eIndexFormat == INDEX_FORMAT_UNSIGNED_16BIT)
	{
		unsigned short* pIndices = (unsigned short*) pIndexData;
		for (unsigned int i = 0; i < uiNumIndices; ++i)
			m_uiMaxIndex = __max(pIndices[i], m_uiMaxIndex);
	}
	else // assume 32-bit
	{
		unsigned int* pIndices = (unsigned int*) pIndexData;
		for (unsigned int i = 0; i < uiNumIndices; ++i)
			m_uiMaxIndex = __max(pIndices[i], m_uiMaxIndex);
	}
}

