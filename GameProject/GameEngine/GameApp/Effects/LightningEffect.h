#pragma once 

#include "LightningRenderer.h"

using namespace LightningDemo;

struct LightningSetting
{
	LightningSetting()
		:Glow(true),
		Lines(false),
		AnimationSpeed(15)
	{	
	}

	bool	Lines;

	float   AnimationSpeed;
	bool	Glow;
	D3DXVECTOR3	BlurSigma;

	LightningAppearance Beam;
};

class LightningEffect : public Singleton<LightningEffect>
{
public:
	LightningEffect(ID3D11Device* device, ID3D11DeviceContext* context, DXGI_SAMPLE_DESC back_buffer_sample_desc );
	~LightningEffect();

	void ApplySettings();

	void AddPathSegment(const LightningPathSegment& seg);

	void CreateLightningStructure();

	void Render();

	void ResizeSwapChain(unsigned width, unsigned height, ID3D11RenderTargetView* render_target_view, ID3D11DepthStencilView* depth_stencil_view);
	void Tick(float tiem, float delta_time);
	
	PathLightning* FirePathLightning( const D3DXVECTOR3& start, const D3DXVECTOR3& end, int pattern_mask);
	ChainLightning* CreateChainLightning();
	bool GenerateSeeds(const D3DXVECTOR3& start, const D3DXVECTOR3& end, int pattern_mask);	
	void Test(const float* start, const float* end, int pattern_mask);
	std::vector<LightningPathSegment> Seeds_;


	float Jitter_;
	int Replicates_;

	LightningRenderer Renderer_;
	
	LightningSetting Settings;
			
	LightningAppearance m_red_beam;
	LightningAppearance m_blue_beam;
	LightningAppearance m_blue_cyan_beam;

	PathLightning*	CurrPathLighting_;

	LightningStructure Inter_coil_structure_;
	LightningStructure Fence_structure_;
	LightningStructure Chain_structure_;
	LightningStructure Coil_helix_structure_;
};

