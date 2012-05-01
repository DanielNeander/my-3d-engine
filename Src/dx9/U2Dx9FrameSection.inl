//-------------------------------------------------------------------------------------------------
inline void	U2FrameSection::SetFrame(U2Frame* pFrame)
{
	m_pOwnerFrame = pFrame;
}

//-------------------------------------------------------------------------------------------------
inline U2Frame* U2FrameSection::GetFrame() const
{
	return m_pOwnerFrame;
}

//-------------------------------------------------------------------------------------------------
inline void U2FrameSection::AddFramePass(U2FramePass* pPass)
{
	m_framePasses.AddElem(pPass);
}

//-------------------------------------------------------------------------------------------------
inline const U2ObjVec<U2FramePassPtr>& U2FrameSection::GetFramePasses() const
{
	return m_framePasses;
}

//-------------------------------------------------------------------------------------------------
inline U2FramePass& U2FrameSection::GetPass(unsigned idx) const
{
	return *SmartPtrCast(U2FramePass, m_framePasses[idx]);
}

//-------------------------------------------------------------------------------------------------
inline bool U2FrameSection::FrameInBegin() const
{
	return m_bFrameInBegin;
}
//-------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------
/**
Begin rendering the section. This will validate all embedded objects.
Returns the number of scene passes in the section.
After begin, each pass should be "rendered" recursively.
*/
inline uint32 U2FrameSection::Begin()
{
	U2ASSERT(!m_bFrameInBegin);

	Validate();
	m_bFrameInBegin = true;
	return m_framePasses.FilledSize();
}

//------------------------------------------------------------------------------
/**
Finish rendering the render path.
*/
inline void U2FrameSection::End()
{
	U2ASSERT(m_bFrameInBegin);
	m_bFrameInBegin = false;
}
