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
//
//  *** Release version 5.0 ***


///////////////////////////////////////////////////////////////////////
//  CShaderTechniqueOpenGL::CShaderTechniqueOpenGL

inline CShaderTechniqueOpenGL::CShaderTechniqueOpenGL( ) :
	m_cgTechnique(NULL),
	m_cgPass(NULL)
{
}


///////////////////////////////////////////////////////////////////////
//  CShaderTechniqueOpenGL::~CShaderTechniqueOpenGL

inline CShaderTechniqueOpenGL::~CShaderTechniqueOpenGL( )
{
#ifndef NDEBUG
	m_cgTechnique = NULL;
	m_cgPass = NULL;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueOpenGL::Bind

inline st_bool CShaderTechniqueOpenGL::Bind(st_uint32 /*uiPass*/)
{
	st_bool bSuccess = false;

	m_cgPass = cgGetFirstPass(m_cgTechnique);
	if (m_cgPass)
	{
		cgSetPassState(m_cgPass);

		bSuccess = true;
	}

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueOpenGL::UnBind

inline st_bool CShaderTechniqueOpenGL::UnBind(void)
{
	m_cgPass = NULL;

	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueOpenGL::CommitConstants

inline st_bool CShaderTechniqueOpenGL::CommitConstants(void)
{
#ifdef SPEEDTREE_CG_DEFERRED_PARAMETERS
	cgUpdatePassParameters(m_cgPass);
#endif

	return true;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueOpenGL::CommitTextures

inline st_bool CShaderTechniqueOpenGL::CommitTextures(void)
{
	st_bool bSuccess = false;

#ifndef NDEBUG
	if (m_cgPass)
	{
#endif
		cgSetPassState(m_cgPass);
		bSuccess = true;
#ifndef NDEBUG
	}
#endif

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderTechniqueOpenGL::IsValid

inline st_bool CShaderTechniqueOpenGL::IsValid(void) const
{
	return (m_cgTechnique != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::CShaderConstantOpenGL

inline CShaderConstantOpenGL::CShaderConstantOpenGL( ) :
	m_cgParameter(NULL)
{
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::~CShaderConstantOpenGL

inline CShaderConstantOpenGL::~CShaderConstantOpenGL( )
{
#ifndef NDEBUG
	m_cgParameter = NULL;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::IsValid

inline st_bool CShaderConstantOpenGL::IsValid(void) const
{
	return (m_cgParameter != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::Set1f

inline st_bool CShaderConstantOpenGL::Set1f(st_float32 x) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif
		cgGLSetParameter1f(m_cgParameter, x);
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::Set2f

inline st_bool CShaderConstantOpenGL::Set2f(st_float32 x, st_float32 y) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif
		cgGLSetParameter2f(m_cgParameter, x, y);
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::Set2fv

inline st_bool CShaderConstantOpenGL::Set2fv(const st_float32 afValues[2]) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif
		cgGLSetParameter2fv(m_cgParameter, afValues);
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::Set2f

inline st_bool CShaderConstantOpenGL::Set3f(st_float32 x, st_float32 y, st_float32 z) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif
		cgGLSetParameter3f(m_cgParameter, x, y, z);
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::Set3fv

inline st_bool CShaderConstantOpenGL::Set3fv(const st_float32 afValues[3]) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif
		cgGLSetParameter3fv(m_cgParameter, afValues);
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::Set3fvPlus1f

inline st_bool CShaderConstantOpenGL::Set3fvPlus1f(const st_float32 afValues[3], st_float32 w) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif
		cgGLSetParameter4f(m_cgParameter, afValues[0], afValues[1], afValues[2], w);
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::Set4f

inline st_bool CShaderConstantOpenGL::Set4f(st_float32 x, st_float32 y, st_float32 z, st_float32 w) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif
		cgGLSetParameter4f(m_cgParameter, x, y, z, w);
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::Set4fv

inline st_bool CShaderConstantOpenGL::Set4fv(const st_float32 afValues[4]) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif
		cgGLSetParameter4fv(m_cgParameter, afValues);
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::SetArray1f

inline st_bool CShaderConstantOpenGL::SetArray1f(const st_float32* pArray, st_int32 nSize, st_int32 nOffset) const
{
#ifndef NDEBUG
	if (IsValid( ) && pArray)
	{
#endif

#ifdef PS3
		cgGLSetParameterArray1f(m_cgParameter, nOffset, nSize, pArray);
#else
		for (st_int32 i = 0; i < nSize; ++i)
		{
			CGparameter pElement = cgGetArrayParameter(m_cgParameter, i + nOffset);
			cgGLSetParameter1fv(pElement, pArray + i);
		}

		return true;
#endif

#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::SetArray3f

inline st_bool CShaderConstantOpenGL::SetArray3f(const st_float32* pArray, st_int32 nSize, st_int32 nOffset) const
{
#ifndef NDEBUG
	if (IsValid( ) && pArray)
	{
#endif

#ifdef PS3
		cgGLSetParameterArray3f(m_cgParameter, nOffset, nSize, pArray);
#else
		for (st_int32 i = 0; i < nSize; ++i)
		{
			CGparameter pElement = cgGetArrayParameter(m_cgParameter, i + nOffset);
			cgGLSetParameter3fv(pElement, pArray + i * 3);
		}

		return true;
#endif

#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::SetArray4f

inline st_bool CShaderConstantOpenGL::SetArray4f(const st_float32* pArray, st_int32 nSize, st_int32 nOffset) const
{
#ifndef NDEBUG
	if (IsValid( ) && pArray)
	{
#endif

#ifdef PS3
		cgGLSetParameterArray4f(m_cgParameter, nOffset, nSize, pArray);
#else
		// this still doesn't work
		//cgGLSetParameterArray4f(m_cgParameter, nOffset, nSize, pArray);

		// alternate code (cgGLSetParameterArray4f() hasn't been reliable in the Windows version of Cg);
		// when cgGLSetParameterArray4f() doesn't work correctly, the symptom is largely unanimated leaf cards
		for (st_int32 i = 0; i < nSize; ++i)
		{
			CGparameter pElement = cgGetArrayParameter(m_cgParameter, i + nOffset);
			cgGLSetParameter4fv(pElement, pArray + i * 4);
		}

		return true;
#endif

#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::SetMatrix

inline st_bool CShaderConstantOpenGL::SetMatrix(const st_float32 afMatrix[16]) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif
		cgGLSetMatrixParameterfc(m_cgParameter, afMatrix);
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::SetMatrixTranspose

inline st_bool CShaderConstantOpenGL::SetMatrixTranspose(const st_float32 afMatrix[16]) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif
		cgGLSetMatrixParameterfr(m_cgParameter, afMatrix);
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::SetMatrixIndex

inline st_bool CShaderConstantOpenGL::SetMatrixIndex(const st_float32 afMatrix[16], st_uint32 uiIndex) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif

#ifdef PS3
		cgGLSetMatrixParameterArrayfr(m_cgParameter, uiIndex, 1, afMatrix);
#else
		// alternate code (cgGLSetParameterArray4f wasn't reliable)
		CGparameter pElement = cgGetArrayParameter(m_cgParameter, uiIndex);
		cgGLSetMatrixParameterfc(pElement, afMatrix);
#endif
		return true;

#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstantOpenGL::SetMatrix4x4Array

inline st_bool CShaderConstantOpenGL::SetMatrix4x4Array(const st_float32* pMatrixArray, st_uint32 uiOffset, st_uint32 uiNumMatrices) const
{
#ifndef NDEBUG
	if (IsValid( ) && pMatrixArray)
	{
#endif
		cgGLSetMatrixParameterArrayfr(m_cgParameter, uiOffset, uiNumMatrices, pMatrixArray);
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstant::SetTexture

inline st_bool CShaderConstant::SetTexture(const CTexture& texTexture) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif
		cgGLSetupSampler(m_tShaderConstantPolicy.m_cgParameter, texTexture.m_tTexturePolicy.GetTextureObject( ));
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstant::SetTexture

inline st_bool CShaderConstant::SetTexture(const CDepthTexture& cDepthTexture) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif
		cgGLSetupSampler(m_tShaderConstantPolicy.m_cgParameter, cDepthTexture.m_tDepthTexturePolicy.GetTextureObject( ));
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstant::SetTextureIndex

inline st_bool CShaderConstant::SetTextureIndex(const CTexture& texTexture, st_uint32 uiIndex) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif
		CGparameter cgElement = cgGetArrayParameter(m_tShaderConstantPolicy.m_cgParameter, uiIndex);
		cgGLSetupSampler(cgElement, texTexture.m_tTexturePolicy.GetTextureObject( ));
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderConstant::SetTextureIndex

inline st_bool CShaderConstant::SetTextureIndex(const CDepthTexture& cDepthTexture, st_uint32 uiIndex) const
{
#ifndef NDEBUG
	if (IsValid( ))
	{
#endif
		CGparameter cgElement = cgGetArrayParameter(m_tShaderConstantPolicy.m_cgParameter, uiIndex);
		cgGLSetupSampler(cgElement, cDepthTexture.m_tDepthTexturePolicy.GetTextureObject( ));
		return true;
#ifndef NDEBUG	
	}
	else
		return false;
#endif
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderOpenGL::CShaderLoaderOpenGL

inline CShaderLoaderOpenGL::CShaderLoaderOpenGL( ) :
	m_pCgEffect(NULL)
{
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderOpenGL::~CShaderLoaderOpenGL

inline CShaderLoaderOpenGL::~CShaderLoaderOpenGL( )
{
	Release( );
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderOpenGL::GetError

inline const char* CShaderLoaderOpenGL::GetError(void) const
{
	return m_strError.c_str( );
}


