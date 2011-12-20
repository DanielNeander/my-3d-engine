// U2UnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <windows.h>
#include <iostream>
#include <conio.h>
using namespace std;

#include "Character.h"
using namespace AURELIO_REIS;

#pragma comment(lib, "WinMM.lib")


// The character object.
CCharacter g_Character;

#include "U2TString.h"



int _tmain(int argc, _TCHAR* argv[])
{
	FILE* pFile = fopen(_T("application.log"), "a");
	Output2FILE::Stream() = pFile;
	FILELog::ReportingLevel() = FILELog::FromString(argv[1] ? argv[1] : "DEBUG1");

	U2Filename fname("E:/c++/Game/Data/texture/gravel.bmp");

	FILE_LOG(logDEBUG) << fname.GetDrive() <<"	" << fname.GetDir() << "	" <<
		fname.GetFilename() << "	" << fname.GetExt();

	char szPath[MAX_PATH];	
	fname.SetDir("/c++/game/data/shader/");
	fname.SetFilename("Default");
	fname.SetExt("cg");
	fname.FullPath(szPath, MAX_PATH);	

	FILE_LOG(logDEBUG) << fname.GetDrive() <<"	" << fname.GetDir() << "	" <<
		fname.GetFilename() << "	" << fname.GetExt() << szPath;

	U2FilePath fpath;
	

	fpath.GetCurrentWorkingDirectory(szPath, sizeof(szPath));

	FILE_LOG(logDEBUG) << szPath;

	strcpy(szPath, "Debug");
	fpath.ConvertToAbs(szPath, sizeof(szPath));


	char szRelHere[64] = "E:\\c++\\Game\\Data";
	char szRelPath[64] = "shader";
	fpath.ConvertToAbs(szPath, sizeof(szPath), szRelPath, szRelHere);

	FILE_LOG(logDEBUG) << szPath;

	U2SearchPath spath;

	//spath.SetFilePath("E:\\c++\\Game\\Data\\texure\\redsky.bmp");
	

	spath.SetDefaultPath("E:\\c++\\Game\\Data");
	spath.SetReferencePath("E:\\c++\\Game\\Data\\");

	spath.SetFilePath("redsky.bmp");
	spath.Reset();

	while(spath.NextSearchPath(szPath, MAX_PATH))
	{
		FILE *pFile = fopen(szPath, "r");
		if(pFile)
		{
			FILE_LOG(logDEBUG) << "FileSearch "<< szPath;
			break;
		}
	}
	
	U2StringA str("I am a boy");

	printf("Dynamic String Output : %s\n", str.Str());


	U2StackStringA str2(str);
	printf("Stack String Output : %s\n", str2);
	
	
	printf("Stack String Ch : %c\n", str2.GetAt(5));

	str2.SetAt(2, 'x');

	printf("SetAt Test  : %s\n", str2);
	
	str2.ToUpper();

	printf("ToUpper Test  : %s\n", str2);

	str2.ToLower();

	printf("ToLower Test : %s\n", str2);

	U2StackStringA temp(str2);
	temp.TrimLeft('i');

	printf("TrimLeft Test : %s\n", temp);

	temp = str2;
	temp.TrimRight('y');

	printf("TrimRight Test : %s\n", temp);

	U2StackStringA test1;
	test1.AppendInt(0);

	printf("AppendInt Test : %s\n", test1);



	// Release  모드 에서 Concatenate 에러.
	U2DynString skinMeshFullpath("Test");
	skinMeshFullpath.Concatenate("_skin");	

	printf("Concatenate Test : %s\n", skinMeshFullpath);

	// TODO: FindFiles in the 'characters' directory to load
	// all character files in it.	
	const char *strFileName = "../Data/Character.txt";

	// Begin the rudimentary timer.
	DWORD dwStartTime = timeGetTime();

	// Load and show the players states.
	if ( !g_Character.LoadStats( strFileName ) )
	{
		return -1;
	}

	cout << "Parsing completed in " << ( timeGetTime() - dwStartTime ) * 0.001f
		<< " seconds." << endl;

	// Show the players stats.
	g_Character.ShowStats();

	cout << endl << endl << endl;
	getch();

	return 0;
}

