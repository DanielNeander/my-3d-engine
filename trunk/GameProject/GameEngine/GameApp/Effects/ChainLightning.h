//----------------------------------------------------------------------------------
// File:  ChainLightning.h
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

#pragma once
#include "GameApp/Utility.h"
#include "EffectVariable.h"
#include "Geometry.h"
#include "ColorRenderBuffer.h"
#include <vector>
#include <map>

#include "LightningSeed.h"

namespace LightningDemo
{

	namespace Effect = Utility::Effect;
	namespace Geometry = Utility::Geometry;


#pragma once

class ChainLightning : public LightningSeed
{
	friend class LightningRenderer;
public:

	
	struct ChainLightningProperties 
	{
		enum
		{
			MaxTargets = 8
		};
		D3DXVECTOR3	ChainSource;
		float Dummy0;

		D3DXVECTOR4 ChainTargetPositions[MaxTargets];
		
		int			NumTargets;
	};

	ChainLightningProperties Properties;

private:

	ChainLightning(ID3DX11Effect* effect, int pattern_mask, unsigned int subdivisions);

	virtual void SetChildConstants()
	{
		m_constants_chain_lightning = Properties;
	}

	virtual void RenderFirstPass()
	{
		m_constants_chain_lightning = Properties;
		
		m_context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
			
		ID3D11Buffer* zero = 0;
		UINT nought = 0;
		m_context->IASetVertexBuffers(0,1,&zero,&nought,&nought);
		m_context->IASetInputLayout(0);

		m_tech_first_pass->GetPassByIndex(0)->Apply(0, m_context);
		m_context->Draw(GetNumVertices(0),0);

		
		m_context->IASetInputLayout(m_subdivide_layout);


	}
	virtual unsigned int GetMaxNumVertices()
	{
		
		return  ChainLightningProperties::MaxTargets * GetNumBoltVertices(m_subdivisions);
	}

	virtual unsigned int GetNumVertices(unsigned int level)
	{
		
		return Properties.NumTargets * GetNumBoltVertices(level);
	}

	~ChainLightning();
	Effect::ConstantBuffer  m_constants_chain_lightning;
	ID3D11InputLayout* m_subdivide_layout;

};


}