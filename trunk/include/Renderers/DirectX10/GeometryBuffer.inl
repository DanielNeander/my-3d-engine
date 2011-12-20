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

inline st_bool CGeometryBufferDirectX10::CreateVertexBuffer(const void* pVertexData, st_uint32 uiNumVertices, st_uint32 uiVertexSize)
{
	st_bool bSuccess = false;

	if (pVertexData && uiNumVertices > 0 && uiVertexSize > 0)
	{
		assert(DX10::Device( ));

		D3D10_BUFFER_DESC sBufferDesc;
		sBufferDesc.ByteWidth = uiNumVertices * uiVertexSize;
		sBufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER;
		sBufferDesc.MiscFlags = 0;
		if (m_bDynamicVertexBuffer)
		{
			sBufferDesc.Usage = D3D10_USAGE_DYNAMIC;
			sBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		}
		else
		{
			sBufferDesc.Usage = D3D10_USAGE_DEFAULT;
			sBufferDesc.CPUAccessFlags = 0;
		}

		D3D10_SUBRESOURCE_DATA sInitData;
		sInitData.pSysMem = pVertexData;

		if (DX10::Device( )->CreateBuffer(&sBufferDesc, &sInitData, &m_pVertexBuffer) == S_OK)
		{
			m_uiCurrentVertexBufferSize = uiNumVertices * uiVertexSize;
			bSuccess = true;
		}
	}

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::ReplaceVertices

inline st_bool CGeometryBufferDirectX10::ReplaceVertices(const void* pVertexData, st_uint32 uiNumVertices, st_uint32 uiVertexSize)
{
	st_bool bSuccess = false;

	// should have been taken care of in CGeometryBufferRI::ReplaceVertices, but be sure
	assert(pVertexData);
	assert(uiNumVertices > 0);
	assert(VertexBufferIsValid( ));

	// if incoming data is same size or smaller as current, just write over the current buffer
	const st_uint32 c_uiSizeInBytes = uiNumVertices * uiVertexSize;
	if (m_bDynamicVertexBuffer && c_uiSizeInBytes <= m_uiCurrentVertexBufferSize)
	{
		// lock and fill the buffer
		void* pVertexBufferContents = NULL;
		if (m_pVertexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, reinterpret_cast< void**>(&pVertexBufferContents)) == S_OK)
		{
			memcpy(pVertexBufferContents, pVertexData, c_uiSizeInBytes);
			m_pVertexBuffer->Unmap( );

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

inline st_bool CGeometryBufferDirectX10::VertexBufferIsValid(void) const
{
	return (m_pVertexBuffer != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::EnableFormat

inline st_bool CGeometryBufferDirectX10::EnableFormat(void) const
{
	assert(DX10::Device( ));

	DX10::Device( )->IASetInputLayout(m_pVertexLayout);

	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::DisableFormat

inline st_bool CGeometryBufferDirectX10::DisableFormat(void)
{
	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::BindVertexBuffer

inline st_bool CGeometryBufferDirectX10::BindVertexBuffer(st_int32 nVertexSize) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (VertexBufferIsValid( ) && IsFormatSet( ))
	{
#endif
		assert(DX10::Device( ));

		st_uint32 uiStride = nVertexSize;
		st_uint32 uiOffset = 0;
		DX10::Device( )->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &uiStride, &uiOffset);
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

inline st_bool CGeometryBufferDirectX10::UnBindVertexBuffer(void)
{
	assert(DX10::Device( ));

	//ID3D10Buffer* aNullBuffers[] = { NULL };
	//UINT uiNULL = 0;
	//DX10::Device( )->IASetVertexBuffers(0, 1, aNullBuffers, &uiNULL, &uiNULL);

	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::SetIndexFormat

inline st_bool CGeometryBufferDirectX10::SetIndexFormat(EIndexFormat eFormat)
{
	m_eIndexFormat = eFormat;

	return (m_eIndexFormat == INDEX_FORMAT_UNSIGNED_16BIT || m_eIndexFormat == INDEX_FORMAT_UNSIGNED_32BIT);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::CreateIndexBuffer

inline st_bool CGeometryBufferDirectX10::CreateIndexBuffer(const void* pIndexData, st_uint32 uiNumIndices)
{
	st_bool bSuccess = false;

	assert(pIndexData);
	assert(uiNumIndices > 0);
	assert(DX10::Device( ));

	if (pIndexData && uiNumIndices > 0)
	{
		D3D10_BUFFER_DESC sBufferDesc;
		sBufferDesc.ByteWidth = uiNumIndices * IndexSize( );
		sBufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;
		sBufferDesc.MiscFlags = 0;
		sBufferDesc.Usage = D3D10_USAGE_DEFAULT;
		sBufferDesc.CPUAccessFlags = 0;
		//sBufferDesc.Usage = D3D10_USAGE_DYNAMIC;
		//sBufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;

		D3D10_SUBRESOURCE_DATA sInitData;
		sInitData.pSysMem = pIndexData;

		if (DX10::Device( )->CreateBuffer(&sBufferDesc, &sInitData, &m_pIndexBuffer) == S_OK)
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

inline st_bool CGeometryBufferDirectX10::ReplaceIndices(const void* pIndexData, st_uint32 uiNumIndices)
{
	st_bool bSuccess = false;

	// should have been taken care of in CGeometryBufferRI::ReplaceVertices, but be sure
	assert(pIndexData);
	assert(uiNumIndices > 0);
	assert(IndexBufferIsValid( ));

	// if incoming data is same size or smaller as current, just write over the current buffer
	if (uiNumIndices <= m_uiCurrentIndexBufferSize)
	{
		// lock and fill the buffer
		void* pIndexBufferContents = NULL;
		if (m_pIndexBuffer->Map(D3D10_MAP_WRITE_DISCARD, 0, reinterpret_cast< void**>(&pIndexBufferContents)) == S_OK)
		{
			memcpy(pIndexBufferContents, pIndexData, uiNumIndices * IndexSize( ));
			m_pIndexBuffer->Unmap( );

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

inline st_bool CGeometryBufferDirectX10::IndexBufferIsValid(void) const
{
	return (m_pIndexBuffer != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::BindIndexBuffer

inline st_bool CGeometryBufferDirectX10::BindIndexBuffer(void) const
{
	assert(DX10::Device( ));

	// CGeometryBufferRI class checks if the index buffer is valid before calling
	// BindIndexBuffer; no need to check twice
	DX10::Device( )->IASetIndexBuffer(m_pIndexBuffer, 
										(m_eIndexFormat == INDEX_FORMAT_UNSIGNED_16BIT ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT), 
										0);

	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::UnBindIndexBuffer

inline st_bool CGeometryBufferDirectX10::UnBindIndexBuffer(void) const
{
	assert(DX10::Device( ));

	//DX10::Device( )->IASetIndexBuffer(NULL, DXGI_FORMAT_UNKNOWN, 0);

	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::RenderIndexed

inline st_bool CGeometryBufferDirectX10::RenderIndexed(EPrimitiveType ePrimType, st_uint32 uiStartIndex, st_uint32 uiNumIndices, st_uint32 /*uiNumVerticesOverride*/) const
{
	st_bool bSuccess = false;

	if (uiNumIndices > 0)
	{
		st_bool bValidPrimitiveType = false;

		D3D10_PRIMITIVE_TOPOLOGY eDirectX10Primitive = D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED;
		switch (ePrimType)
		{
		case PRIMITIVE_TRIANGLE_STRIP:
			eDirectX10Primitive = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			bValidPrimitiveType = true;
			break;
		case PRIMITIVE_TRIANGLES:
			eDirectX10Primitive = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			bValidPrimitiveType = true;
			break;
		case PRIMITIVE_POINTS:
			//eDirectX10Primitive = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST
			CCore::SetError("CGeometryBufferDirectX10::RenderIndexed, PRIMITIVE_POINTS type is not supported under the DirectX10 render interface");
			break;
		case PRIMITIVE_LINE_STRIP:
			//eDirectX10Primitive = D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP;
			CCore::SetError("CGeometryBufferDirectX10::RenderIndexed, PRIMITIVE_LINE_STRIP type is not supported under the DirectX10 render interface");
			break;
		case PRIMITIVE_LINES:
			//eDirectX10Primitive = D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
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
			assert(DX10::Device( ));
			DX10::Device( )->IASetPrimitiveTopology(eDirectX10Primitive);
			DX10::Device( )->DrawIndexed(uiNumIndices, uiStartIndex, 0);
			bSuccess = true;
		}
	}

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::RenderArrays

inline st_bool CGeometryBufferDirectX10::RenderArrays(EPrimitiveType ePrimType, st_uint32 uiStartVertex, st_uint32 uiNumVertices) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (VertexBufferIsValid( ))
	{
#endif
		st_bool bValidPrimitiveType = false;

		D3D10_PRIMITIVE_TOPOLOGY eDirectX10Primitive = D3D10_PRIMITIVE_TOPOLOGY_UNDEFINED;
		switch (ePrimType)
		{
		case PRIMITIVE_TRIANGLE_STRIP:
			eDirectX10Primitive = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
			bValidPrimitiveType = true;
			break;
		case PRIMITIVE_TRIANGLES:
			eDirectX10Primitive = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
			bValidPrimitiveType = true;
			break;
		case PRIMITIVE_LINE_STRIP:
			eDirectX10Primitive = D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP;
			bValidPrimitiveType = true;
			break;
		case PRIMITIVE_LINES:
			eDirectX10Primitive = D3D10_PRIMITIVE_TOPOLOGY_LINELIST;
			bValidPrimitiveType = true;
			break;
		case PRIMITIVE_POINTS:
			eDirectX10Primitive = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST;
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
			assert(DX10::Device( ));
			DX10::Device( )->IASetPrimitiveTopology(eDirectX10Primitive);
			DX10::Device( )->Draw(uiNumVertices, uiStartVertex);
			bSuccess = true;
		}

#ifdef _DEBUG
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::IsPrimitiveTypeSupported

inline st_bool CGeometryBufferDirectX10::IsPrimitiveTypeSupported(EPrimitiveType ePrimType)
{
	return (ePrimType == PRIMITIVE_TRIANGLE_STRIP ||
			ePrimType == PRIMITIVE_TRIANGLES);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::IsFormatSet

inline st_bool CGeometryBufferDirectX10::IsFormatSet(void) const
{
	return (m_pVertexLayout != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::IndexSize

inline st_uint32 CGeometryBufferDirectX10::IndexSize(void) const
{
	return (m_eIndexFormat == INDEX_FORMAT_UNSIGNED_16BIT) ? 2 : 4;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::FindMaxIndex

inline void CGeometryBufferDirectX10::FindMaxIndex(const void* pIndexData, st_uint32 uiNumIndices)
{
	m_uiMaxIndex = 0;
	if (m_eIndexFormat == INDEX_FORMAT_UNSIGNED_16BIT)
	{
		st_uint16* pIndices = (st_uint16*) pIndexData;
		for (st_uint32 i = 0; i < uiNumIndices; ++i)
			m_uiMaxIndex = __max(pIndices[i], m_uiMaxIndex);
	}
	else // assume 32-bit
	{
		st_uint32* pIndices = (st_uint32*) pIndexData;
		for (st_uint32 i = 0; i < uiNumIndices; ++i)
			m_uiMaxIndex = __max(pIndices[i], m_uiMaxIndex);
	}
}

