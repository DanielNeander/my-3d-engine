inline U2Animation::InterpKey::InterpKey()
	:m_eInterpType(INTERP_NONE),
	m_iFirstKeyIdx(-1),
	m_iIsAnimated(1),
	m_startVal(0.f, 0.f, 0.f, 0.f),
	m_firstClipVal(0.f, 0.f, 0.f, 0.f)
{

}


inline U2Animation::InterpKey::~InterpKey()
{

}


inline void U2Animation::InterpKey::SetInterpType(InterpType type)
{
	m_eInterpType = type;
}


inline U2Animation::InterpKey::InterpType U2Animation::InterpKey::GetInterpType() const
{
	return m_eInterpType;
}

inline void U2Animation::InterpKey::SetConstValue(const D3DXVECTOR4& val)
{
	m_constVal = val;
}

inline const D3DXVECTOR4& U2Animation::InterpKey::GetConstValue() const
{
	return m_constVal;
}


inline U2Animation::InterpKey::InterpType U2Animation::InterpKey::StrToInterpType(const TCHAR* str)
{
	if (0 == _tcscmp(_T("none"), str))       return INTERP_NONE;
	if (0 == _tcscmp(_T("step"), str))       return INTERP_STEP;
	if (0 == _tcscmp(_T("quat"), str))       return INTERP_QUAT;
	return INTERP_LINEAR;
}


inline void U2Animation::InterpKey::SetFirstKeyIdx(int idx)
{
	m_iFirstKeyIdx = idx;
}


inline int U2Animation::InterpKey::GetFirstKeyIdx() const
{
	return m_iFirstKeyIdx;
}


inline void U2Animation::InterpKey::SetAnimated(int isAnim)
{
	m_iIsAnimated = isAnim;
}


inline int U2Animation::InterpKey::IsAnimated() const
{
	return m_iIsAnimated;
}


inline void U2Animation::InterpKey::SetStartValue(const D3DXVECTOR4& val)
{
	m_startVal = val;
}


inline D3DXVECTOR4 U2Animation::InterpKey::GetStartValue() const
{
	return m_startVal;
}


inline void U2Animation::InterpKey::SetCurAnimClipValue(const D3DXVECTOR4& val)
{
	m_firstClipVal = val;
}


inline D3DXVECTOR4 U2Animation::InterpKey::GetCurAnimClipValue() const
{
	return m_firstClipVal;
}



//-------------------------------------------------------------------------------------------------
inline U2Animation::InterpKeyGroup::InterpKeyGroup()
	:m_iStartKey(0),
	m_iNumKeys(0),
	m_iKeyStride(0),
	m_fKeyTime(0.f),
	m_eLoopType(LOOP_REPEAT),
	m_fFadeInFrames(0.f),
	m_interpKeyArray(0, 0)
{
	
}

inline U2Animation::InterpKeyGroup::~InterpKeyGroup()
{
	for(unsigned int i=0; i < m_interpKeyArray.FilledSize(); ++i)
	{
		U2_DELETE m_interpKeyArray[i];
	}

	m_interpKeyArray.RemoveAll();

}


inline void U2Animation::InterpKeyGroup::SetNumInterpKeys(int c)
{
	m_interpKeyArray.Resize(c);

	for(unsigned int i=0; i < m_interpKeyArray.Size(); ++i)
	{
		m_interpKeyArray.SetElem(i, U2_NEW InterpKey());
	}
}


inline uint32 U2Animation::InterpKeyGroup::GetNumInterpKeys() const
{
	return (int)m_interpKeyArray.Size();
}


inline U2Animation::InterpKey& U2Animation::InterpKeyGroup::GetInterpKey(int i) const
{
	return *m_interpKeyArray.GetElem(i);
}


inline void U2Animation::InterpKeyGroup::SetStartKey(int idx)
{
	m_iStartKey = idx;
}


inline int U2Animation::InterpKeyGroup::GetStartKey() const
{
	return m_iStartKey;
}


inline void U2Animation::InterpKeyGroup::SetNumKeys(int i)
{
	m_iNumKeys = i;
}


inline int U2Animation::InterpKeyGroup::GetNumKeys() const
{
	return m_iNumKeys;
}


inline void U2Animation::InterpKeyGroup::SetKeyStride(int stride)
{
	m_iKeyStride = stride;
}


inline int U2Animation::InterpKeyGroup::GetKeyStride() const
{
	return m_iKeyStride;
}



inline void U2Animation::InterpKeyGroup::SetKeyTime(float fTime)
{
	m_fKeyTime = fTime;
}


inline float U2Animation::InterpKeyGroup::GetKeyTime() const
{
	return m_fKeyTime;
}

inline void U2Animation::InterpKeyGroup::SetFadeInFrames(float fFrames)
{
	m_fFadeInFrames = fFrames;
}


inline float U2Animation::InterpKeyGroup::GetFadeInFrames() const
{
	return m_fFadeInFrames;
}


inline void U2Animation::InterpKeyGroup::SetLoopType(
	U2Animation::InterpKeyGroup::LoopTytpe eLoop)
{
	m_eLoopType = eLoop;
}


inline U2Animation::InterpKeyGroup::LoopTytpe 
U2Animation::InterpKeyGroup::GetLoopType() const
{
	return m_eLoopType;
}


inline U2Animation::InterpKeyGroup::LoopTytpe 
U2Animation::InterpKeyGroup::StrToLoopType(const TCHAR* str)
{
	if(0 == _tcscmp(_T("repeat"), str)) return LOOP_REPEAT;
	return LOOP_CLAMP;
}


inline void U2Animation::InterpKeyGroup::TimeToIdx(
	float fTime, int& iKeyIdx0, int& iKeyIdx1, float& fInBetween) const
{
	float fFrame = fTime / m_fKeyTime;
	int iFrame = int(fFrame);
	iKeyIdx0 = iFrame - m_iStartKey;
	iKeyIdx1 = iKeyIdx0 + 1;
	fInBetween = U2Math::Saturate(fFrame - float(iFrame));
	if(LOOP_CLAMP == m_eLoopType)
	{
		if(iKeyIdx0 < 0)				iKeyIdx0 = 0;
		else if(iKeyIdx0 >= m_iNumKeys) iKeyIdx0 = m_iNumKeys - 1;

		if(iKeyIdx1 < 0)				iKeyIdx1 = 0;
		else if(iKeyIdx1 >= m_iNumKeys) iKeyIdx1 = m_iNumKeys - 1;
	}
	else 
	{
		iKeyIdx0 %= m_iNumKeys;
		iKeyIdx1 %= m_iNumKeys;
		if(iKeyIdx0 < 0)
		{
			iKeyIdx0 += m_iNumKeys;
		}
		if(iKeyIdx1 < 0)
		{
			iKeyIdx1 += m_iNumKeys;
		}
	}

	iKeyIdx0 *= m_iKeyStride;
	iKeyIdx1 *= m_iKeyStride;
}


//------------------------------------------------------------------------------
/**
Return true if absolute time is between relative start and stop time.
This will handle the case where the curve is looped and time is out
of bounds correctly.

Convert an absolute time into a normalized time. If the group is looped,
this will return a time within start and end.
*/
inline bool U2Animation::InterpKeyGroup::IsBetweenTimes(
	float fTime, float fStartTime, float fStopTime) const
{
	if(LOOP_CLAMP == m_eLoopType)
	{
		return (fTime >= fStopTime) && (fTime <= fStopTime);
	}

	float fDur = float(this->GetDuration());
	float fNormStartTime = U2Math::Fmod(fStartTime, fDur);
	float fNormStopTime = U2Math::Fmod(fStopTime, fDur);
	if(fNormStartTime < fNormStopTime)
	{
		return fTime >= fNormStartTime && fTime <= fNormStopTime;
	}

	return fTime >= fNormStartTime || fTime <= fNormStopTime;
}


inline double U2Animation::InterpKeyGroup::GetDuration() const
{
	return m_iNumKeys * m_fKeyTime;
}



inline void U2Animation::SetNumInterpKeyGroups(int group)
{
	m_interpKeyGroupArray.Resize(group);

	for(uint32 i=0; i < m_interpKeyGroupArray.Size(); ++i)
	{
		m_interpKeyGroupArray.SetElem(i, U2_NEW InterpKeyGroup());
	}
}


inline int U2Animation::GetNumInterpKeyGroups() const
{
	return m_interpKeyGroupArray.FilledSize();
}


inline U2Animation::InterpKeyGroup& U2Animation::GetInterpKeyGroup(int idx) const
{
	return *m_interpKeyGroupArray.GetElem(idx);
}
