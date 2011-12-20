inline U2AnimClip::U2AnimClip()
	:m_iAnimGroupIdx(0),
	m_iNumInterpKeys(0)
{
	
}

inline U2AnimClip::U2AnimClip(const U2DynString& szClipname, 
							  int iAnimGroupIdx, int iNumInterpKeys)
							  :m_iAnimGroupIdx(iAnimGroupIdx),
							  m_iNumInterpKeys(iNumInterpKeys),
							  m_szClipname(szClipname)
{

}

inline const U2DynString& U2AnimClip::GetClipname() const
{
	return m_szClipname;
}

inline int U2AnimClip::GetAnimGroupIdx() const
{	
	return m_iAnimGroupIdx;
}

inline int U2AnimClip::GetNumInterpKeys() const
{	
	return m_iNumInterpKeys;
}

inline void U2AnimClip::SetNumAnimKeyTracks(uint32 num)
{
	m_animKeyTracks.SetSize(num);
}


inline uint32 U2AnimClip::GetNumAimKeyTracks() const
{
	return m_animKeyTracks.Size();
}

inline const U2AnimKeyTrack& U2AnimClip::GetAnimKeyTrack(int idx) const
{
	return m_animKeyTracks[idx];
}

inline U2FixedObjArray<U2AnimKeyTrack>& U2AnimClip::GetAnimKeyTracks()
{
	return m_animKeyTracks;
}