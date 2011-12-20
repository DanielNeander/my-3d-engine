//-------------------------------------------------------------------------------------------------
inline void U2FrameXmlParser::SetFrame(U2Frame* pFrame)
{
	m_pOwnerFrame = pFrame;	
}

//-------------------------------------------------------------------------------------------------
inline U2Frame* U2FrameXmlParser::GetFrame()
{
	return m_pOwnerFrame;
}

inline void U2FrameXmlParser::SetFilename(const U2DynString& szFilename)
{
	m_szFullPath = szFilename;
}

//-------------------------------------------------------------------------------------------------
inline const U2DynString& U2FrameXmlParser::GetFilename() const
{
	return m_szFullPath;
}

//-------------------------------------------------------------------------------------------------
inline bool U2FrameXmlParser::HasAttr(TiXmlElement* elm, const char* name)
{
	U2ASSERT(elm && name);
	return (elm->Attribute(name) != 0);
}

//-------------------------------------------------------------------------------------------------
inline int U2FrameXmlParser::GetIntAttr(TiXmlElement* elm, const char* name, int defaultValue)
{
	U2ASSERT(elm && name);
	int value;
	int ret = elm->QueryIntAttribute(name, &value);
	if (TIXML_SUCCESS == ret)
	{
		return value;
	}
	else
	{
		return defaultValue;
	}
}

//-------------------------------------------------------------------------------------------------
inline float U2FrameXmlParser::GetFloatAttr(TiXmlElement* elm, const char* name, 
											float defaultValue)
{
	U2ASSERT(elm && name);
	double value;
	int ret = elm->QueryDoubleAttribute(name, &value);
	if (TIXML_SUCCESS == ret)
	{
		return float(value);
	}
	else
	{
		return defaultValue;
	}
}

//-------------------------------------------------------------------------------------------------
/// get a vector4 attribute from an xml element
inline D3DXVECTOR4 U2FrameXmlParser::GetVector4Attr(TiXmlElement* elm, const char* name, 
						   const D3DXVECTOR4& defaultValue)
{
	U2ASSERT(elm && name);
#ifdef UNICODE 
	U2DynString valStr = ToUnicode(elm->Attribute(name));
#else 
	U2DynString valStr = elm->Attribute(name);
#endif

	if (!valStr.IsEmpty())
	{
		U2ObjVec<U2DynString> tokens;
		valStr.Tokenize(_T(" "), tokens);
		D3DXVECTOR4 value;
		value.x = tokens[0].ToFloat();
		value.y = tokens[1].ToFloat();
		value.z = tokens[2].ToFloat();
		value.w = tokens[3].ToFloat();
		return value;
	}
	else
	{
		return defaultValue;
	}
}

//-------------------------------------------------------------------------------------------------
/// get a float4 attribute from an xml element
inline U2Float4 U2FrameXmlParser::GetFloat4Attr(TiXmlElement* elm, const char* name, 
												const U2Float4& defaultValue)
{
	U2ASSERT(elm && name);
#ifdef UNICODE 
	U2DynString valStr = ToUnicode(elm->Attribute(name));
#else 
	U2DynString valStr = elm->Attribute(name);
#endif
	if (!valStr.IsEmpty())
	{
		U2ObjVec<U2DynString> tokens;
		valStr.Tokenize(_T(" "), tokens);
		U2Float4 value;
		value.x = tokens[0].ToFloat();
		value.y = tokens[1].ToFloat();
		value.z = tokens[2].ToFloat();
		value.w = tokens[3].ToFloat();
		return value;
	}
	else
	{
		return defaultValue;
	}

}

//-------------------------------------------------------------------------------------------------
/// get a bool attribute from an xml element
inline bool U2FrameXmlParser::GetBoolAttr(TiXmlElement* elm, const char* name, 
										  bool defaultValue)
{
	U2ASSERT(elm && name);
#ifdef UNICODE 
	U2DynString valStr = ToUnicode(elm->Attribute(name));
#else 
	U2DynString valStr = elm->Attribute(name);
#endif
	if (!valStr.IsEmpty())
	{
		if ((valStr == _T("true")) || (valStr == _T("True")) ||
			(valStr == _T("on")) || (valStr == _T("On")) ||
			(valStr == _T("yes")) || (valStr == _T("Yes")))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return defaultValue;
	}

}