//-------------------------------------------------------------------------------------------------
inline void U2Frame::UpdateVariable(U2Variable* pVar)
{
	U2VariableMgr::Instance()->SetGlobalVariable(*pVar);
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2Frame::GetSequenceShaderAndIncrement()
{
	return m_uiFrameSeqShaderIdx++;
}

//-------------------------------------------------------------------------------------------------
inline bool U2Frame::IsOpen() const
{
	return m_bIsOpen;
}

//-------------------------------------------------------------------------------------------------
inline void U2Frame::SetFilename(const U2DynString& szFilename)
{
	m_szXmlFilename = szFilename;
}

//-------------------------------------------------------------------------------------------------
inline const U2DynString& U2Frame::GetFilename() const
{
	return m_szXmlFilename;
}

//-------------------------------------------------------------------------------------------------
inline void U2Frame::AddFrameSection(U2FrameSection* pFrameSection)
{
	m_frameSections.AddElem(pFrameSection);
}

//-------------------------------------------------------------------------------------------------
inline const U2ObjVec<U2FrameSectionPtr>& U2Frame::GetFrameSections() const
{
	return m_frameSections;
}

//-------------------------------------------------------------------------------------------------
inline U2FrameSection* U2Frame::GetFrameSection(int i) const
{
	return m_frameSections[i];
}

//-------------------------------------------------------------------------------------------------
inline void U2Frame::AddVariable(U2Variable* pVar)
{
	U2VariableMgr::Instance()->SetGlobalVariable(*pVar);
	m_varHandles.AddElem(pVar->GetHandle());
}

//-------------------------------------------------------------------------------------------------
inline const U2PrimitiveVec<U2Variable::Handle>& U2Frame::GetVariableHandles() const
{
	return m_varHandles;
}

//-------------------------------------------------------------------------------------------------
inline const U2ObjVec<U2FrameShaderPtr>& U2Frame::GetShaders() const
{
	return m_frameShaders;
}

//-------------------------------------------------------------------------------------------------
inline U2FrameShader& U2Frame::GetShader(unsigned int idx) const
{
	return *m_frameShaders[idx];
}

//-------------------------------------------------------------------------------------------------
inline U2FrameShader& U2Frame::GetShader(const char* shadername) const
{
	return GetShader(this->FindFrameShaderIdx(shadername));
}


//-------------------------------------------------------------------------------------------------
inline U2FrameRenderTargetPtr& U2Frame::GetRenderTarget(unsigned int idx)
{
	return m_renderTargets[idx];
}


//-------------------------------------------------------------------------------------------------
inline void U2Frame::AddShader(U2FrameShader* pShader)
{
	pShader->Validate();
	m_frameShaders.AddElem(pShader);
	pShader->SetBucketIdx(m_frameShaders.FilledSize() - 1);
}

//-------------------------------------------------------------------------------------------------
inline void U2Frame::AddRenderTarget(U2FrameRenderTarget* pRT)
{
	pRT->Validate();
	m_renderTargets.AddElem(pRT);
}
