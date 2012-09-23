#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <Common/Base/System/Io/FileSystem/hkFileSystem.h>

class GameFileSystem : public hkFileSystem
{
public:

	GameFileSystem();
	~GameFileSystem();
		
	static const char* HK_CALL removeBasePath( const char* pathIn, hkArray<char>& buffer );

	void addSearchPath(const char* p, hkBool32 atFront=false);

	virtual hkStreamWriter* openWriter(const char* name);
	virtual hkStreamReader* openReader(const char* name);
	virtual hkResult listDirectory(const char* basePath, DirectoryListing& listingOut);

	hkFileSystem* m_parent;
	hkArray<char*> m_locations;
};



#endif