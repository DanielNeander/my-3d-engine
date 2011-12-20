//-------------------------------------------------------------------------------------------------
inline U2ShaderAttribute::AttbEntry::AttbEntry() 
: m_eParam(U2FxShaderState::InvalidParameter)
{

}

//-------------------------------------------------------------------------------------------------
inline 	U2ShaderAttribute::AttbEntry::AttbEntry(U2FxShaderState::Param eParam, 
					const U2ShaderArg& arg)
					:m_eParam(eParam),
					m_arg(arg)
{

}


inline U2ShaderAttribute::U2ShaderAttribute() 
:m_attbEntryArray(0, 8)
{
	if(!ms_pInvalidArg)
		ms_pInvalidArg = U2_NEW U2ShaderArg;


	this->Clear();
}


inline U2ShaderAttribute::~U2ShaderAttribute()
{
	
}


inline void U2ShaderAttribute::Clear()
{
	int i;
	for(i=0; i < U2FxShaderState::NumParameters; ++i)
	{
		m_tcSCMIdx[i] = -1;
	}

	m_attbEntryArray.RemoveAll();
}


inline int U2ShaderAttribute::GetNumValidAttbEntries() const
{
	return this->m_attbEntryArray.FilledSize();
}


inline bool U2ShaderAttribute::IsValid(U2FxShaderState::Param eParam) const
{
	U2ASSERT(0 <= eParam && eParam < U2FxShaderState::NumParameters);
	return (-1 != m_tcSCMIdx[eParam]);	
}


inline void U2ShaderAttribute::SetArg(U2FxShaderState::Param eParam, 
									  const U2ShaderArg& arg)
{
	U2ASSERT(0 <= eParam && eParam < U2FxShaderState::NumParameters);
	TCHAR idx = m_tcSCMIdx[eParam];
	
	if(idx == -1)
	{
		AttbEntry* pEntry = U2_NEW AttbEntry(eParam, arg);	
		m_attbEntryArray.AddElem(pEntry);
		m_tcSCMIdx[eParam] = m_attbEntryArray.FilledSize() -1;
	}
	else 
	{
		AttbEntry* pEntry = m_attbEntryArray.GetElem(m_tcSCMIdx[eParam]);
		pEntry->m_arg = arg;
		pEntry->m_eParam = eParam;
		//m_attbEntryArray.SetElem(idx, pEntry); 
	}
}


inline void U2ShaderAttribute::ClearAttbEntry(U2FxShaderState::Param eParam)
{
	U2ASSERT(0 <= eParam && eParam < U2FxShaderState::NumParameters);
	TCHAR idx = m_tcSCMIdx[eParam];

	if(idx != -1)
	{
		m_attbEntryArray.Remove(idx);
		m_tcSCMIdx[eParam] = -1;

		int i;
		for(i =0; i < U2FxShaderState::NumParameters; ++i)
		{
			if(m_tcSCMIdx[i] >= idx)
			{
				--m_tcSCMIdx[i];
			}
		}
	}
}


inline U2FxShaderState::Param& U2ShaderAttribute::GetParam(int idx) const 
{
	U2ASSERT(0 <= idx && idx < m_attbEntryArray.FilledSize());
	return m_attbEntryArray.GetElem(idx)->m_eParam;
}

inline const U2ShaderArg& U2ShaderAttribute::GetArg(int idx) const
{
	U2ASSERT(idx >= 0 && idx < m_attbEntryArray.Size());
	return m_attbEntryArray.GetElem(idx)->m_arg;
}



inline const U2ShaderArg& U2ShaderAttribute::GetArg(
	U2FxShaderState::Param eParam) const
{
	
	U2ASSERT(0 <= eParam && eParam < U2FxShaderState::NumParameters);
	TCHAR idx = this->m_tcSCMIdx[eParam];
	if(idx != -1)
	{
		return m_attbEntryArray.GetElem(idx)->m_arg;
	}
	else
	{
		U2ASSERT2(idx != -1, _T("Shader Constant Mapping was't set!"));
		return *ms_pInvalidArg;
	}		
}

