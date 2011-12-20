inline void U2FrameShader::SetFilename(const U2DynString& szFilename)
{
	m_szFilename = szFilename;
}

//-------------------------------------------------------------------------------------------------
inline const U2DynString& U2FrameShader::GetFilename() const
{
	return m_szFilename;
}

//-------------------------------------------------------------------------------------------------
inline void U2FrameShader::SetBucketIdx(unsigned int idx)
{
	U2ASSERT(idx >= 0);
	m_uiBucketIdx = idx;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2FrameShader::GetBucketIdx() const
{
	return m_uiBucketIdx;
}

//-------------------------------------------------------------------------------------------------
inline U2D3DXEffectShader* U2FrameShader::GetShader() const
{
	return m_pEffectShader;
}


