inline U2BonePalette::U2BonePalette() 
:m_joindIdxArray(0, 0)
{

}


inline void U2BonePalette::BeginJoints(int num)
{
	m_joindIdxArray.Resize(num);
}


inline void U2BonePalette::SetJointIdx(int paletteIdx, int jointIdx)
{
	m_joindIdxArray.SetElem(paletteIdx, jointIdx);
}


inline void U2BonePalette::EndJoints()
{

}

inline int U2BonePalette::GetNumJoints() const
{
	return m_joindIdxArray.Size();
}


inline int U2BonePalette::GetJointIdx(int paletteIdx) const
{
	return m_joindIdxArray.GetElem(paletteIdx);
}