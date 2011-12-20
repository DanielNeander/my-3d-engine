#include <src/U2LibPCH.h>

#include "U2Util.h"


LARGE_INTEGER g_freq;
LARGE_INTEGER g_startCnt;

float		GetCurrTimeInSec()
{
	static bool bFirst = true;

	if (bFirst)
	{
		QueryPerformanceFrequency(&g_freq);
		QueryPerformanceCounter(&g_startCnt);		
		bFirst = false;
	}

	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return (float)((long double)
		(count.QuadPart - g_startCnt.QuadPart) / 
		(long double) g_freq.QuadPart);
}


uint32	GetPerformanceCounter()
{
	LARGE_INTEGER count;

	QueryPerformanceCounter(&count);
	return count.LowPart;
}

float	GetPerformaceCounterHz()
{
	LARGE_INTEGER ticksPerSec;
	QueryPerformanceFrequency(&ticksPerSec);
	return (float)ticksPerSec.QuadPart;
}


//--------------------------------------------------------------------------------
//
// Convert a string to unicode in a static buffer
//
//--------------------------------------------------------------------------------
WCHAR* ToUnicode(const char* string)
{
	U2ASSERT(string != NULL);

	static WCHAR buffer[4096];
	U2ASSERT(strlen(string) + 1 < (sizeof(buffer) / sizeof(buffer[0])));

	// Convert string to unicode
	MultiByteToWideChar(CP_ACP, 0, string, (int)strlen(string) + 1, buffer, sizeof(buffer) / sizeof(buffer[0]));

	return(buffer);
}



//--------------------------------------------------------------------------------
//
// Convert a unicode string to a c string in a static buffer
//
//--------------------------------------------------------------------------------
char* FromUnicode(const WCHAR* string)
{
	U2ASSERT(string != NULL);

	BOOL unmappableChars;

	static char buffer[4096];
	U2ASSERT(wcslen(string) + 1 < (sizeof(buffer) / sizeof(buffer[0])));

	// Convert unicode to normal c string
	WideCharToMultiByte(CP_ACP, 0, string, (int)wcslen(string) + 1, buffer, sizeof(buffer) / sizeof(buffer[0]), " ", &unmappableChars);

	return(buffer);
}



//-----------------------------------------------------------------------------
// 
// Print an error message and quit
//
//-----------------------------------------------------------------------------
void FatalError(const char* formatString, ...)
{
	va_list argumentList;
	int     returnValue;
	char    outputString[4096];

	va_start(argumentList, formatString);
	returnValue = vsprintf(outputString, formatString, argumentList);
	va_end(argumentList);

#ifdef UNICODE 
	MessageBox(NULL, ToUnicode(outputString), L"Fatal Error!", MB_OK | MB_ICONERROR);
#else 
	MessageBox(NULL, outputString, "Fatal Error!", MB_OK | MB_ICONERROR);
#endif

	exit(-1);
}

//------------------------------------------------------------------------------
/**
A strdup() implementation using Nebula's malloc() override.

- 17-Jan-99   floh    created
*/
TCHAR*
U2Strdup(const TCHAR* from)
{
	U2ASSERT(from);
	TCHAR* to = U2_ALLOC(TCHAR, _tcslen(from) + 1);
	if (to)
	{
		_tcscpy_s(to, _tcslen(from) + 1, from);
	}
	return to;
}



void GetPath(TCHAR* relPath, unsigned int length )
{
	U2FilePath fPath;		
	fPath.ConvertToAbs(relPath, length);
}