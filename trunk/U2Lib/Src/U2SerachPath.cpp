#include <Src/U2LibPch.h>

#include "U2Filename.h"
#include "U2SearchPath.h"


TCHAR U2SearchPath::ms_defPath[MAX_PATH];

U2SearchPath::U2SearchPath()
	:m_uNextPath(0) 
{
	m_filePath[0] = _T('\0');
	m_refPath[0] = _T('\0');

}

U2SearchPath::~U2SearchPath()
{

}


void U2SearchPath::SetFilePath(const TCHAR* szPath)
{
	if(szPath && szPath[0] != _T('\0'))
	{
		_tcsncpy_s(m_filePath, MAX_PATH, szPath, MAX_PATH-1);

	}
	else 
		m_filePath[0] = _T('\0');

}


void U2SearchPath::SetReferencePath(const TCHAR* szPath)
{
	if(szPath && szPath[0] != _T('\0'))
	{
		U2Filename searchPath(szPath);
		searchPath.SetFilename(_T(""));
		searchPath.SetExt(_T(""));
		searchPath.FullPath(m_refPath, MAX_PATH);

	}
	else 
		m_refPath[0] = _T('\0');
}


void	U2SearchPath::Reset()
{
	m_uNextPath = 0;
}


bool	U2SearchPath::NextSearchPath(TCHAR* szPath, uint32 len )
{
	U2Filename fullPath(m_filePath);

	switch(m_uNextPath)
	{
	case 0:
		break;
	case 1:
		fullPath.SetDrive(_T(""));
		fullPath.SetDir(_T(""));
		break;
	case 2:
		fullPath.SetDrive(_T(""));
		fullPath.SetDir(m_refPath);		
		break;
	case 3:
		if(ms_defPath[0] != _T('\0'))
		{
			fullPath.SetDrive(_T(""));
			fullPath.SetDir(ms_defPath);
			break;
		}
	default:
		return false;
		 
	}

	fullPath.FullPath(szPath, len);
	m_uNextPath++;

	return true;
}


void	U2SearchPath::SetDefaultPath(const TCHAR* path)
{
	_tcsncpy_s(ms_defPath, MAX_PATH, path, MAX_PATH - 1);
}


const TCHAR* U2SearchPath::GetDefaultPath()
{
	return ms_defPath;
}