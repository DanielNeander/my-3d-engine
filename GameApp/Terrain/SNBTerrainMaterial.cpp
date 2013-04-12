#include "stdafx.h"
#include "SNBTerrainMaterial.h"
#include "GameApp/Texture/Clipmap.h"
#include "GameApp/Dx11Util.h"
#include "GameApp/Util/EffectUtil.h"
#include "GameApp/GameApp.h"
#include "ElevationDataSource.h"
#include "GameApp/AppSettings.h"
#include "AdaptiveModelDX11Render.h"
#include "SNBTerrainConfigFile.h"
#include "GameApp/U2Light.h"

static float g_DynamicTessellationFactor = 50.0f;
static float g_StaticTessellationFactor = 12.0f;
static bool g_UseDynamicLOD			= true;
static bool g_RenderCaustics			= true;

SNBTerrainMaterial::SNBTerrainMaterial()
:Initialized(false),
layerdef_texture(NULL), 
Effect(NULL)
{
	memset(clipmap, 0, sizeof(clipmap));
	DiffuseMap1SRV = DiffuseMap2SRV = DiffuseMap3SRV = DiffuseMap4SRV = 0;
	BumpMap1SRV = BumpMap2SRV = BumpMap3SRV = BumpMap4SRV = 0;
	SlopeTexSRV = 0;
	terrain_geometry_scale = 1.0f;
	TerrainFarRange = terrain_geometry_scale * g_iPatchSize;
}

SNBTerrainMaterial::~SNBTerrainMaterial()
{
	Destory();
}

void SNBTerrainMaterial::Destory()
{
	for (int i=0; i < MAX_TEX_LAYER; i++)  {
		SAFE_DELETE(clipmap[i]);
	}
	DiffuseMap1SRV.Detach();
	DiffuseMap2SRV.Detach();
	DiffuseMap3SRV.Detach();
	DiffuseMap4SRV.Detach();

	BumpMap1SRV.Detach();
	BumpMap2SRV.Detach();
	BumpMap3SRV.Detach();
	BumpMap4SRV.Detach();	

	SlopeTexSRV.Detach();

	SAFE_RELEASE(Effect);
}

bool SNBTerrainMaterial::Init( ID3DX11Effect* effect ) {
	if (Initialized) return false;

	Effect = effect;
	Effect->AddRef();
			
	
	if (SplattSrcMap[0].BindShaderMapName.Length() > 0 ) {
			clipmap[0] = new Clipmap;		
			clipmap[0]->ClipmapEffect = effect;
	}	

	TerrainFarRange = g_iPatchSize;

	Initialized = true;
}

void SNBTerrainMaterial::Update(float fDeltaTime) {
	
	noVec3 camPos = GetApp()->ActiveCam_->GetFrom();

	SPatchBoundingBox terrainBB;
	extern CAdaptiveModelDX11Render g_TerrainDX11Render;
	g_TerrainDX11Render.GetTerrainBoundingBox(terrainBB);
	uint32 terrainWidth = terrainBB.fMaxX - terrainBB.fMinX;
	uint32 terrainHeight = terrainBB.fMaxZ - terrainBB.fMinZ;			

	extern int g_iPatchSize;
	//for (int i=0; i < MAX_TEX_LAYER; i++)  {
		if (clipmap[0]) {
			clipmap[0]->recenter(camPos.x / terrainWidth,camPos.z / terrainHeight);
			clipmap[0]->updateEffectParams();
		}
	//}	
}

void SNBTerrainMaterial::CommitChanges()
{	
	if (SplattSrcMap[0].BindShaderMapName.Length() > 0 ) {
		clipmap[0]->SrcTexutreName = SplattSrcMap[0].BindShaderMapName.c_str();
		clipmap[0]->Init();
		clipmap[0]->alloc(D3D11Dev());
	}

	if (SplattSrcMap[1].BindShaderMapName.Length() > 0 ) {
		loadTextureFromFile(SplattSrcMap[1].BindShaderMapName.c_str(), SplattSrcMap[1].BindShaderVarName.c_str(), D3D11Dev(), Effect, &DiffuseMap1SRV);
		DiffuseMap1Var = Effect->GetVariableByName(SplattSrcMap[1].BindShaderVarName.c_str())->AsShaderResource();
	}
	if (SplattSrcMap[2].BindShaderMapName.Length() > 0 ) {
		loadTextureFromFile(SplattSrcMap[2].BindShaderMapName.c_str(), SplattSrcMap[2].BindShaderVarName.c_str(), D3D11Dev(), Effect, &DiffuseMap2SRV);		
		DiffuseMap2Var = Effect->GetVariableByName(SplattSrcMap[2].BindShaderVarName.c_str())->AsShaderResource();
	}

	if (SplattSrcMap[3].BindShaderMapName.Length() > 0 ) {
		loadTextureFromFile(SplattSrcMap[3].BindShaderMapName.c_str(), SplattSrcMap[3].BindShaderVarName.c_str(), D3D11Dev(), Effect, &DiffuseMap3SRV);		
		DiffuseMap3Var = Effect->GetVariableByName(SplattSrcMap[3].BindShaderVarName.c_str())->AsShaderResource();
	}

	if (SplattBumpMap[0].BindShaderMapName.Length() > 0) {
		loadTextureFromFile(SplattBumpMap[0].BindShaderMapName.c_str(), SplattBumpMap[0].BindShaderVarName.c_str(), D3D11Dev(), Effect, &BumpMap1SRV);		
		BumpMap1Var= Effect->GetVariableByName(SplattBumpMap[0].BindShaderVarName.c_str())->AsShaderResource();		
	}
	if (SplattBumpMap[1].BindShaderMapName.Length() > 0) {
		loadTextureFromFile(SplattBumpMap[1].BindShaderMapName.c_str(), SplattBumpMap[1].BindShaderVarName.c_str(), D3D11Dev(), Effect, &BumpMap2SRV);		
		BumpMap2Var= Effect->GetVariableByName(SplattBumpMap[1].BindShaderVarName.c_str())->AsShaderResource();
	}
	if (SplattBumpMap[2].BindShaderMapName.Length() > 0) {
		loadTextureFromFile(SplattBumpMap[2].BindShaderMapName.c_str(), SplattBumpMap[2].BindShaderVarName.c_str(), D3D11Dev(), Effect, &BumpMap3SRV);		
		BumpMap3Var= Effect->GetVariableByName(SplattBumpMap[2].BindShaderVarName.c_str())->AsShaderResource();
	}
	if (SplattBumpMap[3].BindShaderMapName.Length() > 0) {
		loadTextureFromFile(SplattBumpMap[3].BindShaderMapName.c_str(), SplattBumpMap[3].BindShaderVarName.c_str(), D3D11Dev(), Effect, &BumpMap4SRV);		
		BumpMap4Var= Effect->GetVariableByName(SplattBumpMap[3].BindShaderVarName.c_str())->AsShaderResource();
	}

	if (SlopeMapName.BindShaderMapName.Length() > 0) {
		loadTextureFromFile(SlopeMapName.BindShaderMapName.c_str(), SlopeMapName.BindShaderVarName.c_str(), D3D11Dev(), Effect, &SlopeTexSRV);		
		SlopeMapVar= Effect->GetVariableByName(SlopeMapName.BindShaderVarName.c_str())->AsShaderResource();
	}

	
}

void SNBTerrainMaterial::CreateDepthMap(byte * depth_shadow_map_texture_pixels) {
#if 0
	HRESULT result;
	D3D11_SUBRESOURCE_DATA subresource_data;
	D3D11_TEXTURE2D_DESC tex_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC textureSRV_desc; 

	subresource_data.pSysMem = depth_shadow_map_texture_pixels;
	subresource_data.SysMemPitch = terrain_depth_shadow_map_texture_size*4;
	subresource_data.SysMemSlicePitch = 0;

	tex_desc.Width = terrain_depth_shadow_map_texture_size;
	tex_desc.Height = terrain_depth_shadow_map_texture_size;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1; 
	tex_desc.SampleDesc.Quality = 0; 
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;
	result=D3D11Dev()->CreateTexture2D(&tex_desc,&subresource_data,&depthmap_texture);

	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	textureSRV_desc.Format=tex_desc.Format;
	textureSRV_desc.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels=tex_desc.MipLevels;
	D3D11Dev()->CreateShaderResourceView(depthmap_texture,&textureSRV_desc,&depthmap_texSRV);
#endif
}

void SNBTerrainMaterial::CreateOpacityMap(const BYTE* layerdef_map_texture_pixels) {
		
	//layerdef_map_texture_pixels = _layerdef_map_texture_pixels;
#if 0
	HRESULT result;
	D3D11_SUBRESOURCE_DATA subresource_data;
	D3D11_TEXTURE2D_DESC tex_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC textureSRV_desc; 

	subresource_data.pSysMem = layerdef_map_texture_pixels;
	subresource_data.SysMemPitch = terrain_layerdef_map_texture_size*4;
	subresource_data.SysMemSlicePitch = 0;

	tex_desc.Width = terrain_layerdef_map_texture_size;
	tex_desc.Height = terrain_layerdef_map_texture_size;
	tex_desc.MipLevels = 1;
	tex_desc.ArraySize = 1;
	tex_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count = 1; 
	tex_desc.SampleDesc.Quality = 0; 
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = 0;
	result=D3D11Dev()->CreateTexture2D(&tex_desc,&subresource_data,&layerdef_texture);

	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	textureSRV_desc.Format=tex_desc.Format;
	textureSRV_desc.ViewDimension=D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels=tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip=0;
	D3D11Dev()->CreateShaderResourceView(layerdef_texture,&textureSRV_desc,&layerdef_textureSRV);
#else
	loadTextureFromFile("Data/OpacityMap.dds", "g_LayerdefTexture", D3D11Dev(), Effect, &layerdef_textureSRV);
#endif
}

void SNBTerrainMaterial::SetupLightView(BaseCamera *cam)
{

	D3DXVECTOR3 EyePoint= D3DXVECTOR3(-10000.0f,6500.0f,10000.0f);
	D3DXVECTOR3 LookAtPoint = D3DXVECTOR3(TerrainFarRange/2.0f,0.0f,TerrainFarRange/2.0f);
	D3DXVECTOR3 lookUp = D3DXVECTOR3(0,1,0);
	D3DXVECTOR3 cameraPosition(cam->GetFrom().ToFloatPtr());

	float nr, fr;
	nr=sqrt(EyePoint.x*EyePoint.x+EyePoint.y*EyePoint.y+EyePoint.z*EyePoint.z)-TerrainFarRange*0.7f;
	fr=sqrt(EyePoint.x*EyePoint.x+EyePoint.y*EyePoint.y+EyePoint.z*EyePoint.z)+TerrainFarRange*0.7f;

	D3DXMATRIX mView = *D3DXMatrixLookAtLH(&mView,&EyePoint,&LookAtPoint,&lookUp);
	D3DXMATRIX mProjMatrix = *D3DXMatrixOrthoLH(&mProjMatrix,TerrainFarRange*1.5,TerrainFarRange,nr,fr);
	D3DXMATRIX mViewProj = mView * mProjMatrix;
	D3DXMATRIX mViewProjInv;
	D3DXMatrixInverse(&mViewProjInv, NULL, &mViewProj);

	Effect->GetVariableByName("g_ModelViewProjectionMatrix")->AsMatrix()->SetMatrix(mViewProj);
	Effect->GetVariableByName("g_LightModelViewProjectionMatrix")->AsMatrix()->SetMatrix(mViewProj);
	Effect->GetVariableByName("g_LightModelViewProjectionMatrixInv")->AsMatrix()->SetMatrix(mViewProjInv);
	Effect->GetVariableByName("g_CameraPosition")->AsVector()->SetFloatVector(cameraPosition);

	noVec3 dir = cam->GetTo() - cam->GetFrom();
	D3DXVECTOR3 direction(dir.ToFloatPtr());
	D3DXVECTOR3 normalized_direction = *D3DXVec3Normalize(&normalized_direction,&direction);
	Effect->GetVariableByName("g_CameraDirection")->AsVector()->SetFloatVector(normalized_direction);
	Effect->GetVariableByName("g_HalfSpaceCullSign")->AsScalar()->SetFloat(1.0);
	Effect->GetVariableByName("g_HalfSpaceCullPosition")->AsScalar()->SetFloat(terrain_minheight*2);

}

void SNBTerrainMaterial::SetupNormalView(BaseCamera *cam) {

	D3DXVECTOR3 EyePoint(cam->GetFrom().ToFloatPtr());
	D3DXVECTOR3 LookAtPoint(cam->GetTo().ToFloatPtr());

	D3DXMATRIX mView(cam->getViewMatrix2());
	D3DXMATRIX mProjMatrix(cam->getProjectionMatrix2());
	D3DXMATRIX mViewProj = mView * mProjMatrix;
	D3DXMATRIX mViewProjInv;
	D3DXMatrixInverse(&mViewProjInv, NULL, &mViewProj);
	D3DXVECTOR3 cameraPosition(cam->GetFrom().ToFloatPtr());

	Effect->GetVariableByName("View")->AsMatrix()->SetMatrix(mView);
	Effect->GetVariableByName("g_mWorldViewProj")->AsMatrix()->SetMatrix(mViewProj);
	Effect->GetVariableByName("g_ModelViewProjectionMatrixInv")->AsMatrix()->SetMatrix(mViewProjInv);
	Effect->GetVariableByName("g_CameraPosition")->AsVector()->SetFloatVector(cameraPosition);
	D3DXVECTOR3 direction = LookAtPoint - EyePoint;
	D3DXVECTOR3 normalized_direction = *D3DXVec3Normalize(&normalized_direction,&direction);
	Effect->GetVariableByName("g_CameraDirection")->AsVector()->SetFloatVector(normalized_direction);
	Effect->GetVariableByName("g_HalfSpaceCullSign")->AsScalar()->SetFloat(1.0);
	Effect->GetVariableByName("g_HalfSpaceCullPosition")->AsScalar()->SetFloat(terrain_minheight*2);			
}

void SNBTerrainMaterial::EndPass_Main() {
	ID3DX11EffectShaderResourceVariable* tex_variable;
	tex_variable=Effect->GetVariableByName("g_WaterNormalMapTexture")->AsShaderResource();
	tex_variable->SetResource(NULL);
	tex_variable=Effect->GetVariableByName("g_DepthTexture")->AsShaderResource();
	tex_variable->SetResource(NULL);
	//Effect->GetVariableByName("g_LayerdefTexture")->AsShaderResource()->SetResource(NULL);
}


void SNBTerrainMaterial::RenderPass_Main() {
		
	ID3D11DeviceContext* pContext;	
	D3D11Dev()->GetImmediateContext(&pContext);
#if 0	
	D3D11_VIEWPORT main_Viewport;
	float ClearColor[4] = { 0.8f, 0.8f, 1.0f, 1.0f };
		
	main_Viewport.Width=(float)BackbufferWidth*main_buffer_size_multiplier;
	main_Viewport.Height=(float)BackbufferHeight*main_buffer_size_multiplier;
	main_Viewport.MaxDepth=1;
	main_Viewport.MinDepth=0;
	main_Viewport.TopLeftX=0;
	main_Viewport.TopLeftY=0;

	pContext->RSSetViewports(1,&main_Viewport);
	pContext->OMSetRenderTargets( 1, &GetApp()->main_color_resourceRTV, GetApp()->main_depth_resourceDSV);
	pContext->ClearRenderTargetView( GetApp()->main_color_resourceRTV, ClearColor );
	pContext->ClearDepthStencilView( GetApp()->main_depth_resourceDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );
#endif
	SetupNormalView(GetCamera());
	Effect->GetVariableByName("g_WaterNormalMapTexture")->AsShaderResource()->SetResource(water_normalmap_resourceSRV);
	Effect->GetVariableByName("g_TerrainBeingRendered")->AsScalar()->SetFloat(1.0f);
	Effect->GetVariableByName("g_SkipCausticsCalculation")->AsScalar()->SetFloat(0.0f);
	pContext->Release();
}


void SNBTerrainMaterial::EndPass() {
	Effect->GetTechniqueByName("Default")->GetPassByIndex(0)->Apply(0, D3D11Context());
}

void SNBTerrainMaterial::RenderPass_Shadow() {	
	ID3D11DeviceContext* pContext;	
	Direct3D11Renderer* pDX11R = (Direct3D11Renderer*)GetRenderer();
	ID3D11RenderTargetView *colorBuffer = pDX11R->getBackBuffer();
	ID3D11DepthStencilView  *backBuffer = pDX11R->getDepthBuffer();
	D3D11_VIEWPORT currentViewport;
	D3D11_VIEWPORT shadowmap_resource_viewport;
	UINT cRT = 1;

	D3D11Dev()->GetImmediateContext(&pContext);


	shadowmap_resource_viewport.Width=shadowmap_resource_buffer_size_xy;
	shadowmap_resource_viewport.Height=shadowmap_resource_buffer_size_xy;
	shadowmap_resource_viewport.MaxDepth=1;
	shadowmap_resource_viewport.MinDepth=0;
	shadowmap_resource_viewport.TopLeftX=0;
	shadowmap_resource_viewport.TopLeftY=0;

	//saving scene color buffer and back buffer to constants
	pContext->RSGetViewports( &cRT, &currentViewport);
	pContext->OMGetRenderTargets( 1, &colorBuffer, &backBuffer );

	// selecting shadowmap_resource rendertarget

	pContext->RSSetViewports(1,&shadowmap_resource_viewport);
	pContext->OMSetRenderTargets( 0, NULL, shadowmap_resourceDSV);
	pContext->ClearDepthStencilView( shadowmap_resourceDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	SetupLightView(GetCamera());

	Effect->GetVariableByName("g_TerrainBeingRendered")->AsScalar()->SetFloat(1.0f);
	Effect->GetVariableByName("g_SkipCausticsCalculation")->AsScalar()->SetFloat(1.0f);
}

void SNBTerrainMaterial::RenderPass_Reflection() {
	UINT stride=sizeof(float)*6;
	UINT offset=0;
	ID3D11DeviceContext* pContext;	
	D3D11_VIEWPORT reflection_Viewport;
	float RefractionClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	
	reflection_Viewport.Width=(float)BackbufferWidth*reflection_buffer_size_multiplier;
	reflection_Viewport.Height=(float)BackbufferHeight*reflection_buffer_size_multiplier;
	reflection_Viewport.MaxDepth=1;
	reflection_Viewport.MinDepth=0;
	reflection_Viewport.TopLeftX=0;
	reflection_Viewport.TopLeftY=0;

	pContext->RSSetViewports(1,&reflection_Viewport);
	pContext->OMSetRenderTargets( 1, &reflection_color_resourceRTV, reflection_depth_resourceDSV);
	pContext->ClearRenderTargetView( reflection_color_resourceRTV, RefractionClearColor );
	pContext->ClearDepthStencilView( reflection_depth_resourceDSV, D3D11_CLEAR_DEPTH, 1.0, 0 );

	//SetupReflectionView(GetCamera());
}

void SNBTerrainMaterial::RenderPass_Reraction() {
	UINT stride=sizeof(float)*4;
	UINT offset=0;
	ID3D11DeviceContext* pContext;	
	ID3DX11EffectShaderResourceVariable* tex_variable;

	D3D11Dev()->GetImmediateContext(&pContext);

	// drawing terrain to reflection RT
	tex_variable=Effect->GetVariableByName("g_DepthTexture")->AsShaderResource();
	tex_variable->SetResource(shadowmap_resourceSRV);
	Effect->GetVariableByName("g_SkipCausticsCalculation")->AsScalar()->SetFloat(1.0f);
	/*pContext->IASetInputLayout(heightfield_inputlayout);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST);
	pEffect->GetTechniqueByName("RenderHeightfield")->GetPassByIndex(g_RenderWireframe)->Apply(0, pContext);	
	//pContext->IASetVertexBuffers(0,1,&heightfield_vertexbuffer,&stride,&offset);
	//pContext->Draw(terrain_numpatches_1d*terrain_numpatches_1d, 0);
	*/
}

void SNBTerrainMaterial::RenderPass_Caustics(ID3D11InputLayout* trianglestrip_inputlayout, ID3D11Buffer* pathVB) {
	UINT stride=sizeof(float)*4;
	UINT offset=0;
	ID3D11DeviceContext* pContext;	
	D3D11_VIEWPORT water_normalmap_resource_viewport;

	D3D11Dev()->GetImmediateContext(&pContext);

	float ClearColor[4] = { 0.8f, 0.8f, 1.0f, 1.0f };

	pContext->OMSetRenderTargets( 1, &water_normalmap_resourceRTV, NULL);
	pContext->ClearRenderTargetView( water_normalmap_resourceRTV, ClearColor );

	//rendering water normalmap
	SetupNormalView(GetCamera()); // need it just to provide shader with camera position
	pContext->IASetInputLayout(trianglestrip_inputlayout);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	Effect->GetTechniqueByName("WaterNormalmapCombine")->GetPassByIndex(0)->Apply(0, pContext);
	stride=sizeof(float)*6;
	pContext->IASetVertexBuffers(0,1,&pathVB,&stride,&offset);
	pContext->Draw(4, 0); // just need to pass 4 vertices to shader

}

void SNBTerrainMaterial::CommitPerInstanceShaderConstants() {
	ID3DX11EffectShaderResourceVariable* tex_variable;

	tex_variable=Effect->GetVariableByName("g_LayerdefTexture")->AsShaderResource();
	tex_variable->SetResource(layerdef_textureSRV);
}

void SNBTerrainMaterial::CommitCommonShaderConstants()
{

	ID3D11DeviceContext* pContext;
	ID3DX11EffectShaderResourceVariable* tex_variable;
	float origin[2]={0,0};
	UINT stride=sizeof(float)*4;
	UINT offset=0;
	UINT cRT = 1;
	

	D3D11Dev()->GetImmediateContext(&pContext);

	noVec2 ScreenSizeInv(1.0f/(GetApp()->getWidth() * 1.0f/* main_buffer_size_multiplier*/),
		1.0f/(GetApp()->getHeight() * 1.0f/*main_buffer_size_multiplier*/));

	Effect->GetVariableByName("g_ZNear")->AsScalar()->SetFloat(GetCamera()->GetNear());
	Effect->GetVariableByName("g_ZFar")->AsScalar()->SetFloat(GetCamera()->GetFar());
	Effect->GetVariableByName("g_LightPosition")->AsVector()->SetFloatVector(gSun.m_vLitPos.ToFloatPtr());
	//Effect->GetVariableByName("g_WaterBumpTexcoordShift")->AsVector()->SetFloatVector(WaterTexcoordShift);
	Effect->GetVariableByName("g_ScreenSizeInv")->AsVector()->SetFloatVector(ScreenSizeInv.ToFloatPtr());

	Effect->GetVariableByName("g_DynamicTessFactor")->AsScalar()->SetFloat(g_DynamicTessellationFactor);
	Effect->GetVariableByName("g_StaticTessFactor")->AsScalar()->SetFloat(g_StaticTessellationFactor);
	Effect->GetVariableByName("g_UseDynamicLOD")->AsScalar()->SetFloat(g_UseDynamicLOD?1.0f:0.0f);
	Effect->GetVariableByName("g_RenderCaustics")->AsScalar()->SetFloat(g_RenderCaustics?1.0f:0.0f);
	// 정사각형.
	//Effect->GetVariableByName("g_g_HeightFieldSize")->AsScalar()->SetFloat(CElevationDataSource::m_iWidth);		
	
	if (DiffuseMap1SRV) DiffuseMap1Var->SetResource(DiffuseMap1SRV);
	if (DiffuseMap2SRV) DiffuseMap2Var->SetResource(DiffuseMap2SRV);
	if (DiffuseMap3SRV) DiffuseMap3Var->SetResource(DiffuseMap3SRV);
	if (DiffuseMap4SRV) DiffuseMap4Var->SetResource(DiffuseMap4SRV);

	if (BumpMap1SRV)	BumpMap1Var->SetResource(BumpMap1SRV);
	if (BumpMap2SRV)	BumpMap2Var->SetResource(BumpMap2SRV);
	if (BumpMap3SRV)	BumpMap3Var->SetResource(BumpMap3SRV);
	if (BumpMap4SRV)	BumpMap4Var->SetResource(BumpMap4SRV);

	if (SlopeTexSRV)	SlopeMapVar->SetResource(SlopeTexSRV);
	//tex_variable=Effect->GetVariableByName("g_SlopeDiffuseTexture")->AsShaderResource();
	//tex_variable->SetResource(slope_diffuse_textureSRV);

	//tex_variable=Effect->GetVariableByName("g_WaterBumpTexture")->AsShaderResource();
	//tex_variable->SetResource(water_bump_textureSRV);

	//tex_variable=Effect->GetVariableByName("g_SkyTexture")->AsShaderResource();
	//tex_variable->SetResource(sky_textureSRV);
	tex_variable=Effect->GetVariableByName("g_DepthMapTexture")->AsShaderResource();
	tex_variable->SetResource(depthmap_texSRV);
}

void SNBTerrainMaterial::ReCreateBuffers() {
	
	BackbufferWidth = GetApp()->getWidth();
	BackbufferHeight = GetApp()->getHeight();
	MultiSampleCount = AppSettings::MSAAMode.NumSamples();
	MultiSampleQuality = 0;
#if 0
	D3D11_TEXTURE2D_DESC tex_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC textureSRV_desc;
	D3D11_DEPTH_STENCIL_VIEW_DESC DSV_desc;

	ID3D11Device* pDevice = D3D11Dev();

	

	/*SAFE_RELEASE(main_color_resource);
	SAFE_RELEASE(main_color_resourceSRV);
	SAFE_RELEASE(main_color_resourceRTV);

	SAFE_RELEASE(main_color_resource_resolved);
	SAFE_RELEASE(main_color_resource_resolvedSRV);

	SAFE_RELEASE(main_depth_resource);
	SAFE_RELEASE(main_depth_resourceDSV);
	SAFE_RELEASE(main_depth_resourceSRV);*/

	//SAFE_RELEASE(reflection_color_resource);
	//SAFE_RELEASE(reflection_color_resourceSRV);
	//SAFE_RELEASE(reflection_color_resourceRTV);
	//SAFE_RELEASE(refraction_color_resource);
	//SAFE_RELEASE(refraction_color_resourceSRV);
	//SAFE_RELEASE(refraction_color_resourceRTV);

	//SAFE_RELEASE(reflection_depth_resource);
	//SAFE_RELEASE(reflection_depth_resourceDSV);
	//SAFE_RELEASE(refraction_depth_resource);
	//SAFE_RELEASE(refraction_depth_resourceSRV);
	//SAFE_RELEASE(refraction_depth_resourceRTV);

	//SAFE_RELEASE(shadowmap_resource);
	//SAFE_RELEASE(shadowmap_resourceDSV);
	//SAFE_RELEASE(shadowmap_resourceSRV);

	//SAFE_RELEASE(water_normalmap_resource);
	//SAFE_RELEASE(water_normalmap_resourceSRV);
	//SAFE_RELEASE(water_normalmap_resourceRTV);

	// recreating main color buffer

	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc,sizeof(tex_desc));

	tex_desc.Width              = (UINT)(BackbufferWidth*main_buffer_size_multiplier);
	tex_desc.Height             = (UINT)(BackbufferHeight*main_buffer_size_multiplier);
	tex_desc.MipLevels          = 1;
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count   = MultiSampleCount;
	tex_desc.SampleDesc.Quality = MultiSampleQuality;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = 0;

	textureSRV_desc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateTexture2D         ( &tex_desc, NULL, &main_color_resource );
	pDevice->CreateShaderResourceView( main_color_resource, &textureSRV_desc, &main_color_resourceSRV );
	pDevice->CreateRenderTargetView  ( main_color_resource, NULL, &main_color_resourceRTV );


	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc,sizeof(tex_desc));

	tex_desc.Width              = (UINT)(BackbufferWidth*main_buffer_size_multiplier);
	tex_desc.Height             = (UINT)(BackbufferHeight*main_buffer_size_multiplier);
	tex_desc.MipLevels          = 1;
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count   = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = 0;

	textureSRV_desc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateTexture2D         ( &tex_desc, NULL, &main_color_resource_resolved );
	pDevice->CreateShaderResourceView( main_color_resource_resolved, &textureSRV_desc, &main_color_resource_resolvedSRV );

	// recreating main depth buffer

	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc,sizeof(tex_desc));

	tex_desc.Width              = (UINT)(BackbufferWidth*main_buffer_size_multiplier);
	tex_desc.Height             = (UINT)(BackbufferHeight*main_buffer_size_multiplier);
	tex_desc.MipLevels          = 1;
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R32_TYPELESS;
	tex_desc.SampleDesc.Count   = MultiSampleCount;
	tex_desc.SampleDesc.Quality = MultiSampleQuality;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = 0;

	DSV_desc.Format  = DXGI_FORMAT_D32_FLOAT;
	DSV_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	DSV_desc.Flags = 0;
	DSV_desc.Texture2D.MipSlice = 0;

	textureSRV_desc.Format                    = DXGI_FORMAT_R32_FLOAT;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	textureSRV_desc.Texture2D.MipLevels		  = 1;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateTexture2D( &tex_desc, NULL, &main_depth_resource );
	pDevice->CreateDepthStencilView(main_depth_resource, &DSV_desc, &main_depth_resourceDSV );
	pDevice->CreateShaderResourceView( main_depth_resource, &textureSRV_desc, &main_depth_resourceSRV );

	// recreating reflection and refraction color buffers

	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc,sizeof(tex_desc));

	tex_desc.Width              = (UINT)(BackbufferWidth*reflection_buffer_size_multiplier);
	tex_desc.Height             = (UINT)(BackbufferHeight*reflection_buffer_size_multiplier);
	tex_desc.MipLevels          = (UINT)max((float)1,log(max((float)tex_desc.Width,(float)tex_desc.Height))/(float)log(2.0f));
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count   = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	textureSRV_desc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateTexture2D         ( &tex_desc, NULL, &reflection_color_resource );
	pDevice->CreateShaderResourceView( reflection_color_resource, &textureSRV_desc, &reflection_color_resourceSRV );
	pDevice->CreateRenderTargetView  ( reflection_color_resource, NULL, &reflection_color_resourceRTV );


	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc,sizeof(tex_desc));

	tex_desc.Width              = (UINT)(BackbufferWidth*refraction_buffer_size_multiplier);
	tex_desc.Height             = (UINT)(BackbufferHeight*refraction_buffer_size_multiplier);
	tex_desc.MipLevels          = (UINT)max((float)1,log(max((float)tex_desc.Width,(float)tex_desc.Height))/(float)log(2.0f));
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count   = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	textureSRV_desc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateTexture2D         ( &tex_desc, NULL, &refraction_color_resource );
	pDevice->CreateShaderResourceView( refraction_color_resource, &textureSRV_desc, &refraction_color_resourceSRV );
	pDevice->CreateRenderTargetView  ( refraction_color_resource, NULL, &refraction_color_resourceRTV );

	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc,sizeof(tex_desc));

	// recreating reflection and refraction depth buffers

	tex_desc.Width              = (UINT)(BackbufferWidth*reflection_buffer_size_multiplier);
	tex_desc.Height             = (UINT)(BackbufferHeight*reflection_buffer_size_multiplier);
	tex_desc.MipLevels          = 1;
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R32_TYPELESS;
	tex_desc.SampleDesc.Count   = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = 0;

	DSV_desc.Format  = DXGI_FORMAT_D32_FLOAT;
	DSV_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSV_desc.Flags = 0;
	DSV_desc.Texture2D.MipSlice = 0;

	pDevice->CreateTexture2D( &tex_desc, NULL, &reflection_depth_resource );
	pDevice->CreateDepthStencilView(reflection_depth_resource, &DSV_desc, &reflection_depth_resourceDSV );


	tex_desc.Width              = (UINT)(BackbufferWidth*refraction_buffer_size_multiplier);
	tex_desc.Height             = (UINT)(BackbufferHeight*refraction_buffer_size_multiplier);
	tex_desc.MipLevels          = 1;
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R32_FLOAT;
	tex_desc.SampleDesc.Count   = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = 0;

	textureSRV_desc.Format                    = DXGI_FORMAT_R32_FLOAT;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels       = 1;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateTexture2D( &tex_desc, NULL, &refraction_depth_resource );
	pDevice->CreateRenderTargetView  (refraction_depth_resource, NULL, &refraction_depth_resourceRTV );
	pDevice->CreateShaderResourceView(refraction_depth_resource, &textureSRV_desc, &refraction_depth_resourceSRV );

	// recreating shadowmap resource
	tex_desc.Width              = shadowmap_resource_buffer_size_xy;
	tex_desc.Height             = shadowmap_resource_buffer_size_xy;
	tex_desc.MipLevels          = 1;
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R32_TYPELESS;
	tex_desc.SampleDesc.Count   = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = 0;

	DSV_desc.Format  = DXGI_FORMAT_D32_FLOAT;
	DSV_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	DSV_desc.Flags = 0;
	DSV_desc.Texture2D.MipSlice=0;

	textureSRV_desc.Format                    = DXGI_FORMAT_R32_FLOAT;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels       = 1;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateTexture2D( &tex_desc, NULL, &shadowmap_resource);	
	pDevice->CreateShaderResourceView( shadowmap_resource, &textureSRV_desc, &shadowmap_resourceSRV );
	pDevice->CreateDepthStencilView(shadowmap_resource, &DSV_desc, &shadowmap_resourceDSV );

	// recreating water normalmap buffer

	tex_desc.Width              = water_normalmap_resource_buffer_size_xy;
	tex_desc.Height             = water_normalmap_resource_buffer_size_xy;
	tex_desc.MipLevels          = 1;
	tex_desc.ArraySize          = 1;
	tex_desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex_desc.SampleDesc.Count   = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage              = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags     = 0;
	tex_desc.MiscFlags          = 0;

	textureSRV_desc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureSRV_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureSRV_desc.Texture2D.MipLevels = tex_desc.MipLevels;
	textureSRV_desc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateTexture2D         ( &tex_desc, NULL, &water_normalmap_resource );
	pDevice->CreateShaderResourceView( water_normalmap_resource, &textureSRV_desc, &water_normalmap_resourceSRV );
	pDevice->CreateRenderTargetView  ( water_normalmap_resource, NULL, &water_normalmap_resourceRTV );
#endif
}

