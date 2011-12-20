/**************************************************************************************************
module	:	U2MaxCustAttbExport
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_MAXCUSATTBEXPORT_H
#define U2_MAXCUSATTBEXPORT_H

#include "U2MaxLib.h"

class U2MaxCustAttbExport 
{
public:
	void ExportCustAttb(Animatable *pObj, U2Object *pU2Obj);

	bool ConvertParamBlock(IParamBlock2* pParamBlock, U2Object* pU2Obj); 
};


#endif