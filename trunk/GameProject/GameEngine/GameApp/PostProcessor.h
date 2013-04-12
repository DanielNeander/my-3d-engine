//=================================================================================================
//
//  Light Indexed Deferred Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#pragma once


#include "GraphicsTypes.h"
#include "PostProcessorBase.h"
#include "DeviceStates.h"

#include "AppSettings.h"


class PostProcessor : public PostProcessorBase
{

public:

    struct Constants
    {
        float BloomThreshold;
        float BloomMagnitude;
        float BloomBlurSigma;
        float Tau;
        float TimeDelta;
        float KeyValue;
    };

    void Initialize(ID3D11Device* device);

    void SetConstants(const Constants& constants);
    void Render(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* input,
                ID3D11RenderTargetView* output);
    void AfterReset(UINT width, UINT height);

    void Downscale(ID3D11DeviceContext* deviceContext, ID3D11ShaderResourceView* input,
                    ID3D11RenderTargetView* output);

    const Constants& GetConstants() const { return constantBuffer.Data; }

protected:

    void CalcAvgLuminance(ID3D11ShaderResourceView* input);
    void Bloom(ID3D11ShaderResourceView* input, TempRenderTarget*& bloomTarget);
    void ToneMap(ID3D11ShaderResourceView* input, ID3D11ShaderResourceView* bloom,
                 ID3D11RenderTargetView* output);

    ID3D11PixelShaderPtr bloomThreshold;
    ID3D11PixelShaderPtr bloomBlurH;
    ID3D11PixelShaderPtr bloomBlurV;
    ID3D11PixelShaderPtr luminanceMap;
    ID3D11PixelShaderPtr composite;
    ID3D11PixelShaderPtr scale;
    ID3D11PixelShaderPtr adaptLuminance;
    ID3D11PixelShaderPtr reflections;

    RenderTarget2D adaptedLuminance[2];
    RenderTarget2D exposureMap;

    UINT_PTR currLumTarget;

    ConstantBuffer<Constants> constantBuffer;
};