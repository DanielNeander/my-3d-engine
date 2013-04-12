///////////////////////////////////////////////////////////////////////
//  GeometryBuffer.cpp
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
//  Function: GetMatchingDx10Type

DXGI_FORMAT GetMatchingDx10Type(const SVertexAttribDesc* pAttrib)
{
    DXGI_FORMAT eDx10Type = DXGI_FORMAT_UNKNOWN; // not found

    // parts of the SVertexAttribDesc struct we're interested in
    const EVertexAttribType eType = pAttrib->m_eDataType;
    const unsigned short uiNumElements = pAttrib->m_uiNumElements;

    if (uiNumElements > 0 && uiNumElements < 5)
    {
        // unsigned byte 
        if (eType == VERTEX_ATTRIB_TYPE_UNSIGNED_BYTE)
        {
            if (uiNumElements == 4)
                eDx10Type = DXGI_FORMAT_R8G8B8A8_UINT;
        }
        // unsigned byte (normalized)
        else if (eType == VERTEX_ATTRIB_TYPE_UNSIGNED_BYTE_NORMALIZED)
        {
            if (uiNumElements == 4)
                eDx10Type = DXGI_FORMAT_R8G8B8A8_UNORM;
        }
        // signed shorts
        else if (eType == VERTEX_ATTRIB_TYPE_SHORT)
        {
            if (uiNumElements == 2)
                eDx10Type = DXGI_FORMAT_R16G16_SINT;
            else if (uiNumElements == 4)
                eDx10Type = DXGI_FORMAT_R16G16B16A16_SINT;
        }
        // signed shorts (normalized)
        else if (eType == VERTEX_ATTRIB_TYPE_SHORT_NORMALIZED)
        {
            if (uiNumElements == 2)
                eDx10Type = DXGI_FORMAT_R16G16_SNORM;
            else if (uiNumElements == 4)
                eDx10Type = DXGI_FORMAT_R16G16B16A16_SNORM;
        }
        // unsigned shorts (normalized; there is no non-normalized USHORT2 or USHORT4)
        else if (eType == VERTEX_ATTRIB_TYPE_UNSIGNED_SHORT_NORMALIZED)
        {
            if (uiNumElements == 2)
                eDx10Type = DXGI_FORMAT_R16G16_UNORM;
            else if (uiNumElements == 4)
                eDx10Type = DXGI_FORMAT_R16G16B16A16_UNORM;
        }
        // floats
        else if (eType == VERTEX_ATTRIB_TYPE_FLOAT)
        {
            if (uiNumElements == 1)
                eDx10Type = DXGI_FORMAT_R32_FLOAT;
            else if (uiNumElements == 2)
                eDx10Type = DXGI_FORMAT_R32G32_FLOAT;
            else if (uiNumElements == 3)
                eDx10Type = DXGI_FORMAT_R32G32B32_FLOAT;
            else
                eDx10Type = DXGI_FORMAT_R32G32B32A32_FLOAT;
        }
    }

    return eDx10Type;
}


///////////////////////////////////////////////////////////////////////  
//  Function: GetMatchingDx10Semantic

bool GetMatchingDx10Semantic(const SVertexAttribDesc* pAttrib, D3D11_INPUT_ELEMENT_DESC& sElement)
{
    sElement.SemanticIndex = 0;
    switch (pAttrib->m_eSemantic)
    {
    case VERTEX_ATTRIB_SEMANTIC_POS:
        sElement.SemanticName = "POSITION";
        break;
    case VERTEX_ATTRIB_SEMANTIC_BLENDWEIGHT:
        sElement.SemanticName = "BLENDWEIGHT";
        break;
    case VERTEX_ATTRIB_SEMANTIC_NORMAL:
        sElement.SemanticName = "NORMAL";
        break;
    case VERTEX_ATTRIB_SEMANTIC_DIFFUSE:
        sElement.SemanticName = "COLOR";
        break;
    case VERTEX_ATTRIB_SEMANTIC_SPECULAR:
        sElement.SemanticName = "COLOR";
        sElement.SemanticIndex = 1;
        break;
    case VERTEX_ATTRIB_SEMANTIC_TESSFACTOR:
        sElement.SemanticName = "TESSFACTOR";
        break;
    case VERTEX_ATTRIB_SEMANTIC_PSIZE:
        sElement.SemanticName = "PSIZE";
        break;
    case VERTEX_ATTRIB_SEMANTIC_BLENDINDICES:
        sElement.SemanticName = "BLENDINDICES";
        break;
    case VERTEX_ATTRIB_SEMANTIC_TEXCOORD0:
    case VERTEX_ATTRIB_SEMANTIC_TEXCOORD1:
    case VERTEX_ATTRIB_SEMANTIC_TEXCOORD2:
    case VERTEX_ATTRIB_SEMANTIC_TEXCOORD3:
    case VERTEX_ATTRIB_SEMANTIC_TEXCOORD4:
    case VERTEX_ATTRIB_SEMANTIC_TEXCOORD5:
    case VERTEX_ATTRIB_SEMANTIC_TEXCOORD6:
    case VERTEX_ATTRIB_SEMANTIC_TEXCOORD7:
        sElement.SemanticName = "TEXCOORD";
        sElement.SemanticIndex = pAttrib->m_eSemantic - VERTEX_ATTRIB_SEMANTIC_TEXCOORD0;
        break;
    default:
        break;
    }

    return (sElement.SemanticName != NULL);
}


///////////////////////////////////////////////////////////////////////  
//  CGeometryBufferDirectX10::SetVertexFormat

bool CGeometryBufferDirectX10::SetVertexFormat(const SVertexAttribDesc* pAttribDesc, const CShaderTechnique* pTechnique, bool bDynamic)
{
    bool bSuccess = false;

    m_bDynamicVertexBuffer = bDynamic;

    unsigned int uiOffset = 0;
    CArray<D3D11_INPUT_ELEMENT_DESC> aElements;

    assert(pAttribDesc);
    const SVertexAttribDesc* pAttrib = pAttribDesc;
    while (pAttrib->m_eSemantic != VERTEX_ATTRIB_SEMANTIC_END)
    {
        assert(pAttrib->m_uiNumElements > 0);

        // setup element to match this attribute
        D3D11_INPUT_ELEMENT_DESC sElement;
        sElement.Format = GetMatchingDx10Type(pAttrib);
        if (sElement.Format != DXGI_FORMAT_UNKNOWN && GetMatchingDx10Semantic(pAttrib, sElement))
        {           
            // semantic/method
            if (sElement.SemanticName != NULL)
            {
                sElement.InputSlot = 0;
                sElement.AlignedByteOffset = uiOffset;
                sElement.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                sElement.InstanceDataStepRate = 0;

                aElements.push_back(sElement);
                bSuccess = true;
            }
            else
            {
                //CCore::SetError("CGeometryBufferDirectX10::SetFormat, cannot find matching DX10 semantic");
				assert(false);
                bSuccess = false;
                break;
            }
        }
        else
        {
            //CCore::SetError("CGeometryBufferDirectX10::SetFormat, vertex attribute not supported by DX10: (%s, # elements: %d, data type: %s)\n",
            //    pAttrib->SemanticName( ), pAttrib->m_uiNumElements, pAttrib->DataTypeName( ));
			assert(false);
            bSuccess = false;
            break;
        }

        uiOffset += WORD(pAttrib->SizeOfAttrib( ));
        ++pAttrib;
    }

    if (bSuccess)
    {
        assert(DX11::Device( ));
        assert(pTechnique);
        assert(pTechnique->IsValid( ));
        
        D3DX11_PASS_DESC sPassDesc;
        pTechnique->m_tShaderTechniquePolicy.GetDX10Technique( )->GetPassByIndex(0)->GetDesc(&sPassDesc);
        bSuccess = (DX11::Device( )->CreateInputLayout(&aElements[0], (UINT)aElements.size( ), sPassDesc.pIAInputSignature,
                                                        sPassDesc.IAInputSignatureSize, &m_pVertexLayout) == S_OK);
    }

    return bSuccess;
}
