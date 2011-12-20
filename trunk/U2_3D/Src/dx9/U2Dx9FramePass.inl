inline void	U2FramePass::SetFrame(U2Frame* pFrame)
{
	m_pOwnerFrame = pFrame;
}

//-------------------------------------------------------------------------------------------------
inline U2Frame* U2FramePass::GetFrame() const
{
	return m_pOwnerFrame;
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePass::SetShaderAlias(const U2DynString& szShaderAlias)
{
	m_szShaderAlias = szShaderAlias;
}

//-------------------------------------------------------------------------------------------------
inline const U2DynString& U2FramePass::GetShaderAlias() const
{
	return m_szShaderAlias;
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePass::SetTechnique(const U2DynString& tec)
{
	m_szTechnique = tec;
}

//-------------------------------------------------------------------------------------------------
inline const U2DynString& U2FramePass::GetTechnique() const
{
	return m_szTechnique;
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePass::SetClearFlags(unsigned int uiClearFlags)
{
	m_uiClearFlags = uiClearFlags;
}

//-------------------------------------------------------------------------------------------------
inline uint32 U2FramePass::GetClearFlags() const
{
	return m_uiClearFlags;
}

inline void U2FramePass::SetRenderTargetName(uint32 idx, const U2DynString& szName)
{
	m_renderTargetNames[idx] = szName;
}
inline const U2DynString& U2FramePass::GetRenderTargetName(uint32 idx) const
{
	return m_renderTargetNames[idx];
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePass::SetOcclusionQuery(bool b)
{
	m_bOcclusionQuery = b;
}

//-------------------------------------------------------------------------------------------------
inline bool U2FramePass::GetOcculsionQuery() const
{
	return m_bOcclusionQuery;
}

/// set the "shadow enabled condition" flag
inline void U2FramePass::SetShadowEnabled(bool b)
{
	m_bShadowEnabled = b;
}

/// get the "shadow enabled condition" flag
inline bool U2FramePass::GetShadowEnabled() const
{
	return m_bShadowEnabled;
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePass::SetDrawGui(bool bDraw)
{
	m_bDrawGui = bDraw;
}

//-------------------------------------------------------------------------------------------------
inline bool U2FramePass::GetDrawGui() const
{
	return m_bDrawGui;
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePass::SetDrawFullscreenQuad(bool b)
{
	m_bDrawFullscreenQuad = b;
}

//-------------------------------------------------------------------------------------------------
inline bool U2FramePass::GetDrawFullscreenQuad() const
{
	return m_bDrawFullscreenQuad;
}

//------------------------------------------------------------------------------
/**
*/
inline void U2FramePass::SetShadowTechnique(ShadowTechnique t)
{
	m_eShadowTeq = t;
}

//------------------------------------------------------------------------------
/**
*/
inline U2FramePass::ShadowTechnique U2FramePass::GetShadowTechnique() const
{
	return this->m_eShadowTeq;
}

inline U2FramePass::ShadowTechnique U2FramePass::StringToShadowTechnique(const TCHAR* str)
{
	U2ASSERT(str);
	if (_tcscmp(str, _T("NoShadows")) == 0)      return NO_SHADOW;
	if (_tcscmp(str, _T("Simple")) == 0)         return SIMPLE;
	if (_tcscmp(str, _T("MultiLight")) == 0)     return MULTI_LIGHT;
	FDebug("nRpPass::StringToShadowTechnique: Invalid string '%s'!", str);
	return NO_SHADOW;
}


//-------------------------------------------------------------------------------------------------
inline void U2FramePass::AddConstantShaderParam(U2FxShaderState::Param eParam, 
											const U2ShaderArg& arg)
{
	m_shaderAttb.SetArg(eParam, arg);
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePass::AddVariableShaderParam(const U2DynString& szVar, 
											U2FxShaderState::Param eParam, 
											const U2ShaderArg& arg)
{
	AddConstantShaderParam(eParam, arg);

	U2Variable::Handle h = U2VariableMgr::Instance()->GetVariableHandleByName(szVar.Str());
	U2Variable *pNewVar = U2_NEW U2Variable(h, int(eParam));
	m_varContext.AddVariable(*pNewVar);
}

//-------------------------------------------------------------------------------------------------
inline const U2ShaderAttribute& U2FramePass::GetShaderAttb() const
{
	return m_shaderAttb;
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePass::AddFramePhase(U2FramePhase* pPhase)
{
	m_framePhases.AddElem(pPhase);
}

//-------------------------------------------------------------------------------------------------
inline const U2ObjVec<U2FramePhasePtr>& U2FramePass::GetFramePhases() const
{
	return m_framePhases;
}

//-------------------------------------------------------------------------------------------------
inline U2FramePhase* U2FramePass::GetFramePhase(unsigned int idx) const
{
	return m_framePhases[idx];
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePass::SetDepthClear(const float fZClear)
{
	//U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	//U2ASSERT(pRenderer);
	//pRenderer->SetDepthClear(fZClear);
	m_fClearDepth = fZClear;
}

//-------------------------------------------------------------------------------------------------
inline float U2FramePass::GetDepthClear() const
{
	/*U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	U2ASSERT(pRenderer);
	return pRenderer->GetDepthClear();*/
	return m_fClearDepth;
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePass::SetBackgroundColor(const D3DXCOLOR& kColor)
{
	//U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	//U2ASSERT(pRenderer);
	//pRenderer->SetBackgroundColor(kColor);
	m_bgColor = kColor;
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePass::GetBackgroundColor(D3DXCOLOR& kColor) const
{
	//U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	//U2ASSERT(pRenderer);
	//return pRenderer->GetBackgroundColor(kColor);
	kColor = m_bgColor;
}

//-------------------------------------------------------------------------------------------------
inline void U2FramePass::SetStencilClear(unsigned int uiClear)
{
	//U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	//U2ASSERT(pRenderer);
	//pRenderer->SetStencilClear(uiClear);
	m_iClearStencil = uiClear;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2FramePass::GetStencilClear() const
{
	/*U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();
	U2ASSERT(pRenderer);
	return pRenderer->GetStencilClear();*/
	return m_iClearStencil;
}

//-------------------------------------------------------------------------------------------------
inline U2RenderTargetPtr U2FramePass::GetRenderTarget() const
{
	return m_spRenderTarget;
}


//-------------------------------------------------------------------------------------------------
inline void U2FramePass::SetDepthStencil(bool bSM)
{
	m_bDepthStencil = bSM;
}