inline U2StreamInstance::Component::Component() 
	:m_eType(U2ShaderArg::AT_VOID),
	m_eParam(U2FxShaderState::InvalidParameter),
	m_usOffset(0)
{

}

inline U2StreamInstance::Component::Component(U2ShaderArg::ArgType eType, 
											  U2FxShaderState::Param eParam)
											  :m_eType(eType),
											  m_eParam(eParam),
											  m_usOffset(0)
{

}


inline void U2StreamInstance::Component::SetOffset(uint16 offset)
{
	this->m_usOffset = offset;
}


inline uint16 U2StreamInstance::Component::GetOffect() const 
{
	return this->m_usOffset;
}


inline U2ShaderArg::ArgType U2StreamInstance::Component::GetType() const
{
	return m_eType;
}


inline U2FxShaderState::Param U2StreamInstance::Component::GetSCM() const
{
	return m_eParam;
}

inline void U2StreamInstance::SetDeclaration(Declaration* pCompArray)
{
	m_pStreamDecl = pCompArray;
}


inline const U2StreamInstance::Declaration& U2StreamInstance::GetDeclaration() const 
{
	return *m_pStreamDecl;
}


inline int U2StreamInstance::GetStride() const 
{
	return m_iStreamStride;
}


inline int U2StreamInstance::GetCurrSize() const 
{
	return m_streamArray.Size() / this->m_iStreamStride;
}


inline void U2StreamInstance::WriteMatrix44(const D3DXMATRIX& val)
{
	U2ASSERT(LF_WRITE & this->m_iLockFlags);
	m_streamArray.Resize(m_streamArray.FilledSize() + 16);
	float* pfData = m_streamArray.GetBase() + m_streamArray.FilledSize();
	memcpy_s(pfData, 16 * sizeof(float), &val, 16 * sizeof(float));
}

inline void U2StreamInstance::WriteFloat4(const U2Float4& val)
{
	U2ASSERT(LF_WRITE & this->m_iLockFlags);
	m_streamArray.Resize(m_streamArray.FilledSize() + 4);
	float* pfData = m_streamArray.GetBase() + m_streamArray.FilledSize();
	memcpy_s(pfData, 4 * sizeof(float), &val, 4 * sizeof(float));
}



inline void U2StreamInstance::WriteVector3(const D3DXVECTOR3& val)
{
	U2ASSERT(LF_WRITE & this->m_iLockFlags);
	m_streamArray.Resize(m_streamArray.FilledSize() + 4);
	float* pfData = m_streamArray.GetBase() + m_streamArray.FilledSize();
	*pfData++ = val.x;
	*pfData++ = val.y;
	*pfData++ = val.z;
	*pfData++ = 0.f;
}


inline void U2StreamInstance::WriteFloat(float val)
{
	U2ASSERT(LF_WRITE & this->m_iLockFlags);

	this->m_streamArray.AddElem(val);
}


inline const D3DXMATRIX& U2StreamInstance::ReadMatrix44()
{
	const D3DXMATRIX& val = *(D3DXMATRIX*)this->m_fData;
	this->m_fData += 16;
	return val;
}


inline const U2Float4& U2StreamInstance::ReadFloat4()
{
	const U2Float4& val = *(U2Float4*)this->m_fData;
	this->m_fData += 4;
	return val;
}


inline float U2StreamInstance::ReadFloat()
{
	return *this->m_fData++;
}


inline bool U2StreamInstance::IsLocked() const
{
	return (0 != (this->m_iLockFlags & 0xffff));
}