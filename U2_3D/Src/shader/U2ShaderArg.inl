inline U2ShaderArg::U2ShaderArg()
{
	Clear();
}

inline U2ShaderArg::~U2ShaderArg()
{

}


inline void U2ShaderArg::Clear()
{
	m_eType  = U2ShaderArg::AT_VOID;
	memset(&m[0], 0, sizeof(m[4][4]));
}


inline U2ShaderArg::U2ShaderArg(U2ShaderArg::ArgType type)
	:m_eType(type),
	m_iVal(0)
{
	
}


inline U2ShaderArg::U2ShaderArg(bool bVal)
{
	SetBool(bVal);
}


inline U2ShaderArg::U2ShaderArg(int iVal)
{
	SetInt(iVal);
}


inline U2ShaderArg::U2ShaderArg(float fVal)
{
	SetFloat(fVal);
}

inline U2ShaderArg::U2ShaderArg(const U2Float4& f4Val)
{
	SetFloat4(f4Val);
}


inline U2ShaderArg::U2ShaderArg(const D3DXVECTOR4& vVal)
{
	SetVector4(vVal);
}


inline U2ShaderArg::U2ShaderArg(const D3DXMATRIX& matVal)
{
	SetMatrix44(&matVal);
}


inline U2ShaderArg::U2ShaderArg(const U2Dx9BaseTexture* pTexVal)
{
	SetTexture((U2Dx9BaseTexture*)pTexVal);
}




inline bool U2ShaderArg::operator == (const U2ShaderArg& rhs) const
{
	if(m_eType == rhs.m_eType)
	{
		switch(m_eType)
		{
		case U2ShaderArg::AT_VOID:			
			{			
				return true;
			}
		case U2ShaderArg::AT_BOOL:
			{			
				return this->m_bVal == rhs.m_bVal;
			}
		case U2ShaderArg::AT_INT:
			{
				return this->m_iVal == rhs.m_iVal;
			}
		case U2ShaderArg::AT_FLOAT:
			{
				return this->m_fVal == rhs.m_fVal;
			}
		case U2ShaderArg::AT_MATRIX44:
			{
				bool bEqual = true;
				int i;
				for(i=0; i < 4; ++i)
				{
					int j;
					for(j=0; j < 4;++j)
					{
						if(this->m[i][j] != rhs.m[i][j])
						{
							bEqual = false;
						}
					}

				}
				return bEqual;
			}
		case U2ShaderArg::AT_TEXTURE:
			{			
				return this->m_spTex == rhs.m_spTex;
			}

		default:
			U2ASSERT(_T("U2ShaderArg::operatr==() : Invalid U2ShaderARg type!"));
			break;			
		}
	}
	return false;
}


inline void U2ShaderArg::operator=(const U2ShaderArg& rhs)
{
	this->m_eType = rhs.m_eType;
	switch(rhs.m_eType)
	{
	case U2ShaderArg::AT_VOID:
		break;
	case U2ShaderArg::AT_BOOL:
		this->m_bVal = rhs.m_bVal;
		break;
	case U2ShaderArg::AT_INT:
		this->m_iVal = rhs.m_iVal;
		break;
	case U2ShaderArg::AT_FLOAT:
		this->m_fVal = rhs.m_fVal;
		break;
	case U2ShaderArg::AT_FLOAT4:
		this->m_f4Val = rhs.m_f4Val;
		break;

	case U2ShaderArg::AT_MATRIX44:
		memcpy_s(&this->m, sizeof(this->m), &rhs.m, sizeof(this->m));
		break;
	case U2ShaderArg::AT_TEXTURE:
		this->m_spTex = rhs.m_spTex;
		break;

	default:
		U2ASSERT(_T("U2ShaderArg::operator=(): Invalid argument type!"));
		break;
	}
}


inline void U2ShaderArg::SetType(ArgType eType)
{
	this->m_eType = eType;
}


inline U2ShaderArg::ArgType U2ShaderArg::GetType() const
{
	return m_eType;
}


inline void U2ShaderArg::SetBool(bool bVal)
{
	m_eType = U2ShaderArg::AT_BOOL;
	m_bVal = bVal;
}


inline bool U2ShaderArg::GetBool()const
{
	return m_bVal;
}


inline void U2ShaderArg::SetInt(int val)
{
	m_eType = U2ShaderArg::AT_INT;
	m_iVal = val;
}


inline int U2ShaderArg::GetInt() const
{
	return m_iVal;
}


inline void U2ShaderArg::SetFloat(float val)
{
	m_eType = U2ShaderArg::AT_FLOAT;
	m_fVal = val;
}


inline float U2ShaderArg::GetFloat() const
{
	return m_fVal;
}


inline void U2ShaderArg::SetFloat4(const U2Float4& val)
{
	this->m_eType = U2ShaderArg::AT_FLOAT4;
	this->m_f4Val = val;
}


inline const U2Float4& U2ShaderArg::GetFloat4() const
{
	return m_f4Val;
}


inline void U2ShaderArg::SetVector4(const D3DXVECTOR4& val)
{
	m_eType = U2ShaderArg::AT_FLOAT4;
	m_f4Val = *(U2Float4*)&val;
}


inline D3DXVECTOR4& U2ShaderArg::GetVector4() const
{
	return *(D3DXVECTOR4*)&m_f4Val;
}


inline void U2ShaderArg::SetMatrix44(const D3DXMATRIX* val)
{
	U2ASSERT(val);
	m_eType = U2ShaderArg::AT_MATRIX44;
	int i;
	//for(i=0; i < 4;++i)
	//{
	//	int j;
	//	for(j=0; j < 4; j++)
	//	{
	//		this->m[i][j] = val->m[i][j];
	//	}
	//}
	memcpy_s(m, sizeof(D3DXMATRIX), val, sizeof(D3DXMATRIX));
}


inline const D3DXMATRIX* U2ShaderArg::GetMatrix44() const
{
	return (D3DXMATRIX*)m;
}


inline void U2ShaderArg::SetTexture(U2Dx9BaseTexture* val)
{
	m_eType = U2ShaderArg::AT_TEXTURE;
	this->m_spTex = val;
}


inline U2Dx9BaseTexture* U2ShaderArg::GetTexture() const
{
	return m_spTex;
}

