/************************************************************************
module	: U2Filename
Author	: Yun sangyong
Desc	: 파일 이름을 분해하거나 생성할때 쓰는 유틸 클래스
			GameBryo 2.6 버전 참조.
************************************************************************/
#pragma once
#ifndef U2_FILENAME_H
#define	U2_FILENAME_H


#ifndef _MAX_DIR
#define _MAX_DIR 256
#endif
#ifndef _MAX_DRIVE
#define _MAX_DRIVE 3
#endif
#ifndef _MAX_EXT
#define _MAX_EXT 256
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME 256
#endif

class U2Filename
{
public:
	U2Filename(const TCHAR* szFullPath);

	void	SetDir(const TCHAR* szDir);
	void	SetDrive(const TCHAR* szDrive);
	void	SetExt(const TCHAR* szExt);
	void	SetFilename(const TCHAR* szFname);
	
	const TCHAR* GetDir() const;
	const TCHAR* GetDrive() const;
	const TCHAR* GetExt() const;
	const TCHAR* GetFilename() const;
	
	bool	FullPath(TCHAR* szFullPath, uint32 len) const;

private:
	void	SplitPath(const TCHAR* szPath);
	bool	MakePath(TCHAR* szPath, uint32 len) const;

	TCHAR m_dir[_MAX_DIR];
	TCHAR m_drive[_MAX_DRIVE];
	TCHAR m_ext[_MAX_EXT];
	TCHAR m_fname[_MAX_FNAME];
	
};


inline 
void	U2Filename::SetDir(const TCHAR* szDir)
{
	_tcscpy_s(m_dir, _MAX_DIR, szDir);
}


inline 
void	U2Filename::SetDrive(const TCHAR* szDrive)
{
	_tcscpy_s(m_drive, _MAX_DRIVE, szDrive);
}


inline 
void	U2Filename::SetExt(const TCHAR* szExt)
{
	_tcscpy_s(m_ext, _MAX_EXT, szExt);
}


inline 
void	U2Filename::SetFilename(const TCHAR* szFname)
{
	_tcscpy_s(m_fname, _MAX_FNAME, szFname);
}


inline 
const TCHAR* U2Filename::GetDir() const
{
	return m_dir;
}


inline 
const TCHAR* U2Filename::GetDrive() const
{
	return m_drive;
}


inline 
const TCHAR* U2Filename::GetExt() const
{
	return m_ext;
}


inline 
const TCHAR* U2Filename::GetFilename() const
{
	return m_fname;
}


#endif