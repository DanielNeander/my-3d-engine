/************************************************************************
module	:	U2TriListData
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_TRILIST_H
#define	U2_TRILIST_H

#include "U2Triangle.h"
#include "U2TrilistData.h"

class U2_3D U2TriList : public U2Triangle
{
	DECLARE_RTTI;
public:
	U2TriList(uint16 usVerts, D3DXVECTOR3* pVert, D3DXVECTOR3 *pNormal, 
		D3DXCOLOR* pColor, D3DXVECTOR2 *pTexCoords, uint16 usNumTexCoordSets,
		uint16 usTriangle, uint16* pusTriList);

	U2TriList(U2TriListData* pModelData);

	uint32 GetTriListLength() const;
	uint16* GetTriList() const;
	virtual void GetTriangle(uint16 usTriangle, D3DXVECTOR3*& pt0, 
		D3DXVECTOR3*& pt1, D3DXVECTOR3*& pt2);

	virtual void Render(U2Dx9Renderer* pRenderer);

private:
	U2TriList();
};

typedef U2SmartPtr<U2TriList> U2TriListPtr;


#endif