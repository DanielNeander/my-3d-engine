///////////////////////////////////////////////////////////////////////
//  RenderState.inl
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
//  CRenderStateDirectX10::ClearScreen

inline void CRenderStateDirectX10::ClearScreen(bool bClearColor, bool bClearDepth)
{
	if (bClearColor)
		DX11::Context()->ClearRenderTargetView(DX11::MainRenderTargetView( ), D3DXVECTOR4(0, 0, 0, 1));

	if (bClearDepth)
		DX11::Context()->ClearDepthStencilView(DX11::MainDepthStencilView( ), D3D11_CLEAR_DEPTH, 1.0, 0);
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::SetAlphaFunction

inline void CRenderStateDirectX10::SetAlphaFunction(EAlphaFunc /*eFunc*/, float /*fValue*/)
{
	assert(DX11::Device( ));
	// alpha testing state gone in D3D11
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::SetAlphaTesting

inline void CRenderStateDirectX10::SetAlphaTesting(bool /*bFlag*/)
{
	assert(DX11::Device( ));
	// alpha testing state gone in D3D11
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::SetPointSize

inline void CRenderStateDirectX10::SetPointSize(float /*fSize*/)
{
	assert(DX11::Device( ));
	//DX10::Device( )->SetRenderState(D3DRS_POINTSIZE, *((DWORD*)&fSize));
}

