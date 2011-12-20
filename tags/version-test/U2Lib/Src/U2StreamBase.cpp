#include <Src/U2LibPch.h>
#include "U2StreamBase.h"

U2StreamBase::U2StreamBase()
{
	m_uCurrStreamPos = 0;
}


U2StreamBase::~U2StreamBase()
{

}


uint32 U2StreamBase::GetLine(TCHAR *pBuffer, uint32 uiMaxBytes)
{
	unsigned int uiBytesRead = 0;
	unsigned int i = 0;

	U2ASSERT(uiMaxBytes > 0);

	while (i + 1 < uiMaxBytes)
	{
		TCHAR c;
		unsigned int uiRead = Read(&c, 1);
		uiBytesRead += uiRead;

		if (uiRead != 1 || c == _T('\n'))
			break;

		if (c != _T('\r'))
			pBuffer[i++] = c;
	}

	pBuffer[i] = 0;

	return uiBytesRead;

}


uint32 U2StreamBase::PutString(const TCHAR *pBuffer)
{
	unsigned int i = 0;

	while (*pBuffer != 0)
	{
		if (Write(pBuffer++, 1) == 1)
		{
			i++;
		}
		else
		{
			break;
		}
	}

	return i;
}

uint32 U2StreamBase::GetPos() const
{
	return m_uCurrStreamPos;
}


uint32 U2StreamBase::Read(void* pvBuffer, uint32 uBytes)
{
	uint32 uSize = 1;
	uint32 uBytesToRead = ReadEndianDatas(pvBuffer, uBytes, &uSize, 1);
	return uBytesToRead;
}

uint32 U2StreamBase::Write(const void* pvBuffer, uint32 uBytes)
{
	uint32 uSize = 1;
	uint32 uBytesToWritten = WriteEndianDatas(pvBuffer, uBytes, &uSize, 1);
	return uBytesToWritten;
}



