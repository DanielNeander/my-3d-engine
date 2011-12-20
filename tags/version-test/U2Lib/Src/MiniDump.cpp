#define WIN32_LEAN_AND_MEAN

#include <Src/U2LibPCH.h>

#include <windows.h>
#include <dbghelp.h>

#include <stdio.h>



////////////////////////////////////////////////////////////////////////////////
/// \file MiniDump.cpp
/// \author excel96
/// \date 2003.11.18
///
/// 덤프 파일 옵션... from MSDN
///
/// - MiniDumpNormal
///     Include just the information necessary to capture stack traces for 
///     all existing threads in a process. 
///
/// - MiniDumpWithDataSegs 
///     Include the data sections from all loaded modules. This results in 
///     the inclusion of global variables, which can make the minidump file 
///     significantly larger. 
///
/// - MiniDumpWithFullMemory 
///     Include all accessible memory in the process. The raw memory data is 
///     included at the end, so that the Initial structures can be mapped 
///     directly without the raw memory information. This option can result 
///     in a very large file. 
///
/// - MiniDumpWithHandleData
///     Include high-level information about the operating system handles 
///     that are active when the minidump is made. 
///     \n Windows Me/98/95: This value is not supported.
///
/// - MiniDumpFilterMemory
///     Stack and backing store memory written to the minidump file should be 
///     filtered to remove all but the pointer values necessary to reconstruct 
///     a stack trace. Typically, this removes any private information. 
///
/// - MiniDumpScanMemory 
///     Stack and backing store memory should be scanned for pointer 
///     references to modules in the module list. If a module is referenced by 
///     stack or backing store memory, the ModuleWriteFlags member of the 
///     MINIDUMP_CALLBACK_OUTPUT structure is set to ModuleReferencedByMemory. 
///
/// - MiniDumpWithUnloadedModules
///     Include information from the list of modules that were recently 
///     unloaded, if this information is maintained by the operating system.
///     \n DbgHelp 5.1 and earlier: This value is not supported.
///
/// - MiniDumpWithIndirectlyReferencedMemory 
///     Include pages with data referenced by locals or other stack memory. 
///     This option can increase the size of the minidump file significantly.
///     \n DbgHelp 5.1 and earlier:  This value is not supported.
///
/// - MiniDumpFilterModulePaths
///     Filter module paths for information such as user names or important 
///     directories. This option may prevent the system from locating the 
///     image file and should be used only in special situations.
///     \n DbgHelp 5.1 and earlier:  This value is not supported.
///
/// - MiniDumpWithProcessThreadData
///     Include complete per-process and per-thread information from the 
///     operating system.
///     \n DbgHelp 5.1 and earlier:  This value is not supported.
///
/// - MiniDumpWithPrivateReadWriteMemory 
///     Scan the virtual address space for other types of memory to be 
///     included.
///     \n DbgHelp 5.1 and earlier:  This value is not supported.
////////////////////////////////////////////////////////////////////////////////

#include "MiniDump.h"

#include <dbghelp.h>
#include <stdio.h>
#include <tchar.h>

#define HAS_STACK_WALKER 1

#ifdef HAS_STACK_WALKER
#include "StackWalker.h"
#endif

#pragma comment(lib, _T("dbghelp.lib"))


#ifndef Assert
#include <assert.h>
#define Assert assert
#define LogToFile (void)(0);
#endif

cMiniDump::DumpLevel cMiniDump::s_DumpLevel          = cMiniDump::DUMP_LEVEL_LIGHT;
bool                 cMiniDump::s_AddTimeStamp       = true;
TCHAR                cMiniDump::s_AppName[MAX_PATH] = {0,};
TCHAR                cMiniDump::s_CallStack[8192]    = {0,};
TCHAR                cMiniDump::s_Modules[8192]      = {0,};
LPCTSTR              cMiniDump::s_DialogTemplate     = NULL;
DLGPROC              cMiniDump::s_DialogProc         = NULL;

namespace
{
	/// \brief 예외의 원인에 대한 문자열을 반환한다.
	LPCTSTR GetFaultReason(PEXCEPTION_POINTERS exPtrs);

	/// \brief 사용자 정보를 반환한다.
	LPCTSTR GetUserInfo();

	/// \brief 윈도우즈 버전을 반환한다.
	LPCTSTR GetOSInfo();

	/// \brief CPU 정보를 반환한다.
	LPCTSTR GetCpuInfo();

	/// \brief 메모리 정보를 반환한다.
	LPCTSTR GetMemoryInfo();

	/// \brief 윈도우즈 버전을 알아낸다.
	bool GetWinVersion(LPTSTR pszVersion, int *nVersion, LPTSTR pszMajorMinorBuild);

	/// \brief strrchr TCHAR 버전
	TCHAR* lstrrchr(TCHAR* str, TCHAR ch);
}

////////////////////////////////////////////////////////////////////////////////
/// \brief 미니 덤프 기능을 초기화한다.
/// \param dumpLevel 덤프 레벨
/// \param addTimeStamp 덤프 파일 이름에다가 덤프 파일이 생성된 날짜를
/// 집어넣는가의 여부.
/// \param dialogTemplate 대화창 템플릿
/// \param dialogProc 대화창 프로시져
////////////////////////////////////////////////////////////////////////////////
void cMiniDump::Install(
						DumpLevel dumpLevel, bool addTimeStamp, LPCTSTR dialogTemplate, DLGPROC dialogProc)
{
	Assert(s_AppName[0] == 0);
	Assert(dumpLevel >= DUMP_LEVEL_LIGHT);
	Assert(dumpLevel <= DUMP_LEVEL_HEAVY);

	s_DumpLevel      = dumpLevel;
	s_AddTimeStamp   = addTimeStamp;
	s_DialogTemplate = dialogTemplate;
	s_DialogProc     = dialogProc;

	// 모듈 경로를 알아낸다.
	// C:\somewhere\something.exe
	TCHAR szFileName[MAX_PATH];
	::GetModuleFileName(NULL, szFileName, MAX_PATH);

	// 확장자를 제거한 모듈 경로를 준비해둔다.
	// C:\somewhere\something.exe -> C:\somewhere\something
	TCHAR* dot = lstrrchr(szFileName, '.');
	::lstrcpyn(s_AppName, szFileName, (int)(dot - szFileName + 1));

	// 예외 처리 핸들러를 설정한다.
	::SetUnhandledExceptionFilter(WriteDump);
}

////////////////////////////////////////////////////////////////////////////////
/// \brief 예외에 대한 정보를 받아서, 미니 덤프 파일을 생성한다. 
/// 
/// SetUnhandledExceptionFilter() API에 의해서 설정되고, 프로세스 내부에서 
/// Unhandled Exception이 발생될 경우, 호출되게 된다. 단 디버거가 붙어있는 
/// 경우, Unhandled Exception Filter는 호출되지 않는다. 이 말은 이 함수 
/// 내부를 디버깅할 수는 없다는 말이다. 이 함수 내부를 디버깅하기 위해서는 
/// 메시지 박스 또는 파일을 이용해야한다.
/// 
/// \param exPtrs 예외 정보
/// \return LONG 이 함수를 실행하고 난 다음, 취할 행동값. 자세한 것은 SEH
/// 문서를 참고하도록.
////////////////////////////////////////////////////////////////////////////////
LONG WINAPI cMiniDump::WriteDump(PEXCEPTION_POINTERS exPtrs)
{
	// based on dbghelp.h
	typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(
		HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
		CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
		CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
		CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
		);

#ifdef UNICODE
	fwprintf_s(stderr, 
		L"=============================================\n"
		L"unhandled excetpion triggerd! writing dump...\n"
		L"=============================================\n"
		);
#else
	fprintf_s(stderr, 
		"=============================================\n"
		"unhandled excetpion triggerd! writing dump...\n"
		"=============================================\n"
		);
#endif

#ifdef HAS_STACK_WALKER
	// 먼저 콜스택과 모듈들을 문자열로 만든다.
	StackWalker msw;
	msw.ShowCallstack(GetCurrentThread(), exPtrs->ContextRecord);
	//::lstrcpyn(s_CallStack, msw., _ARRAYSIZE(s_CallStack)-1);
	//::lstrcpyn(s_Modules, msw.(), _ARRAYSIZE(s_Modules)-1);
#endif

	// 대화창이 설정되어 있다면 보여준다.
	if (s_DialogTemplate != NULL && s_DialogProc != NULL)
	{
		if (DialogBox(NULL, s_DialogTemplate, HWND_DESKTOP, s_DialogProc) != IDOK)
			return EXCEPTION_EXECUTE_HANDLER;
	}

	HMODULE hDLL = NULL;
	TCHAR szDbgHelpPath[MAX_PATH] = {0, };
	TCHAR szDumpPath[MAX_PATH * 2] = {0,};
	TCHAR szLogPath[MAX_PATH * 2] = {0,};

	// 덤프 파일 이름 += 시간 문자열
	::lstrcat(szDumpPath, s_AppName);
	::lstrcat(szLogPath, s_AppName);
	if (s_AddTimeStamp)
	{
		SYSTEMTIME t;
		::GetLocalTime(&t);

		TCHAR szTail[MAX_PATH];
#ifdef UNICODE
		swprintf(szTail, _ARRAYSIZE(szTail)-1, 
			L" %04d-%02d-%02d %02d-%02d-%02d",
			t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
#else
		_snprintf_s(szTail, _ARRAYSIZE(szTail)-1, _TRUNCATE,
			" %04d-%02d-%02d %02d-%02d-%02d",
			t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
#endif

		::lstrcat(szDumpPath, szTail);
		::lstrcat(szLogPath, szTail);
	}
	::lstrcat(szDumpPath, _T(".dmp"));
	::lstrcat(szLogPath, _T(".log"));

	// 먼저 실행 파일이 있는 디렉토리에서 DBGHELP.DLL을 로드해 본다.
	// Windows 2000 의 System32 디렉토리에 있는 DBGHELP.DLL 파일은 버전이 
	// 오래된 것일 수 있기 때문이다. (최소 5.1.2600.0 이상이어야 한다.)
	if (::GetModuleFileName(NULL, szDbgHelpPath, MAX_PATH))
	{
		if (LPTSTR slash = ::lstrrchr(szDbgHelpPath, '\\'))
		{
			::lstrcpy(slash + 1, _T("DBGHELP.DLL"));
			hDLL = ::LoadLibrary(szDbgHelpPath);
		}
	}

	// 현재 디렉토리에 없다면, 아무 버전이나 로드한다.
	if (hDLL == NULL) hDLL = ::LoadLibrary(_T("dbghelp.dll"));

	// DBGHELP.DLL을 찾을 수 없다면 더 이상 진행할 수 없다.
	if (hDLL == NULL)
	{
		LogToFile(szLogPath, _T("dbghelp.dll not found"));
		return EXCEPTION_CONTINUE_SEARCH;
	}

	// DLL 내부에서 MiniDumpWriteDump API를 찾는다.
	MINIDUMPWRITEDUMP pfnMiniDumpWriteDump = 
		(MINIDUMPWRITEDUMP)::GetProcAddress(hDLL, "MiniDumpWriteDump");
	if (pfnMiniDumpWriteDump == NULL)
	{
		LogToFile(szLogPath, _T("dbghelp.dll too old"));
		return EXCEPTION_CONTINUE_SEARCH;
	}

	// 파일을 생성한다.
	HANDLE hFile = ::CreateFile(
		szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		LogToFile(szLogPath, _T("failed to create dump file '%s' (error %d)"), 
			szDumpPath, ::GetLastError());
		return EXCEPTION_CONTINUE_SEARCH;
	}

	MINIDUMP_EXCEPTION_INFORMATION ExParam;
	ExParam.ThreadId = ::GetCurrentThreadId();
	ExParam.ExceptionPointers = exPtrs;
	ExParam.ClientPointers = FALSE;

	MINIDUMP_TYPE dumptype = MiniDumpNormal;
	switch (s_DumpLevel)
	{
	case DUMP_LEVEL_LIGHT:  dumptype = MiniDumpNormal;         break;
	case DUMP_LEVEL_MEDIUM: dumptype = MiniDumpWithDataSegs;   break;
	case DUMP_LEVEL_HEAVY:  dumptype = MiniDumpWithFullMemory; break;
	}

	// 덤프 파일 생성 결과를 로그 파일에다 기록한다.
	if (pfnMiniDumpWriteDump(
		::GetCurrentProcess(), ::GetCurrentProcessId(), 
		hFile, dumptype, &ExParam, NULL, NULL))
	{
#ifdef UNICODE
		FILE* log = NULL;
		if (_wfopen_s(&log, szLogPath, _T("a")) == 0)
#else
		FILE* log = NULL;
		if (fopen_s(&log, szLogPath, "a") == 0)
#endif
		{
			fprintf(log,
				"saved dump file to '%s'\n"
				"\n<fault reason>\n%s"
				"\n\n<user>\n%s"
				"\n\n<os>\n%s"
				"\n\n<cpu>\n%s"
				"\n\n<memory>\n%s",
				szDumpPath,
				GetFaultReason(exPtrs),
				GetUserInfo(),
				GetOSInfo(),
				GetCpuInfo(),
				GetMemoryInfo()
				);

#ifdef HAS_STACK_WALKER
			//fprintf(log, "\n\n<stack>\n%s\n", GetCallStack());
			//fprintf(log, "\n<modules>\n%s\n", GetModules());
#endif
			fclose(log);
		}

	}
	else
	{
		LogToFile(szLogPath, _T("failed to save dump file to '%s' (error %d)"), 
			szDumpPath, ::GetLastError());
		Assert(false);
	}

	::CloseHandle(hFile);

	return EXCEPTION_EXECUTE_HANDLER;
}

namespace
{
	/// \brief 예외의 원인에 대한 문자열을 반환한다.
	/// \param exPtrs 예외 구조체 포인터
	/// \return LPCTSTR 원인 문자열
	LPCTSTR GetFaultReason(PEXCEPTION_POINTERS exPtrs)
	{
		if (::IsBadReadPtr(exPtrs, sizeof(EXCEPTION_POINTERS))) 
			return _T("bad exception pointers");

		// 간단한 에러 코드라면 그냥 변환할 수 있다.
		switch (exPtrs->ExceptionRecord->ExceptionCode)
		{
		case EXCEPTION_ACCESS_VIOLATION:         return _T("EXCEPTION_ACCESS_VIOLATION");
		case EXCEPTION_DATATYPE_MISALIGNMENT:    return _T("EXCEPTION_DATATYPE_MISALIGNMENT");
		case EXCEPTION_BREAKPOINT:               return _T("EXCEPTION_BREAKPOINT");
		case EXCEPTION_SINGLE_STEP:              return _T("EXCEPTION_SINGLE_STEP");
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return _T("EXCEPTION_ARRAY_BOUNDS_EXCEEDED");
		case EXCEPTION_FLT_DENORMAL_OPERAND:     return _T("EXCEPTION_FLT_DENORMAL_OPERAND");
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return _T("EXCEPTION_FLT_DIVIDE_BY_ZERO");
		case EXCEPTION_FLT_INEXACT_RESULT:       return _T("EXCEPTION_FLT_INEXACT_RESULT");
		case EXCEPTION_FLT_INVALID_OPERATION:    return _T("EXCEPTION_FLT_INVALID_OPERATION");
		case EXCEPTION_FLT_OVERFLOW:             return _T("EXCEPTION_FLT_OVERFLOW");
		case EXCEPTION_FLT_STACK_CHECK:          return _T("EXCEPTION_FLT_STACK_CHECK");
		case EXCEPTION_FLT_UNDERFLOW:            return _T("EXCEPTION_FLT_UNDERFLOW");
		case EXCEPTION_INT_DIVIDE_BY_ZERO:       return _T("EXCEPTION_INT_DIVIDE_BY_ZERO");
		case EXCEPTION_INT_OVERFLOW:             return _T("EXCEPTION_INT_OVERFLOW");
		case EXCEPTION_PRIV_INSTRUCTION:         return _T("EXCEPTION_PRIV_INSTRUCTION");
		case EXCEPTION_IN_PAGE_ERROR:            return _T("EXCEPTION_IN_PAGE_ERROR");
		case EXCEPTION_ILLEGAL_INSTRUCTION:      return _T("EXCEPTION_ILLEGAL_INSTRUCTION");
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: return _T("EXCEPTION_NONCONTINUABLE_EXCEPTION");
		case EXCEPTION_STACK_OVERFLOW:           return _T("EXCEPTION_STACK_OVERFLOW");
		case EXCEPTION_INVALID_DISPOSITION:      return _T("EXCEPTION_INVALID_DISPOSITION");
		case EXCEPTION_GUARD_PAGE:               return _T("EXCEPTION_GUARD_PAGE");
		case EXCEPTION_INVALID_HANDLE:           return _T("EXCEPTION_INVALID_HANDLE");
			//case EXCEPTION_POSSIBLE_DEADLOCK:        return _T("EXCEPTION_POSSIBLE_DEADLOCK");
		case CONTROL_C_EXIT:                     return _T("CONTROL_C_EXIT");
		case 0xE06D7363:                         return _T("Microsoft C++ Exception");
		default:
			break;
		}

		// 뭔가 좀 더 복잡한 에러라면...
		static TCHAR szFaultReason[2048];
		::lstrcpy(szFaultReason, _T("Unknown")); 
		::FormatMessage(
			FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
			::GetModuleHandle(_T("ntdll.dll")),
			exPtrs->ExceptionRecord->ExceptionCode, 
			0,
			szFaultReason,
			0,
			NULL);

		return szFaultReason;
	}

	/// \brief 사용자 정보를 반환한다.
	/// \return LPCTSTR 사용자 이름
	LPCTSTR GetUserInfo()
	{
		static TCHAR szUserName[200] = {0,};

		ZeroMemory(szUserName, sizeof(szUserName));
		DWORD UserNameSize = _ARRAYSIZE(szUserName) - 1;

		if (!::GetUserName(szUserName, &UserNameSize))
			::lstrcpy(szUserName, _T("Unknown"));

		return szUserName;
	}

	/// \brief 윈도우즈 버전을 반환한다.
	/// \return LPCTSTR 윈도우즈 버전 문자열
	LPCTSTR GetOSInfo()
	{
		TCHAR szWinVer[50] = {0,};
		TCHAR szMajorMinorBuild[50] = {0,};
		int nWinVer = 0;
		::GetWinVersion(szWinVer, &nWinVer, szMajorMinorBuild);

		static TCHAR szOSInfo[512] = {0,};
#ifdef UNICODE
		swprintf(szOSInfo, _ARRAYSIZE(szOSInfo)-1, L"%s (%s)",
			szWinVer, szMajorMinorBuild);
#else
		_snprintf_s(szOSInfo, _ARRAYSIZE(szOSInfo)-1, _TRUNCATE, "%s (%s)",
			szWinVer, szMajorMinorBuild);
#endif
		szOSInfo[_ARRAYSIZE(szOSInfo)-1] = 0;
		return szOSInfo;
	}

	/// \brief CPU 정보를 반환한다.
	/// \return LPCTSTR CPU 정보 문자열
	LPCTSTR GetCpuInfo()
	{
		// CPU 정보 기록
		SYSTEM_INFO    SystemInfo;
		GetSystemInfo(&SystemInfo);

		static TCHAR szCpuInfo[512] = {0,};
#ifdef UNICODE
		swprintf(szCpuInfo, _ARRAYSIZE(szCpuInfo)-1, 
			L"%d processor(s), type %d",
			SystemInfo.dwNumberOfProcessors, SystemInfo.dwProcessorType);
#else
		_snprintf_s(szCpuInfo, _ARRAYSIZE(szCpuInfo)-1, _TRUNCATE, 
			"%d processor(s), type %d",
			SystemInfo.dwNumberOfProcessors, SystemInfo.dwProcessorType);
#endif
		return szCpuInfo;
	}

	/// \brief 메모리 정보를 반환한다.
	/// \return LPCTSTR 메모리 정보 문자열
	LPCTSTR GetMemoryInfo()
	{
		static const int ONE_K = 1024;
		static const int ONE_M = ONE_K * ONE_K;
		static const int ONE_G = ONE_K * ONE_K * ONE_K;

		MEMORYSTATUS MemInfo;
		MemInfo.dwLength = sizeof(MemInfo);
		GlobalMemoryStatus(&MemInfo);

		static TCHAR szMemoryInfo[2048] = {0,};
#ifdef UNICODE
		swprintf(szMemoryInfo, _ARRAYSIZE(szMemoryInfo)-1, 
			L"%d%% of memory in use.\n"
			L"%d MB physical memory.\n"
			L"%d MB physical memory free.\n"
			L"%d MB paging file.\n"
			L"%d MB paging file free.\n"
			L"%d MB user address space.\n"
			L"%d MB user address space free.",
			MemInfo.dwMemoryLoad, 
			(MemInfo.dwTotalPhys + ONE_M - 1) / ONE_M, 
			(MemInfo.dwAvailPhys + ONE_M - 1) / ONE_M, 
			(MemInfo.dwTotalPageFile + ONE_M - 1) / ONE_M, 
			(MemInfo.dwAvailPageFile + ONE_M - 1) / ONE_M, 
			(MemInfo.dwTotalVirtual + ONE_M - 1) / ONE_M, 
			(MemInfo.dwAvailVirtual + ONE_M - 1) / ONE_M);
#else
		_snprintf_s(szMemoryInfo, _ARRAYSIZE(szMemoryInfo)-1, _TRUNCATE,
			"%d%% of memory in use.\n"
			"%d MB physical memory.\n"
			"%d MB physical memory free.\n"
			"%d MB paging file.\n"
			"%d MB paging file free.\n"
			"%d MB user address space.\n"
			"%d MB user address space free.",
			MemInfo.dwMemoryLoad, 
			(MemInfo.dwTotalPhys + ONE_M - 1) / ONE_M, 
			(MemInfo.dwAvailPhys + ONE_M - 1) / ONE_M, 
			(MemInfo.dwTotalPageFile + ONE_M - 1) / ONE_M, 
			(MemInfo.dwAvailPageFile + ONE_M - 1) / ONE_M, 
			(MemInfo.dwTotalVirtual + ONE_M - 1) / ONE_M, 
			(MemInfo.dwAvailVirtual + ONE_M - 1) / ONE_M);
#endif

		return szMemoryInfo;
	}

	/// \brief 윈도우즈 버전을 알아낸다.
	///
	/// This table has been assembled from Usenet postings, personal observations, 
	/// and reading other people's code.  Please feel free to add to it or correct 
	/// it.
	///
	/// <pre>
	/// dwPlatFormID  dwMajorVersion  dwMinorVersion  dwBuildNumber
	/// 95            1               4                 0            950
	/// 95 SP1        1               4                 0            >950 && <=1080
	/// 95 OSR2       1               4               <10            >1080
	/// 98            1               4                10            1998
	/// 98 SP1        1               4                10            >1998 && <2183
	/// 98 SE         1               4                10            >=2183
	/// ME            1               4                90            3000
	///
	/// NT 3.51       2               3                51
	/// NT 4          2               4                 0            1381
	/// 2000          2               5                 0            2195
	/// XP            2               5                 1            2600
	/// 2003 Server   2               5                 2            3790
	///
	/// CE            3
	/// </pre>
	///
	/// \param pszVersion 버전 문자열을 집어넣을 포인터
	/// \param nVersion 버전 숫자값을 집어넣을 포인터
	/// \param pszMajorMinorBuild 빌드 문자열을 집어넣을 포인터
	/// \return bool 무사히 실행한 경우에는 true, 뭔가 에러가 생긴 경우에는 false
	bool GetWinVersion(LPTSTR pszVersion, int *nVersion, LPTSTR pszMajorMinorBuild)
	{
		// from winbase.h
#ifndef VER_PLATFORM_WIN32s
#define VER_PLATFORM_WIN32s 0
#endif

#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS 1
#endif

#ifndef VER_PLATFORM_WIN32_NT
#define VER_PLATFORM_WIN32_NT 2
#endif

#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE 3
#endif

		static LPCTSTR WUNKNOWNSTR     = _T("Unknown Windows Version");
		static LPCTSTR W95STR          = _T("Windows 95");
		static LPCTSTR W95SP1STR       = _T("Windows 95 SP1");
		static LPCTSTR W95OSR2STR      = _T("Windows 95 OSR2");
		static LPCTSTR W98STR          = _T("Windows 98");
		static LPCTSTR W98SP1STR       = _T("Windows 98 SP1");
		static LPCTSTR W98SESTR        = _T("Windows 98 SE");
		static LPCTSTR WMESTR          = _T("Windows ME");
		static LPCTSTR WNT351STR       = _T("Windows NT 3.51");
		static LPCTSTR WNT4STR         = _T("Windows NT 4");
		static LPCTSTR W2KSTR          = _T("Windows 2000");
		static LPCTSTR WXPSTR          = _T("Windows XP");
		static LPCTSTR W2003SERVERSTR  = _T("Windows 2003 Server");
		static LPCTSTR WCESTR          = _T("Windows CE");

		static const int WUNKNOWN      = 0;
		static const int W9XFIRST      = 1;
		static const int W95           = 1;
		static const int W95SP1        = 2;
		static const int W95OSR2       = 3;
		static const int W98           = 4;
		static const int W98SP1        = 5;
		static const int W98SE         = 6;
		static const int WME           = 7;
		static const int W9XLAST       = 99;
		static const int WNTFIRST      = 101;
		static const int WNT351        = 101;
		static const int WNT4          = 102;
		static const int W2K           = 103;
		static const int WXP           = 104;
		static const int W2003SERVER   = 105;
		static const int WNTLAST       = 199;
		static const int WCEFIRST      = 201;
		static const int WCE           = 201;
		static const int WCELAST       = 299;

		if (!pszVersion || !nVersion || !pszMajorMinorBuild) return false;

		::lstrcpy(pszVersion, WUNKNOWNSTR);
		*nVersion = WUNKNOWN;

		OSVERSIONINFO osinfo;
		osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		if (!GetVersionEx(&osinfo)) return false;

		DWORD dwPlatformId   = osinfo.dwPlatformId;
		DWORD dwMinorVersion = osinfo.dwMinorVersion;
		DWORD dwMajorVersion = osinfo.dwMajorVersion;
		DWORD dwBuildNumber  = osinfo.dwBuildNumber & 0xFFFF; // Win 95 needs this

		TCHAR buf[50] = {0, };
#ifdef UNICODE
		swprintf(buf, _ARRAYSIZE(buf), L"%u.%u.%u", 
			dwMajorVersion, dwMinorVersion, dwBuildNumber);
#else
		_snprintf_s(buf, _ARRAYSIZE(buf), _TRUNCATE, "%u.%u.%u", 
			dwMajorVersion, dwMinorVersion, dwBuildNumber);
#endif
		::lstrcpy(pszMajorMinorBuild, buf);

		if ((dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && (dwMajorVersion == 4))
		{
			if ((dwMinorVersion < 10) && (dwBuildNumber == 950))
			{
				::lstrcpy(pszVersion, W95STR);
				*nVersion = W95;
			}
			else if ((dwMinorVersion < 10) && 
				((dwBuildNumber > 950) && (dwBuildNumber <= 1080)))
			{
				::lstrcpy(pszVersion, W95SP1STR);
				*nVersion = W95SP1;
			}
			else if ((dwMinorVersion < 10) && (dwBuildNumber > 1080))
			{
				::lstrcpy(pszVersion, W95OSR2STR);
				*nVersion = W95OSR2;
			}
			else if ((dwMinorVersion == 10) && (dwBuildNumber == 1998))
			{
				::lstrcpy(pszVersion, W98STR);
				*nVersion = W98;
			}
			else if ((dwMinorVersion == 10) && 
				((dwBuildNumber > 1998) && (dwBuildNumber < 2183)))
			{
				::lstrcpy(pszVersion, W98SP1STR);
				*nVersion = W98SP1;
			}
			else if ((dwMinorVersion == 10) && (dwBuildNumber >= 2183))
			{
				::lstrcpy(pszVersion, W98SESTR);
				*nVersion = W98SE;
			}
			else if (dwMinorVersion == 90)
			{
				::lstrcpy(pszVersion, WMESTR);
				*nVersion = WME;
			}
		}
		else if (dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			if ((dwMajorVersion == 3) && (dwMinorVersion == 51))
			{
				::lstrcpy(pszVersion, WNT351STR);
				*nVersion = WNT351;
			}
			else if ((dwMajorVersion == 4) && (dwMinorVersion == 0))
			{
				::lstrcpy(pszVersion, WNT4STR);
				*nVersion = WNT4;
			}
			else if ((dwMajorVersion == 5) && (dwMinorVersion == 0))
			{
				::lstrcpy(pszVersion, W2KSTR);
				*nVersion = W2K;
			}
			else if ((dwMajorVersion == 5) && (dwMinorVersion == 1))
			{
				::lstrcpy(pszVersion, WXPSTR);
				*nVersion = WXP;
			}
			else if ((dwMajorVersion == 5) && (dwMinorVersion == 2))
			{
				::lstrcpy(pszVersion, W2003SERVERSTR);
				*nVersion = W2003SERVER;
			}
		}
		else if (dwPlatformId == VER_PLATFORM_WIN32_CE)
		{
			::lstrcpy(pszVersion, WCESTR);
			*nVersion = WCE;
		}

		return true;

#undef VER_PLATFORM_WIN32s
#undef VER_PLATFORM_WIN32_WINDOWS
#undef VER_PLATFORM_WIN32_NT
#undef VER_PLATFORM_WIN32_CE
	}

	/// \brief strrchr TCHAR 버전
	/// \param str 검색할 문자열
	/// \param ch 찾고자 하는 글자
	/// \return TCHAR* 주어진 글자를 찾은 경우 해당 위치 포인터를 반환하고, 
	/// 찾지 못한 경우에는 NULL을 반환한다.
	TCHAR* lstrrchr(TCHAR* str, TCHAR ch)
	{
		TCHAR* start = str;
		while (*str++) ;
		while (--str != start && *str != ch) ;
		return *str == ch ? str : NULL;
	}
}