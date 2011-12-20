//-------------------------------------------------------------------------------------------------
inline LPDIRECT3DDEVICE9 U2Dx9Renderer::GetD3DDevice() const
{
	return m_pD3DDevice9;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2Dx9Renderer::GetAdapter() const
{
	return m_uAdapter;
}

//-------------------------------------------------------------------------------------------------
inline
D3DDEVTYPE	U2Dx9Renderer::GetDevType() const
{
	return m_eD3DDevType;
}

//-------------------------------------------------------------------------------------------------
inline const D3DCAPS9*	U2Dx9Renderer::GetDeviceCaps() const
{
	return &m_d3dCaps9;
}

//-------------------------------------------------------------------------------------------------
inline U2RenderTargets* U2Dx9Renderer::GetDefaultRenderTarget() const
{
	return m_spDefaultRenderTarget;
}

//-------------------------------------------------------------------------------------------------
inline ID3DXEffectPool* U2Dx9Renderer::GetD3DEffectPool() const
{
	return m_pD3DEffectPool;
}

//-------------------------------------------------------------------------------------------------
inline bool U2Dx9Renderer::GetSWVertexCapable() const
{
	return m_bSWVertexCapable;
}

//---------------------------------------------------------------------------
inline bool U2Dx9Renderer::GetSWVertexSwitchable() const
{
	return m_bSWVertexSwitchable;
}

//-------------------------------------------------------------------------------------------------
inline U2Dx9VertexBufferMgr* U2Dx9Renderer::GetVBMgr() const
{
	return m_pVBMgr;
}

//-------------------------------------------------------------------------------------------------
inline U2Dx9IndexBufferMgr* U2Dx9Renderer::GetIBMgr() const
{
	return m_pIBMgr;
}

//-------------------------------------------------------------------------------------------------
inline U2Dx9LightMgr*	U2Dx9Renderer::GetLightMgr() const
{
	return m_pLightMgr;
}

//-------------------------------------------------------------------------------------------------
inline U2Camera*	U2Dx9Renderer::GetCurrCamera() const
{
	return m_spCamera;
}


//-------------------------------------------------------------------------------------------------
inline U2RenderTargets*	U2Dx9Renderer::GetCurrRenderTargets() const
{
	return m_pCurrRenderTarget;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2Dx9Renderer::GetProjectedTextureFlags()
{
	return ms_uProjectedTextureFlags;
}

//-------------------------------------------------------------------------------------------------
inline D3DFORMAT U2Dx9Renderer::GetAdapterFormat() const
{
	return m_eD3DAdapterFormat;
}

//-------------------------------------------------------------------------------------------------
inline D3DFORMAT U2Dx9Renderer::GetD3DFormat(DepthStencilFormat eDSFormat)
{
	//  For now, this is a straight conversion.
	//  This function is provided for future changes
	return (D3DFORMAT)eDSFormat;
}

//-------------------------------------------------------------------------------------------------
inline TCHAR* U2Dx9Renderer::GetDevTypeString() const
{
	switch (m_eD3DDevType)
	{
	case D3DDEVTYPE_HAL:
		return _T("HAL");
	case D3DDEVTYPE_REF:
		return _T("REF");
	case D3DDEVTYPE_SW:
		return _T("SW");
	}

	return _T("???");
}

//-------------------------------------------------------------------------------------------------
inline TCHAR* U2Dx9Renderer::GetBehaviorString() const
{
	m_acBehavior[0] = _T('\0');

	if (m_uBehaviorFlags & D3DCREATE_PUREDEVICE)
		_tcscat_s(m_acBehavior, 32, _T("PURE"));
	if (m_uBehaviorFlags & D3DCREATE_MULTITHREADED)
		_tcscat_s(m_acBehavior, 32, _T(" MT"));
	if (m_uBehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING)
		_tcscat_s(m_acBehavior, 32, _T(" HWVP"));
	if (m_uBehaviorFlags & D3DCREATE_MIXED_VERTEXPROCESSING)
		_tcscat_s(m_acBehavior, 32, _T(" MIXVP"));
	if (m_uBehaviorFlags & D3DCREATE_SOFTWARE_VERTEXPROCESSING)
		_tcscat_s(m_acBehavior, 32, _T(" SWVP"));

	return m_acBehavior;
}

//-------------------------------------------------------------------------------------------------
inline bool U2Dx9Renderer::GetMRTPostPixelShaderBlendingCapability() const
{
	return m_bMRTPostPixelShaderBlending;
}

//-------------------------------------------------------------------------------------------------
inline bool U2Dx9Renderer::GetFormatPostPixelShaderBlendingCapability(
	 D3DFORMAT eFormat) const 
{
	HRESULT hr = ms_pD3D9->CheckDeviceFormat(m_uAdapter, m_eD3DDevType, 
		m_eD3DAdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
		D3DRTYPE_TEXTURE, eFormat);
	return SUCCEEDED(hr);
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9Renderer::SetLight (int i, U2Light* pkLight)
{
	U2ASSERT(0 <= i && i < ms_uMaxActiveLights);
	m_aspLights[i] = pkLight;
}
//----------------------------------------------------------------------------
inline U2Light* U2Dx9Renderer::GetLight (int i)
{
	U2ASSERT(0 <= i && i < ms_uMaxActiveLights);
	return m_aspLights[i];
}

//-------------------------------------------------------------------------------------------------
inline bool U2Dx9Renderer::CheckFrameState(	
	   const char* pcCallingFunction,
	FrameState eExpectedValue) const
{
	if(m_eFrameState == eExpectedValue)
		return true;

#ifdef _DEBUG
	const char* apcCurrentValueStrings[5] = {
		"'not set'",
		"'set'",
		"'set for offscreen'",
		"'waiting for display'",
		"'set for internal'"
	};
	const char* apcNextStepStrings[5] = {
		"'BeginScene()'",
		"'EndScene()'",
		//"'EndOffscreenFrame()'",
		"'DisplayBackBuffer()'",
		//"'EndInternalFrame()"
	};
	U2ASSERT((unsigned int)m_eFrameState < 5);

	DPrintf("U2Dx9Renderer::%s> Failed - " 
		"called while frame is %s, Must call %s first.\n",
		pcCallingFunction,
		apcCurrentValueStrings[m_eFrameState],
		apcNextStepStrings[m_eFrameState]);
#endif
	return false;
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9Renderer::SetDepthClear(const float fZClear)
{
	m_fZClear = fZClear;
}

//-------------------------------------------------------------------------------------------------
inline float U2Dx9Renderer::GetDepthClear() const
{
	return m_fZClear;
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9Renderer::SetBackgroundColor(const D3DXCOLOR& kColor)
{
	unsigned char ucR = (unsigned char)kColor.r * 255;
	unsigned char ucG = (unsigned char)kColor.g * 255;
	unsigned char ucB = (unsigned char)kColor.b * 255;
	unsigned char ucA = (unsigned char)kColor.a * 255;

	m_uBgColor = D3DCOLOR_ARGB(ucA, ucR, ucG, ucB);
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9Renderer::GetBackgroundColor(D3DXCOLOR& kColor) const
{
	const float fConstant = 1.0f / 255.0f;
	kColor.a = ((m_uBgColor >> 24) & 0xff) * fConstant;
	kColor.r = ((m_uBgColor >> 16) & 0xff) * fConstant;
	kColor.g = ((m_uBgColor >> 8) & 0xff) * fConstant;
	kColor.b = (m_uBgColor & 0xff) * fConstant;
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9Renderer::SetStencilClear(unsigned int uiClear)
{
	m_uStencilClear = uiClear;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2Dx9Renderer::GetStencilClear() const
{
	return m_uStencilClear;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2Dx9Renderer::GetMaxActiveLights()
{
	return ms_uMaxActiveLights;
}
//-------------------------------------------------------------------------------------------------
inline const D3DXMATRIXA16& U2Dx9Renderer::GetProjMat() const
{
	return m_projMat;
}

//-------------------------------------------------------------------------------------------------
inline const D3DXMATRIXA16& U2Dx9Renderer::GetViewMat() const
{
	return m_viewMat;
}

//-------------------------------------------------------------------------------------------------
inline const D3DXMATRIXA16& U2Dx9Renderer::GetInvViewMat() const
{
	return m_invView;
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9Renderer::SetCurrEffectShader(U2D3DXEffectShader* pShd)
{
	m_pCurrShader = pShd;
}

//-------------------------------------------------------------------------------------------------
inline U2D3DXEffectShader* U2Dx9Renderer::GetCurrEffectShader() const
{
	return m_pCurrShader;
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9Renderer::SetHint(Hint hint, bool enable)
{
	if (enable) this->m_iHints |= hint;
	else        this->m_iHints &= ~hint;
}

//-------------------------------------------------------------------------------------------------
inline bool U2Dx9Renderer::GetHint(Hint hint) const
{
	return (m_iHints & hint) != 0;
}

//------------------------------------------------------------------------------
/**
get a vector4 of a int shadowLightIndex to set as shaderparameter
*/
inline const D3DXVECTOR4 U2Dx9Renderer::GetShadowLightIndexVector(int shadowLightIndex, float value)
{
	// FIXME: lightIndex should be a bit field (shader must support this?),
	// so there can be 32 shadow casting lights at once,
	// or 8 shadow casting lights in a one channel map (e.g. only alpha write)
	D3DXVECTOR4 indexColor;
	indexColor.x = (shadowLightIndex == 0) ? value : 0.0f;
	indexColor.y = (shadowLightIndex == 1) ? value : 0.0f;
	indexColor.z = (shadowLightIndex == 2) ? value : 0.0f;
	indexColor.w = (shadowLightIndex == 3) ? value : 0.0f;
	return indexColor;
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9Renderer::SetVertexRange(int firstVertex, int numVertices)
{
	m_iVertexRangeFirst = firstVertex;
	m_iNumVertexRange = numVertices;
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9Renderer::SetIndexRange(int firstIdx, int numIndices)
{
	m_iIndexRangeFirst = firstIdx;
	m_iNumIndexRange = numIndices;
}


//-------------------------------------------------------------------------------------------------
inline D3DXMATRIX& U2Dx9Renderer::GetTransform(TransformType eType)
{
	return m_aTM[eType];
}
