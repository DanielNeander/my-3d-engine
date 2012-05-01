/************************************************************************
module	:	U2TriListData
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_TRILISTDATA_H
#define	U2_TRILISTDATA_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2Lib/Src/U2RefObject.h>
#include <U2Lib/Src/U2SmartPtr.h>
#include "U2TriangleData.h"

class U2_3D U2TriListData : public U2TriangleData
{
	DECLARE_RTTI;
public:

	U2TriListData(unsigned int uiNumVerts, D3DXVECTOR3* pVert, 
		D3DXVECTOR3 *pNormal, D3DXCOLOR* pColor, D3DXVECTOR2* pTexCoords,
		unsigned short numTexCoordSets, unsigned short numTriangles, 
		unsigned short* pTriList);

	U2TriListData();

	virtual ~U2TriListData();

	virtual void GetTriangleIndices(unsigned short triIdx, unsigned short& idx0,
		unsigned short& idx1, unsigned short& idx2) const;

	uint16 GetIndexCnt() const;
	unsigned short* GetTriList();
	const unsigned short* GetTriList() const;

	void SetData(unsigned short usTriangles, 
		unsigned short* pusTriList);	
	
private:	

	virtual void UpdateModelNormals();

	
	
	
};

typedef U2SmartPtr<U2TriListData> U2TriListDataPtr;

#endif 