//--------------------------------------------------------------------------------
// VertexElementDX11
//
// This is a class to represent generic vertex information.  The elements
// themselves can be either 1 thru 4 floating point values per element.
//
// Currently the arrays are a fixed size.  This may change in the future, but for
// now the user must create new CVertexElements with the new size and manually
// copy over the vertex data to the new object.
//
// Copyright (C) 2003-2009 Jason Zink. All rights reserved.
//--------------------------------------------------------------------------------
#ifndef VertexElementDX11_h
#define VertexElementDX11_h
//--------------------------------------------------------------------------------
#include <string>
//--------------------------------------------------------------------------------
class VertexElementDX11
{

public:
	VertexElementDX11( int tuple, int elementCount );
	~VertexElementDX11( );
	
	int				SizeInBytes();
	int				Count();
	int				Tuple();

	void*			GetPtr( int i );

	float*			Get1f( int i );
	noVec2*		Get2f( int i );
	noVec3*		Get3f( int i );
	noVec4*		Get4f( int i );

	float*					operator[]( int i );
	const float*			operator[]( int i ) const;

	std::string						m_SemanticName;
	unsigned int					m_uiSemanticIndex;
	DXGI_FORMAT						m_Format;
	unsigned int					m_uiInputSlot;
	unsigned int					m_uiAlignedByteOffset;
	D3D11_INPUT_CLASSIFICATION		m_InputSlotClass;
	unsigned int					m_uiInstanceDataStepRate;

protected:
	VertexElementDX11();

	float*							m_pfData;
	int								m_iTuple;
	int								m_iCount;
};
#endif // VertexElementDX11_h
