// Copyright 2011 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
#include "stdafx.h"
#include "..\include\Oscilloscope.h"
#include "EffectUtil.h"

COscilloscope::COscilloscope(void)
{
    m_OscilloscopeParams.TimeScale = 0.f;
    m_OscilloscopeParams.iFirstFrameNum = 0;
    
    m_ScreenPosition.left = 5;
    m_ScreenPosition.top = 70;
    m_ScreenPosition.right = 220;
    m_ScreenPosition.bottom = 150;
}

COscilloscope::~COscilloscope(void)
{

}

HRESULT COscilloscope::CompileShaders( ID3D11Device* pd3dDevice )
{
    D3D_SHADER_MACRO DefinedMacroses[64];
    ZeroMemory(DefinedMacroses, sizeof(DefinedMacroses) );
    int iMacrosesDefinedCount = 0;

    DefinedMacroses[iMacrosesDefinedCount].Name = "NUMBER_OF_FRAMES_TO_DISPLAY";
    char strNumFrames[8];
    sprintf_s(strNumFrames, sizeof(strNumFrames)/sizeof(strNumFrames[0]), "%d", (int)m_FrameTimes.size());
    DefinedMacroses[iMacrosesDefinedCount].Definition = strNumFrames;
    iMacrosesDefinedCount++;

    HRESULT hr;
    CComPtr<ID3DBlob> pEffectBuffer;

    hr = CompileShaderFromFile( L"fx\\Oscilloscope.hlsl", DefinedMacroses, "vs_4_0", &pEffectBuffer, "RenderOscilloscopeVS" );
    CHECK_HR_RET(hr, _T("Failed to compile shader from file"));
    hr = pd3dDevice->CreateVertexShader(pEffectBuffer->GetBufferPointer(), pEffectBuffer->GetBufferSize(), 0, &m_pRenderOscilloscopeVS);
    CHECK_HR_RET(hr, _T("Failed to create vertex shader from byte code"));
    pEffectBuffer.Release();

    hr = CompileShaderFromFile( L"fx\\Oscilloscope.hlsl", DefinedMacroses, "ps_4_0", &pEffectBuffer, "RenderOscilloscopePS" );
    CHECK_HR_RET(hr, _T("Failed to compile shader from file"));
    hr = pd3dDevice->CreatePixelShader(pEffectBuffer->GetBufferPointer(), pEffectBuffer->GetBufferSize(), 0, &m_pRenderOscilloscopePS);
    CHECK_HR_RET(hr, _T("Failed to create pixel shader from byte code"));
    pEffectBuffer.Release();

    {
        CD3D11_BUFFER_DESC desc(
            (UINT)(sizeof(D3DXVECTOR4) * m_FrameTimes.size() ),
            D3D11_BIND_CONSTANT_BUFFER,
            D3D11_USAGE_DYNAMIC,
            D3D11_CPU_ACCESS_WRITE);

        V_RETURN( pd3dDevice->CreateBuffer(&desc, 0, &m_pFrameTimesBuf) );
    }

    return S_OK;
}

HRESULT COscilloscope::OnD3D11CreateDevice( ID3D11Device* pd3dDevice,
                                            ID3D11DeviceContext* pd3dImmediateContext,
                                            CDXUTTextHelper* pTxtHelper )
{
    HRESULT hr;

    m_pTxtHelper = pTxtHelper;

    D3D11_RASTERIZER_DESC SolidFillNoCullRSDesc = 
    {
        D3D11_FILL_SOLID,
        D3D11_CULL_NONE,
        false, //BOOL FrontCounterClockwise;
        0,// INT DepthBias;
        0,// FLOAT DepthBiasClamp;
        0,// FLOAT SlopeScaledDepthBias;
        false,//BOOL DepthClipEnable;
        false,//BOOL ScissorEnable;
        false,//BOOL MultisampleEnable;
        false,//BOOL AntialiasedLineEnable;
    };
    V_RETURN( pd3dDevice->CreateRasterizerState(&SolidFillNoCullRSDesc, &m_pRSSolidFillNoCull) );

    D3D11_DEPTH_STENCIL_DESC DSDisabledDesc = 
    {
        false, //BOOL DepthEnable;
        D3D11_DEPTH_WRITE_MASK_ZERO, //D3D11_DEPTH_WRITE_MASK DepthWriteMask;
        D3D11_COMPARISON_ALWAYS, //D3D11_COMPARISON_FUNC DepthFunc;
        false, //BOOL StencilEnable;
        0,//UINT8 StencilReadMask;
        0,//UINT8 StencilWriteMask;
        D3D11_STENCIL_OP_KEEP,//D3D11_DEPTH_STENCILOP_DESC FrontFace;
        D3D11_STENCIL_OP_KEEP//D3D11_DEPTH_STENCILOP_DESC BackFace;
    };
    V_RETURN( pd3dDevice->CreateDepthStencilState(&DSDisabledDesc, &m_pDisableDepthDS) );

    D3D11_BLEND_DESC OverAlphaBlendDesc;
    memset(&OverAlphaBlendDesc, 0, sizeof(OverAlphaBlendDesc));
    OverAlphaBlendDesc.AlphaToCoverageEnable = false;
    OverAlphaBlendDesc.IndependentBlendEnable = false;
    OverAlphaBlendDesc.RenderTarget[0].BlendEnable = true;
    OverAlphaBlendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    OverAlphaBlendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    OverAlphaBlendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    OverAlphaBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    OverAlphaBlendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    OverAlphaBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    OverAlphaBlendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    V_RETURN( pd3dDevice->CreateBlendState(&OverAlphaBlendDesc, &m_pOverAlphaBlend) );

    // Create constant buffers
    {
        CD3D11_BUFFER_DESC desc(
            (sizeof(SOscilloscopeParams) + 15) & (-16),
            D3D11_BIND_CONSTANT_BUFFER,
            D3D11_USAGE_DYNAMIC,
            D3D11_CPU_ACCESS_WRITE);

        V_RETURN( pd3dDevice->CreateBuffer(&desc, 0, &m_pOscilloscopeParamsBuf) );
    }

    return S_OK;
}
 
void COscilloscope::OnD3D11DestroyDevice( )
{
    m_pRSSolidFillNoCull.Release();
    m_pDisableDepthDS.Release();
    m_pOverAlphaBlend.Release();
    m_pRenderOscilloscopeVS.Release();
    m_pRenderOscilloscopePS.Release();
    m_pOscilloscopeParamsBuf.Release();
    m_pFrameTimesBuf.Release();
}

void COscilloscope::SetLastFrameTime(float fFrameTime)
{
    // Put last frame time into the cyclic buffer
    m_FrameTimes[m_OscilloscopeParams.iFirstFrameNum] = fFrameTime;
    // Increment buffer position
    if( m_OscilloscopeParams.iFirstFrameNum < (int)m_FrameTimes.size()-1 )
        m_OscilloscopeParams.iFirstFrameNum++;
    else
        m_OscilloscopeParams.iFirstFrameNum = 0;
        
    // Determine max frame time
    float fMaxFrameTime = 0;
    std::vector<float>::const_iterator it = m_FrameTimes.begin();
    for(; it != m_FrameTimes.end(); it++)
    {
        fMaxFrameTime = max(fMaxFrameTime, *it);
    }
    const float fAdoptionSpeed = 10.f;
    float fAdoptionScale = min(fAdoptionSpeed * fFrameTime, 1.f);
    // Smoothly update time scale
    m_OscilloscopeParams.TimeScale = (1-fAdoptionScale) * m_OscilloscopeParams.TimeScale + fAdoptionScale * fMaxFrameTime;
}

HRESULT COscilloscope::OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                                const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc )
{
    m_uiBackBufferWidth  = pBackBufferSurfaceDesc->Width;
    m_uiBackBufferHeight = pBackBufferSurfaceDesc->Height;
    
    return S_OK;
}

void COscilloscope::OnD3D11ReleasingSwapChain( )
{
    m_pRenderOscilloscopeVS.Release();
    m_pRenderOscilloscopePS.Release();
    m_pFrameTimesBuf.Release();
}

void COscilloscope::SetScreenPosition(const RECT &ScreenPosition)
{
    m_ScreenPosition = ScreenPosition;
    // If width of oscilloscope has changed, it is necessary to update shaders and
    // times buffer
    if( m_FrameTimes.size() != m_ScreenPosition.right - m_ScreenPosition.left )
    {
        m_FrameTimes.resize(m_ScreenPosition.right - m_ScreenPosition.left);
        m_OscilloscopeParams.iFirstFrameNum = 0;
        m_pRenderOscilloscopeVS.Release();
        m_pRenderOscilloscopePS.Release();
        m_pFrameTimesBuf.Release();
    }
}

void COscilloscope::Render( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext )
{
    if( !m_pRenderOscilloscopeVS || 
        !m_pRenderOscilloscopePS )
        CompileShaders( pd3dDevice );

    // Calculate projection space coordinates
    D3DXVECTOR4 OscilloscopePosPS;
    m_OscilloscopeParams.PosPS.x = (float)m_ScreenPosition.left/(float)m_uiBackBufferWidth * 2.f - 1.f;
    m_OscilloscopeParams.PosPS.z = (float)m_ScreenPosition.right/(float)m_uiBackBufferWidth * 2.f - 1.f;
    m_OscilloscopeParams.PosPS.y = -(float)m_ScreenPosition.top/(float)m_uiBackBufferHeight * 2.f + 1.f;
    m_OscilloscopeParams.PosPS.w = -(float)m_ScreenPosition.bottom/(float)m_uiBackBufferHeight * 2.f + 1.f;

    // Upload data to the GPU
    D3D11_MAPPED_SUBRESOURCE MapData;
    pd3dImmediateContext->Map(m_pOscilloscopeParamsBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &MapData);
    *(SOscilloscopeParams *)MapData.pData = m_OscilloscopeParams;
    pd3dImmediateContext->Unmap(m_pOscilloscopeParamsBuf, 0);

    pd3dImmediateContext->Map(m_pFrameTimesBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &MapData);
    for(int i=0; i < (int)m_FrameTimes.size(); i++)
        ((D3DXVECTOR4*)MapData.pData)[i] = D3DXVECTOR4(m_FrameTimes[i], m_FrameTimes[i], m_FrameTimes[i], m_FrameTimes[i]);
    pd3dImmediateContext->Unmap(m_pFrameTimesBuf, 0);

    pd3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    pd3dImmediateContext->RSSetState( m_pRSSolidFillNoCull );
    pd3dImmediateContext->OMSetDepthStencilState( m_pDisableDepthDS, 0 );
    float BlendFactors[] = {0.f,0.f,0.f,0.f};
    pd3dImmediateContext->OMSetBlendState( m_pOverAlphaBlend, BlendFactors, 0xFFFFFFFF );
    pd3dImmediateContext->VSSetShader( m_pRenderOscilloscopeVS, NULL, 0 );
    pd3dImmediateContext->PSSetShader( m_pRenderOscilloscopePS, NULL, 0 );
    ID3D11Buffer *pBuffs[] = {m_pOscilloscopeParamsBuf, m_pFrameTimesBuf};
    pd3dImmediateContext->VSSetConstantBuffers(0,2,pBuffs);
    pd3dImmediateContext->PSSetConstantBuffers(0,2,pBuffs);
    pd3dImmediateContext->Draw(4,0);

    m_pTxtHelper->Begin();
    m_pTxtHelper->SetInsertionPos( m_ScreenPosition.left, m_ScreenPosition.bottom );
    m_pTxtHelper->SetForegroundColor( D3DXCOLOR( 1.0f, 1.f, 0.0f, 0.8f ) );
    m_pTxtHelper->DrawFormattedTextLine( L"Time scale: %4.1f ms", m_OscilloscopeParams.TimeScale * 1000.f );
    m_pTxtHelper->End();
}
