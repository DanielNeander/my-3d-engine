#pragma once


#if USE_SECURE_CRT
#define appSSCANF	_stscanf_s
#else
#define appSSCANF	_stscanf
#endif

typedef int QSORT_RETURN;
typedef QSORT_RETURN(CDECL* QSORT_COMPARE)( const void* A, const void* B );
/** Quick sort. */
void appQsort( void* Base, INT Num, INT Width, QSORT_COMPARE Compare );

/** Case insensitive string hash function. */
inline DWORD appStrihash( const UNICHAR* Data )
{
	DWORD Hash=0;
	while( *Data )
	{
		TCHAR Ch = appToUpper(*Data++);
		BYTE  B  = Ch;
		Hash     = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(Hash ^ B) & 0x000000FF];
		B        = Ch>>8;
		Hash     = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(Hash ^ B) & 0x000000FF];
	}
	return Hash;
}

/** Case insensitive string hash function. */
inline DWORD appStrihash( const ANSICHAR* Data )
{
	DWORD Hash=0;
	while( *Data )
	{
		TCHAR Ch = appToUpper(*Data++);
		BYTE  B  = Ch;
		Hash     = ((Hash >> 8) & 0x00FFFFFF) ^ GCRCTable[(Hash ^ B) & 0x000000FF];
	}
	return Hash;
}

inline ANSICHAR* appStrcpy( ANSICHAR* Dest, SIZE_T DestCount, const ANSICHAR* Src )
{
#if USE_SECURE_CRT
	strcpy_s( Dest, DestCount, Src );
	return Dest;
#else
	return (ANSICHAR*)strcpy( Dest, Src );
#endif
}

/**
 * strcpy wrapper
 *
 * @param Dest - destination string to copy to
 * @param Destcount - size of Dest in characters
 * @param Src - source string
 * @return destination string
 */
inline UNICHAR* appStrcpy( UNICHAR* Dest, SIZE_T DestCount, const UNICHAR* Src )
{
#if USE_SECURE_CRT
	_tcscpy_s( Dest, DestCount, Src );
	return Dest;
#else
	return (UNICHAR*)wcscpy( Dest, Src );
#endif
}

/**
* strcpy wrapper
* (templated version to automatically handle static destination array case)
*
* @param Dest - destination string to copy to
* @param Src - source string
* @return destination string
*/
template<SIZE_T DestCount>
inline UNICHAR* appStrcpy( UNICHAR (&Dest)[DestCount], const UNICHAR* Src ) 
{
	return appStrcpy( Dest, DestCount, Src );
}

/**
* strcpy wrapper
* (templated version to automatically handle static destination array case)
*
* @param Dest - destination string to copy to
* @param Src - source string
* @return destination string
*/
template<SIZE_T DestCount>
inline ANSICHAR* appStrcpy( ANSICHAR (&Dest)[DestCount], const ANSICHAR* Src ) 
{
	return appStrcpy( Dest, DestCount, Src );
}

/**
* strcat wrapper
*
* @param Dest - destination string to copy to
* @param Destcount - size of Dest in characters
* @param Src - source string
* @return destination string
*/
inline TCHAR* appStrcat( TCHAR* Dest, SIZE_T DestCount, const TCHAR* Src ) 
{ 
#if USE_SECURE_CRT
	_tcscat_s( Dest, DestCount, Src );
	return Dest;
#else
	return (TCHAR*)_tcscat( Dest, Src );
#endif
}

/**
* strcat wrapper
* (templated version to automatically handle static destination array case)
*
* @param Dest - destination string to copy to
* @param Src - source string
* @return destination string
*/
template<SIZE_T DestCount>
inline TCHAR* appStrcat( TCHAR (&Dest)[DestCount], const TCHAR* Src ) 
{ 
	return appStrcat( Dest, DestCount, Src );
}

/**
* strupr wrapper
*
* @param Dest - destination string to convert
* @param Destcount - size of Dest in characters
* @return destination string
*/
inline TCHAR* appStrupr( TCHAR* Dest, SIZE_T DestCount ) 
{
#if USE_SECURE_CRT
	_tcsupr_s( Dest, DestCount );
	return Dest;
#else
	return (TCHAR*)_tcsupr( Dest );
#endif
}

/**
* strupr wrapper
* (templated version to automatically handle static destination array case)
*
* @param Dest - destination string to convert
* @return destination string
*/
template<SIZE_T DestCount>
inline TCHAR* appStrupr( TCHAR (&Dest)[DestCount] ) 
{
	return appStrupr( Dest, DestCount );
}

// ANSI character versions of string manipulation functions

/**
* strcpy wrapper (ANSI version)
*
* @param Dest - destination string to copy to
* @param Destcount - size of Dest in characters
* @param Src - source string
* @return destination string
*/
inline ANSICHAR* appStrcpyANSI( ANSICHAR* Dest, SIZE_T DestCount, const ANSICHAR* Src ) 
{ 
#if USE_SECURE_CRT
	strcpy_s( Dest, DestCount, Src );
	return Dest;
#else
	return (ANSICHAR*)strcpy( Dest, Src );
#endif
}

/**
* strcpy wrapper (ANSI version)
* (templated version to automatically handle static destination array case)
*
* @param Dest - destination string to copy to
* @param Src - source string
* @return destination string
*/
template<SIZE_T DestCount>
inline ANSICHAR* appStrcpyANSI( ANSICHAR (&Dest)[DestCount], const ANSICHAR* Src ) 
{ 
	return appStrcpyANSI( Dest, DestCount, Src );
}

/**
* strcat wrapper (ANSI version)
*
* @param Dest - destination string to copy to
* @param Destcount - size of Dest in characters
* @param Src - source string
* @return destination string
*/
inline ANSICHAR* appStrcatANSI( ANSICHAR* Dest, SIZE_T DestCount, const ANSICHAR* Src ) 
{ 
#if USE_SECURE_CRT
	strcat_s( Dest, DestCount, Src );
	return Dest;
#else
	return (ANSICHAR*)strcat( Dest, Src );
#endif
}

/**
* strcat wrapper (ANSI version)
*
* @param Dest - destination string to copy to
* @param Destcount - size of Dest in characters
* @param Src - source string
* @return destination string
*/
template<SIZE_T DestCount>
inline ANSICHAR* appStrcatANSI( ANSICHAR (&Dest)[DestCount], const ANSICHAR* Src ) 
{ 
	return appStrcatANSI( Dest, DestCount, Src );
}

inline INT appStrlen( const ANSICHAR* String ) { return strlen( String ); }
inline INT appStrlen( const UNICHAR* String ) { return wcslen( String ); }
inline TCHAR* appStrstr( const TCHAR* String, const TCHAR* Find ) { return (TCHAR*)_tcsstr( String, Find ); }
inline TCHAR* appStrchr( const TCHAR* String, INT c ) { return (TCHAR*)_tcschr( String, c ); }
inline TCHAR* appStrrchr( const TCHAR* String, INT c ) { return (TCHAR*)_tcsrchr( String, c ); }
inline INT appStrcmp( const TCHAR* String1, const TCHAR* String2 ) { return _tcscmp( String1, String2 ); }
inline INT appStricmp( const ANSICHAR* String1, const ANSICHAR* String2 )  { return _stricmp( String1, String2 ); }
inline INT appStricmp( const UNICHAR* String1, const UNICHAR* String2 )  { return wcsicmp( String1, String2 ); }
inline INT appStrncmp( const TCHAR* String1, const TCHAR* String2, INT Count ) { return _tcsncmp( String1, String2, Count ); }
inline INT appAtoi( const TCHAR* String ) { return _tstoi( String ); }
inline FLOAT appAtof( const TCHAR* String ) { return _tstof( String ); }
inline DOUBLE appAtod( const TCHAR* String ) { return _tcstod( String, NULL ); }
inline INT appStrtoi( const TCHAR* Start, TCHAR** End, INT Base ) { return _tcstoul( Start, End, Base ); }
inline INT appStrnicmp( const TCHAR* A, const TCHAR* B, INT Count ) { return _tcsnicmp( A, B, Count ); }


/**
 * Returns a static string that is full of a variable number of space (or other)
 * characters that can be used to space things out, or calculate string widths
 * Since it is static, only one return value from a call is valid at a time.
 *
 * @param NumCharacters Number of characters to but into the string, max of 255
 * @param Char Optional character to put into the string (defaults to space)
 * 
 * @return The string of NumCharacters characters.
 */
const TCHAR* appSpc( INT NumCharacters, BYTE Char=' ' );

/** 
* Copy a string with length checking. Behavior differs from strncpy in that last character is zeroed. 
*
* @param Dest - destination buffer to copy to
* @param Src - source buffer to copy from
* @param MaxLen - max length of the buffer (including null-terminator)
* @return pointer to resulting string buffer
*/
TCHAR* appStrncpy( TCHAR* Dest, const TCHAR* Src, INT MaxLen );

/** 
* Concatenate a string with length checking.
*
* @param Dest - destination buffer to append to
* @param Src - source buffer to copy from
* @param MaxLen - max length of the buffer
* @return pointer to resulting string buffer
*/
TCHAR* appStrncat( TCHAR* Dest, const TCHAR* Src, INT MaxLen );

/** 
* Copy a string with length checking. Behavior differs from strncpy in that last character is zeroed. 
* (ANSICHAR version) 
*
* @param Dest - destination char buffer to copy to
* @param Src - source char buffer to copy from
* @param MaxLen - max length of the buffer (including null-terminator)
* @return pointer to resulting string buffer
*/
ANSICHAR* appStrncpyANSI( ANSICHAR* Dest, const ANSICHAR* Src, INT MaxLen );

/** Finds string in string, case insensitive, requires non-alphanumeric lead-in. */
const TCHAR* appStrfind(const TCHAR* Str, const TCHAR* Find);

/** 
 * Finds string in string, case insensitive 
 * @param Str The string to look through
 * @param Find The string to find inside Str
 * @return Position in Str if Find was found, otherwise, NULL
 */
const TCHAR* appStristr(const TCHAR* Str, const TCHAR* Find);
TCHAR* appStristr(TCHAR* Str, const TCHAR* Find);

/** String CRC. */
DWORD appStrCrc( const TCHAR* Data );
/** String CRC, case insensitive. */
DWORD appStrCrcCaps( const TCHAR* Data );
/** Ansi String CRC. */
DWORD appAnsiStrCrc( const char* Data );
/** Ansi String CRC, case insensitive. */
DWORD appAnsiStrCrcCaps( const char* Data );

class FString;

/** Converts an integer to a string. */
FString appItoa( INT Num );
void appItoaAppend( INT InNum,FString &NumberString );


/** 
* Standard string formatted print. 
* @warning: make sure code using appSprintf allocates enough (>= MAX_SPRINTF) memory for the destination buffer
*/
#define MAX_SPRINTF 1024
VARARG_DECL( INT, static INT, return, appSprintf, VARARG_NONE, const TCHAR*, VARARG_EXTRA(TCHAR* Dest), VARARG_EXTRA(Dest) );
/**
* Standard string formatted print (ANSI version).
* @warning: make sure code using appSprintf allocates enough (>= MAX_SPRINTF) memory for the destination buffer
*/
VARARG_DECL( INT, static INT, return, appSprintfANSI, VARARG_NONE, const ANSICHAR*, VARARG_EXTRA(ANSICHAR* Dest), VARARG_EXTRA(Dest) );

//
// Returns whether the string is pure ANSI.
//
inline UBOOL appIsPureAnsi( const TCHAR* Str )
{
	for( ; *Str; Str++ )
	{
		if( *Str>0xff )
		{
			return 0;
		}
	}
	return 1;
}


//
// Formats the text for appOutputDebugString.
//
inline void VARARGS appOutputDebugStringf( const TCHAR *Format, ... )
{
	TCHAR TempStr[4096];
	GET_VARARGS( TempStr, ARRAY_COUNT(TempStr), ARRAY_COUNT(TempStr)-1, Format, Format );
//	appOutputDebugString( TempStr );
}
