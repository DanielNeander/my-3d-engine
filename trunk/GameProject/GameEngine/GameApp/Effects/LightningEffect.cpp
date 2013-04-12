#include "stdafx.h"
#include "LightningEffect.h"
#include "GameApp/GameApp.h"
#include "GameApp/BaseCamera.h"

using namespace LightningDemo;

LightningEffect::LightningEffect(ID3D11Device* device, ID3D11DeviceContext* context, DXGI_SAMPLE_DESC back_buffer_sample_desc )
	:Renderer_(device, context, back_buffer_sample_desc)
{
	Jitter_ = 1.0f;
	Replicates_ = 1;
	CurrPathLighting_ = 0;
}

LightningEffect::~LightningEffect()
{

}

void LightningEffect::Test(const float* start, const float* end, int pattern_mask)
{

	ApplySettings();

	D3DXVECTOR3 vStart(start), vEnd(end);

	float step = 2.f;
	float xstep = abs(vStart.x - vEnd.x) / step;
	float zstep = abs(vStart.z - vEnd.z) / step;

	std::vector<D3DXVECTOR3> seeds;
	std::vector<LightningPathSegment> segments;

	seeds.resize(step);
	for(int i = 0 ; i < step; ++i)
	{
		seeds[i] = D3DXVECTOR3(vStart.x + xstep * i, vStart.y, vStart.z + zstep * i);
	}

	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

	for(int j = 0; j < Replicates_; ++j)
	{
		if( seeds.size() > 1)
		{
			for(size_t i = 0; i <seeds.size()-1; ++i)
			{
				segments.push_back(					
					LightningPathSegment 
					(
					seeds[i] ,
					seeds[i+1],
					up
					)
					);
			}

			segments.push_back(				
				LightningPathSegment 
				(
				seeds[seeds.size() - 1] ,
				seeds[0],
				up
				)
				);	

		}
	}
	for (size_t i=0; i < segments.size(); ++i)  {
		segments.at(i).Start += Jitter_ * D3DXVECTOR3(Utility::Random(-1,1),Utility::Random(-1,1),Utility::Random(-1,1));
		segments.at(i).End += Jitter_ * D3DXVECTOR3(Utility::Random(-1,1),Utility::Random(-1,1),Utility::Random(-1,1));	
	}
	
	CurrPathLighting_ = Renderer_.CreatePathLightning(segments, pattern_mask, 5);
	CurrPathLighting_->Structure = Fence_structure_;
}

void LightningEffect::AddPathSegment( const LightningPathSegment& seg )
{
	Seeds_.push_back(seg);
}


PathLightning* LightningEffect::FirePathLightning( const D3DXVECTOR3& start, const D3DXVECTOR3& end, int pattern_mask)
{
			
	if (Seeds_.empty())
	{
		assert(false);
		return NULL;
	}
	
			
	PathLightning* newInstance = Renderer_.CreatePathLightning(Seeds_, pattern_mask, 5);
	assert(newInstance);
	Seeds_.clear();
	return newInstance;	
}

ChainLightning*	LightningEffect::CreateChainLightning()
{
	return Renderer_.CreateChainLightning(0x0C, 5);
}

void LightningEffect::CreateLightningStructure()
{
	LightningStructure inter_coil_structure;
	{

		inter_coil_structure.ZigZagFraction = D3DXVECTOR2(0.45f, 0.55f);

		inter_coil_structure.ZigZagDeviationRight = D3DXVECTOR2(-5.0f,5.0f);		
		inter_coil_structure.ZigZagDeviationUp = D3DXVECTOR2(-5.0f,5.0f);

		inter_coil_structure.ZigZagDeviationDecay = 0.5f;


		inter_coil_structure.ForkFraction = D3DXVECTOR2(0.45f, 0.55f);

		inter_coil_structure.ForkZigZagDeviationRight = D3DXVECTOR2(-1.0f,1.0f);
		inter_coil_structure.ForkZigZagDeviationUp = D3DXVECTOR2(-1.0f,1.0f);
		inter_coil_structure.ForkZigZagDeviationDecay = 0.5f;


		inter_coil_structure.ForkDeviationRight = D3DXVECTOR2(-1.0f,1.0f);
		inter_coil_structure.ForkDeviationUp = D3DXVECTOR2(-1.0f,1.0f);
		inter_coil_structure.ForkDeviationForward = D3DXVECTOR2(0.0f,1.0f);
		inter_coil_structure.ForkDeviationDecay = 0.5f;

		inter_coil_structure.ForkLength = D3DXVECTOR2(1.0f,2.0f);
		inter_coil_structure.ForkLengthDecay = 0.01f;
	}

	LightningStructure fence_structure;
	{

		fence_structure.ZigZagFraction = D3DXVECTOR2(0.45f, 0.55f);

		fence_structure.ZigZagDeviationRight = D3DXVECTOR2(-1.0f,1.0f);		
		fence_structure.ZigZagDeviationUp = D3DXVECTOR2(-1.0f,1.0f);

		fence_structure.ZigZagDeviationDecay = 0.5f;


		fence_structure.ForkFraction = D3DXVECTOR2(0.45f, 0.55f);

		fence_structure.ForkZigZagDeviationRight = D3DXVECTOR2(-1.0f,1.0f);
		fence_structure.ForkZigZagDeviationUp = D3DXVECTOR2(-1.0f,1.0f);
		fence_structure.ForkZigZagDeviationDecay = 0.5f;


		fence_structure.ForkDeviationRight = D3DXVECTOR2(-1.0f,1.0f);
		fence_structure.ForkDeviationUp = D3DXVECTOR2(-1.0f,1.0f);
		fence_structure.ForkDeviationForward = D3DXVECTOR2(-1.0f,1.0f);
		fence_structure.ForkDeviationDecay = 0.5f;

		fence_structure.ForkLength = D3DXVECTOR2(1.0f,2.0f);
		fence_structure.ForkLengthDecay = 0.01f;
	}
	LightningStructure coil_helix_structure;
	{

		coil_helix_structure.ZigZagFraction = D3DXVECTOR2(0.45f, 0.55f);

		coil_helix_structure.ZigZagDeviationRight = D3DXVECTOR2(-5.0f,5.0f);		
		coil_helix_structure.ZigZagDeviationUp = D3DXVECTOR2(-5.0f,5.0f);

		coil_helix_structure.ZigZagDeviationDecay = 0.5f;


		coil_helix_structure.ForkFraction = D3DXVECTOR2(0.45f, 0.55f);

		coil_helix_structure.ForkZigZagDeviationRight = D3DXVECTOR2(-1.0f,1.0f);
		coil_helix_structure.ForkZigZagDeviationUp = D3DXVECTOR2(-1.0f,1.0f);
		coil_helix_structure.ForkZigZagDeviationDecay = 0.5f;


		coil_helix_structure.ForkDeviationRight = D3DXVECTOR2(-1.0f,1.0f);
		coil_helix_structure.ForkDeviationUp = D3DXVECTOR2(-1.0f,1.0f);
		coil_helix_structure.ForkDeviationForward = D3DXVECTOR2(0.0f,1.0f);
		coil_helix_structure.ForkDeviationDecay = 0.5f;

		coil_helix_structure.ForkLength = D3DXVECTOR2(1.0f,2.0f);
		coil_helix_structure.ForkLengthDecay = 0.01f;
	}

	LightningStructure chain_structure;
	{

		chain_structure.ZigZagFraction = D3DXVECTOR2(0.45f, 0.55f);

		chain_structure.ZigZagDeviationRight = D3DXVECTOR2(-5.0f,5.0f);		
		chain_structure.ZigZagDeviationUp = D3DXVECTOR2(-5.0f,5.0f);

		chain_structure.ZigZagDeviationDecay = 0.5f;


		chain_structure.ForkFraction = D3DXVECTOR2(0.45f, 0.55f);

		chain_structure.ForkZigZagDeviationRight = D3DXVECTOR2(-1.0f,1.0f);
		chain_structure.ForkZigZagDeviationUp = D3DXVECTOR2(-1.0f,1.0f);
		chain_structure.ForkZigZagDeviationDecay = 0.5f;


		chain_structure.ForkDeviationRight = D3DXVECTOR2(-1.0f,1.0f);
		chain_structure.ForkDeviationUp = D3DXVECTOR2(-1.0f,1.0f);
		chain_structure.ForkDeviationForward = D3DXVECTOR2(0.0f,1.0f);
		chain_structure.ForkDeviationDecay = 0.5f;

		chain_structure.ForkLength = D3DXVECTOR2(1.0f,2.0f);
		chain_structure.ForkLengthDecay = 0.01f;
	}

	{
		m_blue_beam.BoltWidth = D3DXVECTOR2(0.125f,0.5f);
		m_blue_beam.ColorInside = D3DXVECTOR3(1,1,1);
		m_blue_beam.ColorOutside = D3DXVECTOR3(0,0,1);
		m_blue_beam.ColorFallOffExponent = 2.0f;
	}

	{
		m_blue_cyan_beam.BoltWidth = D3DXVECTOR2(0.25f,0.5f);
		m_blue_cyan_beam.ColorInside = D3DXVECTOR3(0,1,1);
		m_blue_cyan_beam.ColorOutside = D3DXVECTOR3(0,0,1);
		m_blue_cyan_beam.ColorFallOffExponent = 5.0f;
	}

	{
		m_red_beam.BoltWidth = D3DXVECTOR2(0.5f,0.5f);
		m_red_beam.ColorInside =  D3DXVECTOR3(1,1,1);
		m_red_beam.ColorOutside = D3DXVECTOR3(1,0,0);
		m_red_beam.ColorFallOffExponent = 5.0f;
	}
	Coil_helix_structure_ = coil_helix_structure;
	Chain_structure_ = chain_structure;
	Fence_structure_ = fence_structure;
	Inter_coil_structure_ = inter_coil_structure;
}

void LightningEffect::Tick( float time, float delta_time )
{
	BaseCamera* pCam = GetApp()->ActiveCam_;
	D3DXMATRIX mProj;
	D3DXMATRIX mView;
	D3DXMATRIX world;
	D3DXMatrixIdentity(&world);

	const unsigned int size16 = sizeof(float) * 16;		 
	memcpy(&mView, pCam->getViewMatrix(), size16);
	memcpy(&mProj, pCam->getProjectionMatrix(), size16);

	Renderer_.SetMatrices(world, mView, mProj);
	Renderer_.SetTime(time);
}

void LightningEffect::ResizeSwapChain( unsigned width, unsigned height, ID3D11RenderTargetView* render_target_view, ID3D11DepthStencilView* depth_stencil_view )
{
	Renderer_.OnRenderTargetResize(width, height, render_target_view, depth_stencil_view);
}

void LightningEffect::Render()
{
	/*Renderer_.Begin();
	if (CurrPathLighting_)
		Renderer_.Render(CurrPathLighting_, m_red_beam, 1.0f, Settings.AnimationSpeed, Settings.Lines);

	Renderer_.End(Settings.Glow, Settings.BlurSigma);*/
}

void LightningEffect::ApplySettings()
{
	//Settings
	LightningAppearance beam_parameters;
	beam_parameters.BoltWidth = D3DXVECTOR2( 0.5f, 0.5f );
	beam_parameters.ColorInside =  D3DXVECTOR3(1,1,1);
	beam_parameters.ColorOutside = D3DXVECTOR3(1,1,0);
	beam_parameters.ColorFallOffExponent = 5.f;
	Settings.BlurSigma = D3DXVECTOR3(0.5f, 0.8f, 0.9f);
	Settings.Beam = beam_parameters;
}
