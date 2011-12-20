//-------------------------------------------------------------------------------------------------
inline void U2FrameSequence::SetFrame(U2Frame* pFrame)
{
	m_pOwnerFrame = pFrame;	
}

//-------------------------------------------------------------------------------------------------
inline U2Frame* U2FrameSequence::GetFrame()
{
	return m_pOwnerFrame;
}

//-------------------------------------------------------------------------------------------------
inline void U2FrameSequence::SetShaderAlias(const U2DynString& p)
{
	m_szShaderAlias = p;
}			


inline const U2DynString& U2FrameSequence::GetShaderAlias() const
{
	return m_szShaderAlias;
}

//-------------------------------------------------------------------------------------------------
inline void U2FrameSequence::SetTechnique(const U2DynString& tec)
{
	m_szTechnique = tec;
}

//-------------------------------------------------------------------------------------------------
inline const U2DynString& U2FrameSequence::GetTechnique() const
{
	return m_szTechnique;
}

//-------------------------------------------------------------------------------------------------
inline void U2FrameSequence::SetFirstLightAlphaEnabled(bool bEnable)
{
	m_bFirstLightAlphaEnabled = bEnable;
}

//-------------------------------------------------------------------------------------------------
inline bool U2FrameSequence::GetFirstLightAlphaEnabled() const
{
	return m_bFirstLightAlphaEnabled;
}

//-------------------------------------------------------------------------------------------------
inline void U2FrameSequence::SetShaderUpdateEnabled(bool bUpdate)
{
	m_bShaderUpdatesEnabled = bUpdate;
}

//-------------------------------------------------------------------------------------------------
inline bool U2FrameSequence::GetShaderUpdatesEnabled() const
{
	return m_bShaderUpdatesEnabled;
}

//-------------------------------------------------------------------------------------------------
inline void U2FrameSequence::SetModelViewProjOnlyHint(bool bUpdate)
{
	m_bOnlyModelViewProj = bUpdate;
}

//-------------------------------------------------------------------------------------------------
inline bool U2FrameSequence::GetModelViewProjOnlyHint() const
{
	return m_bOnlyModelViewProj;
}

//-------------------------------------------------------------------------------------------------
inline void U2FrameSequence::AddConstantShaderParam(U2FxShaderState::Param eParam, 
												const U2ShaderArg& arg)
{
	m_shaderAttb.SetArg(eParam, arg);
}

//-------------------------------------------------------------------------------------------------
inline void U2FrameSequence::AddVariableShaderParam(const U2DynString& szVar, 
													   U2FxShaderState::Param eParam, 
														const U2ShaderArg& arg)
{
	m_shaderAttb.SetArg(eParam, arg);

	U2Variable::Handle varHandle = U2VariableMgr::Instance()->
		GetVariableHandleByName(szVar.Str());
	U2Variable* pNewVar = U2_NEW U2Variable(varHandle, int(eParam));
	m_varContext.AddVariable(*pNewVar);
}

//-------------------------------------------------------------------------------------------------
