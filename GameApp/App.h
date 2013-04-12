
/* * * * * * * * * * * * * Author's note * * * * * * * * * * * *\
*   _       _   _       _   _       _   _       _     _ _ _ _   *
*  |_|     |_| |_|     |_| |_|_   _|_| |_|     |_|  _|_|_|_|_|  *
*  |_|_ _ _|_| |_|     |_| |_|_|_|_|_| |_|     |_| |_|_ _ _     *
*  |_|_|_|_|_| |_|     |_| |_| |_| |_| |_|     |_|   |_|_|_|_   *
*  |_|     |_| |_|_ _ _|_| |_|     |_| |_|_ _ _|_|  _ _ _ _|_|  *
*  |_|     |_|   |_|_|_|   |_|     |_|   |_|_|_|   |_|_|_|_|    *
*                                                               *
*                     http://www.humus.name                     *
*                                                                *
* This file is a part of the work done by Humus. You are free to   *
* use the code in any way you like, modified, unmodified or copied   *
* into your own work. However, I expect you to respect these points:  *
*  - If you use this file and its contents unmodified, or use a major *
*    part of this file, please credit the author and leave this note. *
*  - For use in anything commercial, please request my approval.     *
*  - Share your work and ideas too as much as you can.             *
*                                                                *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "EngineCore/Direct3D11/D3D11App.h"
#include "EngineCore/Util/Model.h"
#include "EngineCore/Util/BSP.h"

struct Light2
{
	float3 position;
	float radius;
};

#define LIGHT_COUNT 19

class App : public D3D11App {
public:
	char *getTitle() const { return "Deferred shading 2"; }

	bool onKey(const uint key, const bool pressed);

	void moveCamera(const float3 &dir);
	void resetCamera();

	void onSize(const int w, const int h);

	bool init();
	void exit();

	bool initAPI();
	void exitAPI();

	bool load();
	void unload();

	void drawFrame();

	float GetAspectRatio(void);

	void SetDefaultRenderTarget(void);
protected:
	ShaderID fillBuffers, ambient, lighting[2], createMask;
	TextureID base[5], bump[5];

	TextureID baseRT, normalRT, depthRT, stencilMask;

	SamplerStateID trilinearAniso, pointClamp;
	BlendStateID blendAdd;
	DepthStateID depthTest, stencilSet, stencilTest;

	Light2 lights[LIGHT_COUNT];

	DropDownList *renderMode;

	Model *map;
	Model *sphere;
	BSP bsp;
};


extern BaseApp *g_pApplication;
inline BaseApp *GetAppBase(void) { return g_pApplication; }
inline App *GetApp(void) { return (App *)g_pApplication; }

inline bool GetKeyDown(int iVirtualKey)
{
	if(GetFocus()!=GetAppBase()->getWindow()) return false;
	return (GetKeyState(iVirtualKey) & 0xfe) ? true : false;
}

// returns true if mouse button VK_LBUTTON / VK_RBUTTON / VK_MBUTTON is down
inline bool GetMouseDown(int iVirtualKey)
{
	if(GetFocus()!=GetAppBase()->getWindow()) return false;
	return (GetAsyncKeyState(iVirtualKey) & 0x8000) ? true : false;
}

HRESULT LoadEffectFromFile( ID3D11Device* pd3dDevice, const char* szFileName, ID3DX11Effect** ppEffect, const D3D10_SHADER_MACRO *pDefines = NULL );