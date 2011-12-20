#include <U2_3D/src/U23DLibPCH.h>
#include "U2TriList.h"

IMPLEMENT_RTTI(U2TriList, U2Triangle);

U2TriList::U2TriList(uint16 usVerts, D3DXVECTOR3* pVert, D3DXVECTOR3 *pNormal, 
		  D3DXCOLOR* pColor, D3DXVECTOR2 *pTexCoords, uint16 usNumTexCoordSets,
		  uint16 usTriangle, uint16* pusTriList) :
	U2Triangle(U2_NEW U2TriListData(usVerts, pVert, pNormal, pColor, 
		pTexCoords, usNumTexCoordSets, usTriangle, pusTriList))
{

}

U2TriList::U2TriList(U2TriListData* pModelData) 
	:U2Triangle(pModelData)
{

}

U2TriList::U2TriList()
{

}

uint32 U2TriList::GetTriListLength() const
{
	return 0;
}


uint16* U2TriList::GetTriList() const
{
	return 0;
}


void U2TriList::GetTriangle(uint16 usTriangle, D3DXVECTOR3*& pPT0, 
						 D3DXVECTOR3*& pPT1, D3DXVECTOR3*& pPT2)
{
	U2TriListData* pModelData = 
		SmartPtrCast(U2TriListData, m_spModelData);
	U2ASSERT(usTriangle < pModelData->GetTriangleCnt());
	D3DXVECTOR3* pVertex = pModelData->GetVertices();
	uint16* pusTriList = pModelData->GetTriList();

	uint32 uStart = 3 *usTriangle;
	pPT0 = &pVertex[pusTriList[uStart++]];
	pPT1 = &pVertex[pusTriList[uStart++]];
	pPT2 = &pVertex[pusTriList[uStart]];

}

void U2TriList::Render(U2Dx9Renderer* pRenderer)
{
	U2Mesh::Render(pRenderer);
	pRenderer->RenderTriList(this);
}

