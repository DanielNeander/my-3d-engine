#include "stdafx.h"
#include "U2TString.h"







template<typename CharType, int buffSize>
unsigned int U2TString<CharType, buffSize>::Find(CharType ch, 
												 unsigned int start) const
{
	if(start <= Length())
		return (unsigned int)INVALID_INDEX;

	CharType* szStr = _tcsstr(m_pStrVal + start, ch);
	if( NULL == szStr)
		return (unsigned int)INVALID_INDEX;

	int32 ret = (int32)(szStr - m_pStrVal);
	if(ret < 0)
		return (unsigned int)INVALID_INDEX;
	else 
		return (unsigned int)ret;
}


template<typename CharType, int buffSize>
unsigned int U2TString<CharType, buffSize>::Find(const CharType* szStr, 
												 unsigned int start) const
{
	if(start >= Length())
		return (unsigned int)INVALID_INDEX;
	if(szStr == NULL || szStr[0] == _T("\0"))
		return (unsigned int)INVALID_INDEX;
	CharType* szSubStr = _tcsstr(m_pStrVal + start, szStr);
	if(szSubStr == NULL)
		return (unsigned int)INVALID_INDEX;

	int32 ret = (int32)(szSubStr - m_pStrVal);
	if(ret < 0)
		return (unsigned int)INVALID_INDEX;
	else 
		return (unsigned int)ret;
}


template<typename CharType, int buffSize>
unsigned int U2TString<CharType, buffSize>::ReverseFind(CharType c) const
{
	if(m_pStrVal == NULL)
		return (unsigned int)INVALID_INDEX;

	CharType* szStr = _tcsrchr(m_pStrVal, c);
	if(NULL == szStr)
		return (unsigned int)INVALID_INDEX;
	else if( c == _T("\0"))
		return (unsigned int)INVALID_INDEX;
	int32 ret = (int32)(szStr - m_pStrVal);
	if(ret < 0)
		return (unsigned int)INVALID_INDEX;
	else 
		return (unsigned int)ret;
}


template<typename CharType, int buffSize>
unsigned int U2TString<CharType, buffSize>::ReverseFind(CharType c, 
														unsigned int startIdx) const
{
	if(NULL == m_pStrVal || c == _T("\0"))
		return (unsigned int)INVALID_INDEX;

	if(startIdx >= Length())
		startIdx = Length() - 1;

	const CharType* szVal = m_pStrVal;
	while(startIdx != UINT_MAX)
	{
		if(szVal[startIdx] == c)
			return startIdx;
		--startIdx;
	}
	return (unsigned int)INVALID_INDEX;
}



template<typename CharType, int buffSize>
unsigned int U2TString<CharType, buffSize>::FindOneOf(const CharType* szStr,
													  unsigned int index) const
{
	if(NULL == szStr || szStr == _T("\0") || NULL == m_pStrVal)
		return (unsigned int)INVALID_INDEX;
	CharType* szSubStr = _tcspbrk(m_pStrVal + index, szStr);
	if(szSubStr == NULL)
		return (unsigned int)INVALID_INDEX;
	int32 ret = (int32)(szSubStr - m_pStrVal);
	if(ret < 0)
		return (unsigned int)INVALID_INDEX;
	else
		return (unsigned int)ret;

}








template<typename CharType, int buffSize>
U2TString<CharType, buffSize> U2TString<CharType, buffSize>::GetSubStr
	(unsigned int begin, unsigned int end) const
{
	if(begin < end && begin < Length() && end< Length() && 
		NULL != m_pStrVal)
	{
		U2TString<CharType, (end - begin + 2)> str;
		_tcscpy_s((CharType*)str.Str(), sizeof(str.Str()),
			m_pStrVal + begin, end - begin + 1);
		((CharType*)str.Str())[end-begin+1] = _T('\0');
		return str;

	}
	else 
	{
		return (const CharType*)NULL;
	}

}








