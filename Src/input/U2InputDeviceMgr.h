/************************************************************************
module	:	U2InputDeviceMgr
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_INPUTDEVICEMGR_H
#define U2_INPUTDEVICEMGR_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2Lib/Src/U2RefObject.h>
#include "U2Keyboard.h"
#include "U2Mouse.h"

class U2_3D U2InputDeviceMgr : public U2RefObject
{
public:
	virtual ~U2InputDeviceMgr();
	
	static U2InputDeviceMgr* Create(HINSTANCE hOwnInst, HWND hOwndWnd,
		DWORD dwKeyboard,
		DWORD dwMouse);

	static void SetOwnerInstance(HINSTANCE hInst);
	static void SetOwnerWindow(HWND hWnd);

	static void SetKeyboardUsage(DWORD dwKeyboard);
	static void SetMouseUsage(DWORD dwMouse);

	static BOOL CALLBACK  DIEnumDevicesCallback(
		LPDIDEVICEINSTANCE pDDI, LPVOID pvref);
	static BOOL CALLBACK DIEnumDeviceObjectsCallback( 
		LPCDIDEVICEOBJECTINSTANCE pDIDOI, LPVOID pvRef);
	
	U2Keyboard* CreateKeyboard();
	U2Mouse*	CreateMouse();

	const LPDIRECTINPUT8 GetDirectInput8() const;

	class DI8DeivceDesc : public U2MemObj
	{
	public:		
		GUID m_guidInstance;
		TCHAR m_tszInstanceName[MAX_PATH];
		TCHAR m_tszProductName[MAX_PATH];
	};

	bool ConfigureDevices();

	
private:
	U2InputDeviceMgr();

	bool Init();
	bool CreateDirectInput8();

	bool EnumerateDevices();
	bool EnumDeviceObjects(U2InputDeviceMgr::DI8DeivceDesc* pDI8Desc,
		LPDIRECTINPUTDEVICE8 pDI8Dev);

	

	bool m_bEnumerating;

	U2KeyboardPtr m_spKeyboard;
	U2MousePtr m_spMouse;

	LPDIRECTINPUT8 m_pDI8;
	static HINSTANCE ms_hOwnInst;
	static HWND ms_hOwnWnd;
	static DWORD ms_dwKeyboardUsage;		
	static DWORD ms_dwMouseUsage;

};

#endif 
