/************************************************************************
module	:	U2Util
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_UTIL_H
#define U2_UTIL_H

#include "U2PoolAllocator.h"
#include "U2LibType.h"

#include <string>

U2LIB float		GetCurrTimeInSec();
U2LIB uint32	GetPerformanceCounter();	
U2LIB float		GetPerformaceCounterHz();


// U2TString�� �����ϵ��� ����...
// U2TString�� �׽�Ʈ�� ���� �ʾҹǷ� �̰� �����..
// U2GPUPRogram::Load �� ����ϴ� �� ���.
//typedef	std::basic_string<TCHAR, std::char_traits<TCHAR>, U2PoolAllocator<TCHAR> >	U2String;


//#ifdef U2LIB_EXPORT
////template class U2LIB std::basic_string<TCHAR, std::char_traits<TCHAR>, U2PoolAllocator<TCHAR> >;
//#else 
//#ifdef U2LIB_IMPORT
//extern template class U2LIB std::basic_string<TCHAR, std::char_traits<TCHAR>, U2PoolAllocator<TCHAR> >;
//#endif
//#endif

// Char Conversion Function
U2LIB WCHAR* ToUnicode(const char* string);
U2LIB char* FromUnicode(const WCHAR* string);

U2LIB void FatalError(const char* formatString, ...);

// Duplicate strings.
U2LIB TCHAR* U2Strdup(const TCHAR* from);

U2LIB void GetPath(TCHAR* relPath, unsigned int length = MAX_PATH );


#endif