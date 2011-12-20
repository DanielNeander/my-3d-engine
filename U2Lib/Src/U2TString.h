/************************************************************************
module	:	U2String
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#ifndef U2_TSTRING_H
#define U2_TSTRING_H

//------------------------------------------------------------------------------
/**
@class nString
@ingroup NebulaDataTypes

@brief A simple Nebula string class.

Very handy when strings must be stored or manipulated. Note that many
Nebula interfaces hand strings around as char pointers, not nString
objects.

The current implementation does not allocate extra memory if the
string has less then 13 characters.

(C) 2001 RadonLabs GmbH
*/


#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <string>

#include <U2Lib/Src/Memory/U2MemObj.h>
#include <U2Lib/src/U2TVec.h>

#include <d3dx9math.h>

template<typename CharType, int buffSize>
class  U2TString : public U2MemObj
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
	U2TString&	operator=	(const CharType* src);
	bool 		operator==	(const U2TString& src) const;
	bool			operator==	(const CharType* rhs) const;

	U2TString&	operator+=	(const CharType* rhs);
	U2TString&	operator+=	(const U2TString& rhs);
	bool			operator!=	(const U2TString& src) const;
	bool			operator!=	(const CharType* rhs) const;
	bool			operator>	(const U2TString& src) const;
	bool			operator<	(const U2TString& src) const;
	
	
	const CharType*		Str() const;
	bool				IsEmpty() const;
	unsigned int		Length() const;
	CharType			GetAt(unsigned int ui) const;
	void				SetAt(unsigned int ui, CharType c);
	//bool				Contains(const CharType* szStr) const;
	U2TString			GetSubStr(unsigned int begin, unsigned int end) const;
	unsigned int		Find(CharType ch, unsigned int start) const;
	unsigned int		Find(const CharType* szStr, unsigned int start) const;
	unsigned int		ReverseFind(CharType ch) const;
	unsigned int		ReverseFind(CharType ch, unsigned int start) const;
	unsigned int		FindOneOf(const CharType* pcStr,unsigned int index) const;
	void				ToUpper();
	void				ToLower();
	void				Reverse();
	 /// delete characters from ch at left side of string
	void				TrimLeft(CharType ch);
	/// delete characters from ch at right side of string
	void				TrimRight(CharType ch);
	/// trim characters from charset at both sides of string
	void				Trim(const CharType* pStr);
	void				Concatenate(const CharType* szStr);
	void				Concatenate(CharType ch);

	U2TString			Substitute(const CharType* szMatchStr, const CharType* szSubStr);
	
	static unsigned int GetBestBufferSize(unsigned int uiReqSize);

	int					Compare(const CharType* rhs) const;
	
	bool				Resize(unsigned int reSize);

	void				Format(const CharType* fmt, ...);
	void				FormatWithArgs(const CharType* fmt, va_list args);

	void				SetInt(int val);
	void				SetFloat(float val);
	void				SetVector3(const D3DXVECTOR3& v);
	void				SetVector4(const D3DXVECTOR4& v);
	void				SetBool(bool val);
	void				SetMatrix44(const D3DXMATRIX& m);

	void				AppendInt(int i);	
	
	uint32				Tokenize(const TCHAR* whitespace, 
		U2ObjVec<U2TString>& tokens) const;

	//int					Tokenize(const TCHAR* whitespace, TCHAR fence,
	//	U2PrimitiveVec<U2TString*>& tokens) const;

	bool				Contains(const CharType* charSet) const;
	
	
	void				UTF8toANSI();
	void				ANSItoUTF8();


	bool				ToBool() const;
	float				ToFloat() const;
	int					ToInt() const;	
	//bool				ToUnit(uint32 ui) const;	

	D3DXVECTOR3			ToVector3() const;
	D3DXVECTOR4			ToVector4() const;
	//D3DXMATRIX			ToMatrix44() const;


	

	friend	std::ostream& operator<<(std::ostream& os, const U2TString<CharType, buffSize>& str);
	
private:
	bool			m_bAlloc;			// heap alloc flag
	CharType	*m_pStrVal;				// heap alloc	
	CharType	m_buffer[buffSize + 1];	// stack alloc
};


template<typename CharType, int buffSize>
bool operator==	(const CharType* lhs,const U2TString<CharType, buffSize>& rhs);
template<typename CharType, int buffSize>
bool operator==	(const U2TString<CharType, buffSize>& lhs,const CharType* rhs);


template<typename CharType, int buffSize>
U2TString<CharType, buffSize> operator+	(CharType lhs,const U2TString<CharType, buffSize>& rhs);

template<typename CharType, int buffSize>
U2TString<CharType, buffSize> operator+	(const U2TString<CharType, buffSize>& lhs, CharType rhs);

template<typename CharType, int buffSize>
U2TString<CharType, buffSize> operator+	(const CharType* lhs,const U2TString<CharType, buffSize>& rhs);

template<typename CharType, int buffSize>
 U2TString<CharType, buffSize> operator+	(const U2TString<CharType, buffSize>& lhs,const CharType* rhs);




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


#include "U2TString.inl"

#endif
