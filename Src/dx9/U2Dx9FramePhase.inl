//-------------------------------------------------------------------------------------------------
inline void U2FramePhase::SetFrame(U2Frame* pFrame)
{
	m_pOwnerFrame = pFrame;	
}

//-------------------------------------------------------------------------------------------------
inline U2Frame* U2FramePhase::GetFrame()
{
	return m_pOwnerFrame;
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePhase::SetShaderAlias(const U2DynString& p)
{
	m_szShaderAlias = p;
}	


inline const U2DynString& U2FramePhase::GetShaderAlias() const
{
	return m_szShaderAlias;
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePhase::SetTechnique(const U2DynString& tec)
{
	m_szTechnique = tec;
}

//-------------------------------------------------------------------------------------------------
inline const U2DynString& U2FramePhase::GetTechnique() const
{
	return m_szTechnique;
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePhase::SetSortOrder(SortingOrder eSort)
{
	m_eSortOrder = eSort;
}

//-------------------------------------------------------------------------------------------------
inline U2FramePhase::SortingOrder U2FramePhase::GetSortOrder() const
{
	return m_eSortOrder;
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePhase::SetLightMode(LightMode eMode)
{
	m_eLightMode = eMode;
}

//-------------------------------------------------------------------------------------------------
inline U2FramePhase::LightMode U2FramePhase::GetLightMode() const
{
	return m_eLightMode;
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePhase::AddFrameSeq(U2FrameSequence* pSeq)
{
	m_frameSeqs.AddElem(pSeq);
}

//-------------------------------------------------------------------------------------------------
inline U2FrameSequence* U2FramePhase::GetFrameSeq(unsigned int idx) const
{
	return m_frameSeqs[idx];
}

//-------------------------------------------------------------------------------------------------
inline const U2ObjVec<U2FrameSequencePtr>& U2FramePhase::GetFrameSeqs() const
{	
	return m_frameSeqs;
}

//-------------------------------------------------------------------------------------------------
inline U2FramePhase::SortingOrder U2FramePhase::StrToSortingOrder(const TCHAR* str)
{
	U2ASSERT(str);
	if (0 == _tcscmp(_T("None"), str)) return NONE;
	else if (0 == _tcscmp(_T("FrontToBack"), str)) return FRONT_TO_BACK;
	else if (0 == _tcscmp(_T("BackToFront"), str)) return BACK_TO_FRONT;
	else
	{
		FDebug("U2Dx9FramePhase::StringToSortingOrder(): invalid string '%s'!", str);
		return NONE;
	}
}

//-------------------------------------------------------------------------------------------------
inline U2FramePhase::LightMode U2FramePhase::StrToLightMode(const TCHAR* str)
{
	U2ASSERT(str);
	if (0 == _tcscmp(_T("Off"), str)) return LM_OFF;
	else if (0 == _tcscmp(_T("FFP"), str)) return LM_FIXED_FUNC;
	else if (0 == _tcscmp(_T("Shader"), str)) return LM_SHADER;
	else
	{
		FDebug("U2Dx9FramePhase::StrToLightMode(): invalid string '%s'!", str);
		return LM_OFF;
	}

}


