///////////////////////////////////////////////////////////////////////
//  RenderState.cpp
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) CONFIDENTIAL AND PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization, Inc. and
//  may not be copied, disclosed, or exploited except in accordance with 
//  the terms of that agreement.
//
//      Copyright (c) 2003-2009 IDV, Inc.
//      All rights reserved in all media.
//
//      IDV, Inc.
//      http://www.idvinc.com


///////////////////////////////////////////////////////////////////////
//  Preprocessor
#include "stdafx.h"

#include "Dx11Resource.h"
using namespace SpeedTree;


///////////////////////////////////////////////////////////////////////  
//  Static members

bool CRenderStateDirectX10::m_bDepthChanged = true;
D3D11_DEPTH_STENCIL_DESC CRenderStateDirectX10::m_sCurrentDepthStateDesc;
CRenderStateDirectX10::DepthStencilMap* CRenderStateDirectX10::m_pDepthStates = NULL;

bool CRenderStateDirectX10::m_bRasterizerChanged = true;
D3D11_RASTERIZER_DESC CRenderStateDirectX10::m_sCurrentRasterizerStateDesc;
CRenderStateDirectX10::RasterizerMap* CRenderStateDirectX10::m_pRasterizerStates = NULL;

bool CRenderStateDirectX10::m_bBlendChanged = true;
D3D11_BLEND_DESC CRenderStateDirectX10::m_sCurrentBlendDesc;
CRenderStateDirectX10::BlendMap* CRenderStateDirectX10::m_pBlendStates = NULL;

unsigned int CRenderStateDirectX10::m_uiRefCount = 0;


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::Initialize

void CRenderStateDirectX10::Initialize(void)
{
    ++m_uiRefCount;

    if (m_uiRefCount == 1)
    {
        m_sCurrentDepthStateDesc.DepthEnable = true;
        m_sCurrentDepthStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        m_sCurrentDepthStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
        m_sCurrentDepthStateDesc.StencilEnable = false;
        m_sCurrentDepthStateDesc.StencilReadMask = 0xFF;
        m_sCurrentDepthStateDesc.StencilWriteMask = 0xFF;
        m_sCurrentDepthStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        m_sCurrentDepthStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        m_sCurrentDepthStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        m_sCurrentDepthStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
        m_sCurrentDepthStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
        m_sCurrentDepthStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
        m_sCurrentDepthStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
        m_sCurrentDepthStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

        m_sCurrentRasterizerStateDesc.FillMode = D3D11_FILL_SOLID;
        m_sCurrentRasterizerStateDesc.CullMode = D3D11_CULL_NONE;
        m_sCurrentRasterizerStateDesc.FrontCounterClockwise = true;
        m_sCurrentRasterizerStateDesc.DepthBias = false;
        m_sCurrentRasterizerStateDesc.DepthBiasClamp = 0;
        m_sCurrentRasterizerStateDesc.SlopeScaledDepthBias = 0;
        m_sCurrentRasterizerStateDesc.DepthClipEnable = true;
        m_sCurrentRasterizerStateDesc.ScissorEnable = false;
        m_sCurrentRasterizerStateDesc.MultisampleEnable = true;
        m_sCurrentRasterizerStateDesc.AntialiasedLineEnable = false;

        m_sCurrentBlendDesc.AlphaToCoverageEnable = false;
        m_sCurrentBlendDesc.RenderTarget[0].BlendEnable = false;
        m_sCurrentBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        m_sCurrentBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
        m_sCurrentBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        m_sCurrentBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        m_sCurrentBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        m_sCurrentBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        m_sCurrentBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

        if (!m_pDepthStates)
            m_pDepthStates = new ("DepthStencilMap") DepthStencilMap;
    
        if (!m_pRasterizerStates)
            m_pRasterizerStates = new ("RasterizerMap") RasterizerMap;
    
        if (!m_pBlendStates)
            m_pBlendStates = new ("BlendMap") BlendMap;
    }

    assert(m_pDepthStates);
    assert(m_pRasterizerStates);
    assert(m_pBlendStates);
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::Destroy

void CRenderStateDirectX10::Destroy(void)
{
    --m_uiRefCount;
    if (m_uiRefCount == 0)
    {
        for (DepthStencilMap::iterator iter = m_pDepthStates->begin( ); iter != m_pDepthStates->end( ); ++iter)
            iter->second->Release( );

        for (RasterizerMap::iterator iter = m_pRasterizerStates->begin( ); iter != m_pRasterizerStates->end( ); ++iter)
            iter->second->Release( );

        for (BlendMap::iterator iter = m_pBlendStates->begin( ); iter != m_pBlendStates->end( ); ++iter)
            iter->second->Release( );

        st_delete<DepthStencilMap>(m_pDepthStates, "DepthStencilMap");
        st_delete<RasterizerMap>(m_pRasterizerStates, "RasterizerMap");
        st_delete<BlendMap>(m_pBlendStates, "BlendMap");
    }
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::SetAlphaToCoverage

void CRenderStateDirectX10::SetAlphaToCoverage(bool bFlag)
{
    m_sCurrentBlendDesc.AlphaToCoverageEnable = bFlag;
    m_bBlendChanged = true;
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::SetBlending

void CRenderStateDirectX10::SetBlending(bool bFlag)
{
    assert(DX11::Device( ));

    if (bFlag)
    {
        m_sCurrentBlendDesc.RenderTarget[0].BlendEnable = true;
        m_sCurrentBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
        m_sCurrentBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    }
    else
    {
        m_sCurrentBlendDesc.RenderTarget[0].BlendEnable = false;
        m_sCurrentBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        m_sCurrentBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    }

    m_bBlendChanged = true;
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::SetColorMask

void CRenderStateDirectX10::SetColorMask(bool bRed, bool bGreen, bool bBlue, bool bAlpha)
{
    UINT8 dwMask = 0;
    if (bRed)
        dwMask |= D3D11_COLOR_WRITE_ENABLE_RED;
    if (bGreen)
        dwMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
    if (bBlue)
        dwMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
    if (bAlpha)
        dwMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

    m_sCurrentBlendDesc.RenderTarget[0].RenderTargetWriteMask = dwMask;
    m_bBlendChanged = true;
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::SetDepthMask

void CRenderStateDirectX10::SetDepthMask(bool bFlag)
{
    assert(DX11::Device( ));

    m_sCurrentDepthStateDesc.DepthWriteMask = bFlag ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    m_bDepthChanged = true;
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::SetDepthTestFunc

void CRenderStateDirectX10::SetDepthTestFunc(EDepthTestFunc eDepthTestFunc)
{
    assert(DX11::Device( ));

    if (eDepthTestFunc == DEPTHTEST_EQUAL)
        m_sCurrentDepthStateDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
    else
        m_sCurrentDepthStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
    m_bDepthChanged = true;
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::SetDepthTesting

void CRenderStateDirectX10::SetDepthTesting(bool bFlag)
{
    assert(DX11::Device( ));

    m_sCurrentDepthStateDesc.DepthEnable = bFlag;
    m_bDepthChanged = true;
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::SetFaceCulling

void CRenderStateDirectX10::SetFaceCulling(ECullType eCullType)
{
    assert(DX11::Device( ));

    if (eCullType == CULLTYPE_BACK)
        m_sCurrentRasterizerStateDesc.CullMode = D3D11_CULL_BACK;
    else if (eCullType == CULLTYPE_FRONT)
        m_sCurrentRasterizerStateDesc.CullMode = D3D11_CULL_FRONT;
    else
        m_sCurrentRasterizerStateDesc.CullMode = D3D11_CULL_NONE;

    m_bRasterizerChanged = true;
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::SetPolygonOffset

void CRenderStateDirectX10::SetPolygonOffset(float fFactor, float fUnits)
{
    assert(DX11::Device( ));

    if (fFactor != 0.0f || fUnits != 0.0f)
    {
		m_sCurrentRasterizerStateDesc.DepthBias = true;
		m_sCurrentRasterizerStateDesc.DepthBiasClamp = -1.0f;
    }
	else
	{
		m_sCurrentRasterizerStateDesc.DepthBias = false;
		m_sCurrentRasterizerStateDesc.DepthBiasClamp = 0.0f;
	}
	
	m_sCurrentRasterizerStateDesc.SlopeScaledDepthBias = fFactor;
	
	m_bRasterizerChanged = true;
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::SetMultisampling

void CRenderStateDirectX10::SetMultisampling(bool bMultisample)
{
    assert(DX11::Device( ));

    m_sCurrentRasterizerStateDesc.MultisampleEnable = bMultisample;
    m_bRasterizerChanged = false;
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::SetRenderStyle

void CRenderStateDirectX10::SetRenderStyle(ERenderStyle eStyle)
{
    assert(DX11::Device( ));

    m_sCurrentRasterizerStateDesc.FillMode = ((eStyle == RENDERSTYLE_WIREFRAME) ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID);
    m_bRasterizerChanged = false;
}


///////////////////////////////////////////////////////////////////////  
//  CRenderStateDirectX10::ApplyStates

void CRenderStateDirectX10::ApplyStates(void)
{
    // depth/stencil states
    if (m_bDepthChanged)
    {
        DepthStencilMap::iterator iter = m_pDepthStates->find(m_sCurrentDepthStateDesc);
        if (iter == m_pDepthStates->end( ))
        {
            ID3D11DepthStencilState* pNew = NULL;
            DX11::Device( )->CreateDepthStencilState(&m_sCurrentDepthStateDesc, &pNew);
            (*m_pDepthStates)[m_sCurrentDepthStateDesc] = pNew;
            iter = m_pDepthStates->find(m_sCurrentDepthStateDesc);
        }
        DX11::Context( )->OMSetDepthStencilState(iter->second, 0);
        m_bDepthChanged = false;
    }

    // rasterizer states
    if (m_bRasterizerChanged)
    {
        RasterizerMap::iterator iter = m_pRasterizerStates->find(m_sCurrentRasterizerStateDesc);
        if (iter == m_pRasterizerStates->end( ))
        {
            ID3D11RasterizerState* pNew = NULL;
            DX11::Device( )->CreateRasterizerState(&m_sCurrentRasterizerStateDesc, &pNew);
            (*m_pRasterizerStates)[m_sCurrentRasterizerStateDesc] = pNew;
            iter = m_pRasterizerStates->find(m_sCurrentRasterizerStateDesc);
        }
        DX11::Context( )->RSSetState(iter->second);
        m_bRasterizerChanged = false;
    }

    // blend states
    if (m_bBlendChanged)
    {
        BlendMap::iterator iter = m_pBlendStates->find(m_sCurrentBlendDesc);
        if (iter == m_pBlendStates->end( ))
        {
            ID3D11BlendState* pNew = NULL;
            DX11::Device( )->CreateBlendState(&m_sCurrentBlendDesc, &pNew);
            (*m_pBlendStates)[m_sCurrentBlendDesc] = pNew;
            iter = m_pBlendStates->find(m_sCurrentBlendDesc);
        }
        FLOAT afPixels[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        DX11::Context( )->OMSetBlendState(iter->second, afPixels, 0xffffffff);
        m_bBlendChanged = false;
    }
}


///////////////////////////////////////////////////////////////////////  
//  Operators for state block maps

bool operator != (const D3D11_DEPTH_STENCIL_DESC& sL, const D3D11_DEPTH_STENCIL_DESC& sR)
{
    return (memcmp(&sL, &sR, sizeof(D3D11_DEPTH_STENCIL_DESC)) != 0);
}

bool operator < (const D3D11_DEPTH_STENCIL_DESC& sL, const D3D11_DEPTH_STENCIL_DESC& sR)
{
    return (memcmp(&sL, &sR, sizeof(D3D11_DEPTH_STENCIL_DESC)) < 0);
}

bool operator != (const D3D11_RASTERIZER_DESC& sL, const D3D11_RASTERIZER_DESC& sR)
{
    return (memcmp(&sL, &sR, sizeof(D3D11_RASTERIZER_DESC)) != 0);
}

bool operator < (const D3D11_RASTERIZER_DESC& sL, const D3D11_RASTERIZER_DESC& sR)
{
    return (memcmp(&sL, &sR, sizeof(D3D11_RASTERIZER_DESC)) < 0);
}

bool operator != (const D3D11_BLEND_DESC& sL, const D3D11_BLEND_DESC& sR)
{
    return (memcmp(&sL, &sR, sizeof(D3D11_BLEND_DESC)) != 0);
}

bool operator < (const D3D11_BLEND_DESC& sL, const D3D11_BLEND_DESC& sR)
{
    return (memcmp(&sL, &sR, sizeof(D3D11_BLEND_DESC)) < 0);
}





