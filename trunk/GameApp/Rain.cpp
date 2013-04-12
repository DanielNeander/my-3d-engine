#include "stdafx.h"
#include "Rain.h"


const D3DXVECTOR3 g_constFogVector1 = D3DXVECTOR3(0.03f,0.03f,0.03f);
const float       g_constDirLightIntensity1 = 0.18f;
const float       g_constResponseDirLight1 = 1.0f;
const float       g_constPointLightIntensity1 = 0.5f;
const float       g_constCosSpotLight1 = 0.3f;
const float       g_constResponsePointLight1 = 2.0f;
const float       g_constDrawFraction1 = 0.7f;
const float       g_constWindAmount1 = 1.0;
const D3DXVECTOR3 g_constVecEye1 = D3DXVECTOR3( 15.5f, 5.0f, 0.0f );
const D3DXVECTOR3 g_constAtVec1  = D3DXVECTOR3( 0.0f, 3.0f, 0.0f );
const D3DXVECTOR3 g_directionalLightVector1 = D3DXVECTOR3(0.551748, 0.731354, 0.400869);

const D3DXVECTOR3 g_constFogVector2 = D3DXVECTOR3(0.02f,0.02f,0.02f);
const float       g_constDirLightIntensity2 = 0.50f;
const float       g_constResponseDirLight2 = 0.68f;
const float       g_constPointLightIntensity2 = 0.4f;
const float       g_constCosSpotLight2 = 0.54f;
const float       g_constResponsePointLight2 = 2.0f;
const float       g_constDrawFraction2 = 0.6f;
const float       g_constWindAmount2 = 0.48;
const D3DXVECTOR3 g_constVecEye2 = D3DXVECTOR3( -4.0f, 18.0f, -5.0f );
const D3DXVECTOR3 g_constAtVec2  = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
const D3DXVECTOR3 g_directionalLightVector2 = D3DXVECTOR3(0.470105, 0.766044, 0.43838);

Rain::Rain()
{
	g_pTechniqueAdvanceRain            = NULL;
	g_pTechniqueDrawRain               = NULL;
	g_pTechniqueDrawRainCheap          = NULL;
	g_pTechniqueRenderScene            = NULL;
	g_pTechniqueRenderSky              = NULL;
	g_pTechniqueRenderArrow            = NULL;

	g_pVertexLayoutRainVertex          = NULL; 
	g_pVertexLayoutArrow               = NULL;
	g_pVertexLayoutScene               = NULL;
	g_pVertexLayoutSky                 = NULL;
	g_pParticleStart                   = NULL;
	g_pParticleStreamTo                = NULL;
	g_pParticleDrawFrom                = NULL;

	g_pWorldShaderVariable                = NULL;
	g_pWorldViewProjShaderVariable        = NULL;
	g_pInvViewShaderVariable              = NULL;
	g_pWorldViewShaderVariable            = NULL; 
	g_pProjectionShaderVariable           = NULL;
	g_pInverseProjectionShaderVariable    = NULL;
	g_pEyePosShaderVariable               = NULL;
	g_pLightPosShaderVariable             = NULL;
	g_pTotalVelShaderVariable             = NULL;
	g_pDiffuseShaderVariable              = NULL;
	g_LightPosWithViewTransformationShaderVariable = NULL;
	g_LightPosWithViewTransformation2ShaderVariable = NULL;
	g_VecPointLightEyeShaderVariable      = NULL;
	g_VecPointLightEye2ShaderVariable      = NULL;
	g_pRenderBGShaderVariable             = NULL;
	g_pMoveParticlesShaderVariable        = NULL;
	g_pDirLightIntensityShaderVariable    = NULL;
	g_pPointLightIntensityShaderVariable  = NULL;
	g_pResponseDirLightShaderVariable     = NULL;
	g_pResponsePointLightShaderVariable   = NULL;
	g_pSpriteSizeShaderVariable           = NULL;
	g_pFogThicknessShaderVariable         = NULL;
	g_ScreenWidthShaderVariable           = NULL;
	g_ScreenHeightShaderVariable          = NULL;
	g_ScreenWidthMultiplierShaderVariable = NULL;
	g_ScreenHeightMultiplierShaderVariable= NULL;
	g_TSVPointLightShaderVariable         = NULL;
	g_DSVPointLightShaderVariable         = NULL;
	g_DSVPointLight2ShaderVariable        = NULL;
	g_DSVPointLight3ShaderVariable        = NULL;
	g_deShaderVariable                    = NULL;
	g_UseSpotLightShaderVariable          = NULL;
	g_CosSpotLightShaderVariable          = NULL;
	g_KdShaderVariable                    = NULL;
	g_KsPointShaderVariable               = NULL;
	g_KsDirShaderVariable                 = NULL;
	g_SpecPowerShaderVariable             = NULL;
	g_FPSShaderVariable                   = NULL;
	g_timeCycleShaderVariable             = NULL;
	g_rainSplashesXDisplaceShaderVariable = NULL;
	g_rainSplashesYDisplaceShaderVariable = NULL;
}