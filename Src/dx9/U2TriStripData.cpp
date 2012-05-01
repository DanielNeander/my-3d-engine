#include <U2_3D/src/U23DLibPCH.h>
#include "U2TriStripData.h"


IMPLEMENT_RTTI(U2TriStripData, U2TriangleData);

U2TriStripData::U2TriStripData(unsigned int uiNumVerts, D3DXVECTOR3* pVert, 
			   D3DXVECTOR3 *pNormal, D3DXCOLOR* pColor, D3DXVECTOR2* pTexCoords,
			   unsigned short numTexCoordSets, unsigned short numTriangles, 
			   unsigned short usStrips, unsigned short* pusStripLengths, 
			   unsigned short* pusStripLists) : U2TriangleData(uiNumVerts, 
			   pVert, pNormal, pColor, pTexCoords, numTexCoordSets, numTriangles)
{
	m_usStrips = usStrips;
	m_pusStripLengths = pusStripLengths;
	m_pusStripLists = pusStripLists;
	SetType(D3DPT_TRIANGLESTRIP);

}

U2TriStripData::U2TriStripData()
{
	m_usStrips = 0;
	m_pusStripLengths = NULL;
	m_pusStripLists = NULL;
	SetType(D3DPT_TRIANGLESTRIP);

}

U2TriStripData::~U2TriStripData()
{
	
	{
		U2_FREE(m_pusStripLengths);
		m_pusStripLengths = NULL;
		U2_FREE(m_pusStripLists);
		m_pusStripLists = NULL;
	}
	
}

void U2TriStripData::GetTriangleIndices(unsigned short i, unsigned short& i0,
								unsigned short& i1, unsigned short& i2) const
{
	U2ASSERT(i < m_uMaxTriCnt);

	unsigned short usTriangle;
	unsigned short usStrip = 0;

	uint16* pusStripLists = m_pusStripLists;

	while(i >= (usTriangle = m_pusStripLengths[usStrip] - 2))
	{
		i -= usTriangle;
		pusStripLists += m_pusStripLengths[usStrip++];
	}

	if(i & 1)
	{
		i0 = pusStripLists[i + 1];
		i1 = pusStripLists[i];
	}
	else 
	{
		i0 = pusStripLists[i];
		i1 = pusStripLists[i + 1];
	}

	i2 = pusStripLists[i + 2];	

}

uint16 U2TriStripData::GetStripCnt() const
{
	return m_usStrips;
}


unsigned short* U2TriStripData::GetStripLengths() const
{
	return m_pusStripLengths;
}


unsigned short* U2TriStripData::GetStripLists() const
{
	return m_pusStripLists;
}


unsigned short U2TriStripData::GetStripLengthSum() const	
{
	return m_uMaxTriCnt + 2 * m_usStrips;
}


void U2TriStripData::GetStripData(unsigned short& usStrips,
						  const unsigned short*& pusStripLengths,
						  const unsigned short*& pusTriList,
						  unsigned int& uiStripLengthSum) const
{
	usStrips = m_usStrips;
	pusStripLengths = m_pusStripLengths;
	pusTriList = m_pusStripLists;
	uiStripLengthSum = GetStripLengthSum();

}