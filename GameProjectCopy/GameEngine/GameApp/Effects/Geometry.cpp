//----------------------------------------------------------------------------------
// File: Geometry.cpp
// Author: Mathias Schott
// Email:  sdkfeedback@nvidia.com
// 
// Copyright (c) 2007 NVIDIA Corporation. All rights reserved.
//
// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
// *AS IS*  AND NVIDIA AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  NVIDIA OR ITS SUPPLIERS
// BE  LIABLE  FOR  ANY  SPECIAL,  INCIDENTAL,  INDIRECT,  OR  CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
//
//----------------------------------------------------------------------------------


#include "stdafx.h"
#include "Geometry.h"

namespace Utility
{
namespace Geometry
{

	ID3D11InputLayout* CreateInputLayout( ID3D11Device* device, const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout, ID3DX11EffectTechnique* technique )
	{
	HRESULT hr;

	ID3D11InputLayout* result = 0;
	
    D3DX11_PASS_DESC PassDesc;
    technique->GetPassByIndex( 0 )->GetDesc( &PassDesc );
    DXCall(device->CreateInputLayout( &layout[0], UINT(layout.size()), PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &result));
   
	return result;
}
	
} // Geometry
}// Utility

