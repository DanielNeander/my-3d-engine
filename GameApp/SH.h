//=================================================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#pragma once

#include "GraphicsTypes.h"



Float4Align struct SH9Color
{
    XMVECTOR c[9];

    static SH9Color Zero();
};

SH9Color ProjectCubemapToSH9Color(ID3D11DeviceContext* context, ID3D11ShaderResourceView* cubeMap);

