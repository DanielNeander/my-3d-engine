///////////////////////////////////////////////////////////////////////
//  Shaders.inl
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) CONFIDENTIAL AND PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization, Inc. and
//  may not be copied, disclosed, or exploited except in accordance with 
//  the terms of that agreement.
//
//      Copyright (c) 2003-2009 IDV, Inc.
//      All rights reserved in all media.
//
//      IDV, Inc.
//      http://www.idvinc.com


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueDirectX9::CShaderTechniqueDirectX9

inline CShaderTechniqueDirectX9::CShaderTechniqueDirectX9( ) :
	m_pEffect(NULL),
	m_hTechnique(NULL)
{
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueDirectX9::~CShaderTechniqueDirectX9

inline CShaderTechniqueDirectX9::~CShaderTechniqueDirectX9( )
{
#ifdef _DEBUG
	m_pEffect = NULL;
	m_hTechnique = NULL;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueDirectX9::Bind

inline st_bool CShaderTechniqueDirectX9::Bind(st_uint32 uiPass)
{
	st_bool bSuccess = false;

	if (m_pEffect->SetTechnique(m_hTechnique) == D3D_OK)
	{
		st_uint32 uiNumPasses = 0;
		if (m_pEffect->Begin(&uiNumPasses, 0) == D3D_OK)
		{
			if (uiPass < uiNumPasses)
			{
				if (m_pEffect->BeginPass(uiPass) == D3D_OK)
					bSuccess = true;
			}
			else
				CCore::SetError("CIdvTechnique::Begin, only %d passes defined, cannot bind pass %d", uiNumPasses, uiPass);
		}
	}

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueDirectX9::UnBind

inline st_bool CShaderTechniqueDirectX9::UnBind(void)
{
	st_bool bSuccess = false;

	if (m_pEffect->EndPass( ) == D3D_OK && m_pEffect->End( ) == D3D_OK)
		bSuccess = true;

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueDirectX9::CommitConstants

inline st_bool CShaderTechniqueDirectX9::CommitConstants(void)
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (m_pEffect)
	{
#endif
		bSuccess = (m_pEffect->CommitChanges( ) == D3D_OK);
#ifdef _DEBUG
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueDirectX9::CommitTextures

inline st_bool CShaderTechniqueDirectX9::CommitTextures(void)
{
	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueDirectX9::IsValid

inline st_bool CShaderTechniqueDirectX9::IsValid(void) const
{
	return (m_hTechnique != NULL && m_pEffect != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::CShaderConstantDirectX9

inline CShaderConstantDirectX9::CShaderConstantDirectX9( ) :
	m_pEffect(NULL),
	m_hParameter(NULL)
{
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::~CShaderConstantDirectX9

inline CShaderConstantDirectX9::~CShaderConstantDirectX9( )
{
#ifdef _DEBUG
	m_pEffect = NULL;
	m_hParameter = NULL;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::IsValid

inline st_bool CShaderConstantDirectX9::IsValid(void) const
{
	return (m_hParameter != NULL && m_pEffect != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::Set1f

inline st_bool CShaderConstantDirectX9::Set1f(st_float32 x) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		bSuccess = (m_pEffect->SetFloat(m_hParameter, x) == D3D_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}



///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::Set2f

inline st_bool CShaderConstantDirectX9::Set2f(st_float32 x, st_float32 y) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		D3DXVECTOR4 vVector(x, y, 0.0f, 0.0f); 
		bSuccess = (m_pEffect->SetVector(m_hParameter, &vVector) == D3D_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::Set2fv

inline st_bool CShaderConstantDirectX9::Set2fv(const st_float32 afValues[2]) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		D3DXVECTOR4 vVector(afValues[0], afValues[1], 0.0f, 0.0f); 
		bSuccess = (m_pEffect->SetVector(m_hParameter, &vVector) == D3D_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::Set2f

inline st_bool CShaderConstantDirectX9::Set3f(st_float32 x, st_float32 y, st_float32 z) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		D3DXVECTOR4 vVector(x, y, z, 0.0f); 
		bSuccess = (m_pEffect->SetVector(m_hParameter, &vVector) == D3D_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::Set3fv

inline st_bool CShaderConstantDirectX9::Set3fv(const st_float32 afValues[3]) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		D3DXVECTOR4 vVector(afValues[0], afValues[1], afValues[2], 0.0f); 
		bSuccess = (m_pEffect->SetVector(m_hParameter, &vVector) == D3D_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::Set3fvPlus1f

inline st_bool CShaderConstantDirectX9::Set3fvPlus1f(const st_float32 afValues[3], st_float32 w) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		D3DXVECTOR4 vVector(afValues[0], afValues[1], afValues[2], w); 
		bSuccess = (m_pEffect->SetVector(m_hParameter, &vVector) == D3D_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::Set4f

inline st_bool CShaderConstantDirectX9::Set4f(st_float32 x, st_float32 y, st_float32 z, st_float32 w) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		D3DXVECTOR4 vVector(x, y, z, w);
		bSuccess = (m_pEffect->SetVector(m_hParameter, &vVector) == D3D_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::Set4fv

inline st_bool CShaderConstantDirectX9::Set4fv(const st_float32 afValues[4]) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		const D3DXVECTOR4* pVector = reinterpret_cast<const D3DXVECTOR4*>(afValues);
		bSuccess = (m_pEffect->SetVector(m_hParameter, pVector) == D3D_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::SetArray4f

inline st_bool CShaderConstantDirectX9::SetArray4f(const st_float32* pArray, st_int32 nSize, st_int32 nOffset) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ) && pArray)
	{
#endif
		D3DXHANDLE hElement = (nOffset > 0) ? m_pEffect->GetParameterElement(m_hParameter, nOffset) : m_hParameter;
		if (hElement)
		{
			bSuccess = (m_pEffect->SetVectorArray(hElement, (const D3DXVECTOR4*) pArray, nSize) == D3D_OK);
		}
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::SetMatrix

inline st_bool CShaderConstantDirectX9::SetMatrix(const st_float32 afMatrix[16]) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		D3DXMATRIX cMatrix(afMatrix);
		bSuccess = (m_pEffect->SetMatrixTranspose(m_hParameter, &cMatrix) == D3D_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::SetMatrixTranspose

inline st_bool CShaderConstantDirectX9::SetMatrixTranspose(const st_float32 afMatrix[16]) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		D3DXMATRIX cMatrix(afMatrix);
		bSuccess = (m_pEffect->SetMatrix(m_hParameter, &cMatrix) == D3D_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::SetMatrixIndex

inline st_bool CShaderConstantDirectX9::SetMatrixIndex(const st_float32 afMatrix[16], st_uint32 uiIndex) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		D3DXMATRIX cMatrix(afMatrix);

		D3DXHANDLE hElement = m_pEffect->GetParameterElement(m_hParameter, uiIndex);
		if (hElement)
			bSuccess = (m_pEffect->SetMatrixTranspose(hElement, &cMatrix) == D3D_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX9::SetMatrix4x4Array

inline st_bool CShaderConstantDirectX9::SetMatrix4x4Array(const st_float32* pMatrixArray, st_uint32 uiNumMatrices) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ) && pMatrixArray)
	{
#endif
		bSuccess = (m_pEffect->SetMatrixArray(m_hParameter, (const D3DXMATRIX*) pMatrixArray, uiNumMatrices) == D3D_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstant::SetTexture

inline st_bool CShaderConstant::SetTexture(const CTexture& texTexture) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		if (texTexture.m_tTexturePolicy.GetCubeTextureObject( ))
			bSuccess = (m_tShaderConstantPolicy.m_pEffect->SetTexture(m_tShaderConstantPolicy.m_hParameter, texTexture.m_tTexturePolicy.GetCubeTextureObject( )) == D3D_OK);
		else
			bSuccess = (m_tShaderConstantPolicy.m_pEffect->SetTexture(m_tShaderConstantPolicy.m_hParameter, texTexture.m_tTexturePolicy.GetTextureObject( )) == D3D_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstant::SetTexture

inline st_bool CShaderConstant::SetTexture(const CDepthTexture& cDepthTexture) const
{
st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		bSuccess = (m_tShaderConstantPolicy.m_pEffect->SetTexture(m_tShaderConstantPolicy.m_hParameter, cDepthTexture.m_tDepthTexturePolicy.GetTextureObject( )) == D3D_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstant::SetTextureIndex

inline st_bool CShaderConstant::SetTextureIndex(const CTexture& /*texTexture*/, st_uint32 /*uiIndex*/) const
{
	st_bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		// todo: not yet implemented
		//bSuccess = (m_tShaderConstantPolicy.m_pEffect->SetTexture(m_tShaderConstantPolicy.m_hParameter, texTexture.m_tTexturePolicy.GetTextureObject( )) == D3D_OK);
#ifdef _DEBUG	
	}
	else
		return false;
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstant::SetTextureIndex

inline st_bool CShaderConstant::SetTextureIndex(const CDepthTexture& /*cDepthTexture*/, st_uint32 /*uiIndex*/) const
{
	// todo: not yet implemented
	return false;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderDirectX9::CShaderLoaderDirectX9

inline CShaderLoaderDirectX9::CShaderLoaderDirectX9( ) :
	m_pEffect(NULL)
{
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderDirectX9::~CShaderLoaderDirectX9

inline CShaderLoaderDirectX9::~CShaderLoaderDirectX9( )
{
	Release( );

#ifdef _DEBUG
	m_pEffect = NULL;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderDirectX9::GetError

inline const char* CShaderLoaderDirectX9::GetError(void) const
{
	return m_strError.c_str( );
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderDirectX9::OnResetDevice

inline void CShaderLoaderDirectX9::OnResetDevice(void)
{
	if (m_pEffect != NULL)
		m_pEffect->OnResetDevice( );
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderDirectX9::OnLostDevice

inline void CShaderLoaderDirectX9::OnLostDevice(void)
{
	if (m_pEffect != NULL)
		m_pEffect->OnLostDevice( );
}

