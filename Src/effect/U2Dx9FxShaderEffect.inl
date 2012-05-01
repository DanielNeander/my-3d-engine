//-------------------------------------------------------------------------------------------------
inline U2D3DXEffectShader* U2Dx9FxShaderEffect::GetEffectShader() const
{
	return m_pShader;
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9FxShaderEffect::SetInt(U2FxShaderState::Param param, int val)
{
	if(U2FxShaderState::InvalidParameter == param)
	{
		FDebug("WARNING: invalid shader parameter in object '%s'\n", GetName());
		return;
	}
	m_shaderAttb.SetArg(param, U2ShaderArg(val));
}

//-------------------------------------------------------------------------------------------------
inline int U2Dx9FxShaderEffect::GetInt(U2FxShaderState::Param param) const
{
	return m_shaderAttb.GetArg(param).GetInt();
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9FxShaderEffect::SetBool(U2FxShaderState::Param param, bool val)
{
	if(U2FxShaderState::InvalidParameter == param)
	{
		FDebug("WARNING: invalid shader parameter in object '%s'\n", GetName());
		return;
	}
	m_shaderAttb.SetArg(param, U2ShaderArg(val));
}

//-------------------------------------------------------------------------------------------------
inline bool U2Dx9FxShaderEffect::GetBool(U2FxShaderState::Param param) const
{
	return m_shaderAttb.GetArg(param).GetBool();
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9FxShaderEffect::SetFloat(U2FxShaderState::Param param, float val)
{
	if(U2FxShaderState::InvalidParameter == param)
	{
		FDebug("WARNING: invalid shader parameter in object '%s'\n", GetName());
		return;
	}
	m_shaderAttb.SetArg(param, U2ShaderArg(val));
}

//-------------------------------------------------------------------------------------------------
inline float U2Dx9FxShaderEffect::GetFloat(U2FxShaderState::Param param) const
{
	return m_shaderAttb.GetArg(param).GetFloat();
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9FxShaderEffect::SetVector(U2FxShaderState::Param param, const D3DXVECTOR4& val)
{
	if(U2FxShaderState::InvalidParameter == param)
	{
		FDebug("WARNING: invalid shader parameter in object '%s'\n", GetName());
		return;		
	}

	m_shaderAttb.SetArg(param, U2ShaderArg(val));
}

inline void U2Dx9FxShaderEffect::SetVector(U2FxShaderState::Param param, float r, float g, float b, float a)
{
	SetVector(param, D3DXVECTOR4(r, g, b, a));
}


//-------------------------------------------------------------------------------------------------
inline const D3DXVECTOR4& U2Dx9FxShaderEffect::GetVector(U2FxShaderState::Param param) const
{
	return m_shaderAttb.GetArg(param).GetVector4();
}

//-------------------------------------------------------------------------------------------------
inline uint32 U2Dx9FxShaderEffect::GetShaderIdx()
{
	return m_uiShaderIdx;
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9FxShaderEffect::SetFrameShaderName(const TCHAR* szFrameShadername)
{
	m_szFrameShadername = szFrameShadername;
}

//-------------------------------------------------------------------------------------------------
inline const TCHAR* U2Dx9FxShaderEffect::GetFrameShaderName() const
{
	return m_szFrameShadername;
}