/************************************************************************
module	:	U2PickRecord
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once
#ifndef U2_PICKRECORD_H
#define U2_PICKRECORD_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2_3D/src/main/U2Spatial.h>

class  U2PickRecord : public U2MemObj
{

public:
	U2SpatialPtr spInterected;

	float fT;

	unsigned int uiTriangle;

	float fB0, fB1, fB2;

	bool operator== (const U2PickRecord& record) const;
	bool operator!= (const U2PickRecord& record) const;
	bool operator<  (const U2PickRecord& record) const;
	bool operator<= (const U2PickRecord& record) const;
	bool operator>  (const U2PickRecord& record) const;
	bool operator>= (const U2PickRecord& record) const;
};

#include "U2PickRecord.inl"

#endif 



