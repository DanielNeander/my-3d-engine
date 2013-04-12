#include "stdafx.h"
#include "LIDRenderer.h"

#include "EngineCore/Math/IntersectionTests.h"
#include "EngineCore/Math/Frustum.h"

#include "Exceptions.h"
#include "Utility.h"
#include "ShaderCompilation.h"

#include "Mesh.h"
#include "M2Mesh.h"
#include "SDKMeshLoader.h"
#include "ShadowCamera.h"



// Constants
static const UINT ShadowMapSize = 2048;
static const float ShadowDist = 1.0f;
static const float Backup = 20.0f;
static const float NearClip = 1.0f;
static const float CascadeSplits[4] = { 0.125f, 0.25f, 0.5f, 1.0f };
static const float Bias = 0.005f;

void LIDRenderer::Initialize( ID3D11Device* device, ID3D11DeviceContext* context )
{
	this->device = device;

	blendStates.Initialize(device);
	rasterizerStates.Initialize(device);
	depthStencilStates.Initialize(device);
	samplerStates.Initialize(device);

	meshVSConstants.Initialize(device);
	meshPSConstants.Initialize(device);
	meshVSSkinConstants.Initialize(device);
	matConstants.Initialize(device);
	
	shConstants.Initialize(device);

	// Load the mesh shaders
	meshDepthVS.Attach(CompileVSFromFile(device, L"Data/Shaders/DepthOnly.hlsl", "VS", "vs_5_0", NULL, NULL, &compiledMeshDepthVS));
	meshSkinDepthVS.Attach(CompileVSFromFile(device, L"Data/Shaders/DepthOnlySkin.hlsl", "vsSkinnedDepth", "vs_5_0", NULL, NULL, &compiledMeshSkinDepthVS));

	for(UINT32 lmode = 0; lmode < NumLightsGUI::NumValues; ++lmode)
	{
		CompileOptions opts;
		opts.Add("MaxLights", NumLightsGUI::NumLights(lmode));
		meshVS[lmode].Attach(CompileVSFromFile(device, L"Data/Shaders/Mesh.hlsl", "VS", "vs_5_0", opts.Defines(), NULL, &compiledMeshVS));
		meshPS[lmode].Attach(CompilePSFromFile(device, L"Data/Shaders/Mesh.hlsl", "PS", "ps_5_0", opts.Defines()));
	}

	meshGBufferVS.Attach(CompileVSFromFile(device, L"Data/Shaders/GBuffer.hlsl", "VS", "vs_5_0", NULL, NULL, &compiledMeshGBufferVS));
	meshGBufferPS.Attach(CompilePSFromFile(device, L"Data/Shaders/GBuffer.hlsl", "PS", "ps_5_0"));

	const D3D11_INPUT_ELEMENT_DESC PNUTBLayout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "BITANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },

	};


	const D3D11_INPUT_ELEMENT_DESC PNTLayout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	
	};
		
	device->CreateInputLayout( 
		PNUTBLayout, ARRAYSIZE( PNUTBLayout ), 
		compiledMeshGBufferVS->GetBufferPointer(),
		compiledMeshGBufferVS->GetBufferSize(), 
		&meshGBufferInputLayout );

	device->CreateInputLayout( 
		PNUTBLayout, ARRAYSIZE( PNUTBLayout ), 
		compiledMeshVS->GetBufferPointer(),
		compiledMeshVS->GetBufferSize(), 
		&meshInputLayout );
	
	const D3D11_INPUT_ELEMENT_DESC PLayout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },		
	};

	device->CreateInputLayout( 
		PLayout, ARRAYSIZE( PLayout ), 
		compiledMeshDepthVS->GetBufferPointer(),
		compiledMeshDepthVS->GetBufferSize(), 
		&meshDepthInputLayout );

	

	/*for(UINT i = 0; i < model->Meshes().size(); ++i)
	{
		Mesh& mesh = model->Meshes()[i];
		ID3D11InputLayoutPtr inputLayout;
		DXCall(device->CreateInputLayout(mesh.InputElements(), mesh.NumInputElements(),
			compiledMeshVS->GetBufferPointer(), compiledMeshVS->GetBufferSize(), &inputLayout));
		meshInputLayouts.push_back(inputLayout);

		DXCall(device->CreateInputLayout(mesh.InputElements(), mesh.NumInputElements(),
			compiledMeshDepthVS->GetBufferPointer(), compiledMeshDepthVS->GetBufferSize(), &inputLayout));
		meshDepthInputLayouts.push_back(inputLayout);

		DXCall(device->CreateInputLayout(mesh.InputElements(), mesh.NumInputElements(),
			compiledMeshGBufferVS->GetBufferPointer(), compiledMeshGBufferVS->GetBufferSize(), &inputLayout));
		meshGBufferInputLayouts.push_back(inputLayout);
	}*/

	// Create the shadow map as a texture atlas
	shadowMap.Initialize(device, ShadowMapSize * 2, ShadowMapSize * 2, DXGI_FORMAT_D24_UNORM_S8_UINT, true, 1, 0, 1);
}

static const float ModelScale = 0.01f;
static const XMMATRIX ModelWorldMatrix = XMMatrixScaling(ModelScale, ModelScale, ModelScale) * XMMatrixRotationY(XM_PIDIV2);

BOOL LIDRenderer::DoFrustumTests( CDXUTSDKMesh* pMesh, BaseCamera* pCam )
{
	Frustum cameraFrustum = pCam->CalculateFrustum(pCam->GetNear(), pCam->GetFar());
	cameraFrustum.CalculateAABB();

	BoundingBox aabb;
	BoundingBox unionBB;

	noVec3 sceneMin(FLT_MAX, FLT_MAX, FLT_MAX);
	noVec3 sceneMax(FLT_MIN, FLT_MIN, FLT_MIN);
	noVec3 vMeshMin;
	noVec3 vMeshMax;
	int NumMeshes = pMesh->GetNumMeshes( );
	int Culled=0;
	for( UINT i =0; i < NumMeshes ; ++i ) 
	{
		SDKMESH_MESH* msh = pMesh->GetMesh( i );
					
		// test only with AABB of frustum
		aabb.m_vMin.Set(msh->BoundingBoxCenter.x - msh->BoundingBoxExtents.x,
			msh->BoundingBoxCenter.y - msh->BoundingBoxExtents.y,
			msh->BoundingBoxCenter.z - msh->BoundingBoxExtents.z );

		aabb.m_vMax.Set(msh->BoundingBoxCenter.x + msh->BoundingBoxExtents.x,
			msh->BoundingBoxCenter.y + msh->BoundingBoxExtents.y,
			msh->BoundingBoxCenter.z + msh->BoundingBoxExtents.z );

		XMFLOAT3 pos = XMFLOAT3(aabb.m_vMin.ToFloatPtr());
		XMVECTOR position = XMLoadFloat3(&pos);
		XMVECTOR minV = XMVector3TransformCoord(position, ModelWorldMatrix) ;		
		XMStoreFloat3(&pos, minV);
		vMeshMin.Set(pos.x, pos.y, pos.z);
		
		pos = XMFLOAT3(aabb.m_vMax.ToFloatPtr());
		position = XMLoadFloat3(&pos);
		XMVECTOR maxV = XMVector3TransformCoord(position, ModelWorldMatrix) ;		
		XMStoreFloat3(&pos, maxV);
		vMeshMax.Set(pos.x, pos.y, pos.z);
				
		aabb.m_vMin = vMeshMin;
		aabb.m_vMax = vMeshMax;
		
		if(!IntersectionTest(aabb, cameraFrustum.m_AABB)) 
		{
			msh->Culled = TRUE;		
			++Culled;
		}
		else 
			msh->Culled = FALSE;		

		noVec3 temp = vMeshMin;
		temp.Min( sceneMin );
		sceneMin = temp;
		temp = vMeshMax;
		temp.Max( sceneMax );		 
		sceneMax = temp;
	}	

	unionBB.m_vMin = sceneMin;
	unionBB.m_vMax = sceneMax;	 

	if(!IntersectionTest(unionBB, cameraFrustum.m_AABB)) 
		return FALSE;

	return TRUE;
}

void LIDRenderer::Setup( const XMFLOAT3& lightDir, const XMFLOAT3& lightColor, const XMMATRIX& world )
{
	this->lightDir = lightDir;
	meshPSConstants.Data.LightDirWS = lightDir;
	meshPSConstants.Data.LightColor = lightColor;
}

void LIDRenderer::RenderDepth( Mesh* pMesh, BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world )
{

}

void LIDRenderer::RenderGBuffer( Mesh* pMesh, BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world )
{

}

void LIDRenderer::Render( Mesh* pMesh, BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world, ID3D11ShaderResourceView* lightsBuffer, ID3D11ShaderResourceView* lightIndicesBuffer, UINT32 numTilesX, UINT32 numTilesY )
{
	
}

void LIDRenderer::BeginDepthSkinRender( BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world, noMat4* boneMatrices, int nBone /*= false */ )
{
	XMMATRIX proj(pCam->getProjectionMatrix());
	XMMATRIX view(pCam->getViewMatrix());

	memcpy(&meshVSSkinConstants.Data.World, &XMMatrixTranspose(world), SIZE16);	
	memcpy(&meshVSSkinConstants.Data.View, &XMMatrixTranspose(view), SIZE16);
	XMMATRIX wvp = world * view * proj;
	memcpy(&meshVSSkinConstants.Data.WorldViewProjection, &XMMatrixTranspose(wvp), SIZE16);
	memcpy(&meshVSSkinConstants.Data.BomeMatrices, &boneMatrices, SIZE16 * nBone);
	meshVSSkinConstants.ApplyChanges(context);
	meshVSSkinConstants.SetVS(context, 0);

	context->VSSetShader(meshSkinDepthVS , NULL, 0);
}



void LIDRenderer::BeginDepthRender( BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world, bool Skinned /*= false */ )
{
	// Set states
	float blendFactor[4] = {1, 1, 1, 1};
	context->OMSetBlendState(blendStates.ColorWriteDisabled(), blendFactor, 0xFFFFFFFF);
	context->OMSetDepthStencilState(depthStencilStates.DepthWriteEnabled(), 1);
	context->RSSetState(rasterizerStates.BackFaceCull());

	XMMATRIX proj(pCam->getProjectionMatrix());
	XMMATRIX view(pCam->getViewMatrix());

	
	 
		
		memcpy(&meshVSConstants.Data.World, &XMMatrixTranspose(world), SIZE16);	
		memcpy(&meshVSConstants.Data.View, &XMMatrixTranspose(view), SIZE16);
		XMMATRIX wvp = world * view * proj;
		memcpy(&meshVSConstants.Data.WorldViewProjection, &XMMatrixTranspose(wvp), SIZE16);
		meshVSConstants.ApplyChanges(context);
		meshVSConstants.SetVS(context, 0);
		context->IASetInputLayout(meshDepthInputLayout);

	// Set shaders
	context->VSSetShader(meshDepthVS , NULL, 0);
	context->PSSetShader(NULL, NULL, 0);
	context->GSSetShader(NULL, NULL, 0);
	context->DSSetShader(NULL, NULL, 0);
	context->HSSetShader(NULL, NULL, 0);
}

void LIDRenderer::RenderDepth2( CDXUTSDKMesh* pMesh, BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world )
{
	if (!DoFrustumTests(pMesh, pCam))
		return;
	// Set states
	float blendFactor[4] = {1, 1, 1, 1};
	context->OMSetBlendState(blendStates.ColorWriteDisabled(), blendFactor, 0xFFFFFFFF);
	context->OMSetDepthStencilState(depthStencilStates.DepthWriteEnabled(), 1);
	context->RSSetState(rasterizerStates.BackFaceCull());

	// Set constant buffers
	XMMATRIX proj(pCam->getProjectionMatrix());
	XMMATRIX view(pCam->getViewMatrix());
		
	memcpy(&meshVSConstants.Data.World, &XMMatrixTranspose(world), SIZE16);	
	memcpy(&meshVSConstants.Data.View, &XMMatrixTranspose(view), SIZE16);
	XMMATRIX wvp = world * view * proj;
	memcpy(&meshVSConstants.Data.WorldViewProjection, &XMMatrixTranspose(wvp), SIZE16);
	meshVSConstants.ApplyChanges(context);
	meshVSConstants.SetVS(context, 0);

	context->IASetInputLayout(meshDepthInputLayout);


	// Set shaders
	context->VSSetShader(meshDepthVS , NULL, 0);
	context->PSSetShader(NULL, NULL, 0);
	context->GSSetShader(NULL, NULL, 0);
	context->DSSetShader(NULL, NULL, 0);
	context->HSSetShader(NULL, NULL, 0);
		
		// Draw all parts
	pMesh->Render(context, 0, 1);
}

void LIDRenderer::RenderGBuffer2( CDXUTSDKMesh* pMesh, BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world )
{
	if (!DoFrustumTests(pMesh, pCam))
		return;
	// Set states
	float blendFactor[4] = {1, 1, 1, 1};
	context->OMSetBlendState(blendStates.BlendDisabled(), blendFactor, 0xFFFFFFFF);

	//if(AppSettings::EnableZPrepass)
	//	context->OMSetDepthStencilState(depthStencilStates.DepthEnabled(), 0);
	//else
		context->OMSetDepthStencilState(depthStencilStates.DepthWriteEnabled(), 0);

	context->RSSetState(rasterizerStates.BackFaceCull());

	ID3D11SamplerState* sampStates[1] = {
		samplerStates.Anisotropic(),
	};
	context->PSSetSamplers(0, 1, sampStates);

	// Set constant buffers
	XMMATRIX proj(pCam->getProjectionMatrix());
	XMMATRIX view(pCam->getViewMatrix());
	meshVSConstants.Data.World = XMMatrixTranspose(world);
	meshVSConstants.Data.View = XMMatrixTranspose(view);
	meshVSConstants.Data.WorldViewProjection = XMMatrixTranspose(world * view * proj);
	meshVSConstants.ApplyChanges(context);
	meshVSConstants.SetVS(context, 0);

	/*float di = AppSettings::DiffuseIntensity;
	float si = AppSettings::SpecularIntensity;

	XMStoreFloat3(&meshPSConstants.Data.CameraPosWS, camera.Position());
	meshPSConstants.Data.DiffuseAlbedo = XMFLOAT3(AppSettings::DiffuseR * di, AppSettings::DiffuseG * di, AppSettings::DiffuseB * di);
	meshPSConstants.Data.SpecularAlbedo = XMFLOAT3(AppSettings::SpecularR * si, AppSettings::SpecularG * si, AppSettings::SpecularB * si);
	meshPSConstants.Data.Balance = AppSettings::Balance;
	meshPSConstants.Data.Roughness = AppSettings::Roughness;*/
	float di = 0.f;//AppSettings::DiffuseIntensity;
	float si = 0.02f;//AppSettings::SpecularIntensity;

	float eyePos[3];
	pCam->getFrom(eyePos);
	meshPSConstants.Data.CameraPosWS = XMFLOAT3(eyePos);		
	//meshPSConstants.Data.DiffuseAlbedo = XMFLOAT3(AppSettings::DiffuseR * di, AppSettings::DiffuseG * di, AppSettings::DiffuseB * di);
	//meshPSConstants.Data.SpecularAlbedo = XMFLOAT3(AppSettings::SpecularR * si, AppSettings::SpecularG * si, AppSettings::SpecularB * si);
	meshPSConstants.Data.DiffuseAlbedo = XMFLOAT3(0.0f, 0.0f, 0.0f);
	meshPSConstants.Data.SpecularAlbedo = XMFLOAT3(0.02f, 0.02f, 0.02f);
	meshPSConstants.Data.NumTilesX = 0;
	meshPSConstants.Data.NumTilesY = 0;
	meshPSConstants.Data.EnableNormalMapping = true;
	meshPSConstants.ApplyChanges(context);
	meshPSConstants.SetPS(context, 0);

	// Set shaders
	context->DSSetShader(NULL, NULL, 0);
	context->HSSetShader(NULL, NULL, 0);
	context->GSSetShader(NULL, NULL, 0);
	context->VSSetShader(meshGBufferVS , NULL, 0);
	context->PSSetShader(meshGBufferPS, NULL, 0);

	context->IASetInputLayout(meshGBufferInputLayout);

	// Draw all meshes
	pMesh->Render(context, 0, 1);
	ID3D11ShaderResourceView* nullSRVs[2] = { NULL };
	context->PSSetShaderResources(0, 2, nullSRVs);
}

void LIDRenderer::BeginRender(ID3D11DeviceContext* context)
{
	float blendFactor[4] = {1, 1, 1, 1};
	context->OMSetBlendState(blendStates.BlendDisabled(), blendFactor, 0xFFFFFFFF);
	//context->OMSetDepthStencilState(depthStencilStates.DepthEnabled(), 0);
	context->OMSetDepthStencilState(depthStencilStates.DepthWriteEnabled(), 1);
	context->RSSetState(rasterizerStates.BackFaceCull());
}

void LIDRenderer::BeginRender( BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world, UINT32 numTilesX, UINT32 numTilesY )
{
	float blendFactor[4] = {1, 1, 1, 1};
	context->OMSetBlendState(blendStates.BlendDisabled(), blendFactor, 0xFFFFFFFF);
	context->OMSetDepthStencilState(depthStencilStates.DepthWriteEnabled(), 1);
	context->RSSetState(rasterizerStates.BackFaceCull());

	/*ID3D11SamplerState* sampStates[1] = {
		samplerStates.Anisotropic(),
	};
	context->PSSetSamplers(0, 1, sampStates);*/

	/*ID3D11ShaderResourceView* srvs[2] = {
		lightsBuffer, lightIndicesBuffer
	};
	context->PSSetShaderResources(2, 2, srvs);*/

	//XMMATRIX proj(pCam->getProjectionMatrix());
	//XMMATRIX view(pCam->getViewMatrix());

	// Set constant buffers	
	/*memcpy(&meshVSConstants.Data.World, &XMMatrixTranspose(world), SIZE16);	
	memcpy(&meshVSConstants.Data.View, &XMMatrixTranspose(view), SIZE16);
	XMMATRIX wvp = world * view * proj;
	memcpy(&meshVSConstants.Data.WorldViewProjection, &XMMatrixTranspose(wvp), SIZE16);	
	meshVSConstants.ApplyChanges(context);
	meshVSConstants.SetVS(context, 0);*/

	float di = 0.f;//AppSettings::DiffuseIntensity;
	float si = 0.02f;//AppSettings::SpecularIntensity;

	float eyePos[3];
	pCam->getFrom(eyePos);
	meshPSConstants.Data.CameraPosWS = XMFLOAT3(eyePos);		
	//meshPSConstants.Data.DiffuseAlbedo = XMFLOAT3(AppSettings::DiffuseR * di, AppSettings::DiffuseG * di, AppSettings::DiffuseB * di);
	//meshPSConstants.Data.SpecularAlbedo = XMFLOAT3(AppSettings::SpecularR * si, AppSettings::SpecularG * si, AppSettings::SpecularB * si);
	meshPSConstants.Data.DiffuseAlbedo = XMFLOAT3(0.0f, 0.0f, 0.0f);
	meshPSConstants.Data.SpecularAlbedo = XMFLOAT3(0.02f, 0.02f, 0.02f);
	meshPSConstants.Data.Balance = 0.5f;//AppSettings::Balance;
	meshPSConstants.Data.Roughness = 0.5f;//AppSettings::Roughness;
	meshPSConstants.Data.NumTilesX = numTilesX;
	meshPSConstants.Data.NumTilesY = numTilesY;
	meshPSConstants.ApplyChanges(context);
	meshPSConstants.SetPS(context, 0);
}

void LIDRenderer::EndRender()
{


}


void LIDRenderer::Render2( CDXUTSDKMesh* pMesh, BaseCamera* pCam, ID3D11DeviceContext* context, const XMMATRIX& world, ID3D11ShaderResourceView* lightsBuffer, ID3D11ShaderResourceView* lightIndicesBuffer, UINT32 numTilesX, UINT32 numTilesY )
{
	// Set states
	/*float blendFactor[4] = {1, 1, 1, 1};
	context->OMSetBlendState(blendStates.BlendDisabled(), blendFactor, 0xFFFFFFFF);

	context->OMSetDepthStencilState(depthStencilStates.DepthEnabled(), 0);

	context->RSSetState(rasterizerStates.BackFaceCull());*/

	ID3D11SamplerState* sampStates[1] = {
		samplerStates.Anisotropic(),
	};
	context->PSSetSamplers(0, 1, sampStates);

	ID3D11ShaderResourceView* srvs[2] = {
		lightsBuffer, lightIndicesBuffer
	};
	context->PSSetShaderResources(2, 2, srvs);

	XMMATRIX proj(pCam->getProjectionMatrix());
	XMMATRIX view(pCam->getViewMatrix());

	// Set constant buffers	
	memcpy(&meshVSConstants.Data.World, &XMMatrixTranspose(world), SIZE16);	
	memcpy(&meshVSConstants.Data.View, &XMMatrixTranspose(view), SIZE16);
	XMMATRIX wvp = world * view * proj;
	memcpy(&meshVSConstants.Data.WorldViewProjection, &XMMatrixTranspose(wvp), SIZE16);	
	meshVSConstants.ApplyChanges(context);
	meshVSConstants.SetVS(context, 0);

	float di = 0.f;//AppSettings::DiffuseIntensity;
	float si = 0.02f;//AppSettings::SpecularIntensity;

	float eyePos[3];
	pCam->getFrom(eyePos);
	meshPSConstants.Data.CameraPosWS = XMFLOAT3(eyePos);		
	//meshPSConstants.Data.DiffuseAlbedo = XMFLOAT3(AppSettings::DiffuseR * di, AppSettings::DiffuseG * di, AppSettings::DiffuseB * di);
	//meshPSConstants.Data.SpecularAlbedo = XMFLOAT3(AppSettings::SpecularR * si, AppSettings::SpecularG * si, AppSettings::SpecularB * si);
	meshPSConstants.Data.DiffuseAlbedo = XMFLOAT3(0.0f, 0.0f, 0.0f);
	meshPSConstants.Data.SpecularAlbedo = XMFLOAT3(0.02f, 0.02f, 0.02f);
	meshPSConstants.Data.Balance = 0.5f;//AppSettings::Balance;
	meshPSConstants.Data.Roughness = 0.5f;//AppSettings::Roughness;
	meshPSConstants.Data.NumTilesX = numTilesX;
	meshPSConstants.Data.NumTilesY = numTilesY;
	meshPSConstants.ApplyChanges(context);
	meshPSConstants.SetPS(context, 0);

	// Set shaders
	context->DSSetShader(NULL, NULL, 0);
	context->HSSetShader(NULL, NULL, 0);
	context->GSSetShader(NULL, NULL, 0);
	context->VSSetShader(meshVS[AppSettings::NumLights], NULL, 0);
	context->PSSetShader(meshPS[AppSettings::NumLights], NULL, 0);

	// Draw all meshes
	meshPSConstants.Data.EnableNormalMapping = 1;
	meshPSConstants.ApplyChanges(context);	

	context->IASetInputLayout(meshInputLayout);

	//Draw();	
	pMesh->Render(context, 0, 1);

	ID3D11ShaderResourceView* nullSRVs[5] = { NULL };
	context->PSSetShaderResources(0, 5, nullSRVs);
}

void LIDRenderer::UpdateMaterial( ID3D11DeviceContext* context, const noVec4 e, const noVec4 o /*= noVec4(1.0f, 1.0f, 1.0f, 1.0f) */, const noVec3& diffuse /*= noVec3(0.5f, 0.5f, 0.5f)*/, int nSlot )
{
	matConstants.Data.Emissive = e;
	matConstants.Data.Diffuse = noVec4(diffuse, o.w);

	matConstants.Data.Specular = vec4_zero;
	matConstants.Data.SpecularPower = 4.0f;
	matConstants.Data.Amibient = noVec4(0.5f, 0.5f, 0.5f, 1.0f);
	matConstants.ApplyChanges(context);
	//matConstants.SetVS(context, 2);
	matConstants.SetPS(context, nSlot);
}

