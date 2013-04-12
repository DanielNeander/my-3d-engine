#include "stdafx.h"
#include <ShellAPI.h>
#include <direct.h>

#include <mmsystem.h>
#pragma comment (lib, "winmm.lib")

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#include <stdio.h>

#include ".\BaseTypes\BaseTypes.h"
#include ".\Interfaces\Interface.h"
#include ".\Framework\FrameworkAPI.h"
#include ".\Framework\EnvironmentManager.h"

#include "EngineCore/Windows/resource.h"
#include "EngineCore/CPU.h"
#include "EngineCore/BaseApp.h"
#include "GameApp/Util/CommandSystem.h"


extern BaseApp *app;

#define GETX(l) (int(l & 0xFFFF))
#define GETY(l) (int(l) >> 16)

BOOL g_activeApp = TRUE;

LRESULT CALLBACK WinProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message){
	case WM_PAINT:
		PAINTSTRUCT paint;
		BeginPaint(hwnd, &paint);
		g_activeApp = !IsRectEmpty(&paint.rcPaint);
		EndPaint(hwnd, &paint);
		break;
	case WM_MOUSEMOVE:
		static int lastX, lastY;
		int x, y;
		x = GETX(lParam);
		y = GETY(lParam);
		app->onMouseMove(x, y, x - lastX, y - lastY);
		lastX = x;
		lastY = y;		
		break;
	case WM_CHAR:
		CommandSystem::getInstance().OnChar((unsigned int)wParam);
		break;
	case WM_KEYDOWN:
		app->onKey((unsigned int) wParam, true);
		break;
	case WM_KEYUP:
		app->onKey((unsigned int) wParam, false);
		break;
	case WM_LBUTTONDOWN:
		app->onMouseButton(GETX(lParam), GETY(lParam), MOUSE_LEFT, true);
		break;
	case WM_LBUTTONUP:
		app->onMouseButton(GETX(lParam), GETY(lParam), MOUSE_LEFT, false);		
		break;
	case WM_MBUTTONDOWN:
		app->onMouseButton(GETX(lParam), GETY(lParam), MOUSE_MIDDLE, true);		
		break;
	case WM_MBUTTONUP:
		app->onMouseButton(GETX(lParam), GETY(lParam), MOUSE_MIDDLE, false);		
		break;
	case WM_RBUTTONDOWN:
		app->onMouseButton(GETX(lParam), GETY(lParam), MOUSE_RIGHT, true);
		break;
	case WM_RBUTTONUP:
		app->onMouseButton(GETX(lParam), GETY(lParam), MOUSE_RIGHT, false);
		break;
	case WM_MOUSEWHEEL:
		static int scroll;
		int s;

		scroll += GET_WHEEL_DELTA_WPARAM(wParam);
		s = scroll / WHEEL_DELTA;
		scroll %= WHEEL_DELTA;

		POINT point;
		point.x = GETX(lParam);
		point.y = GETY(lParam);
		ScreenToClient(hwnd, &point);

		if (s != 0) app->onMouseWheel(point.x, point.y, s);
		break;
	case WM_SIZE:
		app->onSize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_WINDOWPOSCHANGED:
		if ((((LPWINDOWPOS) lParam)->flags & SWP_NOSIZE) == 0){
			RECT rect;
			GetClientRect(hwnd, &rect);
			int w = rect.right - rect.left;
			int h = rect.bottom - rect.top;
			if (w > 0 && h > 0) app->onSize(w, h);
		}
		break;
	case WM_SYSKEYDOWN:
		if ((lParam & (1 << 29)) && wParam == KEY_ENTER){
			app->toggleFullscreen();
		} else {
			app->onKey((unsigned int) wParam, true);
		}
		break;
	case WM_SYSKEYUP:
		app->onKey((unsigned int) wParam, false);
		break;
	case WM_CREATE:
		ShowWindow(hwnd, SW_SHOW);
		break;
	case WM_CLOSE:
		app->closeWindow(true, true);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

extern "C" int APIENTRY
	_tWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR pszCmdLine,
	int CmdShow
	)
{
	USES_CONVERSION;

	WCHAR**				argv;
	int					argc, iArg;
	std::string			sGdfPath = "U2.gdf";
	bool				bLogging;

	//UNREFERENCED_PARAM( hInstance );
	UNREFERENCED_PARAM( hPrevInstance );
	UNREFERENCED_PARAM( pszCmdLine );
	UNREFERENCED_PARAM( CmdShow );

	initCPU();

	// Make sure we're running in the exe's path
	char path[MAX_PATH];
	if (GetModuleFileName(NULL, path, sizeof(path))){
		char *slash = strrchr(path, '\\');
		if (slash) *slash = '\0';
		chdir(path);
	}

	MSG msg;
	WNDCLASS wincl;
	wincl.hInstance = hInstance;
	wincl.lpszClassName = "Game";
	wincl.lpfnWndProc = WinProc;
	wincl.style = 0;
	wincl.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINICON));
	wincl.hCursor = LoadCursor(NULL, IDI_APPLICATION);
	wincl.lpszMenuName = NULL;
	wincl.cbClsExtra = 0;
	wincl.cbWndExtra = 0;
	wincl.hbrBackground = NULL;
	if (!RegisterClass(&wincl)) return 0;


	app->setInstance(hInstance);
		
	// Initialize timer
	app->initTime();

	app->loadConfig();
	app->initGUI();

	// 
	// Parse command line 
	// 
	bLogging = false;

	argv = CommandLineToArgvW( GetCommandLineW(), &argc);
	for(iArg=1; iArg<argc; iArg++)
	{
		switch (*argv[iArg])
		{
		case '-':
		case '/':
			{
				if (0==_wcsicmp(argv[iArg]+1, L"logging" ))		bLogging 		= true;
				// [moved to SystemGraphicsOGRE] if (0==_wcsicmp(argv[iArg]+1, L"windowed" ))	Options.bForceWindowed	= true;
			}	break;

		default:
			{
				sGdfPath = W2A( argv[iArg] );
			}
		}
	}
	LocalFree(argv);

	//
	// Start up debug functionality.
	//
	// NOTE: checking the environment will never work, because the Environment hasn't been read yet
	//Bool bLogging = Singletons::EnvironmentManager.Variables().GetAsBool( "Debug::Logging", False );
	Debug::Startup( bLogging );

	if (app->init()){
		app->resetCamera();

		do {
			app->loadConfig();

			if (!app->initCaps()) break;
			if (!app->initAPI()) break;

			if (!app->load()){
				app->closeWindow(true, false);
			}

			//
			// Call the engine to start execution.
			//
			//EngineExecuteGDF(sGdfPath.c_str());
			ExecuteGDF(sGdfPath.c_str());

		} while (!app->isDone());

		app->exit();
	}

	delete app;


	//
	// Shutdown debug functionality.
	//
	Debug::Shutdown();

	return 0;
}