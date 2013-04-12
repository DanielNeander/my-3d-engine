//#include <U2_3D/Src/U23DLibPCH.h>
//#include <stdafx.h>
#include <assert.h>
#include <tchar.h>
#include "U2WinFile.h"

U2WinFile::U2WinFile()
:lineNumber(0),
isOpen(false),
handle(0)
{

}

U2WinFile::~U2WinFile()
{
	if(IsOpen())
	{
		Close();
	}
}

//------------------------------------------------------------------------------
/**
Check if file exists physically on disk by opening it in read-only mode.
Close file if it was opened.

- 05-Jan-05   floh    Bugfix: missing GENERIC_READ access mode didn't work in Win98
- 12-Oct-05   floh    access move back to 0, Win98 is no longer relevant
*/
bool U2WinFile::Exists(const std::string& fileName) const 
{
	HANDLE fh = CreateFile(fileName.c_str(),       // filename
		0,                    // don't actually open the file, just check for existence
		FILE_SHARE_READ,      // share mode
		0,                    // security flags
		OPEN_EXISTING,        // what to do if file doesn't exist
		FILE_ATTRIBUTE_NORMAL,  // flags'n'attributes
		0);                   // template file
	if (fh != INVALID_HANDLE_VALUE)
	{
		CloseHandle(fh);
		return true;
	}
	else
	{
		return false;
	}	
}

//------------------------------------------------------------------------------
/**
opens the specified file

@param fileName     the name of the file to open
@param accessMode   the access mode ("(r|w|a)[+]")
@return             success

history:
- 30-Jan-2002   peter   created
- 11-Feb-2002   floh    Linux stuff
- 12-Oct-2005   floh    fixed read mode to also open a file which is
already open by another application for writing
*/
bool U2WinFile::Open(const std::string &fileName, const std::string &accessMode)
{
	assert(!IsOpen());
	lineNumber = 0;

	DWORD access = 0;
	DWORD disposition = 0;
	DWORD shareMode = 0;
	if (accessMode.find(_T("rR")))
	{
		access |= GENERIC_READ;
	}
	if (accessMode.find(_T("wW")))
	{
		access |= GENERIC_WRITE;
	}
	if (access & GENERIC_WRITE)
	{
		disposition = CREATE_ALWAYS;
		shareMode = FILE_SHARE_READ;    // allow reading the file in write mode
	}
	else
	{
		disposition = OPEN_EXISTING;
		shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE; // allow read/write in read mode
	}

	this->handle = CreateFile(fileName.c_str(),    // filename
		access,               // access mode
		shareMode,            // share mode
		0,                    // security flags
		disposition,          // what to do if file doesn't exist
		FILE_ATTRIBUTE_NORMAL,   // flags'n'attributes
		0);                   // template file

//	ErrorExit(_T("CreateFile"));

	if (this->handle == INVALID_HANDLE_VALUE)
	{
		this->handle = 0;
		return false;
	}

	isOpen = true;
	return true;
}

//------------------------------------------------------------------------------
/**
closes the file

history:
- 30-Jan-2002   peter   created
- 11-Feb-2002   floh    Linux stuff
*/
void U2WinFile::Close()
{
	assert(IsOpen());

	if(handle)
	{
		CloseHandle(handle);
		handle = 0;
	}

	isOpen = false;
}

//------------------------------------------------------------------------------
/**
writes a number of bytes to the file

@param buffer       buffer with data
@param numBytes     number of bytes to write
@return             number of bytes written

history:
- 30-Jan-2002   peter    created
- 11-Feb-2002   floh    Linux stuff
*/
int
U2WinFile::Write(const void* buffer, int numBytes)
{
	assert(this->IsOpen());	

	DWORD written;
	WriteFile(this->handle, buffer, numBytes, &written, NULL);
	return written;
}


//------------------------------------------------------------------------------
/**
reads a number of bytes from the file

@param buffer          buffer for data
@param numBytes        number of bytes to read
@return                number of bytes read

history:
- 30-Jan-2002   peter    created
*/
int
U2WinFile::Read(void* buffer, int numBytes)
{
	assert(this->IsOpen());

	DWORD read;
	ReadFile(this->handle, buffer, numBytes, &read, NULL);
	return read;
}

//------------------------------------------------------------------------------
/**
gets current position of file pointer

@return          position of pointer

history:
- 30-Jan-2002   peter    created
*/
int
U2WinFile::Tell() const
{
	assert(this->IsOpen());

	return SetFilePointer(this->handle, 0, NULL, FILE_CURRENT);
}


//------------------------------------------------------------------------------
/**
sets the file pointer to given absolute or relative position

@param byteOffset        the offset
@param origin            position from which to count
@return                  success

history:
- 30-Jan-2002   peter    created
*/
bool
U2WinFile::Seek(int byteOffset, nSeekType origin)
{
	assert(this->IsOpen());


	DWORD method;
	switch (origin)
	{
	case CURRENT:
		method = FILE_CURRENT;
		break;
	case START:
		method = FILE_BEGIN;
		break;
	case END:
		method = FILE_END;
		break;
	}

	DWORD ret = SetFilePointer(this->handle, (LONG)byteOffset, NULL, method);
	return (ret != 0xffffffff);
}


//------------------------------------------------------------------------------
/**
*/
bool
U2WinFile::Eof() const
{
	assert(this->IsOpen());

	DWORD fpos = SetFilePointer(this->handle, 0, NULL, FILE_CURRENT);
	DWORD size = GetFileSize(this->handle, NULL);

	// NOTE: THE '>=' IS NOT A BUG!!!
	return fpos >= size;
}

//------------------------------------------------------------------------------
/**
Returns size of file in bytes.

@return     byte-size of file
*/
int
U2WinFile::GetSize() const
{
	assert(this->IsOpen());

	return GetFileSize(this->handle, NULL);
}

//------------------------------------------------------------------------------
/**
Returns time of last write access. The file must be opened in "read" mode
before this function can be called!
*/
U2WinFileTime U2WinFile::GetLastWriteTime() const
{
	assert(this->IsOpen());

	U2WinFileTime fileTime;
	GetFileTime(this->handle, NULL, NULL, &(fileTime.time));
	return fileTime;
}

//------------------------------------------------------------------------------
/**
writes a string to the file

@param buffer        the string to write
@return              success

history:
- 30-Jan-02   peter   created
- 29-Jan-03   floh    the method suddenly wrote a newLine. WRONG!
*/
bool U2WinFile::PutS(const std::string& buffer)
{
	assert(this->IsOpen());

	int len = int(buffer.length());
	int written = this->Write(buffer.c_str(), len);
	if (written != len)
	{
		return false;
	}
	this->lineNumber++;
	return true;
}

//------------------------------------------------------------------------------
/**
reads a string from the file up to and including the first newline character
or up to the end of the buffer

@param buf            buffer for string
@param bufSize        maximum number of chars to read
@return               success (false if eof is reached)

history:
- 30-Jan-2002   peter    created
- 28-Sep-2005   floh     complete rewrite, there were error if bufSize
was below a line length
*/
bool U2WinFile::GetS(char* buf, int bufSize)
{
	assert(this->IsOpen());
	assert(buf);
	assert(bufSize > 1);

	if (this->Eof())
	{
		return false;
	}

	// make room for final terminating 0
	bufSize--;

	// store start filepointer position
	int startPos = this->Tell();

	// read file contents in chunks of 64 bytes, not char by char
	int chunkSize = 256;
	if (chunkSize > bufSize)
	{
		chunkSize = bufSize;
	}
	char* readPos = buf;
	int curIndex;
	for (curIndex = 0; curIndex < bufSize; curIndex++)
	{
		// read next chunk of data?
		if (0 == (curIndex % chunkSize))
		{
			// if we reached end-of-file before, break out
			if (this->Eof())
			{
				break;
			}

			// now, read the next chunk of data
			int readSize = chunkSize;
			if ((curIndex + readSize) >= bufSize)
			{
				readSize = bufSize - curIndex;
			}
			int bytesRead = this->Read(readPos, readSize);
			if (bytesRead != readSize)
			{
				// end of file reached
				readPos[bytesRead] = 0;
			}
			readPos += bytesRead;
		}

		// check for newline
		if (buf[curIndex] == '\n')
		{
			// reset file pointer to position after new-line
			this->Seek(startPos + curIndex + 1, START);
			break;
		}
	}
	buf[curIndex] = 0;
	return true;
}

//------------------------------------------------------------------------------
/**
Append the contents of another file to this file. This and the 'other' file
must both be open! Returns number of bytes copied.
Warning: current implementation reads the complete source file
into a ram buffer.

@return     number of bytes appended
*/
int U2WinFile::AppendFile(U2WinFile* other)
{
	assert(other);

	int numBytes = other->GetSize();
	if (numBytes == 0)
	{
		// nothing to do
		return 0;
	}

	// allocate temp buffer and read bytes
	char* buffer = (char*)malloc(numBytes);
	assert(buffer);
	int numBytesRead = other->Read(buffer, numBytes);
	assert(numBytesRead == numBytes);

	// write to this file
	int numBytesWritten = this->Write(buffer, numBytes);
	assert(numBytesWritten == numBytes);

	// cleanup
	free(buffer);
	buffer = NULL;
	return numBytes;
}
