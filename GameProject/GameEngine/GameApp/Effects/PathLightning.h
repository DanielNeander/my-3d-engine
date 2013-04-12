//----------------------------------------------------------------------------------
// File:  PathLightning.h
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

	struct LightningPathSegment
	{
	LightningPathSegment(const D3DXVECTOR3&	start, const D3DXVECTOR3& end, const D3DXVECTOR3& up):
		Start(start),
		End(end),
		Up(up)
	{
	}
	LightningPathSegment(float sx, float sy, float sz, float ex, float ey, float ez, float ux = 0, float uy = 0, float uz = 1):
		Start(sx,sy,sz),
		End(ex, ey, ez),
		Up(ux, uy, uz)
	{
	}

	operator SubdivideVertex () const
	{
		SubdivideVertex result;

		result.Start = this->Start;
		result.End = this->End;
		result.Up = this->Up;

		result.Level = 0;

		return result;
	}
	
	D3DXVECTOR3	Start;
	D3DXVECTOR3	End;
	D3DXVECTOR3	Up;
};

	class LightningRenderer;

	class PathLightning : public LightningSeed
	{
		friend class LightningRenderer;
	public:

		virtual void RenderFirstPass()
		{

			m_path_segments->BindToInputAssembler();
			m_context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
			m_context->IASetInputLayout(m_subdivide_layout);

			m_tech_first_pass->GetPassByIndex(0)->Apply(0, m_context);
			m_context->Draw(GetNumVertices(0),0);
		}

		virtual unsigned int GetNumVertices(unsigned int level)
		{
			return m_path_segments->NumVertices() * GetNumBoltVertices(level);
		}

		PathLightning(ID3DX11Effect* effect, const std::vector<LightningPathSegment>& segments, int pattern_mask, unsigned int subdivisions);
		~PathLightning();

	private:
		ID3D11InputLayout* m_subdivide_layout;
		Geometry::SimpleVertexBuffer<SubdivideVertex>* m_path_segments;
	};

}