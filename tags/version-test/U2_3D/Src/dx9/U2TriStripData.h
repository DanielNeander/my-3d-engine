/************************************************************************
module	:	U2TriStripData
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_TRISTRIPDATA_H
#define	U2_TRISTRIPDATA_H

#include <U2_3D/src/U2_3DLibType.h>
#include "U2TriangleData.h"

class U2_3D U2TriStripData : public U2TriangleData
{
	DECLARE_RTTI;
public:

	U2TriStripData(unsigned int uiNumVerts, D3DXVECTOR3* pVert, 
		D3DXVECTOR3 *pNormal, D3DXCOLOR* pColor, D3DXVECTOR2* pTexCoords,
		unsigned short numTexCoordSets, unsigned short numTriangles, 
		unsigned short usStrips, unsigned short* pStripLengths, 
		unsigned short* pusStripLists);

	U2TriStripData();

	virtual ~U2TriStripData();

	virtual void GetTriangleIndices(unsigned short triIdx, unsigned short& idx0,
		unsigned short& idx1, unsigned short& idx2) const;

	uint16 GetStripCnt() const;
	unsigned short* GetStripLengths() const;
	unsigned short* GetStripLists() const;
	unsigned short GetStripLengthSum() const;	

	void SetStripLengths(unsigned short* pStripLengths) { m_pusStripLengths = pStripLengths; }
	void SetStripLists(unsigned short* pusStripLists) { m_pusStripLists = pusStripLists; }
	void SetStripsCnt(unsigned short usStrips) { m_usStrips = usStrips; }

	virtual void GetStripData(unsigned short& usStrips,
		const unsigned short*& pusStripLengths,
		const unsigned short*& pusTriList,
		unsigned int& uiStripLengthSum) const;

private:	
	unsigned short m_usStrips;
	unsigned short* m_pusStripLengths;
	unsigned short* m_pusStripLists;

};

typedef U2SmartPtr<U2TriStripData> U2TriStripDataPtr;

#endif 