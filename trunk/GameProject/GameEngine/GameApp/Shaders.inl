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
//  CShaderTechniqueDirectX10::CShaderTechniqueDirectX10

inline CShaderTechniqueDirectX10::CShaderTechniqueDirectX10( ) :
	m_pTechnique(NULL)
{
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueDirectX10::~CShaderTechniqueDirectX10

inline CShaderTechniqueDirectX10::~CShaderTechniqueDirectX10( )
{
#ifdef _DEBUG
	m_pTechnique = NULL;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueDirectX10::Bind

inline bool CShaderTechniqueDirectX10::Bind(unsigned int uiPass)
{
	CRenderState::ApplyStates( );
	return (m_pTechnique->GetPassByIndex(uiPass)->Apply(0, DX11::Context()) == S_OK);
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueDirectX10::UnBind

inline bool CShaderTechniqueDirectX10::UnBind(void)
{
	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueDirectX10::CommitConstants

inline bool CShaderTechniqueDirectX10::CommitConstants(void)
{
	CRenderState::ApplyStates( );
	return (m_pTechnique->GetPassByIndex(0)->Apply(0, DX11::Context()) == S_OK);
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueDirectX10::CommitTextures

inline bool CShaderTechniqueDirectX10::CommitTextures(void)
{
	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueDirectX10::IsValid

inline bool CShaderTechniqueDirectX10::IsValid(void) const
{
	return (m_pTechnique != NULL && m_pTechnique->IsValid( ));
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueDirectX10::GetDX10Technique

inline ID3DX11EffectTechnique* CShaderTechniqueDirectX10::GetDX10Technique(void) const
{
	return m_pTechnique;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::CShaderConstantDirectX10

inline CShaderConstantDirectX10::CShaderConstantDirectX10( ) :
	m_pParameter(NULL)
{
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::~CShaderConstantDirectX10

inline CShaderConstantDirectX10::~CShaderConstantDirectX10( )
{
#ifdef _DEBUG
	m_pParameter = NULL;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::IsValid

inline bool CShaderConstantDirectX10::IsValid(void) const
{
	return (m_pParameter != NULL && m_pParameter->IsValid( ));
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::Set1f

inline bool CShaderConstantDirectX10::Set1f(float x) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		bSuccess = (m_pParameter->AsScalar( )->SetFloat(x) == S_OK);
#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::Set2f

inline bool CShaderConstantDirectX10::Set2f(float x, float y) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		float afVector[4] = { x, y, 0.0f, 0.0f }; 
		bSuccess = (m_pParameter->AsVector( )->SetFloatVector(afVector) == S_OK);

#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::Set2fv

inline bool CShaderConstantDirectX10::Set2fv(const float afValues[2]) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		float afVector[4] = { afValues[0], afValues[1], 0.0f, 0.0f }; 
		bSuccess = (m_pParameter->AsVector( )->SetFloatVector(afVector) == S_OK);

#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::Set2f

inline bool CShaderConstantDirectX10::Set3f(float x, float y, float z) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		float afVector[4] = { x, y, z, 0.0f }; 
		bSuccess = (m_pParameter->AsVector( )->SetFloatVector(afVector) == S_OK);

#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::Set3fv

inline bool CShaderConstantDirectX10::Set3fv(const float afValues[3]) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		float afVector[4] = { afValues[0], afValues[1], afValues[2], 0.0f }; 
		bSuccess = (m_pParameter->AsVector( )->SetFloatVector(afVector) == S_OK);

#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::Set3fvPlus1f

inline bool CShaderConstantDirectX10::Set3fvPlus1f(const float afValues[3], float w) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		float afVector[4] = { afValues[0], afValues[1], afValues[2], w }; 
		bSuccess = (m_pParameter->AsVector( )->SetFloatVector(afVector) == S_OK);

#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::Set4f

inline bool CShaderConstantDirectX10::Set4f(float x, float y, float z, float w) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		float afVector[4] = { x, y, z, w }; 
		bSuccess = (m_pParameter->AsVector( )->SetFloatVector(afVector) == S_OK);

#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::Set4fv

inline bool CShaderConstantDirectX10::Set4fv(const float afValues[4]) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		bSuccess = (m_pParameter->AsVector( )->SetFloatVector(const_cast<float*>(afValues)) == S_OK);

#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::SetArray4f

inline bool CShaderConstantDirectX10::SetArray4f(const float* pArray, int nSize, int nOffset) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ) && pArray)
	{
#endif
		bSuccess = (m_pParameter->AsVector( )->SetFloatVectorArray(const_cast<float*>(pArray), nOffset, nSize) == S_OK);

#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::SetMatrix

inline bool CShaderConstantDirectX10::SetMatrix(const float afMatrix[16]) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		bSuccess = (m_pParameter->AsMatrix( )->SetMatrixTranspose(const_cast<float*>(afMatrix)) == S_OK);

#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::SetMatrixTranspose

inline bool CShaderConstantDirectX10::SetMatrixTranspose(const float afMatrix[16]) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		bSuccess = (m_pParameter->AsMatrix( )->SetMatrix(const_cast<float*>(afMatrix)) == S_OK);

#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::SetMatrixIndex

inline bool CShaderConstantDirectX10::SetMatrixIndex(const float afMatrix[16], unsigned int uiIndex) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		bSuccess = (m_pParameter->GetElement(uiIndex)->AsMatrix( )->SetMatrixTranspose(const_cast<float*>(afMatrix)) == S_OK);

#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantDirectX10::SetMatrix4x4Array

inline bool CShaderConstantDirectX10::SetMatrix4x4Array(const float* pMatrixArray, unsigned int uiNumMatrices) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ) && pMatrixArray)
	{
#endif
		bSuccess = (m_pParameter->AsMatrix( )->SetMatrixTransposeArray(const_cast<float*>(pMatrixArray), 0, uiNumMatrices) == S_OK);

#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstant::SetTexture

inline bool CShaderConstant::SetTexture(const CTexture& texTexture) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		bSuccess = (m_tShaderConstantPolicy.m_pParameter->AsShaderResource( )->SetResource(texTexture.m_tTexturePolicy.GetTextureObject( )) == S_OK);

#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstant::SetTexture

inline bool CShaderConstant::SetTexture(const CDepthTexture& cDepthTexture) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		bSuccess = (m_tShaderConstantPolicy.m_pParameter->AsShaderResource( )->SetResource(cDepthTexture.m_tDepthTexturePolicy.GetTextureObject( )) == S_OK);

#ifdef _DEBUG	
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstant::SetTextureIndex

inline bool CShaderConstant::SetTextureIndex(const CTexture& /*texTexture*/, unsigned int /*uiIndex*/) const
{
	bool bSuccess = false;

#ifdef _DEBUG
	if (IsValid( ))
	{
#endif
		// todo: not yet implemented

#ifdef _DEBUG	
	}
	else
		return false;
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstant::SetTextureIndex

inline bool CShaderConstant::SetTextureIndex(const CDepthTexture& /*cDepthTexture*/, unsigned int /*uiIndex*/) const
{
	// todo: not yet implemented
	return false;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderDirectX10::CShaderLoaderDirectX10

inline CShaderLoaderDirectX10::CShaderLoaderDirectX10( ) :
	m_pEffect(NULL)
{
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderDirectX10::~CShaderLoaderDirectX10

inline CShaderLoaderDirectX10::~CShaderLoaderDirectX10( )
{
	Release( );

#ifdef _DEBUG
	m_pEffect = NULL;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderDirectX10::GetError

inline const char* CShaderLoaderDirectX10::GetError(void) const
{
	return m_strError.c_str( );
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderDirectX10::OnResetDevice

inline void CShaderLoaderDirectX10::OnResetDevice(void)
{
	//if (m_pEffect != NULL)
	//	m_pEffect->OnResetDevice( );
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderDirectX10::OnLostDevice

inline void CShaderLoaderDirectX10::OnLostDevice(void)
{
	//if (m_pEffect != NULL)
	//	m_pEffect->OnLostDevice( );
}
