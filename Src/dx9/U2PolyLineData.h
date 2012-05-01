/************************************************************************
module	:	U2PolyLineData
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once
#ifndef U2_POLYLINEDATA_H
#define U2_POLYLINEDATA_H

#include "U2MeshData.h"

class U2_3D U2PolyLineData : public U2MeshData 
{

public:

	U2PolyLineData(unsigned short usNumVerts, D3DXVECTOR3* pVert, 
		D3DXVECTOR3 *pNormal, D3DXCOLOR* pColor, D3DXVECTOR2* pTexCoords,
		unsigned short numTexCoordSets, unsigned char* pFlags);

	U2PolyLineData();
	
	virtual ~U2PolyLineData();

	// Line  Connectivity Flags

	void SetConnFlags(unsigned char* pConn);

	uint8* GetConnFlags();
	const uint8* GetConnFlags() const;

private:
	uint8* m_pcFlags;	
};

typedef U2SmartPtr<U2PolyLineData> U2PolyLineDataPtr;


#endif
