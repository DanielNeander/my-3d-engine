inline void U2Dx9RenderStateMgr::GetCameraNearAndFar(float& fNear, float& fFar)
{
	fNear = m_fCameraNear;
	fFar = m_fCameraFar;
}
//---------------------------------------------------------------------------
inline void U2Dx9RenderStateMgr::SetCameraNearAndFar(float fNear, float fFar)
{
	m_fCameraNear = fNear;
	m_fCameraFar = fFar;
	m_fCameraDepthRange = fFar - fNear;
}


inline U2Dx9RenderStateMgr::U2Dx9RSBlock::U2Dx9RSBlock() : 
m_uCurrVal(0x7FFFFFFF), 
m_uPrevVal(0x7FFFFFFF)
{
	//* *//
}
//---------------------------------------------------------------------------
inline void U2Dx9RenderStateMgr::U2Dx9RSBlock::SetValue(
	unsigned int uiValue, bool bSave)
{
	if (bSave)
		m_uPrevVal = m_uCurrVal;
	m_uCurrVal = uiValue;
}
//---------------------------------------------------------------------------
inline void U2Dx9RenderStateMgr::U2Dx9RSBlock::InitValue(
	unsigned int uiValue)
{
	m_uCurrVal = uiValue;
	m_uPrevVal = uiValue;
}
//---------------------------------------------------------------------------
inline void U2Dx9RenderStateMgr::U2Dx9RSBlock::SaveValue()
{
	m_uPrevVal = m_uCurrVal;
}
//---------------------------------------------------------------------------
inline void U2Dx9RenderStateMgr::U2Dx9RSBlock::RestoreValue()
{
	m_uCurrVal = m_uPrevVal;
}
//---------------------------------------------------------------------------
inline unsigned int U2Dx9RenderStateMgr::U2Dx9RSBlock::GetValue() const
{
	return m_uCurrVal;
}
//---------------------------------------------------------------------------
inline unsigned int U2Dx9RenderStateMgr::U2Dx9RSBlock::GetPrevValue() 
const
{
	return m_uPrevVal;
}

inline VOID U2Dx9RenderStateMgr::DisableAlpha()
{
	
}

inline unsigned int U2Dx9RenderStateMgr::GetSrcAlphaCaps()
{
	return m_uSrcAlphaCaps;
}


inline unsigned int U2Dx9RenderStateMgr::GetDestAlphaCaps()
{
	return m_uDestAlphaCaps;
}

inline unsigned int U2Dx9RenderStateMgr::GetD3DBlendMode(U2AlphaState::BlendMode eBlend)
{
	return m_uAlphaBlendModes[eBlend];
}

inline unsigned int U2Dx9RenderStateMgr::GetD3DTestMode(U2AlphaState::TestMode eTest)
{
	return m_uAlphaTestModes[eTest];
}

inline void U2Dx9RenderStateMgr::SetBoneCount(unsigned short usNumBones)
{
	SetRenderState(D3DRS_VERTEXBLEND, m_d3dVertBlendFlags[usNumBones]);
}

inline void U2Dx9RenderStateMgr::SetPixelShader(LPDIRECT3DPIXELSHADER9 pPS, 
											 bool bSave)
{
	if(bSave)
		m_pPrevPS = m_pCurrPS;
	if(m_pCurrPS != pPS)
	{
		m_pCurrPS = pPS;
		m_pD3DDev->SetPixelShader(m_pCurrPS);
	}
}


inline LPDIRECT3DPIXELSHADER9 U2Dx9RenderStateMgr::GetPixelShader() const 
{
	return m_pCurrPS;
}

inline void U2Dx9RenderStateMgr::RestorePixelShader()
{
	SetPixelShader(m_pPrevPS);
}


inline void U2Dx9RenderStateMgr::ClearPixelShader(LPDIRECT3DPIXELSHADER9 pPS)
{
	if(m_pCurrPS == pPS)
	{
		m_pCurrPS = 0;
		m_pD3DDev->SetPixelShader(0);
	}

	if(m_pPrevPS == pPS)
	{
		m_pPrevPS = 0;
	}
}

inline void U2Dx9RenderStateMgr::SetVertexShader(LPDIRECT3DVERTEXSHADER9 pVS, 
												 bool bSave /* = false */)
{
	if(bSave)
		m_pPrevVS = m_pCurrVS;
	if(m_pCurrVS != pVS)
	{
		m_pCurrVS = pVS;
		m_pD3DDev->SetVertexShader(m_pCurrVS);
	}
}

inline LPDIRECT3DVERTEXSHADER9 U2Dx9RenderStateMgr::GetVertexShader() const 
{
	return m_pCurrVS;
}


inline void U2Dx9RenderStateMgr::RestoreVertexShader()
{
	SetVertexShader(m_pPrevVS);
}

inline void U2Dx9RenderStateMgr::ClearVertexShader(LPDIRECT3DVERTEXSHADER9 pVS)
{
	if(m_pCurrVS == pVS)
	{
		m_pCurrVS = 0;
		m_pD3DDev->SetVertexShader(0);
	}
	if(m_pPrevVS == pVS)
		m_pPrevVS = 0;
}


inline void U2Dx9RenderStateMgr::SetFVF(unsigned int uiFVF, bool bSave /* = false */)
{
	if(uiFVF != 0 && (m_bVertDecl || m_uCurrFVF != uiFVF))
	{
		m_bVertDecl = false;
		if(bSave)
			m_uPrevFVF = m_uCurrFVF;
		m_uCurrFVF = uiFVF;
		m_pD3DDev->SetFVF(m_uCurrFVF);
	}
}


inline unsigned int U2Dx9RenderStateMgr::GetFVF() const 
{
	if(m_bVertDecl)
		return 0;
	else 
		return m_uCurrFVF;
}


inline void U2Dx9RenderStateMgr::RestoreFVF()
{
	SetFVF(m_uPrevFVF);
}


inline void U2Dx9RenderStateMgr::ClearFVF(unsigned int uFVF)
{
	if(uFVF == 0 || m_bVertDecl)
		return;

	if(m_uCurrFVF == uFVF)
	{
		m_uCurrFVF = D3DFVF_XYZ;
		m_pD3DDev->SetFVF(D3DFVF_XYZ);
	}

	if(m_uPrevFVF == uFVF)
	{
		m_uPrevFVF = D3DFVF_XYZ;
	}
}


inline void U2Dx9RenderStateMgr::SetVertexDecl(LPDIRECT3DVERTEXDECLARATION9 pVertDecl, 
												bool bSave /* = false */)
{
	if(pVertDecl != NULL && (!m_bVertDecl || m_pCurrVertDecl != pVertDecl))
	{
		m_bVertDecl = true;
		if(bSave)
			m_pPrevVertDecl = m_pCurrVertDecl;
		m_pCurrVertDecl = pVertDecl;
		m_pD3DDev->SetVertexDeclaration(m_pCurrVertDecl);
	}
}


inline LPDIRECT3DVERTEXDECLARATION9 U2Dx9RenderStateMgr::GetVertDecl() const 
{
	if(m_bVertDecl)
		return m_pCurrVertDecl;
	else 
		return NULL;
}


inline void U2Dx9RenderStateMgr::RestoreVertDecl()
{
	SetVertexDecl(m_pPrevVertDecl);
}


inline void U2Dx9RenderStateMgr::ClearVertDecl(LPDIRECT3DVERTEXDECLARATION9 pVertDecl)
{
	if(!pVertDecl || !m_bVertDecl) 
		return;

	if(m_pCurrVertDecl == pVertDecl)
	{
		m_pCurrVertDecl = 0;
		m_pD3DDev->SetVertexDeclaration(0);
	}

	if(m_pPrevVertDecl == pVertDecl)
	{
		m_pPrevVertDecl = 0;
	}
}


inline void U2Dx9RenderStateMgr::SetRenderState(D3DRENDERSTATETYPE eState, 
												unsigned int uValue, bool bSave)
{
	if(m_RSBlocks[eState].GetValue() != uValue)
	{
		m_pD3DDev->SetRenderState(eState, uValue);		
	}

	m_RSBlocks[eState].SetValue(uValue, bSave);	
}

inline void U2Dx9RenderStateMgr::SetRenderState(D3DRENDERSTATETYPE eState, 
												unsigned int uValue)
{
	if(m_RSBlocks[eState].GetValue() != uValue)
	{
		m_pD3DDev->SetRenderState(eState, uValue);
		m_RSBlocks[eState].SetValue(uValue, false);
	}
}

inline unsigned int U2Dx9RenderStateMgr::GetRenderState(D3DRENDERSTATETYPE eState) const
{
	return m_RSBlocks[eState].GetValue();
}


inline void U2Dx9RenderStateMgr::RestoreRenderState(D3DRENDERSTATETYPE eState)
{
	SetRenderState(eState, m_RSBlocks[eState].GetPrevValue());
}


inline void U2Dx9RenderStateMgr::SetTextureStageState(unsigned int uiStage, 
													  D3DTEXTURESTAGESTATETYPE eState, 
													  unsigned int uiValue, bool bSave)
{
	if(m_RSBlocks[uiStage].GetValue() != uiValue) 
		m_pD3DDev->SetTextureStageState(uiStage, eState, uiValue);

	m_TSRSBlocks[uiStage][eState].SetValue(uiValue, bSave);	
}

inline void U2Dx9RenderStateMgr::SetTextureStageState(unsigned int uiStage, 
													  D3DTEXTURESTAGESTATETYPE eState, 
													  unsigned int uiValue)
{
	if(m_RSBlocks[uiStage].GetValue() != uiValue) 
		m_pD3DDev->SetTextureStageState(uiStage, eState, uiValue);

	m_TSRSBlocks[uiStage][eState].SetValue(uiValue, false);	
}

inline unsigned int U2Dx9RenderStateMgr::GetTextureStageState(unsigned int uiStage, 
															  D3DTEXTURESTAGESTATETYPE eState) 
															  const
{
	return m_TSRSBlocks[uiStage][eState].GetValue();
}

inline void U2Dx9RenderStateMgr::RestoreTextureStageState(unsigned int uiStage, 
														  D3DTEXTURESTAGESTATETYPE eState)
{
	SetTextureStageState(uiStage, eState, m_TSRSBlocks[uiStage][eState].GetPrevValue());
}

inline void U2Dx9RenderStateMgr::SetSamplerState(unsigned int uiStage, D3DSAMPLERSTATETYPE eState, 
												 unsigned int uiValue, bool bSave)
{
	if(m_SSRSBlocks[uiStage][eState].GetValue() != uiValue)
	{
		m_pD3DDev->SetSamplerState(uiStage, eState, uiValue);
		m_SSRSBlocks[uiStage][eState].SetValue(uiValue, bSave);
	}
}

inline void U2Dx9RenderStateMgr::SetSamplerState(unsigned int uiStage,
							D3DSAMPLERSTATETYPE eState, unsigned int uiValue)
{
	if(m_SSRSBlocks[uiStage][eState].GetValue() != uiValue)
	{
		m_pD3DDev->SetSamplerState(uiStage, eState, uiValue);
		m_SSRSBlocks[uiStage][eState].SetValue(uiValue, false);
	}	
}


inline unsigned int U2Dx9RenderStateMgr::GetSamplerState(unsigned int uiStage, 
														 D3DSAMPLERSTATETYPE eState)
{
	return m_SSRSBlocks[uiStage][eState].GetValue();
}


inline void U2Dx9RenderStateMgr::RestoreSamplerState(unsigned int uiStage, 
													 D3DSAMPLERSTATETYPE eState)
{
	SetSamplerState(uiStage, eState, m_SSRSBlocks[uiStage][eState].GetPrevValue());
}


inline void U2Dx9RenderStateMgr::InitTextures()
{
	for(uint32 u = 0; u < MAX_TEXTURE_STAGES ; ++u)
		m_apTSTextures[u] = 0;
}


inline void U2Dx9RenderStateMgr::SetTexture(uint32 uStage, LPDIRECT3DBASETEXTURE9 pTex)
{
	U2ASSERT(uStage < MAX_TEXTURE_STAGES);
	if(m_apTSTextures[uStage] != pTex)
	{
		m_apTSTextures[uStage] = pTex;
		HRESULT hr = m_pD3DDev->SetTexture(uStage, pTex);
		U2ASSERT(SUCCEEDED(hr));
	}
}


inline LPDIRECT3DBASETEXTURE9 U2Dx9RenderStateMgr::GetTexture(unsigned int uStage)
{
	U2ASSERT( uStage < MAX_TEXTURE_STAGES);
	return m_apTSTextures[uStage];
}


inline void U2Dx9RenderStateMgr::ClearTexture(LPDIRECT3DBASETEXTURE9 pkTexture)
{
	if(!pkTexture)
		return;
	for(uint32 uStage = 0; uStage <= MAX_TEXTURE_STAGES; uStage++)
	{
		if(m_apTSTextures[uStage] == pkTexture)
		{
			m_apTSTextures[uStage] = 0;
			m_pD3DDev->SetTexture(uStage, 0);
		}
	}
}


inline bool U2Dx9RenderStateMgr::SetVertexShaderConstantB(
	unsigned int uiStartRegister, const BOOL* pbConstantData, 
	unsigned int uiBoolCount, bool bSave)
{
	return m_spShaderConstMgr->SetVertexShaderConstantB(
		uiStartRegister, pbConstantData, uiBoolCount, bSave);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::GetVertexShaderConstantB(
	unsigned int uiStartRegister, BOOL* pbConstantData, 
	unsigned int uiBoolCount)
{
	return m_spShaderConstMgr->GetVertexShaderConstantB(
		uiStartRegister, pbConstantData, uiBoolCount);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::RestoreVertexShaderConstantB(
	unsigned int uiStartRegister, unsigned int uiBoolCount)
{
	return m_spShaderConstMgr->RestoreVertexShaderConstantB(
		uiStartRegister, uiBoolCount);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::SetVertexShaderConstantF(
	unsigned int uiStartRegister, const float* pfConstantData, 
	unsigned int uiFloat4Count, bool bSave)
{
	return m_spShaderConstMgr->SetVertexShaderConstantF(
		uiStartRegister, pfConstantData, uiFloat4Count, bSave);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::GetVertexShaderConstantF(
	unsigned int uiStartRegister, float* pfConstantData, 
	unsigned int uiFloat4Count)
{
	return m_spShaderConstMgr->GetVertexShaderConstantF(
		uiStartRegister, pfConstantData, uiFloat4Count);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::RestoreVertexShaderConstantF(
	unsigned int uiStartRegister, unsigned int uiFloat4Count)
{
	return m_spShaderConstMgr->RestoreVertexShaderConstantF(
		uiStartRegister, uiFloat4Count);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::SetVertexShaderConstantI(
	unsigned int uiStartRegister, const int* piConstantData, 
	unsigned int uiInt4Count, bool bSave)
{
	return m_spShaderConstMgr->SetVertexShaderConstantI(
		uiStartRegister, piConstantData, uiInt4Count, bSave);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::GetVertexShaderConstantI(
	unsigned int uiStartRegister, int* piConstantData, 
	unsigned int uiInt4Count)
{
	return m_spShaderConstMgr->GetVertexShaderConstantI(
		uiStartRegister, piConstantData, uiInt4Count);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::RestoreVertexShaderConstantI(
	unsigned int uiStartRegister, unsigned int uiInt4Count)
{
	return m_spShaderConstMgr->RestoreVertexShaderConstantI(
		uiStartRegister, uiInt4Count);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::SetVertexShaderConstant(
	unsigned int uiStartRegister, const float* pfConstantData, 
	unsigned int uiFloat4Count, bool bSave)
{
	return m_spShaderConstMgr->SetVertexShaderConstantF(
		uiStartRegister, pfConstantData, uiFloat4Count, bSave);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::GetVertexShaderConstant(
	unsigned int uiStartRegister, float* pfConstantData, 
	unsigned int uiFloat4Count)
{
	return m_spShaderConstMgr->GetVertexShaderConstantF(
		uiStartRegister, pfConstantData, uiFloat4Count);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::RestoreVertexShaderConstant(
	unsigned int uiStartRegister, unsigned int uiFloat4Count)
{
	return m_spShaderConstMgr->RestoreVertexShaderConstantF(
		uiStartRegister, uiFloat4Count);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::SetPixelShaderConstantB(
	unsigned int uiStartRegister, const BOOL* pbConstantData, 
	unsigned int uiBoolCount, bool bSave)
{
	return m_spShaderConstMgr->SetPixelShaderConstantB(
		uiStartRegister, pbConstantData, uiBoolCount, bSave);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::GetPixelShaderConstantB(
	unsigned int uiStartRegister, BOOL* pbConstantData, 
	unsigned int uiBoolCount)
{
	return m_spShaderConstMgr->GetPixelShaderConstantB(
		uiStartRegister, pbConstantData, uiBoolCount);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::RestorePixelShaderConstantB(
	unsigned int uiStartRegister, unsigned int uiBoolCount)
{
	return m_spShaderConstMgr->RestorePixelShaderConstantB(
		uiStartRegister, uiBoolCount);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::SetPixelShaderConstantF(
	unsigned int uiStartRegister, const float* pfConstantData, 
	unsigned int uiFloat4Count, bool bSave)
{
	return m_spShaderConstMgr->SetPixelShaderConstantF(
		uiStartRegister, pfConstantData, uiFloat4Count, bSave);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::GetPixelShaderConstantF(
	unsigned int uiStartRegister, float* pfConstantData, 
	unsigned int uiFloat4Count)
{
	return m_spShaderConstMgr->GetPixelShaderConstantF(
		uiStartRegister, pfConstantData, uiFloat4Count);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::RestorePixelShaderConstantF(
	unsigned int uiStartRegister, unsigned int uiFloat4Count)
{
	return m_spShaderConstMgr->RestorePixelShaderConstantF(
		uiStartRegister, uiFloat4Count);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::SetPixelShaderConstantI(
	unsigned int uiStartRegister, const int* piConstantData, 
	unsigned int uiInt4Count, bool bSave)
{
	return m_spShaderConstMgr->SetPixelShaderConstantI(
		uiStartRegister, piConstantData, uiInt4Count, bSave);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::GetPixelShaderConstantI(
	unsigned int uiStartRegister, int* piConstantData, 
	unsigned int uiInt4Count)
{
	return m_spShaderConstMgr->GetPixelShaderConstantI(
		uiStartRegister, piConstantData, uiInt4Count);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::RestorePixelShaderConstantI(
	unsigned int uiStartRegister, unsigned int uiInt4Count)
{
	return m_spShaderConstMgr->RestorePixelShaderConstantI(
		uiStartRegister, uiInt4Count);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::SetPixelShaderConstant(
	unsigned int uiStartRegister, const float* pfConstantData, 
	unsigned int uiFloat4Count, bool bSave)
{
	return m_spShaderConstMgr->SetPixelShaderConstantF(
		uiStartRegister, pfConstantData, uiFloat4Count, bSave);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::GetPixelShaderConstant(
	unsigned int uiStartRegister, float* pfConstantData, 
	unsigned int uiFloat4Count)
{
	return m_spShaderConstMgr->GetPixelShaderConstantF(
		uiStartRegister, pfConstantData, uiFloat4Count);
}
//---------------------------------------------------------------------------
inline bool U2Dx9RenderStateMgr::RestorePixelShaderConstant(
	unsigned int uiStartRegister, unsigned int uiFloat4Count)
{
	return m_spShaderConstMgr->RestorePixelShaderConstantF(
		uiStartRegister, uiFloat4Count);
}

inline void U2Dx9RenderStateMgr::CommitShaderConsts()
{
	m_spShaderConstMgr->Commit();	
}