//-------------------------------------------------------------------------------------------------
inline U2Dx9ShadowMgr* U2Dx9ShadowMgr::Instance()
{
	return ms_pSingleton;
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9ShadowMgr::SetDepthFail(bool bFail)
{
	m_bUseDepthFail = bFail;
}

//-------------------------------------------------------------------------------------------------
inline bool U2Dx9ShadowMgr::GetDepthFail() const
{
	return m_bUseDepthFail;
}

//-------------------------------------------------------------------------------------------------
inline void U2Dx9ShadowMgr::SetEnableShadows(bool bEnable)
{
	m_bShadowEnabled = bEnable;
}

//-------------------------------------------------------------------------------------------------
inline bool U2Dx9ShadowMgr::GetEnableShadows() const
{
	return m_bShadowEnabled;
}