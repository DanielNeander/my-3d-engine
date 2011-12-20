#include "stdafx.h"


#include "U2Filename.h"

#define DIR_DELIMITER _T("\\")


U2Filename::U2Filename(const TCHAR* szFullPath)
{
	SplitPath(szFullPath);
}


bool U2Filename::FullPath(TCHAR* szFullpath, uint32 len) const 
{
	return MakePath(szFullpath, len);
}

void	U2Filename::SplitPath(const TCHAR* szPath)
{
	_tsplitpath_s(szPath, m_drive, _MAX_DRIVE, m_dir, _MAX_DIR, 
		m_fname, _MAX_FNAME, m_ext, _MAX_EXT);
}


bool	U2Filename::MakePath(TCHAR* szPath, uint32 len) const
{
	if(szPath == NULL)
		return false;

	size_t reqLen = 1;
	if(m_drive[0] != _T('\0'))
	{
		reqLen += 2;
	}

	if(m_dir[0] != _T('\0'))
	{
		size_t len = _tcslen(m_dir);
		reqLen += len;
		if(m_dir[len-1] != _T('\\') && m_dir[len-1] != _T('/'))
			reqLen += _tcslen(DIR_DELIMITER);
	}

	if(m_fname[0] != _T('\0'))
	{
		reqLen += _tcslen(m_fname);
	}

	if(m_ext[0] != _T('\0'))
	{
		if(m_ext[0] != _T('.'))
			reqLen++;
		reqLen += _tcslen(m_ext);
	}

	if(reqLen > len)
		return false;

	
	// √ ±‚»≠
	szPath[0] = _T('\0');

	if(m_drive[0] != _T('\0'))
	{
		szPath[0] = m_drive[0];
		szPath[1] = _T(':');
		szPath[2] = _T('\0');
	}

	if(m_dir[0] != _T('\0'))
	{
		_tcscat_s(szPath, len, m_dir);
		size_t lenPath = _tcslen(szPath);

		if(szPath[lenPath -1] != _T('\\') && szPath[lenPath-1] != _T('/'))
		{
			_tcscat_s(szPath, len, DIR_DELIMITER);
		}
	}

	if(m_fname[0] != _T('\0'))
		_tcscat_s(szPath, len, m_fname);

	if(m_ext[0] != _T('\0'))
	{
		if(m_ext[0] != _T('.'))
			_tcscat_s(szPath, len, _T("."));
		_tcscat_s(szPath, len, m_ext);
	}
	return true;
}
