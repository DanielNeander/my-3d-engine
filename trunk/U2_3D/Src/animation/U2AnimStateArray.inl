inline U2AnimStateArray::U2AnimStateArray()
:m_stateArray(0, 0)
{

}

inline U2AnimStateArray::~U2AnimStateArray()
{	

}

inline void U2AnimStateArray::Begin(uint32 num)
{
	m_stateArray.Resize(num);
}

inline void U2AnimStateArray::SetState(int idx, const U2AnimStateInfo& state)
{
	m_stateArray.SetElem(idx, (U2AnimStateInfo*)&state);
}

inline void U2AnimStateArray::End()
{

}

inline uint32 U2AnimStateArray::GetNumStates() const
{
	return m_stateArray.Size();	
}

inline U2AnimStateInfo& U2AnimStateArray::GetAnimStateInfo(int idx) const
{
	return *m_stateArray[idx];
}

inline U2AnimStateInfo* U2AnimStateArray::FindAnimStateInfo(const U2DynString& name) const
{
	uint32 i;
	for(i=0; i < m_stateArray.Size(); ++i)
	{
		U2AnimStateInfo& state = *m_stateArray[i];
		if(state.GetName() == name)
		{
			return &state;
		}
	}

	return 0;
}

inline int U2AnimStateArray::FindStateIdx(const U2DynString& name) const
{
	uint32 i;
	for(i=0; i < m_stateArray.Size(); ++i)
	{
		U2AnimStateInfo& state = *m_stateArray[i];
		if(state.GetName() == name)
		{
			return i;
		}	
	}

	return -1;
	
}
