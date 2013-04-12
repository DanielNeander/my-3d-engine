#include "stdafx.h"
#include "GameApp.h"
#include <GameApp/Util/EffectUtil.h>
#include "Terrain.h"
#include "Dx11Util.h"
			
#include "Water/ocean_simulator.h"
#include "Terrain/SkyBox11.h"
#include "Terrain/AdaptiveModelDX11Render.h"
#include "Terrain/SNBTerrainMaterial.h"


// Iland11 start
float g_LightPosition[3]        = {-10000.0f,6500.0f,10000.0f};

float g_DynamicTessellationFactor = 50.0f;
float g_StaticTessellationFactor = 12.0f;


D3DXVECTOR3 g_EyePoints[6]=  {D3DXVECTOR3(365.0f,  3.0f, 166.0f),
	D3DXVECTOR3(478.0f, 15.0f, 248.0f),
	D3DXVECTOR3(430.0f,  3.0f, 249.0f),
	D3DXVECTOR3(513.0f, 10.0f, 277.0f),
	D3DXVECTOR3(303.0f,  3.0f, 459.0f),
	D3DXVECTOR3(20.0f,  12.0f, 477.0f),

};

D3DXVECTOR3 g_LookAtPoints[6]={D3DXVECTOR3(330.0f,-11.0f, 259.0f),
	D3DXVECTOR3(388.0f,-16.0f, 278.0f),
	D3DXVECTOR3(357.0f,-59.0f, 278.0f),
	D3DXVECTOR3(438.0f,-12.0f, 289.0f),
	D3DXVECTOR3(209.0f,-20.0f, 432.0f),
	D3DXVECTOR3(90.0f,  -7.0f, 408.0f),
};

int g_NumViews=6;

D3DXVECTOR3 g_EyePoint = g_EyePoints[0];
D3DXVECTOR3 g_LookAtPoint = g_LookAtPoints[0];
D3DXVECTOR3 g_EyePointspeed = D3DXVECTOR3(0,0,0);
D3DXVECTOR3 g_LookAtPointspeed = D3DXVECTOR3(0,0,0);

static ID3DX11Effect*      g_pEffect       = NULL;
CTerrain			g_Terrain;


bool	g_QueryStats = false;
bool g_RenderWireframe          = false;
bool g_UseDynamicLOD			= true;
bool g_RenderCaustics			= true;
bool g_FrustumCullInHS			= true;
bool g_CycleViewPoints			= false;

// Iland11 end

// Ocean11 Start 
// Ocean simulation variables
OceanSimulator* g_pOceanSimulator = NULL;

//bool g_RenderWireframe = false;
bool g_PauseSimulation = false;
int g_BufferType = 0;

// Skybox
ID3D11Texture2D* g_pSkyCubeMap = NULL;
ID3D11ShaderResourceView* g_pSRV_SkyCube = NULL;

static CSkybox11 g_Skybox;

void initRenderResource(const OceanParameter& ocean_param, ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
void cleanupRenderResource();
// Rendering routines
void renderShaded(const BaseCamera& camera, ID3D11ShaderResourceView* displacemnet_map, ID3D11ShaderResourceView* gradient_map, float time, ID3D11DeviceContext* pd3dContext);
void renderWireframe(const BaseCamera& camera, ID3D11ShaderResourceView* displacemnet_map, float time, ID3D11DeviceContext* pd3dContext);
// end


//--------------------------------------------------------------------------------------
// Create an OceanSimulator object at startup time
//--------------------------------------------------------------------------------------
void CreateOceanSimAndRender(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	// Create ocean simulating object
	// Ocean object
	OceanParameter ocean_param;

	// The size of displacement map. In this sample, it's fixed to 512.
	ocean_param.dmap_dim			= 512;
	// The side length (world space) of square patch
	ocean_param.patch_length		= 2000.0f;
	// Adjust this parameter to control the simulation speed
	ocean_param.time_scale			= 0.8f;
	// A scale to control the amplitude. Not the world space height
	ocean_param.wave_amplitude		= 0.35f;
	// 2D wind direction. No need to be normalized
	ocean_param.wind_dir			= D3DXVECTOR2(0.8f, 0.6f);
	// The bigger the wind speed, the larger scale of wave crest.
	// But the wave scale can be no larger than patch_length
	ocean_param.wind_speed			= 600.0f;
	// Damp out the components opposite to wind direction.
	// The smaller the value, the higher wind dependency
	ocean_param.wind_dependency		= 0.07f;
	// Control the scale of horizontal movement. Higher value creates
	// pointy crests.
	ocean_param.choppy_scale		= 1.3f;

	g_pOceanSimulator = new OceanSimulator(ocean_param, pd3dDevice);

	// Update the simulation for the first time.
	g_pOceanSimulator->updateDisplacementMap(0);

	

	// Init D3D11 resources for rendering
	initRenderResource(ocean_param, pd3dDevice, pBackBufferSurfaceDesc);
}


void App::LoadOcean()
{
	HRESULT hr;
	ID3D11Texture2D* pBackBuffer;
	hr = swapChain->GetBuffer( 0, __uuidof( *pBackBuffer ), ( LPVOID* )&pBackBuffer );
	DXGI_SURFACE_DESC pBBufferSurfaceDesc;
	ZeroMemory( &pBBufferSurfaceDesc, sizeof( DXGI_SURFACE_DESC ) );
	if( SUCCEEDED( hr ) )
	{
		D3D11_TEXTURE2D_DESC TexDesc;
		pBackBuffer->GetDesc( &TexDesc );
		pBBufferSurfaceDesc.Width = ( UINT )TexDesc.Width;
		pBBufferSurfaceDesc.Height = ( UINT )TexDesc.Height;
		pBBufferSurfaceDesc.Format = TexDesc.Format;
		pBBufferSurfaceDesc.SampleDesc = TexDesc.SampleDesc;
		SAFE_RELEASE( pBackBuffer );
	}

	noVec3 vecEye(1562.24f, 854.291f, -1224.99f);
	noVec3 vecAt (1562.91f, 854.113f, -1225.71f);
	ActiveCam_->setFrom(vecEye.ToFloatPtr());
	ActiveCam_->setTo(vecAt.ToFloatPtr());
	noVec3 up(0, 1, 0);
	ActiveCam_->setUp(up.ToFloatPtr());
	ActiveCam_->setNear(100.f);
	ActiveCam_->setFar(200000.0f);
	
	ActiveCam_->computeModelView();	
	ActiveCam_->ComputeProjection();
	

	// Create an OceanSimulator object and D3D11 rendering resources
	CreateOceanSimAndRender(device, &pBBufferSurfaceDesc);

	WCHAR strPath[MAX_PATH];
	FindMediaFileCch(strPath, MAX_PATH, L"Media\\OceanCS\\sky_cube.dds");
	D3DX11CreateShaderResourceViewFromFileW(device, strPath, NULL, NULL, &g_pSRV_SkyCube, NULL);
	assert(g_pSRV_SkyCube);

	g_pSRV_SkyCube->GetResource((ID3D11Resource**)&g_pSkyCubeMap);
	assert(g_pSkyCubeMap);

	g_Skybox.OnD3D11CreateDevice2(device, 50, g_pSkyCubeMap, g_pSRV_SkyCube);
	//g_Skybox.OnD3D11ResizedSwapChain(&pBBufferSurfaceDesc);
	
	ResetWater();
}

void App::UpdateOcean()
{	
	if (g_PauseSimulation == false)
	{		
		g_pOceanSimulator->updateDisplacementMap(GetSecTime());
	}
}

void App::RenderOcean()
{
	// Sky box rendering
	D3DXMATRIXA16 mView = D3DXMATRIX(1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,1) * D3DXMATRIX(ActiveCam_->getViewMatrix());
	D3DXMATRIXA16 mProj(ActiveCam_->getProjectionMatrix());	
	D3DXMATRIXA16 mWorldViewProjection = mView * mProj;
		
	if (!g_RenderWireframe)
		g_Skybox.D3D11Render( &mWorldViewProjection, context );

	// Time
	static double app_time = GetSecTime();
	static double app_prev_time = GetSecTime();
	if (g_PauseSimulation == false)
		app_time += GetSecTime() - app_prev_time;
	app_prev_time = GetSecTime();

	// Ocean rendering
	ID3D11ShaderResourceView* tex_displacement = g_pOceanSimulator->getD3D11DisplacementMap();
	ID3D11ShaderResourceView* tex_gradient = g_pOceanSimulator->getD3D11GradientMap();
	if (g_RenderWireframe)
		renderWireframe(*ActiveCam_, tex_displacement, (float)app_time, context);
	else
		renderShaded(*ActiveCam_, tex_displacement, tex_gradient, (float)app_time, context);
}

void App::LoadWater()
{
#if 1

	// MSAA should be applied not to the back buffer but to offscreen buffer
	// Nuking MSAA to 4x if it's >4x, to simplify manual depth resolve pass in fx file
	g_Terrain.MultiSampleCount = 1;//min(4,pDeviceSettings->d3d11.sd.SampleDesc.Count);
	g_Terrain.MultiSampleQuality = 0;

	// Create queries
	D3D11_QUERY_DESC queryDesc;
	queryDesc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
	queryDesc.MiscFlags = 0;
	device->CreateQuery(&queryDesc, &PipelineQuery_);

	//LoadEffectFromFile(device, "Data/Shaders/Island11.fx", &g_pEffect);
	
#if 0
	// Initialize terrain 
	g_Terrain.Initialize(device,g_pEffect);
	g_Terrain.LoadTextures();
	g_Terrain.ReCreateBuffers();
	ActiveCam_->setFrom((float*)&g_EyePoints[0]);
	ActiveCam_->setTo((float*)&g_LookAtPoints[0]);
	ActiveCam_->computeModelView();
#endif
	
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	CComPtr<ID3DBlob> pShader;
	CompileShaderFromFile("Data/Shaders/Water.fx", NULL, NULL,"fx_5_0", &pShader);
	D3DX11CreateEffectFromMemory(pShader->GetBufferPointer(), pShader->GetBufferSize(), dwShaderFlags, device, &g_pEffect);
	pShader.Detach();

	extern CAdaptiveModelDX11Render g_TerrainDX11Render;

	//g_Terrain.InitializeWater(device, g_TerrainDX11Render.m_pRenderEffect11);
	g_Terrain.InitializeWater(device, g_pEffect);
	g_Terrain.BackbufferWidth=width;
	g_Terrain.BackbufferHeight=height;
	g_Terrain.RecreateWaterBuffers();		
	g_TerrainDX11Render.SetWaterRSV(g_Terrain.water_normalmap_resourceSRV);
#endif
	//LoadOcean();
}

void App::ResetWater()
{
	HRESULT hr;
	ID3D11Texture2D* pBackBuffer;
	hr = swapChain->GetBuffer( 0, __uuidof( *pBackBuffer ), ( LPVOID* )&pBackBuffer );
	DXGI_SURFACE_DESC pBBufferSurfaceDesc;
	ZeroMemory( &pBBufferSurfaceDesc, sizeof( DXGI_SURFACE_DESC ) );
	if( SUCCEEDED( hr ) )
	{
		D3D11_TEXTURE2D_DESC TexDesc;
		pBackBuffer->GetDesc( &TexDesc );
		pBBufferSurfaceDesc.Width = ( UINT )TexDesc.Width;
		pBBufferSurfaceDesc.Height = ( UINT )TexDesc.Height;
		pBBufferSurfaceDesc.Format = TexDesc.Format;
		pBBufferSurfaceDesc.SampleDesc = TexDesc.SampleDesc;
		SAFE_RELEASE( pBackBuffer );
	}
	//g_Skybox.OnD3D11ResizedSwapChain(&pBBufferSurfaceDesc);


	g_Terrain.BackbufferWidth = width;
	g_Terrain.BackbufferHeight = height;
	//g_Terrain.ReCreateBuffers();
	g_Terrain.RecreateWaterBuffers();
}

void App::RenderPass_Main() {

	ID3D11DeviceContext* pContext;	
	D3D11_VIEWPORT main_Viewport;
	//float ClearColor[4] = { 0.8f, 0.8f, 1.0f, 1.0f };
	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	main_Viewport.Width=(float)width *main_buffer_size_multiplier;
	main_Viewport.Height=(float)height*main_buffer_size_multiplier;
	main_Viewport.MaxDepth=1;
	main_Viewport.MinDepth=0;
	main_Viewport.TopLeftX=0;
	main_Viewport.TopLeftY=0;

	D3D11Dev()->GetImmediateContext(&pContext);
	pContext->RSSetViewports(1,&main_Viewport);
	pContext->OMSetRenderTargets( 1, &main_color_resourceRTV, main_depth_resourceDSV);
	pContext->ClearRenderTargetView( main_color_resourceRTV, ClearColor );
	pContext->ClearDepthStencilView( main_depth_resourceDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );	
	g_Terrain.SetupNormalView(GetCamera());

	// Render terrain	
	pContext->Release();
}

void App::RenderWaterCaustics() {

	if (!g_Terrain.initialized) return;
	g_Terrain.RenderPass_Caustics();
}

void App::RenderPass_Reflection( float fDeltaTime )
{
	if (!g_Terrain.initialized) return;
	g_Terrain.RenderPass_Reflection();
	//RenderSky(fDeltaTime);
#if 1
	RenderSky(fDeltaTime);
	bool bWireframe = false;
	bool bShowBoundBoxes = false;
	// Render terrain
	extern CAdaptiveModelDX11Render g_TerrainDX11Render;
	g_TerrainDX11Render.UpdateTerrain( context, g_Terrain.ReflectCamPos, g_Terrain.ReflectViewProj );
	g_TerrainDX11Render.Render( context, g_Terrain.ReflectCamPos, g_Terrain.ReflectViewProj, bShowBoundBoxes, false, bWireframe, false);
#endif
}

void App::RenderWater(float fElapsedTime)
{
	//UpdateOcean();
	//RenderOcean();

#if 0
	float viewpoints_slide_speed_factor=0.01f;
	float viewpoints_slide_speed_damp=0.97f;
	float scaled_time=(1.0f+GetSecTime()/25.0f);
	int viewpoint_index = (int)(scaled_time)%6;
	D3DXVECTOR3 predicted_camera_position;
	float dh;

	if(g_CycleViewPoints)
	{
		g_EyePointspeed+=(g_EyePoints[viewpoint_index]-g_EyePoint)*viewpoints_slide_speed_factor;
		g_LookAtPointspeed+=(g_LookAtPoints[viewpoint_index]-g_LookAtPoint)*viewpoints_slide_speed_factor;

		predicted_camera_position=(g_EyePoint+g_EyePointspeed*fElapsedTime*15.0f)/terrain_geometry_scale;

		dh=predicted_camera_position.y-g_Terrain.height[((int)predicted_camera_position.x)%terrain_gridpoints]
		[((int)(terrain_gridpoints-predicted_camera_position.z))%terrain_gridpoints]-3.0f;
		if(dh<0)
		{
			g_EyePointspeed.y-=dh;
		}

		dh=predicted_camera_position.y-3.0f;
		if(dh<0)
		{
			g_EyePointspeed.y-=dh;
		}

		g_EyePointspeed*=viewpoints_slide_speed_damp;
		g_LookAtPointspeed*=viewpoints_slide_speed_damp;

		g_EyePoint+=g_EyePointspeed*fElapsedTime;
		g_LookAtPoint+=g_LookAtPointspeed*fElapsedTime;


		if(g_EyePoint.x<0)g_EyePoint.x=0;
		if(g_EyePoint.y<0)g_EyePoint.y=0;
		if(g_EyePoint.z<0)g_EyePoint.z=0;

		if(g_EyePoint.x>terrain_gridpoints*terrain_geometry_scale)g_EyePoint.x=terrain_gridpoints*terrain_geometry_scale;
		if(g_EyePoint.y>terrain_gridpoints*terrain_geometry_scale)g_EyePoint.y=terrain_gridpoints*terrain_geometry_scale;
		if(g_EyePoint.z>terrain_gridpoints*terrain_geometry_scale)g_EyePoint.z=terrain_gridpoints*terrain_geometry_scale;

		//g_Camera.SetViewParams(&g_EyePoint, &g_LookAtPoint);
		//g_Camera.FrameMove(fElapsedTime);
	}
	else
	{
		g_EyePointspeed=D3DXVECTOR3(0,0,0);
		g_LookAtPointspeed=D3DXVECTOR3(0,0,0);
	}
#if 0
	D3D11_VIEWPORT defaultViewport;
	UINT viewPortsNum = 1;
	context->RSGetViewports(&viewPortsNum, &defaultViewport);

	D3DXVECTOR2 WaterTexcoordShift(GetSecTime()*1.5f,GetSecTime()*0.75f);
	D3DXVECTOR2 ScreenSizeInv(1.0f/(width*main_buffer_size_multiplier),1.0f/(height*main_buffer_size_multiplier));

	g_pEffect->GetVariableByName("g_ZNear")->AsScalar()->SetFloat(scene_z_near);
	g_pEffect->GetVariableByName("g_ZFar")->AsScalar()->SetFloat(scene_z_far);
	g_pEffect->GetVariableByName("g_LightPosition")->AsVector()->SetFloatVector(g_LightPosition);
	g_pEffect->GetVariableByName("g_WaterBumpTexcoordShift")->AsVector()->SetFloatVector(WaterTexcoordShift);
	g_pEffect->GetVariableByName("g_ScreenSizeInv")->AsVector()->SetFloatVector(ScreenSizeInv);

	g_pEffect->GetVariableByName("g_DynamicTessFactor")->AsScalar()->SetFloat(g_DynamicTessellationFactor);
	g_pEffect->GetVariableByName("g_StaticTessFactor")->AsScalar()->SetFloat(g_StaticTessellationFactor);
	g_pEffect->GetVariableByName("g_UseDynamicLOD")->AsScalar()->SetFloat(g_UseDynamicLOD?1.0f:0.0f);
	g_pEffect->GetVariableByName("g_RenderCaustics")->AsScalar()->SetFloat(g_RenderCaustics?1.0f:0.0f);
	g_pEffect->GetVariableByName("g_FrustumCullInHS")->AsScalar()->SetFloat(g_FrustumCullInHS?1.0f:0.0f);
	
	g_Terrain.Render(ActiveCam_);
#endif
#endif
	if(g_QueryStats)
	{
		context->Begin(PipelineQuery_);
	}
	g_Terrain.RenderWater(ActiveCam_);

	if(g_QueryStats)
	{
		context->End(PipelineQuery_);
		while(S_OK != context->GetData(PipelineQuery_, &PipelineQueryData_, sizeof(PipelineQueryData_), 0));
	}

}
