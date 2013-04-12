#include "stdafx.h"
//--------------------------------------------------------------------------------
// VertexElementDX11
//
// Copyright (C) 2003-2009 Jason Zink. All rights reserved.
//--------------------------------------------------------------------------------
#include "VertexElementDX11.h"
//--------------------------------------------------------------------------------
VertexElementDX11::VertexElementDX11( int float_tuple, int elementCount )
{
	m_iTuple = float_tuple;
	m_iCount = elementCount;
	m_pfData = new float[ m_iTuple * m_iCount ];
}
//--------------------------------------------------------------------------------
VertexElementDX11::~VertexElementDX11()
{
	delete[] m_pfData;
}
//--------------------------------------------------------------------------------
int	VertexElementDX11::SizeInBytes()
{
	return( m_iTuple * sizeof(float) );
}
//--------------------------------------------------------------------------------
int	VertexElementDX11::Tuple()
{
	return( m_iTuple );
}
//--------------------------------------------------------------------------------
int VertexElementDX11::Count()
{
	return( m_iCount );
}
//--------------------------------------------------------------------------------
float* VertexElementDX11::Get1f( int i )
{
	return( &m_pfData[ i ] );
}
//--------------------------------------------------------------------------------
noVec2* VertexElementDX11::Get2f( int i )
{
	return( (noVec2*)&m_pfData[ 2*i ] );
}
//--------------------------------------------------------------------------------
noVec3* VertexElementDX11::Get3f( int i )
{
	return( (noVec3*)&m_pfData[ 3*i ] );
}
//--------------------------------------------------------------------------------
noVec4* VertexElementDX11::Get4f( int i )
{
	return( (noVec4*)&m_pfData[ 4*i ] );
}
//--------------------------------------------------------------------------------
float* VertexElementDX11::operator[]( int i )
{
	return( &m_pfData[ m_iTuple * i ] );
}
//--------------------------------------------------------------------------------
const float* VertexElementDX11::operator[]( int i ) const
{
	return( &m_pfData[ m_iTuple * i ] );
}
//--------------------------------------------------------------------------------
void* VertexElementDX11::GetPtr( int i )
{
	return( &m_pfData[ m_iTuple * i ] );
}
//--------------------------------------------------------------------------------