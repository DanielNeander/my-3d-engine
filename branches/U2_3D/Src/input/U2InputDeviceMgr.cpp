#include <U2_3D/src/U23DLibPCH.h>
#include "U2InputDeviceMgr.h"
#include <U2_3D/src/Dx9/U2Dx9Renderer.h>


#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")


HINSTANCE U2InputDeviceMgr::ms_hOwnInst = 0;
HWND U2InputDeviceMgr::ms_hOwnWnd = 0;

DWORD U2InputDeviceMgr::ms_dwKeyboardUsage = 0;
DWORD U2InputDeviceMgr::ms_dwMouseUsage = 0;


U2InputDeviceMgr::U2InputDeviceMgr()
	:m_pDI8(0),
	m_bEnumerating(false)	
{

}

U2InputDeviceMgr::~U2InputDeviceMgr()
{
	
}


U2InputDeviceMgr* U2InputDeviceMgr::Create(HINSTANCE hOwnInst, HWND hOwndWnd, 
										   DWORD dwKeyboard,
										   DWORD dwMouse)
{

	U2InputDeviceMgr *pIDMgr = U2_NEW U2InputDeviceMgr();

	if(pIDMgr)
	{
		pIDMgr->SetOwnerInstance(hOwnInst);
		pIDMgr->SetOwnerWindow(hOwndWnd);
		pIDMgr->SetKeyboardUsage(dwKeyboard);
		pIDMgr->SetMouseUsage(dwMouse);

		if(!pIDMgr->Init())
		{
			U2_DELETE pIDMgr;
		}
		else 
		{
			pIDMgr->EnumerateDevices();
		}
		
	}
	else 
	{
		U2ASSERT(_T("Failed to Create U2InputDeviceMgr."));
	}

	return pIDMgr;
}

bool U2InputDeviceMgr::Init()
{
	if(!CreateDirectInput8())
		return false;

	return true;
}


const LPDIRECTINPUT8 U2InputDeviceMgr::GetDirectInput8() const
{
	return m_pDI8;
}


bool U2InputDeviceMgr::ConfigureDevices()
{
	DICOLORSET dics;
	ZeroMemory((void*)&dics, sizeof(DICOLORSET));
	dics.dwSize = sizeof(DICOLORSET);

	DICONFIGUREDEVICESPARAMS dicdp;
	ZeroMemory((void*)&dicdp, sizeof(DICONFIGUREDEVICESPARAMS));
	dicdp.dwSize = sizeof(dicdp);
	dicdp.dwcUsers = 1;
	dicdp.lptszUserNames = NULL;

	dicdp.dwcFormats = 1;
	dicdp.lprgFormats = NULL;
	dicdp.hwnd = ms_hOwnWnd;

	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	U2ASSERT(pRenderer);
//	D3DPRESENT_PARAMETERS* pD3DPP = pRenderer->Get

	return true;
}



U2Keyboard* U2InputDeviceMgr::CreateKeyboard()
{
	if(!m_pDI8)
	{
		return NULL;
	}

	if(m_spKeyboard)
		return m_spKeyboard;

	LPDIRECTINPUTDEVICE8 pDIDev = NULL;
	HRESULT hr = m_pDI8->CreateDevice(GUID_SysKeyboard, &pDIDev, NULL);
	if(FAILED(hr))
	{
		return 0;
	}

	U2Keyboard* pDI8Keyboard = U2_NEW U2Keyboard(pDIDev, ms_dwKeyboardUsage);
	if(!pDI8Keyboard)
	{
		U2_DELETE pDI8Keyboard;
		return 0;
	}

	m_spKeyboard = pDI8Keyboard;

	return m_spKeyboard;
}

U2Mouse*	U2InputDeviceMgr::CreateMouse()
{
	if(!m_pDI8)
	{
		return NULL;
	}

	if(m_spMouse)
		return m_spMouse;

	LPDIRECTINPUTDEVICE8 pDIDev = 0;

	HRESULT hr = m_pDI8->CreateDevice(GUID_SysMouse, &pDIDev, NULL);
	if(FAILED(hr))
	{
		return 0;
	}

	U2Mouse* pMouse = U2_NEW U2Mouse(pDIDev, ms_dwMouseUsage);
	if(!pMouse)
	{
		U2_DELETE pMouse;
		return NULL;
	}

	m_spMouse = pMouse;

	return m_spMouse;
}


BOOL CALLBACK U2InputDeviceMgr::DIEnumDeviceObjectsCallback( 
	LPCDIDEVICEOBJECTINSTANCE pDIDOI, LPVOID pvRef)
{
	DI8DeivceDesc *pDI8Desc = (DI8DeivceDesc*)pvRef;
	if(!pDI8Desc)
	{
		U2ASSERT("DIENumDeviceObjectsCallback Failed");
		return DIENUM_STOP;
	}

	return DIENUM_CONTINUE;	
}

BOOL CALLBACK  U2InputDeviceMgr::DIEnumDevicesCallback(LPDIDEVICEINSTANCE pDDI, LPVOID pvRef)
{
	U2InputDeviceMgr* pIDMgr = (U2InputDeviceMgr*)pvRef;
	if(!pIDMgr)
	{
		U2ASSERT(_T("EnumDeviceProc is no InputDeviceMgr"));
		return DIENUM_STOP;
	}

	LPDIRECTINPUT8 pDI8 = pIDMgr->GetDirectInput8();
	if(!pDI8)
	{
		U2ASSERT(_T("EnumDeviceProc is No Direct Input 8"));
	}

	LPDIRECTINPUTDEVICE8 pDI8Dev = NULL;
	HRESULT hr = pDI8->CreateDevice(pDDI->guidInstance, &pDI8Dev, NULL);
	if(FAILED(hr))
		return DIENUM_CONTINUE;

	bool bValidDevice = false; 

	DI8DeivceDesc *pDI8Desc = U2_NEW DI8DeivceDesc();
	U2ASSERT(pDI8Desc);

	if(memcmp((const void*)&GUID_SysKeyboard, (const void*)&(pDDI->guidInstance), 
		sizeof(GUID)) == 0)
	{
		LOG_INFO << _T("System Keyboard Found.\n");
	}

	if(memcmp((const void*)&GUID_SysMouse, (const void*)&(pDDI->guidInstance), 
		sizeof(GUID)) == 0)
	{
		LOG_INFO << _T("System Mouse Found.\n");
	}

	switch (pDDI->dwDevType & 0x000000ff)
	{
	case DI8DEVTYPE_1STPERSON:
	case DI8DEVTYPE_DRIVING:
	case DI8DEVTYPE_FLIGHT:
	case DI8DEVTYPE_GAMEPAD:
	case DI8DEVTYPE_JOYSTICK:
	case DI8DEVTYPE_SUPPLEMENTAL:
		break;
	case DI8DEVTYPE_KEYBOARD:
		bValidDevice = true;
		break;
	case DI8DEVTYPE_MOUSE:
	case DI8DEVTYPE_SCREENPOINTER:
		bValidDevice = true;
		break;
	case DI8DEVTYPE_DEVICE:
	case DI8DEVTYPE_DEVICECTRL:
	case DI8DEVTYPE_REMOTE:
		break;
	default:
		break;
	}

	if(bValidDevice)
	{		
		pDI8Desc->m_guidInstance = pDDI->guidInstance;
		_tcscpy_s(pDI8Desc->m_tszInstanceName, MAX_PATH, pDDI->tszInstanceName);
		_tcscpy_s(pDI8Desc->m_tszProductName, MAX_PATH, pDDI->tszProductName);
		// 입력 장치를 나열.
		pIDMgr->EnumDeviceObjects(pDI8Desc, pDI8Dev);
	}

	pDI8Dev->Release();

	return DIENUM_CONTINUE;
}

bool U2InputDeviceMgr::EnumDeviceObjects(U2InputDeviceMgr::DI8DeivceDesc* pDI8Desc,
	LPDIRECTINPUTDEVICE8 pDI8Dev)
{
	if(!pDI8Dev)
		return false;

	pDI8Dev->EnumObjects(U2InputDeviceMgr::DIEnumDeviceObjectsCallback, 
		(void*)pDI8Desc, DIDFT_ALL);

	return true;
}



bool U2InputDeviceMgr::EnumerateDevices()
{
	if(!m_pDI8)
	{
		return false;
	}

	if(m_bEnumerating)
		return false;

	//HRESULT hr = m_pDI8->EnumDevices(DI8DEVCLASS_ALL, 
	//	U2InputDeviceMgr::DIEnumDevicesCallback, this, DIEDFL_ATTACHEDONLY);
	//if(FAILED(hr))
	//	return false;

	m_bEnumerating = true;
	return true;	
}


void U2InputDeviceMgr::SetOwnerInstance(HINSTANCE hInst)
{
	ms_hOwnInst = hInst;
}

void U2InputDeviceMgr::SetOwnerWindow(HWND hWnd)
{
	ms_hOwnWnd = hWnd;
}

void U2InputDeviceMgr::SetKeyboardUsage(DWORD dwKeyboard)
{
	ms_dwKeyboardUsage = dwKeyboard;
}
void U2InputDeviceMgr::SetMouseUsage(DWORD dwMouse)
{
	ms_dwMouseUsage = dwMouse;
}


bool U2InputDeviceMgr::CreateDirectInput8()
{
	if((ms_hOwnInst == 0)|| (ms_hOwnWnd == 0))
	{
		return false;
	}

	HRESULT hr = DirectInput8Create(ms_hOwnInst, DIRECTINPUT_VERSION, 
		IID_IDirectInput8A, (void**)(&m_pDI8), NULL);
	if(FAILED(hr))
	{
		//Error
		return false;
	}

	return true;
}
