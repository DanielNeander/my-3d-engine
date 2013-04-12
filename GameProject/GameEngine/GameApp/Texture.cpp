///////////////////////////////////////////////////////////////////////
//  Texture.cpp
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
//  CTextureDirectX10 static member variables

CTextureDirectX10::TextureCacheMap* CTextureDirectX10::m_pCache = NULL;


///////////////////////////////////////////////////////////////////////  
//  CTextureDirectX10::Load
//
//  todo: missing texture caching

bool CTextureDirectX10::Load(const char* pFilename, int /*nMaxAnisotropy*/)
{
    if (pFilename && strlen(pFilename) > 0)
    {
        m_strFilename = pFilename;

        m_pTexture = SearchCache(pFilename);
        if (!m_pTexture)
        {
            assert(DX11::Device( ));

            D3DX11_IMAGE_LOAD_INFO sInfo;
            sInfo.Filter = D3DX11_FILTER_TRIANGLE | D3DX11_FILTER_MIRROR;
            if (D3DX11CreateShaderResourceViewFromFile(DX11::Device( ), pFilename, &sInfo, NULL, &m_pTexture, NULL) == S_OK)
            {
                AddToCache(pFilename, m_pTexture);
            }
        }
    }

    return (m_pTexture != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CTextureDirectX10::LoadColor

bool CTextureDirectX10::LoadColor(unsigned int uiColor)
{
    bool bSuccess = false;

    Unload( );

    assert(DX11::Device( ));

    CD3D11_TEXTURE2D_DESC sDesc;
    ZeroMemory(&sDesc, sizeof(sDesc));
    sDesc.Width = 4;
    sDesc.Height = 4;
    sDesc.MipLevels = sDesc.ArraySize = 1;
    sDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sDesc.SampleDesc.Count = 1;
    sDesc.Usage = D3D11_USAGE_DYNAMIC;
    sDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    sDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    ID3D11Texture2D* pTexture = NULL;
    if (DX11::Device( )->CreateTexture2D(&sDesc, NULL, &pTexture) == S_OK)
    {
        D3D11_MAPPED_SUBRESOURCE  sMappedTex;
        DX11::Context()->Map(pTexture, D3D11CalcSubresource(0, 0, 1), D3D11_MAP_WRITE_DISCARD, 0, &sMappedTex);

        UCHAR* pTexels = (UCHAR*)sMappedTex.pData;
        for (UINT uiRow = 0; uiRow < sDesc.Height; ++uiRow)
        {
            UINT uiRowStart = uiRow * sMappedTex.RowPitch;
            for (UINT uiCol = 0; uiCol < sDesc.Width; ++uiCol)
            {
                UINT uiColStart = uiCol * 4;
                pTexels[uiRowStart + uiColStart + 0] = (UCHAR)((uiColor & 0xff000000) >> 24); // Red
                pTexels[uiRowStart + uiColStart + 1] = (UCHAR)((uiColor & 0x00ff0000) >> 16); // Green
                pTexels[uiRowStart + uiColStart + 2] = (UCHAR)((uiColor & 0x0000ff00) >> 8);  // Blue
                pTexels[uiRowStart + uiColStart + 3] = (UCHAR)((uiColor & 0x000000ff) >> 0);  // Alpha
            }
        }

        DX11::Context()->Unmap(pTexture, D3D11CalcSubresource(0, 0, 1));

        bSuccess = (DX11::Device( )->CreateShaderResourceView(pTexture, NULL, &m_pTexture) == S_OK);
        pTexture->Release( );
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTextureDirectX10::LoadAlphaNoise

bool CTextureDirectX10::LoadAlphaNoise(int nWidth, int nHeight)
{
    bool bSuccess = false;

    Unload( );

    if (nWidth > 4 && nHeight > 4 && nWidth <= 4096 && nHeight <= 4096)
    {
        assert(DX11::Device( ));
        CRandom cRandom;

        D3D11_TEXTURE2D_DESC sDesc;
        ZeroMemory(&sDesc, sizeof(sDesc));
        sDesc.Width = nWidth;
        sDesc.Height = nHeight;
        sDesc.MipLevels = sDesc.ArraySize = 1;
        sDesc.Format = DXGI_FORMAT_A8_UNORM;
        sDesc.SampleDesc.Count = 1;
        sDesc.Usage = D3D11_USAGE_DYNAMIC;
        sDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        sDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        ID3D11Texture2D* pTexture = NULL;
        if (DX11::Device( )->CreateTexture2D(&sDesc, NULL, &pTexture) == S_OK)
        {
            D3D11_MAPPED_SUBRESOURCE sMappedTex;
            DX11::Context()->Map(pTexture, D3D11CalcSubresource(0, 0, 1), D3D11_MAP_WRITE_DISCARD, 0, &sMappedTex);

            UCHAR* pTexels = (UCHAR*)sMappedTex.pData;
            for (UINT uiRow = 0; uiRow < sDesc.Height; ++uiRow)
            {
                UINT uiRowStart = uiRow * sMappedTex.RowPitch;
                for (UINT uiCol = 0; uiCol < sDesc.Width; ++uiCol)
                {
                    UINT uiColStart = uiCol * 1;
                    pTexels[uiRowStart + uiColStart + 0] = (UCHAR)(cRandom.GetInteger(84, 255));
                }
            }

            DX11::Context()->Unmap(pTexture, D3D11CalcSubresource(0, 0, 1));

            bSuccess = (DX11::Device( )->CreateShaderResourceView(pTexture, NULL, &m_pTexture) == S_OK);
            pTexture->Release( );
        }
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTextureDirectX10::LoadSpecularLookup

bool CTextureDirectX10::LoadSpecularLookup(void)
{
    bool bSuccess = false;

    Unload( );

    assert(DX11::Device( ));
    
    // unused right now

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTextureDirectX10::LoadCubeNormalizer

bool CTextureDirectX10::LoadCubeNormalizer(int /*nResolution*/)
{
    bool bSuccess = false;

    Unload( );

    // unused right now

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CTextureDirectX10::Unload

bool CTextureDirectX10::Unload(void)
{
    bool bSuccess = false;

    if (!m_strFilename.empty( ))
    {
        bSuccess = RemoveFromCache(m_strFilename);
        m_strFilename.clear( );
    }
    else if (m_pTexture)
    {
        m_pTexture->Release( );
        m_pTexture = NULL;
        bSuccess = true;
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////
//  CTextureDirectX10::SearchCache

ID3D11ShaderResourceView* CTextureDirectX10::SearchCache(const CString& strFilename)
{
    ID3D11ShaderResourceView* pTexture = NULL;

    if (!strFilename.empty( ))
    {
        if (!m_pCache)
            m_pCache = new ("TextureCacheMap") TextureCacheMap;
        assert(m_pCache);

        TextureCacheMap::iterator iFind = m_pCache->find(strFilename);
        if (iFind != m_pCache->end( ))
        {
            ++iFind->second.m_nRefCount;
            pTexture = iFind->second.m_pTexture;
            assert(pTexture);
        }
    }

    return pTexture;
}


///////////////////////////////////////////////////////////////////////
//  CTextureDirectX10::AddToCache

void CTextureDirectX10::AddToCache(const CString& strFilename, ID3D11ShaderResourceView* pTexture)
{
    assert(pTexture);

    if (!m_pCache)
        m_pCache = new ("TextureCacheMap") TextureCacheMap;
    assert(m_pCache);

    SCacheEntry sEntry;
    sEntry.m_pTexture = pTexture;
    sEntry.m_nRefCount = 1;

    (*m_pCache)[strFilename] = sEntry;
}


///////////////////////////////////////////////////////////////////////
//  CTextureDirectX10::RemoveFromCache

bool CTextureDirectX10::RemoveFromCache(const CString& strFilename)
{
    bool bSuccess = false;

    if (m_pCache)
    {
        TextureCacheMap::iterator iFind = m_pCache->find(strFilename);
        if (iFind != m_pCache->end( ))
        {
            if (--iFind->second.m_nRefCount == 0)
            {
                assert(iFind->second.m_pTexture);
                iFind->second.m_pTexture->Release( );
                iFind->second.m_pTexture = NULL;

                m_pCache->erase(iFind);

                if (m_pCache->empty( ))
                    st_delete<TextureCacheMap>(m_pCache, "TextureCacheMap");
            }

            bSuccess = true;
        }
    }

    return bSuccess;
}

