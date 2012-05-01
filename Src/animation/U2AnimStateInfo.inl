inline U2AnimStateInfo::U2AnimStateInfo() 
	: m_clipArray(0, 0),
	m_clipWeights(0, 0),
	m_fFadeInTime(0.0f),
	m_fStateStarted(0.0f),
	m_fStateOffset(0.0f)
{

}

inline void U2AnimStateInfo::SetFadeInTime(float fTime)
{
	m_fFadeInTime = fTime;
}


inline float U2AnimStateInfo::GetFadeInTime() const
{
	return m_fFadeInTime;
}

inline void U2AnimStateInfo::SetStateStarted(float fTime)
{
	m_fStateStarted = fTime;
}


inline float U2AnimStateInfo::GetStateStarted() const
{
	return m_fStateStarted;
}


inline void U2AnimStateInfo::SetStateOffset(float offset)
{
	m_fStateOffset = offset;
}


inline float U2AnimStateInfo::GetStateOffset() const
{
	return m_fStateOffset;
}


inline bool U2AnimStateInfo::IsValid() const
{
	return (m_clipArray.Size() > 0);
}


inline void U2AnimStateInfo::BeginClips(int num)
{
	m_clipArray.Resize(num);
	m_clipWeights.Resize(num);

	uint32 i;
	for(i = 0; i < m_clipArray.Size(); ++i)
	{
		m_clipArray.SetElem(i, U2_NEW U2AnimClip());
	}

}


inline void U2AnimStateInfo::SetClip(int idx, const U2AnimClip& clip, float weight)
{
	U2ASSERT(idx >=0);
	U2ASSERT(0 <= weight && weight <= 1.0f);

	if(m_clipArray.GetElem(0)->GetNumInterpKeys() > 0)
	{
		if(m_clipArray.GetElem(0)->GetNumInterpKeys() != clip.GetNumInterpKeys())
		{
			U2ASSERT(_T("Only Clips with identical number of curves can be active at one time."));
		}
	}

	m_clipArray.SetElem(idx, (U2AnimClip*)&clip);
	m_clipWeights.SetElem(idx, weight);

}


inline void U2AnimStateInfo::EndClips()
{

}

inline int U2AnimStateInfo::GetNumClips() const
{
	return m_clipArray.Size();
}

inline U2AnimClip& U2AnimStateInfo::GetClip(int idx) const
{
	return *m_clipArray.GetElem(idx);
}


inline float U2AnimStateInfo::GetClipWeight(int idx) const
{
	return m_clipWeights.GetElem(idx);
}