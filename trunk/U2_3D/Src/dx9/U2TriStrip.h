/************************************************************************
module	:	U2TriStripData
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_TRISTRIP_H
#define	U2_TRISTRIP_H

#include "U2Triangle.h"
#include "U2TriStripData.h"

class U2_3D U2TriStrip : public U2Triangle
{
	DECLARE_RTTI;
public:
	U2TriStrip(uint16 usVerts, D3DXVECTOR3* pVert, D3DXVECTOR3 *pNormal, 
		D3DXCOLOR* pColor, D3DXVECTOR2 *pTexCoords, uint16 usNumTexCoordSets,
		uint16 usTriangle, unsigned short usStrips, unsigned short* pStripLengths, 
		unsigned short* pusStripLists);

	U2TriStrip(U2TriStripData* pModelData);

	
	virtual void GetTriangle(uint16 usTriangle, D3DXVECTOR3*& pt0, 
		D3DXVECTOR3*& pt1, D3DXVECTOR3*& pt2);

	virtual void Render(U2Dx9Renderer* pRenderer);

private:
	U2TriStrip();
};

typedef U2SmartPtr<U2TriStrip> U2TriStripPtr;


#endif