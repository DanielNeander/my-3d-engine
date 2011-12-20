/************************************************************************
module	:	U2WinFile
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_WINFILE_H
#define U2_WINFILE_H

//------------------------------------------------------------------------------
/**
@class U2WinFile
@ingroup File

@brief Wrapper class for accessing file system files.

provides functions for reading and writing files

(C) 2002 RadonLabs GmbH
*/
#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winbase.h>
#endif


#include <U2Lib/src/U2RefObject.h>
#include <U2Lib/src/U2SmartPtr.h>
#include <U2Lib/Src/U2TString.h>

#include "U2FileTime_Win32.h"

class U2MISC U2WinFile : public U2RefObject
{
public:
	/// start point for seeking in file
	enum nSeekType
	{
		CURRENT,
		START,
		END,
	};
	/// NOTE: constructor is private because only U2WinFileServer2 may create objects
	U2WinFile();
	/// destructor
	virtual ~U2WinFile();

	/// does the file physically exist on disk?
	virtual bool Exists(const U2DynString& fileName) const;
	/// opens a file
	virtual bool Open(const U2DynString& fileName, const U2DynString& accessMode);
	/// closes the file
	virtual void Close();
	/// writes some bytes to the file
	virtual int Write(const void* buffer, int numBytes);
	/// reads some bytes from the file
	virtual int Read(void* buffer, int numBytes);
	/// gets actual position in file
	virtual int Tell() const;
	/// sets new position in file
	virtual bool Seek(int byteOffset, nSeekType origin);
	/// is the file at the end
	virtual bool Eof() const;
	/// get size of file in bytes
	virtual int GetSize() const;
	/// get the last write time
	virtual U2WinFileTime GetLastWriteTime() const;
	/// writes a string to the file
	bool PutS(const U2String& buffer);	// U2DynString 사용시 mem is full 메시지 에러.
	/// reads a string from the file
	bool GetS(char* buffer, int numChars);
	/// get current line number (incremented by PutS() and GetS())
	int GetLineNumber() const;
	/// determines whether the file is opened
	bool IsOpen() const;
	/// append one file to another file
	virtual int AppendFile(U2WinFile* other);
	/// write a 32bit int to the file
	int PutInt(int val);
	/// write a 16bit int to the file
	int PutShort(short val);
	/// write a 8bit int to the file
	int PutChar(char val);
	/// write a float to the file
	int PutFloat(float val);
	/// write a double to the file
	int PutDouble(double val);
	/// read a 32 bit int from the file
	int GetInt();
	/// read a signed 16 bit int from the file
	short GetShort();
	/// read an unsigned 16 bit int from the file
	uint16 GetUShort();
	/// read a 8 bit int from the file
	char GetChar();
	/// read a float from the file
	float GetFloat();
	/// read a double from the file
	double GetDouble();

protected:	

	

	uint16 lineNumber;
	bool isOpen;

	// win32 file handle
	HANDLE handle;

};

typedef U2SmartPtr<U2WinFile> U2WinFilePtr;

#include "U2WinFile.inl"

#endif