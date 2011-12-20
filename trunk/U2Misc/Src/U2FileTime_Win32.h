/************************************************************************
module	:	U2WinFileTime
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_WINFILETIME_H
#define U2_WINFILETIME_H
//------------------------------------------------------------------------------
/**
@class nFileTimeWin32
@ingroup File

Implement Win32 specific nFileTime class. Please use the platform neutral
nFileTime class for indirect access to _nFileTimeWin32.

(C) 2003 RadonLabs GmbH
*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#ifndef WIN32
#error _T("U2WinFileTime : trying to compile Win32 class on hostile platform")
#endif

//------------------------------------------------------------------------------
class U2WinFileTime : public U2MemObj
{
public:
	/// constructor
	U2WinFileTime();
	/// operator ==
	friend bool operator ==(const U2WinFileTime& a, const U2WinFileTime& b);
	/// operator !=
	friend bool operator !=(const U2WinFileTime& a, const U2WinFileTime& b);
	/// operator >
	friend bool operator >(const U2WinFileTime& a, const U2WinFileTime& b);
	/// operator <
	friend bool operator <(const U2WinFileTime& a, const U2WinFileTime& b);

private:
	friend class U2WinFile;
	FILETIME time;
};

//------------------------------------------------------------------------------
/**
*/
inline
U2WinFileTime::U2WinFileTime()
{
	time.dwLowDateTime = 0;
	time.dwHighDateTime = 0;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
operator ==(const U2WinFileTime& a, const U2WinFileTime& b)
{
	return (0 == CompareFileTime(&(a.time), &(b.time)));
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
operator !=(const U2WinFileTime& a, const U2WinFileTime& b)
{
	return (0 != CompareFileTime(&(a.time), &(b.time)));
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
operator >(const U2WinFileTime& a, const U2WinFileTime& b)
{
	return (1 == CompareFileTime(&(a.time), &(b.time)));
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
operator <(const U2WinFileTime& a, const U2WinFileTime& b)
{
	return (-1 == CompareFileTime(&(a.time), &(b.time)));
}

//------------------------------------------------------------------------------
#endif