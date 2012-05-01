#include <U2_3D/src/U23DLibPCH.h>
#include "U2Keyboard.h"


U2Keyboard::U2Keyboard(LPDIRECTINPUTDEVICE8 pDIDev, 
					   uint32 uUsage)
					   :m_uBufferSize(64),
					   m_pDIDev(pDIDev)
{
	if(!m_pDIDev)
		return;

	HRESULT hr;
	hr = m_pDIDev->SetDataFormat(&c_dfDIKeyboard);
	if(FAILED(hr))
	{
		// Error 
		return;
	}

	// 버퍼 설정 
	DIPROPDWORD diPropDW;
	diPropDW.diph.dwSize = sizeof(DIPROPDWORD);
	diPropDW.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diPropDW.diph.dwObj = 0;
	diPropDW.diph.dwHow = DIPH_DEVICE;
	diPropDW.dwData = m_uBufferSize;

	hr = m_pDIDev->SetProperty(DIPROP_BUFFERSIZE, &diPropDW.diph);
	if(FAILED(hr))
	{
		// Error
		return;
	}

	DWORD dwFlags = 0;

	dwFlags |= DISCL_FOREGROUND; 
	dwFlags |= DISCL_EXCLUSIVE;

	//hr = m_pDIDev->SetCooperativeLevel()
	


}