//-------------------------------------------------------------------------------------------------
inline void U2FrameRenderTarget::SetRelativeSize(float size)
{
	m_fRelSize = size;
}

//-------------------------------------------------------------------------------------------------
inline float U2FrameRenderTarget::GetRelativeSize() const
{
	return m_fRelSize;
}

//-------------------------------------------------------------------------------------------------
inline void U2FrameRenderTarget::SetWidth(int w)
{
	m_iWidth = w;
}

//-------------------------------------------------------------------------------------------------
inline int U2FrameRenderTarget::GetWidth() const
{
	return m_iWidth;
}

//-------------------------------------------------------------------------------------------------
inline void U2FrameRenderTarget::SetHeight(int height) 
{
	m_iHeight = height;
}

//-------------------------------------------------------------------------------------------------
inline int U2FrameRenderTarget::GetHeight() const
{
	return m_iHeight;
}

//-------------------------------------------------------------------------------------------------
inline void U2FrameRenderTarget::SetFormat(U2Dx9BaseTexture::Format f)
{
	m_ePixelFmt = f;
}

//-------------------------------------------------------------------------------------------------
inline U2Dx9BaseTexture::Format U2FrameRenderTarget::GetFormat() const
{
	return m_ePixelFmt;
}

//-------------------------------------------------------------------------------------------------
inline const U2Dx9Texture* U2FrameRenderTarget::GetTexture() const
{
	return m_pRenderedTex;
}



