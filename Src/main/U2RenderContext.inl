//-------------------------------------------------------------------------------------------------
inline U2RenderContext::U2RenderContext() 
:m_uiFrameId(0xffffffff),
m_uiFlags(DO_ALL), 
m_fPriority(1.0f),
m_fShadowIntensity(1.0f),
pUserData(0)
{
	
}

//-------------------------------------------------------------------------------------------------
inline U2RenderContext::~U2RenderContext()
{

}

//-------------------------------------------------------------------------------------------------
inline void U2RenderContext::SetFrameID(uint32 id)
{
	m_uiFrameId = id;
}

//-------------------------------------------------------------------------------------------------
inline uint32 U2RenderContext::GetFrameID() const
{
	return m_uiFrameId;
}

//-------------------------------------------------------------------------------------------------
inline void U2RenderContext::SetRootNode(U2Node* pNode)
{
	m_spRootNode = pNode;
}

inline U2NodePtr U2RenderContext::GetRootNode() const
{
	return m_spRootNode;
}

//-------------------------------------------------------------------------------------------------
inline bool U2RenderContext::IsValid() const
{
	return m_spRootNode != NULL;
};

//-------------------------------------------------------------------------------------------------
/// access to shader parameter overrides
inline U2ShaderAttribute& U2RenderContext::GetShaderOverides()
{
	return m_shderOverrides;
}

//-------------------------------------------------------------------------------------------------
inline uint32 U2RenderContext::AddLocalVar(U2Variable* val)
{
	return m_aspLocalVars.AddElem(val);
}	

//-------------------------------------------------------------------------------------------------
inline U2Variable& U2RenderContext::GetLocalVar(uint32 idx)
{
	return *SmartPtrCast(U2Variable, m_aspLocalVars.GetElem(idx));
}

//-------------------------------------------------------------------------------------------------
inline void U2RenderContext::ClearLocalVars()
{
	
}

//-------------------------------------------------------------------------------------------------
inline U2Variable* U2RenderContext::FindLocalVar(U2Variable::Handle handle)
{
	uint32 i;
	uint32 numVar = m_aspLocalVars.FilledSize();
	for(i=0; i < numVar; ++i)
	{
		if(m_aspLocalVars[i]->GetHandle() == handle)
		{
			return m_aspLocalVars[i];
		}
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
inline void U2RenderContext::SetShadowIntensity(float shadowIntensity)
{
	m_fShadowIntensity = shadowIntensity;	
}

//-------------------------------------------------------------------------------------------------
inline float U2RenderContext::GetShadowIntensity() const
{
	return m_fShadowIntensity;
}

//-------------------------------------------------------------------------------------------------
/// set flags
inline void U2RenderContext::SetFlag(Flag f, bool b)
{
	if(b) m_uiFlags |= f;
	else m_uiFlags &= ~f;
	
}

//-------------------------------------------------------------------------------------------------
/// get visibility hint
inline bool U2RenderContext::GetFlag(Flag f) const
{
	return 0 != (m_uiFlags & f);
}