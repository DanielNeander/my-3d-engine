//--------------------------------------------------------------------------------
// CLineIndices 
//
// This class represents the two indices of a line.
//
// Copyright (C) 2003-2009 Jason Zink. All rights reserved.
//--------------------------------------------------------------------------------
#ifndef CLineIndices_h
#define CLineIndices_h
//--------------------------------------------------------------------------------
class CLineIndices
{
public:
	CLineIndices();
	CLineIndices( unsigned int P1, unsigned int P2 );
	~CLineIndices();

	CLineIndices& operator= ( const CLineIndices& Line );

	// member access
	unsigned int P1( ) const;
	unsigned int& P1( );
	unsigned int P2( ) const;
	unsigned int& P2( );

	void swapP1P2( );

protected:
	unsigned int m_uiIndices[2];
};
//--------------------------------------------------------------------------------
#endif // CLineIndices_h
