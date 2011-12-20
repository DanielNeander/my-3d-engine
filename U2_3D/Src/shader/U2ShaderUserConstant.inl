inline 
U2ShaderUserConstant::U2ShaderUserConstant(const U2String& name, 
										   int baseRegister, int registerCnt)
										   :m_name(name)
{
	U2ASSERT(baseRegister >= 0);
	U2ASSERT(registerCnt > 0);

	m_iBaseRegister = baseRegister;
	m_iRegisterCnt = registerCnt;

	m_pfData = 0;
}


inline 
U2ShaderUserConstant::~U2ShaderUserConstant()
{

}


inline 
void U2ShaderUserConstant::SetDataSource(float* pfData)
{
	U2ASSERT(pfData);

	m_pfData = pfData;
}

inline
const U2String& U2ShaderUserConstant::GetName() const
{
	return m_name;
}

inline
int U2ShaderUserConstant::GetBaseRegister() const
{
	return m_iBaseRegister;
}

inline
int U2ShaderUserConstant::GetRegisterCnt () const
{
	return m_iRegisterCnt;
}

inline
float* U2ShaderUserConstant::GetData () const
{
	return m_pfData;
}