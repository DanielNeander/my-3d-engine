//--------------------------------------------------------------------------------
// CPointIndices 
//
// This class represents the single index of a point.
//
// Copyright (C) 2003-2009 Jason Zink. All rights reserved.
//--------------------------------------------------------------------------------
#ifndef CPointIndices_h
#define CPointIndices_h
//--------------------------------------------------------------------------------
class CPointIndices
{
public:
	CPointIndices();
	CPointIndices( unsigned int P1 );
	~CPointIndices();

	CPointIndices& operator= ( const CPointIndices& Point );

	// member access
	unsigned int P1( ) const;
	unsigned int& P1( );

protected:
	unsigned int m_uiIndices[1];
};
//--------------------------------------------------------------------------------
#endif // CPointIndices_h
