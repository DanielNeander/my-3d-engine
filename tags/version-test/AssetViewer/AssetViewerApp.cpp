#include "stdafx.h"
#include "AssetViewerApp.h"
#include "Terrain.h"
#include "TerrainMesh.h"
#include "HeightQuery.h"
#include "QuadMesh.h"

#define CAMERA_TEST

static float g_fSplitPos[10];

AssetViewApp* AssetViewApp::ms_pSingleton = NULL;

using namespace luabind;


static void AttachMeshGroup(U2SceneMgr* pMgr, U2N2Mesh* pMesh)
{
	pMgr->AttachMeshGroup(pMesh);
}


static AssetViewApp* GetApp()
{
	return AssetViewApp::ms_pSingleton;
}

static void AddObject(U2Scene *pScene, U2N2Mesh* pMesh)
{
	pScene->AddObject(pMesh);
}

//static U2SceneMgr* GetSceneMgr()
//{
//	return U2SceneMgr::Instance();
//}

static U2SceneMgr* GetSceneMgr(AssetViewApp* pApp)
{
	return U2SceneMgr::Instance();
}

static U2Scene* GetScene(AssetViewApp* pApp)
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
			class_<AssetViewApp>("AssetViewApp")
			
			.def("GetScene", GetScene)
			.def("GetSceneMgr", GetSceneMgr)
		];

	
}


AssetViewApp* AssetViewApp::Create()
{
	U2App::Run = &WinApp::Run;
	ms_pSingleton = U2_NEW AssetViewApp;	
	TheApplication = ms_pSingleton;
	return ms_pSingleton;
}

AssetViewApp::AssetViewApp()
:WinApp(_T("AssetViewApp"), 0, 0, 1280, 960, false)
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

void AssetViewApp::OnTerminate ()
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


void AssetViewApp::CreateRenderer()
{
	m_spRenderer = U2Dx9Renderer::Create(m_iWidth, m_iHeight, 
		U2Dx9Renderer::USE_16BITBUFFERS | U2Dx9Renderer::USE_STENCIL, 
		m_hWnd, NULL, false, 0, U2Dx9Renderer::DEVDESC_HAL_HWVERTEX);

	// Shader Loading..
	m_spRenderer->SetBackgroundColor(D3DXCOLOR(0.f, 0.f, 0.f, 1.0f));	
	m_spCamera = U2_NEW U2Camera();
	//m_spRenderer->SetCameraData(m_spCamera);
}


void AssetViewApp::AdjuestVerticalDistance()
{
	D3DXVECTOR3 vPos = m_spCamera->GetEye();
	D3DXVECTOR3 vDir = D3DXVECTOR3(0.0f, -1.0f, 0.0f);
	m_pPicker->Execute(m_spRootNode, vPos, vDir, 0.0f, FLT_MAX);

	if (m_pPicker->records.FilledSize() > 0)
	{
		const U2PickRecord& record = m_pPicker->GetClosestNonnegative();
		U2Triangle* pMesh = DynamicCast<U2Triangle>(record.spInterected);
		U2ASSERT(pMesh);

		U2Triangle3 tri;
		uint16 i0, i1, i2;
		pMesh->GetTriangleIndices(record.uiTriangle, i0, i1, i2);
				
		D3DXVECTOR3 *pVert = pMesh->GetVertices();
		tri.v[0] = pMesh->m_tmWorld.Apply(pVert[i0]);
		tri.v[1] = pMesh->m_tmWorld.Apply(pVert[i1]);
		tri.v[2] = pMesh->m_tmWorld.Apply(pVert[i2]);

		D3DXVECTOR3 closestPt = record.fB0 * tri.v[0] + record.fB1 * tri.v[1] 
		+record.fB2 * tri.v[2];

		closestPt.y += m_fVertDist;
		m_spCamera->SetEye(closestPt);
	}
}

bool AssetViewApp::AllowMotion (float fSign)
{
	D3DXVECTOR3 vPos = m_spCamera->GetEye() + fSign * m_fTransSpeed * m_aWorldAxis[0]
	- 0.5f * m_fVertDist * m_aWorldAxis[1];

	for (int i = 0; i < m_iNumRays; i++)
	{
		D3DXVECTOR3 vDir = m_afCos[i] * m_aWorldAxis[2] + 
			fSign * m_afSin[i] * m_aWorldAxis[0];

		m_pPicker->Execute(m_spRootNode, vPos, vDir, 0.0f, FLT_MAX);
		if ( m_pPicker->records.FilledSize() > 0)
		{
			const U2PickRecord& record = m_pPicker->GetClosestNonnegative();
			if (record.fT <= m_afTolerance[i])
			{
				return false;
			}
		}
	}

	return true;
}

bool AssetViewApp::OnMouseClick(int button, int state, int x, int y, 
								 unsigned int uModifiers)
{
	if (state != MOUSE_DOWN)
	{
		return false;
	}

	D3DXVECTOR3 vPos, vDir;
	m_spCamera->GetPickRay(x, y, GetWidth(), GetHeight(), vPos, vDir);
	m_pPicker->Execute(m_spRootNode, vPos, vDir, 0.0f, FLT_MAX);

	if (m_pPicker->records.FilledSize() > 0)
	{
		const U2PickRecord& record = m_pPicker->GetClosestNonnegative();
		U2Spatial* pIntersected = record.spInterected;

		if (pIntersected->GetName().Length() > 0)
		{
			strcpy_s(m_acPickedName, MAX_PATH, pIntersected->GetName());
		}
		else 
		{
			strcpy_s(m_acPickedName, MAX_PATH, _T("Unnamed Object"));
		}
	}
	else 
	{
		strcpy_s(m_acPickedName, MAX_PATH, _T("Not Intersected"));
	}

	return true;

}

bool AssetViewApp::ProcessUnbufferedKeyInput()
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


void AssetViewApp::UpdateCamera()
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


void AssetViewApp::MoveForward()
{
	
	if (AllowMotion(1.0f))
	{
		WinApp::MoveForward();
		AdjuestVerticalDistance();
	}
	
}

void AssetViewApp::MoveBackward()
{
	
	if (AllowMotion(-1.0f))
	{
		WinApp::MoveBackward();
		AdjuestVerticalDistance();
	}
	
}

bool AssetViewApp::OnInitialize()
{
		
	ResetTime();

	if (!m_spCamera)
		m_spCamera = U2_NEW U2Camera();	
	m_spObj = 0;


	CreateInput();
	CreateScene();

	InitCameraMotion(300.0f, 36.f);

	// Start Pick
	m_pPicker = U2_NEW U2Picker();
	m_iNumRays = 5;
	m_fHalfAngle = 0.25f * U2_PI;
	m_afCos = U2_ALLOC(float, m_iNumRays);
	m_afSin = U2_ALLOC(float, m_iNumRays);
	m_afTolerance = U2_ALLOC(float, m_iNumRays);
	float fMult = 1.0f / (m_iNumRays * 0.5f);
	for (int i = 0;  i < m_iNumRays; i++)
	{
		float fUnit = i * fMult - 1.0f;
		float fAngle = U2_HALF_PI + m_fHalfAngle * fUnit;
		m_afCos[i] = U2Math::Cos(fAngle);
		m_afSin[i] = U2Math::Sin(fAngle);
		m_afTolerance[i] = 2.0f - 1.5f * U2Math::FAbs(fUnit);
	}
	// EndPick

	m_spCamNode = U2_NEW U2CameraNode(m_spCamera);	
	//m_spCamNode->AttachChild(m_spSceneCreator->GetDefaultLight());
	m_spRootNode->AttachChild(m_spCamNode);

	

	m_spRootNode->Update();
	m_spRootNode->UpdateRenderStates();	
	m_spRootNode->UpdateBound();

	m_spCamera->SetFrustum(45.0f, (float)GetWidth() / (float)GetHeight(), 0.1f, 10000.0f);

	m_spRenderer->SetCamera(m_spCamera);
	m_spRenderer->SetProjectionTM(m_spCamera);
	m_spRenderer->SetTransform(U2Dx9Renderer::TMT_PROJ, m_spRenderer->GetProjMat());

	m_culler.SetCamera(m_spCamera);
	m_culler.ComputeVisibleSet(m_spRootNode);

	

	return true;
}

//-------------------------------------------------------------------------------------------------
void AssetViewApp::CreateScene ()
{
	// Fix : D3DDevice Reference Count 
	//m_spSceneMgr = U2_NEW U2SceneMgr(m_spRenderer);
	// Fix : D3DDevice Reference Count 
	//m_spSceneMgr->Initialize();	

	// Create Scene Start
	// Create Ground
	m_spRootNode = U2_NEW U2Node;	
	m_spRootNode->SetName(_T("Scene Rroot"));


#if 1
	m_spSceneMgr = U2SceneMgr::Create(m_spRenderer);

	m_spRootNode->AttachChild(m_spSceneMgr->GetRootNode());	

	m_spScene = U2Scene::Create();

	m_spSceneMgr->GetRootNode()->AttachChild(m_spScene->GetSceneNode());
	

	bool success = false;
	

	// Terrain Test 

#if 1

	m_pTerrain = U2_NEW Terrain(m_spCamera);
	m_pTerrain->Initialize(129);

	m_spSceneMgr->AttachMeshGroup(m_pTerrain->GetMesh());
	m_spScene->AddObject(m_pTerrain->GetMesh());


	//m_spScene->AttachChild(m_pTerrain);

#endif

//	pData->SetMeshState(U2MeshData::DYNAMIC_MESH);
	//U2DynamicVBGroup* pVBGroup = (U2DynamicVBGroup*)U2DynamicVBGroup::Create();
	//m_spRenderer->GetVBGroupMgr()->AddMeshToGroup(pVBGroup, pData, true);
	//pData->SetFVF(0);	
	//m_spRenderer->GetVBMgr()->AllocateBuffer(pData, 0);

	// Vertex Declaration
	
	//loader.SetFilename()
	//loader.SetIndexType(U2XMeshLoader::Index16);
	// index type 체크

	//pData->SetVertexStride(loader.GetVertexStride());
	
	// Vertex 성분 체크...	

	
	m_spRootNode->AttachChild(U2SceneMgr::Instance()->m_spRootNode);	
	// 메모리 릭 주의..
	
	LUASCRIPT* pScript = NULL;
	lua_State* L  = NULL;


	L = ScriptMgr->GetLuaMgr()->masterState;

	BindApp(L);

	// Skinning Animation
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
//#else 
//	m_spMesh = CreateSkinned();		
//	m_spSceneMgr->AttachMeshGroup(m_spMesh);
//	m_spSceneCreator->AddObject(m_spMesh);
#endif

	// Parallax Bump mapping
#if 0
	pScript = ScriptMgr->GetLuaMgr()->CreateScript();

	char parallaxmapPath[MAX_PATH] = "data\\scripts\\parallaxmapping.lua";

	GetPath(parallaxmapPath);

	L = ScriptMgr->GetLuaMgr()->masterState;


	lua_pushstring(L, "static");
	lua_setglobal(L, "static_shader_name");

	pScript->RunFile(parallaxmapPath);

#endif

	// Environment Mapping
#if 0
	pScript = ScriptMgr->GetLuaMgr()->CreateScript();

	char envmapPath[MAX_PATH] = "data\\scripts\\envrionmentmapping.lua";

	GetPath(envmapPath);

	L = ScriptMgr->GetLuaMgr()->masterState;

	lua_pushstring(L, "environment");
	lua_setglobal(L, "envmap_shader_name");

	pScript->RunFile(envmapPath);

#endif


	// light mapping(dark mapping)
#if 0
	/*U2N2Mesh* pMesh = CreateTower();						
	m_spSceneMgr->AttachMeshGroup(pMesh);
	m_spSceneCreator->AddObject(pMesh);*/

	//m_spMeshLoader = 0;
	pScript = ScriptMgr->GetLuaMgr()->CreateScript();

	char lightmapPath[MAX_PATH] = "data\\scripts\\lightmap.lua";

	GetPath(lightmapPath, MAX_PATH);

	L = ScriptMgr->GetLuaMgr()->masterState;


	lua_pushstring(L, "lightmapped");
	lua_setglobal(L, "lightmap_shader_name");

	pScript->RunFile(lightmapPath);
#endif


// keyframe animation
#if 0
	CreateBouncingBall();	
#endif

	// Multi textures
#if 0
	pScript = ScriptMgr->GetLuaMgr()->CreateScript();

	char multilayeredPath[MAX_PATH] = "data\\scripts\\multilayered.lua";

	GetPath(multilayeredPath, MAX_PATH);

	L = ScriptMgr->GetLuaMgr()->masterState;


	lua_pushstring(L, "multilayered");
	lua_setglobal(L, "multilayer_shader_name");

	pScript->RunFile(multilayeredPath);

#endif
	
	
	
	//D3DXMATRIX yRot,xRot;
	//D3DXMatrixRotationY(&yRot, DegToRad(180.f));
	//m_spScene->SetLocalRot(yRot);

	//D3DXMatrixRotationX(&xRot, DegToRad(60.f));
	//m_spScene->SetLocalRot(yRot * xRot);

	/*transforminterpcontroller anim_0
		setchannel time
		setlooptype loop
		setanim bouncingball.xanim
		setanimgroup 0*/



	m_spRootNode->UpdateBound();

	//D3DXMATRIX xRot, yRot;
	//D3DXMatrixRotationX(&xRot, DegToRad(180.f));
	//D3DXMatrixRotationY(&yRot, DegToRad(180.f));
	//m_spScene->SetLocalRot(yRot * xRot);
	
	D3DXVECTOR3 center = m_spRootNode->m_spWorldBound->GetCenter();
		
	if(m_spLitSphere)
	{
		m_spLitSphere->SetLocalTrans(0.0f, 30.0f, 0.0f);
	}
	

	D3DXVECTOR3 eye(center);	
	//eye = D3DXVECTOR3(-4, 20, -50);		
	eye.y += 20.0f;
	eye.z -= 30.0f;
	D3DXVECTOR3 dir(0.0f, 0.0f, 1.0f);

	//eye = D3DXVECTOR3(1000, 100, 1000);		
	//D3DXVECTOR3 lookAt(1000, 100, 1005);
	//dir = lookAt - eye;
	//D3DXVec3Normalize(&dir, &dir); 
	
	D3DXVECTOR3 up(0.f, 1.f, 0.f);

	D3DXVECTOR3 right;
	::D3DXVec3Cross(&right, &up, &dir);	// Left

	//::D3DXVec3Cross(&up, &right, &dir);	// Left
	//m_spSceneCreator->GetDefaultLight()->SetLocalTrans(0.0f, 20.f, 0.0f);


	m_spCamera->SetFrame(eye, dir, up, right);


#endif
	
}

//-------------------------------------------------------------------------------------------------
U2N2Mesh* AssetViewApp::CreateTower()
{
	// Init Default Light
	

	U2FilePath fPath;		
	TCHAR fullPath[MAX_PATH];
	// StackString Memory Leak...
	U2DynString includePath(MODEL_PATH);	

	fPath.ConvertToAbs(fullPath, MAX_PATH, _T("tower_bbox.xmesh"), includePath);		
	//fPath.ConvertToAbs(fullPath, MAX_PATH, _T("terrain.xmesh"), includePath);		
	
	m_spMeshLoader = U2_NEW U2XMeshLoader;

	m_spMeshLoader->SetFilename(fullPath);

	U2TriListData *pData = U2_NEW U2TriListData;	
	U2N2Mesh* pMesh = U2_NEW U2N2Mesh(pData);

	pMesh->LightOnOff(false);

	pMesh->LoadFile(m_spMeshLoader);		

	//// static shader 
	U2FrameShader& frameShader = m_spSceneMgr->m_spFrame->GetShader(_T("static"));

	//select mesh loader
	U2D3DXEffectShader* pShader = frameShader.GetShader();

	uint32 i;
	for(i = 0; i < pMesh->GetNumGroups(); ++i)
	{
		nMeshGroup& group = pMesh->Group(i);

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
		group.UpdateMeshState(true);
	}	

	return pMesh;

}

U2N2Mesh* AssetViewApp::CreateTerrain()
{
	return 0;
}

U2N2Mesh* AssetViewApp::CreateBouncingBall()
{
	U2TriListData* pData = U2_NEW U2TriListData;

	//U2FilePath fPath;		
	TCHAR fullPath[MAX_PATH] = _T("data\\models\\bouncingball.xmesh");
	//// StackString Memory Leak...
	//U2DynString includePath(MODEL_PATH);	

	//fPath.ConvertToAbs(fullPath, MAX_PATH, _T("bouncingball.xmesh"), includePath);			

	GetPath( fullPath, MAX_PATH);
	fullPath[MAX_PATH-1] = '\0';

	U2XMeshLoaderPtr spMeshLoader = U2_NEW U2XMeshLoader;

	spMeshLoader->SetFilename(fullPath);

	U2N2Mesh* pMesh = U2_NEW U2N2Mesh(pData);

	pMesh->LightOnOff(false);

	pMesh->LoadFile(spMeshLoader);	

	U2FrameShader& frameShader = m_spSceneMgr->m_spFrame->GetShader(
		_T("static"));

	//select mesh loader
	U2D3DXEffectShader* pStaticShader = frameShader.GetShader();

	U2Node* pRootNode = U2_NEW U2Node;	
	pRootNode->SetName(_T("BouncingBall"));

	U2Node* pModelNode = U2_NEW U2Node;
	pModelNode->SetName(_T("Model"));

	// meshbucket에 삽입하기 위해 필요.. 
	// 현재 그리지는 않음..
	pRootNode->AttachChild(pMesh);
	pRootNode->AttachChild(pModelNode);

	m_spTrnNode = U2_NEW U2Node;
	U2TransformInterpController* pController = U2_NEW U2TransformInterpController;
	pController->SetName(_T("anim_0"));
	pController->SetLoopType(U2AnimLoopType::LOOP);

	//memcpy(fullPath, _T("data\\models\\bouncingball.xanim"), sizeof(fullPath));
	//GetPath(fullPath, MAX_PATH);	
	//fullPath[MAX_PATH-1] = '\0';

	TCHAR filename[MAX_PATH] = _T("bouncingball.xanim");
	pController->SetAnim(filename);
	pController->SetAnimGroup(0);
	pController->SetTarget(m_spTrnNode);
	m_spTrnNode->AttachController(pController);
	m_spTrnNode->SetLocalTrans(0.0f, 8.840729, 0.0f);
	m_spTrnNode->SetLocalScale(3.000000, 3.991129, 3.000000);
	

	pModelNode->AttachChild(m_spTrnNode);
	
	uint32 i;
	for(i = 0; i < pMesh->GetNumGroups(); ++i)
	{
		nMeshGroup& group = pMesh->Group(i);

		// ShaderAttb에 추가	

		U2Dx9FxShaderEffect *pEffect = NULL;

		switch(i)
		{
		case 0: // plane			
			pEffect = U2_NEW U2Dx9FxShaderEffect(pStaticShader, m_spRenderer);		
			pEffect->SetVector(U2FxShaderState::MatDiffuse, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
			pEffect->SetVector(U2FxShaderState::MatEmissive, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));
			pEffect->SetFloat(U2FxShaderState::MatEmissiveIntensity, 1.0f);
			pEffect->SetVector(U2FxShaderState::MatSpecular, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
			pEffect->SetFloat(U2FxShaderState::MatSpecularPower, 72.627998);
			//pEffect->SetInt(U2FxShaderState::CullMode, 2);	// CW right가 (-1, 0, 0)일 때
			pEffect->SetInt(U2FxShaderState::CullMode, 3);		//CCW
			pEffect->SetFloat(U2FxShaderState::BumpScale, 0.0f);
			pEffect->SetFrameShaderName(frameShader.GetName());
			pEffect->SetTexture(U2FxShaderState::DiffMap0, _T("materials/checker.dds"));
			pEffect->SetTexture(U2FxShaderState::BumpMap0, _T("materials/checker_bump.dds"));

			// 현재 ball의 애니메이션을 플레이하면 플레이하기 전보다 화면 쪽에 들어가서 
			// 플레이하므로 plane의 위치를 수정..
			group.SetLocalTrans(0.0f, 0.0f, -5.0f);
			pModelNode->AttachChild(&group);
			break;
		case 1:	// ball
			pEffect = U2_NEW U2Dx9FxShaderEffect(pStaticShader, m_spRenderer);		
			pEffect->SetVector(U2FxShaderState::MatDiffuse, D3DXVECTOR4(0.879440f, 0.879440f, 0.879440f, 1.0f));
			pEffect->SetVector(U2FxShaderState::MatEmissive, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));
			pEffect->SetFloat(U2FxShaderState::MatEmissiveIntensity, 1.0f);
			pEffect->SetVector(U2FxShaderState::MatSpecular, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
			pEffect->SetFloat(U2FxShaderState::MatSpecularPower, 52.655998f);
			pEffect->SetInt(U2FxShaderState::CullMode, 3);	
			pEffect->SetFloat(U2FxShaderState::BumpScale, 0.0f);
			pEffect->SetFrameShaderName(frameShader.GetName());
			pEffect->SetTexture(U2FxShaderState::DiffMap0, _T("materials/checker.dds"));
			pEffect->SetTexture(U2FxShaderState::BumpMap0, _T("materials/checker_bump.dds"));

			pController->SetTarget(&group);
			group.AttachController(pController);
			pModelNode->AttachChild(&group);
					
			//m_spTrnNode->AttachChild(&group);
			break;

		case 2: // light_sphere
			pEffect = U2_NEW U2Dx9FxShaderEffect(pStaticShader, m_spRenderer);		
			pEffect->SetVector(U2FxShaderState::MatDiffuse, D3DXVECTOR4(0.879440f, 0.879440f, 0.879440f, 1.0f));
			pEffect->SetVector(U2FxShaderState::MatEmissive, D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f));
			pEffect->SetFloat(U2FxShaderState::MatEmissiveIntensity, 1.0f);
			pEffect->SetVector(U2FxShaderState::MatSpecular, D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f));
			pEffect->SetFloat(U2FxShaderState::MatSpecularPower, 52.655998f);
			pEffect->SetInt(U2FxShaderState::CullMode, 3);	
			pEffect->SetFloat(U2FxShaderState::BumpScale, 0.0f);
			pEffect->SetFrameShaderName(frameShader.GetName());
			pEffect->SetTexture(U2FxShaderState::DiffMap0, _T("materials/checker.dds"));
			pEffect->SetTexture(U2FxShaderState::BumpMap0, _T("materials/checker_bump.dds"));

			pModelNode->AttachChild(&group);
			m_spLitSphere = &group;
			break;
		}

		group.AttachEffect(pEffect);
		group.UpdateMeshState(true, -1);
	}

	m_spSceneMgr->AttachMeshGroup(pMesh);
	

	spMeshLoader = 0;

	// Load Shadow
#if 0
	U2Node* pTrnNode = U2_NEW U2Node();	

	pRootNode->AttachChild(pTrnNode);

	pController = U2_NEW U2TransformInterpController;
	pController->SetName(_T("anim_1"));
	pController->SetLoopType(U2AnimLoopType::LOOP);

	pController->SetAnim(filename);
	pController->SetAnimGroup(1);	
	pController->SetTarget(pTrnNode);
	
	pTrnNode->AttachController(pController);
	pTrnNode->SetLocalTrans(0.0f, 8.840729, 0.0f);
	pTrnNode->SetLocalScale(3.000000, 3.991129, 3.000000);

	U2ShadowNode *pShadowNode = U2_NEW U2ShadowNode();

	pTrnNode->AttachChild(pShadowNode);
	
	//U2FilePath fPath;		
	_tcscpy_s(fullPath, MAX_PATH, _T("data\\models\\ball_sh.xmesh"));
	//// StackString Memory Leak...
	//U2DynString includePath(MODEL_PATH);	

	//fPath.ConvertToAbs(fullPath, MAX_PATH, _T("bouncingball.xmesh"), includePath);			
	pShadowNode->SetName(fullPath);	
	pShadowNode->LoadResources();
#endif

	m_spScene->AddObject(pRootNode);

	return pMesh;
}


U2N2Mesh* AssetViewApp::CreateSkinned()
{
	// Init Default Light

	U2TriListData *pData = U2_NEW U2TriListData;	

	U2FilePath fPath;		
	TCHAR fullPath[MAX_PATH] = _T("data\\models\\betty0_skin.xmesh");
	// StackString Memory Leak...
	//U2DynString includePath(MODEL_PATH);	
	
	//fPath.ConvertToAbs(fullPath, MAX_PATH, _T("betty0_skin.xmesh"), includePath);			

	GetPath(fullPath, MAX_PATH);	

	m_spMeshLoader = U2_NEW U2XMeshLoader;

	m_spMeshLoader->SetFilename(fullPath);

			
	U2N2Mesh* pMesh = U2_NEW U2N2Mesh(pData);

	pMesh->LightOnOff(false);

	pMesh->LoadFile(m_spMeshLoader);	

	

	// skinned shader 
	U2FrameShader& frameShader = m_spSceneMgr->m_spFrame->GetShader(
		_T("skinned"));

	//select mesh loader
	U2D3DXEffectShader* pSkinnedShader = frameShader.GetShader();

	U2FrameShader& frameShader2 = m_spSceneMgr->m_spFrame->GetShader(
		_T("skinned_alpha"));

	U2D3DXEffectShader* pSkinnedAlphaShader = frameShader2.GetShader();

	uint32 i;
	for(i = 0; i < pMesh->GetNumGroups(); ++i)
	{
		nMeshGroup& group = pMesh->Group(i);

		// ShaderAttb에 추가	

		U2Dx9FxShaderEffect *pEffect = NULL;

		switch(i)
		{
		case 0: // m_headarmleg
		case 1:	// m_torso
		case 4:	// m_gunarm
			{			
			pEffect = U2_NEW U2Dx9FxShaderEffect(pSkinnedShader, m_spRenderer);		
			//pEffect->SetFloat(U2FxShaderState::MatEmissiveIntensity, 1.0f);
			pEffect->SetVector(U2FxShaderState::MatDiffuse, D3DXVECTOR4(0.588235f, 0.588235, 0.588235, 1.0f));
			pEffect->SetVector(U2FxShaderState::MatAmbient, D3DXVECTOR4(0.588235f, 0.588235, 0.588235, 1.0f));			
			pEffect->SetVector(U2FxShaderState::MatSpecular, D3DXVECTOR4(0.9f, .9f, .9f, 1.0f));
			pEffect->SetFloat(U2FxShaderState::MatSpecularPower, 0.000001f);
			pEffect->SetInt(U2FxShaderState::CullMode, 2);	
			//pEffect->SetFloat(U2FxShaderState::BumpScale, 0.0f);
			pEffect->SetFrameShaderName(frameShader.GetName());
			pEffect->SetTexture(U2FxShaderState::DiffMap0, _T("betty.jpg"));
			//pEffect->SetTexture(U2FxShaderState::BumpMap0, _T("system/nobump.dds"));
			}
			break;
		case 2:	// m_fanvent
			{			

			pEffect = U2_NEW U2Dx9FxShaderEffect(pSkinnedShader, m_spRenderer);		
			pEffect->SetTexture(U2FxShaderState::DiffMap0, _T("betty_fanvent.jpg"));
			pEffect->SetVector(U2FxShaderState::MatDiffuse, D3DXVECTOR4(0.588235f, 0.588235, 0.588235, 1.0f));
			pEffect->SetVector(U2FxShaderState::MatAmbient, D3DXVECTOR4(0.588235f, 0.588235, 0.588235, 1.0f));
			pEffect->SetVector(U2FxShaderState::MatSpecular, D3DXVECTOR4(0.9f, .9f, .9f, 1.0f));
			pEffect->SetFloat(U2FxShaderState::MatSpecularPower, 0.000001f);			
			pEffect->SetFrameShaderName(frameShader2.GetName());					
			}
			break;
		case 3: // m_energy
			{			
			pEffect = U2_NEW U2Dx9FxShaderEffect(pSkinnedAlphaShader, m_spRenderer);		
			//pEffect->SetFloat(U2FxShaderState::MatEmissiveIntensity, 1.0f);
			pEffect->SetVector(U2FxShaderState::MatDiffuse, D3DXVECTOR4(0.588235f, 0.588235, 0.588235, 1.0f));
			pEffect->SetVector(U2FxShaderState::MatAmbient, D3DXVECTOR4(0.588235f, 0.588235, 0.588235, 1.0f));
			pEffect->SetVector(U2FxShaderState::MatSpecular, D3DXVECTOR4(0.9f, .9f, .9f, 1.0f));
			pEffect->SetFloat(U2FxShaderState::MatSpecularPower, 0.000001f);			
			pEffect->SetInt(U2FxShaderState::CullMode, 2);	
			//pEffect->SetFloat(U2FxShaderState::BumpScale, 0.0f);
			pEffect->SetFrameShaderName(frameShader.GetName());		
			pEffect->SetTexture(U2FxShaderState::DiffMap0, _T("betty_energy.jpg"));
			//pEffect->SetTexture(U2FxShaderState::BumpMap0, _T("system/nobump.dds"));
			}
			break;		
		default: //m_gun
			{			
			pEffect = U2_NEW U2Dx9FxShaderEffect(pSkinnedShader, m_spRenderer);		
			//pEffect->SetFloat(U2FxShaderState::MatEmissiveIntensity, 1.0f);
			pEffect->SetVector(U2FxShaderState::MatDiffuse, D3DXVECTOR4(0.588235f, 0.588235, 0.588235, 1.0f));
			pEffect->SetVector(U2FxShaderState::MatAmbient, D3DXVECTOR4(0.588235f, 0.588235, 0.588235, 1.0f));
			pEffect->SetVector(U2FxShaderState::MatSpecular, D3DXVECTOR4(0.9f, .9f, .9f, 1.0f));
			pEffect->SetFloat(U2FxShaderState::MatSpecularPower, 0.000001f);			
			pEffect->SetInt(U2FxShaderState::CullMode, 2);	
			pEffect->SetFloat(U2FxShaderState::BumpScale, 0.0f);
			pEffect->SetFrameShaderName(frameShader.GetName());
			pEffect->SetTexture(U2FxShaderState::DiffMap0, _T("betty_blaster.jpg"));
			//pEffect->SetTexture(U2FxShaderState::BumpMap0, _T("system/nobump.dds"));
			}
			break;
		}

		group.AttachEffect(pEffect);
		group.UpdateMeshState(true, -1);

	}	
		
	return pMesh;

}

//-------------------------------------------------------------------------------------------------
void AssetViewApp::OnIdle()
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

	
	
			

	unsigned int lastDrawHeight = DrawCamera(m_spCamera, 8, 30, COLOR_WHITE);	

	//const size_t uiSize = 128;

	//TCHAR atcMsg[uiSize];

	//_sntprintf(atcMsg, uiSize, _T("Visible Object : %d"),
	//	visibles.GetCount());

	//lastDrawHeight += WinApp::DRAW_HEIGHT;
	//m_spRenderer->DrawFont(8, lastDrawHeight, COLOR_WHITE, atcMsg);

	//if(m_spMesh)
	//{	
	//	_sntprintf(atcMsg, uiSize, _T("Model Position : %f, %f, %f"), 
	//		m_spMesh->GetWorldTranslate().x, m_spMesh->GetWorldTranslate().y, m_spMesh->GetWorldTranslate().z);

	//	lastDrawHeight += WinApp::DRAW_HEIGHT;
	//	m_spRenderer->DrawFont(8, lastDrawHeight, COLOR_WHITE, atcMsg);
	//}

	DrawFrameRate(8,GetHeight()-8, COLOR_WHITE);

	m_spRenderer->DrawFont(320,GetHeight()-8,COLOR_WHITE,m_acPickedName);

	m_spRenderer->EndRenderTarget();

	m_spRenderer->EndScene();	

	m_spRenderer->DisplayBackBuffer();

	timer.Stop();

	UpdateFrameCount();

}



bool AssetViewApp::OnKeyDown (unsigned char ucKey, int iX, int iY)
{
	if (WinApp::OnKeyDown(ucKey,iX,iY))
	{
		return true;
	}

	switch (ucKey)
	{
	case 'c':
		
		U2SceneMgr::Instance()->SetShadowEnabled(
			!U2SceneMgr::Instance()->GetShadowEnabled());
		return true;
	case '+':
	case '=':
		m_fVertDist += 0.1f;
		AdjuestVerticalDistance();
		return true;
	case '-':
	case '_':
		m_fVertDist -= 0.1f;
		AdjuestVerticalDistance();
		return true;	
	}

	return true;
}






