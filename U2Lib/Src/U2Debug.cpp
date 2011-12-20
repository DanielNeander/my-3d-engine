#include <Src/U2LibPCH.h>
#include "U2Debug.h"

//#include "U2StackTrace.h"

#include <dxerr.h>


#ifdef _DEBUG
//#define _STACKTRACE
#endif


ERROR_RESULT notifyError(uint32 errorCode, const TCHAR* fileName, int lineNumber)
{
	// if no error code is provided, get the last known error
	if (errorCode == 0)
	{
		errorCode = GetLastError();
	}

	// use DirectX to supply a string and description for our error.
	// This will handle all known DirectX error codes (HRESULTs)
	// as well as Win32 error codes normally found via FormatMessage
	//const TCHAR* pErrorString = DXGetErrorString(errorCode);
	//const TCHAR* pErrorDescription = DXGetErrorDescription(errorCode);

	// pass the data on to the message box
	ERROR_RESULT result = displayError(	_T("Debug Error!"),
		NULL,//pErrorString,
		NULL,//pErrorDescription,
		fileName,
		lineNumber);

	// Put the incoming last error back.
	SetLastError(errorCode);

	return(result);
}

ERROR_RESULT notifyAssertion(const TCHAR* condition, const TCHAR* description, const TCHAR* fileName, int lineNumber)
{
	// pass the data on to the message box
	ERROR_RESULT result = displayError(_T("Assert Failed!"),
		condition,
		description,
		fileName,
		lineNumber);

	return(result);

}

ERROR_RESULT displayError(const TCHAR* errorTitle,
									  const TCHAR* errorText,
									  const TCHAR* errorDescription,
									  const TCHAR* fileName, 
									  int lineNumber)
{
	const	int		MODULE_NAME_SIZE = 255;
	TCHAR moduleName[MODULE_NAME_SIZE];

	// attempt to get the module name
	if (!GetModuleFileName(NULL, moduleName, MODULE_NAME_SIZE))
	{
		size_t len =  _tcsclen(_T("<unknown application>"));
		_tcscpy_s(moduleName, len, _T("<unknown application>"));
	}

	// if stack tracing is enabled, build a string containing the 
	// unwound stack information
#ifdef _STACKTRACE
	const	int		STACK_STRING_SIZE = 255;
	TCHAR stackText[STACK_STRING_SIZE];

	buildStackTrace(stackText, STACK_STRING_SIZE, 2);
#else
	TCHAR stackText[] = _T("<stack trace disabled>");
#endif

	// build a collosal string containing the entire asster message
	const	int		MAX_BUFFER_SIZE = 1024;
	TCHAR	buffer[MAX_BUFFER_SIZE];

	int Size = _sntprintf_s(	buffer, 
		MAX_BUFFER_SIZE, 
		_T("%s\n\n")						\
		_T("Program : %s\n")               \
		_T("File : %s\n")                  \
		_T("Line : %d\n")                  \
		/*_T("Error: %s\n")			\
		_T("Comment: %s\n")						\ */
		_T("\nStack:\n%s\n\n")						\
		_T("Abort to exit (or debug), Retry to continue,\n")\
		_T("Ignore to disregard all occurances of this error\n"),
		errorTitle,
		moduleName,
		fileName,
		lineNumber,
		//errorText,
		//errorDescription,
		stackText
		);


	// place a copy of the message into the clipboard
	if (OpenClipboard(NULL))
	{
		size_t bufferLength = _tcsclen(buffer);
		HGLOBAL hMem = GlobalAlloc(GHND|GMEM_DDESHARE, bufferLength+1);

		if (hMem)
		{
			uint8* pMem = (uint8*)GlobalLock(hMem);
			memcpy(pMem, buffer, bufferLength);
			GlobalUnlock(hMem);
			EmptyClipboard();
			SetClipboardData(CF_TEXT, hMem);
		}

		CloseClipboard();
	}


	// find the top most window of the current application
	HWND hWndParent = GetActiveWindow ( ) ;
	if ( NULL != hWndParent )
	{
		hWndParent = GetLastActivePopup ( hWndParent ) ;
	}

	// put up a message box with the error
	int iRet = MessageBox ( hWndParent,
		buffer,
		_T ( "ERROR NOTIFICATION..." ),
		MB_TASKMODAL
		|MB_SETFOREGROUND
		|MB_ABORTRETRYIGNORE
		|MB_ICONERROR);

	// Figure out what to do on the return.
	if (iRet == IDRETRY)
	{
		// ignore this error and continue
		return (ER_CONTINUE);
	}
	if (iRet == IDIGNORE)
	{
		// ignore this error and continue,
		// plus never stop on this error again (handled by the caller)
		return (ER_IGNORE);
	}

	// The return has to be IDABORT, but does the user want to enter the debugger
	// or just exit the application?
	iRet = MessageBox ( hWndParent,
		_T("Do you wish to debug the last error?"),
		_T ( "DEBUG OR EXIT?" ),
		MB_TASKMODAL
		|MB_SETFOREGROUND
		|MB_YESNO
		|MB_ICONQUESTION);

	if (iRet == IDYES)
	{
		// inform the caller to break on the current line of execution
		return (ER_BREAKPOINT);
	}

	// must be a full-on termination of the app
	ExitProcess ( (UINT)-1 ) ;
	return (ER_ABORT);
}


void MsgBox( TCHAR * szCaption, TCHAR * szFormat, ... )
{
	TCHAR buffer[256];
	va_list args;
	va_start( args, szFormat );
	_vsntprintf_s( buffer, sizeof(TCHAR) * 256, 256, szFormat, args );
	va_end( args );
	buffer[256-1] = _T('\0');			// terminate in case of overflow
	::MessageBox( NULL, buffer, szCaption, MB_OK );
}


#ifdef _DEBUG
void FDebug ( TCHAR * szFormat, ... )
{
	// It does not work to call FMsg( szFormat ).  The variable arg list will be incorrect
	static TCHAR buffer[2048];
	va_list args;
	va_start( args, szFormat );
	_vsntprintf_s( buffer, 2048 * sizeof(TCHAR), 2048, szFormat, args );
	va_end( args );
	buffer[2048-1] = _T('\0');			// terminate in case of overflow
	::OutputDebugString( buffer );
	//		Sleep( 2 );		// Interceptors that catch OutputDebugString text sometimes misses lines if
	//  called too rapidly in succession.
}
#ifdef UNICODE
void FDebug( char * szFormat, ... )
{
	static char buffer[2048];
	va_list args;
	va_start( args, szFormat );
	_vsnprintf( buffer, 2048, szFormat, args );
	va_end( args );
	buffer[2048-1] = '\0';			// terminate in case of overflow

#ifdef UNICODE
	int nLen = MultiByteToWideChar( CP_ACP, 0, buffer, -1, NULL, NULL );
	LPWSTR lpszW = new WCHAR[ nLen ];
	if( lpszW != NULL )
	{
		::MultiByteToWideChar( CP_ACP, 0, buffer, -1, lpszW, nLen );
		::OutputDebugString( lpszW );
		delete lpszW;
		lpszW = NULL;
	}
#else
	OutputDebugString( buffer );
#endif
	//		Sleep( 2 );		// OutputDebugString sometimes misses lines if
	//  called too rapidly in succession.
}
#pragma warning( disable : 4100 )	// unreferenced formal parameter
inline void NullFunc( TCHAR * szFormat, ... ) {}
#ifdef UNICODE
inline void NullFunc( char * szFormat, ... ) {}
#endif
#pragma warning( default : 4100 )

#if 0
#define WMDIAG(str) { OutputDebugString(str); }
#else
#define WMDIAG(str) {}
#endif
#endif
#else
inline void FDebug( TCHAR * szFormat, ... )		{ szFormat; }
#ifdef UNICODE
inline void FDebug( char * szFormat, ... )		{ szFormat; }
#endif
inline void NullFunc( char * szFormat, ... )		{ szFormat; }
#define WMDIAG(str) {}
#endif


void FMsg( const TCHAR * szFormat, ... )
{	
	static TCHAR buffer[2048];
	va_list args;
	va_start( args, szFormat );
	_vsntprintf_s( buffer, sizeof(TCHAR) * 2048,  2048, szFormat, args );
	va_end( args );
	buffer[2048-1] = _T('\0');			// terminate in case of overflow
	::OutputDebugString( buffer );
	//	Sleep( 2 );		// Interceptors that catch OutputDebugString text sometimes misses lines if
	//  called too rapidly in succession.
}

void FMsgW( const WCHAR * wszFormat, ... )
{
	WCHAR wbuff[2048];
	va_list args;
	va_start( args, wszFormat );
	_vsnwprintf_s( wbuff, sizeof(WCHAR) * 2048, 2048, wszFormat, args );
	va_end( args );
	wbuff[2048-1] = '\0';				// terminate in case of overflow
	::OutputDebugStringW( wbuff );
	//	Sleep( 2 );		// Interceptors that catch OutputDebugString text sometimes misses lines if
	//  called too rapidly in succession.
}

#ifdef UNICODE
// You must make sure that the variable arg list is also char * and NOT WCHAR *
// This function allows FMsg("") in old non-UNICODE builds to work without requiring FMsg(TEXT(""))
void FMsg( const char * szFormat, ... )
{
	static char buffer[2048];
	va_list args;
	va_start( args, szFormat );
	_vsnprintf( buffer, 2048, szFormat, args );
	va_end( args );
	buffer[2048-1] = '\0';			// terminate in case of overflow
#ifdef UNICODE
	int nLen = MultiByteToWideChar( CP_ACP, 0, buffer, -1, NULL, NULL );
	LPWSTR lpszW = new WCHAR[ nLen ];
	if( lpszW != NULL )
	{
		::MultiByteToWideChar( CP_ACP, 0, buffer, -1, lpszW, nLen );
		::OutputDebugString( lpszW );
		delete lpszW;
		lpszW = NULL;
	}
#else
	OutputDebugString( buffer );
#endif
	//		Sleep( 2 );		// Interceptors that catch OutputDebugString text sometimes misses lines if
	//  called too rapidly in succession.
}
#endif

void ErrorExit(LPTSTR lpszFunction) 
{ 
	TCHAR szBuf[80]; 
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError(); 

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );

	wsprintf(szBuf, 
		"%s failed with error %d: %s", 
		lpszFunction, dw, lpMsgBuf); 

	FDebug(szBuf);

	LocalFree(lpMsgBuf);	
}