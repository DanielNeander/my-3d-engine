inline U2WireframeState::U2WireframeState()
{
	m_uFlags = 0;
	SetWireFrame(false);
}


inline void U2WireframeState::SetWireFrame(bool bWireFrame)
{
	SetBit(bWireFrame, WIREFRAME_MASK);
}

inline bool U2WireframeState::GetWireFrame() const 
{
	return GetBit(WIREFRAME_MASK);
}

