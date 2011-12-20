///////////////////////////////////////////////////////////////////////  
//  Texture.inl
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
//  CTextureOpenGL::CTextureOpenGL

inline CTextureOpenGL::CTextureOpenGL( ) :
	m_uiTexture(0)
{
}


///////////////////////////////////////////////////////////////////////  
//  CTextureOpenGL::~CTextureOpenGL

inline CTextureOpenGL::~CTextureOpenGL( )
{
	(void) Unload( );
}


///////////////////////////////////////////////////////////////////////  
//  CTextureOpenGL::Load

inline st_bool CTextureOpenGL::Load(const char* pFilename, st_int32 nMaxAnisotropy)
{
	CString strExtension = CString(pFilename).Extension( );
	if (_stricmp(strExtension.c_str( ), "tga") == 0)
		return LoadTGA(pFilename, nMaxAnisotropy);
	else
		return LoadDDS(pFilename, nMaxAnisotropy);
}


///////////////////////////////////////////////////////////////////////  
//  CTextureOpenGL::FixedFunctionBind

inline st_bool CTextureOpenGL::FixedFunctionBind(void)
{
	st_bool bSuccess = false;

	if (m_uiTexture != 0)
	{
		glBindTexture(GL_TEXTURE_2D, m_uiTexture);
		bSuccess = true;
	}

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTextureOpenGL::GetTextureObject

inline st_uint32 CTextureOpenGL::GetTextureObject(void) const
{
	return m_uiTexture;
}
