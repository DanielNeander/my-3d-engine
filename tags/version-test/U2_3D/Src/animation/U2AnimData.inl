
inline U2Skeleton& U2AnimData::GetSkeleton()
{
	return *m_pSkel;
}


inline void U2AnimData::SetAnim(U2Animation* pAnim)
{
	m_spAnim = pAnim;
}


inline const U2AnimationPtr& U2AnimData::GetAnim() const
{
	return m_spAnim;
}




inline U2SkinController* U2AnimData::GetSkinController() const
{
	return SmartPtrCast(U2SkinController, m_spSkinCtrl);
}


inline void U2AnimData::SetAnimEnabled(bool b)
{
	m_bAnimEnabled = b;
}


inline bool U2AnimData::IsAnimEnabled() const
{
	return m_bAnimEnabled;
}

inline void U2AnimData::SetLastEvalFrameId(uint32 id)
{
	m_uiLastEvalFrameId = id;
}


inline uint32 U2AnimData::GetLastEvalFrameId() const
{
	return m_uiLastEvalFrameId;
}


inline const U2AnimStateInfo& U2AnimData::GetActiveState() const
{
	return *m_pCurStateInfo;
}
