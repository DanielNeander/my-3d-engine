#pragma once 

struct RainVertex
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 seed;
	D3DXVECTOR3 speed;
	float random;
	unsigned char  Type;
};

struct WindValue
{
	D3DXVECTOR3 windAmount;
	int time;
	WindValue(D3DXVECTOR3 wA, int t){windAmount = wA; time=t;};
};
std::vector<WindValue> WindAnimation;

class Rain 
{
public:
	Rain();
	void Reset();
	void LoadShaders();

	void Render();

	bool Init();

private:
	ID3DX11EffectTechnique*  g_pTechniqueAdvanceRain            ;
	ID3DX11EffectTechnique*  g_pTechniqueDrawRain               ;
	ID3DX11EffectTechnique*  g_pTechniqueDrawRainCheap          ;
	ID3DX11EffectTechnique*  g_pTechniqueRenderScene            ;
	ID3DX11EffectTechnique*  g_pTechniqueRenderSky              ;
	ID3DX11EffectTechnique*  g_pTechniqueRenderArrow            ;

	ID3D11InputLayout*      g_pVertexLayoutRainVertex         ; 
	ID3D11InputLayout*      g_pVertexLayoutArrow              ; // Vertex Layout for arrow
	ID3D11InputLayout*      g_pVertexLayoutScene              ;
	ID3D11InputLayout*      g_pVertexLayoutSky                ;
	ID3D11Buffer*           g_pParticleStart                  ;
	ID3D11Buffer*           g_pParticleStreamTo               ;
	ID3D11Buffer*           g_pParticleDrawFrom               ;

private:
	ID3DX11EffectMatrixVariable* g_pWorldShaderVariable               ;
	ID3DX11EffectMatrixVariable* g_pWorldViewProjShaderVariable       ;
	ID3DX11EffectMatrixVariable* g_pInvViewShaderVariable             ;
	ID3DX11EffectMatrixVariable* g_pWorldViewShaderVariable           ; 
	ID3DX11EffectMatrixVariable* g_pProjectionShaderVariable          ;
	ID3DX11EffectMatrixVariable* g_pInverseProjectionShaderVariable   ;
	ID3DX11EffectVectorVariable* g_pEyePosShaderVariable              ;
	ID3DX11EffectVectorVariable* g_pLightPosShaderVariable            ;
	ID3DX11EffectVectorVariable* g_pTotalVelShaderVariable            ;
	ID3DX11EffectVectorVariable* g_pDiffuseShaderVariable             ;
	ID3DX11EffectVectorVariable* g_LightPosWithViewTransformationShaderVariable;
	ID3DX11EffectVectorVariable* g_LightPosWithViewTransformation2ShaderVariable;
	ID3DX11EffectVectorVariable* g_VecPointLightEyeShaderVariable     ;
	ID3DX11EffectVectorVariable* g_VecPointLightEye2ShaderVariable     ;
	ID3DX11EffectScalarVariable* g_pRenderBGShaderVariable            ;
	ID3DX11EffectScalarVariable* g_pMoveParticlesShaderVariable       ;
	ID3DX11EffectScalarVariable* g_pDirLightIntensityShaderVariable   ;
	ID3DX11EffectScalarVariable* g_pPointLightIntensityShaderVariable ;
	ID3DX11EffectScalarVariable* g_pResponseDirLightShaderVariable    ;
	ID3DX11EffectScalarVariable* g_pResponsePointLightShaderVariable  ;
	ID3DX11EffectScalarVariable* g_pSpriteSizeShaderVariable          ;
	ID3DX11EffectVectorVariable* g_pFogThicknessShaderVariable        ;
	ID3DX11EffectScalarVariable* g_ScreenWidthShaderVariable          ;
	ID3DX11EffectScalarVariable* g_ScreenHeightShaderVariable         ;
	ID3DX11EffectScalarVariable* g_ScreenWidthMultiplierShaderVariable;
	ID3DX11EffectScalarVariable* g_ScreenHeightMultiplierShaderVariable;
	ID3DX11EffectScalarVariable* g_TSVPointLightShaderVariable        ;
	ID3DX11EffectScalarVariable* g_DSVPointLightShaderVariable        ;
	ID3DX11EffectScalarVariable* g_DSVPointLight2ShaderVariable       ;
	ID3DX11EffectScalarVariable* g_DSVPointLight3ShaderVariable       ;
	ID3DX11EffectScalarVariable* g_deShaderVariable                   ;
	ID3DX11EffectScalarVariable* g_UseSpotLightShaderVariable         ;
	ID3DX11EffectScalarVariable* g_CosSpotLightShaderVariable         ;
	ID3DX11EffectScalarVariable* g_KdShaderVariable                   ;
	ID3DX11EffectScalarVariable* g_KsPointShaderVariable              ;
	ID3DX11EffectScalarVariable* g_KsDirShaderVariable                ;
	ID3DX11EffectScalarVariable* g_SpecPowerShaderVariable            ;
	ID3DX11EffectScalarVariable* g_FPSShaderVariable                  ;
	ID3DX11EffectScalarVariable* g_timeCycleShaderVariable            ;
	ID3DX11EffectScalarVariable* g_rainSplashesXDisplaceShaderVariable;
	ID3DX11EffectScalarVariable* g_rainSplashesYDisplaceShaderVariable;
};