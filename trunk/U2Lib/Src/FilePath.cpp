#include "Path.h"
#include "FilePath.h"
#include <windows.h>


#undef RemoveDirectory

namespace Main {;

TCHAR FilePath::ms_path[FilePath::MAX_FILE_PATH] = _T("");
std::vector<String>* FilePath::ms_directories = 0;


void
FilePath::Initialize()
{
	
	TCHAR currPath[256];
	::GetCurrentDirectory(256, currPath);

	ms_directories = new std::vector<String>;
}

void
FilePath::Terminate()
{
	delete ms_directories;
	ms_directories = 0;
}

int
FilePath::GetNumDirectory()
{
	return ms_directories->size();
}

const TCHAR* 
FilePath::GetDirectory(int i)
{
	if(0 <= i && i < (int)ms_directories->size())
		return (*ms_directories)[i].c_str();
	
	return 0;
}


bool
FilePath::InsertDirectory(const TCHAR* directory)
{
	String dirPlusSlash = String(directory) + String(_T("/"));
	for(int i = 0; i < (int)ms_directories->size() ; ++i)
	{
		if(dirPlusSlash == (*ms_directories)[i])
			return false;
	}

	ms_directories->push_back(dirPlusSlash);
	return true;
}

bool
 FilePath::RemoveDirectory(const TCHAR* directory)
{
	String dirPlusSlash = String(directory) + String(_T("/"));
	std::vector<String>::iterator iter = ms_directories->begin();
	for( ;iter != ms_directories->end() ; ++iter)
	{
		if(dirPlusSlash == *iter)
		{
			ms_directories->erase(iter);
			return true;
		}
	}

	return false;

}

void
FilePath::RemoveAllDirectories()
{
	ms_directories->clear();
}

const TCHAR* 
FilePath::GetPath(const TCHAR* directory, const TCHAR* filename)
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

const TCHAR* 
FilePath::GetFullPath(const TCHAR* file)
{
	FULL_PATH(ms_path, file, MAX_FILE_PATH);

	return ms_path;

}


}	// namespace Main