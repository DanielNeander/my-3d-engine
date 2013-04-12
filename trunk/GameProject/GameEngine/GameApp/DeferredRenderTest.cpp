#include "stdafx.h"
#include <EngineCore/Util/BSP.h>
#include <EngineCore/Util/Model.h>

#include "GameApp.h"
#include "CascadedShadowsManager.h"
#include "GameApp/Mesh.h"
#include "GameApp/SceneNode.h"
#include "GameApp/M2Mesh.h"
#include "GameApp/Mesh_D3D11.h"
#include "GameApp/TestObjects.h"
#include "GameApp/GameObjectManager.h"
#include "GameApp/GameObjectUtil.h"
#include "GameApp/Common.h"
#include "ModelLib/M2Loader.h"
#include "GameApp/Actor.h"
#include "GameApp/Font/Dx11TextHelper.h"
#include "SDKMeshLoader.h"

#include "U2Light.h"

struct LightInfo
{
	float3 position;
	float radius;
};

#define LIGHT_COUNT 19

ShaderID fillBuffers, ambient, lighting[2], createMask;
TextureID base[5], bump[5];

TextureID baseRT, normalRT, depthRT, stencilMask;

SamplerStateID trilinearAniso, pointClamp;
BlendStateID blendAdd;
DepthStateID depthTest, stencilSet, stencilTest;

LightInfo lights[LIGHT_COUNT];
DropDownList *renderMode;

static Model *Map;
static Model *sphere;
static BSP bsp;

void App::moveCamera(const vec3 &dir)
{
	float3 newPos = camPos + dir * (speed * frameTime);

	float3 point;
	const BTri *tri;
	if (bsp.intersects(camPos, newPos, &point, &tri)){
		newPos = point + tri->plane.xyz();
	}
	bsp.pushSphere(newPos, 35);

	camPos = newPos;
}

bool App::InitDeferredShading()
{
	// No framework created depth buffer
	depthBits = 0;

	Map = new Model();
	if (!Map->loadObj("Models/Corridor2/Map.obj")) return false;
	Map->scale(0, float3(1, 1, -1));

	uint nIndices = Map->getIndexCount();
	float3 *src = (float3 *) Map->getStream(0).vertices;
	uint *inds = Map->getStream(0).indices;

	for (uint i = 0; i < nIndices; i += 3){
		float3 v0 = src[inds[i]];
		float3 v1 = src[inds[i + 1]];
		float3 v2 = src[inds[i + 2]];

		bsp.addTriangle(v0, v1, v2);
	}
	bsp.build();

	Map->computeTangentSpace(true);

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

	int sampleCount = max(antiAliasSamples, 1);

	char def[256];
	sprintf(def, "#define SAMPLE_COUNT %d\n", sampleCount);

	// Shaders
	if ((fillBuffers = renderer->addShader("Data/Shaders/FillBuffers.hlsl")) == SHADER_NONE) return false;
	if ((ambient     = renderer->addShader("Data/Shaders/Ambient.hlsl", def)) == SHADER_NONE) return false;
	if ((createMask  = renderer->addShader("Data/Shaders/CreateMask.hlsl")) == SHADER_NONE) return false;
	if ((lighting[0] = renderer->addShader("Data/Shaders/Lighting.shd", def)) == SHADER_NONE) return false;
	strcat(def, "#define SINGLE_SAMPLE\n");
	if ((lighting[1] = renderer->addShader("Data/Shaders/Lighting.shd", def)) == SHADER_NONE) return false;

	// Samplerstates
	if ((trilinearAniso = renderer->addSamplerState(TRILINEAR_ANISO, WRAP, WRAP, WRAP)) == SS_NONE) return false;
	if ((pointClamp = renderer->addSamplerState(NEAREST, CLAMP, CLAMP, CLAMP)) == SS_NONE) return false;

	// Main render targets
	if ((baseRT      = renderer->addRenderTarget(width, height, 1, 1, 1, FORMAT_RGB10A2, sampleCount, SS_NONE)) == TEXTURE_NONE) return false;
	if ((normalRT    = renderer->addRenderTarget(width, height, 1, 1, 1, FORMAT_RGBA8S,  sampleCount, SS_NONE)) == TEXTURE_NONE) return false;
	if ((depthRT     = renderer->addRenderDepth (width, height, 1,       FORMAT_D16,     sampleCount, SS_NONE, SAMPLE_DEPTH)) == TEXTURE_NONE) return false;
	if ((stencilMask = renderer->addRenderDepth (width, height, 1,       FORMAT_D24S8,   1,           SS_NONE)) == TEXTURE_NONE) return false;

	// Textures
	if ((base[0] = renderer->addTexture  ("Textures/Tx_wood_brown_shelf_small.dds",                    true, trilinearAniso)) == TEXTURE_NONE) return false;
	if ((bump[0] = renderer->addNormalMap("Textures/Tx_wood_brown_shelf_smallBump.dds", FORMAT_RGBA8S, true, trilinearAniso)) == TEXTURE_NONE) return false;

	if ((base[1] = renderer->addTexture  ("Textures/Tx_imp_wall_01_small.dds",              true, trilinearAniso)) == TEXTURE_NONE) return false;
	if ((bump[1] = renderer->addNormalMap("Textures/Tx_imp_wall_01Bump.dds", FORMAT_RGBA8S, true, trilinearAniso)) == TEXTURE_NONE) return false;

	if ((base[2] = renderer->addTexture  ("Textures/floor_wood_4.dds",                    true, trilinearAniso)) == TEXTURE_NONE) return false;
	if ((bump[2] = renderer->addNormalMap("Textures/floor_wood_4Bump.dds", FORMAT_RGBA8S, true, trilinearAniso)) == TEXTURE_NONE) return false;

	if ((base[3] = renderer->addTexture  ("Textures/floor_wood_3.dds",                    true, trilinearAniso)) == TEXTURE_NONE) return false;
	if ((bump[3] = renderer->addNormalMap("Textures/floor_wood_3Bump.dds", FORMAT_RGBA8S, true, trilinearAniso)) == TEXTURE_NONE) return false;

	if ((base[4] = renderer->addTexture  ("Textures/light2.dds",                    true, trilinearAniso)) == TEXTURE_NONE) return false;
	if ((bump[4] = renderer->addNormalMap("Textures/light2Bump.dds", FORMAT_RGBA8S, true, trilinearAniso)) == TEXTURE_NONE) return false;

	// Blendstates
	if ((blendAdd = renderer->addBlendState(ONE, ONE)) == BS_NONE) return false;

	// Depth states - use reversed depth (1 to 0) to improve precision
	if ((depthTest = renderer->addDepthState(true, true, GEQUAL)) == DS_NONE) return false;
	if ((stencilSet = renderer->addDepthState(false, false, GEQUAL, true, 0xFF, ALWAYS, REPLACE, REPLACE, REPLACE)) == DS_NONE) return false;
	if ((stencilTest = renderer->addDepthState(false, false, GEQUAL, true, 0xFF, EQUAL, KEEP, KEEP, KEEP)) == DS_NONE) return false;

	// Upload map to vertex/index buffer
	if (!Map->makeDrawable(renderer, true, fillBuffers)) return false;
	if (!sphere->makeDrawable(renderer, true, lighting[0])) return false;

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

void App::DrwaDeferredPass()
{
	if (!Map) return;
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

		for (uint i = 0; i < Map->getBatchCount(); i++){
			renderer->setTexture("Base", base[i]);
			renderer->setTexture("Bump", bump[i]);
			renderer->applyTextures();

			Map->drawBatch(renderer, i);
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