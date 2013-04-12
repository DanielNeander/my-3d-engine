#include "Core.h"
#include "UnArc.h"
#include "UnStringConv.h"
#include "UnString.h"


//
// Convert an integer to a string.
//
// Faster Itoa that also appends to a string
void appItoaAppend( INT InNum,FString &NumberString )
{
	SQWORD	Num					= InNum; // This avoids having to deal with negating -MAXINT-1
	TCHAR*	NumberChar[11]		= { TEXT("0"), TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"), TEXT("5"), TEXT("6"), TEXT("7"), TEXT("8"), TEXT("9"), TEXT("-") };
	UBOOL	bIsNumberNegative	= FALSE;
	TCHAR	TempNum[16];		// 16 is big enough
	INT		TempAt				= 16; // fill the temp string from the top down.

	// Correctly handle negative numbers and convert to positive integer.
	if( Num < 0 )
	{
		bIsNumberNegative = TRUE;
		Num = -Num;
	}

	TempNum[--TempAt] = 0; // NULL terminator

	// Convert to string assuming base ten and a positive integer.
	do 
	{
		TempNum[--TempAt] = *NumberChar[Num % 10];
		Num /= 10;
	} while( Num );

	// Append sign as we're going to reverse string afterwards.
	if( bIsNumberNegative )
	{
		TempNum[--TempAt] = *NumberChar[10];
	}

	NumberString += TempNum + TempAt;
}

FString appItoa( INT InNum )
{
	FString NumberString;
	appItoaAppend(InNum,NumberString );
	return NumberString;
}

//
// Find string in string, case insensitive, requires non-alphanumeric lead-in.
//
const TCHAR* appStrfind( const TCHAR* Str, const TCHAR* Find )
{
	if( Find == NULL || Str == NULL )
	{
		return NULL;
	}
	UBOOL Alnum  = 0;
	TCHAR f      = (*Find<'a' || *Find>'z') ? (*Find) : (*Find+'A'-'a');
	INT   Length = appStrlen(Find++)-1;
	TCHAR c      = *Str++;
	while( c )
	{
		if( c>='a' && c<='z' )
		{
			c += 'A'-'a';
		}
		if( !Alnum && c==f && !appStrnicmp(Str,Find,Length) )
		{
			return Str-1;
		}
		Alnum = (c>='A' && c<='Z') || (c>='0' && c<='9');
		c = *Str++;
	}
	return NULL;
}

/** 
 * Finds string in string, case insensitive 
 * @param Str The string to look through
 * @param Find The string to find inside Str
 * @return Position in Str if Find was found, otherwise, NULL
 */
const TCHAR* appStristr(const TCHAR* Str, const TCHAR* Find)
{
	// both strings must be valid
	if( Find == NULL || Str == NULL )
	{
		return NULL;
	}
	// get upper-case first letter of the find string (to reduce the number of full strnicmps)
	TCHAR FindInitial = appToUpper(*Find);
	// get length of find string, and increment past first letter
	INT   Length = appStrlen(Find++) - 1;
	// get the first letter of the search string, and increment past it
	TCHAR StrChar = *Str++;
	// while we aren't at end of string...
	while (StrChar)
	{
		// make sure it's upper-case
		StrChar = appToUpper(StrChar);
		// if it matches the first letter of the find string, do a case-insensitive string compare for the length of the find string
		if (StrChar == FindInitial && !appStrnicmp(Str, Find, Length))
		{
			// if we found the string, then return a pointer to the beginning of it in the search string
			return Str-1;
		}
		// go to next letter
		StrChar = *Str++;
	}

	// if nothing was found, return NULL
	return NULL;
}
TCHAR* appStristr(TCHAR* Str, const TCHAR* Find)
{
	return (TCHAR*)appStristr((const TCHAR*)Str, Find);
}

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
const TCHAR* appSpc( INT NumCharacters, BYTE Char )
{
	// static string storage
	static TCHAR StaticString[256];
	// previous number of chars, used to avoid duplicate work if it didn't change
	static INT OldNum=-1;
	// previous character filling the string, used to avoid duplicate work if it didn't change
	static BYTE OldChar=255;
	
	assert(NumCharacters < 256);
	// if the character changed, we need to start this string over from scratch
	if (OldChar != Char)
	{
		OldNum = -1;
		OldChar = Char;
	}

	// if the number changed, fill in the array
	if( NumCharacters != OldNum )
	{
		// fill out the array with the characer
		for( OldNum=0; OldNum<NumCharacters; OldNum++ )
		{
			StaticString[OldNum] = Char;
		}
		// null terminate it
		StaticString[NumCharacters] = 0;
	}

	// return the one string
	return StaticString;
}

// 
// Trim spaces from an ascii string by zeroing them.
//
void appTrimSpaces( ANSICHAR* String )
{		
	// Find 0 terminator.
	INT t=0;
	while( (String[t]!=0 ) && (t< 1024) ) t++;
	if (t>0) t--;
	// Zero trailing spaces.
	while( (String[t]==32) && (t>0) )
	{
		String[t]=0;
		t--;
	}
}

/*-----------------------------------------------------------------------------
	Memory functions.
-----------------------------------------------------------------------------*/

//
// Memory functions.
//
void appMemswap( void* Ptr1, void* Ptr2, DWORD Size )
{
	void* Temp = appAlloca(Size);
	appMemcpy( Temp, Ptr1, Size );
	appMemcpy( Ptr1, Ptr2, Size );
	appMemcpy( Ptr2, Temp, Size );
}

/*-----------------------------------------------------------------------------
	CRC functions.
-----------------------------------------------------------------------------*/

//
// CRC32 computer based on CRC32_POLY.
//
DWORD appMemCrc( const void* InData, INT Length, DWORD CRC )
{
	BYTE* Data = (BYTE*)InData;
	CRC = ~CRC;
	for( INT i=0; i<Length; i++ )
		CRC = (CRC << 8) ^ GCRCTable[(CRC >> 24) ^ Data[i]];
	return ~CRC;
}

//
// String CRC.
//
DWORD appStrCrc( const TCHAR* Data )
{
	INT Length = appStrlen( Data );
	DWORD CRC = 0xFFFFFFFF;
	for( INT i=0; i<Length; i++ )
	{
		TCHAR C   = Data[i];
		INT   CL  = (C&255);
		CRC       = (CRC << 8) ^ GCRCTable[(CRC >> 24) ^ CL];;
		INT   CH  = (C>>8)&255;
		CRC       = (CRC << 8) ^ GCRCTable[(CRC >> 24) ^ CH];;
	}
	return ~CRC;
}

//
// String CRC, case insensitive.
//
DWORD appStrCrcCaps( const TCHAR* Data )
{
	INT Length = appStrlen( Data );
	DWORD CRC = 0xFFFFFFFF;
	for( INT i=0; i<Length; i++ )
	{
		TCHAR C   = appToUpper(Data[i]);
		INT   CL  = (C&255);
		CRC       = (CRC << 8) ^ GCRCTable[(CRC >> 24) ^ CL];
		INT   CH  = (C>>8)&255;
		CRC       = (CRC << 8) ^ GCRCTable[(CRC >> 24) ^ CH];
	}
	return ~CRC;
}

// Ansi String CRC.
//
DWORD appAnsiStrCrc( const char* Data )
{
	INT Length = strlen( Data );
	DWORD CRC = 0xFFFFFFFF;
	for( INT i=0; i<Length; i++ )
	{
		char C   = Data[i];
		INT   CL  = (C&255);
		CRC       = (CRC << 8) ^ GCRCTable[(CRC >> 24) ^ CL];;
		INT   CH  = (C>>8)&255;
		CRC       = (CRC << 8) ^ GCRCTable[(CRC >> 24) ^ CH];;
	}
	return ~CRC;
}

//
// Ansi String CRC, case insensitive.
//
DWORD appAnsiStrCrcCaps( const char* Data )
{
	INT Length = strlen( Data );
	DWORD CRC = 0xFFFFFFFF;
	for( INT i=0; i<Length; i++ )
	{
		char C   = toupper(Data[i]);
		INT   CL  = (C&255);
		CRC       = (CRC << 8) ^ GCRCTable[(CRC >> 24) ^ CL];
		INT   CH  = (C>>8)&255;
		CRC       = (CRC << 8) ^ GCRCTable[(CRC >> 24) ^ CH];
	}
	return ~CRC;
}
