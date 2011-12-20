#include "stdafx.h"

#include "U2FilePath.h"



void U2FilePath::RemoveSlashDotSlash(TCHAR* szPath)
{
	const TCHAR slashToRemove[] = _T("\\.\\");

	// \.\를 발견하면 이후 값으로 복사하고 
	// \.\가 없을 때까지 반복한다.
	TCHAR *szNext = _tcsstr(szPath, slashToRemove);
	while(szNext)
	{
		size_t len = _tcslen(szNext);
		const TCHAR* afterSlash = szNext + 2;
		_tcscpy_s(szNext, len, afterSlash);
		szNext = _tcsstr(szPath, slashToRemove);

	}
}

bool U2FilePath::IsRelPath(const TCHAR *pcPath)
{
	// An absolute path is one that begins with either of the following:
	// [1] a forward or backward slash
	// [2] A drive letter followed by ":"

	size_t stLen = _tcslen(pcPath);
	if (stLen < 2)
	{
		// the smallest absolute path is slash followed by letter, so
		// this must be a relative path.
		return true;
	}

	// check for case 1
	TCHAR c1stChar = pcPath[0];   
	if (c1stChar == _T('\\') || c1stChar == ('/'))
	{
		// test for case 1 indicates this is an absolute path
		return false;
	}

	// check for case 2
	c1stChar = static_cast<TCHAR>(toupper(c1stChar));
	TCHAR c2ndChar = pcPath[1];
	if (c2ndChar == ':' && (c1stChar >= _T('A') && c1stChar <= _T('Z')))
	{   
		// test for case 2 indicates this is an absolute path
		return false;
	}

	return true;
}


void U2FilePath::RemoveDotDot(TCHAR* szPath)
{
	RemoveSlashDotSlash(szPath);

	TCHAR* szPtr;

	// szPath는 상대 경로
	szPtr = szPath;
	while(szPtr && (*szPtr == _T('.') || *szPtr == _T('\\')))
		szPtr++;

	if(!szPtr)
		return;
	
	TCHAR* szPathNext = szPtr + _tcslen(szPath) + 1;
	const TCHAR slashDotDot[] = _T("\\..");

	TCHAR* szNextDir = _tcsstr(szPtr, slashDotDot);
	while(szNextDir)
	{
		// /..을 발견.
		*szNextDir = 0;

		TCHAR* szLastDir = _tcschr(szPtr, _T('\\'));

		if(szLastDir)
		{
			szNextDir += 3;
		}
		else 
		{
			// 1) /../dir 2) /../0 3) /../
			if(szNextDir + 3)
			{
				szNextDir += 4;
			}
			else 
			{
				szNextDir += 3;
			}

			szLastDir = szPtr;
		}

		for(int i = 0; i < szPathNext - szNextDir ; ++i)
		{
			szLastDir[i] = szNextDir[i];
		}

		while(szPtr && (*szPtr == _T('.') || *szPtr == _T('\\')))
		{
			++szPtr;
		}
		
		szNextDir = _tcsstr(szPtr, slashDotDot);
	}
}


bool U2FilePath::GetCurrentWorkingDirectory(TCHAR* pcPath, size_t stDestSize)
{
	return (_tgetcwd(pcPath, (int)stDestSize) != NULL);
}

size_t U2FilePath::ConvertToAbs(TCHAR* szPath, size_t pathBytes, 
							  const TCHAR* szRelHere)
{
	const size_t maxPathBufferLen = MAX_PATH * 2 + 2;
	TCHAR acAbsPath[maxPathBufferLen];
	size_t absBytes = ConvertToAbs(acAbsPath, maxPathBufferLen, szPath, szRelHere);
	if(absBytes < pathBytes)
	{
		_tcscpy_s(szPath, pathBytes, acAbsPath);
		return absBytes;
	}
	else 
	{
		szPath[0] = NULL;
		return 0;
	}
}

size_t U2FilePath::ConvertToAbs(TCHAR* szPath, size_t pathBytes)
{
	TCHAR acCWD[MAX_PATH];
	if(!GetCurrentWorkingDirectory(acCWD, MAX_PATH))
		return 0;
	return ConvertToAbs(szPath, pathBytes, acCWD);
}

size_t U2FilePath::ConvertToAbs(TCHAR *szAbsPath, size_t absBytes, 
				  const TCHAR* szRelPath,const TCHAR* szRelHere)
{
	assert(szAbsPath && szRelPath);
	assert(IsRelPath(szRelPath));
	assert(szAbsPath != szRelPath);

	if(!szRelHere)
	{
		szRelHere = _T("");
	}

	size_t lenRelHere = _tcslen(szRelHere);
	if(lenRelHere == 0)
	{
		TCHAR acCWD[MAX_PATH];
		if(!GetCurrentWorkingDirectory(acCWD, MAX_PATH))
		{
			if(absBytes > 0)
				szAbsPath[0] = NULL;
			return 0;
		}
		assert(_tcslen(acCWD)!=0);
		return ConvertToAbs(szAbsPath, absBytes, szRelPath, szRelHere);
	}

	size_t lenRelPath = _tclen(szRelPath);

	bool bInsertDelimiter = (szRelHere[lenRelHere-1] != _T('\\'));

	size_t reqSize = 1 + lenRelHere + 
		lenRelPath + ((bInsertDelimiter) ? 1 : 0);

	if(absBytes < reqSize)
	{
		if(absBytes > 0)
			szAbsPath[0] = NULL;
		return 0;
	}

	_tcscpy_s(szAbsPath, absBytes, szRelHere);

	if(bInsertDelimiter)
		_tcscat_s(szAbsPath, absBytes, _T("\\"));

	_tcscat_s(szAbsPath, absBytes, szRelPath);
	RemoveDotDot(szAbsPath);

	FILE_LOG(logDEBUG) << szAbsPath << szRelPath << szRelHere;


	return _tcslen(szAbsPath);
}