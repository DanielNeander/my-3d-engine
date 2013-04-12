
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
#include "stdafx.h"
#include "App.h"


#pragma comment( lib, "D3DX11Effects.lib")

#ifdef _DEBUG
#ifdef _WINDOWS
#pragma comment( lib, "wxmsw28d_core.lib" )	// wxCore Debug Lib
#pragma comment( lib, "wxmsw28d_adv.lib" )
#pragma comment( lib, "wxmsw28d_qa.lib" )
#pragma comment( lib, "wxmsw28d_aui.lib" )
#pragma comment(lib, "cximagecrt32d_VC100.lib")
// Winsock 2
#pragma comment( lib, "ws2_32.lib ") // This lib is required by wxbase28_net lib
#elif _MAC
#pragma comment( lib, "wxmac28d_core.lib" )
#pragma comment( lib, "wxmac28d_adv.lib" )
#pragma comment( lib, "wxmac28d_gl.lib" )
#pragma comment( lib, "wxmac28d_qa.lib" )
#pragma comment( lib, "wxmac28d_aui.lib" )
#endif

#pragma comment( lib, "wxzlibd.lib" )
#pragma comment( lib, "wxregexd.lib" )
#pragma comment( lib, "wxbase28d.lib" )
#pragma comment( lib, "wxbase28d_net.lib" )
#pragma comment( lib, "wxexpatd.lib" )
#pragma comment( lib, "wxbase28d_xml.lib" )

#pragma comment( lib, "openSteerLibd.lib" )
#pragma comment( lib, "d3dx9d.lib" )


// cxImage
//#ifdef _WINDOWS
//#if _MSC_VER==1600		// If VC100 (VS2010)
//#ifdef _WIN64
//#pragma message("     Adding library: cximagecrt64d_VC100.lib" ) 
//#pragma comment( lib, "cximagecrt64d_VC100.lib" )
//#else
//#pragma message("     Adding library: cximagecrt32d_VC100.lib" ) 
//#pragma comment( lib, "cximagecrt32d_VC100.lib" )
//#endif
//#elif _MSC_VER==1500	// If VC90 (VS2008)
//#ifdef _WIN64
//#pragma message("     Adding library: cximagecrt64d_VC90.lib" ) 
//#pragma comment( lib, "cximagecrt64d_VC90.lib" )
//#else
//#pragma message("     Adding library: cximagecrt32d_VC90.lib" ) 
//#pragma comment( lib, "cximagecrt32d_VC90.lib" )
//#endif
//#else					// Otherwise
//#pragma message("     Adding library: cximagecrtd.lib" ) 
//#pragma comment( lib, "cximagecrtd.lib" )
//#endif
//#else
//#pragma message("     Adding library: cximagecrtd.lib" ) 
//#pragma comment( lib, "cximagecrtd.lib" )
//#endif
#else	// Release
#define NDEBUG			// Disables Asserts in release
#define VC_EXTRALEAN	// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

#ifdef _WINDOWS
#pragma comment( lib, "wxmsw28_core.lib" )
#pragma comment( lib, "wxmsw28_adv.lib" )
#pragma comment( lib, "wxmsw28_qa.lib" )
#pragma comment( lib, "wxmsw28_aui.lib" )

#pragma comment( lib, "ws2_32.lib ") // This lib is required by wxbase28_net lib
#elif _MAC
#pragma comment( lib, "wxmac28_core.lib" )
#pragma comment( lib, "wxmac28_adv.lib" )
#pragma comment( lib, "wxmac28_gl.lib" )
#pragma comment( lib, "wxmac28_qa.lib" )
#pragma comment( lib, "wxmac28_aui.lib" )
#endif

#pragma comment( lib, "wxzlib.lib" )
#pragma comment( lib, "wxregex.lib" )
#pragma comment( lib, "wxbase28.lib" )
#pragma comment( lib, "wxbase28_net.lib" )
#pragma comment( lib, "wxexpat.lib" )
#pragma comment( lib, "wxbase28_xml.lib" )

#pragma comment(lib, "cximagecrt32_VC100.lib")

#pragma comment( lib, "openSteerLib.lib" )


//// cxImage
//#if defined(_WINDOWS)
//#if _MSC_VER==1600		// If VC100 (VS2010)
//#ifdef _WIN64
//#pragma message("     Adding library: cximagecrt64_VC100.lib" ) 
//#pragma comment( lib, "cximagecrt64_VC100.lib" )
//#else
//#pragma message("     Adding library: cximagecrt32_VC100.lib" ) 
//#pragma comment( lib, "cximagecrt32_VC100.lib" )
//#endif
//#elif _MSC_VER==1500	// If VC90 (VS2008)
//#ifdef _WIN64
//#pragma message("     Adding library: cximagecrt64_VC90.lib" ) 
//#pragma comment( lib, "cximagecrt64_VC90.lib" )
//#else
//#pragma message("     Adding library: cximagecrt32_VC90.lib" ) 
//#pragma comment( lib, "cximagecrt32_VC90.lib" )
//#endif
//#else					// Otherwise
//#pragma message("     Adding library: cximagecrt.lib" ) 
//#pragma comment( lib, "cximagecrt.lib" )
//#endif
//#else
//#pragma message("     Adding library: cximagecrt.lib" ) 
//#pragma comment( lib, "cximagecrt.lib" )
//#endif
#endif // _DEBUG

BaseApp *g_pApplication;

BaseApp *app = new App();

bool App::onKey(const uint key, const bool pressed){
	if (D3D11App::onKey(key, pressed)) return true;

	if (pressed){
		if (key >= KEY_F5 && key <= KEY_F8){
			renderMode->selectItem(key - KEY_F5);
		}
	}

	return false;
}

void App::moveCamera(const float3 &dir){
	float3 newPos = camPos + dir * (speed * frameTime);

	float3 point;
	const BTri *tri;
	if (bsp.intersects(camPos, newPos, &point, &tri)){
		newPos = point + tri->plane.xyz();
	}
	bsp.pushSphere(newPos, 35);

	camPos = newPos;
}

void App::resetCamera(){
	camPos = vec3(-730, 20, 2010);
	wx = 0.14f;
	wy = -2.63f;
}

void App::onSize(const int w, const int h){
	D3D11App::onSize(w, h);

	if (renderer){
		// Make sure render targets are the size of the window
		renderer->resizeRenderTarget(baseRT,      w, h, 1, 1, 1);
		renderer->resizeRenderTarget(normalRT,    w, h, 1, 1, 1);
		renderer->resizeRenderTarget(depthRT,     w, h, 1, 1, 1);
		renderer->resizeRenderTarget(stencilMask, w, h, 1, 1, 1);
	}
}

bool App::init(){
	// No framework created depth buffer
	//depthBits = 0;

	map = new Model();
	if (!map->loadObj("../Models/Corridor2/Map.obj")) return false;
	map->scale(0, float3(1, 1, -1));

	uint nIndices = map->getIndexCount();
	float3 *src = (float3 *) map->getStream(0).vertices;
	uint *inds = map->getStream(0).indices;

	for (uint i = 0; i < nIndices; i += 3){
		float3 v0 = src[inds[i]];
		float3 v1 = src[inds[i + 1]];
		float3 v2 = src[inds[i + 2]];

		bsp.addTriangle(v0, v1, v2);
	}
	bsp.build();

	map->computeTangentSpace(true);

	sphere = new Model();
	sphere->createSphere(3);

	// Initialize all lights
	lights[ 0].position = float3( 576, 96,    0); lights[ 0].radius = 640.0f;
	lights[ 1].position = float3( 0,   96,  576); lights[ 1].radius = 640.0f;
	lights[ 2].position = float3(-576, 96,    0); lights[ 2].radius = 640.0f;
	lights[ 3].position = float3( 0,   96, -576); lights[ 3].radius = 640.0f;
	lights[ 4].position = float3(1792, 96,  320); lights[ 4].radius = 550.0f;
	lights[ 5].position = float3(1792, 96, -320); lights[ 5].radius = 550.0f;
	lights[ 6].position = float3(-192, 96, 1792); lights[ 6].radius = 550.0f;
	lights[ 7].position = float3(-832, 96, 1792); lights[ 7].radius = 550.0f;
	lights[ 8].position = float3(1280, 32,  192); lights[ 8].radius = 450.0f;
	lights[ 9].position = float3(1280, 32, -192); lights[ 9].radius = 450.0f;
	lights[10].position = float3(-320, 32, 1280); lights[10].radius = 450.0f;
	lights[11].position = float3(-704, 32, 1280); lights[11].radius = 450.0f;
	lights[12].position = float3( 960, 32,  640); lights[12].radius = 450.0f;
	lights[13].position = float3( 960, 32, -640); lights[13].radius = 450.0f;
	lights[14].position = float3( 640, 32, -960); lights[14].radius = 450.0f;
	lights[15].position = float3(-640, 32, -960); lights[15].radius = 450.0f;
	lights[16].position = float3(-960, 32,  640); lights[16].radius = 450.0f;
	lights[17].position = float3(-960, 32, -640); lights[17].radius = 450.0f;
	lights[18].position = float3( 640, 32,  960); lights[18].radius = 450.0f;

	// Init GUI components
	int tab = configDialog->addTab("Rendering mode");
	configDialog->addWidget(tab, new Label(10, 10, 340, 36, "Rendering mode (F5-F8)"));

	renderMode = new DropDownList(10, 50, 380, 36);
	renderMode->addItem("Default, 2 pass, combined");
	renderMode->addItem("Multiple sample evaluation");
	renderMode->addItem("Single sample evaluation");
	renderMode->addItem("Visualize stencil mask");
	renderMode->selectItem(0);
	configDialog->addWidget(tab, renderMode);

	return true;
}

void App::exit(){
	delete sphere;
	delete map;
}

bool App::initAPI(){
	// Override the user's MSAA settings
	return D3D11App::initAPI(D3D11, DXGI_FORMAT_R10G10B10A2_UNORM, DXGI_FORMAT_UNKNOWN, 1, NO_SETTING_CHANGE | SAMPLE_BACKBUFFER);
}

void App::exitAPI(){
	D3D11App::exitAPI();
}

bool App::load(){
	int sampleCount = max(antiAliasSamples, 1);

	char def[256];
	sprintf(def, "#define SAMPLE_COUNT %d\n", sampleCount);

	// Shaders
	if ((fillBuffers = renderer->addShader("FillBuffers.shd")) == SHADER_NONE) return false;
	if ((ambient     = renderer->addShader("Ambient.shd", def)) == SHADER_NONE) return false;
	if ((createMask  = renderer->addShader("CreateMask.shd")) == SHADER_NONE) return false;
	if ((lighting[0] = renderer->addShader("Lighting.shd", def)) == SHADER_NONE) return false;
	strcat(def, "#define SINGLE_SAMPLE\n");
	if ((lighting[1] = renderer->addShader("Lighting.shd", def)) == SHADER_NONE) return false;

	// Samplerstates
	if ((trilinearAniso = renderer->addSamplerState(TRILINEAR_ANISO, WRAP, WRAP, WRAP)) == SS_NONE) return false;
	if ((pointClamp = renderer->addSamplerState(NEAREST, CLAMP, CLAMP, CLAMP)) == SS_NONE) return false;

	// Main render targets
	if ((baseRT      = renderer->addRenderTarget(width, height, 1, 1, 1, FORMAT_RGB10A2, sampleCount, SS_NONE)) == TEXTURE_NONE) return false;
	if ((normalRT    = renderer->addRenderTarget(width, height, 1, 1, 1, FORMAT_RGBA8S,  sampleCount, SS_NONE)) == TEXTURE_NONE) return false;
	if ((depthRT     = renderer->addRenderDepth (width, height, 1,       FORMAT_D16,     sampleCount, SS_NONE, SAMPLE_DEPTH)) == TEXTURE_NONE) return false;
	if ((stencilMask = renderer->addRenderDepth (width, height, 1,       FORMAT_D24S8,   1,           SS_NONE)) == TEXTURE_NONE) return false;

	// Textures
	if ((base[0] = renderer->addTexture  ("../Textures/Tx_wood_brown_shelf_small.dds",                    true, trilinearAniso)) == TEXTURE_NONE) return false;
	if ((bump[0] = renderer->addNormalMap("../Textures/Tx_wood_brown_shelf_smallBump.dds", FORMAT_RGBA8S, true, trilinearAniso)) == TEXTURE_NONE) return false;

	if ((base[1] = renderer->addTexture  ("../Textures/Tx_imp_wall_01_small.dds",              true, trilinearAniso)) == TEXTURE_NONE) return false;
	if ((bump[1] = renderer->addNormalMap("../Textures/Tx_imp_wall_01Bump.dds", FORMAT_RGBA8S, true, trilinearAniso)) == TEXTURE_NONE) return false;

	if ((base[2] = renderer->addTexture  ("../Textures/floor_wood_4.dds",                    true, trilinearAniso)) == TEXTURE_NONE) return false;
	if ((bump[2] = renderer->addNormalMap("../Textures/floor_wood_4Bump.dds", FORMAT_RGBA8S, true, trilinearAniso)) == TEXTURE_NONE) return false;

	if ((base[3] = renderer->addTexture  ("../Textures/floor_wood_3.dds",                    true, trilinearAniso)) == TEXTURE_NONE) return false;
	if ((bump[3] = renderer->addNormalMap("../Textures/floor_wood_3Bump.dds", FORMAT_RGBA8S, true, trilinearAniso)) == TEXTURE_NONE) return false;

	if ((base[4] = renderer->addTexture  ("../Textures/light2.dds",                    true, trilinearAniso)) == TEXTURE_NONE) return false;
	if ((bump[4] = renderer->addNormalMap("../Textures/light2Bump.dds", FORMAT_RGBA8S, true, trilinearAniso)) == TEXTURE_NONE) return false;

	// Blendstates
	if ((blendAdd = renderer->addBlendState(ONE, ONE)) == BS_NONE) return false;

	// Depth states - use reversed depth (1 to 0) to improve precision
	if ((depthTest = renderer->addDepthState(true, true, GEQUAL)) == DS_NONE) return false;
	if ((stencilSet = renderer->addDepthState(false, false, GEQUAL, true, 0xFF, ALWAYS, REPLACE, REPLACE, REPLACE)) == DS_NONE) return false;
	if ((stencilTest = renderer->addDepthState(false, false, GEQUAL, true, 0xFF, EQUAL, KEEP, KEEP, KEEP)) == DS_NONE) return false;

	// Upload map to vertex/index buffer
	if (!map->makeDrawable(renderer, true, fillBuffers)) return false;
	if (!sphere->makeDrawable(renderer, true, lighting[0])) return false;

	return true;
}

void App::unload(){

}

void App::drawFrame(){
	const float near_plane = 20.0f;
	const float far_plane = 4000.0f;

	// Reversed depth
	float4x4 projection = toD3DProjection(perspectiveMatrixY(1.2f, width, height, far_plane, near_plane));
	float4x4 view = rotateXY(-wx, -wy);
	view.translate(-camPos);
	float4x4 viewProj = projection * view;
	// Pre-scale-bias the matrix so I can use the screen position directly
	float4x4 viewProjInv = (!viewProj) * (translate(-1.0f, 1.0f, 0.0f) * scale(2.0f / width, -2.0f / height, 1.0f));


	TextureID bufferRTs[] = { baseRT, normalRT };
	renderer->changeRenderTargets(bufferRTs, elementsOf(bufferRTs), depthRT);
		renderer->clear(false, true, false, NULL, 0.0f);

		/*
			Main scene pass.
			This is where the buffers are filled for the later deferred passes.
		*/
		renderer->reset();
		renderer->setRasterizerState(cullBack);
		renderer->setShader(fillBuffers);
		renderer->setShaderConstant4x4f("viewProj", viewProj);
		renderer->setShaderConstant3f("camPos", camPos);
		renderer->setSamplerState("baseFilter", trilinearAniso);
		renderer->setDepthState(depthTest);
		renderer->apply();

		for (uint i = 0; i < map->getBatchCount(); i++){
			renderer->setTexture("Base", base[i]);
			renderer->setTexture("Bump", bump[i]);
			renderer->applyTextures();

			map->drawBatch(renderer, i);
		}

	renderer->changeToMainFramebuffer();


	int mode = renderMode->getSelectedItem();

	/*
		Deferred ambient pass
	*/
	renderer->reset();
	renderer->setRasterizerState(cullNone);
	renderer->setShader(ambient);
	renderer->setShaderConstant2f("factors", float2((mode == 3)? 0.0f : 0.1f, (mode == 3)? 1.0f : 0.0f));
	renderer->setTexture("Base", baseRT);
	if (antiAliasSamples == 1)
		renderer->setSamplerState("filter", pointClamp);
	renderer->setDepthState(noDepthTest);
	renderer->apply();


	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->Draw(3, 0);

	if (mode == 3)
		return;


	renderer->changeRenderTargets(NULL, 0, stencilMask);
		renderer->clear(false, true, true, NULL, 0.0f, 0);

		/*
			Create the stencil mask
		*/
		renderer->reset();
		renderer->setRasterizerState(cullNone);
		renderer->setShader(createMask);
		renderer->setTexture("BackBuffer", backBufferTexture);
		renderer->setSamplerState("filter", pointClamp);
		renderer->setDepthState(stencilSet, 0x1);
		renderer->apply();

		context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->Draw(3, 0);

	renderer->changeRenderTarget(FB_COLOR, stencilMask);


	/*
		Deferred lighting pass.
		Draw twice, using stencil to separate pixels for single or multiple sample evaluation.
	*/
	float2 zw = projection.rows[2].zw();

	int passCount = (mode == 0 && antiAliasSamples > 1)? 2 : 1;

	for (int p = 0; p < passCount; p++){
		renderer->reset();

		if (mode == 0 && antiAliasSamples > 1){
			renderer->setDepthState(stencilTest, (p == 0)? 0x1 : 0x0);
			renderer->setShader(lighting[p]);
		} else {
			renderer->setDepthState(noDepthTest);
			if (mode == 1){
				renderer->setShader(lighting[0]);
			} else {
				renderer->setShader(lighting[1]);
			}
		}


		renderer->setRasterizerState(cullFront);
		renderer->setBlendState(blendAdd);
		renderer->setShaderConstant4x4f("viewProj", viewProj);
		renderer->setShaderConstant4x4f("viewProjInv", viewProjInv);
		renderer->setShaderConstant3f("camPos", camPos);
		renderer->setTexture("Base", baseRT);
		renderer->setTexture("Normal", normalRT);
		renderer->setTexture("Depth", depthRT);
		renderer->apply();

		for (uint i = 0; i < LIGHT_COUNT; i++){
			float3 lightPos = lights[i].position;
			float radius = lights[i].radius;
			float invRadius = 1.0f / radius;

			// Compute z-bounds
			float4 lPos = view * float4(lightPos, 1.0f);
			float z1 = lPos.z + radius;

			if (z1 > near_plane){
				float z0 = max(lPos.z - radius, near_plane);

				float2 zBounds;
				zBounds.y = saturate(zw.x + zw.y / z0);
				zBounds.x = saturate(zw.x + zw.y / z1);

				renderer->setShaderConstant3f("lightPos", lightPos);
				renderer->setShaderConstant1f("radius", radius);
				renderer->setShaderConstant1f("invRadius", invRadius);
				renderer->setShaderConstant2f("zBounds", zBounds);
				renderer->applyConstants();

				sphere->draw(renderer);
			}
		}

	}

	// Display help text
	static float displayTime = 5.0f;
	if (displayTime > 0 && antiAliasSamples <= 1){
		if (configDialog->isVisible()){
			displayTime = 0;
		} else {
			displayTime -= min(frameTime, 0.1f);
			renderer->drawText("Press F1 to select\na multisampled mode", width * 0.5f - 140, height * 0.5f - 38, 30, 38, defaultFont, linearClamp, blendSrcAlpha, noDepthTest);
		}
	}

}

float App::GetAspectRatio(void)
{
	RECT rcClient;
	GetClientRect(GetAppBase()->getWindow(), &rcClient);
	unsigned int iClientWidth = rcClient.right - rcClient.left;
	unsigned int iClientHeight = rcClient.bottom - rcClient.top;
	return iClientWidth / (float)iClientHeight;
}

void App::SetDefaultRenderTarget( void )
{
	renderer->changeToMainFramebuffer();

	RECT rcClient;
	GetClientRect(getWindow(), &rcClient);
	unsigned int iClientWidth = rcClient.right - rcClient.left;
	unsigned int iClientHeight = rcClient.bottom - rcClient.top;

	D3D11_VIEWPORT vp;
	vp.Width = iClientWidth;
	vp.Height = iClientHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	context->RSSetViewports( 1, &vp );
}