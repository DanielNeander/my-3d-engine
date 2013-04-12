///////////////////////////////////////////////////////////////////////
//  Shaders.cpp
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
#include "Utility.h"
#if defined(WIN32)
#include "direct.h"
#endif
using namespace SpeedTree;


///////////////////////////////////////////////////////////////////////  
//  Function StringDup

const char* StringDup(const CString& strSource)
{
    char* pDup = st_new_array<char>(strSource.size( ) + 1, "StringDup::char");
    strcpy(pDup, strSource.c_str( ));

    return pDup;
}


///////////////////////////////////////////////////////////////////////  
//  Function SetupShaderMacros

D3D10_SHADER_MACRO* SetupShaderMacros(const CArray<CString>& aDefines)
{
    // CShaderLoaderRI::Load() is expected to check for this
    assert(aDefines.size( ) % 2 == 0);

    // setup shader #defines
    D3D10_SHADER_MACRO* pShaderMacros = st_new_array<D3D10_SHADER_MACRO>(aDefines.size( ) / 2 + 1, "D3DXMACRO");
    int i = 0;
    for ( ; i < int(aDefines.size( )); i += 2)
    {
        pShaderMacros[i / 2].Name = aDefines[i].c_str( );
        pShaderMacros[i / 2].Definition = aDefines[i + 1].c_str( );
    }

    // terminate the shader array
    pShaderMacros[i / 2].Name = NULL;
    pShaderMacros[i / 2].Definition = NULL;

    return pShaderMacros;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderDirectX10::Load

bool CShaderLoaderDirectX10::Load(const char* pFilename, const CArray<CString>& aDefines)
{
    bool bSuccess = false;

    if (pFilename && GetFileAttributesA(pFilename))
    {
        ID3DBlob* pBuffer = NULL;
		ID3DBlob* pErrorBlob = NULL;

        // D3D11_SHADER_NO_PRESHADER is being used a direct result of a bug in the March 2009 release of the
        // DirectX SDK.  Functions like saturate(), min(), and max() used it certain places cause an assertion
        // to fail within the preshader.  Set dwShaderFlags to zero if not using this SDK.
        UINT dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS; 
        //dwShaderFlags |= D3D11_SHADER_NO_PRESHADER;
        //dwShaderFlags |= D3D11_SHADER_DEBUG;

        D3D10_SHADER_MACRO* pShaderMacros = SetupShaderMacros(aDefines);

        // change the current path to the path where the shader file is located
        char szCurrentDir[_MAX_PATH];
        _getcwd(szCurrentDir, _MAX_PATH);
        _chdir(CString(pFilename).Path( ).c_str( ));

        assert(DX11::Device( ));		
        if (FAILED(D3DX11CompileFromFile(CString(pFilename).NoPath( ).c_str( ), pShaderMacros, NULL, NULL,
			"fx_4_0", dwShaderFlags, NULL, NULL,  &pBuffer, &pErrorBlob, NULL)))
        {

			 D3DX11CreateEffectFromMemory(pBuffer->GetBufferPointer(), pBuffer->GetBufferSize(), 0, DX11::Device(), &m_pEffect);

            if (pBuffer)
            {
                LPVOID pCompileErrors = pBuffer->GetBufferPointer( );
                m_strError = (const char*) pCompileErrors;
            }

            if (m_pEffect)
            {
                m_pEffect->Release( );
                m_pEffect = NULL;
            }
        }
        else
            bSuccess = true;

        // restore the path to its previous value
        _chdir(szCurrentDir);

        st_delete_array<D3D10_SHADER_MACRO>(pShaderMacros, "D3DXMACRO");
    }
    else
        //CCore::SetError("CShaderLoaderDirectX10::Load, filename was either NULL or does not exist");
		assert(false);

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderDirectX10::GetShader

bool CShaderLoaderDirectX10::GetShader(const char* pName, CShaderTechnique& cTechnique) const
{
    bool bSuccess = false;

    if (m_pEffect)
    {
        cTechnique.m_tShaderTechniquePolicy.m_pTechnique = m_pEffect->GetTechniqueByName(pName);

        if (cTechnique.m_tShaderTechniquePolicy.m_pTechnique->IsValid( ))
            bSuccess = true;
        else
            //CCore::SetError("Failed to find shader [%s]\n", pName);
			assert(false);
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderDirectX10::GetConstant

bool CShaderLoaderDirectX10::GetConstant(const char* pName, CShaderConstant& cConstant) const
{
    bool bSuccess = false;

    if (m_pEffect)
    {
        cConstant.m_tShaderConstantPolicy.m_pParameter = m_pEffect->GetVariableByName(pName);

        if (cConstant.m_tShaderConstantPolicy.m_pParameter->IsValid( ))
            bSuccess = true;
        else
            //CCore::SetError("Failed to find shader constant [%s]\n", pName);
			assert(false);
    }

    return bSuccess;
}


///////////////////////////////////////////////////////////////////////  
//  CShaderLoaderDirectX10::Release

void CShaderLoaderDirectX10::Release(void)
{
    if (m_pEffect)
    {
        m_pEffect->Release( );
        m_pEffect = NULL;
    }
}
