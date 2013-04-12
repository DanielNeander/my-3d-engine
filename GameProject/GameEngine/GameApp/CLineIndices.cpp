#include "stdafx.h"
//--------------------------------------------------------------------------------
// CLineIndices
//
// Copyright (C) 2003-2009 Jason Zink. All rights reserved.
//--------------------------------------------------------------------------------
#include "CLineIndices.h"
//--------------------------------------------------------------------------------
CLineIndices::CLineIndices()
{
	m_uiIndices[0] = 0;
	m_uiIndices[1] = 0;
}
//--------------------------------------------------------------------------------
CLineIndices::CLineIndices( unsigned int P1, unsigned int P2 )
{
	m_uiIndices[0] = P1;
	m_uiIndices[1] = P2;
}
//--------------------------------------------------------------------------------
CLineIndices::~CLineIndices()
{
}
//--------------------------------------------------------------------------------
CLineIndices& CLineIndices::operator= ( const CLineIndices& Line )
{
	m_uiIndices[0] = Line.m_uiIndices[0];
	m_uiIndices[1] = Line.m_uiIndices[1];

	return( *this );
}
//--------------------------------------------------------------------------------
unsigned int CLineIndices::P1() const
{
	return( m_uiIndices[0] );
}
//--------------------------------------------------------------------------------
unsigned int& CLineIndices::P1()
{
	return( m_uiIndices[0] );
}
//--------------------------------------------------------------------------------
unsigned int CLineIndices::P2() const
{
	return( m_uiIndices[1] );
}
//--------------------------------------------------------------------------------
unsigned int& CLineIndices::P2()
{
	return( m_uiIndices[1] );
}
//--------------------------------------------------------------------------------
void CLineIndices::swapP1P2( )
{
	unsigned int swap = m_uiIndices[0];
	m_uiIndices[0] = m_uiIndices[1];
	m_uiIndices[1] = swap;
}
//--------------------------------------------------------------------------------
