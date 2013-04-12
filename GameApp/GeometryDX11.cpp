#include "stdafx.h"

//--------------------------------------------------------------------------------
// GeometryDX11
//
// Copyright (C) 2003-2009 Jason Zink. All rights reserved.
//--------------------------------------------------------------------------------
#include "GeometryDX11.h"
//--------------------------------------------------------------------------------
GeometryDX11::GeometryDX11( )
{
	m_iVertexSize = 0;
	m_iVertexCount = 0;

	m_pVertexBuffer = 0;
	m_pIndexBuffer = 0;
}
//--------------------------------------------------------------------------------
GeometryDX11::~GeometryDX11()
{
	for ( int i = 0; i < m_vElements.count(); i++ )
	{
		if ( m_vElements[i] != NULL )
			delete m_vElements[i];
	}

	SAFE_RELEASE( m_pVertexBuffer );
	SAFE_RELEASE( m_pIndexBuffer );
}
//--------------------------------------------------------------------------------
void GeometryDX11::AddElement( VertexElementDX11* element )
{
	int index = -1;
	for (int i = 0; i < m_vElements.count(); i++ )
	{
		if ( m_vElements[i]->m_SemanticName == element->m_SemanticName )
			index = i;
	}

	if ( index == -1 )
	{
		m_vElements.add( element );		// the element wasn't found, so add it
	}
	else
	{
		delete m_vElements[index];		// delete the old element
		m_vElements[index] = element;	// set the new element
	}
}
//--------------------------------------------------------------------------------
void GeometryDX11::AddFace( CTriangleIndices& face )
{
	m_vIndices.add( face.P1() );
	m_vIndices.add( face.P2() );
	m_vIndices.add( face.P3() );
}
//--------------------------------------------------------------------------------
void GeometryDX11::AddLine( CLineIndices& line )
{
	m_vIndices.add( line.P1() );
	m_vIndices.add( line.P2() );
}
//--------------------------------------------------------------------------------
void GeometryDX11::AddPoint( CPointIndices& point )
{
	m_vIndices.add( point.P1() );
}
//--------------------------------------------------------------------------------
void GeometryDX11::AddIndex( UINT index )
{
	m_vIndices.add( index );
}
//--------------------------------------------------------------------------------
VertexElementDX11* GeometryDX11::GetElement( std::string name )
{
	VertexElementDX11* pElement = NULL;
	for ( int i = 0; i < m_vElements.count(); i++ )
	{
		if ( m_vElements[i]->m_SemanticName == name )
			pElement = m_vElements[i];
	}

	return pElement;
}
//--------------------------------------------------------------------------------
VertexElementDX11* GeometryDX11::GetElement( int index )
{
	return( m_vElements[index] );
}
//--------------------------------------------------------------------------------
UINT GeometryDX11::GetIndex( int index )
{
	if ( ( index >= 0 ) && ( index < m_vIndices.count() ) )
		return( m_vIndices[index] );
	
	return( 0 );
}
//--------------------------------------------------------------------------------
D3D11_PRIMITIVE_TOPOLOGY GeometryDX11::GetPrimitiveType()
{
	return( m_ePrimType );
}
//--------------------------------------------------------------------------------
int GeometryDX11::GetPrimitiveCount()
{
	UINT count = 0;
	UINT indices = m_vIndices.count();

	switch ( m_ePrimType )
	{
	case D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED:
		break;
	case D3D11_PRIMITIVE_TOPOLOGY_POINTLIST:
		count = indices; break;
	case D3D11_PRIMITIVE_TOPOLOGY_LINELIST:
		count = indices / 2; break;
	case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP:
		count = indices - 1; break;
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
		count = indices / 3; break;
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		count = indices - 2; break;
	case D3D11_PRIMITIVE_TOPOLOGY_LINELIST_ADJ:
		count = indices / 4; break;
	case D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ:
		count = indices - 3; break;
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ:
		count = indices / 6; break;
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ:
		count = ( indices - 3 ) / 2; break;
	case D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST:
		count = indices; break;
	case D3D11_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST:
		count = indices / 2; break;
	case D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST:
		count = indices / 3; break;
	case D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST:
		count = indices / 4; break;
	case D3D11_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST:
		count = indices / 5; break;
	case D3D11_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST:
		count = indices / 6; break;
	case D3D11_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST:
		count = indices / 7; break;
	case D3D11_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST:
		count = indices / 8; break;
	case D3D11_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST:
		count = indices / 9; break;
	case D3D11_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST:
		count = indices / 10; break;
	case D3D11_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST:
		count = indices / 11; break;
	case D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST:
		count = indices / 12; break;
	case D3D11_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST:
		count = indices / 13; break;
	case D3D11_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST:
		count = indices / 14; break;
	case D3D11_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST:
		count = indices / 15; break;
	case D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST:
		count = indices / 16; break;
	case D3D11_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST:
		count = indices / 17; break;
	case D3D11_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST:
		count = indices / 18; break;
	case D3D11_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST:
		count = indices / 19; break;
	case D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST:
		count = indices / 20; break;
	case D3D11_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST:
		count = indices / 21; break;
	case D3D11_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST:
		count = indices / 22; break;
	case D3D11_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST:
		count = indices / 23; break;
	case D3D11_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST:
		count = indices / 24; break;
	case D3D11_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST:
		count = indices / 25; break;
	case D3D11_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST:
		count = indices / 26; break;
	case D3D11_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST:
		count = indices / 27; break;
	case D3D11_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST:
		count = indices / 28; break;
	case D3D11_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST:
		count = indices / 29; break;
	case D3D11_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST:
		count = indices / 30; break;
	case D3D11_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST:
		count = indices / 31; break;
	case D3D11_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST:
		count = indices / 32; break;
	}

	return( count );
}
//--------------------------------------------------------------------------------
void GeometryDX11::SetPrimitiveType( D3D11_PRIMITIVE_TOPOLOGY type )
{
	m_ePrimType = type;
}
//--------------------------------------------------------------------------------
int GeometryDX11::GetVertexCount()
{
	m_iVertexCount = m_vElements[0]->Count();

	return( m_iVertexCount );
}
//--------------------------------------------------------------------------------
int GeometryDX11::GetElementCount()
{
	return( m_vElements.count() );
}
//--------------------------------------------------------------------------------
int GeometryDX11::GetVertexSize()
{
	return( m_iVertexSize );
}
//--------------------------------------------------------------------------------
int GeometryDX11::CalculateVertexSize()
{
	// Reset the current vertex size 
	m_iVertexSize = 0;

	// Loop through the elements and add their per-vertex size
	for ( int i = 0; i < m_vElements.count(); i++ )
		m_iVertexSize += m_vElements[i]->SizeInBytes();

	return( m_iVertexSize );
}
//--------------------------------------------------------------------------------
void GeometryDX11::LoadToBuffers( ID3D11Device* pDevice )
{
	// Check the size of the assembled vertices
	CalculateVertexSize();

	// Load the vertex buffer first by calculating the required size
	unsigned int vertices_length = GetVertexSize() * GetVertexCount();

	char* pBytes = new char[vertices_length];

	for ( int j = 0; j < m_iVertexCount; j++ )
	{
		int iElemOffset = 0;
		for ( int i = 0; i < m_vElements.count(); i++ )
		{
			memcpy( pBytes + j * m_iVertexSize + iElemOffset, m_vElements[i]->GetPtr(j), m_vElements[i]->SizeInBytes() );
			iElemOffset += m_vElements[i]->SizeInBytes();
		}
	}


	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = reinterpret_cast<void*>( pBytes );
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;

	D3D11_BUFFER_DESC vbuffer;
	vbuffer.ByteWidth = vertices_length;
    vbuffer.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbuffer.MiscFlags = 0;
    vbuffer.StructureByteStride = 0;
	vbuffer.Usage = D3D11_USAGE_IMMUTABLE;
	vbuffer.CPUAccessFlags = 0;
    vbuffer.StructureByteStride = 0;

	pDevice->CreateBuffer( &vbuffer, &data, &m_pVertexBuffer );

	delete [] pBytes; 
	// TODO: add error checking here!

	
	// Load the index buffer by calculating the required size
	// based on the number of indices.

	data.pSysMem = reinterpret_cast<void*>( &m_vIndices[0] );
	data.SysMemPitch = 0;
	data.SysMemSlicePitch = 0;
	
	D3D11_BUFFER_DESC ibuffer;

	ibuffer.ByteWidth = sizeof( unsigned int ) * GetIndexCount();
    ibuffer.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibuffer.MiscFlags = 0;
    ibuffer.StructureByteStride = 0;
	ibuffer.Usage = D3D11_USAGE_IMMUTABLE;
	ibuffer.CPUAccessFlags = 0;
    ibuffer.StructureByteStride = 0;

	pDevice->CreateBuffer( &ibuffer, &data, &m_pIndexBuffer );
	//m_iIB = RendererDX11::Get()->CreateIndexBuffer( &ibuffer, &data );
}
//--------------------------------------------------------------------------------
UINT GeometryDX11::GetIndexCount()
{
	return( m_vIndices.count() );
}
//--------------------------------------------------------------------------------
