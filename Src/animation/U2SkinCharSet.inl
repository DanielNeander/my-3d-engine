inline U2SkinCharSet::U2SkinCharSet() 
	:m_bDirty(false),
	m_fFadeInTime(1.0f)
{

}

inline U2SkinCharSet::~U2SkinCharSet()
{

}

inline void U2SkinCharSet::SetFadeInTime(float fTime)
{
	m_fFadeInTime = fTime;
}


inline float U2SkinCharSet::GetFadeInTime() const
{
	return m_fFadeInTime;
}


inline void U2SkinCharSet::AddClip(const U2DynString& clipName, float clipWeight)
{

	m_clipNames.AddElem(clipName);
	m_clipWeights.AddElem(clipWeight);
	m_bDirty = true;
}


inline void U2SkinCharSet::RemoveClip(const U2DynString& clipName)
{
	uint32 idx = GetClipIdx(clipName);
	U2ASSERT(idx != -1);
	m_clipNames.Remove(idx);
	m_clipWeights.Remove(idx);
	m_bDirty = true;
}


inline void U2SkinCharSet::SetClipWeight(int idx, float clipWeight)
{
	m_clipWeights[idx] = clipWeight;
	m_bDirty = true;
}


inline int U2SkinCharSet::GetClipIdx(const U2DynString& clipName) const
{
	uint32 i;
	for(i=0; i < m_clipNames.Size(); ++i)
	{
		if(m_clipNames[i] == clipName)
		{
			return i;
		}
	}

	return -1;
}


inline const U2DynString& U2SkinCharSet::GetClipname(int idx) const
{
	return m_clipNames[idx];
}


inline float U2SkinCharSet::GetClipWeight(int idx) const
{
	return m_clipWeights[idx];
}


inline int U2SkinCharSet::GetNumClips() const
{
	return m_clipNames.Size();
}


inline void U2SkinCharSet::ClearClips() 
{
	m_clipNames.RemoveAll();
	
	m_clipWeights.RemoveAll();
	m_bDirty = true;
}


inline bool U2SkinCharSet::IsDirty() const
{
	return m_bDirty;
}


inline void U2SkinCharSet::SetDirty(bool b)
{
	m_bDirty = b;
}