/************************************************************************                                                                      

************************************************************************/
#pragma once 
#ifndef U2_DEBUG_H
#define U2_DEBUG_H

#include "U2DataType.h"
#include "U2Log.h"


#ifdef _DEBUG 

#define DEBUG_PRINT 

#define DEBUG_ASSERT(x, comment) {\
	static bool _ignoreAssert = false;\
	if (!_ignoreAssert && !(x)) \
			{\
			ERROR_RESULT _err_result = notifyAssertion(_T(#x), comment, _T(__FILE__), __LINE__);\
			if (_err_result == ER_IGNORE)\
				{\
				_ignoreAssert = true;\
				}\
				else if (_err_result == ER_BREAKPOINT)\
				{\
				_asm{int 3};\
				}\
			}}

#define DEBUG_ERR(x) {\
	static bool _ignoreError = false;\
	if (!_ignoreError) \
			{\
			ERROR_RESULT _err_result = notifyError((x), \
			__FILE__, __LINE__);\
			if (_err_result == ER_IGNORE)\
				{\
				_ignoreError = true;\
				}\
				else if (_err_result == ER_BREAKPOINT)\
				{\
				_asm{int 3};\
				}\
			}}	

#else 
#define DEBUG_ASSERT(x, comment)
#define DEBUG_ERR	

#endif	// _DEBUG

#define U2ASSERT(x)								//DEBUG_ASSERT(x,_T(""))
#define U2ASSERT2(x, comment)					//DEBUG_ASSERT(x, comment)
#define U2ERR(x)								//DEBUG_ERR(x)
#define TRACE			

// dx9 specific: check HRESULT and display DX9 specific message box
#define U2_DXTrace(hr, msg) { if (FAILED(hr)) DXTrace(__FILE__,__LINE__,hr,msg,true); }


enum ERROR_RESULT
{
	ER_IGNORE = 0,
	ER_CONTINUE,
	ER_BREAKPOINT,
	ER_ABORT
};

U2LIB ERROR_RESULT notifyError(uint32 errorCode, const TCHAR* fileName, int lineNumber);
U2LIB ERROR_RESULT notifyAssertion(const TCHAR* condition, const TCHAR* description, const TCHAR* fileName, int lineNumber);

// Private Functions...
U2LIB ERROR_RESULT displayError(	const TCHAR* errorTitle,
								 const TCHAR* errorText,
								 const TCHAR* errorDescription,
								 const TCHAR* fileName, 
								 int lineNumber);


// Nvidia SDK 9.5 NV_D3DCommonDx9 프로젝트의 NV_Error 발췌.
// 일단 이걸 개발 시간 단축을 위해 이걸 사용하기로 함.

U2LIB void MsgBox( TCHAR * szCaption, TCHAR * szFormat, ... );


U2LIB void FDebug ( TCHAR * szFormat, ... );
U2LIB void FDebug( char * szFormat, ... );



U2LIB void FMsg( const TCHAR * szFormat, ... );
U2LIB void FMsgW( const WCHAR * wszFormat, ... );


// You must make sure that the variable arg list is also char * and NOT WCHAR *
// This function allows FMsg("") in old non-UNICODE builds to work without requiring FMsg(TEXT(""))
U2LIB void FMsg( const char * szFormat, ... );


// Print GetLastError 
U2LIB void ErrorExit(LPTSTR lpszFunction);



#endif

