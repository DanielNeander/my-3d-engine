#include <Src/U2LibPch.h>
#include "U2File.h"


const int U2File::ms_iSeekSet = SEEK_SET;
const int U2File::ms_iSeekCur = SEEK_CUR;
const int U2File::ms_iSeekEnd = SEEK_END;


U2File::U2File()
{
	m_pBuff = 0;
	m_pFile = 0;
}


U2File::U2File(const TCHAR *szFilename, U2File::FileMode eMode, 
			   unsigned int uBuffSize)
{
	U2ASSERT(eMode == READ_ONLY || eMode == WRITE_ONLY || eMode == APPEND_ONLY);

	m_eMode = eMode;
	const TCHAR* szMode;

	if(m_eMode == READ_ONLY)
		szMode = _T("rb");
	else 
		szMode = (m_eMode == WRITE_ONLY) ? _T("wb") : _T("ab");

	m_bGood = (_tfopen_s(&m_pFile, szFilename, szMode) == 0 && m_pFile != NULL);

	m_uBuffAllocSize = uBuffSize;
	m_uPos = m_uBuffReadSize = 0;

	if(m_bGood && uBuffSize > 0)
	{
		m_pBuff = U2_ALLOC(TCHAR, m_uBuffAllocSize);
		U2ASSERT(m_pBuff != NULL);
	}
	else 
		m_pBuff = 0;
}


U2File::~U2File()
{
	if(m_bGood && m_pFile)
	{
		Flush();
		fclose(m_pFile);
	}

	U2_FREE(m_pBuff);
	m_pBuff = NULL;
}


U2File::operator bool() const
{
	return m_bGood;
}


U2File* U2File::GetFile(const TCHAR* szName, FileMode eMode, 
					   uint32 uBuffSize)
{
	return U2_NEW U2File(szName, eMode, uBuffSize);
}


bool U2File::Access(const TCHAR* szName, FileMode eMode)
{
	U2File file(szName, eMode, 0);
	return file.m_bGood;
}


uint32 U2File::GetfileSize() const 
{
	int curr = ftell(m_pFile);
	if(curr < 0)
		return 0;
	fseek(m_pFile, 0, SEEK_END);
	int size = ftell(m_pFile);
	fseek(m_pFile, curr, SEEK_SET);
	if(size < 0)
		return 0;
	return (uint32)size;
}

bool U2File::CreateDir(const TCHAR *szDirname)
{
	bool bCreated = ::CreateDirectory(szDirname, NULL) != 0;

	return bCreated;
}

uint32 U2File::DiskWrite(const void* pvBuff, uint32 uBytes)
{
	return fwrite(pvBuff, 1, uBytes, m_pFile);
}
uint32 U2File::DiskRead(void* pvBuff, uint32 uBytes)
{
	return fread(pvBuff, 1, uBytes, m_pFile);
}


bool U2File::Flush()
{
	U2ASSERT(m_bGood);

	if(m_eMode == READ_ONLY)
		m_uBuffReadSize = 0;
	else 
	{
		if(m_uPos > 0)
		{
			if(DiskWrite(m_pBuff, m_uPos) != m_uPos)
			{
				m_bGood = false;
				return false;
			}
		}
	}
	m_uPos = 0;
	return true;
}


bool U2File::IsDirExists(const TCHAR* szDirname)
{
	DWORD att = GetFileAttributes(szDirname);
	if(att == -1)
		return false;
	return (att & FILE_ATTRIBUTE_DIRECTORY) != 0;	
}


void U2File::Seek(int numBytes)
{
	Seek(numBytes, ms_iSeekCur);
}

void U2File::Seek(int offset, int seek)
{
	U2ASSERT(seek == ms_iSeekSet || seek == ms_iSeekCur 
		|| seek == ms_iSeekEnd);
	U2ASSERT(m_eMode != APPEND_ONLY);

	if(m_bGood)
	{
		uint32 uNewPos = (int)m_uCurrStreamPos + offset;

		if(offset == ms_iSeekCur)
		{
			int iNewPos = (int)m_uPos + offset;
			if(iNewPos >= 0 && iNewPos < (int)m_uBuffReadSize)
			{
				m_uPos = iNewPos;
				m_uCurrStreamPos = (int)m_uCurrStreamPos + offset;
				return;
			}
			if(U2File::READ_ONLY == m_eMode)
				offset -= (m_uBuffReadSize - m_uPos);
		}
		Flush();

		m_bGood = (fseek(m_pFile, offset, seek) == 0);
		if(m_bGood)
		{
			m_uCurrStreamPos = ftell(m_pFile);

#ifdef _DEBUG 
			if(seek == ms_iSeekCur)
			{
				U2ASSERT(uNewPos == m_uCurrStreamPos);
			}
			else if(seek == ms_iSeekSet)
			{
				U2ASSERT(m_uPos == offset);		
			}
#endif
		}
	}
}


uint32 U2File::ReadEndianDatas(void * pvBuffer, uint32 uTotalBytes, 
					   uint32* puComponentSizes, uint32 uNumComponents)
{
	uint32 uBytesRead = ReadNoSwapBytes(this, pvBuffer, uTotalBytes, 
		puComponentSizes, uNumComponents);
	m_uCurrStreamPos += uBytesRead;
	return uBytesRead;
}


uint32 U2File::WriteEndianDatas(const void * pvBuffer, uint32 uTotalBytes, 
						uint32* puComponentSizes, uint32 uNumComponents)
{
	uint32 uBytesWritten = WriteNoSwapBytes(this, pvBuffer, uTotalBytes, 
		puComponentSizes, uNumComponents);
	m_uCurrStreamPos += uBytesWritten;
	return uBytesWritten;
}


uint32 U2File::ReadNoSwapBytes(U2StreamBase *pThis, 
							   void* pvBuffer, uint32 uBytes, 
							  uint32* puComponentSizes, uint32 uNumComponents)
{
	return ((U2File*)pThis)->FileRead(pvBuffer, uBytes);
}


uint32 U2File::WriteNoSwapBytes(U2StreamBase *pThis,
								const void* pvBuffer, uint32 uBytes, 
							   uint32* puComponentSizes, uint32 uNumComponents)
{
	return ((U2File*)pThis)->FileWrite(pvBuffer, uBytes);
}


unsigned int U2File::FileRead(void* pvBuffer, unsigned int uBytes)
{
	U2ASSERT(m_eMode == READ_ONLY);
	if(m_bGood)
	{
		uint32 uAvailBuffBytes, uRead;
		uRead = 0;
		uAvailBuffBytes = m_uBuffReadSize - m_uPos;
		if(uBytes > uAvailBuffBytes)
		{
			if(uAvailBuffBytes > 0)
			{
				memcpy(pvBuffer, &m_pBuff[m_uPos], uAvailBuffBytes);
				pvBuffer = &(((TCHAR*)pvBuffer)[uAvailBuffBytes]);
				uBytes -= uAvailBuffBytes;
				uRead = uAvailBuffBytes;
			}
			Flush();

			if(uBytes > m_uBuffAllocSize)
				return uRead + DiskRead(pvBuffer, uBytes);
			else 
			{
				m_uBuffReadSize = DiskRead(m_pBuff, m_uBuffAllocSize);
				if(m_uBuffReadSize < uBytes)
					uBytes = m_uBuffReadSize;
			}

		}

		memcpy(pvBuffer, &m_pBuff[m_uPos], uBytes);
		m_uPos += uBytes;
		return uRead + uBytes;
	}
	else 
		return 0;
}


unsigned int U2File::FileWrite(const void* pvBuffer, unsigned int uBytes)
{
	U2ASSERT(m_eMode != READ_ONLY);
	U2ASSERT(uBytes != 0);

	if(m_bGood)
	{
		uint32 uAvailBuffBytes, uWrite;

		uWrite = 0;
		uAvailBuffBytes = m_uBuffAllocSize - m_uPos;
		if(uBytes > uAvailBuffBytes)
		{
			if(uAvailBuffBytes > 0)
			{
				memcpy(&m_pBuff[m_uPos], pvBuffer, uAvailBuffBytes);
				pvBuffer = &(((TCHAR*)pvBuffer)[uAvailBuffBytes]);
				uBytes -= uAvailBuffBytes;
				uWrite = uAvailBuffBytes;
				m_uPos = m_uBuffAllocSize;
			}

			if(!Flush())
				return 0;

			if(uBytes >= m_uBuffAllocSize)
				return uWrite + DiskWrite(pvBuffer, uBytes);
		}

		memcpy(&m_pBuff[m_uPos], pvBuffer, uBytes);
		m_uPos += uBytes;
		return uWrite + uBytes;
	}
	else 
		return 0;
}
