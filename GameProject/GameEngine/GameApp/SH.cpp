//=================================================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#include "stdafx.h"
#include "SH.h"
#include "Utility.h"
#include "ShaderCompilation.h"

// == SH9Color ====================================================================================

SH9Color SH9Color::Zero()
{
    SH9Color sh;
    for(UINT_PTR i = 0; i < 9; ++i)
        sh.c[i] = XMVectorZero();

    return sh;
}

SH9Color ProjectCubemapToSH9Color(ID3D11DeviceContext* context, ID3D11ShaderResourceView* cubeMap)
{
    ID3D11Texture2DPtr srcTexture;
    cubeMap->GetResource(reinterpret_cast<ID3D11Resource**>(&srcTexture));

    D3D11_TEXTURE2D_DESC srcDesc;
    srcTexture->GetDesc(&srcDesc);

    ID3D11DevicePtr device;
    context->GetDevice(&device);

    ID3D11Texture2DPtr tempTexture;
    D3D11_TEXTURE2D_DESC tempDesc = srcDesc;
    tempDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    tempDesc.MipLevels = 1;
    tempDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
    tempDesc.Usage = D3D11_USAGE_DEFAULT;
    DXCall(device->CreateTexture2D(&tempDesc, NULL, &tempTexture));

    ID3D11UnorderedAccessViewPtr tempUAV;
    DXCall(device->CreateUnorderedAccessView(tempTexture, NULL, &tempUAV));

    ID3D11ShaderResourceViewPtr tempSRV;
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = srcDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = 0;
    srvDesc.Texture2DArray.MipLevels = srcDesc.MipLevels;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize = 6;
    DXCall(device->CreateShaderResourceView(srcTexture, &srvDesc, &tempSRV));

    static const UINT32 TGSize = 1024;
    static ID3D11ComputeShaderPtr decodeShader;
    if(decodeShader.GetInterfacePtr() == NULL)
    {
        CompileOptions opts;
        opts.Add("TGSize_", TGSize);
        decodeShader.Attach(CompileCSFromFile(device, L"SampleFramework11\\Shaders\\DecodeTextureCS.hlsl", 
                                                "DecodeTextureCS", "cs_5_0", opts.Defines()));
    }

    ID3D11ShaderResourceView* srvs[1] = { tempSRV };
    context->CSSetShaderResources(0, 1, srvs);

    ID3D11UnorderedAccessView* uavs[1] = { tempUAV };
    context->CSSetUnorderedAccessViews(0, 1, uavs, NULL);

    context->CSSetShader(decodeShader, NULL, 0);

    context->Dispatch(DispatchSize(TGSize, srcDesc.Width), srcDesc.Height, 6);

    float red[9];
    float green[9];
    float blue[9];

    DXCall(D3DX11SHProjectCubeMap(context, 3, tempTexture, red, green, blue));    

    SH9Color sh;
    for(UINT_PTR i = 0; i < 9; ++i)
        sh.c[i] = XMVectorSet(red[i], green[i], blue[i], 0.0f);

    return sh;
}

