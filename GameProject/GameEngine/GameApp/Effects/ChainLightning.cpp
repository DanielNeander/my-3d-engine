//----------------------------------------------------------------------------------
// File:  ChainLightning.cpp
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
#include "ChainLightning.h"
using namespace std;

namespace LightningDemo
{
ChainLightning::ChainLightning(ID3DX11Effect* effect,  int pattern_mask, unsigned int subdivisions):
	LightningSeed
	(
		effect,
		effect->GetTechniqueByName("ChainLightning"),
		effect->GetTechniqueByName("Subdivide"),
		pattern_mask,
		subdivisions
	),
	m_constants_chain_lightning(effect,"LightningChain")	

{
	m_subdivide_layout = Geometry::CreateInputLayout(m_device, SubdivideVertex::GetLayout(),m_tech_subdivide);


}


ChainLightning::~ChainLightning()
{
	m_subdivide_layout->Release();
}
}