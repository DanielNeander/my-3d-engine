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
#pragma once

#include <deque>
#include <vector>

// Implements oscilloscope functionality
class COscilloscope
{
public:
    COscilloscope(void);
    ~COscilloscope(void);

    // Creates Direct3D11 device resources
    HRESULT OnD3D11CreateDevice( ID3D11Device* pd3dDevice,
                                 ID3D11DeviceContext* pd3dImmediateContext,
                                 CDXUTTextHelper* pTxtHelper);
    
    // Releases Direct3D11 device resources
    void OnD3D11DestroyDevice( );

    HRESULT OnD3D11ResizedSwapChain( ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
                                     const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc );

    void OnD3D11ReleasingSwapChain( );

    void SetLastFrameTime(float fFrameTime);
    void SetScreenPosition(const RECT &ScreenPosition);

    // Renders the oscilloscope on the screen
    void Render( ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dImmediateContext );

private:
    HRESULT CompileShaders( ID3D11Device* pd3dDevice );

    CDXUTTextHelper* m_pTxtHelper;

    std::vector<float> m_FrameTimes;

    RECT m_ScreenPosition;

    UINT m_uiBackBufferWidth, m_uiBackBufferHeight;

    CComPtr<ID3D11RasterizerState> m_pRSSolidFillNoCull;
    CComPtr<ID3D11DepthStencilState> m_pDisableDepthDS;
    CComPtr<ID3D11BlendState> m_pOverAlphaBlend;
    CComPtr<ID3D11VertexShader> m_pRenderOscilloscopeVS;
    CComPtr<ID3D11PixelShader> m_pRenderOscilloscopePS;
    CComPtr<ID3D11Buffer> m_pOscilloscopeParamsBuf;
    CComPtr<ID3D11Buffer> m_pFrameTimesBuf;

    struct SOscilloscopeParams
    {
	    D3DXVECTOR4 PosPS;
	    float TimeScale;
	    int iFirstFrameNum;
    }m_OscilloscopeParams;
};
