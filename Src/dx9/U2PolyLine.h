/************************************************************************
module	:	U2PolyLine
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once
#ifndef U2_PolyLine_H
#define U2_PolyLine_H

#include "U2PolyLineData.h"

class U2_3D U2PolyLine : public U2Mesh
{
	DECLARE_RTTI;
public:

	U2PolyLine(U2PolyLineData* pModelData);
	
	void SetConnFlags(unsigned char* pConn);

	unsigned char* GetConnFlags();
	const unsigned char* GetConnFlags() const;

	virtual void Render(U2Dx9Renderer* pRenderer);


private:
	U2PolyLine();	

};

typedef U2SmartPtr<U2PolyLine> U2PolyLinePtr;

#endif