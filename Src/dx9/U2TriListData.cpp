#include <U2_3D/src/U23DLibPCH.h>
#include "U2TriListData.h"


IMPLEMENT_RTTI(U2TriListData, U2TriangleData);

U2TriListData::U2TriListData(unsigned int uiNumVerts, D3DXVECTOR3* pVert, 
			  D3DXVECTOR3 *pNormal, D3DXCOLOR* pColor, D3DXVECTOR2* pTexCoords,
			  unsigned short numTexCoordSets, unsigned short numTriangles, 
			  unsigned short* pusTriList) : U2TriangleData(uiNumVerts, 
			  pVert, pNormal, pColor, pTexCoords, numTexCoordSets, numTriangles)
			 
{
	U2ASSERT(pusTriList);

	m_uIndexCnt = 3 * m_uMaxTriCnt;
	m_pusIndexArray = pusTriList;
	SetType(D3DPT_TRIANGLELIST);
}

U2TriListData::U2TriListData()
{
	m_uIndexCnt = 0;
	m_pusIndexArray = 0;	
	SetType(D3DPT_TRIANGLELIST);

}


U2TriListData::~U2TriListData()
{
	
}


void U2TriListData::SetData(unsigned short usTriangles, 
							 unsigned short* pusTriList)
{
	U2ASSERT(pusTriList);

	SetTriangleCount(usTriangles);
	m_uIndexCnt = 3 * m_uMaxTriCnt;
	m_pusIndexArray = pusTriList;
}


void U2TriListData::GetTriangleIndices(unsigned short triIdx, unsigned short& idx0,
								unsigned short& idx1, unsigned short& idx2) const
{
	unsigned int j0 = 3 * triIdx;
	unsigned int j1 = j0 + 1;
	unsigned int j2 = j1 + 1;
	idx0 = m_pusIndexArray[j0];
	idx1 = m_pusIndexArray[j1];
	idx2 = m_pusIndexArray[j2];
	
}

//-------------------------------------------------------------------------------------------------
uint16 U2TriListData::GetIndexCnt() const
{
	return m_uIndexCnt;
}

//-------------------------------------------------------------------------------------------------
unsigned short* U2TriListData::GetTriList()
{
	return m_pusIndexArray;
}

//-------------------------------------------------------------------------------------------------
const unsigned short* U2TriListData::GetTriList() const
{
	return m_pusIndexArray;
}



//-------------------------------------------------------------------------------------------------
void U2TriListData::UpdateModelNormals()
{
	if(!m_pVertsAttb || !m_pVertsAttb->HasNormal())
		return;

	unsigned short usActiveVerts = GetActiveVertexCount();
	unsigned short i;
	for(i=0; i < usActiveVerts; ++i)
	{
		m_pNorms[i] = VECTOR3_ZERO;
	}

	unsigned short numTris = GetActiveTriangleCount();	
	for(i=0; i < numTris; ++i)
	{
		// get vertex indices
		unsigned short iV0, iV1, iV2;
		GetTriangleIndices(i, iV0, iV1, iV2);

		D3DXVECTOR3 edge1 = m_pVerts[iV1] - m_pVerts[iV0];
		D3DXVECTOR3 edge2 = m_pVerts[iV2] - m_pVerts[iV0];
		D3DXVECTOR3 normal;
		D3DXVec3Cross(&normal, &edge1, &edge2);		

		m_pNorms[iV0] += normal;
		m_pNorms[iV1] += normal;
		m_pNorms[iV2] += normal;	
	}
	
	for(i=0; i < GetActiveVertexCount(); ++i)
	{
		D3DXVECTOR3 tempNorm = m_pNorms[i];
		D3DXVec3Normalize(&m_pNorms[i], &tempNorm);
	}
}

