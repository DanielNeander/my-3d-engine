///////////////////////////////////////////////////////////////////////
//  DepthTextureDirectX9.cpp
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
//#include "Utility.h"
using namespace SpeedTree;


///////////////////////////////////////////////////////////////////////
//  CDepthTextureDirectX10::Create

bool CDepthTextureDirectX10::Create(int nWidth, int nHeight)
{
    if (!m_bInitSucceeded)
    {
        m_nBufferWidth = nWidth;
        m_nBufferHeight = nHeight;

        D3D11_TEXTURE2D_DESC sDesc;
        ZeroMemory(&sDesc, sizeof(sDesc));
        sDesc.Width = nWidth;
        sDesc.Height = nHeight;
        sDesc.MipLevels = sDesc.ArraySize = 1;
        sDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sDesc.SampleDesc.Count = 1;
        sDesc.Usage = D3D11_USAGE_DEFAULT;
        sDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        sDesc.CPUAccessFlags = 0;

        ID3D11Texture2D* pColorTexture = NULL;
        ID3D11Texture2D* pDepthTexture = NULL;

        if (DX11::Device( )->CreateTexture2D(&sDesc, NULL, &pColorTexture) == S_OK)
        {
            sDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
            sDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
            m_bInitSucceeded = (DX11::Device( )->CreateTexture2D(&sDesc, NULL, &pDepthTexture) == S_OK);
        }

        if (!m_bInitSucceeded)
            //CCore::SetError("Failed to create DirectX10 render target textures");
			assert(false);
        else
        {
            m_bInitSucceeded &= (DX11::Device( )->CreateShaderResourceView(pColorTexture, NULL, &m_pColorTexture) == S_OK);
            m_bInitSucceeded &= (DX11::Device( )->CreateRenderTargetView(pColorTexture, NULL, &m_pDepthTextureView) == S_OK);

            D3D11_SHADER_RESOURCE_VIEW_DESC sSRDesc;
            ZeroMemory(&sSRDesc, sizeof(sSRDesc));
            sSRDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            sSRDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            sSRDesc.Texture2D.MostDetailedMip = 0;
            sSRDesc.Texture2D.MipLevels = 1;

            D3D11_DEPTH_STENCIL_VIEW_DESC sDSDesc;
            ZeroMemory(&sDSDesc, sizeof(sDSDesc));
            sDSDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
            sDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            sDSDesc.Texture2D.MipSlice = 0;

            m_bInitSucceeded &= (DX11::Device( )->CreateShaderResourceView(pDepthTexture, &sSRDesc, &m_pDepthTexture) == S_OK);
            m_bInitSucceeded &= (DX11::Device( )->CreateDepthStencilView(pDepthTexture, &sDSDesc, &m_pDepthStencilView) == S_OK);

            if (!m_bInitSucceeded)
                //CCore::SetError("Failed to create DirectX10 render targets");
				assert(false);

            pColorTexture->Release( );
            pDepthTexture->Release( );
        }
    }

    return m_bInitSucceeded;
}


///////////////////////////////////////////////////////////////////////
//  CDepthTextureDirectX10::Release

void CDepthTextureDirectX10::Release(void)
{
    // release surfaces first
    if (m_pDepthTextureView != NULL)
    {
        m_pDepthTextureView->Release( );
        m_pDepthTextureView = NULL;
    }

    if (m_pDepthStencilView != NULL)
    {
        m_pDepthStencilView->Release( );
        m_pDepthStencilView = NULL;
    }

    // release textures
    if (m_pColorTexture != NULL)
    {
        m_pColorTexture->Release( );
        m_pColorTexture = NULL;
    }

    if (m_pDepthTexture != NULL)
    {
        m_pDepthTexture->Release( );
        m_pDepthTexture = NULL;
    }

    // release old surfaces just in case
    if (m_pOldDepthTextureView != NULL)
    {
        m_pOldDepthTextureView->Release( );
        m_pOldDepthTextureView = NULL;
    }

    if (m_pOldDepthStencilView != NULL)
    {
        m_pOldDepthStencilView->Release( );
        m_pOldDepthStencilView = NULL;
    }
}


///////////////////////////////////////////////////////////////////////
//  CDepthTextureDirectX10::SetAsTarget

bool CDepthTextureDirectX10::SetAsTarget(void)
{
    assert(DX11::Device( ));

    DX11::Context( )->OMGetRenderTargets(1, &m_pOldDepthTextureView, &m_pOldDepthStencilView);
    DX11::Context( )->OMSetRenderTargets(1, &m_pDepthTextureView, m_pDepthStencilView);
    DX11::SetMainRenderTargetView(m_pDepthTextureView);
    DX11::SetMainDepthStencilView(m_pDepthStencilView);

    UINT uiViewports = 1;
    DX11::Context( )->RSGetViewports(&uiViewports, &m_sOldViewport);

    D3D11_VIEWPORT sViewData = { 0, 0, m_nBufferWidth, m_nBufferHeight, 0.0f, 1.0f };
    DX11::Context( )->RSSetViewports(1, &sViewData);

    return true;
}


///////////////////////////////////////////////////////////////////////
//  CDepthTextureDirectX10::ReleaseAsTarget

bool CDepthTextureDirectX10::ReleaseAsTarget(void)
{
    DX11::Context( )->OMSetRenderTargets(1, &m_pOldDepthTextureView, m_pOldDepthStencilView);

    DX11::SetMainRenderTargetView(m_pOldDepthTextureView);
    DX11::SetMainDepthStencilView(m_pOldDepthStencilView);

    if (m_pOldDepthTextureView != NULL)
    {
        m_pOldDepthTextureView->Release( );
        m_pOldDepthTextureView = NULL;
    }

    if (m_pOldDepthStencilView != NULL)
    {
        m_pOldDepthStencilView->Release( );
        m_pOldDepthStencilView = NULL;
    }

    DX11::Context( )->RSSetViewports(1, &m_sOldViewport);

    return true;
}


///////////////////////////////////////////////////////////////////////
//  CDepthTextureDirectX10::BindAsTexture

bool CDepthTextureDirectX10::BindAsTexture(void)
{
    return true;
}


///////////////////////////////////////////////////////////////////////
//  CDepthTextureDirectX10::UnBindAsTexture

bool CDepthTextureDirectX10::UnBindAsTexture(void)
{
    return true;
}


///////////////////////////////////////////////////////////////////////
//  CDepthTextureDirectX10::EnableShadowMapComparison

void CDepthTextureDirectX10::EnableShadowMapComparison(bool bFlag)
{
    //UnreferencedParameter(bFlag);
}
