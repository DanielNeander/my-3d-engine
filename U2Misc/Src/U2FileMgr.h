/**************************************************************************************************
module	:	U2FileMgr
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_FILEMGR_H
#define U2_FILEMGR_H

#include <U2Lib/Src/U2RefObject.h>
#include <U2Lib/Src/U2SmartPtr.h>


class U2MISC U2FileMgr : public U2RefObject
{
public:
	U2FileMgr();
	virtual ~U2FileMgr();

	
	static U2FileMgr* Create();
	static void Terminate();

	
	static U2FileMgr* Instance() { return ms_pInstance; }

	virtual bool FileExists(const U2DynString& pathName) const;
	virtual U2WinFile* CreateNewFile() const;

	static void Initialize();
	
	static int	GetNumDirectory();
	static const TCHAR* GetDirectory(int i);
	static bool InsertDirectory(const TCHAR* directory);
	static bool RemoveDirectory(const TCHAR* directory);
	static void RemoveAllDirectories();

	static const TCHAR* GetPath(const TCHAR* directory, const TCHAR* filename);
	static const TCHAR* GetFullPath(const TCHAR* file);

	
private:
	
	static U2FileMgr* ms_pInstance;

	uint32 m_uBytesRead;
	uint32 m_uBytesWritten;
	uint32 m_uNumSeeks;

	enum { MAX_FILE_PATH = 1024};

	static TCHAR ms_path[MAX_FILE_PATH];
	static U2ObjVec<U2DynString>	ms_directories;

};

typedef U2SmartPtr<U2FileMgr> U2FileMgrPtr;


#endif 