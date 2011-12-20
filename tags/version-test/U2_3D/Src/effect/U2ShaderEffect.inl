inline 
uint32 U2ShaderEffect::GetPassCnt() const
{
	return m_uPassCnt;
}

inline 
U2AlphaState* U2ShaderEffect::GetBlending(int iPass)
{
	U2ASSERT(0 <= iPass && iPass < m_alphaStateArray.Size());
	return m_alphaStateArray.GetElem(iPass);
}


inline 
U2VertexShaderPtr U2ShaderEffect::GetVShader(int iPass)
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_vsArray.GetElem(iPass);

}

inline 
U2VertexProgramPtr U2ShaderEffect::GetVProgram(int iPass)
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_vsArray.GetElem(iPass)->GetGPUProgram();
}


inline 
const U2DynString& U2ShaderEffect::GetVSName(int iPass) const
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_vsArray.GetElem(iPass)->GetShaderName();
}


inline 
uint32 U2ShaderEffect::GetVConstantQuantity(int iPass) const
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	U2VertexProgram* pVProgram = m_vsArray.GetElem(iPass)->GetGPUProgram();
	return pVProgram ? pVProgram->GetUserConstantCnt() : 0;
}

inline
U2ShaderUserConstant* U2ShaderEffect::GetVConstant(int iPass, const U2DynString& name)
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	U2VertexProgram* pVProgram = m_vsArray.GetElem(iPass)->GetGPUProgram();
	return pVProgram ? pVProgram->GetUserConstant(name.Str()) : 0;
}


inline
U2ShaderUserConstant* U2ShaderEffect::GetVConstant(int iPass, int idx)
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	U2VertexProgram* pVProgram = m_vsArray.GetElem(iPass)->GetGPUProgram();
	if(pVProgram)
	{
		U2ASSERT(0 <= idx && idx < pVProgram->GetUserConstantCnt());
		return pVProgram->GetUserConstant(idx);
	}
	return 0;
}


inline 
uint32 U2ShaderEffect::GetVTextureQuantity(int iPass) const 
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_vsArray.GetElem(iPass)->GetTextureCnt();
}



inline 
U2Dx9BaseTexturePtr U2ShaderEffect::GetVTexture(int iPass, int idx) const 
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_vsArray.GetElem(iPass)->GetTexture(idx);
}

inline 
U2Dx9BaseTexturePtr U2ShaderEffect::GetVTexture(int iPass, const U2DynString& name) const 
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_vsArray.GetElem(iPass)->GetTexture(name);
}


inline 
bool U2ShaderEffect::SetVTexture(int iPass, int i, U2Dx9BaseTexturePtr spTex)
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_vsArray.GetElem(iPass)->SetTexture(i, spTex);
}

inline 
bool U2ShaderEffect::SetVTexture(int iPass, int i, const U2DynString& name)
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_vsArray.GetElem(iPass)->SetTexture(i, name);
}



inline 
U2PixelShaderPtr U2ShaderEffect::GetPShader(int iPass)
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_psArray.GetElem(iPass);

}

inline 
U2PixelProgramPtr U2ShaderEffect::GetPProgram(int iPass)
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_psArray.GetElem(iPass)->GetGPUProgram();
}


inline 
const U2DynString& U2ShaderEffect::GetPSName(int iPass) const
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_psArray.GetElem(iPass)->GetShaderName();
}


inline 
uint32 U2ShaderEffect::GetPConstantQuantity(int iPass) const
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	U2PixelProgram* pVProgram = m_psArray.GetElem(iPass)->GetGPUProgram();
	return pVProgram ? pVProgram->GetUserConstantCnt() : 0;
}


inline
U2ShaderUserConstant* U2ShaderEffect::GetPConstant(int iPass, const U2DynString& name)
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	U2PixelProgram* pVProgram = m_psArray.GetElem(iPass)->GetGPUProgram();
	return pVProgram ? pVProgram->GetUserConstant(name.Str()) : 0;
}


inline
U2ShaderUserConstant* U2ShaderEffect::GetPConstant(int iPass, int idx)
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	U2PixelProgram* pVProgram = m_psArray.GetElem(iPass)->GetGPUProgram();
	if(pVProgram)
	{
		U2ASSERT(0 <= idx && idx < pVProgram->GetUserConstantCnt());
		return pVProgram->GetUserConstant(idx);
	}
	return 0;
}


inline 
uint32 U2ShaderEffect::GetPTextureQuantity(int iPass) const 
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	U2PixelProgram* pPProgram = m_psArray.GetElem(iPass)->GetGPUProgram();
	return pPProgram ? pPProgram->GetSamplerInfoCnt() : 0;
}



inline 
U2Dx9BaseTexturePtr U2ShaderEffect::GetPTexture(int iPass, int idx) const
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_psArray.GetElem(iPass)->GetTexture(idx);
}

inline 
U2Dx9BaseTexturePtr U2ShaderEffect::GetPTexture(int iPass, const U2DynString& name) const 
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_psArray.GetElem(iPass)->GetTexture(name);
}


inline 
bool U2ShaderEffect::SetPTexture(int iPass, int i, U2Dx9BaseTexturePtr pTex)
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_psArray.GetElem(iPass)->SetTexture(i, pTex);
}

inline 
bool U2ShaderEffect::SetPTexture(int iPass, int i, const U2DynString& name)
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	return m_psArray.GetElem(iPass)->SetTexture(i, name);
}
