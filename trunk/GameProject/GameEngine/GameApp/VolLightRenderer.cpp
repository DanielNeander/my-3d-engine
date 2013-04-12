#include "stdafx.h"
#include "GraphicsTypes.h"
#include "ShaderCompilation.h"
#include "PostProcessor.h"
#include "DeviceStates.h"
#include "AppSettings.h"
#include "GameApp.h"
#include <GameApp/Util/EffectUtil.h>

#include "SDKMeshLoader.h"
#include "VolLightRenderer.h"

static CDXUTSDKMesh gObjectMesh;


void VolLightRenderer::CreateRenderTargets()
{	
	const UINT NumSamples = AppSettings::MSAAMode.NumSamples();
	UINT width = GetApp()->getWidth();
	UINT height = GetApp()->getHeight();
	UINT dtWidth = AppSettings::MSAAEnabled() ? width * 2 : width;
	UINT dtHeight = AppSettings::MSAAMode == MSAAModeGUI::MSAA4x ? height * 2 : height;	

	CreateDepthBufferTexture(dtWidth, dtHeight);

	Shadowmap_->Initialize(Device, ShadowMapSize, ShadowMapSize, DXGI_FORMAT_D32_FLOAT, true);
	ShadowmapWorld_->Initialize(Device, ShadowMapSize, ShadowMapSize, DXGI_FORMAT_R32_TYPELESS, true);


	UINT size = ShadowMapSize;
	for ( int i=0; i < ShadowMips; ++i)	
	{
		size /= 2;		
		ShadowMapScaled_[i]->Initialize(Device, size, size, DXGI_FORMAT_R32G32_TYPELESS);

		if ( i == ShadowHoleMip )
		{
			for ( int j=0; j < 2; ++j)
			{
				ShadowMapHole[j]->Initialize(Device, size, size, DXGI_FORMAT_R32G32_TYPELESS);
			}
		}

	}

	ShadowMapScaledOpt_->Initialize(Device, size, size, DXGI_FORMAT_R32G32_TYPELESS);
	
	HDRTex->Initialize(Device, dtWidth, dtHeight, DXGI_FORMAT_R16G16B16A16_FLOAT);
	
	uint scaledW = dtWidth /ScaleRatio;
	uint scaledH = dtHeight /ScaleRatio;
	EdgeTexFS->Initialize(Device, scaledW, scaledH, DXGI_FORMAT_R16G16_FLOAT);

	CreateScaledTextures(scaledW, scaledH);
}

void VolLightRenderer::ReleaseScaledTextures()
{

}

void VolLightRenderer::CreateScaledTextures( uint width, uint height )
{
	ReleaseScaledTextures();

	HDRTexScaled->Initialize(Device, width, height, DXGI_FORMAT_R16_FLOAT);
	HDRTexScaled2->Initialize(Device, width, height, DXGI_FORMAT_R16_FLOAT);
	HDRTexScaled3->Initialize(Device, width, height, DXGI_FORMAT_R16_FLOAT);
}

void VolLightRenderer::LoadShaders()
{

	LoadEffectFromFile(Device, "Data/Shaders/VolumeLight.fx", &VolLitEffect, NULL, "fx_5_0");
	UseAngleOptimizationESV = VolLitEffect->GetVariableByName( "g_UseAngleOptimization" )->AsScalar();
	UseAngleOptimizationESV->SetBool(UseAngleOptimization);

	D3D11_INPUT_ELEMENT_DESC shadowmaplayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
	};

	const D3D11_INPUT_ELEMENT_DESC scenelayout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3DX11_PASS_DESC PassDesc;
	VolLitEffect->GetTechniqueByName( "RenderDiffuse" )->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	Device->CreateInputLayout( scenelayout, sizeof(scenelayout) / sizeof(scenelayout[0]), PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &SceneVertexLayout );


	g_pS0 = VolLitEffect->GetVariableByName("s0")->AsShaderResource();
	g_pS1 = VolLitEffect->GetVariableByName("s1")->AsShaderResource();
	g_pS2 = VolLitEffect->GetVariableByName("s2")->AsShaderResource();

	g_pavSampleOffsetsHorizontal = VolLitEffect->GetVariableByName("g_avSampleOffsetsHorizontal")->AsVector();
	g_pavSampleOffsetsVertical = VolLitEffect->GetVariableByName("g_avSampleOffsetsVertical")->AsVector();
	g_pavSampleWeights = VolLitEffect->GetVariableByName("g_avSampleWeights")->AsVector();

}

void VolLightRenderer::Reset()
{
	CreateRenderTargets();

	NoiseTex_->Initialize(Device, GetApp()->getWidth() / 8, GetApp()->getHeight() / 8, DXGI_FORMAT_R8_UNORM);

	D3D11_SHADER_RESOURCE_VIEW_DESC SRVdesc;
	SRVdesc.Format = DXGI_FORMAT_R8_UNORM;
	SRVdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVdesc.Texture2D.MipLevels = 1;
	SRVdesc.Texture2D.MostDetailedMip = 0;
	Device->CreateShaderResourceView( NoiseTex_->Texture, &SRVdesc, &NoiseTexSRV );

	VolLitEffect->GetVariableByName( "g_BufferWidth" )->AsScalar()->SetFloat(float(GetApp()->getWidth()));
	VolLitEffect->GetVariableByName( "g_BufferHeight" )->AsScalar()->SetFloat(float(GetApp()->getHeight()));
	VolLitEffect->GetVariableByName( "g_BufferWidthInv" )->AsScalar()->SetFloat( 1.0f / float(GetApp()->getWidth()));
	VolLitEffect->GetVariableByName( "g_BufferHeightInv" )->AsScalar()->SetFloat( 1.0f / float(GetApp()->getHeight()));
}

void VolLightRenderer::LoadVolumLightShader()
{

}

void VolLightRenderer::DestoryRenderTargets()
{

}

bool VolLightRenderer::Init(ID3D11Device* dev, ID3D11DeviceContext* context)
{
	ShadowMapSize = 1024;
	ShadowMips = 4;
	ShadowHoleMip = 3;
	LightSize = 2500.f;
	LightNear = 500.f;
	LightFar = 10000.f;
	ScaleRatio = 4;
	SamplingRate = 1.0f;

	UsePostProcessing = true;
	UseZOptimization = true;
	UseAngleOptimization = false;
	DrawBloom = DrawScaled = true;
	DrawVolumeLight = true;

	float up[] = {0.f, 1.0f, 0.f};
	LightCam = new ShadowMapCamera(noVec3(500.f, 1500.f, 500.f).ToFloatPtr(),vec3_zero.ToFloatPtr(), up, 45);
	LightCam->setNear(LightNear);
	LightCam->setFar(LightFar);
	LightCam->setProjectionMode(CAMERA_COMPUTE_ORTHOGRAPHIC);	
	LightCam->computeModelView();
	//LightCam->ComputeOrtho();
	LightCam->ComputeProjection();
	
	Device = dev;
	Context = context;
	LoadShaders();
	Shadowmap_ = new DepthStencilBuffer;

	ShadowMapScaledOpt_ = new RenderTarget2D;
	ShadowMapScaled_ = new RenderTarget2D*[ShadowMips];
	for(int i=0; i < ShadowMips; ++i)
		ShadowMapScaled_[i] = new RenderTarget2D();
	ShadowmapWorld_ = new RenderTarget2D;

	ShadowMapHole = new RenderTarget2D*[2];
	for(int i=0; i < 2; ++i)
		ShadowMapHole[i] = new RenderTarget2D();
		
	HDRTex = new RenderTarget2D;
	EdgeTexFS = new RenderTarget2D;

	DepthBufer = new DepthStencilBuffer;
	DepthBuferWS = new RenderTarget2D();

	HDRTexScaled = new RenderTarget2D;
	HDRTexScaled2 = new RenderTarget2D;
	HDRTexScaled3 = new RenderTarget2D;


	NoiseTex_ = new D3D11Texture2D;
	

	Reset();

	gObjectMesh.Create( Device, L"Media\\VolumeLight\\Shed.sdkmesh", true);

	return true;
}


void VolLightRenderer::DrawFullScreenQuad10( ID3D10Device* pd3dDevice )
{
	pd3dDevice->IASetInputLayout( NULL );
	pd3dDevice->IASetIndexBuffer( NULL, DXGI_FORMAT_R32_UINT, 0 );        
	pd3dDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_POINTLIST );
	pd3dDevice->Draw( 1, 0 );
}

void VolLightRenderer::DrawFullScreenQuad10( ID3D10Device* pd3dDevice, unsigned width, unsigned height )
{
	D3D10_VIEWPORT vpOld[1];
	UINT nViewPorts = 1;
	pd3dDevice->RSGetViewports( &nViewPorts, vpOld );

	// Setup the viewport to match the backbuffer
	D3D10_VIEWPORT vp;
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pd3dDevice->RSSetViewports( 1, &vp );

	pd3dDevice->IASetInputLayout( NULL );
	pd3dDevice->IASetIndexBuffer( NULL, DXGI_FORMAT_R32_UINT, 0 );        
	pd3dDevice->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_POINTLIST );
	pd3dDevice->Draw( 1, 0 );

	// Restore the old viewport
	pd3dDevice->RSSetViewports( nViewPorts, vpOld );
}

void VolLightRenderer::DrawFullScreenQuad11( ID3D11DeviceContext* pd3dDeviceContext )
{
	pd3dDeviceContext->IASetInputLayout( NULL );
	pd3dDeviceContext->IASetIndexBuffer( NULL, DXGI_FORMAT_R32_UINT, 0 );        
	pd3dDeviceContext->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_POINTLIST );
	pd3dDeviceContext->Draw( 1, 0 );	
}

void VolLightRenderer::DrawFullScreenQuad11( ID3D11DeviceContext* pd3dDeviceContext, unsigned width, unsigned height )
{
	D3D11_VIEWPORT vpOld[1];
	UINT nViewPorts = 1;
	pd3dDeviceContext->RSGetViewports( &nViewPorts, vpOld );

	// Setup the viewport to match the backbuffer
	D3D11_VIEWPORT vp;
	vp.Width = width;
	vp.Height = height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pd3dDeviceContext->RSSetViewports( 1, &vp );

	pd3dDeviceContext->IASetInputLayout( NULL );
	pd3dDeviceContext->IASetIndexBuffer( NULL, DXGI_FORMAT_R32_UINT, 0 );        
	pd3dDeviceContext->IASetPrimitiveTopology( D3D10_PRIMITIVE_TOPOLOGY_POINTLIST );
	pd3dDeviceContext->Draw( 1, 0 );

	// Restore the old viewport
	pd3dDeviceContext->RSSetViewports( nViewPorts, vpOld );
}

void VolLightRenderer::CreateDepthBufferTexture( UINT width, UINT height )
{
	DeleteDepthBufferTexture();

	DepthBufer->Initialize(Device, width, height, DXGI_FORMAT_D32_FLOAT, true);
	DepthBuferWS->Initialize(Device, width, height, DXGI_FORMAT_R32_TYPELESS);
}

void VolLightRenderer::DeleteDepthBufferTexture()
{
	
}

void VolLightRenderer::Render(ShadowMapCamera* shadowcam, BaseCamera* viewCam)
{
	D3DXMATRIX g_MProjection(viewCam->getProjectionMatrix());
	D3DXVECTOR3 g_EyePosition(viewCam->GetFrom().ToFloatPtr());
	D3DXMATRIX g_MView(viewCam->getViewMatrix());
	D3DXMATRIX  g_MLight;
	D3DXMATRIX  g_MWorldViewProjection;
	D3DXMATRIX  g_MInvWorldViewProjection;
	D3DXMATRIX  g_MWorldLightProjection;
	D3DXMATRIX	g_MWorldLightProjectionInv;
	D3DXMATRIX  g_MLightProjection;
	D3DXVECTOR3 g_LightPosition;
	g_LightPosition.x = 500.0f;
	g_LightPosition.y = 1500.0f;
	g_LightPosition.z = 500.0f;
	D3DXMATRIX g_LightProjMatrix(LightCam->getProjectionMatrix());
		
	VolLitEffect->GetVariableByName( "g_rSamplingRate" )->AsScalar()->SetFloat( 1.0f / SamplingRate );

	shadowcam = LightCam;

	D3DXVECTOR3 vecRight;
	vecRight.x = g_MView.m[0][0];
	vecRight.y = g_MView.m[1][0];
	vecRight.z = g_MView.m[2][0];

	D3DXVECTOR3 vecUp;
	vecUp.x = g_MView.m[0][1];
	vecUp.y = g_MView.m[1][1];
	vecUp.z = g_MView.m[2][1];

	D3DXVECTOR3 vecForward;
	vecForward.x = g_MView.m[0][2];
	vecForward.y = g_MView.m[1][2];
	vecForward.z = g_MView.m[2][2];

	VolLitEffect->GetVariableByName( "g_WorldRight" )->AsVector()->SetFloatVectorArray( (float*)&vecRight, 0, 3 );
	VolLitEffect->GetVariableByName( "g_WorldUp" )->AsVector()->SetFloatVectorArray( (float*)&vecUp, 0, 3 );
	VolLitEffect->GetVariableByName( "g_WorldFront" )->AsVector()->SetFloatVectorArray( (float*)&vecForward, 0, 3 );
	VolLitEffect->GetVariableByName( "g_ZNear" )->AsScalar()->SetFloat( viewCam->GetNear() );
	VolLitEffect->GetVariableByName( "g_ZFar" )->AsScalar()->SetFloat( viewCam->GetFar() );

	g_MWorldViewProjection = g_MView * g_MProjection;
	VolLitEffect->GetVariableByName( "g_ModelViewProj" )->AsMatrix()->SetMatrix( g_MWorldViewProjection );

	D3DXMATRIX g_MWorldViewProjectionInv;
	D3DXMatrixInverse( &g_MWorldViewProjectionInv, NULL, &g_MWorldViewProjection );
	VolLitEffect->GetVariableByName( "g_MWorldViewProjectionInv" )->AsMatrix()->SetMatrix( g_MWorldViewProjectionInv );

	D3DXMATRIX g_ProjectionInv;
	D3DXMatrixInverse( &g_ProjectionInv, NULL, &g_MProjection );
	VolLitEffect->GetVariableByName( "g_ProjectionInv" )->AsMatrix()->SetMatrix( g_ProjectionInv );

	VolLitEffect->GetVariableByName( "g_EyePosition" )->AsVector()->SetFloatVector( (float*)&g_EyePosition );
	VolLitEffect->GetVariableByName( "g_LightPosition" )->AsVector()->SetFloatVector( (float*)&g_LightPosition );

	D3DXMATRIX viewMatrix(shadowcam->getViewMatrix());

	vecRight.x = viewMatrix.m[0][0];
	vecRight.y = viewMatrix.m[1][0];
	vecRight.z = viewMatrix.m[2][0];

	vecUp.x = viewMatrix.m[0][1];
	vecUp.y = viewMatrix.m[1][1];
	vecUp.z = viewMatrix.m[2][1];

	vecForward.x = viewMatrix.m[0][2];
	vecForward.y = viewMatrix.m[1][2];
	vecForward.z = viewMatrix.m[2][2];


	VolLitEffect->GetVariableByName( "g_LightRight" )->AsVector()->SetFloatVectorArray( (float*)&vecRight, 0, 3);
	VolLitEffect->GetVariableByName( "g_LightUp" )->AsVector()->SetFloatVectorArray( (float*)&vecUp, 0, 3);
	VolLitEffect->GetVariableByName( "g_LightForward" )->AsVector()->SetFloatVectorArray( (float*)&vecForward, 0, 3);

    g_MLightProjection = g_LightProjMatrix ;
    g_MLight = viewMatrix;
    g_MWorldLightProjection = g_MLight * g_MLightProjection;
    VolLitEffect->GetVariableByName( "g_ModelLightProj" )->AsMatrix()->SetMatrix( g_MWorldLightProjection );

	D3DXMatrixInverse( &g_MWorldLightProjectionInv, NULL, &g_MWorldLightProjection );
	VolLitEffect->GetVariableByName( "g_ModelLightProjInv" )->AsMatrix()->SetMatrix( g_MWorldLightProjectionInv );
	
	Direct3D11Renderer* renderer = (Direct3D11Renderer*)GetRenderer();
	ID3D11RenderTargetView *pDefaultRTV = renderer->getBackBuffer();
	D3D11_VIEWPORT defaultViewport;

	// Set all critical states for main passes
	VolLitEffect->GetTechniqueByName( "DummyPass" )->GetPassByIndex( 0 )->Apply( 0, Context );

		
	UINT viewPortsNum = 1;
	Context->RSGetViewports( &viewPortsNum, &defaultViewport );
	
	Context->ClearDepthStencilView( Shadowmap_->DSView, D3D11_CLEAR_DEPTH, 1.0, 0 );
	Context->OMSetRenderTargets( 0, NULL, Shadowmap_->DSView );

	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
	viewport.Width = ShadowMapSize;
	viewport.Height = ShadowMapSize;
	Context->RSSetViewports( 1, &viewport );

	Context->IASetInputLayout( SceneVertexLayout );
	gObjectMesh.RenderFX( Device, Context, VolLitEffect->GetTechniqueByName( "ShadowmapPass" ) );
		
	

	D3D11_TEXTURE2D_DESC desc;	
	ID3D11RenderTargetView* hdrRT = HDRTex->RTViewPtr;
	Context->OMSetRenderTargets( 1, &hdrRT, DepthBufer->DSView );
	HDRTex->Texture->GetDesc( &desc );
	VolLitEffect->GetVariableByName( "g_BufferWidthInv" )->AsScalar()->SetFloat( 1.0f / desc.Width  );
	VolLitEffect->GetVariableByName( "g_BufferHeightInv" )->AsScalar()->SetFloat( 1.0f / desc.Height );

	static float clearColor[4] = { 0.65f, 0.65f, 0.9f, 1.0f };
	static float clearColorBlack[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	Context->ClearRenderTargetView( HDRTex->RTViewPtr, clearColor );
	Context->ClearDepthStencilView( DepthBufer->DSView, D3D11_CLEAR_DEPTH, 1.0, 0 );
	Context->RSSetViewports( 1, &defaultViewport );

	VolLitEffect->GetVariableByName( "DepthTexture" )->AsShaderResource()->SetResource( Shadowmap_->SRView );

	Context->IASetInputLayout( SceneVertexLayout );
	gObjectMesh.RenderFX( Device ,Context, VolLitEffect->GetTechniqueByName( "RenderDiffuse" ) ,
		VolLitEffect->GetVariableByName( "DiffuseTexture" )->AsShaderResource());
	
		
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Convert depth values to world scale
	////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	ID3D11RenderTargetView* shadowWorldRT = ShadowmapWorld_->RTView;
	Context->OMSetRenderTargets( 1, &shadowWorldRT, NULL );
	ShadowmapWorld_->Texture->GetDesc( &desc );

	viewport.Width = desc.Width;
	viewport.Height = desc.Height;
	Context->RSSetViewports( 1, &viewport );

	g_pS0->SetResource( ShadowmapWorld_->SRView );	
	VolLitEffect->GetVariableByName( "g_ModelLightProjInv" )->AsMatrix()->SetMatrix( g_MWorldLightProjectionInv );	
	VolLitEffect->GetTechniqueByName( "DepthProcessing" )->GetPassByName( "pConvertToWorld" )->Apply( 0, Context );
	DrawFullScreenQuad11( Context );

	g_pS0->SetResource( NULL );
	
	ID3D11RenderTargetView* ShadowMapScaledRT0 = ShadowMapScaled_[0]->RTView;
	Context->OMSetRenderTargets( 1, &ShadowMapScaledRT0, NULL );
	ShadowMapScaled_[0]->Texture->GetDesc( &desc );
	VolLitEffect->GetVariableByName( "g_BufferWidthInv" )->AsScalar()->SetFloat( 1.0f / desc.Width  );
	VolLitEffect->GetVariableByName( "g_BufferHeightInv" )->AsScalar()->SetFloat( 1.0f / desc.Height );

	viewport.Width = desc.Width;
	viewport.Height = desc.Height;
	Context->RSSetViewports( 1, &viewport );

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Downscale Depth, produce Min and Max mips for optimized tracing
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	g_pS0->SetResource( ShadowmapWorld_->SRView );
	VolLitEffect->GetTechniqueByName( "DepthProcessing" )->GetPassByName( "pMinMax2x2_1" )->Apply( 0 , Context);
	DrawFullScreenQuad11( Context );

	g_pS0->SetResource( NULL );

	for( int i = 0; i < ShadowMips - 1; i++ )
	{
		ID3D11RenderTargetView* ShadowMapScaledRT = ShadowMapScaled_[i + 1]->RTView;
		Context->OMSetRenderTargets( 1, &ShadowMapScaledRT, NULL );
		ShadowMapScaled_[i + 1]->Texture->GetDesc( &desc );
		VolLitEffect->GetVariableByName( "g_BufferWidthInv" )->AsScalar()->SetFloat( 1.0f / desc.Width  );
		VolLitEffect->GetVariableByName( "g_BufferHeightInv" )->AsScalar()->SetFloat( 1.0f / desc.Height );
		viewport.Width = desc.Width;
		viewport.Height = desc.Height;
		Context->RSSetViewports( 1, &viewport );

		g_pS0->SetResource( ShadowMapScaled_[i]->SRView );
		VolLitEffect->GetTechniqueByName( "DepthProcessing" )->GetPassByName( "pMinMax2x2_2" )->Apply( 0 , Context);
		DrawFullScreenQuad11( Context );

		g_pS0->SetResource( NULL );
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//		
	//		Use a number of passes to fill the "holes" in the depth map
	//
	//		The idea is to increase light density in shadowmap "hole" areas 
	//		(small) areas with big depth discontinuetes. 
	//		
	//		DEPTH MAP PROCESSING
	//
	//		1) BEFORE: 
	//		____    ___
	//				   \____     _______
	//
	//
	//		2) AFTER:
	//		___________
	//				   \_________________
	//
	//		If, after the final step, the area is occluded, but was initially lit,
	//		we increase light density for those.
	//
	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	
	Context->OMSetRenderTargets( 1, &ShadowMapHole[0]->RTViewPtr, NULL );
	Context->ClearRenderTargetView( ShadowMapHole[0]->RTViewPtr, clearColorBlack );
	ShadowMapHole[0]->Texture->GetDesc( &desc );
	viewport.Width = desc.Width;
	viewport.Height = desc.Height;
	Context->RSSetViewports( 1, &viewport );

	g_pS0->SetResource( ShadowMapScaled_[ShadowHoleMip]->SRView );
	VolLitEffect->GetVariableByName( "DepthTexture" )->AsShaderResource()->SetResource( ShadowmapWorld_->SRView );

	VolLitEffect->GetTechniqueByName( "DepthProcessing" )->GetPassByName( "pPropagateMinDepth_0" )->Apply( 0, Context );
	DrawFullScreenQuad11( Context );

	int oddPass = 0;

	for( int i = 0; i < 2; i++, oddPass = 1 - oddPass )
	{
		ID3D11RenderTargetView* ShadowMapHoleRT = ShadowMapHole[1 - oddPass]->RTView;
		Context->OMSetRenderTargets( 1, &ShadowMapHoleRT, NULL );
		g_pS0->SetResource( ShadowMapHole[oddPass]->SRView );
		VolLitEffect->GetTechniqueByName( "DepthProcessing" )->GetPassByName( "pPropagateMinDepth_1" )->Apply( 0, Context );
		DrawFullScreenQuad11( Context );		
	}

	for( int i = 0; i < 5; i++, oddPass = 1 - oddPass )
	{
		ID3D11RenderTargetView* ShadowMapHoleRT = ShadowMapHole[1 - oddPass]->RTView;
		Context->OMSetRenderTargets( 1, &ShadowMapHoleRT, NULL );
		g_pS0->SetResource( ShadowMapHole[oddPass]->SRView );
		VolLitEffect->GetTechniqueByName( "DepthProcessing" )->GetPassByName( "pPropagateMaxDepth" )->Apply( 0, Context );
		DrawFullScreenQuad11( Context );
		
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////

	g_pS0->SetResource( NULL );

	if( DrawVolumeLight )
	{
		ID3D11RenderTargetView * HDRTexScaledRT = HDRTexScaled->RTView;
		Context->OMSetRenderTargets( 1, &HDRTexScaledRT, NULL );
		HDRTexScaled->Texture->GetDesc( &desc );
		VolLitEffect->GetVariableByName( "g_BufferWidthInv" )->AsScalar()->SetFloat( 1.0f / desc.Width  );
		VolLitEffect->GetVariableByName( "g_BufferHeightInv" )->AsScalar()->SetFloat( 1.0f / desc.Height );

		viewport.Width = desc.Width;
		viewport.Height = desc.Height;
		Context->RSSetViewports( 1, &viewport );

		VolLitEffect->GetVariableByName( "DepthBufferTexture" )->AsShaderResource()->SetResource( DepthBufer->SRView );
		VolLitEffect->GetVariableByName( "NoiseTexture" )->AsShaderResource()->SetResource( NoiseTexSRV );
		VolLitEffect->GetVariableByName( "DepthTexture" )->AsShaderResource()->SetResource( ShadowmapWorld_->SRView );

		g_pS0->SetResource( ShadowMapScaledOpt_->SRView );
		g_pS1->SetResource( ShadowMapHole[1]->SRView ); // Map for light density calculation

		ShadowMapScaledOpt_->Texture->GetDesc( &desc );
		VolLitEffect->GetVariableByName( "g_CoarseDepthTexelSize" )->AsScalar()->SetFloat( LightSize / desc.Width );

		if( UseZOptimization )
			VolLitEffect->GetTechniqueByName( "Tracing" )->GetPassByName( "FullScreen_Optimized" )->Apply( 0 , Context);
		else
			VolLitEffect->GetTechniqueByName( "Tracing" )->GetPassByName( "FullScreen_Base" )->Apply( 0, Context );

		DrawFullScreenQuad11( Context );		
		g_pS0->SetResource( NULL );
	}

	Context->OMSetRenderTargets( 1, &HDRTex->RTView, NULL );
	Context->RSSetViewports( 1, &defaultViewport );

	if( DrawVolumeLight )
	{
		ID3D11RenderTargetView * DepthBufferWSRT = DepthBuferWS->RTView;
		Context->OMSetRenderTargets( 1, &DepthBufferWSRT, NULL );
		ShadowmapWorld_->Texture->GetDesc( &desc );
		viewport.Width = desc.Width;
		viewport.Height = desc.Height;
		Context->RSSetViewports( 1, &viewport );

		g_pS0->SetResource( DepthBufer->SRView );
		VolLitEffect->GetTechniqueByName( "DepthProcessing" )->GetPassByName( "pConvertDepthWorldNormalized" )->Apply( 0 , Context);
		DrawFullScreenQuad11( Context );
		


		//g_pS0->SetResource( g_pHDRTextureScaledSRV );
		g_pS0->SetResource( DepthBufer->SRView ); // Use this map for edge detection

		HDRTexScaled->Texture->GetDesc( &desc );
		VolLitEffect->GetVariableByName( "g_CoarseTextureWidthInv" )->AsScalar()->SetFloat( 1.0f / desc.Width );
		VolLitEffect->GetVariableByName( "g_CoarseTextureHeightInv" )->AsScalar()->SetFloat( 1.0f / desc.Height );

		// Perform edge detection based on the depth discontinuities
		Context->OMSetRenderTargets( 1, &EdgeTexFS->RTViewPtr, NULL );
		EdgeTexFS->Texture->GetDesc( &desc );
		viewport.Width = desc.Width;
		viewport.Height = desc.Height;
		Context->RSSetViewports( 1, &viewport );

		VolLitEffect->GetTechniqueByName( "EdgeProcessing" )->GetPassByName( "pEdgeDetection" )->Apply( 0 , Context);
		DrawFullScreenQuad11( Context );
		
		HDRTexScaled->Texture->GetDesc( &desc );
		VolLitEffect->GetVariableByName( "g_CoarseTextureWidthInv" )->AsScalar()->SetFloat( 1.0f / desc.Width );
		VolLitEffect->GetVariableByName( "g_CoarseTextureHeightInv" )->AsScalar()->SetFloat( 1.0f / desc.Height );

		// Blur the image along the edges
		Context->OMSetRenderTargets( 1, &HDRTexScaled2->RTViewPtr, NULL );
		HDRTexScaled2->Texture->GetDesc( &desc );
		viewport.Width = desc.Width;
		viewport.Height = desc.Height;
		Context->RSSetViewports( 1, &viewport );
		g_pS0->SetResource( HDRTexScaled->SRView );
		g_pS2->SetResource( EdgeTexFS->SRView );
		VolLitEffect->GetTechniqueByName( "EdgeProcessing" )->GetPassByName( "pGradientBlur" )->Apply( 0 , Context);
		DrawFullScreenQuad11( Context );
		
		// Blur the image with 3x3 kernel
		Context->OMSetRenderTargets( 1, &HDRTexScaled3->RTViewPtr, NULL );
		HDRTexScaled2->Texture->GetDesc( &desc );
		viewport.Width = desc.Width;
		viewport.Height = desc.Height;
		Context->RSSetViewports( 1, &viewport );
		g_pS0->SetResource( HDRTexScaled2->SRView );
		VolLitEffect->GetTechniqueByName( "EdgeProcessing" )->GetPassByName( "pImageBlur" )->Apply( 0 , Context);
		DrawFullScreenQuad11( Context );
		
		// Blend Volume Light with main scene
		Context->OMSetRenderTargets( 1, &HDRTex->RTViewPtr, NULL );
		Context->RSSetViewports( 1, &defaultViewport );
		g_pS0->SetResource( HDRTexScaled->SRView );
		g_pS1->SetResource(  HDRTexScaled3->SRView );

		UINT passIndex = 0;

		//if( g_DrawScene == false )
		//	passIndex = 1;

		if( UsePostProcessing )
			VolLitEffect->GetTechniqueByName( "BlendFullscreenPP" )->GetPassByIndex( passIndex )->Apply( 0 , Context);
		else
			VolLitEffect->GetTechniqueByName( "BlendFullscreen" )->GetPassByIndex( passIndex )->Apply( 0 , Context);

		DrawFullScreenQuad11( Context );		
		g_pS0->SetResource( NULL );
	}
	
}