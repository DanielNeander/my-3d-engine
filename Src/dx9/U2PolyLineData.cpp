#include <U2_3D/src/U23DLibPCH.h>
#include "U2PolyLineData.h"

U2PolyLineData::U2PolyLineData(unsigned short usNumVerts, D3DXVECTOR3* pVert, 
			   D3DXVECTOR3 *pNormal, D3DXCOLOR* pColor, D3DXVECTOR2* pTexCoords,
			   unsigned short numTexCoordSets, unsigned char* pFlags)
			   :U2MeshData(usNumVerts, pVert, pNormal, pColor, pTexCoords, 
			   numTexCoordSets)
{
	if(pFlags)
	{
		m_pcFlags = pFlags;
	}
	else 
	{
		m_pcFlags = U2_ALLOC(unsigned char, usNumVerts);
		U2ASSERT(m_pcFlags);
		for(unsigned short i=0; i < usNumVerts; ++i)
			m_pcFlags[i] = (i & 1 ? false : true);
	}
}

U2PolyLineData::U2PolyLineData()
{
	m_pcFlags = 0;
}


U2PolyLineData::~U2PolyLineData()
{
	U2_FREE(m_pcFlags);
	m_pcFlags = NULL;
}


void U2PolyLineData::SetConnFlags(unsigned char* pConn)
{
	m_pcFlags = pConn;
}



uint8* U2PolyLineData::GetConnFlags()
{
	return m_pcFlags;
}
//---------------------------------------------------------------------------
const uint8* U2PolyLineData::GetConnFlags() const
{
	return m_pcFlags;
}





