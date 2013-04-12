#ifndef __M2EFFECTRENDERER_H__
#define __M2EFFECTRENDERER_H__

#include "SceneNode.h"

struct RibbonVertex {
	noVec3 pos;
	noVec2 uv;
	noVec4 color;
};

struct ParticleVertex {
	noVec3 pos;
	noVec2 uv;
	noVec4 color;
};

class M2EffectRender : public SceneNode
{

public:
	M2EffectRender(class M2Loader* pM2);
	~M2EffectRender();

	struct RenderData {

		RenderData();
		~RenderData();

		ID3DX11EffectShaderResourceVariable* gptxDiffuse;
		ID3D11InputLayout*                  gpVertexLayout;
		std::vector<ID3D11Buffer*>	                     ParticleBuffers;
		std::map<TextureID, ID3D11ShaderResourceView*>           ParticleTextureSRVMaps;
		ID3DX11Effect*                       gpEffect10;
		ID3DX11EffectTechnique*              gpRenderParticles;
		ID3DX11EffectMatrixVariable*         gpmWorldViewProjection;
		ID3DX11EffectMatrixVariable*         gpmWorld;
		ID3DX11EffectMatrixVariable*         gpmInvViewProj;
		ID3DX11EffectScalarVariable*         gpfTime;
		ID3DX11EffectVectorVariable*         gpvEyePt;
		ID3DX11EffectVectorVariable*         gpvRight;
		ID3DX11EffectVectorVariable*         gpvUp;
		ID3DX11EffectVectorVariable*         gpvForward;
		std::vector<int>					gNumActiveParticles;
		

		std::map<TextureID, std::string>	ParticleTexNames;
	};

	void UpdateEffects();
	void Render();

	void CreateResources();
	
protected:	
	void CreateParticles();
	void CreateRibbons();
	
	void UpdateParicles();
	void UpdateRibbons();

	void RenderParticles();
	void RenderRibbons();
	void CopyRibbonsToVertexBuffer(int index, class M2RibbonEmitter& Ribbon, RibbonVertex* pVB );		
	void CopyParticlesToVertexBuffer(uint32 index, class M2ParticleSystem* ps, ParticleVertex* pVB, D3DXVECTOR3 vEye, D3DXVECTOR3 _vRight, D3DXVECTOR3 _vUp );
	void LoadShaders();
	RenderData RibbonData_;
	RenderData ParticleData_; 

	ID3DX11Effect* Effect_;
	
	 M2Loader* Owner_;
	bool ExistRibbon_, ExistParticle_;
};



#endif