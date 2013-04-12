/************************************************************************
module	:	U2Log
Author	:	Petru Marginean 
Desc	:   Logging In C++ Dr.Dobb's Article
************************************************************************/
#ifndef U2_LOG_H
#define U2_LOG_H


#if defined (WIN32)
#pragma once
#endif

//#include "U2Util.h"

#include <tchar.h>
#include <sstream>
#include <stdio.h>

#ifdef UNICODE 
typedef std::wstring		U2String;
#else 
typedef std::string			U2String;
#endif

#ifdef UNICODE 
typedef std::basic_ostringstream<wchar_t> OSSStream;
#else
typedef std::basic_ostringstream<char> OSSStream;
#endif

inline U2String NowTime();

//

enum TLogLevel {logERROR, logWARNING, logINFO, logDEBUG, logDEBUG1, logDEBUG2, logDEBUG3, logDEBUG4};

template <typename T>
class  U2Log 
{
public:
	U2Log();
	virtual ~U2Log();
	OSSStream& Get(TLogLevel level = logINFO);
public:
	static TLogLevel& ReportingLevel();
	static U2String ToString(TLogLevel level);
	static TLogLevel FromString(const U2String& level);
protected:
	OSSStream os;
private:
	U2Log(const U2Log&);
	U2Log& operator =(const U2Log&);
};

template <typename T> 
inline
U2Log<T>::U2Log()
{
}



template <typename T> 
inline 
OSSStream& U2Log<T>::Get(TLogLevel level)
{
	os << _T("- ") << NowTime();
	os << _T(" ") << ToString(level) << _T(": ");
	os << U2String(level > logDEBUG ? level - logDEBUG : 0, _T('\t'));
	return os;
}


template <typename T>
inline 
U2Log<T>::~U2Log()
{
	os << std::endl;
	T::Output(os.str());
}

template <typename T>
inline 
TLogLevel& U2Log<T>::ReportingLevel()
{
	static TLogLevel reportingLevel = logDEBUG4;
	return reportingLevel;
}

template <typename T>
inline 
U2String U2Log<T>::ToString(TLogLevel level)
{
	static const TCHAR* const buffer[] = {_T("ERROR"), _T("WARNING"),
		_T("INFO"), _T("DEBUG"), _T("DEBUG1"), _T("DEBUG2"), _T("DEBUG3"), _T("DEBUG4")};
	return buffer[level];
}

template <typename T>
inline 
TLogLevel U2Log<T>::FromString(const U2String& level)
{
	if (level == _T("DEBUG4"))
		return logDEBUG4;
	if (level == _T("DEBUG3"))
		return logDEBUG3;
	if (level == _T("DEBUG2"))
		return logDEBUG2;
	if (level == _T("DEBUG1"))
		return logDEBUG1;
	if (level == _T("DEBUG"))
		return logDEBUG;
	if (level == _T("INFO"))
		return logINFO;
	if (level == _T("WARNING"))
		return logWARNING;
	if (level == _T("ERROR"))
		return logERROR;
	U2Log<T>().Get(logWARNING) << _T("Unknown logging level '") 
		<< level << _T("'. Using INFO level as default.");
	return logINFO;
}

class Output2FILE
{
public:
	static FILE*& Stream();
	static void Output(const U2String& msg);
};

inline FILE*& Output2FILE::Stream()
{
	static FILE* pStream = stderr;
	return pStream;
}

//-------------------------------------------------------------------------------------------------
inline void Output2FILE::Output(const U2String& msg)
{   
	FILE* pStream = Stream();
	if (!pStream)
		return;
	_ftprintf(pStream, _T("%s"), msg.c_str());
	int res = fflush(pStream);	
	// Why?
	//FDebug(strerror( errno ));
}

//-------------------------------------------------------------------------------------------------
class FILELog : public U2Log<Output2FILE> 
{
public:
	static bool Init();		
	static FILE *ms_pFile;
};
//typedef Log<Output2FILE> FILELog;


//-------------------------------------------------------------------------------------------------
inline bool FILELog::Init()
{
	bool bSuccess = false;	

	ms_pFile = _tfopen(_T("U2Main.log"), _T("w+"));	
	Output2FILE::Stream() = ms_pFile;;
	bSuccess = true;

	return bSuccess;
}

#ifndef FILELOG_MAX_LEVEL
#define FILELOG_MAX_LEVEL logDEBUG4
#endif

#define FILE_LOG(level) \
	if (level > FILELOG_MAX_LEVEL) ;\
	else if (level > FILELog::ReportingLevel() || !Output2FILE::Stream()) ; \
	else FILELog().Get(level) 


#define LOG_ERR					FILELog().Get(logERROR)
#define LOG_DEBUG				FILELog().Get(logDEBUG)
#define LOG_DEBUG1				FILELog().Get(logDEBUG1)
#define LOG_DEBUG2				FILELog().Get(logDEBUG2)
#define LOG_DEBUG3				FILELog().Get(logDEBUG3)
#define LOG_DEBUG4				FILELog().Get(logDEBUG4)
#define LOG_INFO				FILELog().Get(logINFO)
#define LOG_WARN				FILELog().Get(logWARNING)

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)


inline U2String NowTime()
{
	const int MAX_LEN = 200;
	TCHAR buffer[MAX_LEN];
	if (GetTimeFormat(LOCALE_USER_DEFAULT, 0, 0, 
		_T("HH':'mm':'ss"), buffer, MAX_LEN) == 0)
		return _T("Error in NowTime()");

	TCHAR result[100] = {0};
	static DWORD first = GetTickCount();
	_stprintf_s(result, 100, _T("%s.%03ld"), buffer, (long)(GetTickCount() - first) % 1000); 
	return result;
}

#else

#include <sys/time.h>

inline U2String NowTime()
{
	char buffer[11];
	time_t t;
	time(&t);
	tm r = {0};
	strftime(buffer, sizeof(buffer), "%X", localtime_r(&t, &r));
	struct timeval tv;
	gettimeofday(&tv, 0);
	char result[100] = {0};
	std::sprintf(result, "%s.%03ld", buffer, (long)tv.tv_usec / 1000); 
	return result;
}

#endif //WIN32

#endif //__LOG_H__
