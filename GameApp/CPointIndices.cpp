#include "stdafx.h"

//--------------------------------------------------------------------------------
// CPointIndices
//
// Copyright (C) 2003-2009 Jason Zink. All rights reserved.
//--------------------------------------------------------------------------------
#include "CPointIndices.h"
//--------------------------------------------------------------------------------
CPointIndices::CPointIndices()
{
	m_uiIndices[0] = 0;
}
//--------------------------------------------------------------------------------
CPointIndices::CPointIndices( unsigned int P1 )
{
	m_uiIndices[0] = P1;
}
//--------------------------------------------------------------------------------
CPointIndices::~CPointIndices()
{
}
//--------------------------------------------------------------------------------
CPointIndices& CPointIndices::operator= (const CPointIndices& Point)
{
	m_uiIndices[0] = Point.m_uiIndices[0];

	return( *this );
}
//--------------------------------------------------------------------------------
unsigned int CPointIndices::P1() const
{
	return( m_uiIndices[0] );
}
//--------------------------------------------------------------------------------
unsigned int& CPointIndices::P1()
{
	return( m_uiIndices[0] );
}
//--------------------------------------------------------------------------------
