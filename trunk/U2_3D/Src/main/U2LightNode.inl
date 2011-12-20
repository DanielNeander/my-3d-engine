inline U2Light* U2LightNode::GetLight ()
{
	return m_spLight;
}
//----------------------------------------------------------------------------
inline const U2Light* U2LightNode::GetLight () const
{
	return m_spLight;
}

//-------------------------------------------------------------------------------------------------
inline U2ShaderAttribute& U2LightNode::GetShaderAttb()
{
	return m_shaderAttb;
}

//-------------------------------------------------------------------------------------------------
inline void U2LightNode::SetCastShadows(bool bCastShadows)
{
	m_spLight->m_bCastShadows = bCastShadows;
}

//-------------------------------------------------------------------------------------------------
inline bool U2LightNode::GetCastShadows() const
{
	return m_spLight->m_bCastShadows;
}



