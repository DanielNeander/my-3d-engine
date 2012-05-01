


//-------------------------------------------------------------------------------------------------
/**
Turn obey light links on/off. When turned off, every shape will
be lit by every light in the scene. If turned on, the Nebula2 application
is responsible for establishing bidirectional light links between the
render context objects.
*/
inline void U2SceneMgr::SetObjectLightLinks(bool bLitLinks)
{
	m_bObjLightLinks = bLitLinks;
}

//-------------------------------------------------------------------------------------------------
inline bool U2SceneMgr::GetObjectLightLinks() const
{
	return m_bObjLightLinks;
}

