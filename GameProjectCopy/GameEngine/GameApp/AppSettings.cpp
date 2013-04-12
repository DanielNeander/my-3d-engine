//=================================================================================================
//
//  Light Indexed Deferred Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#include "stdafx.h"
#include "AppSettings.h"

//Slider AppSettings::BloomThreshold;
//Slider AppSettings::BloomMagnitude;
//Slider AppSettings::BloomBlurSigma;
//Slider AppSettings::KeyValue;
//Slider AppSettings::AdaptationRate;
//Slider AppSettings::DiffuseR;
//Slider AppSettings::DiffuseG;
//Slider AppSettings::DiffuseB;
//Slider AppSettings::DiffuseIntensity;
//Slider AppSettings::SpecularR;
//Slider AppSettings::SpecularG;
//Slider AppSettings::SpecularB;
//Slider AppSettings::SpecularIntensity;
//Slider AppSettings::Balance;
//Slider AppSettings::Roughness;

//BoolGUI AppSettings::EnableZPrepass(L"Enable Z Prepass", true, KeyboardState::N);

MSAAModeGUI AppSettings::MSAAMode;
RenderingModeGUI AppSettings::RenderingMode;
NumLightsGUI AppSettings::NumLights;

//std::vector<GUIObject*> AppSettings::GUIObjects;
//std::vector<Slider*> AppSettings::Sliders;
std::vector<TextGUI*> AppSettings::TextGUIs;


TextGUI::TextGUI(const WCHAR* name, UINT32 value, UINT32 numValues, const WCHAR** valueNames)
	: name(name), value(value), numValues(numValues)
{
	assert(numValues > 0);
	assert(value < numValues);
	SetNames(valueNames);
}

TextGUI::TextGUI(const WCHAR* name, UINT32 value, UINT32 numValues)
	: name(name), value(value), numValues(numValues)
{
	assert(numValues > 0);
	assert(value < numValues);
}


void AppSettings::Initialize(ID3D11Device* device)
{
    // Sliders
  /*  BloomThreshold.Initialize(device, 0.0f, 20.0f, 3.0f, L"Bloom Threshold");
    Sliders.push_back(&BloomThreshold);

    BloomMagnitude.Initialize(device, 0.0f, 2.0f, 1.0f, L"Bloom Magnitude");
    Sliders.push_back(&BloomMagnitude);

    BloomBlurSigma.Initialize(device, 0.5f, 1.5f, 0.8f, L"Bloom Blur Sigma");
    Sliders.push_back(&BloomBlurSigma);

    KeyValue.Initialize(device, 0.0f, 0.5f, 0.115f, L"Auto-Exposure Key Value");
    Sliders.push_back(&KeyValue);

    AdaptationRate.Initialize(device, 0.0f, 4.0f, 0.5f, L"Adaptation Rate");
    Sliders.push_back(&AdaptationRate);

    SpecularR.Initialize(device, 0.0f, 1.0f, 1.0f, L"Specular Albedo R");
    Sliders.push_back(&SpecularR);

    SpecularG.Initialize(device, 0.0f, 1.0f, 1.0f, L"Specular Albedo G");
    Sliders.push_back(&SpecularG);

    SpecularB.Initialize(device, 0.0f, 1.0f, 1.0f, L"Specular Albedo B");
    Sliders.push_back(&SpecularB);

    SpecularIntensity.Initialize(device, 0.0f, 1.0f, 0.02f, L"Specular Intensity");
    Sliders.push_back(&SpecularIntensity);

    Balance.Initialize(device, 0.0f, 1.0f, 0.5f, L"Specular Balance");
    Sliders.push_back(&Balance);

    Roughness.Initialize(device, 0.0f, 1.0f, 0.5f, L"Material Roughness");
    Sliders.push_back(&Roughness);
	   

    for(UINT_PTR i = 0; i < Sliders.size(); ++i)
        GUIObjects.push_back(Sliders[i]);

    for(UINT_PTR i = 0; i < TextGUIs.size(); ++i)
        GUIObjects.push_back(TextGUIs[i]);
		*/
	TextGUIs.push_back(&MSAAMode);
	//TextGUIs.push_back(&EnableZPrepass);
	TextGUIs.push_back(&RenderingMode);
	TextGUIs.push_back(&NumLights);
}

bool AppSettings::EnableTangentSpace = false;

RendererType AppSettings::RendererTypeEnum = FORWARD_RENDER;

AssetType AppSettings::AssetTypeEnum = ASSERT_NONE;

EditMode AppSettings::EditModeEnum = EDIT_TEST_MODE;
ShadowMode AppSettings::ShadowModeEnum = SHADOW_NONE;

//void AppSettings::AdjustGUI(UINT32 displayWidth, UINT32 displayHeight)
//{
//    float width = static_cast<float>(displayWidth);
//    float x = width - 250;
//    float y = 20.0f;
//    const float Spacing = 40.0f;
//
//    for(UINT_PTR i = 0; i < Sliders.size(); ++i)
//    {
//        Sliders[i]->Position() = XMFLOAT2(x, y);
//        y += Spacing;
//    }
//
//    float textY = displayHeight - TextGUIs.size() * 25.0f - 25.0f;
//    for(UINT_PTR i = 0; i < TextGUIs.size(); ++i)
//    {
//        TextGUIs[i]->Position() = XMFLOAT2(25.0f, textY);
//        textY += 25.0f;
//    }
//}

//void AppSettings::Update(const KeyboardState& kbState, const MouseState& mouseState)
//{
//    for(UINT_PTR i = 0; i < GUIObjects.size(); ++i)
//        GUIObjects[i]->Update(kbState, mouseState);
//}
//
//void AppSettings::Render(SpriteRenderer& spriteRenderer, SpriteFont& font)
//{
//    for(UINT_PTR i = 0; i < GUIObjects.size(); ++i)
//        GUIObjects[i]->Render(spriteRenderer, font);
//}