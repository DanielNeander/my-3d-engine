#include "stdafx.h"


#include "ShadowMapApp.h"
#include "QuadMesh.h"

// 2011/09/26
// Recreate시 VB, IB Release 필요...


using namespace luabind;

IDirect3DDevice9*	gd3dDevice;
ShadowMapApp* ShadowMapApp::ms_pSingleton = NULL;


static U2Dx9FxShaderEffect* CreateSkyEffect(U2Dx9Renderer* pRenderer)
{
	U2FrameShader& frameShader = U2SceneMgr::Instance()->
		m_spFrame->GetShader(_T("static"));

	//select mesh loader
	U2D3DXEffectShader* pShader = frameShader.GetShader();

	uint32 i;	
	{		
		U2Dx9FxShaderEffect *pEffect = U2_NEW U2Dx9FxShaderEffect(pShader, pRenderer);	
		/*pEffect->SetTexture(U2FxShaderState::DiffMap0, _T("system/white.dds"));
		pEffect->SetTexture(U2FxShaderState::BumpMap0, _T("system/nobump.dds"));
		pEffect->SetVector(U2FxShaderState::MatDiffuse, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
		pEffect->SetVector(U2FxShaderState::MatEmissive, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));
		pEffect->SetFloat(U2FxShaderState::MatEmissiveIntensity, 1.0f);
		pEffect->SetVector(U2FxShaderState::MatSpecular, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
		pEffect->SetFloat(U2FxShaderState::MatSpecularPower, 32.0f);*/

		pEffect->SetInt(U2FxShaderState::CullMode, 1);	
		pEffect->SetFloat(U2FxShaderState::BumpScale, 0.0f);

		pEffect->SetFrameShaderName(frameShader.GetName());
	}	
}



static void AttachMeshGroup(U2SceneMgr* pMgr, U2N2Mesh* pMesh)
{
	pMgr->AttachMeshGroup(pMesh);
}


static ShadowMapApp* GetApp()
{
	return ShadowMapApp::ms_pSingleton;
}

static void AddObject(U2Scene *pScene, U2N2Mesh* pMesh)
{
	pScene->AddObject(pMesh);
}

static U2SceneMgr* GetSceneMgr(ShadowMapApp* pApp)
{
	return U2SceneMgr::Instance();
}

static U2Scene* GetScene(ShadowMapApp* pApp)
{
	return pApp->GetScene();
}

static void BindApp(lua_State* pLua)
{	

	module(pLua)
		[
			def("GetApp", GetApp)
		];

	module(pLua)
		[
			class_<U2SceneMgr>("U2SceneMgr")			
			.def("AttachMeshGroup", AttachMeshGroup)			
		];

	module(pLua)
		[
			class_<U2Scene>("U2Scene")
			.def("AddObject", AddObject)
		];

	module(pLua)
		[
			class_<ShadowMapApp>("ShadowMapApp")

			.def("GetScene", GetScene)
			.def("GetSceneMgr", GetSceneMgr)
		];


}


ShadowMapApp* ShadowMapApp::Create()
{
	U2App::Run = &WinApp::Run;
	ms_pSingleton = U2_NEW ShadowMapApp;	
	TheApplication = ms_pSingleton;
	return ms_pSingleton;
}

ShadowMapApp::ShadowMapApp()
:WinApp(_T("ShadowMapApp"), 0, 0, 1280, 960, false)
{
	m_spRootNode = 0;
	m_spSceneMgr =  0;
	m_spScene = 0;	
	m_spLitSphere = 0;
	m_spMesh = 0;
	m_pTerrain = 0;
	m_spTrnNode = 0;
	m_spCamNode = 0;
	m_pPicker = 0;
	m_fVertDist = 5.0f;
	m_acPickedName[0] = 0;
	m_bAllowedCameraMove = false;
}

void ShadowMapApp::OnTerminate ()
{
	U2_DELETE m_pTerrain;
	U2_FREE(m_afCos);
	U2_FREE(m_afSin);
	U2_FREE(m_afTolerance);
	U2_DELETE m_pPicker;
	m_spTrnNode = 0;
	m_spCamNode = 0;	
	m_spMesh = 0;
	m_spLitSphere = 0;
	m_spRootNode = 0;
	m_spScene = 0;		
	m_spSceneMgr =  0;		
	WinApp::OnTerminate();
}


void ShadowMapApp::CreateRenderer()
{
	m_spRenderer = U2Dx9Renderer::Create(m_iWidth, m_iHeight, 
		U2Dx9Renderer::USE_16BITBUFFERS | U2Dx9Renderer::USE_STENCIL, 
		m_hWnd, NULL, false, 0, U2Dx9Renderer::DEVDESC_HAL_HWVERTEX);

	// Shader Loading..
	m_spRenderer->SetBackgroundColor(D3DXCOLOR(0.f, 0.f, 0.f, 1.0f));	
	m_spCamera = U2_NEW U2Camera();
	//m_spRenderer->SetCameraData(m_spCamera);
}

bool ShadowMapApp::ProcessUnbufferedKeyInput()
{
	// Right Hand..
	if(m_pKeyboard->isKeyDown(OIS::KC_A))
		m_trans.x = -m_fMoveScale * 0.01f;

	if(m_pKeyboard->isKeyDown(OIS::KC_D))
		m_trans.x = m_fMoveScale * 0.01f;

	if(m_pKeyboard->isKeyDown(OIS::KC_UP) || m_pKeyboard->isKeyDown(OIS::KC_W))
	{
		//	if (AllowMotion(1.0f))
		{
			m_bAllowedCameraMove = true;
			m_trans.z = +m_fMoveScale * 0.1f;			
		}
	}


	if(m_pKeyboard->isKeyDown(OIS::KC_DOWN) || m_pKeyboard->isKeyDown(OIS::KC_S))
	{
		//	if (AllowMotion(-1.0f))
		{
			m_bAllowedCameraMove = true;
			m_trans.z = -m_fMoveScale * 0.1f;			
		}
	}


	if(m_pKeyboard->isKeyDown(OIS::KC_PGUP))
		m_trans.y = m_fMoveScale;			// Move camera up

	if(m_pKeyboard->isKeyDown(OIS::KC_PGDOWN))
		m_trans.y = -m_fMoveScale;			// Move camera down

	if(m_pKeyboard->isKeyDown(OIS::KC_RIGHT))
		//RotateCamera(UNIT_Y, -m_fRotScale);
		TurnRight();

	if(m_pKeyboard->isKeyDown(OIS::KC_LEFT))
		//RotateCamera(UNIT_Y, m_fRotScale);
		TurnLeft();

	return true;
}

void ShadowMapApp::UpdateCamera()
{
	if (!m_spCamera)
		return;

	D3DXMATRIX xRot, yRot;
	D3DXMatrixRotationAxis(&xRot, &m_spCamera->GetRight(), m_fRotY);	
	RotateCamera(xRot);

	D3DXMatrixRotationAxis(&yRot, &UNIT_Y, m_fRotX);
	RotateCamera(yRot);

	D3DXVECTOR3 eye = m_spCamera->GetEye();	

	if(m_trans.x != 0)
		eye += m_spCamera->GetRight() * m_trans.x;

	if(m_trans.y != 0)
		eye.y += m_trans.y;

	if (m_bAllowedCameraMove)
	{	
		if(m_trans.z != 0)
			eye += m_spCamera->GetDir() * m_trans.z;		
	}

	m_spCamera->SetEye(eye);

	//if (m_bAllowedCameraMove)
	//	AdjuestVerticalDistance();

	m_bAllowedCameraMove = false;
}

bool ShadowMapApp::OnInitialize()
{

	ResetTime();

	if (!m_spCamera)
		m_spCamera = U2_NEW U2Camera();	
	m_spObj = 0;


	CreateInput();
	CreateScene();


	InitCameraMotion(300.0f, 36.f);
	
	m_spCamNode = U2_NEW U2CameraNode(m_spCamera);	
	//m_spCamNode->AttachChild(m_spSceneCreator->GetDefaultLight());
	m_spRootNode->AttachChild(m_spCamNode);


	m_spRootNode->Update();
	m_spRootNode->UpdateRenderStates();	
	m_spRootNode->UpdateBound();

	D3DXVECTOR3 eye = m_spRootNode->m_spWorldBound->GetCenter();

	m_spCamera->SetEye(D3DXVECTOR3(eye.x, eye.y, eye.z));

	m_spCamera->SetFrustum(45.0f, (float)GetWidth() / (float)GetHeight(), 0.1f, 10000.0f);

	m_spRenderer->SetCamera(m_spCamera);
	m_spRenderer->SetProjectionTM(m_spCamera);
	m_spRenderer->SetTransform(U2Dx9Renderer::TMT_PROJ, m_spRenderer->GetProjMat());

	m_culler.SetCamera(m_spCamera);
	m_culler.ComputeVisibleSet(m_spRootNode);

	return true;
}

//-------------------------------------------------------------------------------------------------
void ShadowMapApp::CreateScene ()
{
	// Fix : D3DDevice Reference Count 
	//m_spSceneMgr = U2_NEW U2SceneMgr(m_spRenderer);
	// Fix : D3DDevice Reference Count 
	//m_spSceneMgr->Initialize();	

	// Create Scene Start
	// Create Ground
	m_spRootNode = U2_NEW U2Node;	
	m_spRootNode->SetName(_T("Scene Rroot"));



	m_spSceneMgr = U2SceneMgr::Create(m_spRenderer);

	m_spRootNode->AttachChild(m_spSceneMgr->GetRootNode());	

	m_spScene = U2Scene::Create();

	m_spSceneMgr->GetRootNode()->AttachChild(m_spScene->GetSceneNode());

	m_spSceneMgr->GetRootNode()->SetLocalRot(-U2_HALF_PI, 0.0f, 1.0f, 0.0f);

	
	// 메모리 릭 주의..

	LUASCRIPT* pScript = NULL;
	lua_State* L  = NULL;


	L = ScriptMgr->GetLuaMgr()->masterState;

	BindApp(L);

#if 1
	pScript = ScriptMgr->GetLuaMgr()->CreateScript();

	char meshPath[64] = "data\\models\\betty0_skin.xmesh";
	GetPath(meshPath, 64);
	lua_pushstring(L, meshPath);
	lua_setglobal(L, "relpath");

	lua_pushstring(L, "skinned");
	lua_setglobal(L, "skin_shader_name");

	lua_pushstring(L, "skinned_alpha");
	lua_setglobal(L, "skinnedalpha_shader_name");


	char fullpath[MAX_PATH] = "data\\scripts\\test.lua";

	GetPath(fullpath, MAX_PATH);


	pScript->RunFile(fullpath);

	m_spMeshLoader = 0;	
#endif
	

#if 1
	QuadMesh* pQuad = U2_NEW QuadMesh(NULL);
	pQuad->SetName("Quad1");
	pQuad->Initialize(50, 50, 50);
	pQuad->LightOnOff(false); // 사용안함.. LightShader 적용 수정..

	//// static shader 
	U2FrameShader& frameShader = m_spSceneMgr->m_spFrame->GetShader(_T("static"));

	//select mesh loader
	U2D3DXEffectShader* pShader = frameShader.GetShader();

	uint32 i;
	for(i = 0; i < pQuad->GetNumGroups(); ++i)
	{
		nMeshGroup& group = pQuad->Group(i);

		U2Dx9FxShaderEffect *pEffect = U2_NEW U2Dx9FxShaderEffect(pShader, m_spRenderer);	
		pEffect->SetTexture(U2FxShaderState::DiffMap0, _T("system/white.dds"));
		pEffect->SetTexture(U2FxShaderState::BumpMap0, _T("system/nobump.dds"));
		pEffect->SetVector(U2FxShaderState::MatDiffuse, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
		pEffect->SetVector(U2FxShaderState::MatEmissive, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));
		pEffect->SetFloat(U2FxShaderState::MatEmissiveIntensity, 1.0f);
		pEffect->SetVector(U2FxShaderState::MatSpecular, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
		pEffect->SetFloat(U2FxShaderState::MatSpecularPower, 32.0f);

		pEffect->SetInt(U2FxShaderState::CullMode, 1);	
		pEffect->SetFloat(U2FxShaderState::BumpScale, 0.0f);

		pEffect->SetFrameShaderName(frameShader.GetName());

		group.AttachEffect(pEffect);

		group.SetLocalTrans(-5 * 9 / 2.0f, 0, 0.0);			
	}	

	m_spSceneMgr->AttachMeshGroup(pQuad);
	m_spScene->AddObject(pQuad);
#endif 	

#if 0
	pQuad = U2_NEW QuadMesh(NULL);
	pQuad->SetName("Quad2");
	pQuad->Initialize(10, 10, 5);
	pQuad->LightOnOff(false);
	
	//// static shader 
	U2FrameShader& frameShader2 = m_spSceneMgr->m_spFrame->GetShader(_T("static"));

	//select mesh loader
	pShader = frameShader2.GetShader();

	
	for(i = 0; i < pQuad->GetNumGroups(); ++i)
	{
		nMeshGroup& group = pQuad->Group(i);

		U2Dx9FxShaderEffect *pEffect = U2_NEW U2Dx9FxShaderEffect(pShader, m_spRenderer);	
		pEffect->SetTexture(U2FxShaderState::DiffMap0, _T("system/white.dds"));
		pEffect->SetTexture(U2FxShaderState::BumpMap0, _T("system/nobump.dds"));
		pEffect->SetVector(U2FxShaderState::MatDiffuse, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
		pEffect->SetVector(U2FxShaderState::MatEmissive, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));
		pEffect->SetFloat(U2FxShaderState::MatEmissiveIntensity, 1.0f);
		pEffect->SetVector(U2FxShaderState::MatSpecular, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
		pEffect->SetFloat(U2FxShaderState::MatSpecularPower, 32.0f);

		pEffect->SetInt(U2FxShaderState::CullMode, 1);	
		pEffect->SetFloat(U2FxShaderState::BumpScale, 0.0f);

		pEffect->SetFrameShaderName(frameShader.GetName());

		group.AttachEffect(pEffect);

		group.SetLocalTrans(5 * 9 /2.0f, 0, 0.0);
		group.SetLocalRot(-U2_HALF_PI, 0, 0, 1);	
		
	}	

	m_spSceneMgr->AttachMeshGroup(pQuad);
	m_spScene->AddObject(pQuad);
#endif 	
	
	// Example : Convert X Mesh To U2N2Mesh
	{
		ID3DXMesh* pXMesh;	
		::LoadXFile("data\\models\\skullocc.x", &pXMesh, m_vMtrl, m_vTex);
		
		U2N2Mesh* pMesh = ConvertXToMesh(pXMesh, m_spRenderer->GetD3DDevice());

		U2FrameShader& frameShader = m_spSceneMgr->
			m_spFrame->GetShader(_T("PhongDirLtTexTech"));

		U2Dx9FxShaderEffect *pEffect = U2_NEW U2Dx9FxShaderEffect(pShader, m_spRenderer);	

		for(int j = 0; j < m_vMtrl.size(); ++j)
		{
			D3DXVECTOR4 diffuse(m_vMtrl[j].diffuse);
			pEffect->SetVector(U2FxShaderState::MatDiffuse, diffuse);
			D3DXVECTOR4 ambient(m_vMtrl[j].ambient);
			pEffect->SetVector(U2FxShaderState::MatAmbient, ambient);
			D3DXVECTOR4 spec(m_vMtrl[j].spec);
			pEffect->SetVector(U2FxShaderState::MatSpecular, spec);
			pEffect->SetFloat(U2FxShaderState::MatSpecularPower, m_vMtrl[j].specPower);

			if (!m_vTex[j])
			{
				pEffect->SetTexture(U2FxShaderState::DiffMap0, _T("system/whitetex.dds"));
			}
		}						
		pEffect->SetInt(U2FxShaderState::CullMode, 1);	
		pEffect->SetFrameShaderName(frameShader.GetName());

		for(i = 0; i < pMesh->GetNumGroups(); ++i)
		{
			nMeshGroup& group = pMesh->Group(i);

			group.AttachEffect(pEffect);
		}

		m_spSceneMgr->AttachMeshGroup(pMesh);
		m_spScene->AddObject(pMesh);
	}
	
	
}

//-------------------------------------------------------------------------------------------------
void ShadowMapApp::OnIdle()
{

	MeasureTime();

	U2StopWatch timer;
	timer.Start();

	static uint32 frameId = 0;

	// GetCurrentRenderTarget == NULL ?
	//m_spRenderer->SetCamera(m_spCamera);

	//MoveCamera();
	bool bInput = UpdateInput();


	if (m_spCamNode && bInput)
	{
		m_spCamNode->SetCamera(m_spCamera);				
	}



	//m_culler.ComputeVisibleSet(m_spScene);

	if(m_spSceneMgr)
	{		

		m_culler.ComputeVisibleSet(m_spRootNode);

		// adjust camera planes to contain scene tightly
		//m_spCamera->AdjustPlanes(m_culler.GetVisibleSet());

		//m_spRenderer->DrawScene(m_culler.GetVisibleSet());
		if(m_spScene)
		{
			m_spScene->Trigger(m_fAccumTime, frameId++);

			m_spScene->UpdateRenderContext();
		}		

		//if(MoveObject())
		{
			m_spRootNode->Update(0.f);
			m_spRenderer->SetCamera(m_spCamera);
		}

		D3DXMATRIX viewMat = m_spRenderer->GetViewMat();		
		m_spRenderer->SetTransform(U2Dx9Renderer::TMT_VIEW, viewMat);				

		m_spSceneMgr->UpdatePerFrameSharedShaderParams();

		m_spSceneMgr->DrawScene(m_culler.GetVisibleSet());
	}

	m_spRenderer->BeginScene();			


	U2VisibleSet& visibles = m_culler.GetVisibleSet();


	m_spRenderer->SetBackgroundColor(D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f));

	if(visibles.GetCount() == 0)	
		m_spRenderer->BeginDefaultRenderTarget(U2Dx9Renderer::CLEAR_ALL);
	else 
		m_spRenderer->BeginDefaultRenderTarget(U2Dx9Renderer::CLEAR_NONE);



#if 1
	//unsigned int lastDrawHeight = DrawCamera(m_spCamera, 8, 30, COLOR_WHITE);	

	DrawFrameRate(8,GetHeight()-8, COLOR_WHITE);

	//m_spRenderer->DrawFont(320,GetHeight()-8,COLOR_WHITE,m_acPickedName);
#endif

	m_spRenderer->EndRenderTarget();

	m_spRenderer->EndScene();	

	m_spRenderer->DisplayBackBuffer();

	timer.Stop();

	UpdateFrameCount();

}

