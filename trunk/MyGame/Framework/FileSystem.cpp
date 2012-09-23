#include "../Havok/Havok.h"
#include "FileSystem.h"
#include <Common/Base/System/Io/Reader/hkStreamReader.h>
#include <Common/Base/System/Io/Writer/hkStreamWriter.h>

namespace
{
	class NullStreamReader : public hkStreamReader
	{
	public:
		NullStreamReader() { }
		hkBool isOk() const { return false; }
		virtual int read(void* buf, int nbytes) { return 0; }
	};

	class NullStreamWriter : public hkStreamWriter
	{
	public:
		NullStreamWriter() { }
		hkBool isOk() const { return false; }
		virtual int write(const void* buf, int nbytes) { return 0;}
	};

	class hkStdioStreamWriter : public hkStreamWriter
	{
	public:

		hkStdioStreamWriter(const char* fname)
			
		{
			fopen_s(&m_fhandle, fname, "rb");
		}

		virtual ~hkStdioStreamWriter()
		{
			if(m_fhandle) { fclose(m_fhandle); }
		}

		virtual int write( const void* buf, int nbytes)
		{
			HK_ASSERT(0x0, m_fhandle != HK_NULL);
			return fwrite(buf, 1, nbytes, m_fhandle );
		}

		virtual hkBool isOk() const
		{
			return m_fhandle != HK_NULL && !feof(m_fhandle);
		}

		// 
		// This simple example does not implement seek/tell
		//
	protected:

		FILE* m_fhandle;
	};
}

GameFileSystem::GameFileSystem()
{
	hkFileSystem* parent = &hkFileSystem::getInstance();
	parent->addReference();
	m_parent = parent;
	addSearchPath("./");
	//addSearchPath(hkDemoDatabase::getInstance().getPrefix().cString());
	m_locations.pushBack(hkString::strDup(""));
}

void GameFileSystem::addSearchPath(const char* p, hkBool32 atFront)
{
	int pos = atFront ? 0 : m_locations.getSize();
	m_locations.insertAt(pos, hkString::strDup(p));
}

GameFileSystem::~GameFileSystem()
{
	m_parent->removeReference();
	for( int i = 0; i < m_locations.getSize(); ++i )
	{
		hkDeallocate( m_locations[i] );
	}
}

const char* HK_CALL GameFileSystem::removeBasePath( const char* pathIn, hkArray<char>& buffer )
{
	hkString path( pathIn );
	path.makeLowerCase();
	// strip everything up to the demo root folder
	hkString root("/demos/");
	root.makeLowerCase();

	const char* str = path.cString();
	const char* start = hkString::strStr(str, root.cString());
	if ( start != HK_NULL )
	{
		int pos = static_cast<int>(start - str) + root.getLength();
		buffer.setSize( path.getLength() - pos + 1 );
		// copy the part of the string that follows the demo prefix
		hkString::memCpy( buffer.begin(), pathIn+pos, buffer.getSize()-1 );
		buffer[ buffer.getSize()-1 ] = 0; // null terminate
		return buffer.begin();
	}
	else 
	{
		// nothing to do
		return pathIn;
	}
}

hkStreamWriter* GameFileSystem::openWriter(const char* name)
{
	for(int i = 0; i < m_locations.getSize(); ++i)
	{
		hkString p = m_locations[i];
		p += name;
		hkStreamWriter* sb = m_parent->openWriter(p.cString());
		if(sb->isOk() || i == (m_locations.getSize()-1))
		{
			return sb;
		}
		else
		{
			sb->removeReference();
		}
	}
	return new NullStreamWriter();
}

hkStreamReader* GameFileSystem::openReader(const char* name)
{
	for(int i = 0; i < m_locations.getSize(); ++i)
	{
		hkString p = m_locations[i];
		p += name;

		hkStreamReader* sb = m_parent->openReader(p.cString());
		if(sb->isOk() || i == (m_locations.getSize()-1))
		{
			return sb;
		}
		else
		{
			sb->removeReference();
		}
	}
	return new NullStreamReader();
}

hkResult GameFileSystem::listDirectory(const char* basePath, DirectoryListing& listingOut)
{
	for(int i = 0; i < m_locations.getSize(); ++i)
	{
		hkString p = m_locations[i];
		p += basePath;
		if( m_parent->listDirectory(p.cString(), listingOut ) == HK_SUCCESS )
		{
			return HK_SUCCESS;
		}
	}
	return HK_FAILURE;
}
