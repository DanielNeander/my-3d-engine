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
#include "Utility.h"
#include "ShaderCompilation.h"
#include "PostProcessor.h"
#include "DeviceStates.h"

#include "Terrain/AdaptiveModelDX11Render.h"
#include "LIDRenderer.h"
#include "U2Light.h"


static ShadowMode gShadowMode = AppSettings::ShadowModeEnum;

uint App::LightTileSize = 16;  


struct sLight
{
	float3 Position;
	float3 Color;
	float Falloff;
};

const XMFLOAT3 SunColor = XMFLOAT3(10.0f, 8.0f, 5.0f);
const XMFLOAT3 SunDirection = Normalize(XMFLOAT3(0.2f, 0.977f, -0.4f));
static const float NearClip = 0.01f;
static const float FarClip = 1000.0f;
static const float ModelScale = 0.1f;
static const XMMATRIX ModelWorldMatrix = XMMatrixScaling(ModelScale, ModelScale, ModelScale) * XMMatrixRotationY(XM_PIDIV2);

CDXUTSDKMesh gSample;

extern CAdaptiveModelDX11Render g_TerrainDX11Render;
extern  std::vector<Mesh*>	gShadowReceivers;
extern SRenderingParams g_TerrainRenderParams;
extern U2Light gSun;


static D3DXMATRIX mCameraViewProjection;
static D3DXVECTOR3 cameraPos;

void App::LoadeIndexedDeferred()
{
	LIDRender_ = new LIDRenderer;

	postProcessor = new PostProcessor;
	csConstants = new ConstantBuffer<CSConstantsLID>;
	// Init the post processor
	postProcessor->Initialize(device);
	csConstants->Initialize(device);

	colorTargetMSAA = new RenderTarget2D;
	colorTarget = new RenderTarget2D;
	deferredOutputTarget = new RenderTarget2D;
	depthBuffer = new DepthStencilBuffer;
	normalsTarget = new RenderTarget2D;
	diffuseAlbedoTarget = new RenderTarget2D;
	specularAlbedoTarget = new RenderTarget2D;
	lightingTarget = new RenderTarget2D;
	lightsBuffer = new StructuredBuffer;
	lightIndicesBuffer = new RWBuffer;

	CHAR path[256];
	//GetCurrentDirectory(256, path);
	//SetCurrentDirectory("Media/Sponza");
	

	
	
	//SetCurrentDirectory(path);

	XMMATRIX id;
	id = XMMatrixIdentity();
	id = XMMatrixTranspose(id);
	
	LIDRender_->Initialize(device, context);

	//ActiveCam_->setFar(1000);

	for(UINT32 msaa = 0; msaa < MSAAModeGUI::NumValues; ++msaa)
	{
		for(UINT32 rmode = 0; rmode < RenderingModeGUI::NumValues; ++rmode)
		{
			for(UINT32 lmode = 0; lmode < NumLightsGUI::NumValues; ++lmode)
			{
				CompileOptions opts;
				opts.Add("MSAASamples_", MSAAModeGUI::NumSamples(msaa));
				opts.Add("IndexedDeferred_", rmode == RenderingModeGUI::IndexedDeferred);
				opts.Add("MaxLights", NumLightsGUI::NumLights(lmode));
				lightTilesCS[msaa][rmode][lmode].Attach(CompileCSFromFile(device, L"Data/Shaders/LightTiles.hlsl", "LightTiles", "cs_5_0", opts.Defines()));
			}
		}
	}

	// Init the GUI
	AppSettings::Initialize(device);

	
	gShadowMode = AppSettings::ShadowModeEnum;
	
	
	if (AppSettings::RendererTypeEnum == FORWARD_RENDER )
		return;
		
	if (gShadowMode == SHADOW_CASCADED)
	{
		LoadCascadedShadows();
		
		//gSample.Create(device, L"Sponza.sdkmesh");
		//gSample.Create( device, L"powerplant\\powerplant.sdkmesh" );
		////gSample.Create(device, L"ShadowColumns\\testscene.sdkmesh");

		//BoundingBox unionBB;
		//noVec3 vMeshMin;
		//noVec3 vMeshMax;

		//noVec3 sceneMin(FLT_MAX, FLT_MAX, FLT_MAX);
		//noVec3 sceneMax(FLT_MIN, FLT_MIN, FLT_MIN);
		//for( UINT i =0; i < gSample.GetNumMeshes( ); ++i ) 
		//{
		//	SDKMESH_MESH* msh = gSample.GetMesh( i );
		//	vMeshMin.Set(msh->BoundingBoxCenter.x - msh->BoundingBoxExtents.x,
		//		msh->BoundingBoxCenter.y - msh->BoundingBoxExtents.y,
		//		msh->BoundingBoxCenter.z - msh->BoundingBoxExtents.z );

		//	vMeshMax.Set(msh->BoundingBoxCenter.x + msh->BoundingBoxExtents.x,
		//		msh->BoundingBoxCenter.y + msh->BoundingBoxExtents.y,
		//		msh->BoundingBoxCenter.z + msh->BoundingBoxExtents.z );

		//	noVec3 temp = vMeshMin;
		//	temp.Min( sceneMin );
		//	sceneMin = temp;
		//	temp = vMeshMax;
		//	temp.Max( sceneMax );		 
		//	sceneMax = temp;
		//}
		//unionBB.m_vMin = sceneMin;
		//unionBB.m_vMax = sceneMax;	 

		//pCascaded_->m_vSceneAABBMin = noVec4(sceneMin,1);
		//pCascaded_->m_vSceneAABBMax = noVec4(sceneMax,1);
	}
	else {

	
		if (AppSettings::EditModeEnum == EDIT_TERRAIN)
		{
			LoadSNBTerrain();

				
			SPatchBoundingBox terrainBB;
			g_TerrainDX11Render.GetTerrainBoundingBox(terrainBB);
			uint32 terrainWidth = terrainBB.fMaxX - terrainBB.fMinX;
			uint32 terrainHeight = terrainBB.fMaxZ - terrainBB.fMinZ;			
			
			m_speedFlyStrafe = terrainWidth / 100;
			
			float fHeight = (g_TerrainRenderParams.m_fGlobalMinElevation + g_TerrainRenderParams.m_fGlobalMaxElevation) * 0.25;
		
			float centerW = terrainWidth * 0.5f;
			float centerH = terrainHeight * 0.5f;
			noVec3 boundCenter(centerW, centerH, fHeight);	
			D3DXVECTOR3 vecEye( boundCenter.x, boundCenter.y, boundCenter.z );
			D3DXVECTOR3 vecAt ( boundCenter.x + 1, boundCenter.y, boundCenter.z );
			//g_Camera.SetViewParams( &vecEye, &vecAt );
			//g_Camera.SetRotateButtons(true, false, false);
			{
				DefCam_->setFrom((float*)vecEye);
				DefCam_->setTo((float*)vecAt);
				DefCam_->setFOV(RAD2DEG(noMath::PI / 4));
				DefCam_->setNear(0.5f);
				DefCam_->setFar(50000.f);
				DefCam_->SetAspect(GetAspectRatio());
				DefCam_->computeModelView();	

				pShadowCam_->setFrom((float*)vecEye);
				pShadowCam_->setTo((float*)vecAt);
				pShadowCam_->setFar(50000.0f);
				pShadowCam_->setNear(0.5f);
				pShadowCam_->SetAspect(width / (FLOAT)height);	
				pShadowCam_->computeModelView();
				pShadowCam_->ComputeProjection();	

				ResizeSNBTerrain();

				//LoadEntity();

				gShadowReceivers.push_back(&g_TerrainDX11Render);
				g_TerrainDX11Render.m_bOnlyReceiveShadows = true;

				gSun.m_fFOV = 45.f;
				gSun.m_fNear = 0.1f;
				gSun.m_fFar = 50000.f;
				gSun.m_vTarget = boundCenter;
				gSun.m_vTarget.y = g_TerrainRenderParams.m_fGlobalMinElevation;
	
				float fMaxLength = 1.414214f * 3000;
				fHeight = (fMaxLength + 32.0f) / tanf(gSun.m_fFOV * 0.5f);		
				gSun.m_vLitPos = boundCenter;
				gSun.m_vLitPos.y = fHeight;
				gSun.m_vLitUp = noVec3(0, 0, 1);		
			}
		}
		else 
			LoadEntity();
	}

	// First Load
	OnSizeLID();
}

void App::OnSizeLID()
{
	ActiveCam_->SetAspect(GetAspectRatio());
	CreateRenderTargets();
	postProcessor->AfterReset(width, height);

}

void App::CreateRenderTargets()
{
	const UINT NumSamples = AppSettings::MSAAMode.NumSamples();
	const UINT32 NumLights = AppSettings::NumLights.NumLights();
		
	colorTargetMSAA->Initialize(device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT, 1, NumSamples, 0);
	colorTarget->Initialize(device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT, 1, 1, 0);
	
	UINT dtWidth = AppSettings::MSAAEnabled() ? width * 2 : width;
	UINT dtHeight = AppSettings::MSAAMode == MSAAModeGUI::MSAA4x ? height * 2 : height;	
	
	deferredOutputTarget->Initialize(device, dtWidth, dtHeight, DXGI_FORMAT_R16G16B16A16_FLOAT, 1, 1, 0, false, true);
	depthBuffer->Initialize(device, width, height, DXGI_FORMAT_D24_UNORM_S8_UINT, true, NumSamples, 0);

	normalsTarget->Initialize(device, width, height, DXGI_FORMAT_R10G10B10A2_UNORM, 1, NumSamples, 0);
	diffuseAlbedoTarget->Initialize(device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1, NumSamples, 0);
	specularAlbedoTarget->Initialize(device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1, NumSamples, 0);
	lightingTarget->Initialize(device, width, height, DXGI_FORMAT_R11G11B10_FLOAT, 1, NumSamples, 0);

	UINT32 numTilesX = DispatchSize(LightTileSize, width);
	UINT32 numTilesY = DispatchSize(LightTileSize, height);
	UINT32 numElements = numTilesX * numTilesY * NumLights;

	DXGI_FORMAT format = DXGI_FORMAT_R8_UINT;
	UINT32 stride = sizeof(UINT8);
	if(NumLights >= 255)
	{
		format = DXGI_FORMAT_R16_UINT;
		stride = sizeof(UINT16);
	}

	lightIndicesBuffer->Initialize(device, format, stride, numElements, FALSE);

	CreateLightBuffer();
}

void App::CreateLightBuffer()
{	

	srand(1);

	const UINT32 NumLights = AppSettings::NumLights.NumLights();

	SPatchBoundingBox terrainBB;
	g_TerrainDX11Render.GetTerrainBoundingBox(terrainBB);
	uint32 terrainWidth = terrainBB.fMaxX - terrainBB.fMinX;
	uint32 terrainHeight = terrainBB.fMaxZ - terrainBB.fMinZ;			
	
	std::vector<sLight> lights;
	lights.resize(NumLights);
	for(UINT32 i = 0; i < NumLights; ++i)
	{
		sLight& light = lights[i];
		light.Position.x = RandFloat() * terrainWidth - 7.5f;
		light.Position.y = RandFloat() * terrainHeight;
		light.Position.z = RandFloat() * terrainWidth - 10.0f;
		light.Color.x = RandFloat();
		light.Color.y = RandFloat();
		light.Color.z = RandFloat();
		light.Falloff = RandFloat() * 5.0f + 1.5f;
	}
	lightsBuffer->Initialize(device, sizeof(sLight), NumLights, false, false, false, &lights[0]);
}

void App::RenderLID(float fDeltaTime)
{	
	
	BaseCamera* pCam = GetApp()->ActiveCam_;
	D3DXMATRIX mProj;
	D3DXMATRIX mView;
	const unsigned int size16 = sizeof(float) * 16;		 
	memcpy(&mView, pCam->getViewMatrix(), size16);
	memcpy(&mProj, pCam->getProjectionMatrix(), size16);
	mCameraViewProjection = mView * mProj;

	//D3DXVECTOR3 SunLightDir = *g_LightCamera.GetWorldAhead();
	//g_vDirectionOnSun = -SunLightDir;
	cameraPos = D3DXVECTOR3(pCam->GetFrom().ToFloatPtr());	
	g_TerrainDX11Render.UpdateTerrain( context, cameraPos, mCameraViewProjection );
	
		
	// Mesh 클래스 형	
	if (gShadowMode == SHADOW_CASCADED)
	{
		if (terrainload_)
		{		
			pCascaded_->m_vSceneAABBMin = noVec4(g_TerrainDX11Render.m_AABB.m_vMin,1);
			pCascaded_->m_vSceneAABBMax = noVec4(g_TerrainDX11Render.m_AABB.m_vMax,1);
		}
		pCascaded_->InitFrame( device );

		
		pCascaded_->EnableRendering();
		pCascaded_->RenderShadowsForAllCascades();		
		pCascaded_->RenderShadow2(&gSample);
		pCascaded_->DisableRendering();
	}
	if (gShadowMode == SHADOW_SSAO)  {
		SSAO_ShadowPass(fDeltaTime);
	}
	else 
		PSSM_ShadowPass(fDeltaTime);

	RenderDeferredScene(fDeltaTime);

	if(AppSettings::MSAAEnabled())
	{
		if(AppSettings::RenderingMode == RenderingModeGUI::IndexedDeferred)
			context->ResolveSubresource(colorTarget->Texture, 0, colorTargetMSAA->Texture, 0, colorTargetMSAA->Format);
		else
			postProcessor->Downscale(context, deferredOutputTarget->SRView, colorTarget->RTView);
	}
	else if(AppSettings::RenderingMode == RenderingModeGUI::TiledDeferred)
		context->CopyResource(colorTarget->Texture, deferredOutputTarget->Texture);
		
	// Kick off post-processing
	//D3DPERF_BeginEvent(0xFFFFFFFF, L"Post Processing");
	PostProcessor::Constants constants;
	constants.BloomThreshold = 3;
	constants.BloomMagnitude = 1.0f;
	constants.BloomBlurSigma = 0.8f;
	constants.Tau = 0.5f;
	constants.KeyValue = 0.115f;
	
	constants.TimeDelta = fDeltaTime;

	postProcessor->SetConstants(constants);

	Direct3D11Renderer* pDx11Renderer = (Direct3D11Renderer*)renderer;
	postProcessor->Render(context, colorTarget->SRView, pDx11Renderer->getBackBuffer());
	//D3DPERF_EndEvent();		
}

void App::ComputeLightTiles()
{
	//PIXEvent _event(L"Light Tile Computation");
	//ProfileBlock profileBlock(L"Light Tile Computation");
	XMMATRIX proj(ActiveCam_->getProjectionMatrix());
	XMMATRIX view(ActiveCam_->getViewMatrix());
		

	UINT32 numTilesX = DispatchSize(LightTileSize, depthBuffer->Width);
	UINT32 numTilesY = DispatchSize(LightTileSize, depthBuffer->Height);

	memcpy(&csConstants->Data.View, &XMMatrixTranspose(view), SIZE16);	
	memcpy(&csConstants->Data.Projection, &XMMatrixTranspose(proj), SIZE16);
	 

	XMVECTOR det;
	memcpy(&csConstants->Data.InvViewProjection, &XMMatrixTranspose(XMMatrixInverse(&det, view * proj)), SIZE16);
	
	float eyePos[3];
	ActiveCam_->getFrom(eyePos);
	csConstants->Data.CameraPosWS = XMFLOAT3(eyePos);

	csConstants->Data.CameraClipPlanes.x = ActiveCam_->GetNear();
	csConstants->Data.CameraClipPlanes.y = ActiveCam_->GetFar();
	csConstants->Data.NumTilesX = numTilesX;
	csConstants->Data.NumTilesY = numTilesY;
	csConstants->Data.DisplaySizeX = depthBuffer->Width;
	csConstants->Data.DisplaySizeY = depthBuffer->Height;
	csConstants->ApplyChanges(context);
	csConstants->SetCS(context, 0);

	ID3D11ShaderResourceView* srvs[6] = { NULL };
	srvs[0] = lightsBuffer->SRView;
	srvs[1] = depthBuffer->SRView;
	if(AppSettings::RenderingMode == RenderingModeGUI::TiledDeferred)
	{
		srvs[2] = normalsTarget->SRView;
		srvs[3] = diffuseAlbedoTarget->SRView;
		srvs[4] = specularAlbedoTarget->SRView;
		srvs[5] = lightingTarget->SRView;
		SetCSOutputs(context, deferredOutputTarget->UAView);
	}
	else
		SetCSOutputs(context, lightIndicesBuffer->UAView);

	context->CSSetShaderResources(0, 6, srvs);	
	SetCSShader(context, lightTilesCS[AppSettings::MSAAMode][AppSettings::RenderingMode][AppSettings::NumLights]);

	context->Dispatch(numTilesX, numTilesY, 1);

	ClearCSOutputs(context);

	for(UINT32 i = 0; i < 6; ++i)
		srvs[i] = NULL;
	context->CSSetShaderResources(0, 6, srvs);
}

static void RenderTerrain(bool bOnlyZ)
{
	

	bool bWireframe = false;
	bool bShowBoundBoxes = false;
	// Render terrain
	g_TerrainDX11Render.Render( D3D11Context(), cameraPos, mCameraViewProjection, bShowBoundBoxes, false, bWireframe, bOnlyZ);
}

void App::RenderDeferredScene( float fDeltaTime )
{
	//PIXEvent event(L"Main Pass");
	SetDefaultRenderTarget();
		
	
	ID3D11RenderTargetView* renderTargets[4] = { NULL };
	ID3D11DepthStencilView* ds = depthBuffer->DSView;
	if(AppSettings::RenderingMode == RenderingModeGUI::TiledDeferred)
	{
		renderTargets[0] = normalsTarget->RTView;
		renderTargets[1] = diffuseAlbedoTarget->RTView;
		renderTargets[2] = specularAlbedoTarget->RTView;
		renderTargets[3] = lightingTarget->RTView;
		for(UINT i = 0; i < 4; ++i)
			context->ClearRenderTargetView(renderTargets[i], reinterpret_cast<float*>(&XMFLOAT4(0, 0, 0, 0)));
	}

	ID3D11RenderTargetView* colorRT = AppSettings::MSAAEnabled() ? colorTargetMSAA->RTView : colorTarget->RTView;
	
	context->ClearRenderTargetView(colorRT, reinterpret_cast<float*>(&XMFLOAT4(0, 0, 0, 0)));

	context->OMSetRenderTargets(4, renderTargets, ds);

	context->ClearDepthStencilView(ds, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

	LIDRender_->Setup(SunDirection, SunColor, ModelWorldMatrix);

	if( AppSettings::RenderingMode == RenderingModeGUI::IndexedDeferred)
	{
		if (gSample.IsLoaded())
			LIDRender_->RenderDepth2(&gSample, ActiveCam_, context, ModelWorldMatrix);
	//	meshRenderer.RenderDepth(context, camera, ModelWorldMatrix);		
		
		switch (AppSettings::EditModeEnum)
		{
		case EDIT_TEST_MODE:
			//if (gShadowMode == SHADOW_PSSM) 		{
				LIDRender_->BeginRender(context);
				//RenderPSSMScenes(fDeltaTime);						
				for (size_t i =0; i < m_ActorRoot->childNodes_.size(); ++i)
				{
					M2Mesh* pMesh = (M2Mesh*)m_ActorRoot->childNodes_[i];
					noMat4 tm(pMesh->worldTM_.rotate_ * pMesh->worldTM_.scale_, pMesh->worldTM_.trans_);
					tm.TransposeSelf();	
					pMesh->m2_->DrawZOnly(tm, ViewMat(), ProjMat());
				}
			//}
			break;
		case EDIT_TERRAIN:
			RenderTerrain(true);
			break;
		}
		
						
		
		
	}


	if(AppSettings::RenderingMode == RenderingModeGUI::TiledDeferred)
	{
		if (gSample.IsLoaded())
			LIDRender_->RenderGBuffer2(&gSample, ActiveCam_, context, ModelWorldMatrix);
		//meshRenderer.RenderGBuffer(context, camera, ModelWorldMatrix);

		renderTargets[0] = lightingTarget->RTView;
		renderTargets[1] = renderTargets[2] = renderTargets[3] = NULL;
		context->OMSetRenderTargets(4, renderTargets, depthBuffer->DSView);

		//skybox.RenderSky(context, SunDirection, true, camera.ViewMatrix(), camera.ProjectionMatrix());
	}

	for(UINT i = 0; i < 4; ++i)
		renderTargets[i] = NULL;
	context->OMSetRenderTargets(4, renderTargets, NULL);

	ComputeLightTiles();
		
	if(AppSettings::RenderingMode == RenderingModeGUI::IndexedDeferred)
	{
		renderTargets[0] = colorRT;
		context->OMSetRenderTargets(1, renderTargets, ds);

		UINT32 numTilesX = DispatchSize(LightTileSize, colorTarget->Width);
		UINT32 numTilesY = DispatchSize(LightTileSize, colorTarget->Height);

		if (gSample.IsLoaded())
		{
			//m_pLIDRender->BeginRender(context);
			LIDRender_->Render2(&gSample, ActiveCam_, context, ModelWorldMatrix, lightsBuffer->SRView, lightIndicesBuffer->SRView, numTilesX, numTilesY);
		}

		if (gShadowMode == SHADOW_CASCADED) 		{					
			LIDRender_->BeginRender(context);	
			pCascaded_->RenderScene2(ActiveCam_, &gSample);
			
			LIDRender_->BeginRender(context);		
			for (size_t i = 0; i < gShadowReceivers.size(); ++i)
				pCascaded_->RenderScene(DefCam_, gShadowReceivers[i]);
		}
		
		
		switch (AppSettings::EditModeEnum)
		{
		case EDIT_TEST_MODE:
			//if (gShadowMode == SHADOW_PSSM) 		{
				LIDRender_->BeginRender(context);
				RenderPSSMScenes(fDeltaTime);		
			//}
			break;
		case EDIT_TERRAIN:
			RenderTerrain(false);
			//LIDRender_->BeginRender(context);
			RenderMeshes(false);
			//RenderPSSMScenes(fDeltaTime);
			
			break;
		}		
		//skybox.RenderSky(context, SunDirection, true, camera.ViewMatrix(), camera.ProjectionMatrix());*/
	}
}
