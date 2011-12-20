//#include <U2_3D/Src/U23DLibPCH.h>
#include <stdafx.h>
#include "U2FileMgr.h"

U2FileMgr* U2FileMgr::ms_pInstance = 0;

TCHAR U2FileMgr::ms_path[MAX_FILE_PATH] = {0};
U2ObjVec<U2DynString> U2FileMgr::ms_directories;


U2FileMgr::U2FileMgr()
:m_uBytesRead(0),
m_uBytesWritten(0),
m_uNumSeeks(0)
{
	ms_pInstance = this;
}

U2FileMgr::~U2FileMgr()
{
	
}



U2FileMgr* U2FileMgr::Create()
{
	U2FileMgr* pFileMgr = U2_NEW U2FileMgr;
	U2ASSERT(pFileMgr);

	return pFileMgr;
}

//-------------------------------------------------------------------------------------------------
void U2FileMgr::Terminate()
{
	U2_DELETE ms_pInstance;
	ms_pInstance = NULL;
}

void U2FileMgr::Initialize()
{

	TCHAR currPath[256];
	::GetCurrentDirectory(256, currPath);	
}

int U2FileMgr::GetNumDirectory()
{
	return ms_directories.FilledSize();
}

const TCHAR* U2FileMgr::GetDirectory(int i)
{
	if(0 <= i && i < (int)ms_directories.FilledSize())
		return ms_directories[i].Str();

	return 0;
}


bool U2FileMgr::InsertDirectory(const TCHAR* directory)
{
	U2DynString dirPlusSlash(directory);
	dirPlusSlash.Concatenate(_T("\\"));
	for(int i = 0; i < (int)ms_directories.FilledSize() ; ++i)
	{
		if(dirPlusSlash == ms_directories[i])
			return false;
	}

	ms_directories.AddElem(dirPlusSlash);
	return true;
}

bool U2FileMgr::RemoveDirectory(const TCHAR* directory)
{
	U2DynString dirPlusSlash(directory);
	dirPlusSlash.Concatenate(_T("\\"));
	for(unsigned int i=0; i < ms_directories.FilledSize(); ++i)
	{
		if(dirPlusSlash == ms_directories[i])
		{
			ms_directories.Remove(i);
			return true;
		}
	}

	return false;

}

void
U2FileMgr::RemoveAllDirectories()
{
	ms_directories.RemoveAll();
}

const TCHAR* U2FileMgr::GetPath(const TCHAR* directory, const TCHAR* filename)
{
	size_t dirLength = _tcslen(directory);
	size_t fileLength = _tcslen(filename);

	if(dirLength + fileLength + 1 <= MAX_FILE_PATH)
	{
		_tcscpy_s(ms_path, MAX_FILE_PATH, directory);
		_tcscat_s(ms_path, MAX_FILE_PATH, filename);
		return ms_path;
	}

	return 0;

}

const TCHAR* U2FileMgr::GetFullPath(const TCHAR* file)
{
	_tfullpath(ms_path, file, MAX_FILE_PATH);

	return ms_path;

}


//-------------------------------------------------------------------------------------------------
U2WinFile* U2FileMgr::CreateNewFile() const
{
	U2WinFile* pNew = U2_NEW U2WinFile;
	U2ASSERT(pNew != 0);
	return pNew;
}


bool U2FileMgr::FileExists(const U2DynString& pathname) const
{
	U2ASSERT(!pathname.IsEmpty());
	bool res = false;
	U2WinFile* pNew = CreateNewFile();
	if(pNew->Exists(pathname))
	{
		res = true;
	}	
	return res;	
}


