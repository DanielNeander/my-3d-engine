#pragma once 

#pragma pack(push,8)
#ifndef STRICT
#define STRICT
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#undef PF_MAX
#pragma pack(pop)

#include "EngineCore/Types.h"


#define VARARG_EXTRA(A) A,
#define VARARG_NONE
#define VARARG_PURE =0

static inline DWORD			CheckVA(DWORD dw)		{ return dw; }
static inline BYTE			CheckVA(BYTE b)			{ return b; }
static inline UINT			CheckVA(UINT ui)		{ return ui; }
static inline INT			CheckVA(INT i)			{ return i; }
static inline QWORD			CheckVA(QWORD qw)		{ return qw; }
static inline SQWORD		CheckVA(SQWORD sqw)		{ return sqw; }
static inline DOUBLE		CheckVA(DOUBLE d)		{ return d; }
static inline TCHAR			CheckVA(TCHAR c)		{ return c; }
static inline void*			CheckVA(ANSICHAR* s)	{ return (void*)s; }
template<class T> T*		CheckVA(T* p)			{ return p; }
template<class T> const T*	CheckVA(const T* p)		{ return p; }

#define VARARG_DECL( FuncRet, StaticFuncRet, Return, FuncName, Pure, FmtType, ExtraDecl, ExtraCall )	\
	FuncRet FuncName##__VA( ExtraDecl FmtType Fmt, ... ) Pure;  \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt) {Return FuncName##__VA(ExtraCall (Fmt));} \
	template<class T1> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1) {T1 v1=CheckVA(V1);Return FuncName##__VA(ExtraCall (Fmt),(v1));} \
	template<class T1,class T2> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2));} \
	template<class T1,class T2,class T3> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3));} \
	template<class T1,class T2,class T3,class T4> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4));} \
	template<class T1,class T2,class T3,class T4,class T5> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15,T16 V16) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);T16 v16=CheckVA(V16);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15),(v16));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15,T16 V16,T17 V17) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);T16 v16=CheckVA(V16);T17 v17=CheckVA(V17);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15),(v16),(v17));} \
	template<class T1,class T2,class T3,class T4,class T5,class T6,class T7,class T8,class T9,class T10,class T11,class T12,class T13,class T14,class T15,class T16,class T17,class T18> \
	StaticFuncRet FuncName(ExtraDecl FmtType Fmt,T1 V1,T2 V2,T3 V3,T4 V4,T5 V5,T6 V6,T7 V7,T8 V8,T9 V9,T10 V10,T11 V11,T12 V12,T13 V13,T14 V14,T15 V15,T16 V16,T17 V17,T18 V18) {T1 v1=CheckVA(V1);T2 v2=CheckVA(V2);T3 v3=CheckVA(V3);T4 v4=CheckVA(V4);T5 v5=CheckVA(V5);T6 v6=CheckVA(V6);T7 v7=CheckVA(V7);T8 v8=CheckVA(V8);T9 v9=CheckVA(V9);T10 v10=CheckVA(V10);T11 v11=CheckVA(V11);T12 v12=CheckVA(V12);T13 v13=CheckVA(V13);T14 v14=CheckVA(V14);T15 v15=CheckVA(V15);T16 v16=CheckVA(V16);T17 v17=CheckVA(V17);T18 v18=CheckVA(V18);Return FuncName##__VA(ExtraCall (Fmt),(v1),(v2),(v3),(v4),(v5),(v6),(v7),(v8),(v9),(v10),(v11),(v12),(v13),(v14),(v15),(v16),(v17),(v18));}

#define VARARG_BODY( FuncRet, FuncName, FmtType, ExtraDecl )		\
	FuncRet FuncName##__VA( ExtraDecl  FmtType Fmt, ... )

#define VARARGS     __cdecl					/* Functions with variable arguments */
#define CDECL	    __cdecl					/* Standard C function */
#define STDCALL		__stdcall				/* Standard calling convention */
//#define FORCEINLINE __forceinline			/* Force code to be inline */
#define FORCENOINLINE __declspec(noinline)	/* Force code to NOT be inline */
#define ZEROARRAY                           /* Zero-length arrays in structs */


/**
 * Computes the base 2 logarithm for an integer value that is greater than 0.
 * The result is rounded down to the nearest integer.
 *
 * @param Value the value to compute the log of
 */
inline DWORD appFloorLog2(DWORD Value) 
{
	DWORD Log2;
	// Use BSR to return the log2 of the integer
	__asm
	{
		bsr eax, Value
		mov Log2, eax
	}
	return Log2;
}

/**
 * Counts the number of leading zeros in the bit representation of the value
 *
 * @param Value the value to determine the number of leading zeros for
 *
 * @return the number of zeros before the first "on" bit
 */
inline DWORD appCountLeadingZeros(DWORD Value)
{
	if (Value == 0) return 32;
	return 31 - appFloorLog2(Value);
}

// Strings.
#define LINE_TERMINATOR TEXT("\r\n")
#define PATH_SEPARATOR TEXT("\\")
#define appIsPathSeparator( Ch )	((Ch) == PATH_SEPARATOR[0])

/*-----------------------------------------------------------------------------
	Character type functions.
-----------------------------------------------------------------------------*/
#define UPPER_LOWER_DIFF	32

/** @name Character functions */
//@{
inline TCHAR appToUpper( TCHAR c )
{
	// compiler generates incorrect code if we try to use TEXT('char') instead of the numeric values directly
	//@hack - ideally, this would be data driven or use some sort of lookup table
	// some special cases
	switch (UNICHAR(c))
	{
	// these special chars are not 32 apart
	case /*TEXT('ÿ')*/ 255: return /*TEXT('?)*/ 159; // diaeresis y
	case /*TEXT('?)*/ 156: return /*TEXT('?)*/ 140; // digraph ae


	// characters within the 192 - 255 range which have no uppercase/lowercase equivalents
	case /*TEXT('?)*/ 240: return c;
	case /*TEXT('?)*/ 208: return c;
	case /*TEXT('?)*/ 223: return c;
	case /*TEXT('?)*/ 247: return c;
	}

	if ( (c >= TEXT('a') && c <= TEXT('z')) || (c > /*TEXT('?)*/ 223 && c < /*TEXT('ÿ')*/ 255) )
	{
		return c - UPPER_LOWER_DIFF;
	}

	// no uppercase equivalent
	return c;
}
inline TCHAR appToLower( TCHAR c )
{
	// compiler generates incorrect code if we try to use TEXT('char') instead of the numeric values directly
	// some special cases
	switch (UNICHAR(c))
	{
	// these are not 32 apart
	case /*TEXT('?)*/ 159: return /*TEXT('ÿ')*/ 255; // diaeresis y
	case /*TEXT('?)*/ 140: return /*TEXT('?)*/ 156; // digraph ae

	// characters within the 192 - 255 range which have no uppercase/lowercase equivalents
	case /*TEXT('?)*/ 240: return c;
	case /*TEXT('?)*/ 208: return c;
	case /*TEXT('?)*/ 223: return c;
	case /*TEXT('?)*/ 247: return c;
	}

	if ( (c >= /*TEXT('?)*/192 && c < /*TEXT('?)*/ 223) || (c >= TEXT('A') && c <= TEXT('Z')) )
	{
		return c + UPPER_LOWER_DIFF;
	}

	// no lowercase equivalent
	return c;
}
inline UBOOL appIsUpper( TCHAR cc )
{
	UNICHAR c(cc);
	// compiler generates incorrect code if we try to use TEXT('char') instead of the numeric values directly
	return (c==/*TEXT('?)*/ 159) || (c==/*TEXT('?)*/ 140)	// these are outside the standard range
		|| (c==/*TEXT('?)*/ 240) || (c==/*TEXT('?)*/ 247)	// these have no lowercase equivalents
		|| (c>=TEXT('A') && c<=TEXT('Z')) || (c >= /*TEXT('?)*/ 192 && c <= /*TEXT('?)*/ 223);
}
inline UBOOL appIsLower( TCHAR cc )
{
	UNICHAR c(cc);
	// compiler generates incorrect code if we try to use TEXT('char') instead of the numeric values directly
	return (c==/*TEXT('?)*/ 156) 															// outside the standard range
		|| (c==/*TEXT('?)*/ 215) || (c==/*TEXT('?)*/ 208) || (c==/*TEXT('?)*/ 223)	// these have no lower-case equivalents
		|| (c>=TEXT('a') && c<=TEXT('z')) || (c >=/*TEXT('?)*/ 224 && c <= /*TEXT('ÿ')*/ 255);
}

inline UBOOL appIsAlpha( TCHAR cc )
{
	UNICHAR c(cc);
	// compiler generates incorrect code if we try to use TEXT('char') instead of the numeric values directly
	return (c>=TEXT('A') && c<=TEXT('Z')) 
		|| (c>=/*TEXT('?)*/ 192 && c<=/*TEXT('ÿ')*/ 255)
		|| (c>=TEXT('a') && c<=TEXT('z')) 
		|| (c==/*TEXT('?)*/ 159) || (c==/*TEXT('?)*/ 140) || (c==/*TEXT('?)*/ 156);	// these are outside the standard range
}
inline UBOOL appIsDigit( TCHAR c )
{
	return c>=TEXT('0') && c<=TEXT('9');
}
inline UBOOL appIsAlnum( TCHAR c )
{
	return appIsAlpha(c) || (c>=TEXT('0') && c<=TEXT('9'));
}
inline UBOOL appIsWhitespace( TCHAR c )
{
	return c == TEXT(' ') || c == TEXT('\t');
}
inline UBOOL appIsLinebreak( TCHAR c )
{
	//@todo - support for language-specific line break characters
	return c == TEXT('\n');
}

/** Returns nonzero if character is a space character. */
inline UBOOL appIsSpace( TCHAR c )
{
    return( iswspace(c) != 0 );
}

inline UBOOL appIsPunct( TCHAR c )
{
	return( iswpunct( c ) != 0 );
}
//@}


/**
* Helper function to write formatted output using an argument list
*
* @param Dest - destination string buffer
* @param DestSize - size of destination buffer
* @param Count - number of characters to write (not including null terminating character)
* @param Fmt - string to print
* @param Args - argument list
* @return number of characters written or -1 if truncated
*/
INT appGetVarArgs( TCHAR* Dest, SIZE_T DestSize, INT Count, const TCHAR*& Fmt, va_list ArgPtr );

/**
* Helper function to write formatted output using an argument list
* ASCII version
*
* @param Dest - destination string buffer
* @param DestSize - size of destination buffer
* @param Count - number of characters to write (not including null terminating character)
* @param Fmt - string to print
* @param Args - argument list
* @return number of characters written or -1 if truncated
*/
INT appGetVarArgsAnsi( ANSICHAR* Dest, SIZE_T DestSize, INT Count, const ANSICHAR*& Fmt, va_list ArgPtr );

#define GET_VARARGS(msg,msgsize,len,lastarg,fmt) { va_list ap; va_start(ap,lastarg);appGetVarArgs(msg,msgsize,len,fmt,ap); }
#define GET_VARARGS_ANSI(msg,msgsize,len,lastarg,fmt) { va_list ap; va_start(ap,lastarg);appGetVarArgsAnsi(msg,msgsize,len,fmt,ap); }
#define GET_VARARGS_RESULT(msg,msgsize,len,lastarg,fmt,result) { va_list ap; va_start(ap,lastarg); result = appGetVarArgs(msg,msgsize,len,fmt,ap); }
#define GET_VARARGS_RESULT_ANSI(msg,msgsize,len,lastarg,fmt,result) { va_list ap; va_start(ap,lastarg); result = appGetVarArgsAnsi(msg,msgsize,len,fmt,ap); }

#define TCHAR_TO_ANSI(str) (char*)FTCHARToANSI((const WCHAR*)str)
#define TCHAR_TO_OEM(str) (char*)FTCHARToOEM((const WCHAR*)str)
#define ANSI_TO_TCHAR(str) (TCHAR*)FANSIToTCHAR((const ANSICHAR*)str)

extern DWORD GCRCTable[];


extern DOUBLE					GDeltaTime;
extern DOUBLE					GCurrentTime;
extern INT						GSRandSeed;
extern UBOOL					GIsAsyncLoading;
extern UBOOL					GIsGame;
extern UBOOL					GIsEditor;
/** Time at which appSeconds() was first initialized (very early on)				*/
extern DOUBLE					GStartTime;
extern DOUBLE					GSecondsPerCycle;
extern DWORD					GUglyHackFlags;
extern FLOAT GPixelCenterOffset;

extern "C" void* __cdecl _alloca(size_t);
#define appAlloca(size) ((size==0) ? 0 : _alloca((size+7)&~7))

#if STATS
#define STAT(x) x
#else
#define STAT(x)
#endif



// Notification hook.
class FNotifyHook
{
public:
	virtual void NotifyDestroy( void* Src ) {}
	/*virtual void NotifyPreChange( void* Src, UProperty* PropertyAboutToChange ) {}
	virtual void NotifyPostChange( void* Src, UProperty* PropertyThatChanged ) {}
	virtual void NotifyPreChange( void* Src, class FEditPropertyChain* PropertyAboutToChange );
	virtual void NotifyPostChange( void* Src, class FEditPropertyChain* PropertyThatChanged );*/
	virtual void NotifyExec( void* Src, const TCHAR* Cmd ) {}
};



#include "EngineCore/Math/Math.h"

#include "ContainerAllocationPolicies.h"
#include "StringFunc.h"
#include "UnTypeTraits.h"
#include "UnTemplate.h"
#include "FMallocWindows.h"

//#include "UnTypeTraits.h"
//#include "Unfile.h"
//#include "UnArc.h"
//#include "UnTemplate.h"
//#include "Array.h"
class UObject 
{
public:
};

class UProperty
{
public:

};

