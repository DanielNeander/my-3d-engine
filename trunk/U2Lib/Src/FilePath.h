#pragma  once

#include "String.h"
#include <vector>


namespace Main {;


class DLL_LIB FilePath  
{

public:

	static void Initialize();
	static void Terminate();
	static int	GetNumDirectory();
	static const TCHAR* GetDirectory(int i);
	static bool InsertDirectory(const TCHAR* directory);
	static bool RemoveDirectory(const TCHAR* directory);
	static void RemoveAllDirectories();

	static const TCHAR* GetPath(const TCHAR* directory, const TCHAR* filename);
	static const TCHAR* GetFullPath(const TCHAR* file);

private:
	enum { MAX_FILE_PATH = 1024};

	static TCHAR ms_path[MAX_FILE_PATH];
	static std::vector<String >*	ms_directories;

};




} // namespace MAin