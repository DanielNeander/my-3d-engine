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
//  CGeometryBufferDirectX9::CGeometryBufferDirectX9

inline CGeometryBufferDirectX9::CGeometryBufferDirectX9( ) :
	m_pVertexDeclaration(NULL),
	m_pVertexBuffer(NULL),
	m_pIndexBuffer(NULL),
	m_eIndexFormat(INDEX_FORMAT_UNSIGNED_32BIT),
	m_uiMaxIndex(0),
	m_uiCurrentVertexBufferSize(0),
	m_uiCurrentIndexBufferSize(0)
{
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::~CGeometryBufferDirectX9

inline CGeometryBufferDirectX9::~CGeometryBufferDirectX9( )
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

	if (m_pVertexDeclaration)
	{
		m_pVertexDeclaration->Release( );
		m_pVertexDeclaration = NULL;
	}

#ifdef _DEBUG
	m_uiMaxIndex = 0;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::CreateVertexBuffer

inline st_bool CGeometryBufferDirectX9::CreateVertexBuffer(const void* pVertexData, st_uint32 uiNumVertices, st_uint32 uiVertexSize)
{
	st_bool bSuccess = false;

	if (pVertexData && uiNumVertices > 0 && uiVertexSize > 0)
	{
		assert(DX9::Device( ));

		const st_uint32 c_uiNumBytes = uiNumVertices * uiVertexSize;
		if (DX9::Device( )->CreateVertexBuffer(c_uiNumBytes, D3DUSAGE_WRITEONLY, 0, D3DPOOL_MANAGED, &m_pVertexBuffer, NULL) == D3D_OK)
		{
			if (m_pVertexBuffer)
			{
				// make sure the buffer's not bound when updating
				if (DX9::Device( )->SetStreamSource(0, NULL, 0, 0) == D3D_OK)
				{
					// lock and fill the buffer
					void* pVertexBufferContents = NULL;
					if (m_pVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBufferContents), 0) == D3D_OK)
					{
						memcpy(pVertexBufferContents, pVertexData, c_uiNumBytes);

						// buffer update complete, release the lock
						bSuccess = (m_pVertexBuffer->Unlock( ) == D3D_OK);

						if (bSuccess)
							m_uiCurrentVertexBufferSize = c_uiNumBytes;
					}
				}
			}
		}
	}

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::ReplaceVertices

inline st_bool CGeometryBufferDirectX9::ReplaceVertices(const void* pVertexData, st_uint32 uiNumVertices, st_uint32 uiVertexSize)
{
	st_bool bSuccess = false;

	// should have been taken care of in CGeometryBufferRI::ReplaceVertices, but be sure
	assert(pVertexData);
	assert(uiNumVertices > 0);
	assert(VertexBufferIsValid( ));

	// if incoming data is same size or smaller as current, just write over the current buffer
	const st_uint32 c_uiSizeInBytes = uiNumVertices * uiVertexSize;
	if (c_uiSizeInBytes <= m_uiCurrentVertexBufferSize)
	{
		// make sure the buffer's not bound when updating
		if (DX9::Device( )->SetStreamSource(0, NULL, 0, 0) == D3D_OK)
		{
			// lock and fill the buffer
			void* pVertexBufferContents = NULL;
			if (m_pVertexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pVertexBufferContents), 0) == D3D_OK)
			{
				memcpy(pVertexBufferContents, pVertexData, c_uiSizeInBytes);

				// buffer update complete, release the lock
				bSuccess = (m_pVertexBuffer->Unlock( ) == D3D_OK);
			}
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
//  CGeometryBufferDirectX9::VertexBufferIsValid

inline st_bool CGeometryBufferDirectX9::VertexBufferIsValid(void) const
{
	return (m_pVertexBuffer != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::EnableFormat

inline st_bool CGeometryBufferDirectX9::EnableFormat(void) const
{
	assert(DX9::Device( ));

	return (DX9::Device( )->SetVertexDeclaration(m_pVertexDeclaration) == D3D_OK);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::DisableFormat

inline st_bool CGeometryBufferDirectX9::DisableFormat(void)
{
	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::BindVertexBuffer

inline st_bool CGeometryBufferDirectX9::BindVertexBuffer(st_int32 nVertexSize) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (VertexBufferIsValid( ) && IsFormatSet( ))
	{
#endif
		assert(DX9::Device( ));
		bSuccess = (DX9::Device( )->SetStreamSource(0, m_pVertexBuffer, 0, nVertexSize) == D3D_OK);
#ifdef _DEBUG
	}
	else
		CCore::SetError("CGeometryBufferDirectX9::BindVertexBuffer, vertex buffer is not valid, cannot bind");
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::UnBindVertexBuffer

inline st_bool CGeometryBufferDirectX9::UnBindVertexBuffer(void)
{
	assert(DX9::Device( ));

	return (DX9::Device( )->SetStreamSource(0, NULL, 0, 0) == D3D_OK);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::SetIndexFormat

inline st_bool CGeometryBufferDirectX9::SetIndexFormat(EIndexFormat eFormat)
{
	m_eIndexFormat = eFormat;

	return (m_eIndexFormat == INDEX_FORMAT_UNSIGNED_16BIT || m_eIndexFormat == INDEX_FORMAT_UNSIGNED_32BIT);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::CreateIndexBuffer

inline st_bool CGeometryBufferDirectX9::CreateIndexBuffer(const void* pIndexData, st_uint32 uiNumIndices)
{
	st_bool bSuccess = false;

	assert(pIndexData);
	assert(uiNumIndices > 0);
	assert(DX9::Device( ));

	// create the DX9 index buffer
	st_uint32 c_uiNumBytes = uiNumIndices * IndexSize( );
	if (DX9::Device( )->CreateIndexBuffer(c_uiNumBytes, D3DUSAGE_WRITEONLY, m_eIndexFormat == INDEX_FORMAT_UNSIGNED_16BIT ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_pIndexBuffer, NULL) == D3D_OK)
	{
		// make sure the buffer's not bound when updating
		if (DX9::Device( )->SetIndices(NULL) == D3D_OK)
		{
			// lock and fill the buffer
			void* pIndexBufferContents = NULL;
			if (m_pIndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pIndexBufferContents), 0) == D3D_OK)
			{
				memcpy(pIndexBufferContents, pIndexData, c_uiNumBytes);

				FindMaxIndex(pIndexData, uiNumIndices);

				// buffer update complete, release the lock
				if (m_pIndexBuffer->Unlock( ) == D3D_OK)
				{
					bSuccess = true;
				}
			}
		}
	}

	if (bSuccess)
		m_uiCurrentIndexBufferSize = uiNumIndices;	

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::ReplaceIndices

inline st_bool CGeometryBufferDirectX9::ReplaceIndices(const void* pIndexData, st_uint32 uiNumIndices)
{
	st_bool bSuccess = false;

	// should have been taken care of in CGeometryBufferRI::ReplaceVertices, but be sure
	assert(pIndexData);
	assert(uiNumIndices > 0);
	assert(IndexBufferIsValid( ));

	// if incoming data is same size or smaller as current, just write over the current buffer
	if (uiNumIndices <= m_uiCurrentIndexBufferSize)
	{
		// make sure the buffer's not bound when updating
		if (DX9::Device( )->SetStreamSource(0, NULL, 0, 0) == D3D_OK)
		{
			// lock and fill the buffer
			void* pIndexBufferContents = NULL;
			if (m_pIndexBuffer->Lock(0, 0, reinterpret_cast<void**>(&pIndexBufferContents), 0) == D3D_OK)
			{
				memcpy(pIndexBufferContents, pIndexData, uiNumIndices * IndexSize( ));

				FindMaxIndex(pIndexData, uiNumIndices);

				// buffer update complete, release the lock
				bSuccess = (m_pIndexBuffer->Unlock( ) == D3D_OK);
			}
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
//  CGeometryBufferDirectX9::IndexBufferIsValid

inline st_bool CGeometryBufferDirectX9::IndexBufferIsValid(void) const
{
	return (m_pIndexBuffer != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::BindIndexBuffer

inline st_bool CGeometryBufferDirectX9::BindIndexBuffer(void) const
{
	assert(DX9::Device( ));

	// CGeometryBufferRI class checks if the index buffer is valid before calling
	// BindIndexBuffer; no need to check twice
	return (DX9::Device( )->SetIndices(m_pIndexBuffer) == D3D_OK);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::UnBindIndexBuffer

inline st_bool CGeometryBufferDirectX9::UnBindIndexBuffer(void) const
{
	assert(DX9::Device( ));

	return (DX9::Device( )->SetIndices(NULL) == D3D_OK);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::RenderIndexed

inline st_bool CGeometryBufferDirectX9::RenderIndexed(EPrimitiveType ePrimType, st_uint32 uiStartIndex, st_uint32 uiNumIndices, st_uint32 uiNumVerticesOverride) const
{
	st_bool bSuccess = false;

	if (uiNumIndices > 0)
	{
		st_bool bValidPrimitiveType = true;

		st_int32 nNumPrimitiveElements = 1;
		D3DPRIMITIVETYPE eDirectX9Primitive = D3DPT_POINTLIST;
		switch (ePrimType)
		{
		case PRIMITIVE_TRIANGLE_STRIP:
			nNumPrimitiveElements = uiNumIndices - 2;
			eDirectX9Primitive = D3DPT_TRIANGLESTRIP;
			break;
		case PRIMITIVE_TRIANGLES:
			nNumPrimitiveElements = uiNumIndices / 3;
			eDirectX9Primitive = D3DPT_TRIANGLELIST;
			break;
		case PRIMITIVE_LINE_STRIP:
			//eDirectX9Primitive = D3DPT_LINESTRIP;
			CCore::SetError("CGeometryBufferDirectX9::RenderIndexed, PRIMITIVE_LINE_STRIP type is not supported under the DirectX9 render interface");
			break;
		case PRIMITIVE_LINES:
			//eDirectX9Primitive = D3DPT_LINELIST;
			CCore::SetError("CGeometryBufferDirectX9::RenderIndexed, PRIMITIVE_LINES type is not supported under the DirectX9 render interface");
			break;
		case PRIMITIVE_TRIANGLE_FAN:
			//eDirectX9Primitive = D3DPT_TRIANGLEFAN;
			CCore::SetError("CGeometryBufferDirectX9::RenderIndexed, PRIMITIVE_TRIANGLE_FAN type is not supported under the DirectX9 render interface");
			break;
		case PRIMITIVE_QUAD_STRIP:
			CCore::SetError("CGeometryBufferDirectX9::RenderIndexed, PRIMITIVE_QUAD_STRIP type is not supported under the DirectX9 render interface");
			bValidPrimitiveType = false;
			break;
		case PRIMITIVE_QUADS:
			CCore::SetError("CGeometryBufferDirectX9::RenderIndexed, PRIMITIVE_QUADS type is not supported under the DirectX9 render interface");
			bValidPrimitiveType = false;
			break;
		case PRIMITIVE_LINE_LOOP:
			CCore::SetError("CGeometryBufferDirectX9::RenderIndexed, PRIMITIVE_LINE_LOOP type is not supported under the DirectX9 render interface");
			bValidPrimitiveType = false;
			break;
		default:
			assert(false);
		}

		if (bValidPrimitiveType)
		{
			assert(DX9::Device( ));
			UINT uiNumVertices = (uiNumVerticesOverride ? uiNumVerticesOverride : m_uiMaxIndex + 1);
			bSuccess = (DX9::Device( )->DrawIndexedPrimitive(eDirectX9Primitive, 0, 0, uiNumVertices, uiStartIndex, nNumPrimitiveElements) == D3D_OK);
		}
	}

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::RenderArrays

inline st_bool CGeometryBufferDirectX9::RenderArrays(EPrimitiveType ePrimType, st_uint32 uiStartVertex, st_uint32 uiNumVertices) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (VertexBufferIsValid( ))
	{
#endif
		st_bool bValidPrimitiveType = true;

		int nNumPrimitiveElements = 1;
		D3DPRIMITIVETYPE eDirectX9Primitive = D3DPT_POINTLIST;
		switch (ePrimType)
		{
		case PRIMITIVE_TRIANGLE_STRIP:
			eDirectX9Primitive = D3DPT_TRIANGLESTRIP;
			nNumPrimitiveElements = uiNumVertices - 2;
			break;
		case PRIMITIVE_TRIANGLES:
			eDirectX9Primitive = D3DPT_TRIANGLELIST;
			nNumPrimitiveElements = uiNumVertices / 3;
			break;
		case PRIMITIVE_LINE_STRIP:
			eDirectX9Primitive = D3DPT_LINESTRIP;
			nNumPrimitiveElements = uiNumVertices - 1;
			break;
		case PRIMITIVE_LINES:
			eDirectX9Primitive = D3DPT_LINELIST;
			nNumPrimitiveElements = uiNumVertices / 2;
			break;
		case PRIMITIVE_TRIANGLE_FAN:
			eDirectX9Primitive = D3DPT_TRIANGLEFAN;
			nNumPrimitiveElements = uiNumVertices - 2;
			break;
		case PRIMITIVE_POINTS:
			eDirectX9Primitive = D3DPT_POINTLIST;
			nNumPrimitiveElements = uiNumVertices;
			break;
		case PRIMITIVE_QUAD_STRIP:
			CCore::SetError("CGeometryBufferDirectX9::RenderArrays, PRIMITIVE_QUAD_STRIP type is not supported under the DirectX9 render interface");
			bValidPrimitiveType = false;
			break;
		case PRIMITIVE_QUADS:
			CCore::SetError("CGeometryBufferDirectX9::RenderArrays, PRIMITIVE_QUADS type is not supported under the DirectX9 render interface");
			bValidPrimitiveType = false;
			break;
		case PRIMITIVE_LINE_LOOP:
			CCore::SetError("CGeometryBufferDirectX9::RenderArrays, PRIMITIVE_LINE_LOOP type is not supported under the DirectX9 render interface");
			bValidPrimitiveType = false;
			break;
		default:
			// let D3DPT_POINTLIST fall through
			break;
		}

		if (bValidPrimitiveType)
		{
			assert(DX9::Device( ));
			bSuccess = (DX9::Device( )->DrawPrimitive(eDirectX9Primitive, uiStartVertex, nNumPrimitiveElements) == D3D_OK);
		}

#ifdef _DEBUG
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::IsPrimitiveTypeSupported

inline st_bool CGeometryBufferDirectX9::IsPrimitiveTypeSupported(EPrimitiveType ePrimType)
{
	return (ePrimType == PRIMITIVE_TRIANGLE_STRIP ||
			ePrimType == PRIMITIVE_TRIANGLES);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::IsFormatSet

inline st_bool CGeometryBufferDirectX9::IsFormatSet(void) const
{
	return (m_pVertexDeclaration != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::IndexSize

inline st_uint32 CGeometryBufferDirectX9::IndexSize(void) const
{
	return (m_eIndexFormat == INDEX_FORMAT_UNSIGNED_16BIT) ? 2 : 4;
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX9::FindMaxIndex

inline void CGeometryBufferDirectX9::FindMaxIndex(const void* pIndexData, st_uint32 uiNumIndices)
{
	m_uiMaxIndex = 0;
	if (m_eIndexFormat == INDEX_FORMAT_UNSIGNED_16BIT)
	{
		st_uint16* pIndices = (st_uint16*) pIndexData;
		for (st_uint32 i = 0; i < uiNumIndices; ++i)
			m_uiMaxIndex = st_max(st_uint32(pIndices[i]), m_uiMaxIndex);
	}
	else // assume 32-bit
	{
		st_uint32* pIndices = (st_uint32*) pIndexData;
		for (st_uint32 i = 0; i < uiNumIndices; ++i)
			m_uiMaxIndex = st_max(pIndices[i], m_uiMaxIndex);
	}
}

