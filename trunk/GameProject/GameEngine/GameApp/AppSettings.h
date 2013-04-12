//=================================================================================================
//
//  Light Indexed Deferred Sample
//  by MJP
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#pragma once

//#include "Input.h"
//#include "Slider.h"
//#include "TextGUI.h"

class TextGUI 
{
public:
	TextGUI::TextGUI(const WCHAR* name, UINT32 value, UINT32 numValues);
	TextGUI::TextGUI(const WCHAR* name, UINT32 value, UINT32 numValues, const WCHAR** valueNames);
	
	operator UINT32() { return value; }
	
	void SetNames(const WCHAR** valueNames)
	{
		for(UINT32 i = 0; i < numValues; ++i)
			this->valueNames.push_back(valueNames[i]);
	}
	void SetValue(UINT32 newValue) { value = min(newValue, numValues - 1); }

	

	std::wstring name;
	UINT32 value;
	UINT32 numValues;
	std::vector<std::wstring> valueNames;
};

class RenderingModeGUI : public TextGUI
{
public:

    enum Values {
        TiledDeferred = 0,
        IndexedDeferred = 1,

        NumValues
    };

    RenderingModeGUI() : TextGUI(L"Rendering Mode", IndexedDeferred, NumValues)
    {
        static const WCHAR* Names[] =
        {
            L"Tiled Deferred",
            L"Indexed Deferred",
        };

        SetNames(Names);
    }
};

class MSAAModeGUI : public TextGUI
{
public:

    enum Values {
        None = 0,
        MSAA2x = 1,
        MSAA4x = 2,

        NumValues
    };

    MSAAModeGUI() : TextGUI(L"MSAA Mode", None, NumValues)
    {
        static const WCHAR* Names[] =
        {
            L"None",
            L"2x MSAA",
            L"4x MSAA"
        };

        SetNames(Names);
    }

    static UINT32 NumSamples(UINT32 msaaMode)
    {
        if(msaaMode == MSAA2x)
            return 2;
        else if(msaaMode == MSAA4x)
            return 4;
        else
            return 1;
    }

    UINT32 NumSamples()
    {
        return NumSamples(value);
    }
};

class NumLightsGUI : public TextGUI
{
public:

    enum Values {
        Lights128 = 0,
        Lights256 = 1,
        Lights512 = 2,
        Lights1024 = 3,

        NumValues
    };

    NumLightsGUI() : TextGUI(L"Lights", Lights128, NumValues)
    {
        static const WCHAR* Names[] =
        {
            L"128",
            L"256",
            L"512",
            L"1024"
        };

        SetNames(Names);
    }

    static UINT32 NumLights(UINT32 lightsMode)
    {
        if(lightsMode == Lights128)
            return 12;
        else if(lightsMode == Lights256)
            return 25;
        else if(lightsMode == Lights512)
            return 51;
        else
            return 1024;
    }

    UINT32 NumLights()
    {
        return NumLights(value);
    }
};

enum ShadowMode 
{
	SHADOW_NONE,
	SHADOW_PSSM,
	SHADOW_SSAO,
	SHADOW_CASCADED,
};

enum EditMode 
{
	EDIT_TEST_MODE, 
	EDIT_TERRAIN,
};

enum AssetType 
{
	ASSERT_NONE,
	ASSERT_SPELLS,
	ASSET_DECO,
	ASSET_BUILDING,
	ASSET_CHAR,
};

enum RendererType
{
	FORWARD_RENDER,
	DEFERRED_RENDER,
};

class AppSettings
{
public:
	

    // Sliders for adjusting values
   /* static Slider BloomThreshold;
    static Slider BloomMagnitude;
    static Slider BloomBlurSigma;
    static Slider KeyValue;
    static Slider AdaptationRate;
    static Slider DiffuseR;
    static Slider DiffuseG;
    static Slider DiffuseB;
    static Slider DiffuseIntensity;
    static Slider SpecularR;
    static Slider SpecularG;
    static Slider SpecularB;
    static Slider SpecularIntensity;
    static Slider Balance;
    static Slider Roughness;

    static BoolGUI EnableZPrepass;*/
	static ShadowMode ShadowModeEnum;
	static EditMode		EditModeEnum;	
	static AssetType	AssetTypeEnum;
	static RendererType RendererTypeEnum;
	static bool	EnableTangentSpace;

    static RenderingModeGUI RenderingMode;
    static MSAAModeGUI MSAAMode;
    static NumLightsGUI NumLights;

    // Collections of GUI objects
    //static std::vector<GUIObject*> GUIObjects;
    //static std::vector<Slider*> Sliders;
    static std::vector<TextGUI*> TextGUIs;

    // Initialization
    static void Initialize(ID3D11Device* device);

    // Resize adjustment
    //static void AdjustGUI(UINT32 displayWidth, UINT32 displayHeight);

    // Frame update/render
    //static void Update(const KeyboardState& kbState, const MouseState& mouseState);
    //static void Render(SpriteRenderer& spriteRenderer, SpriteFont& font);

    static bool MSAAEnabled() { return MSAAMode == MSAAModeGUI::MSAA2x || MSAAMode == MSAAModeGUI::MSAA4x; }
};