/************************************************************************
module	:	U2Keyboard
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_KEYBOARD_H
#define U2_KEYBOARD_H

#include "U2InputDevice.h"

class U2_3D U2Keyboard : public U2InputDevice 
{

public:
	U2Keyboard(LPDIRECTINPUTDEVICE8 pDIDev, uint32 uUsage);


private:
	
	
	unsigned int m_uBufferSize;
	LPDIRECTINPUTDEVICE8 m_pDIDev;

};

typedef U2SmartPtr<U2Keyboard>	U2KeyboardPtr;

#endif