#pragma once 


class VolLightRenderer 
{
public:
	bool Init(ID3D11Device* dev, ID3D11DeviceContext* context);
	
	void LoadShaders();
	void CreateRenderTargets();
	void DestoryRenderTargets();

	void CreateDepthBufferTexture(UINT width, UINT height);
	void DeleteDepthBufferTexture();

	void Reset();
		
	void LoadVolumLightShader();
	void DrawFullScreenQuad11( ID3D11DeviceContext* pd3dDeviceContext );
	void DrawFullScreenQuad11( ID3D11DeviceContext* pd3dDeviceContext, unsigned width, unsigned height );
	void DrawFullScreenQuad10( ID3D10Device* pd3dDevice, unsigned width, unsigned height );
	void DrawFullScreenQuad10( ID3D10Device* pd3dDevice );
	void Render(ShadowMapCamera* shadowcam, BaseCamera* viewCam);
	void ReleaseScaledTextures();
	void CreateScaledTextures(uint width, uint height);

	ID3D11ShaderResourceView* GetHDRSrcSRV() const { return HDRTexScaled->SRView; }
private:
	
	class PostProcessor* PostProcessor_;	
	struct DepthStencilBuffer* Shadowmap_;	
	struct RenderTarget2D**	ShadowMapScaled_;
	RenderTarget2D* ShadowmapWorld_;
	RenderTarget2D*	ShadowMapScaledOpt_;
	RenderTarget2D** ShadowMapHole;
	RenderTarget2D* HDRTex;
	RenderTarget2D* HDRTexScaled, *HDRTexScaled2, *HDRTexScaled3;
	RenderTarget2D* EdgeTexFS;


	DepthStencilBuffer*	DepthBufer;
	RenderTarget2D*	DepthBuferWS;

	struct D3D11Texture2D*	NoiseTex_;
	ID3D11ShaderResourceViewPtr		NoiseTexSRV;

	int	LightSize;
	int	LightNear;
	int LightFar;
	int	ShadowMapSize;
	int	ShadowMips;
	int	ShadowHoleMip;
	float ScaleRatio;
	float SamplingRate;

	ID3D11Device*		Device;
	ID3D11DeviceContext*	Context;
	ID3D11InputLayout*	SceneVertexLayout;

	ID3DX11Effect*	VolLitEffect;
	ID3DX11EffectShaderResourceVariable* g_pS0;
	ID3DX11EffectShaderResourceVariable* g_pS1;
	ID3DX11EffectShaderResourceVariable* g_pS2;
	ID3DX11EffectVectorVariable* g_pavSampleOffsetsHorizontal;
	ID3DX11EffectVectorVariable* g_pavSampleOffsetsVertical;
	ID3DX11EffectVectorVariable* g_pavSampleWeights;
	ID3DX11EffectScalarVariable* UseAngleOptimizationESV;


	class ShadowMapCamera* LightCam;	
	class U2Light*	Light_;

	bool	DrawVolumeLight;
	bool	DrawBloom;
	bool	DrawScaled;
	bool	UseAngleOptimization;
	bool	UseZOptimization;
	bool	UsePostProcessing;
		
};