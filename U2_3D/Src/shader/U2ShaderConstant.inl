//----------------------------------------------------------------------------
inline U2ShaderConstant::ShaderConstantMapping U2ShaderConstant::GetSCM () const
{
	return m_eSCM;
}
//----------------------------------------------------------------------------
inline int U2ShaderConstant::GetBaseRegister () const
{
	return m_iBaseRegister;
}
//----------------------------------------------------------------------------
inline uint32 U2ShaderConstant::GetRegisterCnt () const
{
	return m_uRegisterCnt;
}
//----------------------------------------------------------------------------
inline float* U2ShaderConstant::GetData () const
{
	return (float*)&m_afData[0];
}
//----------------------------------------------------------------------------