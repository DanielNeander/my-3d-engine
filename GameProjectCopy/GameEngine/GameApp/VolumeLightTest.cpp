#include "stdafx.h"
#include "GameApp/GameApp.h"
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
#include "Utility.h"
#include "ShaderCompilation.h"
#include "PostProcessor.h"
#include "DeviceStates.h"
#include "VolLightRenderer.h"

static PostProcessor postprocess;
static VolLightRenderer litRender;

void App::LoadLights()
{
	litRender.Init(device, context);
	postprocess.Initialize( device );
	ActiveCam_->setFrom(noVec3(500.0f, 250.0f, 200.0f).ToFloatPtr());
	ActiveCam_->setTo(noVec3(0.0f, 100.0f, -100.0f).ToFloatPtr());
	ActiveCam_->computeModelView();
	
	
	postprocess.AfterReset(width, height);
}

void App::RenderLights(float fDeltaTime)
{
	litRender.Render(NULL, DefCam_);

	PostProcessor::Constants constants;
	constants.BloomThreshold = 3;
	constants.BloomMagnitude = 1.0f;
	constants.BloomBlurSigma = 0.8f;
	constants.Tau = 0.5f;
	constants.KeyValue = 0.115f;

	constants.TimeDelta = fDeltaTime;

	postprocess.SetConstants(constants);

	Direct3D11Renderer* pDx11Renderer = (Direct3D11Renderer*)renderer;


	postprocess.Render(context, litRender.GetHDRSrcSRV(), pDx11Renderer->getBackBuffer());
}