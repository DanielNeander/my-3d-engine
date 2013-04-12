#include "Core.h"
#include "strsafe.h"

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
INT appGetVarArgs( TCHAR* Dest, SIZE_T DestSize, INT Count, const TCHAR*& Fmt, va_list ArgPtr )
{
#if USE_SECURE_CRT
	INT Result = _vsntprintf_s(Dest,DestSize,Count/*_TRUNCATE*/,Fmt,ArgPtr);
#else
	INT Result = _vsntprintf(Dest,Count,Fmt,ArgPtr);
#endif
	va_end( ArgPtr );
	return Result;
}

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
INT appGetVarArgsAnsi( ANSICHAR* Dest, SIZE_T DestSize, INT Count, const ANSICHAR*& Fmt, va_list ArgPtr )
{
#if USE_SECURE_CRT
	INT Result = _vsnprintf_s(Dest,DestSize,Count/*_TRUNCATE*/,Fmt,ArgPtr);
#else
	INT Result = _vsnprintf(Dest,Count,Fmt,ArgPtr);
#endif
	va_end( ArgPtr );
	return Result;
}

void appDebugMessagef( const TCHAR* Fmt, ... )
{
	TCHAR TempStr[4096]=TEXT("");
	GET_VARARGS( TempStr, ARRAY_COUNT(TempStr), ARRAY_COUNT(TempStr)-1, Fmt, Fmt );
	//if( GIsUnattended == TRUE )
	//{
	//	//debugf(TempStr);
	//}
	//else
	{
		MessageBox(NULL, TempStr, TEXT("appDebugMessagef"),MB_OK|MB_SYSTEMMODAL);
	}
}

FMalloc*				GMalloc							= NULL;						/* Memory allocator */
DOUBLE					GSecondsPerCycle				= 0.0;						/* Seconds per CPU cycle for this PC */
DWORD					GUglyHackFlags					= 0;						/* Flags for passing around globally hacked stuff */
/** Time at which appSeconds() was first initialized (very early on)										*/
DOUBLE					GStartTime;
FLOAT GPixelCenterOffset = 0.5f;
