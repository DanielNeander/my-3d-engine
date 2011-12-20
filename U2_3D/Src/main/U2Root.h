#pragma once 
/************************************************************************
module	:	U2Root
Author	:	Yun sangyong

Desc	:	SAVE/LOAD, CLONE, 
************************************************************************/
#include "U2Object.h"


class U2_3D U2Root : public U2Object
{

	virtual bool Save(const char* filename) {}
	virtual bool Load(const char* filename) {}
		

};