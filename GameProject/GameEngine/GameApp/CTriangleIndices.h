//--------------------------------------------------------------------------------
// CTriangleIndices 
//
// This class represents the three indices of a triangle.
//
// Copyright (C) 2003-2009 Jason Zink. All rights reserved.
//--------------------------------------------------------------------------------
#ifndef CTriangleIndices_h
#define CTriangleIndices_h
//--------------------------------------------------------------------------------
class CTriangleIndices
{
public:
	CTriangleIndices();
	CTriangleIndices( unsigned int P1, unsigned int P2, unsigned int P3 );
	~CTriangleIndices();

	CTriangleIndices& operator= ( const CTriangleIndices& Triangle );

	// member access
	unsigned int P1( ) const;
	unsigned int& P1( );
	unsigned int P2( ) const;
	unsigned int& P2( );
	unsigned int P3( ) const;
	unsigned int& P3( );

	void swapP1P2( );
	void swapP2P3( );
	void swapP3P1( );

protected:
	unsigned int m_uiIndices[3];
};
//--------------------------------------------------------------------------------
#endif // CTriangleIndices_h
