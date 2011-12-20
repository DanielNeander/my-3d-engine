#include "stdafx.h"
#include "AppMisc.h"


namespace AppMisc {
//--------------------------------------------------------------------------------------
// Multimon API handling for OSes with or without multimon API support
//--------------------------------------------------------------------------------------
#define APP_PRIMARY_MONITOR ((HMONITOR)0x12340042)
typedef HMONITOR ( WINAPI* LPMONITORFROMWINDOW )( HWND, DWORD );
typedef BOOL ( WINAPI* LPGETMONITORINFO )( HMONITOR, LPMONITORINFO );
typedef HMONITOR ( WINAPI* LPMONITORFROMRECT )( LPCRECT lprcScreenCoords, DWORD dwFlags );

HMONITOR WINAPI MonitorFromWindow( HWND hWnd, DWORD dwFlags )
{
	static bool s_bInited = false;
    static LPMONITORFROMWINDOW s_pFnGetMonitorFromWindow = NULL;
    if( !s_bInited )
    {
        s_bInited = true;
        HMODULE hUser32 = GetModuleHandle( _T("USER32") );
        if( hUser32 ) s_pFnGetMonitorFromWindow = ( LPMONITORFROMWINDOW )GetProcAddress( hUser32,
                                                                                         "MonitorFromWindow" );
    }

    if( s_pFnGetMonitorFromWindow )
        return s_pFnGetMonitorFromWindow( hWnd, dwFlags );
    else
        return APP_PRIMARY_MONITOR;
}

HMONITOR WINAPI MonitorFromRect( LPCRECT lprcScreenCoords, DWORD dwFlags )
{
	    static bool s_bInited = false;
    static LPMONITORFROMRECT s_pFnGetMonitorFromRect = NULL;
    if( !s_bInited )
    {
        s_bInited = true;
        HMODULE hUser32 = GetModuleHandle( _T("USER32") );
        if( hUser32 ) s_pFnGetMonitorFromRect = ( LPMONITORFROMRECT )GetProcAddress( hUser32, "MonitorFromRect" );
    }

    if( s_pFnGetMonitorFromRect )
        return s_pFnGetMonitorFromRect( lprcScreenCoords, dwFlags );
    else
        return APP_PRIMARY_MONITOR;	
}

BOOL WINAPI GetMonitorInfo( HMONITOR hMonitor, LPMONITORINFO lpMonitorInfo )
{
	static bool s_bInited = false;
    static LPGETMONITORINFO s_pFnGetMonitorInfo = NULL;
    if( !s_bInited )
    {
        s_bInited = true;
        HMODULE hUser32 = GetModuleHandle( _T("USER32") );
        if( hUser32 )
        {
            OSVERSIONINFOA osvi = {0}; osvi.dwOSVersionInfoSize = sizeof( osvi );
            GetVersionExA( ( OSVERSIONINFOA* )&osvi );
            bool bNT = ( VER_PLATFORM_WIN32_NT == osvi.dwPlatformId );
            s_pFnGetMonitorInfo = ( LPGETMONITORINFO )( bNT ? GetProcAddress( hUser32,
                                                                              "GetMonitorInfoW" ) :
                                                        GetProcAddress( hUser32, "GetMonitorInfoA" ) );
        }
    }

    if( s_pFnGetMonitorInfo )
        return s_pFnGetMonitorInfo( hMonitor, lpMonitorInfo );

    RECT rcWork;
    if( ( hMonitor == APP_PRIMARY_MONITOR ) && lpMonitorInfo && ( lpMonitorInfo->cbSize >= sizeof( MONITORINFO ) ) &&
        SystemParametersInfoA( SPI_GETWORKAREA, 0, &rcWork, 0 ) )
    {
        lpMonitorInfo->rcMonitor.left = 0;
        lpMonitorInfo->rcMonitor.top = 0;
        lpMonitorInfo->rcMonitor.right = GetSystemMetrics( SM_CXSCREEN );
        lpMonitorInfo->rcMonitor.bottom = GetSystemMetrics( SM_CYSCREEN );
        lpMonitorInfo->rcWork = rcWork;
        lpMonitorInfo->dwFlags = MONITORINFOF_PRIMARY;
        return TRUE;
    }
    return FALSE;

}

//--------------------------------------------------------------------------------------
// Get the desktop resolution of an adapter. This isn't the same as the current resolution 
// from GetAdapterDisplayMode since the device might be fullscreen 
//--------------------------------------------------------------------------------------
void WINAPI GetDesktopResolution( UINT AdapterOrdinal, UINT* pWidth, UINT* pHeight )
{

}


}