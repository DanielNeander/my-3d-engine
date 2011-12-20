#define STACKTRACE_CPP
#define CORE_DLL

#define DBGHELP_TRANSLATE_TCHAR
/***************************************************************\

^^^^^^^^  ^^^^^^^^  ^^^^^^^^  ^^^^^^^^
^^    ^^  ^^    ^^     ^^     ^^    ^^
^^        ^^^^^^^^     ^^     ^^^^^^^^
^^   ^^^  ^^    ^^     ^^     ^^    ^^
^^    ^^  ^^    ^^     ^^     ^^    ^^
^^^^^^^^  ^^    ^^  ^^^^^^^^  ^^    ^^

sample code from the book...
Real Time 3D Terrain Engines Using C++ and DirectX

by Greg Snook
greg@mightystudios.com

\***************************************************************/
#include "U2stacktrace.h"
#include <windows.h>
#include <tchar.h>
#include "imagehlp.h"
#include "u2debug.h"
#include <vector>


/*	Stack Tracing
------------------------------------------------------------------------------------------

The stack tracing code is based on an article by John Robbins
that appeared in Microsoft Systems Journal Bugslayer Column - Feb 99

The article can be found at:
http://www.microsoft.com/msj/defaultframe.asp?page=/msj/0299/bugslayer/bugslayer0299.htm

------------------------------------------------------------------------------------------
*/

// The address typedef.
typedef std::vector<uint32> ADDRESS_VECTOR ;

// The current process handle
static HANDLE g_hProcess = 0;


static uint32 __stdcall getModBase( HANDLE hProcess, uint32 dwAddr )
{
	IMAGEHLP_MODULE imageHelp ;
	imageHelp.SizeOfStruct = sizeof( IMAGEHLP_MODULE );

	if( SymGetModuleInfo(g_hProcess, dwAddr, &imageHelp ) )
	{
		return( imageHelp.BaseOfImage );
	}
	else
	{
		MEMORY_BASIC_INFORMATION memoryBasicInfo;

		if( 0 != VirtualQueryEx(hProcess,
			(LPCVOID)dwAddr,
			&memoryBasicInfo,
			sizeof( memoryBasicInfo )  ) )
		{
			uint32 dwNameLen = 0 ;
			CHAR szFileA[ MAX_PATH ];
			WCHAR szFileW[ MAX_PATH ];

			dwNameLen = GetModuleFileName((HINSTANCE)
				memoryBasicInfo.AllocationBase,
				szFile,
				MAX_PATH                  );

			HANDLE hFile = NULL ;

			if( 0 != dwNameLen )
			{
				hFile = CreateFile( szFile,
					GENERIC_READ,
					FILE_SHARE_READ,
					NULL,
					OPEN_EXISTING,
					0,
					0);
			}

			SymLoadModule64( g_hProcess, hFile,
				( dwNameLen ? szFile: NULL ),
				NULL,
				(uint32)memoryBasicInfo.AllocationBase,
				0);
		

			return((uint32)memoryBasicInfo.AllocationBase );
		}
	}
	return( 0 );
}

static uint32 convertAddress( uint32 dwAddr, LPTSTR szOutBuff )
{
	char szTemp [ MAX_PATH + sizeof( IMAGEHLP_SYMBOL ) ];

	PIMAGEHLP_SYMBOL pIHSymbol =(PIMAGEHLP_SYMBOL)&szTemp ;

	IMAGEHLP_MODULET imageHelp ;

	LPTSTR pCurrPos = szOutBuff ;

	ZeroMemory( pIHSymbol, MAX_PATH + sizeof( IMAGEHLP_SYMBOL ) );
	ZeroMemory( &imageHelp, sizeof( IMAGEHLP_MODULE ) );

	pIHSymbol->SizeOfStruct = sizeof( IMAGEHLP_SYMBOL );
	pIHSymbol->Address = dwAddr ;
	pIHSymbol->MaxNameLength = MAX_PATH ;

	imageHelp.SizeOfStruct = sizeof( IMAGEHLP_MODULE );

	// Always stick the address in first.
	pCurrPos += wsprintf( pCurrPos, _T( "0x%08X " ), dwAddr );

	// Get the module name.
	if( 0 != SymGetModuleInfoT( g_hProcess, dwAddr, &imageHelp ) )
	{
		// Strip off the path.
		LPTSTR szName = _tcsrchr( imageHelp.ImageName, _T( '\\' ) );
		if( NULL != szName )
		{
			szName++;
		}
		else
		{
			szName = imageHelp.ImageName ;
		}
		pCurrPos += wsprintf( pCurrPos, _T( "%s: " ), szName );
	}
	else
	{
		pCurrPos += wsprintf( pCurrPos, _T( "<unknown module>: " ) );
	}

	// Get the function.
	uint32 dwDisp ;
	if( 0 != SymGetSymFromAddr( g_hProcess, dwAddr, &dwDisp, pIHSymbol ) )
	{
		if( 0 == dwDisp )
		{
			pCurrPos += wsprintf( pCurrPos, _T( "%s" ), pIHSymbol->Name);
		}
		else
		{
			pCurrPos += wsprintf( pCurrPos,
				_T( "%s + %d bytes" ),
				pIHSymbol->Name,
				dwDisp                  );
		}

		// If I got a symbol, give the source and line a whirl.
		IMAGEHLP_LINE stIHL ;

		ZeroMemory( &stIHL, sizeof( IMAGEHLP_LINE ) );

		stIHL.SizeOfStruct = sizeof( IMAGEHLP_LINE );

		if( 0 != SymGetLineFromAddr( g_hProcess, dwAddr,
			&dwDisp,
			&stIHL   ) )
		{
			// Put this on the next line and indented a bit.
			pCurrPos += wsprintf( pCurrPos,
				_T( "\n\t\t%s, Line %d" ),
				stIHL.FileName,
				stIHL.LineNumber            );
			if( 0 != dwDisp )
			{
				pCurrPos += wsprintf( pCurrPos,
					_T( " + %d bytes" ),
					dwDisp                );
			}
		}
	}
	else
	{
		pCurrPos += wsprintf( pCurrPos, _T( "<unknown symbol>" ) );
	}

	// Tack on a CRLF.
	pCurrPos += wsprintf( pCurrPos, _T( "\n" ) );

	return( pCurrPos - szOutBuff );
}

void buildStackTrace( TCHAR* szString,
						   uint32  dwSize,
						   uint32  dwNumSkip  )
{
	HANDLE hProcess = GetCurrentProcess( );

	// if the symbol handler is not initialized, 
	// set it up now.
	if( !g_hProcess && SymInitialize(hProcess, NULL, FALSE))
	{
		uint32 dwOpts = SymGetOptions( );

		// Turn on load lines.
		SymSetOptions( dwOpts|SYMOPT_LOAD_LINES      );

		g_hProcess = hProcess;
	}

	ADDRESS_VECTOR vAddrs;
	CONTEXT context;

	context.ContextFlags = CONTEXT_FULL ;

	if( GetThreadContext( GetCurrentThread( ), &context ) )
	{
		STACKFRAME stackFrame ;
		uint32      dwMachine ;

		ZeroMemory( &stackFrame, sizeof( STACKFRAME ) );

		stackFrame.AddrPC.Mode = AddrModeFlat ;

		dwMachine                = IMAGE_FILE_MACHINE_I386 ;
		stackFrame.AddrPC.Offset    = context.Eip   ;
		stackFrame.AddrStack.Offset = context.Esp   ;
		stackFrame.AddrStack.Mode   = AddrModeFlat ;
		stackFrame.AddrFrame.Offset = context.Ebp   ;
		stackFrame.AddrFrame.Mode   = AddrModeFlat ;

		// Loop for the first 512 stack elements.
		for( uint32 i = 0 ; i < 512 ; i++ )
		{
			if( FALSE == StackWalk( dwMachine,
				hProcess,
				hProcess,
				&stackFrame,
				&context,
				NULL,
				SymFunctionTableAccess,
				getModBase,
				NULL                    ) )
			{
				break ;
			}
			if( i > dwNumSkip )
			{
				// Also check that the address is not zero.  Sometimes
				//  StackWalk returns TRUE with a frame of zero.
				if( 0 != stackFrame.AddrPC.Offset )
				{
					vAddrs.push_back( stackFrame.AddrPC.Offset );
				}
			}
		}

		// Now start converting the addresses.
		uint32 dwSizeLeft = dwSize ;
		uint32 dwSymSize ;

		TCHAR szSym [ MAX_PATH * 2 ];
		LPTSTR szCurrPos = szString ;

		ADDRESS_VECTOR::iterator loop ;
		for( loop =  vAddrs.begin( );
			loop != vAddrs.end( )  ;
			loop++                     )
		{

			dwSymSize = convertAddress( *loop, szSym );
			if( dwSizeLeft < dwSymSize )
			{
				break ;
			}
			_tcscpy( szCurrPos, szSym );
			szCurrPos += dwSymSize ;
			dwSizeLeft -= dwSymSize ;
		}
	}
}