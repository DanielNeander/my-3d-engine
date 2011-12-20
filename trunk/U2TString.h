/************************************************************************
module	:	U2String
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#ifndef U2_TSTRING_H
#define U2_TSTRING_H

#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <string>

template<typename CharType, int buffSize>
class U2TString //: public U2MemObj
{

public: 
	enum
	{
		INVALID_INDEX = (unsigned int) -1
	};	
	
	U2TString(const CharType *pStr =_T(""), bool alloc = true ) ;
	U2TString(const U2TString& src);
	~U2TString();
	
	operator const CharType*() const;
	U2TString&	operator=	(const U2TString& src);
	int			operator==	(const U2TString& src) const;
	int			operator!=	(const U2TString& src) const;
	int			operator>	(const U2TString& src) const;
	int			operator<	(const U2TString& src) const;
	
	
	const CharType*		Str() const;
	bool				IsEmpty() const;
	unsigned int		Length() const;
	CharType			GetAt(unsigned int ui) const;
	void				SetAt(unsigned int ui, CharType c);
	bool				Contains(const CharType* szStr) const;
	U2TString			GetSubStr(unsigned int begin, unsigned int end) const;
	unsigned int		Find(CharType ch, unsigned int start) const;
	unsigned int		Find(const CharType* szStr, unsigned int start) const;
	unsigned int		ReverseFind(CharType ch) const;
	unsigned int		ReverseFind(CharType ch, unsigned int start) const;
	unsigned int		FindOneOf(const CharType* pcStr,unsigned int index) const;
	void				ToUpper();
	void				ToLower();
	void				Reverse();
	void				TrimLeft(CharType ch);
	void				TrimRight(CharType ch);
	void				Concatenate(const CharType* szStr);
	void				Concatenate(CharType ch);
	bool				Resize(unsigned int reSize);
	

/*	bool				ToBool(bool& b) const;
	bool				ToFloat(float& f) const;
	bool				ToInt(int32& i) const;
	bool				ToUnit(uint32 ui) const;*/	

	void				Format(const TCHAR* fmt, ...);
	void				FormatWithArgs(const TCHAR* fmt, va_list args);

	void				SetInt(int val);
	void				AppendInt(int i);


	

	friend	std::ostream& operator<<(std::ostream& os, const U2TString<CharType, buffSize>& str);
	
private:
	CharType	*m_pStrVal;				// heap alloc
	bool			m_bAlloc;			// heap alloc flag
	CharType	m_buffer[buffSize + 1];	// stack alloc
};


typedef U2TString<char, 0>			U2StringA;
typedef U2TString<char, 256>		U2StackStringA;
#define FAST_STRINGA(id, str)		U2StringA (id)((str), 0)

typedef U2TString<wchar_t, 0>		U2StringW;
typedef U2TString<wchar_t, 256>		U2StackStringW;
#define FAST_STRINGW(id, str)		U2StringW (id)((str), 0)

#ifdef UNICODE 
#define U2DynString		U2StringW
#define U2StackString	U2StackStringW
#define FAST_STRING(id, str)		U2StringW (id)((str), 0)	
#else 
#define U2DynString		U2StringA
#define U2StackString	U2StackStringA
#define FAST_STRING(id, str)		U2StringA (id)((str), 0)	
#endif


template<typename CharType, int buffSize>
inline 
const CharType* U2TString<CharType, buffSize>::Str() const
{
	return m_pStrVal;
}

template<typename CharType, int buffSize>
inline 
std::ostream& operator<<(std::ostream& os, const U2TString<CharType, buffSize>& src)
{
	return os << src.m_pStrVal;
}


template<typename CharType, int buffSize>
inline 
unsigned int U2TString<CharType, buffSize>::Length() const 
{
	//U2ASSERT(m_pStrVal);
	return _tcslen(m_pStrVal);
}


template<typename CharType, int buffSize>
inline 
bool U2TString<CharType, buffSize>::IsEmpty() const 
{
	return Length() != 0;
}


template<typename CharType, int buffSize>
inline 
U2TString<CharType, buffSize>::U2TString(const U2TString<CharType, buffSize>& src)
:m_pStrVal(m_buffer)
{
	if( buffSize == 0)
	{
		if( m_bAlloc = src.m_bAlloc)
		{
			m_pStrVal = new CharType[ _tcslen(src.m_pStrVal) + 1];
			_tcscpy(m_pStrVal, src.m_pStrVal);
		}
		else 
		
			m_pStrVal = src.m_pStrVal;
	}
	else 
	{
		m_buffer[buffSize] = 0;		
		_tcscpy_s(m_pStrVal, buffSize, src.m_pStrVal);
	}

}

template<typename CharType, int buffSize > 
inline 
U2TString<CharType, buffSize>::U2TString(const CharType *pStr, bool alloc ) 
:m_pStrVal(m_buffer)
{
	if(	!buffSize )
	{
		if(!(m_bAlloc = alloc))
		{
			m_pStrVal = (CharType*)pStr;
		}
		else 
		{
			int strLen = _tcslen(pStr);
			m_pStrVal = new CharType[ strLen + 1];
			_tcscpy(m_pStrVal, pStr);
		}

	}
	else 
	{
		// Make on stack
		m_buffer[buffSize] = 0;
		_tcscpy_s(m_pStrVal, buffSize, pStr);
	}

	// Debug
}

template<typename CharType, int buffSize>
inline 
U2TString<CharType, buffSize>::~U2TString()
{
	if( buffSize == 0 && m_bAlloc)
		delete m_pStrVal;
}

template<typename CharType, int buffSize>
inline 
U2TString<CharType, buffSize>::operator const CharType*() const
{
	return m_pStrVal;
}


template<typename CharType, int buffSize>
inline 
U2TString<CharType, buffSize>& 
U2TString<CharType, buffSize>::operator=(const U2TString<CharType, buffSize>& src)
{
	if(&src == this)
		return *this;

	if( !buffSize )
	{
		if(m_bAlloc)
		{
			delete [] m_pStrVal;
			m_pStrVal = new CharType[ _tcslen(src.m_pStrVal) + 1 ];
			_tcscpy(m_pStrVal, src.m_pStrVal);
		}
		else 
			m_pStrVal = src.m_pStrVal;

	}
	else 
	{
		m_buffer[buffSize] = 0;
		_tcscpy_s(m_pStrVal, buffSize, src.m_pStrVal);
	}

	return *this;
}




template<typename CharType, int buffSize>
inline 
int U2TString<CharType, buffSize>::operator==	(const U2TString& src) const
{
	return !_tcscmp(m_pStrVal, src.m_pStrVal);	
}


template<typename CharType, int buffSize>
inline 
int U2TString<CharType, buffSize>::operator!=	(const U2TString& src) const
{
	return _tcscmp(m_pStrVal, src.m_pStrVal);	
}


template<typename CharType, int buffSize>
inline 
int U2TString<CharType, buffSize>::operator>	(const U2TString& src) const
{
	return _tcscmp(m_pStrVal, src.m_pStrVal) > 0;	
}


template<typename CharType, int buffSize>
inline 
int U2TString<CharType, buffSize>::operator<	(const U2TString& src) const
{
	return _tcscmp(m_pStrVal, src.m_pStrVal) < 0;
}




template<typename CharType, int buffSize>
inline 
CharType U2TString<CharType, buffSize>::GetAt(unsigned int ui) const
{
	CharType ch;
	if(ui > Length())
		ch = _T('\0');
	else 
		ch = m_pStrVal[ui];
	return ch;
}

template<typename CharType, int buffSize>
inline 
void U2TString<CharType, buffSize>::SetAt(unsigned int ui, CharType ch)
{
	if(ui >= Length())
		return;
	m_pStrVal[ui] = ch;
}


template<typename CharType, int buffSize>
inline 
void U2TString<CharType, buffSize>::ToUpper()
{
	if( NULL == m_pStrVal)
		return;

	for(unsigned int ui = 0; ui < Length() ; ++ui)
	{
		CharType ch = m_pStrVal[ui];
		if((_T('a') <= ch) && (ch <= _T('z')))
			m_pStrVal[ui] -= _T('a') - _T('A');
	}
}


template<typename CharType, int buffSize>
inline 
void U2TString<CharType, buffSize>::ToLower()
{
	if( NULL == m_pStrVal)
		return;

	for(unsigned int ui = 0; ui < Length() ; ++ui)
	{
		CharType ch = m_pStrVal[ui];
		if((_T('A') <= ch) && (ch <= _T('Z')))
			m_pStrVal[ui] -= _T('A') - _T('a');
	}
}


template<typename CharType, int buffSize>
inline 
void U2TString<CharType, buffSize>::TrimLeft(CharType ch)
{
	if( _T('\0') == ch || NULL == m_pStrVal)
		return;

	unsigned int trimCnt = 0;
	bool bContinue = true;
	for(unsigned int ui = 0; ui < Length() && bContinue; ++ui)
	{
		if(m_pStrVal[ui] == ch)
			++trimCnt;
		else 
			bContinue = false;
	}

	if(trimCnt > 0)
	{
		memmove(m_pStrVal, m_pStrVal + trimCnt, Length() - trimCnt + 1);
	}
}


template<typename CharType, int buffSize>
inline 
void U2TString<CharType, buffSize>::TrimRight(CharType ch)
{
	if( ch == _T('\0') || NULL == m_pStrVal )
		return;

	unsigned int trimCnt = 0;
	bool bContinue = true;
	for(unsigned int i = Length() - 1; i >= 0 && bContinue; --i)
	{
		if(m_pStrVal[i] == ch)
			trimCnt++;
		else
			bContinue = false;
	}

	if(trimCnt > 0)
	{
		CharType* szStr = m_pStrVal + Length() - trimCnt;
		szStr[0] = _T('\0');
	}
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline
void U2TString<CharType, buffSize>::Format(const TCHAR* fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	FormatWithArgs(fmt, argList);
	va_end(argList);	
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline
void U2TString<CharType, buffSize>::FormatWithArgs(const TCHAR* fmt, va_list args)
{
	va_list argList;
	argList = args;

	size_t reqLen;

	reqLen = _vsctprintf (fmt, argList);

	reqLen++; // accunt for NULL termination
	va_end(argList);

	TCHAR* buff = new TCHAR[reqLen];

	_vsntprintf(buff, reqLen, fmt, args);

	*this = buff;

}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline
void U2TString<CharType, buffSize>::SetInt(int val)
{
	this->Format(_T("%d"), val);
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline
void U2TString<CharType, buffSize>::AppendInt(int val)
{
	U2DynString str;
	str.SetInt(val);
	Concatenate(str.Str());		
}



template<typename CharType, int buffSize>
inline 
void U2TString<CharType, buffSize>::Concatenate(const CharType* szStr)
{
	if(NULL == szStr)
		return;

	size_t len = _tcslen(szStr);

	if(len > 0)
	{
		int oldLen = Length();
		Resize(_tcslen(szStr));

		if(!buffSize)
		{				
			
			_tcscpy_s(m_pStrVal + oldLen , len + 1, szStr);
		}
		else 
			_tcscpy_s(m_pStrVal + oldLen, len + 1, szStr);
	}
}

template<typename CharType, int buffSize>
inline 
void U2TString<CharType, buffSize>::Concatenate(CharType ch)
{
	Resize(1);
	size_t len = Length();

	m_pStrVal[len] = ch;
	m_pStrVal[len + 1] = _T('\0');
}


template<typename CharType, int buffSize>
inline 
bool U2TString<CharType, buffSize>::Resize(unsigned int reSize)
{
	if(!buffSize)
	{
		size_t newStrLen = Length() + reSize;
		if(newStrLen > Length())
		{	

			
			CharType *newStrVal = new CharType[newStrLen + 1];
			memcpy_s(newStrVal, sizeof(newStrVal), m_pStrVal, Length());		
			newStrVal[newStrLen] = _T('\0');		
			delete m_pStrVal;
			m_pStrVal = newStrVal;
			return true;
		}
		else 
			return false;	
	}
	else  // stack 
	{
		return true;		
	}
}


template<typename CharType, int buffSize>
inline
bool
U2TString<CharType, buffSize>::Contains(const CharType* charSet) const
{
	assert(charSet);
	CharType* str = (CharType*)Str();
	CharType* ptr = _tcspbrk(str, charSet);
	return (0 != ptr);
}


#endif
