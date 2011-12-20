#include "stdafx.h"

#define TEST_ASSETLOAD	0
#define TEST_SHADOWMAP  1


#include "MemLeakDetect.h"
#include <U2Misc/Src/DebugConsole.h>
#include <U2_3D/src/U2Main.h>

#if TEST_ASSETLOAD
#include "AssetViewerApp.h"
#elif TEST_SHADOWMAP
#include "ShadowMapApp.h"
#endif

#define CHECK_MEMORY_LEAK 0

#if CHECK_MEMORY_LEAK
CMemLeakDetect gMemLeakDetect;
#endif 



int main(int IQuality, TCHAR* apcArg[])
{

	// 디렉토리 세팅 
#if CHECK_MEMORY_LEAK
	_CrtSetDbgFlag (
		_CRTDBG_ALLOC_MEM_DF |
		_CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode ( _CRT_ERROR,
		_CRTDBG_MODE_DEBUG);
#endif

	U2Main::Initialize();

	int exitCode = 0;



	/*std::string str("DebugTest");
	debug_con << str.c_str();
	DebugConsole::flush();
	debug_con << str.c_str();
	DebugConsole::flush();

	dbgout("test3");
	dbgout(5);
	dbgout(5.0f);*/

#if TEST_ASSETLOAD
	AssetViewApp::Create();
#elif TEST_SHADOWMAP
	ShadowMapApp::Create();
#endif
	


	

	if(U2App::Run)
	{
		// Default Data Path 

		U2App::TheCommand = U2_NEW U2Command(IQuality, apcArg);
		exitCode = U2App::Run(IQuality, apcArg);
		U2_DELETE U2App::TheCommand;
		U2App::TheCommand = 0;
	}
	else 
		exitCode = INT_MAX;

	U2_DELETE U2App::TheApplication;
	U2App::TheApplication = 0;

	U2Main::Terminate();


	return exitCode;
}