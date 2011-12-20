
//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
U2TString<CharType, buffSize>::U2TString(const U2TString<CharType, buffSize>& src)
:m_pStrVal(m_buffer)
{
	if( buffSize == 0)
	{	
		if(src == NULL)
		{
			m_pStrVal = NULL;
		}
		else 
		if( m_bAlloc = src.m_bAlloc)
		{
			m_pStrVal = U2_ALLOC(CharType, 
				sizeof(TCHAR) * GetBestBufferSize((unsigned int)_tcslen(src.m_pStrVal)+ 1));
			_tcscpy_s(m_pStrVal, _tcslen(src.m_pStrVal) + 1, src.m_pStrVal);
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

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize > 
inline 
U2TString<CharType, buffSize>::U2TString(const CharType *pStr, bool alloc ) 
:m_pStrVal(m_buffer)
{
	if(	!buffSize )
	{
		if(pStr == NULL)
		{
			m_pStrVal = NULL;
		}
		else 
		if(!(m_bAlloc = alloc))
		{
			m_pStrVal = (CharType*)pStr;
		}
		else 
		{
			int strLen = (int)_tcslen(pStr);			
			m_pStrVal = U2_ALLOC(CharType, sizeof(TCHAR) * GetBestBufferSize(strLen + 1));
			_tcscpy_s(m_pStrVal, strLen+1, pStr);
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

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
U2TString<CharType, buffSize>::~U2TString()
{
	if(0 == buffSize)
	{	
		if(m_bAlloc && m_pStrVal)
		{	
			U2_FREE(m_pStrVal);
			m_pStrVal = NULL;		
		}
	}
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline unsigned int U2TString<CharType, buffSize>::GetBestBufferSize(unsigned int uiReqSize)
{
	if (uiReqSize < 32)
		return 32;
	else if (uiReqSize < 64)
		return 64;
	else if (uiReqSize < 128)
		return 128;
	else if (uiReqSize < 255)
		return 255;
	else if (uiReqSize < 512)
		return 512;
	else if (uiReqSize < 1024)
		return 1024;
	else 
		return uiReqSize + 1;
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
U2TString<CharType, buffSize>::operator const CharType*() const
{
	return m_pStrVal;
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
U2TString<CharType, buffSize>& 
U2TString<CharType, buffSize>::operator=(const U2TString<CharType, buffSize>& src)
{
	if(&src == this)
		return *this;

	if( !buffSize )
	{
		if(src == NULL)
		{
			m_pStrVal = NULL;
		}
		else 
		if(m_bAlloc)
		{
			U2_FREE(m_pStrVal);	
			m_pStrVal = NULL;
			m_pStrVal = U2_ALLOC(CharType,  
				sizeof(TCHAR) * GetBestBufferSize((unsigned int)_tcslen(src.m_pStrVal) + 1));
			_tcscpy_s(m_pStrVal, _tcslen(src.m_pStrVal) + 1, src.m_pStrVal);
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
U2TString<CharType, buffSize>& 
U2TString<CharType, buffSize>::operator=(const CharType* src)
{
	if(src == *this)
		return *this;

	if( !buffSize )
	{	
		if(src == NULL)
		{
			m_pStrVal = NULL;
		}
		else 
		if(m_bAlloc)
		{
			U2_FREE(m_pStrVal);	
			m_pStrVal = NULL;
			m_pStrVal = U2_ALLOC(CharType,  
				sizeof(TCHAR) * GetBestBufferSize((unsigned int)_tcslen(src) + 1));
			_tcscpy_s(m_pStrVal, _tcslen(src) + 1, src);
		}
		else 
			m_pStrVal = (CharType*)src;

	}
	else 
	{
		m_buffer[buffSize] = 0;
		_tcscpy_s(m_pStrVal, buffSize, src);
	}

	return *this;
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
int	U2TString<CharType, buffSize>::Compare(const CharType* rhs) const
{
	if(rhs == NULL && m_pStrVal == NULL) 
		return 0;
	else if(m_pStrVal == NULL)
		return -rhs[0];
	else if(rhs == NULL)
		return m_pStrVal[0];
	return _tcscmp(m_pStrVal, rhs);	
}


//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
bool U2TString<CharType, buffSize>::operator==	(const U2TString& src) const
{
	return this->Compare(src.Str()) == 0;
}

template<typename CharType, int buffSize>
inline 
bool U2TString<CharType, buffSize>::operator==	(const CharType* src) const
{
	return this->Compare(src) == 0;
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
U2TString<CharType, buffSize>& 
U2TString<CharType, buffSize>::operator+=	(const CharType* rhs)
{
	Concatenate(rhs);
	return *this;
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
U2TString<CharType, buffSize>& 
U2TString<CharType, buffSize>::operator+=	(const U2TString& rhs)
{
	Concatenate(rhs.Str());
	return *this;
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
bool U2TString<CharType, buffSize>::operator!=	(const U2TString& src) const
{
	return this->Compare(src.Str()) != 0;
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
bool U2TString<CharType, buffSize>::operator!=	(const CharType* rhs) const
{
	return this->Compare(src) != 0;	
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
bool U2TString<CharType, buffSize>::operator>	(const U2TString& src) const
{
	return this->Compare(src) > 0;
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
bool U2TString<CharType, buffSize>::operator<	(const U2TString& src) const
{
	return this->Compare(src) < 0;
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
const CharType* U2TString<CharType, buffSize>::Str() const
{
	return m_pStrVal;
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
std::ostream& operator<<(std::ostream& os, const U2TString<CharType, buffSize>& src)
{
	return os << src.m_pStrVal;
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
unsigned int U2TString<CharType, buffSize>::Length() const 
{
	if(NULL == m_pStrVal)
		return 0;
	
	return (unsigned int)_tcslen(m_pStrVal);
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
bool U2TString<CharType, buffSize>::IsEmpty() const 
{
	return Length() == 0;
}

//-------------------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
void U2TString<CharType, buffSize>::SetAt(unsigned int ui, CharType ch)
{
	if(ui >= Length())
		return;
	m_pStrVal[ui] = ch;
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
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

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
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

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
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

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
unsigned int U2TString<CharType, buffSize>::ReverseFind(CharType c, 
														unsigned int startIdx) const
{
	if(NULL == m_pStrVal || c == _T('\0'))
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

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline 
unsigned int U2TString<CharType, buffSize>::FindOneOf(const CharType* szStr,
													  unsigned int index) const
{
	if(NULL == szStr || szStr == _T('\0') || NULL == m_pStrVal)
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

//-------------------------------------------------------------------------------------------------
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

//-------------------------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
/**
Trim both sides of a string.
*/

template<typename CharType, int buffSize>
inline 
void  U2TString<CharType, buffSize>::Trim(const CharType* pStr)
{
	int len = (int)_tcslen(pStr);
	
	for(int i=0; i < len; ++i)
	{
		TrimLeft(pStr[i]);
	}

	for(int i=0; i < len; ++i)
	{
		TrimRight(pStr[i]);
	}	
}


template<typename CharType, int buffSize>
inline 
bool U2TString<CharType, buffSize>::Resize(unsigned int reSize)
{
	if(!buffSize)
	{	
		size_t newStrLen = Length() + reSize;
		if(newStrLen + 1 > Length())
		{	
			CharType *newStrVal = U2_ALLOC(CharType, 
				sizeof(TCHAR) * GetBestBufferSize((unsigned int)newStrLen + 1));
			memcpy_s(newStrVal, newStrLen, m_pStrVal, Length());		
			newStrVal[newStrLen] = _T('\0');
			U2_FREE(m_pStrVal);
			//  ¿ÿ¡ˆ ∏ª∞Õ
			m_pStrVal = NULL;
			m_pStrVal = newStrVal;
			
			return true;
		}
		else 
			return false;	
	}
	else 
	{
		return true;
	}
}


template<typename CharType, int buffSize>
inline 
void U2TString<CharType, buffSize>::Concatenate(const CharType* szStr)
{
	if(NULL == szStr)
		return;

	size_t len = _tcslen(szStr);	

	size_t oldLen = Length();

	if(len > 0)
	{		
		Resize(uint32(_tcslen(szStr)));

		if(!buffSize)
		{		
			_tcscpy_s(m_pStrVal + oldLen, len+1, szStr);
		}
		else 
			_tcscpy_s(m_pStrVal + oldLen, len+1, szStr);
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
U2TString<CharType, buffSize> U2TString<CharType, buffSize>::GetSubStr
(unsigned int begin, unsigned int end) const
{
	if(begin < end && begin < Length() && end< Length() && 
		NULL != m_pStrVal)
	{		
		U2TString<CharType, buffSize> str;
		_tcsncpy_s((CharType*)str.Str(), buffSize,
			m_pStrVal + begin, end - begin + 1);
		((CharType*)str.Str())[end-begin+1] = _T('\0');
		return str;

	}
	else 
	{
		return (const CharType*)NULL;
	}

}

//------------------------------------------------------------------------------
/**
Tokenize the string into a provided nString array. Returns the number
of tokens. This method is recommended over GetFirstToken()/GetNextToken(),
since it is atomic. This nString object will not be destroyed
(as is the case with GetFirstToken()/GetNextToken().

@param  whiteSpace      [in] a string containing the whitespace characters
@param  tokens          [out] nArray<nString> where tokens will be appended
@return                 number of tokens found
*/
template<typename CharType, int buffSize>
inline 
uint32 U2TString<CharType, buffSize>::Tokenize(const TCHAR* whiteSpace, 
											U2ObjVec<U2TString>& tokens) const
{
	uint32 numTokens = 0;

	U2TString str(*this);
	TCHAR* ptr = (TCHAR*)str.Str();
	const TCHAR* token;
	TCHAR* szNextToken;
	while(0 != (token = _tcstok_s(ptr, whiteSpace, &szNextToken)))
	{
		tokens.AddElem(U2TString(token));
		ptr = 0;
		numTokens++;	
	}
	return numTokens;
}

//------------------------------------------------------------------------------
/**
Tokenize a string, but keeps the string within the fence-character
intact. For instance for the sentence:

He said: "I don't know."

A Tokenize(" ", '"', tokens) would return:

token 0:    He
token 1:    said:
token 2:    I don't know.
*/
//template<typename CharType, int buffSize>
//inline 
//uint32 U2TString<CharType, buffSize>::Tokenize(const TCHAR* whiteSpace, 
//											   TCHAR fence,
//											   U2PrimitiveVec<U2TString*>& tokens) const
//{
//	
//	
//
//
//
//
//
//
//}

//------------------------------------------------------------------------------
/**
Returns true if string contains one of the characters from charset.
*/
template<typename CharType, int buffSize>
inline
bool
U2TString<CharType, buffSize>::Contains(const CharType* charSet) const
{
	U2ASSERT(charSet);
	CharType* str = (CharType*)Str();
	CharType* ptr = _tcspbrk(str, charSet);
	return (0 != ptr);
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline
void U2TString<CharType, buffSize>::Format(const CharType* fmt, ...)
{
	va_list argList;
	va_start(argList, fmt);
	FormatWithArgs(fmt, argList);
	va_end(argList);	
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline
void U2TString<CharType, buffSize>::FormatWithArgs(const CharType* fmt, va_list args)
{
	va_list argList;
	argList = args;

	size_t reqLen;

	reqLen = _vsctprintf (fmt, argList);

	reqLen++; // accunt for NULL termination
	va_end(argList);

	TCHAR* buff = U2_ALLOC(TCHAR, reqLen);

	_vsntprintf_s(buff, sizeof(TCHAR) * reqLen, reqLen, fmt, args);

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
void U2TString<CharType, buffSize>::SetFloat(float val)
{
	this->Format(_T("%.6f"), val);
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline
void U2TString<CharType, buffSize>::SetVector3(const D3DXVECTOR3& v)
{
	this->Format(_T("%.6f,%.6f,%.6f,"), v.x, v.y, v.z);
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline
void U2TString<CharType, buffSize>::SetVector4(const D3DXVECTOR4& v)
{
	this->Format(_T("%.6f,%.6f,%.6f,%.6f"), v.x, v.y, v.z, v.w);
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline
void U2TString<CharType, buffSize>::SetMatrix44(const D3DXMATRIX& m)
{
	this->Format(_T("%.6f, %.6f, %.6f, %.6f, "
		"%.6f, %.6f, %.6f, %.6f, "
		"%.6f, %.6f, %.6f, %.6f, "
		"%.6f, %.6f, %.6f, %.6f"),
		m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
		m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
		m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
		m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3]);
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

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline
bool U2TString<CharType, buffSize>::ToBool() const
{
	static const TCHAR* bools[] = {
		_T("no"), _T("yes"), _T("off"), _T("on"), _T("false"), _T("true"), 0
	};
	int i = 0;
	while (bools[i] != 0)
	{
		if (0 == _tcsicmp(bools[i], this->Str()))
		{
			return (1 == (i & 1));

		}
		i++;
	}
	
	U2ASSERT2(false, _T("Invalid string value for bool!"));	

	return false;	
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline
float U2TString<CharType, buffSize>::ToFloat() const
{
	return float(_tstof(Str()));
	
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline
int U2TString<CharType, buffSize>::ToInt() const
{
	return _tstoi(Str());
	
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline
D3DXVECTOR3 U2TString<CharType, buffSize>::ToVector3() const
{
	U2ObjVec<U2DynString> tokens;
	this->Tokenize(_T(", \t"), tokens);

	D3DXVECTOR3 v(tokens[0].ToFloat(), tokens[1].ToFloat(), tokens[2].ToFloat());
	return v;
}

//-------------------------------------------------------------------------------------------------
template<typename CharType, int buffSize>
inline
D3DXVECTOR4 U2TString<CharType, buffSize>::ToVector4() const
{
	U2ObjVec<U2DynString> tokens;
	this->Tokenize(_T(", \t"), tokens);

	D3DXVECTOR4 v(tokens[0].ToFloat(), tokens[1].ToFloat(), tokens[2].ToFloat(),
		tokens[3].ToFloat());
	return v;
}


template<typename CharType, int buffSize>
inline
void U2TString<CharType, buffSize>::SetBool(bool val)
{
	if (val)
	{
		*this = _T("true");
	}
	else
	{
		*this = _T("false");
	}
}

//------------------------------------------------------------------------------
/**
This converts an UTF-8 string to 8-bit-ANSI. Note that only characters
in the range 0 .. 255 are converted, all other characters will be converted
to a question mark.

For conversion rules see http://www.cl.cam.ac.uk/~mgk25/unicode.html#utf-8
*/
template<typename CharType, int buffSize>
inline
void U2TString<CharType, buffSize>::UTF8toANSI()
{
	uint8* src = (uint8*)this->Str();
	uint8* dst = src;
	uint8 c;
	while ((c = *src++))
	{
		if (c >= 0x80)
		{
			if ((c & 0xE0) == 0xC0)
			{
				// a 2 byte sequence with 11 bits of information
				uint16 wide = ((c & 0x1F) << 6) | (*src++ & 0x3F);
				if (wide > 0xff)
				{
					c = '?';
				}
				else
				{
					c = (uint8) wide;
				}
			}
			else if ((c & 0xF0) == 0xE0)
			{
				// a 3 byte sequence with 16 bits of information
				c = '?';
				src += 2;
			}
			else if ((c & 0xF8) == 0xF0)
			{
				// a 4 byte sequence with 21 bits of information
				c = '?';
				src += 3;
			}
			else if ((c & 0xFC) == 0xF8)
			{
				// a 5 byte sequence with 26 bits of information
				c = '?';
				src += 4;
			}
			else if ((c & 0xFE) == 0xFC)
			{
				// a 6 byte sequence with 31 bits of information
				c = '?';
				src += 5;
			}
		}
		*dst++ = c;
	}
	*dst = 0;
}

//------------------------------------------------------------------------------
/**
Convert contained ANSI string to UTF-8 in place.
*/
template<typename CharType, int buffSize>
inline
void
U2TString<CharType, buffSize>::ANSItoUTF8()
{
	U2ASSERT(!this->IsEmpty());
	int bufSize = this->Length() * 2 + 1;
	char* buffer = U2_ALLOC(char, bufSize);
	char* dstPtr = buffer;
	const char* srcPtr = this->Str();
	unsigned char c;
	while ((c = *srcPtr++))
	{
		// note: this only covers the 2 cases that the character
		// is between 0 and 127 and between 128 and 255
		if (c < 128)
		{
			*dstPtr++ = c;
		}
		else
		{
			*dstPtr++ = 192 + (c / 64);
			*dstPtr++ = 128 + (c % 64);
		}
	}
	*dstPtr = 0;
	*this = buffer;
	U2_FREE(buffer);
	buffer = 0;
}


template<typename CharType, int buffSize>
inline bool			operator==	(const CharType* lhs,const U2TString<CharType, buffSize>& rhs)
{
	return rhs.Compare(lhs) == 0;
}

template<typename CharType, int buffSize>
inline bool			operator==	(const U2TString<CharType, buffSize>& lhs,const CharType* rhs)
{
	return lhs.Compare(rhs) == 0;
}


template<typename CharType, int buffSize>
U2TString<CharType, buffSize> operator+	(CharType lhs,const U2TString<CharType, buffSize>& rhs)
{
	U2TString<CharType, buffSize> res(lhs);
	res.Concatenate(rhs);
	return res;
}

template<typename CharType, int buffSize>
U2TString<CharType, buffSize> operator+	(const U2TString<CharType, buffSize>& lhs, CharType rhs)
{
	U2TString<CharType, buffSize> res(lhs);
	res.Concatenate(rhs);
	return res;
}


template<typename CharType, int buffSize>
U2TString<CharType, buffSize> operator+	(const CharType* lhs,const U2TString<CharType, buffSize>& rhs)
{
	U2TString<CharType, buffSize> res(lhs);
	res.Concatenate(rhs);
	return res;
}

template<typename CharType, int buffSize>
U2TString<CharType, buffSize> operator+	(const U2TString<CharType, buffSize>& lhs,const CharType* rhs)
{
	U2TString<CharType, buffSize> res(lhs);
	res.Concatenate(rhs);
	return res;
}


template<typename CharType, int buffSize>
inline
U2TString<CharType, buffSize>
U2TString<CharType, buffSize>::Substitute(const CharType* szMatchStr, const CharType* szSubStr)
{
	U2ASSERT(szMatchStr && szSubStr);

	const CharType* szPtr = this->Str();
	size_t matchStrLen = _tcslen(szMatchStr);

	U2TString dest;


	const CharType* pOccur;
	while((pOccur == _tcsstr(szPtr, szMatchStr)))
	{
		// append string 
		dest.Concatenate(this->GetSubStr(0, pOccur - szPtr));
		dest.Concatenate(szSubStr);

		szPtr = pOccur + matchStrLen;
	}

	dest.Concatenate(szPtr);
	return dest;
}