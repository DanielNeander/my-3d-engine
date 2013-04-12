#include "stdafx.h"

//--------------------------------------------------------------------------------
// CTriangleIndices
//
// Copyright (C) 2003-2009 Jason Zink. All rights reserved.
//--------------------------------------------------------------------------------
#include "CTriangleIndices.h"
//--------------------------------------------------------------------------------
CTriangleIndices::CTriangleIndices()
{
	m_uiIndices[0] = 0;
	m_uiIndices[1] = 0;
	m_uiIndices[2] = 0;
}
//--------------------------------------------------------------------------------
CTriangleIndices::CTriangleIndices(unsigned int P1, unsigned int P2, unsigned int P3)
{
	m_uiIndices[0] = P1;
	m_uiIndices[1] = P2;
	m_uiIndices[2] = P3;
}
//--------------------------------------------------------------------------------
CTriangleIndices::~CTriangleIndices()
{
}
//--------------------------------------------------------------------------------
CTriangleIndices& CTriangleIndices::operator= (const CTriangleIndices& Triangle)
{
	m_uiIndices[0] = Triangle.m_uiIndices[0];
	m_uiIndices[1] = Triangle.m_uiIndices[1];
	m_uiIndices[2] = Triangle.m_uiIndices[2];

	return *this;
}
//--------------------------------------------------------------------------------
unsigned int CTriangleIndices::P1() const
{
	return(m_uiIndices[0]);
}
//--------------------------------------------------------------------------------
unsigned int& CTriangleIndices::P1()
{
	return(m_uiIndices[0]);
}
//--------------------------------------------------------------------------------
unsigned int CTriangleIndices::P2() const
{
	return(m_uiIndices[1]);
}
//--------------------------------------------------------------------------------
unsigned int& CTriangleIndices::P2()
{
	return(m_uiIndices[1]);
}
//--------------------------------------------------------------------------------
unsigned int CTriangleIndices::P3() const
{
	return(m_uiIndices[2]);
}
//--------------------------------------------------------------------------------
unsigned int& CTriangleIndices::P3()
{
	return(m_uiIndices[2]);
}
//--------------------------------------------------------------------------------
void CTriangleIndices::swapP1P2( )
{
	unsigned int swap = m_uiIndices[0];
	m_uiIndices[0] = m_uiIndices[1];
	m_uiIndices[1] = swap;
}
//--------------------------------------------------------------------------------
void CTriangleIndices::swapP2P3( )
{
	unsigned int swap = m_uiIndices[1];
	m_uiIndices[1] = m_uiIndices[2];
	m_uiIndices[2] = swap;
}
//--------------------------------------------------------------------------------
void CTriangleIndices::swapP3P1( )
{
	unsigned int swap = m_uiIndices[2];
	m_uiIndices[2] = m_uiIndices[0];
	m_uiIndices[0] = swap;
}
//--------------------------------------------------------------------------------