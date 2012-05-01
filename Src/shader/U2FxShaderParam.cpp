#include <U2_3D/Src/U23DLibPCH.h>
#include "U2FxShaderParam.h"

//-------------------------------------------------------------------------------------------------
/// the shader parameter type string table
static const char* g_szTypeTable[U2FxShaderState::NumTypes] =
{
	"v",
	"b",
	"i",
	"f",
	"f4",
	"m44",
	"t"
};

//-------------------------------------------------------------------------------------------------
/// the shader parameter name string table
static const char* g_szStateTable[U2FxShaderState::NumParameters] =
{
	"Model",
	"InvModel",
	"View",
	"InvView",
	"Projection",
	"ModelView",
	"InvModelView",
	"ModelViewProjection",
	"ModelShadowProjection",
	"EyeDir",
	"EyePos",
	"ModelEyePos",
	"ModelLightPos",
	"LightPos",
	"LightType",
	"LightRange",
	"LightAmbient",
	"LightDiffuse",
	"LightDiffuse1",
	"LightSpecular",
	"MatAmbient",
	"MatDiffuse",
	"MatEmissive",
	"MatEmissiveIntensity",
	"MatSpecular",
	"MatSpecularPower",
	"MatTransparency",
	"MatFresnel",
	"Scale",
	"Noise",
	"MatTranslucency",
	"AlphaRef",
	"CullMode",
	"DirAmbient",
	"FogDistances",
	"FogColor",
	"DiffMap0",
	"DiffMap1",
	"DiffMap2",
	"DiffMap3",
	"DiffMap4",
	"DiffMap5",
	"DiffMap6",
	"DiffMap7",
	"SpecMap0",
	"SpecMap1",
	"SpecMap2",
	"SpecMap3",
	"AmbientMap0",
	"AmbientMap1",
	"AmbientMap2",
	"AmbientMap3",
	"BumpMap0",
	"BumpMap1",
	"BumpMap2",
	"BumpMap3",
	"CubeMap0",
	"CubeMap1",
	"CubeMap2",
	"CubeMap3",
	"NoiseMap0",
	"NoiseMap1",
	"NoiseMap2",
	"NoiseMap3",
	"LightModMap",
	"ShadowMap",
	"SpecularMap",
	"ShadowModMap",
	"NormalMap",
	"ShadowColor",
	"JointPalette",
	"Time",
	"Wind",
	"Swing",
	"InnerLightIntensity",
	"OuterLightIntensity",
	"BoxMinPos",
	"BoxMaxPos",
	"BoxCenter",
	"MinDist",
	"MaxDist",
	"SpriteSize",
	"MinSpriteSize",
	"MaxSpriteSize",
	"SpriteSwingAngle",
	"SpriteSwingTime",
	"SpriteSwingTranslate",
	"DisplayResolution",
	"TexGenS",
	"TexGenT",
	"TexGenR",
	"TexGenQ",
	"TextureTransform0",
	"TextureTransform1",
	"TextureTransform2",
	"TextureTransform3",
	"SampleOffsets",
	"SampleWeights",
	"VertexStreams",
	"VertexWeights1",
	"VertexWeights2",
	"AlphaBlendEnable",
	"AlphaSrcBlend",
	"AlphaDstBlend",
	"BumpScale",
	"FresnelBias",
	"FresnelPower",
	"Intensity0",
	"Intensity1",
	"Intensity2",
	"Intensity3",
	"Amplitude",
	"Frequency",
	"Velocity",
	"StencilFrontZFailOp",
	"StencilFrontPassOp",
	"StencilBackZFailOp",
	"StencilBackPassOp",
	"ZWriteEnable",
	"ZEnable",
	"ShadowIndex",
	"CameraFocus",
	"Color0",
	"Color1",
	"Color2",
	"Color3",
	"HalfPixelSize",
	"MLPUVStretch",
	"MLPSpecIntensity",
	"UVStretch0",
	"UVStretch1",
	"UVStretch2",
	"UVStretch3",
	"UVStretch4",
	"UVStretch5",
	"UVStretch6",
	"UVStretch7",
	"LeafCluster",
	"LeafAngleMatrices",
	"WindMatrices",
	"RenderTargetOffset",
	"RenderComplexity",
	"SkyBottom",
	"SunFlat",
	"SunRange",
	"SunColor",
	"CloudMod",
	"CloudPos",
	"CloudGrad",
	"Brightness",
	"Lightness",
	"Density",
	"Glow",
	"Saturation",
	"Weight",
	"TopColor",
	"BottomColor",
	"Move",
	"Position",
	"ScaleVector",
	"Map0uvRes",
	"Map1uvRes",
	"BumpFactor",

	"BiasU",
	"BiasV", 
	"NormalScale", 
	"TexelSize",
	"LightDir",	

};

//-------------------------------------------------------------------------------------------------
const char*
U2FxShaderState::TypeToString(U2FxShaderState::Type t)
{
	U2ASSERT((t >= 0) && (t < U2FxShaderState::NumTypes));
	return g_szTypeTable[t];
}

//-------------------------------------------------------------------------------------------------
U2FxShaderState::Type
U2FxShaderState::StringToType(const char* str)
{
	U2ASSERT(str);
	int i;
	for (i = 0; i < U2FxShaderState::NumTypes; i++)
	{
		if (0 == strcmp(str, g_szTypeTable[i]))
		{
			return (U2FxShaderState::Type) i;
		}
	}
	// fallthrough: state not found
	return U2FxShaderState::InvalidType;
}

//-------------------------------------------------------------------------------------------------
const char*
U2FxShaderState::ParamToString(U2FxShaderState::Param p)
{
	U2ASSERT((p >= 0) && (p < U2FxShaderState::NumParameters));
	return g_szStateTable[p];
}

//-------------------------------------------------------------------------------------------------
U2FxShaderState::Param
U2FxShaderState::StringToParam(const char* str)
{
	U2ASSERT(str);
	int i;
	for (i = 0; i < U2FxShaderState::NumParameters; i++)
	{
		if (0 == strcmp(str, g_szStateTable[i]))
		{
			return (U2FxShaderState::Param) i;
		}
	}
	// fallthrough: state not found
	return U2FxShaderState::InvalidParameter;
}
//-------------------------------------------------------------------------------------------------