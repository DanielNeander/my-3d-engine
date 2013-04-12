//----------------------------------------------------------------------------------
// File:  LightningSeed.cpp
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
#include "LightningSeed.h"
#include "GameApp/GameApp.h"
using namespace std;

namespace LightningDemo
{

LightningSeed::LightningSeed(ID3DX11Effect* effect, ID3DX11EffectTechnique* first_pass, ID3DX11EffectTechnique* subdivide, int pattern_mask, unsigned int subdivisions):
	m_device(0),
	m_effect(effect),

	m_tech_first_pass(first_pass),
	m_tech_subdivide(subdivide),

	m_constants_lightning_structure(effect,"LightningStructure"),

	m_pattern_mask(pattern_mask),
	m_subdivisions(subdivisions)

{
	HRESULT hr;

	m_effect->AddRef();

	DXCall(effect->GetDevice(&m_device));	
	m_context = D3D11Context();
	m_context->AddRef();
}
		

LightningSeed::~LightningSeed(void)
{
	m_effect->Release();
	m_device->Release();
	m_context->Release();
}
}