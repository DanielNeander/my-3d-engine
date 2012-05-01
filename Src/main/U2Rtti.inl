

inline 
U2Rtti::~U2Rtti()
{

}

inline 
const TCHAR* U2Rtti::GetName() const 
{
	return m_acName;
}

inline 
int U2Rtti::GetDiskUsed() const 
{
	int len = (int)_tcslen(m_acName);
	return sizeof(len) + len * sizeof(TCHAR);
}

inline 
bool U2Rtti::IsExactly(const U2Rtti& type) const 
{
	return &type == this;
}

inline 
bool U2Rtti::IsDerived(const U2Rtti& type) const 
{
	const U2Rtti *pSearch = this;
	while(pSearch)
	{
		if(pSearch == &type)
		{
			return true;
		}
		pSearch = pSearch->m_pBaseType;
	}
	return false;
}