// Logger.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#ifdef LOG1
#include "log1.h"
#else
#include "log.h"
#endif


int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		FILE* pFile = fopen(_T("application.log"), "a");
		Output2FILE::Stream() = pFile;
		FILELog::ReportingLevel() = FILELog::FromString(argv[1] ? argv[1] : "DEBUG1");
		const int count = 5;
		FILE_LOG(logDEBUG) << "A loop with " << count << " iterations";		
		for (int i = 0; i != count; ++i)
		{
			FILE_LOG(logDEBUG1) << "the counter i = " << i;			
		}
		return 0;
	}
	catch(const std::exception& e)
	{
		FILE_LOG(logERROR) << e.what();
	}
	return -1;
}

