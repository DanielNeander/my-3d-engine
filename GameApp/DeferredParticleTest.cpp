#include "stdafx.h"
#include "GameApp.h"
#include <GameApp/Util/EffectUtil.h>

UINT                                g_NumParticles = 200;
float                               g_fSpread = 4.0f;
float                               g_fStartSize = 0.0f;
float                               g_fEndSize = 10.0f;
float                               g_fSizeExponent = 128.0f;

float                               g_fMushroomCloudLifeSpan = 10.0f;
float                               g_fGroundBurstLifeSpan = 9.0f;
float                               g_fPopperLifeSpan = 9.0f;


float                               g_fMushroomStartSpeed = 20.0f;
float                               g_fStalkStartSpeed = 50.0f;
float                               g_fGroundBurstStartSpeed = 100.0f;
float                               g_fLandMineStartSpeed = 250.0f;

float                               g_fEndSpeed = 4.0f;
float                               g_fSpeedExponent = 32.0f;
float                               g_fFadeExponent = 4.0f;
float                               g_fRollAmount = 0.2f;
float                               g_fWindFalloff = 20.0f;
D3DXVECTOR3                         g_vPosMul( 1,1,1 );
D3DXVECTOR3                         g_vDirMul( 1,1,1 );
D3DXVECTOR3                         g_vWindVel( -2.0f,10.0f,0 );
D3DXVECTOR3                         g_vGravity( 0,-9.8f,0.0f );

float                               g_fGroundPlane = 0.5f;
float                               g_fLightRaise = 1.0f;

float                               g_fWorldBounds = 100.0f;

#define MAX_FLASH_COLORS 4
D3DXVECTOR4 g_vFlashColor[MAX_FLASH_COLORS] =
{
	D3DXVECTOR4( 1.0f, 0.5f, 0.00f, 0.9f ),
	D3DXVECTOR4( 1.0f, 0.3f, 0.05f, 0.9f ),
	D3DXVECTOR4( 1.0f, 0.4f, 0.00f, 0.9f ),
	D3DXVECTOR4( 0.8f, 0.3f, 0.05f, 0.9f )
};

D3DXVECTOR4                         g_vFlashAttenuation( 0,0.0f,3.0f,0 );
D3DXVECTOR4                         g_vMeshLightAttenuation( 0,0,1.5f,0 );
float                               g_fFlashLife = 0.50f;
float                               g_fFlashIntensity = 1000.0f;

UINT                                g_NumParticlesToDraw = 0;

#define MAX_MUSHROOM_CLOUDS 8
#define MAX_GROUND_BURSTS 23
#define MAX_PARTICLE_SYSTEMS 30
#define MAX_FLASH_LIGHTS 8
#define MAX_INSTANCES 200


ID3D11InputLayout*                  g_pVertexLayout = NULL;
ID3D11InputLayout*                  g_pScreenQuadLayout = NULL;
ID3D11InputLayout*                  g_pMeshLayout = NULL;


ID3D11Buffer*                       g_pParticleBuffer = NULL;
ID3D11Buffer*                       g_pScreenQuadVB = NULL;

ID3D11Texture2D*                    g_pOffscreenParticleTex = NULL;
ID3D11ShaderResourceView*           g_pOffscreenParticleSRV = NULL;
ID3D11RenderTargetView*             g_pOffscreenParticleRTV = NULL;
ID3D11Texture2D*                    g_pOffscreenParticleColorTex = NULL;
ID3D11ShaderResourceView*           g_pOffscreenParticleColorSRV = NULL;
ID3D11RenderTargetView*             g_pOffscreenParticleColorRTV = NULL;

ID3D11ShaderResourceView*           g_pParticleTextureSRV = NULL;

ID3DX11Effect*                       g_pEffect10 = NULL;
ID3DX11EffectTechnique*              g_pRenderParticlesToBuffer = NULL;
ID3DX11EffectTechnique*              g_pRenderParticles = NULL;
ID3DX11EffectTechnique*              g_pCompositeParticlesToScene = NULL;
ID3DX11EffectTechnique*              g_pRenderMesh = NULL;
ID3DX11EffectTechnique*              g_pRenderMeshInst = NULL;

ID3DX11EffectShaderResourceVariable* g_ptxDiffuse = NULL;
ID3DX11EffectShaderResourceVariable* g_ptxParticleColor = NULL;
ID3DX11EffectVectorVariable*         g_pLightDir = NULL;
ID3DX11EffectMatrixVariable*         g_pmWorldViewProjection = NULL;
ID3DX11EffectMatrixVariable*         g_pmWorld = NULL;
ID3DX11EffectMatrixVariable*         g_pmInvViewProj = NULL;
ID3DX11EffectScalarVariable*         g_pfTime = NULL;
ID3DX11EffectVectorVariable*         g_pvEyePt = NULL;
ID3DX11EffectVectorVariable*         g_pvRight = NULL;
ID3DX11EffectVectorVariable*         g_pvUp = NULL;
ID3DX11EffectVectorVariable*         g_pvForward = NULL;

ID3DX11EffectScalarVariable*         g_pNumGlowLights = NULL;
ID3DX11EffectVectorVariable*         g_pvGlowLightPosIntensity = NULL;
ID3DX11EffectVectorVariable*         g_pvGlowLightColor = NULL;
ID3DX11EffectVectorVariable*         g_pvGlowLightAttenuation = NULL;
ID3DX11EffectVectorVariable*         g_pvMeshLightAttenuation = NULL;

ID3DX11EffectMatrixVariable*         g_pmViewProj = NULL;
ID3DX11EffectMatrixVariable*         g_pmWorldInst = NULL;


static HRESULT CreateShaders()
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3D10_SHADER_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3D10_SHADER_DEBUG;
#endif

	// Read the D3DX effect file
	CHAR str[MAX_PATH];
	char strMaxGlowLights[MAX_PATH];
	char strMaxInstances[MAX_PATH];
	sprintf_s( strMaxGlowLights, MAX_PATH, "%d", MAX_FLASH_LIGHTS );
	sprintf_s( strMaxInstances, MAX_PATH, "%d", MAX_INSTANCES );
	D3D10_SHADER_MACRO macros[3] =
	{
		{ "MAX_GLOWLIGHTS", strMaxGlowLights },
		{ "MAX_INSTANCES", strMaxInstances },
		{ NULL, NULL }
	};

	LoadEffectFromFile(GetApp()->GetDevice(), "Data/Shaders/DeferredParticles.fx", &g_pEffect10, macros);

	// Obtain technique objects
	g_pRenderParticlesToBuffer = g_pEffect10->GetTechniqueByName( "RenderParticlesToBuffer" );
	g_pRenderParticles = g_pEffect10->GetTechniqueByName( "RenderParticles" );
	g_pCompositeParticlesToScene = g_pEffect10->GetTechniqueByName( "CompositeParticlesToScene" );
	g_pRenderMesh = g_pEffect10->GetTechniqueByName( "RenderMesh" );
	g_pRenderMeshInst = g_pEffect10->GetTechniqueByName( "RenderMeshInst" );
	
	// Obtain variables
	g_ptxDiffuse = g_pEffect10->GetVariableByName( "g_txMeshTexture" )->AsShaderResource();
	g_ptxParticleColor = g_pEffect10->GetVariableByName( "g_txParticleColor" )->AsShaderResource();
	g_pLightDir = g_pEffect10->GetVariableByName( "g_LightDir" )->AsVector();
	g_pmWorldViewProjection = g_pEffect10->GetVariableByName( "g_mWorldViewProjection" )->AsMatrix();
	g_pmWorld = g_pEffect10->GetVariableByName( "g_mWorld" )->AsMatrix();
	g_pmInvViewProj = g_pEffect10->GetVariableByName( "g_mInvViewProj" )->AsMatrix();
	g_pfTime = g_pEffect10->GetVariableByName( "g_fTime" )->AsScalar();
	g_pvEyePt = g_pEffect10->GetVariableByName( "g_vEyePt" )->AsVector();
	g_pvRight = g_pEffect10->GetVariableByName( "g_vRight" )->AsVector();
	g_pvUp = g_pEffect10->GetVariableByName( "g_vUp" )->AsVector();
	g_pvForward = g_pEffect10->GetVariableByName( "g_vForward" )->AsVector();

	g_pNumGlowLights = g_pEffect10->GetVariableByName( "g_NumGlowLights" )->AsScalar();
	g_pvGlowLightPosIntensity = g_pEffect10->GetVariableByName( "g_vGlowLightPosIntensity" )->AsVector();
	g_pvGlowLightColor = g_pEffect10->GetVariableByName( "g_vGlowLightColor" )->AsVector();
	g_pvGlowLightAttenuation = g_pEffect10->GetVariableByName( "g_vGlowLightAttenuation" )->AsVector();
	g_pvMeshLightAttenuation = g_pEffect10->GetVariableByName( "g_vMeshLightAttenuation" )->AsVector();

	g_pmWorldInst = g_pEffect10->GetVariableByName( "g_mWorldInst" )->AsMatrix();
	g_pmViewProj = g_pEffect10->GetVariableByName( "g_mViewProj" )->AsMatrix();

	return hr;
}

static void CreateMeshes(CParticleSystem** ppParticleSystem_)
{
	HRESULT hr;
	// Create our vertex input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "LIFE",      0, DXGI_FORMAT_R32_FLOAT,       0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "THETA",     0, DXGI_FORMAT_R32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",     0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3D11DeviceContext* pContext = GetApp()->GetContext();
	ID3D11Device* device = GetApp()->GetDevice();

	D3DX11_PASS_DESC PassDesc;
	 g_pRenderParticlesToBuffer->GetPassByIndex( 0 )->GetDesc( &PassDesc ) ;
	 device->CreateInputLayout( layout, 5, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &g_pVertexLayout ) ;


	 const D3D11_INPUT_ELEMENT_DESC screenlayout[] =
	 {
		 { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	 };
	 g_pCompositeParticlesToScene->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	 device->CreateInputLayout( screenlayout, 1, PassDesc.pIAInputSignature,
		 PassDesc.IAInputSignatureSize, &g_pScreenQuadLayout );

	 const D3D11_INPUT_ELEMENT_DESC meshlayout[] =
	 {
		 { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		 { "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		 { "TEXCOORD",  0, DXGI_FORMAT_R32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	 };
	 g_pRenderMesh->GetPassByIndex( 0 )->GetDesc( &PassDesc );
	 device->CreateInputLayout( meshlayout, 3, PassDesc.pIAInputSignature,
		 PassDesc.IAInputSignatureSize, &g_pMeshLayout );

	 // Particle system
	 UINT NumStalkParticles = 500;
	 UINT NumGroundExpParticles = 345;
	 UINT NumLandMineParticles = 125;
	 UINT MaxParticles = MAX_MUSHROOM_CLOUDS * ( g_NumParticles + NumStalkParticles ) +
		 ( MAX_GROUND_BURSTS - MAX_MUSHROOM_CLOUDS ) * NumGroundExpParticles +
		 ( MAX_PARTICLE_SYSTEMS - MAX_GROUND_BURSTS ) * NumLandMineParticles;
	  CreateParticleArray( MaxParticles ) ;

	 D3DXVECTOR4 vColor0( 1.0f,1.0f,1.0f,1 );
	 D3DXVECTOR4 vColor1( 0.6f,0.6f,0.6f,1 );

	 srand( timeGetTime() );
	 
	 g_NumParticlesToDraw = 0;
	 for( UINT i = 0; i < MAX_MUSHROOM_CLOUDS; i += 2 )
	 {
		 D3DXVECTOR3 vLocation;
		 vLocation.x = RPercent() * 50.0f;
		 vLocation.y = g_fGroundPlane;
		 vLocation.z = RPercent() * 50.0f;

		 ppParticleSystem_[i] = new CMushroomParticleSystem();
		 ppParticleSystem_[i]->CreateParticleSystem( g_NumParticles );
		 ppParticleSystem_[i]->SetSystemAttributes( vLocation,
			 g_fSpread, g_fMushroomCloudLifeSpan, g_fFadeExponent,
			 g_fStartSize, g_fEndSize, g_fSizeExponent,
			 g_fMushroomStartSpeed, g_fEndSpeed, g_fSpeedExponent,
			 g_fRollAmount, g_fWindFalloff,
			 1, 0, D3DXVECTOR3( 0, 0, 0 ), D3DXVECTOR3( 0, 0, 0 ),
			 vColor0, vColor1,
			 g_vPosMul, g_vDirMul );

		 g_NumParticlesToDraw += g_NumParticles;

		 ppParticleSystem_[i + 1] = new CStalkParticleSystem();
		 ppParticleSystem_[i + 1]->CreateParticleSystem( NumStalkParticles );
		 ppParticleSystem_[i + 1]->SetSystemAttributes( vLocation,
			 15.0f, g_fMushroomCloudLifeSpan, g_fFadeExponent * 2.0f,
			 g_fStartSize * 0.5f, g_fEndSize * 0.5f, g_fSizeExponent,
			 g_fStalkStartSpeed, -1.0f, g_fSpeedExponent,
			 g_fRollAmount, g_fWindFalloff,
			 1, 0, D3DXVECTOR3( 0, 0, 0 ), D3DXVECTOR3( 0, 0, 0 ),
			 vColor0, vColor1,
			 D3DXVECTOR3( 1, 0.1f, 1 ), D3DXVECTOR3( 1, 0.1f, 1 ) );

		 g_NumParticlesToDraw += NumStalkParticles;
	 }

	 for( UINT i = MAX_MUSHROOM_CLOUDS; i < MAX_GROUND_BURSTS; i++ )
	 {
		 D3DXVECTOR3 vLocation;
		 vLocation.x = RPercent() * 50.0f;
		 vLocation.y = g_fGroundPlane;
		 vLocation.z = RPercent() * 50.0f;

		 ppParticleSystem_[i] = new CGroundBurstParticleSystem();
		 ppParticleSystem_[i]->CreateParticleSystem( NumGroundExpParticles );
		 ppParticleSystem_[i]->SetSystemAttributes( vLocation,
			 1.0f, g_fGroundBurstLifeSpan, g_fFadeExponent,
			 0.5f, 8.0f, 1.0f,
			 g_fGroundBurstStartSpeed, g_fEndSpeed, 4.0f,
			 g_fRollAmount, 1.0f,
			 30, 100.0f, D3DXVECTOR3( 0, 0.5f, 0 ), D3DXVECTOR3( 1.0f, 0.5f,
			 1.0f ),
			 vColor0, vColor1,
			 g_vPosMul, g_vDirMul );

		 g_NumParticlesToDraw += NumGroundExpParticles;
	 }

	 for( UINT i = MAX_GROUND_BURSTS; i < MAX_PARTICLE_SYSTEMS; i++ )
	 {
		 D3DXVECTOR3 vLocation;
		 vLocation.x = RPercent() * 50.0f;
		 vLocation.y = g_fGroundPlane;
		 vLocation.z = RPercent() * 50.0f;

		 ppParticleSystem_[i] = new CLandMineParticleSystem();
		 ppParticleSystem_[i]->CreateParticleSystem( NumLandMineParticles );
		 ppParticleSystem_[i]->SetSystemAttributes( vLocation,
			 1.5f, g_fPopperLifeSpan, g_fFadeExponent,
			 1.0f, 6.0f, 1.0f,
			 g_fLandMineStartSpeed, g_fEndSpeed, 2.0f,
			 g_fRollAmount, 4.0f,
			 0, 70.0f, D3DXVECTOR3( 0, 0.8f, 0 ), D3DXVECTOR3( 0.3f, 0.2f,
			 0.3f ),
			 vColor0, vColor1,
			 g_vPosMul, g_vDirMul );

		 g_NumParticlesToDraw += NumGroundExpParticles;
	 }

	 D3D11_BUFFER_DESC BDesc;
	 BDesc.ByteWidth = sizeof( PARTICLE_VERTEX ) * 6 * g_NumParticlesToDraw;
	 BDesc.Usage = D3D11_USAGE_DYNAMIC;
	 BDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	 BDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	 BDesc.MiscFlags = 0;
	 device->CreateBuffer( &BDesc, NULL, &g_pParticleBuffer );

	 
	 D3DX11CreateShaderResourceViewFromFile( device, "Textures/DeferredParticle.dds", NULL, NULL, 
		 &g_pParticleTextureSRV, NULL );
	 	 
	 // Create the screen quad
	 BDesc.ByteWidth = 4 * sizeof( D3DXVECTOR3 );
	 BDesc.Usage = D3D11_USAGE_IMMUTABLE;
	 BDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	 BDesc.CPUAccessFlags = 0;
	 BDesc.MiscFlags = 0;

	 D3DXVECTOR3 verts[4] =
	 {
		 D3DXVECTOR3( -1, -1, 0.5f ),
		 D3DXVECTOR3( -1, 1, 0.5f ),
		 D3DXVECTOR3( 1, -1, 0.5f ),
		 D3DXVECTOR3( 1, 1, 0.5f )
	 };
	 D3D11_SUBRESOURCE_DATA InitData;
	 InitData.pSysMem = verts;
	 device->CreateBuffer( &BDesc, &InitData, &g_pScreenQuadVB );

}

void App::LoadParticles()
{
	D3DXVECTOR3 vecEye( 0.0f, 150.0f, 336.0f );
	D3DXVECTOR3 vecAt ( 0.0f, 0.0f, 0.0f );
	D3DXVECTOR3 vUp = D3DXVECTOR3( 0,1,0 );
	DefCam_->setFrom(vecEye);
	DefCam_->setTo(vecAt);	
	DefCam_->setUp(vUp);
	DefCam_->setFar(4000.f);
	DefCam_->setNear(2.0f);
	DefCam_->setFOV( 45.f);;
	DefCam_->computeModelView();
	DefCam_->ComputeProjection();

	ppParticleSystem_ = new CParticleSystem*[MAX_PARTICLE_SYSTEMS];
	CreateShaders();
	CreateMeshes(ppParticleSystem_);

}

void App::UpdateParticles(float fTime, float fElapsedTime)
{
	if (fElapsedTime > 0.1f) fElapsedTime = 0.1f;

	
	D3DXVECTOR3 vEye;
	DefCam_->getFrom(vEye);
	XMMATRIX mView;
	DefCam_->getViewMatrix(mView);
	D3DXVECTOR3 vRight( mView._11, mView._21, mView._31 );
	D3DXVECTOR3 vUp( mView._12, mView._22, mView._32 );
	D3DXVECTOR3 vFoward( mView._13, mView._23, mView._33 );

	D3DXVec3Normalize( &vRight, &vRight );
	D3DXVec3Normalize( &vUp, &vUp );
	D3DXVec3Normalize( &vFoward, &vFoward );

	g_pvRight->SetFloatVector( ( float* )&vRight );
	g_pvUp->SetFloatVector( ( float* )&vUp );
	g_pvForward->SetFloatVector( ( float* )&vFoward );

	UINT NumActiveSystems = 0;
	D3DXVECTOR4 vGlowLightPosIntensity[MAX_PARTICLE_SYSTEMS];
	D3DXVECTOR4 vGlowLightColor[MAX_PARTICLE_SYSTEMS];

	// Advance the system
	for( UINT i = 0; i < MAX_PARTICLE_SYSTEMS; i++ )
	{
		ppParticleSystem_[i]->AdvanceSystem( ( float )fTime, fElapsedTime, vRight, vUp, g_vWindVel, g_vGravity );
	}

	PARTICLE_VERTEX* pVerts = NULL;
	
	ID3D11Resource* pRes = g_pParticleBuffer;;
	D3D11_MAPPED_SUBRESOURCE Data;
	
	GetApp()->GetContext()->Map( pRes, 0, D3D11_MAP_WRITE_DISCARD, 0, &Data );	
	pVerts = (PARTICLE_VERTEX*)Data.pData;
	CopyParticlesToVertexBuffer( pVerts, vEye, vRight, vUp );

	GetApp()->GetContext()->Unmap(pRes, 0);


	for( UINT i = 0; i < MAX_MUSHROOM_CLOUDS; i += 2 )
	{
		float fCurrentTime = ppParticleSystem_[i]->GetCurrentTime();
		float fLifeSpan = ppParticleSystem_[i]->GetLifeSpan();
		if( fCurrentTime > fLifeSpan )
		{
			D3DXVECTOR3 vCenter;
			vCenter.x = RPercent() * g_fWorldBounds;
			vCenter.y = g_fGroundPlane;
			vCenter.z = RPercent() * g_fWorldBounds;
			float fStartTime = -fabs( RPercent() ) * 4.0f;
			D3DXVECTOR4 vFlashColor = g_vFlashColor[ rand() % MAX_FLASH_COLORS ];

			ppParticleSystem_[i]->SetCenter( vCenter );
			ppParticleSystem_[i]->SetStartTime( fStartTime );
			ppParticleSystem_[i]->SetFlashColor( vFlashColor );
			ppParticleSystem_[i]->Init();

			ppParticleSystem_[i + 1]->SetCenter( vCenter );
			ppParticleSystem_[i + 1]->SetStartTime( fStartTime );
			ppParticleSystem_[i + 1]->SetFlashColor( vFlashColor );
			ppParticleSystem_[i + 1]->Init();
		}
		else if( fCurrentTime > 0.0f && fCurrentTime < g_fFlashLife && NumActiveSystems < MAX_FLASH_LIGHTS )
		{
			D3DXVECTOR3 vCenter = ppParticleSystem_[i]->GetCenter();
			D3DXVECTOR4 vFlashColor = ppParticleSystem_[i]->GetFlashColor();

			float fIntensity = g_fFlashIntensity * ( ( g_fFlashLife - fCurrentTime ) / g_fFlashLife );
			vGlowLightPosIntensity[NumActiveSystems] = D3DXVECTOR4( vCenter.x, vCenter.y + g_fLightRaise, vCenter.z,
				fIntensity );
			vGlowLightColor[NumActiveSystems] = vFlashColor;
			NumActiveSystems ++;
		}
	}

	// Ground bursts
	for( UINT i = MAX_MUSHROOM_CLOUDS; i < MAX_GROUND_BURSTS; i++ )
	{
		float fCurrentTime = ppParticleSystem_[i]->GetCurrentTime();
		float fLifeSpan = ppParticleSystem_[i]->GetLifeSpan();
		if( fCurrentTime > fLifeSpan )
		{
			D3DXVECTOR3 vCenter;
			vCenter.x = RPercent() * g_fWorldBounds;
			vCenter.y = g_fGroundPlane;
			vCenter.z = RPercent() * g_fWorldBounds;
			float fStartTime = -fabs( RPercent() ) * 4.0f;
			D3DXVECTOR4 vFlashColor = g_vFlashColor[ rand() % MAX_FLASH_COLORS ];

			float fStartSpeed = g_fGroundBurstStartSpeed + RPercent() * 30.0f;
			ppParticleSystem_[i]->SetCenter( vCenter );
			ppParticleSystem_[i]->SetStartTime( fStartTime );
			ppParticleSystem_[i]->SetStartSpeed( fStartSpeed );
			ppParticleSystem_[i]->SetFlashColor( vFlashColor );
			ppParticleSystem_[i]->Init();
		}
		else if( fCurrentTime > 0.0f && fCurrentTime < g_fFlashLife && NumActiveSystems < MAX_FLASH_LIGHTS )
		{
			D3DXVECTOR3 vCenter = ppParticleSystem_[i]->GetCenter();
			D3DXVECTOR4 vFlashColor = ppParticleSystem_[i]->GetFlashColor();

			float fIntensity = g_fFlashIntensity * ( ( g_fFlashLife - fCurrentTime ) / g_fFlashLife );
			vGlowLightPosIntensity[NumActiveSystems] = D3DXVECTOR4( vCenter.x, vCenter.y + g_fLightRaise, vCenter.z,
				fIntensity );
			vGlowLightColor[NumActiveSystems] = vFlashColor;
			NumActiveSystems ++;
		}
	}

	// Land mines
	for( UINT i = MAX_GROUND_BURSTS; i < MAX_PARTICLE_SYSTEMS; i++ )
	{
		float fCurrentTime = ppParticleSystem_[i]->GetCurrentTime();
		float fLifeSpan = ppParticleSystem_[i]->GetLifeSpan();
		if( fCurrentTime > fLifeSpan )
		{
			D3DXVECTOR3 vCenter;
			vCenter.x = RPercent() * g_fWorldBounds;
			vCenter.y = g_fGroundPlane;
			vCenter.z = RPercent() * g_fWorldBounds;
			float fStartTime = -fabs( RPercent() ) * 4.0f;
			D3DXVECTOR4 vFlashColor = g_vFlashColor[ rand() % MAX_FLASH_COLORS ];

			float fStartSpeed = g_fLandMineStartSpeed + RPercent() * 100.0f;
			ppParticleSystem_[i]->SetCenter( vCenter );
			ppParticleSystem_[i]->SetStartTime( fStartTime );
			ppParticleSystem_[i]->SetStartSpeed( fStartSpeed );
			ppParticleSystem_[i]->SetFlashColor( vFlashColor );
			ppParticleSystem_[i]->Init();
		}
		else if( fCurrentTime > 0.0f && fCurrentTime < g_fFlashLife && NumActiveSystems < MAX_FLASH_LIGHTS )
		{
			D3DXVECTOR3 vCenter = ppParticleSystem_[i]->GetCenter();
			D3DXVECTOR4 vFlashColor = ppParticleSystem_[i]->GetFlashColor();

			float fIntensity = g_fFlashIntensity * ( ( g_fFlashLife - fCurrentTime ) / g_fFlashLife );
			vGlowLightPosIntensity[NumActiveSystems] = D3DXVECTOR4( vCenter.x, vCenter.y + g_fLightRaise, vCenter.z,
				fIntensity );
			vGlowLightColor[NumActiveSystems] = vFlashColor;
			NumActiveSystems ++;
		}
	}

	// Setup light variables
	g_pNumGlowLights->SetInt( NumActiveSystems );
	g_pvGlowLightPosIntensity->SetFloatVectorArray( ( float* )&vGlowLightPosIntensity, 0, NumActiveSystems );
	g_pvGlowLightColor->SetFloatVectorArray( ( float* )&vGlowLightColor, 0, NumActiveSystems );
	g_pvGlowLightAttenuation->SetFloatVector( ( float* )&g_vFlashAttenuation );
	g_pvMeshLightAttenuation->SetFloatVector( ( float* )&g_vMeshLightAttenuation );

	
}

static void RenderParticles( ID3D11DeviceContext* pd3dDevice, ID3DX11EffectTechnique* pRenderTechnique )
{
	//IA setup
	pd3dDevice->IASetInputLayout( g_pVertexLayout );
	UINT Strides[1];
	UINT Offsets[1];
	ID3D11Buffer* pVB[1];
	pVB[0] = g_pParticleBuffer;
	Strides[0] = sizeof( PARTICLE_VERTEX );
	Offsets[0] = 0;
	pd3dDevice->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
	pd3dDevice->IASetIndexBuffer( NULL, DXGI_FORMAT_R16_UINT, 0 );
	pd3dDevice->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	g_ptxDiffuse->SetResource( g_pParticleTextureSRV );

	//Render
	D3DX11_TECHNIQUE_DESC techDesc;
	pRenderTechnique->GetDesc( &techDesc );

	g_NumParticlesToDraw = GetNumActiveParticles();
	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		pRenderTechnique->GetPassByIndex( p )->Apply( 0, GetApp()->GetContext() );
		pd3dDevice->Draw( 6 * g_NumParticlesToDraw, 0 );
	}
}

void App::RenderParticles(float fTime)
{
	HRESULT hr;

	renderer->changeToMainFramebuffer();

	ID3D11RenderTargetView* pRTV;
	ID3D11DepthStencilView* pDSV;

	GetApp()->GetContext()->OMGetRenderTargets(1, &pRTV, &pDSV);

	// Clear the render target and depth stencil
	float ClearColor[4] =
	{
		0.0f, 0.0f, 0.0f, 1.0f
	};
	ID3D11Device* pd3dDevice = GetApp()->GetDevice();
	ID3D11DeviceContext* pd3dContext = GetApp()->GetContext();
	
	//pd3dContext->ClearRenderTargetView( pRTV, ClearColor);
	//pd3dContext->ClearDepthStencilView( pDSV, D3D11_CLEAR_DEPTH, 1.0, 0);

	D3DXVECTOR3 vEyePt;
	D3DXMATRIX mWorldViewProjection;
	D3DXVECTOR4 vLightDir;
	D3DXMATRIX mWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
	D3DXMATRIX mViewProj;
	D3DXMATRIX mInvViewProj;	
	DefCam_->getFrom(vEyePt);

	const unsigned int size16 = sizeof(float) * 16;
	memcpy(&mView, DefCam_->getViewMatrix(), size16);	 
	memcpy(&mProj, DefCam_->getProjectionMatrix(), size16);	 
	
	mWorldViewProjection = mView * mProj;
	mViewProj = mView * mProj;
	D3DXMatrixInverse( &mInvViewProj, NULL, &mViewProj );
	D3DXMATRIX mSceneWorld;
	D3DXMatrixScaling( &mSceneWorld, 20, 20, 20 );
	D3DXMATRIX mSceneWVP = mSceneWorld * mViewProj;

	// Default Light Dir
	D3DXVECTOR3 vDir( 1,1,0 );
	D3DXVec3Normalize( &vDir, &vDir );
	vLightDir = D3DXVECTOR4( vDir, 1 );
	
	// Per frame variables
	V( g_pmWorldViewProjection->SetMatrix( ( float* )&mSceneWVP ) );
	V( g_pmWorld->SetMatrix( ( float* )&mSceneWorld ) );
	V( g_pLightDir->SetFloatVector( ( float* )vLightDir ) );
	V( g_pmInvViewProj->SetMatrix( ( float* )&mInvViewProj ) );
	V( g_pfTime->SetFloat( ( float )fTime ) );
	V( g_pvEyePt->SetFloatVector( ( float* )&vEyePt ) );
	V( g_pmViewProj->SetMatrix( ( float* )&mViewProj ) );

	// Set our input layout	
	//pd3dContext->IASetInputLayout( g_pMeshLayout );

	// Render particles
	V( g_pmWorldViewProjection->SetMatrix( ( float* )&mWorldViewProjection ) );
	V( g_pmWorld->SetMatrix( ( float* )&mWorld ) );

	::RenderParticles( pd3dContext, g_pRenderParticles );
}