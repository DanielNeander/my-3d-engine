#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <tchar.h>
#include <string>

namespace AppMisc {

HMONITOR WINAPI MonitorFromWindow( HWND hWnd, DWORD dwFlags );
HMONITOR WINAPI MonitorFromRect( LPCRECT lprcScreenCoords, DWORD dwFlags );
BOOL WINAPI GetMonitorInfo( HMONITOR hMonitor, LPMONITORINFO lpMonitorInfo );
void WINAPI GetDesktopResolution( UINT AdapterOrdinal, UINT* pWidth, UINT* pHeight );

inline void MakeUpperCase(std::string &str) {
	for (std::string::iterator i = str.begin(); i != str.end(); i++) {
		*i = toupper(*i);
	}
}

inline std::string RemoveQuotes(std::string &str) {
	for (std::string::iterator i = str.begin(); i != str.end(); i++) {
		if (*i == '\"') {
			i = str.erase(i); if (i == str.end()) break;
		}
	}
	return(str);
}

};