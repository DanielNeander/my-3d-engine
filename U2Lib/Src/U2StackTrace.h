/************************************************************************
module	: U2StackTrace
Author	: Greg Snook
Desc	: 가이아 엔진으로 부터 발췌.
************************************************************************/
#ifndef U2_STACKTRACE_H
#define U2_STACKTRACE_H

#include "U2DataType.h"

/*	Stack Tracing
------------------------------------------------------------------------------------------

The stack tracing code is based on an article by John Robbins
that appeared in Microsoft Systems Journal Bugslayer Column - Feb 99

The article can be found at:
http://www.microsoft.com/msj/defaultframe.asp?page=/msj/0299/bugslayer/bugslayer0299.htm

------------------------------------------------------------------------------------------
*/

#ifdef UNICODE 
#define IMAGEHLP_MODULET IMAGEHLP_MODULEW
#define SymGetModuleInfoT SymGetModuleInfoW
#else 
#define IMAGEHLP_MODULET IMAGEHLP_MODULE
#define SymGetModuleInfoT SymGetModuleInfo
#endif


void buildStackTrace(TCHAR* szString, uint32 dwSize, uint32 dwNumSkip);




#endif