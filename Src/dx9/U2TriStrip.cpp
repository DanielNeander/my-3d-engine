#include <U2_3D/src/U23DLibPCH.h>
#include "U2TriStrip.h"

IMPLEMENT_RTTI(U2TriStrip, U2Triangle);

U2TriStrip::U2TriStrip(uint16 usVerts, D3DXVECTOR3* pVert, D3DXVECTOR3 *pNormal, 
		   D3DXCOLOR* pColor, D3DXVECTOR2 *pTexCoords, uint16 usNumTexCoordSets,
		   uint16 usTriangle, unsigned short usStrips, unsigned short* pStripLengths, 
		   unsigned short* pusStripLists) :U2Triangle(
		   U2_NEW U2TriStripData(usVerts, pVert, pNormal, pColor, 
		   pTexCoords, usNumTexCoordSets, usTriangle, usStrips, 
		   pStripLengths, pusStripLists))
		   
{

}

U2TriStrip::U2TriStrip(U2TriStripData* pModelData) 
	:U2Triangle(pModelData)
{


}


void U2TriStrip::GetTriangle(uint16 usTriangle, D3DXVECTOR3*& pt0, 
						 D3DXVECTOR3*& pt1, D3DXVECTOR3*& pt2)
{
	U2TriStripData* pData = SmartPtrCast(U2TriStripData, m_spModelData);
	U2ASSERT(usTriangle < pData->GetTriangleCnt());
	D3DXVECTOR3* pVert = pData->GetVertices();

	unsigned short i0, i1, i2;
	GetTriangleIndices(usTriangle, i0, i1, i2);
	pt0 = &pVert[i0];
	pt1 = &pVert[i1];
	pt2 = &pVert[i2];

}

void U2TriStrip::Render(U2Dx9Renderer* pRenderer)
{
	U2Mesh::Render(pRenderer);
	//pRenderer->RenderStrips(this);
}

