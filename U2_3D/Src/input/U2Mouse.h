/************************************************************************
module	:	U2Mouse
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_MOUSE_H
#define U2_MOUSE_H

#include "U2InputDevice.h"

class U2_3D U2Mouse : public U2InputDevice 
{

public:
	U2Mouse(LPDIRECTINPUTDEVICE8 pDIDev, uint32 uUsage);


private:



	LPDIRECTINPUTDEVICE8 m_pDIDev;

};

typedef U2SmartPtr<U2Mouse>	U2MousePtr;

#endif
