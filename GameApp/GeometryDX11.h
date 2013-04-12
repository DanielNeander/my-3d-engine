//--------------------------------------------------------------------------------
// GeometryDX11
//
//
// Copyright (C) 2003-2009 Jason Zink. All rights reserved.
//--------------------------------------------------------------------------------
#ifndef GeometryDX11_h
#define GeometryDX11_h
//--------------------------------------------------------------------------------
#include "VertexElementDX11.h"
#include "CTriangleIndices.h"
#include "CLineIndices.h"
#include "CPointIndices.h"
#include "TArray.h"
//--------------------------------------------------------------------------------
class GeometryDX11
{
public:
	GeometryDX11( );
	virtual ~GeometryDX11( );

	void AddElement( VertexElementDX11* element );
	void AddFace( CTriangleIndices& face );
	void AddLine( CLineIndices& line );
	void AddPoint( CPointIndices& point );
	void AddIndex( unsigned int index );

	VertexElementDX11* GetElement( std::string name );
	VertexElementDX11* GetElement( int index );

	unsigned int GetIndex( int index );

	D3D11_PRIMITIVE_TOPOLOGY GetPrimitiveType();
	void SetPrimitiveType( D3D11_PRIMITIVE_TOPOLOGY type );

	int GetPrimitiveCount();
	unsigned int GetIndexCount();
	
	int GetVertexCount();
	int GetElementCount();
	int GetVertexSize();
	int CalculateVertexSize();

	void LoadToBuffers( ID3D11Device* pDevice );

	T_Array<VertexElementDX11*> m_vElements;
	T_Array<unsigned int> m_vIndices;
	
	ID3D11Buffer*	m_pVertexBuffer;
	ID3D11Buffer*	m_pIndexBuffer;

	// The size 
	int m_iVertexSize;
	int m_iVertexCount;

	// The type of primitives listed in the index buffer
	D3D11_PRIMITIVE_TOPOLOGY m_ePrimType;
};
//--------------------------------------------------------------------------------
#endif // GeometryDX11_h
