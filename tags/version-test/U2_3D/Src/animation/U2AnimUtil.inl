inline
U2DynString
U2AnimLoopType::ToString(U2AnimLoopType::Type t)
{
	switch (t)
	{
	case LOOP:  return U2DynString(_T("loop"));
	case CLAMP: return U2DynString(_T("clamp"));
	default:
		U2ASSERT(_T("AnimLoopType::ToString(): invalid enum value!"));
		return U2DynString(_T(""));
	}
}

//------------------------------------------------------------------------------
/**
*/
inline
U2AnimLoopType::Type
U2AnimLoopType::FromString(const U2DynString& s)
{
	if (s == _T("loop")) return LOOP;
	else if (s == _T("clamp")) return CLAMP;
	else
	{
		FDebug(_T("AnimLoopType::ToString(): invalid loop type '%s'\n"), s.Str());
		return CLAMP;
	}
}
