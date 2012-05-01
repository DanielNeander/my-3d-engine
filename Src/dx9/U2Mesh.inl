
//-------------------------------------------------------------------------------------------------
inline void U2Mesh::SetSkinModifier(U2SkinModifier* pSkinMod)
{
	m_spSkinMod = pSkinMod;
}

//-------------------------------------------------------------------------------------------------
inline U2SkinModifier* U2Mesh::GetSkinModifier() const
{
	return SmartPtrCast(U2SkinModifier, m_spSkinMod);
}

//-------------------------------------------------------------------------------------------------
inline void U2Mesh::UpdateModelNormals()
{

}

//-------------------------------------------------------------------------------------------------
inline void U2Mesh::UpdateModelTangentSpace(int iTangentSpaceType)
{

}

//-------------------------------------------------------------------------------------------------
inline void  U2Mesh::LightOnOff(bool bEnable)
{
	m_bLightEnable = bEnable;	
}

//-------------------------------------------------------------------------------------------------
inline bool U2Mesh::GetLightOnOff() const
{
	return m_bLightEnable;
}

