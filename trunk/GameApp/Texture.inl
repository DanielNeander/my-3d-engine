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


///////////////////////////////////////////////////////////////////////  
//  CTextureDirectX10::CTextureDirectX10

inline CTextureDirectX10::CTextureDirectX10( ) :
	m_pTexture(NULL)
{
}


///////////////////////////////////////////////////////////////////////  
//  CTextureDirectX10::~CTextureDirectX10

inline CTextureDirectX10::~CTextureDirectX10( )
{
	(void) Unload( );
}


///////////////////////////////////////////////////////////////////////  
//  CTextureDirectX10::FixedFunctionBind

inline bool CTextureDirectX10::FixedFunctionBind(void)
{
	bool bSuccess = false;

	return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTextureDirectX10::GetTextureObject

inline ID3D11ShaderResourceView* CTextureDirectX10::GetTextureObject(void) const
{
	return m_pTexture;
}

