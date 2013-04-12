// GameApp.cpp :	Defines the entry point for the application.
//

#include "stdafx.h"
#include <fstream>
#include <mmsystem.h>
#include <D3Dcompiler.h>

#include <EngineCore/Util/GameLog.h>
#include <EngineCore/Math/MathTools.h>
#include <EngineCore/Util/U2WinFile.h>
#include "EngineCore/TBB/TaskMgrTbb.h"

// 유니코드 빌드 필요
//#include "dxut/core/stdafx.h
//#include "dxut/optional/SDKmisc.h"
//#include "dxut/optional/SDKMesh.h"
#include "common.h"
#include "SDKMeshLoader.h"

#include "GameApp.h"

#include <ModelLib/mpq_libmpq.h>
#include <ModelLib/Model_M2.h>
#include <ModelLib/M2Loader.h>
#include "M2Object.h"

#include "AnimationSystem.h"

#include "SceneNode.h"
#include "Mesh.h"
#include "M2Mesh.h"
#include "Mesh_D3D11.h"
#include "ObjLoader.h"
#include "AI/AIMeshLoader.h"
#include "AI/AICommon.h"
#include "AI/MsgRoute.h"
#include "AI/SMDebugLoig.h"
#include "AI/GameWorld.h"
#include "AI/CombatSystem.h"

#include "TestObjects.h"

#include "GameObjectUtil.h"

#include "EngineCore/direct3d11/Dx11TextureManager.h"
#include "Terrain/AdaptiveModelDX11Render.h"
#include "Terrain/SNBTerrainMaterial.h"

#include "Font/Dx11TextHelper.h"

#include "GameObjectManager.h"
#include "GameCamera.h"

#include "U2Light.h"
#include "DemoSetup.h"
#include "PSSMShadowMapDx11.h"

#include "Light.h"

#include "Effects/EffectSystem.h"
#include "Effects/LightningEffect.h"

#include "DebugDraw.h"
#include "AI/NavDebugDraw.h"
#include "AI/navigationmesh.h"
#include "GraphicsTypes.h"
#include "ShaderCompilation.h"

#include "LIDRenderer.h"
#include "PostProcessorBase.h"
#include "CascadedShadowsManager.h"

#include <GameApp/MD5/MD5Material.h>
#include <GameApp/MD5/MD5MapFile.h>
#include <GameApp/Collision/CollisionModel.h>
#include <GameApp/Collision/CollisionModel_local.h>
#include "GameApp/Physics/Physics.h"
#include "GameApp/Physics/Force.h"
#include "GameApp/Physics/Clip.h"
#include "GameApp/MD5/MD5Model.h"
#include "GameApp/Animation/MD5Anim.h"
#include "GameApp/MD5/MD5Entity.h"
#include "GameApp/MD5/MD5Game.h"

#include "./Light/SSAORenderer.h"
#include "GameApp/Objects/DecorationSystem.h"
#include "GameApp/Util/CommandSystem.h"
//
//
//#define SPEEDTREE_USE_ALLOCATOR_INTERFACE
//#include "CustomAllocator.h"
//
//#ifdef SPEEDTREE_USE_ALLOCATOR_INTERFACE
//static  CCustomAllocator    g_cCustomAllocator;
////static  CAllocatorInterface g_cAllocatorInterface(&g_cCustomAllocator);
//SpeedTree::CAllocator* SpeedTree::g_pAllocator = &g_cCustomAllocator;
//size_t SpeedTree::g_sHeapMemoryUsed;
//#endif



CAdaptiveModelDX11Render g_TerrainDX11Render;
NavigationMesh			NaviMesh;		/* our navigation mesh */



	#pragma comment(lib, "TBBGraphicsSamples.lib")
	#pragma  comment(lib, "dinput8.lib")
	#pragma  comment(lib, "Pdh.lib")

#ifdef _DEBUG
	#pragma comment( lib, "wxmsw28d_core.lib" )	// wxCore Debug Lib
	#pragma comment( lib, "wxmsw28d_adv.lib" )
	#pragma comment( lib, "wxmsw28d_qa.lib" )
	#pragma comment( lib, "wxmsw28d_aui.lib" )
	#pragma comment(lib, "cximagecrtd.lib")
	#pragma comment(lib, "libmpqd.lib")
	// Winsock 2
	#pragma comment( lib, "ws2_32.lib ") // This lib is required by wxbase28_net lib	

	#pragma comment( lib, "wxzlibd.lib" )
	#pragma comment( lib, "wxregexd.lib" )
	#pragma comment( lib, "wxbase28d.lib" )
	#pragma comment( lib, "wxbase28d_net.lib" )
	#pragma comment( lib, "wxexpatd.lib" )
	#pragma comment( lib, "wxbase28d_xml.lib" )

	#pragma comment( lib, "Effects11d.lib")
	//#pragma comment( lib, "Dxutd.lib")
	//#pragma comment( lib, "DxutOptd.lib")
	#pragma comment( lib, "openSteerLibd.lib" )
	#pragma comment( lib, "d3dx9d.lib" )

	//#pragma comment( lib, "tbb_debug.lib" )
	#pragma comment( lib, "tinyxmld.lib" )



	#pragma comment( lib, "DXTCompressorDLL_2010D.lib")

	#pragma comment( lib, "BulletCollision_vs2010_debug.lib")
	#pragma comment( lib, "BulletDynamics_vs2010_debug.lib")
	#pragma comment( lib, "LinearMath_vs2010_debug.lib")

	// cxImage
	//#ifdef _WINDOWS
	//#if _MSC_VER==1600		// If VC100 (VS2010)
	//#ifdef _WIN64
	//#pragma message("     Adding library: cximagecrt64d_VC100.lib" ) 
	//#pragma comment( lib, "cximagecrt64d_VC100.lib" )
	//#else
	//#pragma message("     Adding library: cximagecrt32d_VC100.lib" ) 
	//#pragma comment( lib, "cximagecrt32d_VC100.lib" )
	//#endif
	//#elif _MSC_VER==1500	// If VC90 (VS2008)
	//#ifdef _WIN64
	//#pragma message("     Adding library: cximagecrt64d_VC90.lib" ) 
	//#pragma comment( lib, "cximagecrt64d_VC90.lib" )
	//#else
	//#pragma message("     Adding library: cximagecrt32d_VC90.lib" ) 
	//#pragma comment( lib, "cximagecrt32d_VC90.lib" )
	//#endif
	//#else					// Otherwise
	//#pragma message("     Adding library: cximagecrtd.lib" ) 
	//#pragma comment( lib, "cximagecrtd.lib" )
	//#endif
	//#else
	//#pragma message("     Adding library: cximagecrtd.lib" ) 
	//#pragma comment( lib, "cximagecrtd.lib" )
	//#endif
#else // Release
	#define NDEBUG			// Disables Asserts in release
	#define VC_EXTRALEAN	// Exclude rarely-used stuff from Windows headers
	#define WIN32_LEAN_AND_MEAN

	#ifdef _WINDOWS
	#pragma comment( lib, "wxmsw28_core.lib" )
	#pragma comment( lib, "wxmsw28_adv.lib" )
	#pragma comment( lib, "wxmsw28_qa.lib" )
	#pragma comment( lib, "wxmsw28_aui.lib" )

	#pragma comment( lib, "ws2_32.lib ") // This lib is required by wxbase28_net lib
	#elif _MAC
	#pragma comment( lib, "wxmac28_core.lib" )
	#pragma comment( lib, "wxmac28_adv.lib" )
	#pragma comment( lib, "wxmac28_gl.lib" )
	#pragma comment( lib, "wxmac28_qa.lib" )
	#pragma comment( lib, "wxmac28_aui.lib" )
	#endif

	#pragma comment( lib, "wxzlib.lib" )
	#pragma comment( lib, "wxregex.lib" )
	#pragma comment( lib, "wxbase28.lib" )
	#pragma comment( lib, "wxbase28_net.lib" )
	#pragma comment( lib, "wxexpat.lib" )
	#pragma comment( lib, "wxbase28_xml.lib" )

	#pragma comment(lib, "cximagecrt.lib")
	#pragma comment( lib, "Effects11.lib")

	#pragma comment( lib, "d3dx9.lib" )
	//#pragma comment( lib, "Dxut.lib")	
	//#pragma comment( lib, "DxutOpt.lib")
	//#pragma comment( lib, "tbb.lib" )
	#pragma comment( lib, "tinyxml.lib" )

	#pragma comment( lib, "DXTCompressorDLL_2010.lib")

	#pragma comment( lib, "BulletCollision_vs2010.lib")
	#pragma comment( lib, "BulletDynamics_vs2010.lib")
	#pragma comment( lib, "LinearMath_vs2010.lib")

	//#pragma comment( lib, "openSteerLib.lib" )


	//// cxImage
	//#if defined(_WINDOWS)
	//#if _MSC_VER==1600		// If VC100 (VS2010)
	//#ifdef _WIN64
	//#pragma message("     Adding library: cximagecrt64_VC100.lib" ) 
	//#pragma comment( lib, "cximagecrt64_VC100.lib" )
	//#else
	//#pragma message("     Adding library: cximagecrt32_VC100.lib" ) 
	//#pragma comment( lib, "cximagecrt32_VC100.lib" )
	//#endif
	//#elif _MSC_VER==1500	// If VC90 (VS2008)
	//#ifdef _WIN64
	//#pragma message("     Adding library: cximagecrt64_VC90.lib" ) 
	//#pragma comment( lib, "cximagecrt64_VC90.lib" )
	//#else
	//#pragma message("     Adding library: cximagecrt32_VC90.lib" ) 
	//#pragma comment( lib, "cximagecrt32_VC90.lib" )
	//#endif
	//#else					// Otherwise
	//#pragma message("     Adding library: cximagecrt.lib" ) 
	//#pragma comment( lib, "cximagecrt.lib" )
	//#endif
	//#else
	//#pragma message("     Adding library: cximagecrt.lib" ) 
	//#pragma comment( lib, "cximagecrt.lib" )
	//#endif
#endif // _DEBUG

#define MAX_MOUSE_DELTA 50



struct SimpleVertex
{
	XMFLOAT3 Pos;
	//XMFLOAT3 Normal;
	XMFLOAT4 Color;
	//XMFLOAT2 uv;
};

struct QuadVertex
{
	XMFLOAT4 Position;
	XMFLOAT2 TexCoord;
};

struct QuadPSConstant {
	float fMainBufferMultiplier;	
};


extern ID3DX11Effect *g_pEffect;
extern ID3DX11EffectTechnique *g_pTechniqueShadowMap_GSC;
extern ID3DX11EffectTechnique *g_pTechniqueShadowMap_Inst;
extern ID3DX11EffectTechnique *g_pTechniqueShadows;
extern ID3DX11EffectMatrixVariable *g_pWorldVariable;
extern ID3DX11EffectMatrixVariable *g_pViewProjVariable;
extern ID3DX11EffectVectorVariable *g_pLightDirVariable;
extern ID3DX11EffectVectorVariable *g_pLightAmbientVariable;
extern ID3DX11EffectVectorVariable *g_pLightColorVariable;
extern ID3DX11EffectMatrixVariable *g_pViewVariable;
extern ID3DX11EffectMatrixVariable *g_pCropMatrixVariable;
extern ID3DX11EffectShaderResourceVariable *g_pShadowMapTextureArrayVariable;
extern ID3DX11EffectMatrixVariable *g_pTextureMatrixVariable;
extern ID3DX11EffectScalarVariable *g_pSplitPlaneVariable;
extern ID3DX11EffectScalarVariable *g_pFirstSplitVariable;
extern ID3DX11EffectScalarVariable *g_pLastSplitVariable;


extern bool bShowParticle;

BaseApp *g_pApplication;
Renderer* gRenderer;

extern Light g_Light;
extern std::vector<Mesh*>	gShadowReceivers;

ShaderID gTreeShader;
ShaderID gFireShader;
ShaderID gColor;
ShaderID gSkinned;
ShaderID gPssmSkinned;
ShaderID gPssm;
ShaderID gColorPssm;
ShaderID gSkinnedColorPssm;

ShaderID gColorSsao;
ShaderID gSkinnedColorSsao;
ShaderID gColorPssm2;
SamplerStateID gPassmSampler;

SpellEffectsDB spelleffectsdb;


// not a hard limit, just what we keep track of for debugging
xthreadInfo *g_threads[MAX_THREADS];
int g_thread_count = 0;

BaseApp *app = new App();

RenderData gData;

static const char* modelname[] = {
	"Character\\BloodElf\\female\\BloodElfFemale.m2",
	"Creature\\AbyssalOutland\\Abyssal_Outland.m2",
	"Creature\\alexstrasza\\alexstrasza.m2",
	"Creature\\arthas\\arthas.m2",
	"Creature\\FireSpiritsmall\\firespiritsmall.m2",
	"Particles\\MorphFX.M2"
};

static const char* Spells[] = {
	"Spells/Abyssal_impact_base.m2",
	"Spells/Abyssal_ball.m2",
	"Spells/Acidburn.m2",
	"Spells/Amplifymagic_impact_base.m2",
	"Spells/Aimedshot_impact_chest.m2",
	"Spells/Aegis.m2",
	"Spells/Alysrazor_wings_01.m2",
	"Spells/Arcane_fire_weapon_effect.m2",
	"Spells/Arcanepower_state_chest.m2",
	//"Spells/Axistestobject.m2",
	"Spells/Aimedshot_impact_chest.m2",
};

int spellIndex = 0;
int spellcount = ARRAYSIZE(Spells);


App::App() : D3D11App(),
	m_speedMousePan(10),
	m_speedMouseWheelZoom(10),
	m_speedMouseButtonZoom(1),
	m_speedPadPan(10),
	m_speedPadZoom(10),
	m_speedFlyStrafe(1000),
	m_speedFlyMove(10),
	m_lbPressed(false),
	m_mbPressed(false),
	m_rbPressed(false),
	m_absolute(false),
	m_navMode(CAMERA_NAV_TRACKBALL),
	LIDRender_(NULL),
	Lightning_(NULL),
	Sky_(NULL),
	ActiveCam_(NULL)
{
	g_pApplication = this;
	frameNum = 0;
	loader = 0;
	DefCam_ = 0;
	FpsCam_ = 0;
	FollowCam_ = 0;
	TopDownCam_ = 0;
	IsometricCam_ = 0;
	SideScrollCam_ = 0;
	ppParticleSystem_ = 0;
	pTerrain = 0;
	rayOrig_ = noVec3(FLT_MAX, FLT_MAX, FLT_MAX);
	rayTarget_ = noVec3(FLT_MAX, FLT_MAX, FLT_MAX);
	pickPos_ = noVec3(FLT_MAX, FLT_MAX, FLT_MAX);

	m_navMesh = 0;
	terrainload_ = false;
	pCascaded_ = NULL;
	g_World = identity4();
	m_mouseAbsoluteSpeed = false;
	currMesh_ = NULL;
	EffectSys_ = NULL;
	ddt = 0;
	globalTime = 0;

	m_dynamicsWorld = NULL;
	m_broadphase = NULL;
	m_dispatcher = NULL;
	m_solver = NULL;
	m_collisionConfiguration = NULL;

	main_color_resource = 0;
	main_color_resourceSRV= 0;
	main_color_resourceRTV= 0;
	main_depth_resource= 0;
	main_depth_resourceDSV= 0;
	main_depth_resourceSRV= 0;
	main_color_resource_resolved= 0;
	main_color_resource_resolvedSRV= 0;
}


void App::resetCamera(){
	camPos = vec3(-730, 20, 2010);
	wx = 0.14f;
	wy = -2.63f;
}

bool App::onKey(const uint key, const bool pressed){
	if (D3D11App::onKey(key, pressed)) return true;

	if (pressed && key == KEY_F5){		
		return true;
	}
	if (pressed && key == KEY_P) {		
		AppSettings::AssetTypeEnum = ASSET_DECO;		
	}

	if (pressed && key == KEY_I) {		
		AppSettings::AssetTypeEnum = ASSET_BUILDING;
	}

	if (pressed && key == KEY_O) {		
		AppSettings::AssetTypeEnum = ASSET_CHAR;		
	}
	if (pressed && key == KEY_J) {		
		AppSettings::AssetTypeEnum = ASSERT_SPELLS;		
	}

	if (pressed && key == KEY_B){		

		if (AppSettings::EditModeEnum == EDIT_TEST_MODE)
		{
			if (PickUtil::Picking( m_WorldRoot))
			{					
				noVec3 target = PickUtil::pickDatas[0].vert;

				if (AppSettings::AssetTypeEnum == ASSERT_SPELLS)
					LoadModel(target);
			}
		}		
	}

	if (pressed && key >= KEY_0 && key <= KEY_9)
	{
		int nSlot = key - '0';
		spellIndex = nSlot;
		switch(AppSettings::AssetTypeEnum)
		{
		case ASSET_DECO:
		case ASSET_CHAR:
		case ASSET_BUILDING:
			NewEntity(nSlot);
			break;
		}
	}
	if (pressed && key == KEY_Y) 
	{
		static int i=0;
		if (i > 2)
			i = 0;
		switch(i)
		{
		case 0:
			ActiveCam_ = FollowCam_;
			break;
		case 1:
			ActiveCam_ = FpsCam_;
			break;
		case 2:
			ActiveCam_ = DefCam_;
			break;
		}
		++i;


	}

	if (pressed && key == KEY_U) {		
		//AddPath();
		//
		//if (loader)
		//{
		//	loader->animManager->Pause(false);
		//	loader->animManager->SetAnim(0, 80, 1);
		//	loader->animManager->Play();
		//}

		m_navMode = CAMERA_NAV_TRACKBALL;

	}
	if (pressed && key == KEY_L) {		
		
		
		//ExecuteAI();
		for (size_t i =0; i < m_ActorRoot->childNodes_.size(); ++i)
		{
			M2Mesh* pMesh = (M2Mesh*)m_ActorRoot->childNodes_[i];
			if (pMesh)
			{
				pMesh->ToggleDebug();
				//pMesh->m2_->animManager->Pause(pMesh->m2_->animManager->IsPaused());			
			}
		}		
	}
	if (pressed && key == KEY_K) {		

		m_navMode = CAMERA_NAV_FLY;
		static std::vector<int> index; 
		index.resize(m_ActorRoot->childNodes_.size());
				
		for (size_t i =0; i < m_ActorRoot->childNodes_.size(); ++i)
		{
			M2Mesh* pMesh = dynamic_cast<M2Mesh*>(m_ActorRoot->childNodes_[i]);
			if (pMesh)
			{
				if (index[i] >= BONE_MAX)
					index[i] = 0;
				pMesh->ToggleBones(index[i]++);
			}			
		}		
		
	}

	if (pressed && key == KEY_N) {		

		//for (size_t i =0; i < m_ActorRoot->childNodes_.size(); ++i)
		//{
		//	M2Mesh* pMesh = (M2Mesh*)m_ActorRoot->childNodes_[i];
		//	if (pMesh)
		//	{				
		//		//pMesh->m2_->animManager->Pause(true);
		//		//pMesh->m2_->animManager->SetAnim(0, 80, 1);
		//		pMesh->m2_->animManager->PrevFrame();
		//	}
		//}
		extern CAdaptiveModelDX11Render g_TerrainDX11Render;

		CTerrainPatch* Patch = g_TerrainDX11Render.GetCurrPatch();
		if (Patch)
		{	
			extern DecorationSystem gDecoSys;
			noVec2 PatchXY = g_TerrainDX11Render.GetPatchCornerXY(Patch);		
			char name[128];
			idStr::snPrintf(name, 128, "Grass%f_%f", PatchXY.x, PatchXY.y);
			Patch->GetGrass() = gDecoSys.CreateNewGrass(name);

			float patchScale = g_TerrainDX11Render.GetPatchScale(Patch);

			extern int g_iPatchSize;
			float radius = g_iPatchSize / 2.0f;
			//noVec3 center(PatchXY.x + radius, 0.0f, PatchXY.y + radius);		
			noVec3 center = Pick();
			gDecoSys.GenerateGrass(Patch->GetGrass(), center, radius,  70, 15.0f);
		}
	}

	if (pressed && key == KEY_B) {		

		for (size_t i =0; i < m_ActorRoot->childNodes_.size(); ++i)
		{
			M2Mesh* pMesh = (M2Mesh*)m_ActorRoot->childNodes_[i];
			if (pMesh)
			{				
				//pMesh->m2_->animManager->Pause(true);
				//pMesh->m2_->animManager->SetAnim(0, 80, 1);
				pMesh->m2_->animManager->NextFrame();
			}
		}
	}
	
	if (pressed && key == KEY_C) {		
		WowActor* pActor = (WowActor*)g_database.Find(1);
		if (pActor)
		{
			static int angle = 0;
			if (angle > 360)
				angle = 0;
			noRotation rot(noVec3(0, 0, 0), noVec3(0, 1, 0), angle++);
			pActor->mesh_->SetRotate(rot.ToMat3());
		}
	}

	if (pressed && key == KEY_V) {		
		//for (size_t i =0; i < m_ActorRoot->childNodes_.size(); ++i)
		{
			M2Mesh* pMesh = (M2Mesh*)m_ActorRoot->childNodes_[1];
			if (pMesh)
			{
				static int i =0;
				if(i >= pMesh->m2_->numPass)
					i = 0;
				pMesh->m2_->render_data[i++].visible = !pMesh->m2_->render_data[i].visible;
			}
		}

		static int i = 0; 
		if ( i >= 5)
			i = 0;		
		m_currBlendState = i;
		//m2->ToggleBlendState(2,  i);
		i++;
	}

	

	return false;
}

bool App::initAPI(){
	// Override the user's MSAA settings
	if (D3D11App::initAPI(D3D11, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_D24_UNORM_S8_UINT, 1, NO_SETTING_CHANGE | SAMPLE_BACKBUFFER))
	{
		gRenderer = renderer;

		m_winLLRelative[0] = 0.0f;
		m_winLLRelative[1] = 0.0f;
		m_winURRelative[0] = 1.0f;
		m_winURRelative[1] = 1.0f;
			

		int winW = getWidth();
		int winH = getHeight();

		m_winLL[0] = int(winW * m_winLLRelative[0]);
		m_winLL[1] = int(winH * m_winLLRelative[1]);
		m_winUR[0] = int(winW * m_winURRelative[0]);
		m_winUR[1] = int(winH * m_winURRelative[1]);

		InitRenderer();

		return true;	
	}


	return false;
}

void App::onSize(const int w, const int h){	
	ReleaseRes();

	D3D11App::onSize(w, h);

	if (LIDRender_) OnSizeLID();

	OnSizeTerrain();	
	ResizeSNBTerrain();

	UpdateViewerCameraNearFar();

	if (!m_absolute)
	{
		int winW = getWidth();
		int winH = getHeight();

		m_winLL[0] = unsigned(winW * m_winLLRelative[0]);
		m_winLL[1] = unsigned(winH * m_winLLRelative[1]);

		m_winUR[0] = unsigned(winW * m_winURRelative[0]);
		m_winUR[1] = unsigned(winH * m_winURRelative[1]);
	}
	if (device) {
		RecreateBuffers();
		ResetWater();
	}
}

extern bool useLocalFiles;

void OpenDBs()
{
	if (!itemdb.open()) {
		//initDB = false;
		wxLogMessage(wxT("Error: Could not open the Item DB."));
	}

	if (!itemsparsedb.open()) {
		wxLogMessage(wxT("Error: Could not open the Item Sparse DB."));
	}
		
	wxString filename = wxT("koKR/items.csv");
	wxLogMessage(wxT("Trying to open %s file (for updating locale files)"),filename.c_str());
		
	if (wxFile::Exists(filename)) {
		items.open(filename);
	} else {
		wxLogMessage(wxT("Error: Could not find items.csv to load an item list from."));
	}

	if (!skyboxdb.open()) {		
		wxLogMessage(wxT("Error: Could not open the SkyBox DB."));
	}

	if (!spellitemenchantmentdb.open()) {

		wxLogMessage(wxT("Error: Could not open the Spell Item Enchanement DB."));
	}

	if (!itemvisualsdb.open()) {

		wxLogMessage(wxT("Error: Could not open the Item Visuals DB."));
	}

	if (!animdb.open()) {	
		wxLogMessage(wxT("Error: Could not open the Animation DB."));
	}

	if (!modeldb.open()) {		
		wxLogMessage(wxT("Error: Could not open the Creatures DB."));
	}

	if (!skindb.open()) {		
		wxLogMessage(wxT("Error: Could not open the CreatureDisplayInfo DB."));
	}	

	if(!hairdb.open()) {		
		wxLogMessage(wxT("Error: Could not open the Hair DB."));
	}

	if(!chardb.open()) {		
		wxLogMessage(wxT("Error: Could not open the Character DB."));
	}

	if(!facialhairdb.open()) {		
		wxLogMessage(wxT("Error: Could not open the Char Facial Hair DB."));
	}

	if(!racedb.open()) {		
		wxLogMessage(wxT("Error: Could not open the Char Facial Hair DB."));
	}

	if(!classdb.open()) {	
		wxLogMessage(wxT("Error: Could not open the Char Classes DB."));
	}

	if(!visualdb.open()) {
		wxLogMessage(wxT("Error: Could not open the ItemVisuals DB."));
	}
	if(!effectdb.open())
		wxLogMessage(wxT("Error: Could not open the ItemVisualEffects DB."));

	if(!subclassdb.open())
		wxLogMessage(wxT("Error: Could not open the Item Subclasses DB."));

	if(!startdb.open())
		wxLogMessage(wxT("Error: Could not open the Start Outfit Sets DB."));

	if(!npcdb.open()) 
		wxLogMessage(wxT("Error: Could not open the Start Outfit NPC DB."));

	if(!npctypedb.open())
		wxLogMessage(wxT("Error: Could not open the Creature Type DB."));

	if(!camcinemadb.open())
		wxLogMessage(wxT("Error: Could not open the Cinema Camera DB."));

	if(!itemdisplaydb.open())
		wxLogMessage(wxT("Error: Could not open the ItemDisplayInfo DB."));
	else
		items.cleanup(itemdisplaydb);

	if(!setsdb.open())
		wxLogMessage(wxT("Error: Could not open the Item Sets DB."));
	else
		setsdb.cleanup(items);

	filename = wxT("koKR/npcs.csv");

	if(wxFile::Exists(filename)) {
		npcs.open(filename);
	} else {
		NPCRecord rec(wxT("26499,24949,7,Arthas"));
		if (rec.model > 0) {
			npcs.npcs.push_back(rec);
		}		
		wxLogMessage(wxT("Error: Could not find npcs.csv, unable to create NPC list."));
	}

	/*if(spelleffectsdb.open())
		GetSpellEffects();
	else
		wxLogMessage(wxT("Error: Could not open the SpellVisualEffects DB."));*/

	wxLogMessage(wxT("Finished initiating database files."));
}

bool App::init(){

	Mem_Init();

	idStr::InitMemory();

	idSIMD::Init();

	noMath::Init();
	

	FILELog::Init();

	timer_ = new Time();
	msgRoute_ = new MsgRoute;
	dLog_ = new DebugLog();
	combat_ = new CombatSystem;
	
	MathHelpers::SetYUp(true);
	

	gameVersion = VERSION_CATACLYSM;
	globalTime = 0;

	useLocalFiles = true;

	gTaskMgr.Init();
	
	lastTime = timeGetTime();

	GameObjectManager::Create();

	M2Loader::createAnimMgr = AnimationSystem::CreateAnimSystem;

	
	CMeshBundle::getInstance().addDirectory( "Model/");

	//
	// device dependant resources

	CDeviceResourceManager& deviceManager = CDeviceResourceManager::getInstance();
	deviceManager.addListener( CMeshBundle::getInstance() );

	
	return true;
}

void App::InitRenderer()
{
	
	blendModes_[0] = renderer->addBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA,
			D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 0x0F, D3D11_BLEND_OP_ADD);

	
	blendModes_[1] = renderer->addBlendState(D3D11_BLEND_SRC_COLOR, D3D11_BLEND_ONE, 
			D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 0x0F, D3D11_BLEND_OP_ADD);

	
	blendModes_[2] = /*renderer->addBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE,
			D3D11_BLEND_ZERO, D3D11_BLEND_ZERO,  0x0F, D3D11_BLEND_OP_ADD);*/
			renderer->addBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE,
			D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE,  0x0F, D3D11_BLEND_OP_ADD);
		
	
	blendModes_[3]  = renderer->addBlendState(D3D11_BLEND_DEST_COLOR, D3D11_BLEND_SRC_COLOR, 
			D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 0x0F, D3D11_BLEND_OP_ADD);
			
	blendModes_[4] = renderer->addBlendState(D3D11_BLEND_DEST_COLOR, D3D11_BLEND_SRC_COLOR, 
			D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, 0x0F, D3D11_BLEND_OP_ADD);

	blendModes_[5] = renderer->addBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, 
		D3D11_BLEND_ONE, D3D11_BLEND_ZERO, 0x0F, D3D11_BLEND_OP_ADD);

	blendModes_[5] = renderer->addBlendState(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, 
		D3D11_BLEND_ONE, D3D11_BLEND_ZERO, 0x0F, D3D11_BLEND_OP_ADD);


	depthWrite_ = renderer->addDepthState(true, true);
	depthTest_ = renderer->addDepthState(true, false);
	m_currBlendState = blendModes_[5];
	
	rasterizerStates_ = new RasterizerStates;
	blendStates_ = new BlendStates;
	depthStencilStates_ = new DepthStencilStates;	
	samplerStates_ = new SamplerStates;
	
	rasterizerStates_->Initialize(device);
	blendStates_->Initialize(device);
	depthStencilStates_->Initialize(device);
	samplerStates_->Initialize(device);

	SSAORender_ = new SSAORenderer;
	SSAORender_->Init(device, context);	
	SSAORender_->LoadShaders();
}


#define STRAFE_SPEEDXY 0.0025f
#define STRAFE_SPEEDZ 0.0025f


void App::UpdateCamera(float dt)
{
	if (m_navMode != CAMERA_NAV_FLY) 
		return;

	dt *= 60;

	if(GetKeyDown('W'))
	{
		ActiveCam_->dolly( -STRAFE_SPEEDZ * m_speedFlyStrafe * dt, true, true);
	}
	else if(GetKeyDown('S'))
	{
		ActiveCam_->dolly( STRAFE_SPEEDZ * m_speedFlyStrafe * dt, true, true);	
	}

	if(GetKeyDown('Q'))
	{
		ActiveCam_->pan(0, STRAFE_SPEEDXY * m_speedFlyStrafe * dt,  true, true);
	}
	else if(GetKeyDown('Z'))
	{
		ActiveCam_->pan(0, -STRAFE_SPEEDXY * m_speedFlyStrafe * dt,  true, true ); 
	}

	// Strafing
	//
	if(GetKeyDown('D'))
	{
		ActiveCam_->pan( -STRAFE_SPEEDXY * m_speedFlyStrafe * dt, 0, true, true);
	}
	else if(GetKeyDown('A'))
	{
		ActiveCam_->pan( STRAFE_SPEEDXY * m_speedFlyStrafe * dt, 0, true, true ); 
	}
}

bool App::CreateBox()
{
#if 1
	shader = renderer->addShader("Data/Shaders/WireColor.hlsl");

	FormatDesc cubeFmt[] = {
		0, TYPE_VERTEX, FORMAT_FLOAT, 3,
		//0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,		
		0, TYPE_COLOR, FORMAT_FLOAT, 4,		
	};
	if ((vf = renderer->addVertexFormat(cubeFmt, elementsOf(cubeFmt), shader)) == VF_NONE) return false;


	// Create vertex buffer
	SimpleVertex vertices[] =
	{
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 0.0f, 1.0f, 1.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT4( 0.0f, 1.0f, 0.0f, 1.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT4( 0.0f, 1.0f, 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 0.0f, 1.0f, 1.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT4( 1.0f, 1.0f, 0.0f, 1.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT4( 0.0f, 0.0f, 0.0f, 1.0f ) },
	};
	/*SimpleVertex vertices[] =
	{
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

		{ XMFLOAT3( -1.0f, -1.0f, -1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, -1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, -1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, -1.0f ), XMFLOAT2( 0.0f, 1.0f ) },

		{ XMFLOAT3( -1.0f, -1.0f, 1.0f ), XMFLOAT2( 0.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f, -1.0f, 1.0f ), XMFLOAT2( 1.0f, 0.0f ) },
		{ XMFLOAT3( 1.0f, 1.0f, 1.0f ), XMFLOAT2( 1.0f, 1.0f ) },
		{ XMFLOAT3( -1.0f, 1.0f, 1.0f ), XMFLOAT2( 0.0f, 1.0f ) },
	};
*/

	vb = renderer->addVertexBuffer(sizeof( SimpleVertex ) * 24 , STATIC, vertices);

	WORD indices[] =
	{
		3,1,0,
		2,1,3,

		0,5,4,
		1,5,0,

		3,4,7,
		0,4,3,

		1,6,5,
		2,6,1,

		2,7,6,
		3,7,2,

		6,4,5,
		7,4,6,
	};

	ib = renderer->addIndexBuffer(36, sizeof(WORD), STATIC, indices);

	//rt = renderer->addTexture("spells/fire1.tga", true);
#endif
	return true;
}

void App::LoadShaders()
{
	gPssm = renderer->addShader("Data/Shaders/PSSM_Instancing.hlsl");

	gColorPssm = renderer->addShader("Data/Shaders/ColorPSSM.hlsl");
	gColorPssm2 = renderer->addShader("Data/Shaders/ColorPSSM2.hlsl");
	
	gPassmSampler = renderer->addSamplerState(COMPARISON_LINEAR, BORDER, BORDER, BORDER, 0.0f, 16, D3D11_COMPARISON_LESS);
	gColor = renderer->addShader("Data/Shaders/BlinnPhong.hlsl");

	char def[256];		
	sprintf(def, "#define MATRIX_PALETTE_SIZE_DEFAULT	164\n", def);
	gSkinned = renderer->addShader("Data/Shaders/skinnedShader.hlsl", def);
	gPssmSkinned = renderer->addShader("Data/Shaders/PSSM_Skinned_Instancing.hlsl", def);
	gSkinnedColorPssm  = renderer->addShader("Data/Shaders/SkinColorPSSM.hlsl", def);

	gColorSsao = renderer->addShader("Data/Shaders/ColorSSAO.hlsl", def);
	gSkinnedColorSsao = renderer->addShader("Data/Shaders/SkinColorSSAO.hlsl");

	
	gTreeShader = renderer->addShader("Data/Shaders/Tree.hlsl");
	gFireShader = -1; //renderer->addShader("Data/Shaders/Fire.hlsl");

	ID3D10BlobPtr byteCode;
	QuadVS_.Attach(CompileVSFromFile(device, L"Data\\Shaders\\Quad.hlsl", "FullScreenQuadVS", "vs_4_0", NULL, NULL, &byteCode));
	QuadPS_.Attach(CompilePSFromFile(device, L"Data\\Shaders\\Quad.hlsl", "MainToBackBufferPS", "ps_4_0", NULL, NULL));

	// Create the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	DXCall(device->CreateInputLayout(layout, 2, byteCode->GetBufferPointer(), byteCode->GetBufferSize(), &QuadInputLayout_));

	// Create and initialize the vertex and index buffers
	QuadVertex verts[4] =
	{
		{ XMFLOAT4(-1, -1, 0, 1), XMFLOAT2(0, 1) },
		{ XMFLOAT4(1, -1, 0, 1), XMFLOAT2(1, 1) },
		{ XMFLOAT4(-1, 1, 0, 1), XMFLOAT2(0, 0) },
		{ XMFLOAT4(1, 1, 0, 1), XMFLOAT2(1, 0) }
	};

	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.ByteWidth = sizeof(verts);
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = verts;
	initData.SysMemPitch = 0;
	initData.SysMemSlicePitch = 0;
	DXCall(device->CreateBuffer(&desc, &initData, &QuadVB_));

	// Create the constant buffer
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = CBSize(sizeof(QuadPSConstant));
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	DXCall(device->CreateBuffer(&desc, NULL, &QuadPSConstants_));

	RecreateBuffers();
}

void App::DrawFullScreenQuad() {

	SetDefaultRenderTarget();

	context->RSSetState(rasterizerStates_->NoCullNoMS());
	context->OMSetDepthStencilState(depthStencilStates_->DepthDisabled(), 1.0f);
	float blendfactor[4] = {0.0f};
	context->OMSetBlendState(blendStates_->BlendDisabled(), blendfactor, 0xFFFFFFFF);
	//resolving main buffer 
	context->ResolveSubresource(main_color_resource_resolved,0,main_color_resource,0,DXGI_FORMAT_R8G8B8A8_UNORM);

	ID3D11SamplerState* samplerStates[1]  = { LIDRender_->GetSamplerState().Linear() };
	context->PSSetSamplers(0, 1, samplerStates);
	
	ID3D11ShaderResourceView* ps_srvs[1] = {main_color_resource_resolvedSRV};
	context->PSSetShaderResources(0, 1, ps_srvs);

	UINT stride = sizeof(QuadVertex);
	UINT offset = 0;
	ID3D11Buffer* vertexBuffers[1] = { QuadVB_ };
	context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
#if 1
	D3D11_MAPPED_SUBRESOURCE mapped;
	DXCall(context->Map(QuadPSConstants_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped));
	QuadPSConstant* constants = reinterpret_cast<QuadPSConstant*>(mapped.pData);
	constants->fMainBufferMultiplier = 1.0f; //main_buffer_size_multiplier;
	context->Unmap(QuadPSConstants_, 0);

	ID3D11Buffer* constantBuffers[1] = { QuadPSConstants_ };
	context->PSSetConstantBuffers(0, 1, constantBuffers);
#endif
	context->IASetInputLayout(QuadInputLayout_);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->VSSetShader(QuadVS_, NULL, 0);
	context->PSSetShader(QuadPS_, NULL, 0);
	context->Draw(4, 0);
	ps_srvs[0] = NULL;
	context->PSSetShaderResources(0, 1, ps_srvs);
}

void App::RecreateBuffers() {

	D3D11_TEXTURE2D_DESC tex_desc;
	D3D11_SHADER_RESOURCE_VIEW_DESC textureSRV_desc;
	D3D11_DEPTH_STENCIL_VIEW_DESC DSV_desc;
	
	SAFE_RELEASE(main_color_resource);
	SAFE_RELEASE(main_color_resourceSRV);
	SAFE_RELEASE(main_color_resourceRTV);

	SAFE_RELEASE(main_color_resource_resolved);
	SAFE_RELEASE(main_color_resource_resolvedSRV);

	SAFE_RELEASE(main_depth_resource);
	SAFE_RELEASE(main_depth_resourceDSV);
	SAFE_RELEASE(main_depth_resourceSRV);

	// recreating main color buffer

	ZeroMemory(&textureSRV_desc,sizeof(textureSRV_desc));
	ZeroMemory(&tex_desc,sizeof(tex_desc));

	int MultiSampleCount = 1;
	int MultiSampleQuality = 0;

	uint32 BackbufferWidth = GetApp()->getWidth();
	uint32 BackbufferHeight = GetApp()->getHeight();

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

	device->CreateTexture2D         ( &tex_desc, NULL, &main_color_resource );
	device->CreateShaderResourceView( main_color_resource, &textureSRV_desc, &main_color_resourceSRV );
	device->CreateRenderTargetView  ( main_color_resource, NULL, &main_color_resourceRTV );


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

	device->CreateTexture2D         ( &tex_desc, NULL, &main_color_resource_resolved );
	device->CreateShaderResourceView( main_color_resource_resolved, &textureSRV_desc, &main_color_resource_resolvedSRV );

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

	device->CreateTexture2D( &tex_desc, NULL, &main_depth_resource );
	device->CreateDepthStencilView(main_depth_resource, &DSV_desc, &main_depth_resourceDSV );
	device->CreateShaderResourceView( main_depth_resource, &textureSRV_desc, &main_depth_resourceSRV );

}

void App::CreateCameras()
{
	float eye[] = { 100.0f, 5.0f, 5.f };
	float at[] = { 0.0f, 0.0f, 0.f };
	float up[] = {  0.0f, 1.0f, 0.0f };


	DefCam_ = new ShadowMapCamera(eye, at, up, 45.f);
	DefCam_->setFar(25000.0f);	// CasCaded Shadow Setting
	DefCam_->setNear(0.1f);
	DefCam_->SetAspect(width / (FLOAT)height);	
	DefCam_->m_fNearMin = DefCam_->GetNear();
	DefCam_->m_fFarMax = DefCam_->GetFar();


	//noVec3 yAxis = noVec3(0, 1, 0);
	//pcamera_->rotateAboutTo(90.f, yAxis.TofloatPtr(), true);

	//pcamera_->setProjectionMode(CAMERA_COMPUTE_ORTHOGRAPHIC);
	DefCam_->setHandednessMode(CAMERA_HANDEDNESS_LEFT);	
	DefCam_->computeModelView();
	DefCam_->ComputeProjection();	

	ActiveCam_ = DefCam_;

	float eye_[] = {  -320.0f, 300.0f, -220.3f };
	float at_[] = { 0.0f, 0.0f, 0.f };
	float up_[] = {  0.0f, 1.0f, 0.0f };

	pShadowCam_ = new ShadowMapCamera(eye_, at_, up_, 45.f);
	pShadowCam_->setFar(10000.0f);	// CasCaded Shadow Setting
	pShadowCam_->setNear(0.1f);
	pShadowCam_->SetAspect(width / (FLOAT)height);	
	pShadowCam_->setHandednessMode(CAMERA_HANDEDNESS_LEFT);	
	pShadowCam_->setProjectionMode(CAMERA_COMPUTE_ORTHOGRAPHIC);
	pShadowCam_->computeModelView();
	pShadowCam_->ComputeProjection();	
	pShadowCam_->m_fNearMin = pShadowCam_->GetNear();
	pShadowCam_->m_fFarMax = pShadowCam_->GetFar();

	float maxCameraRaise = 10 * ONE_METER;
	float maxCameraDrop = 10 * ONE_METER;
	float walkVel = 2 * ONE_METER;
	float runVel = 5 * ONE_METER;

	FpsCam_ = new FirstPersonCamera(eye, at, up, 45.f);
	FpsCam_->setFar(10000.0f);	// CasCaded Shadow Setting
	FpsCam_->setNear(0.1f);
	FpsCam_->SetAspect(width / (FLOAT)height);	
	FpsCam_->setHandednessMode(CAMERA_HANDEDNESS_LEFT);	


	FirstPersonCameraAttrib *pkAttrib = GetGameObjectAttribute(FpsCam_,FirstPersonCameraAttrib);

	if (pkAttrib)
	{
		pkAttrib->m_fMaxCameraRaise = maxCameraRaise;
		pkAttrib->m_fMaxCameraDrop = maxCameraDrop;
		pkAttrib->m_fWalkVelocity = walkVel;
		pkAttrib->m_fRunVelocity = runVel;

	}
	FpsCam_->ComputeProjection();
	//ActiveCam_ = FpsCam_;

	FollowCam_ = new FollowCamera(eye, at, up, 45.f);
	FollowCam_->setFar(10000.0f);	// CasCaded Shadow Setting
	FollowCam_->setNear(0.1f);
	FollowCam_->SetAspect(width / (FLOAT)height);	
	FollowCam_->m_fNearMin = FollowCam_->GetNear();
	FollowCam_->m_fFarMax = FollowCam_->GetFar();

	FollowCam_->setHandednessMode(CAMERA_HANDEDNESS_LEFT);	
	//ActiveCam_ = FollowCam_;
	FollowCam_->ComputeProjection();
}

bool App::load()
{
	OpenDBs();	
	LoadShaders();

	EffectSys_ = new EffectSystem;
	EffectSys_->LoadAll();


	ID3D11Texture2D* pBackBuffer;
	HRESULT hr = swapChain->GetBuffer( 0, __uuidof( *pBackBuffer ), ( LPVOID* )&pBackBuffer );	
	D3D11_TEXTURE2D_DESC TexDesc;
	pBackBuffer->GetDesc( &TexDesc );
	Lightning_ = new LightningEffect(device, context, TexDesc.SampleDesc);
	Lightning_->CreateLightningStructure();
	Lightning_->ResizeSwapChain(width, height, backBufferRTV, depthBufferDSV);

	DestroyShadowMaps();
	PSSMShadowMapDx11 *pShadowMap = new PSSMShadowMapDx11();	
	g_ShadowMaps.push_back(pShadowMap);
	/*for (int i=0; i < g_iNumSplits; ++i)
		g_ShadowMaps.push_back(pShadowMap);*/
	
	if(!pShadowMap->CreateAsTextureCube(1024)) return false;
	
		
	/*float eye[] = { 10.0f, 1.0f, 0.0f };
	float at[] = { 0.0f, 1.0f, 0.0f };
	float up[] = {  0.0f, 1.0f, 0.0f };*/	

	CreateCameras();
	
	// Light Setup	
	m_SceneRoot = new SceneNode;
	m_ActorRoot = new SceneNode;
	m_WorldRoot = new SceneNode;
	m_SceneRoot->AddChild(m_ActorRoot);
	m_SceneRoot->AddChild(m_WorldRoot);

	SetupLight();
	text_ = new Dx11TextHelper(device, context);
	text_->InitFont();

	tex_manager->renderer_ = renderer;
	tex_factory->SetResManager(tex_manager);


	// Initialize the world matrix
	XMMATRIX World = XMMatrixIdentity();
	g_World = ToMat4(World);

	
	// Initialize the view matrix
	XMVECTOR Eye = XMVectorSet( 0.0f, 1.0f, -10.0f, 0.0f );
	XMVECTOR At = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	XMVECTOR Up = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
	XMMATRIX View = XMMatrixLookAtLH( Eye, At, Up );
	g_View = ToMat4(View);

	// Initialize the projection matrix
	XMMATRIX Projection = XMMatrixPerspectiveFovLH( XM_PIDIV4, width / (FLOAT)height, 0.01f, 1000.0f );
	g_Projection = ToMat4(Projection);

	//PathManager::Create();
	//Path* path = new Path("Test");
	//PathManager::Get()->AddPath(path);
	gGameWorld = new GameWorld;
	/*SceneNode test;
	test.SetDir(noVec3(-100, 0, 0), SceneNode::TS_WORLD, noVec3(0, 0, -1));
	noRotation rot_(vec3_zero, noVec3(0, 1, 0), -90);
	noMat3 te_ = rot_.ToMat3();*/

	AppSettings::ShadowModeEnum = SHADOW_NONE;
	AppSettings::EditModeEnum = EDIT_TERRAIN;
	AppSettings::RendererTypeEnum = FORWARD_RENDER;



	gDecoSys.Initialize();

	//LoadMD5();
	LoadeIndexedDeferred();
	//CreateBox();

	//LoadLights();
	//LoadTerrain();
	loader = NULL;
	//LoadModel();
	//LoadM2Particles();
	//LoadM2Ribbons();

	
	//FrustumTestLoad();
	//LoadSSAO();
	// Camera 세팅함.
	//LoadParticles();
	//LoadTessellatedTerrain();

	//CreateNavScene();
	//LoadEntity();

	CreateLightMesh();

	//InitDeferredShading();


	//LoadPhysics();
	ddray = new NavDebugDraw;
	ddray->setOffset(0.15f);
	ddray->CreateVertexBuffer(NULL, 2, sizeof(DebugVertex));
	//ddMain->CreateIndexBuffer(geom->getMesh()->getTris(), geom->getMesh()->getTriCount() * 3);
	ddray->SetupShader();

	if (AppSettings::RendererTypeEnum == FORWARD_RENDER)
	{		
		if( AppSettings::ShadowModeEnum == SHADOW_CASCADED)
			LoadCascadedShadows();
		else 
		{
			if (AppSettings::EditModeEnum == EDIT_TEST_MODE)
			{
				m_speedFlyStrafe = 1;
				LoadEntity();
				//LoadEditObjs();

			}
			else 
			{
				m_speedFlyStrafe = 1000;
				LoadSNBTerrain();

				extern SRenderingParams g_TerrainRenderParams;

				SPatchBoundingBox terrainBB;
				//g_TerrainDX11Render.GetTerrainBoundingBox(terrainBB);

				//uint32 terrainWidth = terrainBB.fMaxX - terrainBB.fMinX;
				//uint32 terrainHeight = terrainBB.fMaxZ - terrainBB.fMinZ;				
				//float fHeight = (g_TerrainRenderParams.m_fGlobalMinElevation + g_TerrainRenderParams.m_fGlobalMaxElevation) * 0.25;*/

				//float centerW = terrainWidth * 0.5f;
				//float centerH = terrainHeight * 0.5f;
				//noVec3 boundCenter(centerW, centerH, fHeight);	

				/*D3DXVECTOR3 vecEye( boundCenter.x, boundCenter.y, boundCenter.z );
				D3DXVECTOR3 vecAt ( boundCenter.x + 1, boundCenter.y, boundCenter.z );
				DefCam_->setFrom((float*)vecEye);
				DefCam_->setTo((float*)vecAt);
				DefCam_->setFOV(RAD2DEG(noMath::PI / 4));
				DefCam_->setNear(0.5f);
				DefCam_->setFar(50000.f);
				DefCam_->SetAspect(GetAspectRatio());
				DefCam_->computeModelView();	
				DefCam_->ComputeProjection();*/

				gShadowReceivers.push_back(&g_TerrainDX11Render);
				g_TerrainDX11Render.m_bOnlyReceiveShadows = true;

				gSun.m_fFOV = 45.f;
				gSun.m_fNear = 0.1f;
				gSun.m_fFar = 50000.f;
				//gSun.m_vTarget = boundCenter;
				//gSun.m_vTarget.y = g_TerrainRenderParams.m_fGlobalMinElevation;

				//LoadSky();
				//LoadWater();
			}
		}
	}
	
	

	return true;
}

void App::SSAO_ShadowPass(float fDeltaTime) {

	SSAORender_->BeginDepthRender();

	bool bWireframe = false;
	bool bShowBoundBoxes = false;
	// Render terrain
	extern CAdaptiveModelDX11Render g_TerrainDX11Render;
	g_TerrainDX11Render.UpdateTerrain( context, ActiveCam_->GetFrom().ToFloatPtr(), ActiveCam_->getProjectionMatrix2());
	g_TerrainDX11Render.Render( context, ActiveCam_->GetFrom().ToFloatPtr(), ActiveCam_->getProjectionMatrix2(), bShowBoundBoxes, false, bWireframe, true);
	
	drawZOnly(gridMesh->m_rd, gridMesh);
	RenderMeshes(true);

	SSAORender_->RenderCS();
}


void App::drawFrame()
{	
	if (!ActiveCam_) return;

	static double _fLastUpdate = 0.0;
	float fDeltaTime = DeltaTimeUpdate(_fLastUpdate);
			
	
	UpdateCamera(fDeltaTime);
	ActiveCam_->Update(fDeltaTime);

	memcpy(&g_View, ActiveCam_->getViewMatrix(), 16 * sizeof(float));			
	memcpy(&g_Projection, ActiveCam_->getProjectionMatrix(), 16 * sizeof(float));

	//g_Light.DoControls();
	
	// Update our time	
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
	renderer->clear(true, true, false, ClearColor, 1.f);
#if 0
	renderer->reset();	
	renderer->setVertexFormat(vf);
	renderer->setVertexBuffer(0, vb);
	renderer->setIndexBuffer(ib);
	renderer->setRasterizerState(cullNone);
	renderer->setShader(shader);
	renderer->setDepthState(noDepthWrite);
	renderer->setBlendState(blendSrcAlpha);
	//renderer->setSamplerState("samLinear", linearClamp);
	//renderer->setTexture("txDiffuse", rt);
	renderer->setShaderConstant4x4f("World", g_World);
	renderer->setShaderConstant4x4f("View", g_View);
	renderer->setShaderConstant4x4f("Projection", g_Projection);	
		
	renderer->apply();
	
	// DrawIndexed	
	renderer->drawElements(PRIM_TRIANGLES, 0, 36, 0, 24);

	//drawModel(gData);

#if 0
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

		for (uint i = 0; i < map->getBatchCount(); i++){
			renderer->setTexture("Base", base[i]);
			renderer->setTexture("Bump", bump[i]);
			renderer->applyTextures();

			map->drawBatch(renderer, i);
		}

	renderer->changeToMainFramebuffer();
#endif
#endif	

	ddt = 0;

	ddt = (timeGetTime() - lastTime);// * animSpeed;
	lastTime = timeGetTime();
		
	if (ddt > 1000) ddt = 1000;

	globalTime += ddt;

	if(GetKeyDown('H')) 
		bShowParticle = !bShowParticle;

#if 0
	g_time.MarkTimeThisTick();	
	GameObjectManager::Get()->Update(fDeltaTime);
	g_msgroute.DeliverDelayedMessages();

	//XMMATRIX xRot = XMMatrixRotationX(-PI * 0.5f);
	GetSceneRoot()->Update(fDeltaTime);
	
	gDecoSys.Update(fDeltaTime);
#endif
	const noVec3 eye = DefCam_->GetFrom();
		
	//if (loader)
	//{	
		//loader->update(ddt, globalTime);

		//UpdateM2Particles(fDeltaTime);	
		//UpdateRibbonEmitters();
		//renderer->resetToDefaults();			
		//loader->DrawModel(g_World, g_View, 	g_Projection);				
		//if (bShowParticle)
		{	
		//RenderM2Particles(_fLastUpdate, g_World, g_View, g_Projection);
		//RenderRibbions(_fLastUpdate, g_World, g_View, g_Projection);
		}
	//}


	//drawShadowedModels();

	//RenderSSAO(this->renderer);

	//UpdateParticles(_fLastUpdate, fDeltaTime);
	
	//RenderParticles(_fLastUpdate);
	bool bMoved = eye != DefCam_->GetFrom() ? true : false;	
	//RenderTerrain2(bMoved, _fLastUpdate, fDeltaTime);	
	UpdateSNBTerrain();

	RenderSNBTerrain();	
#if 0
	if (AppSettings::RendererTypeEnum == FORWARD_RENDER) 	{			
		//RenderLights(fDeltaTime);
		//RenderWater(fDeltaTime);
	
		//RenderNavScene();
		if (AppSettings::ShadowModeEnum == SHADOW_CASCADED) 
			CasCaded_ShadowPass(fDeltaTime);
		if (AppSettings::ShadowModeEnum == SHADOW_SSAO)
			SSAO_ShadowPass(fDeltaTime);
		else 
			PSSM_ShadowPass(fDeltaTime);									
		
		//SetDefaultRenderTarget();			

		if (AppSettings::EditModeEnum == EDIT_TERRAIN) 		{			
			//RenderWaterCaustics();
			//RenderPass_Reflection(fDeltaTime);			
			RenderPass_Main();
			RenderSky(fDeltaTime);			
			gSun.Apply(5);			
			RenderSNBTerrain();	
			gDecoSys.Render(fDeltaTime);
			RenderMeshes(false);
			//RenderWater(fDeltaTime);

		}
		else if (AppSettings::EditModeEnum == EDIT_TEST_MODE) {
		//float blendFactor[4] = {1, 1, 1, 1};
		//context->OMSetBlendState(blendStates_->BlendDisabled(), blendFactor, 0xFFFFFFFF);
		//context->OMSetDepthStencilState(depthStencilStates_->DepthWriteEnabled(), 1);
		//context->RSSetState(rasterizerStates_->BackFaceCull());
			RenderPass_Main();
			

			if (AppSettings::ShadowModeEnum == SHADOW_CASCADED)
				RenderCascasedShadowedScene();
			//else if (AppSettings::ShadowModeEnum == SHADOW_SSAO)			
			//	RenderMeshes(false);			
			else 
			{			
				RenderPSSMScenes(fDeltaTime);
				//RenderEdit(fDeltaTime, GetSecTime());				
			}
		}
		gameLocal.RunFrame();


		if (currMesh_)
		{
			currMesh_->Update(fDeltaTime);
			currMesh_->Draw();
		}		
		Lightning_->Tick(GetSecTime(), ddt / 1000.f);
		Lightning_->Render();
		//RenderPhysics();

		//ID3D11RenderTargetView* pnullView = NULL; 
		//GetApp()->GetContext()->OMSetRenderTargets( 0, &pnullView, NULL );
		DrawFullScreenQuad();

	}
	else {	
		DrwaDeferredPass();
		RenderLID(fDeltaTime);
	}

#endif

#if 0
	ddray->begin(DU_DRAW_LINES_STRIP);

	pickPos_ =		PickUtil::GetPickVert();	
	int col = duRGBA(0,192,255,64);
	//vec4 ray = g_View * vec4(rayOrig_.x, rayOrig_.y, rayOrig_.z, 1);	
	//ddray->vertex(ray.x, ray.y, ray.z, col);
	ddray->vertex(rayOrig_.x, rayOrig_.y, rayOrig_.z, col);	
	ddray->vertex(rayTarget_.x, rayTarget_.y, rayTarget_.z, col);

	ddray->end();


	//if (gGameWorld)
		//gGameWorld->Draw();
	
	++frameNum;

	noVec3 v;	
	ActiveCam_->project(PickUtil::GetPickVert().x, PickUtil::GetPickVert().y, PickUtil::GetPickVert().z,
		getWidth(), getHeight(), v);

	renderer->resetToDefaults();
	renderer->changeToMainFramebuffer();
		
#if 1
	text_->Begin();
	text_->SetInsertionPos(50, 100);
	text_->SetForegroundColor(D3DXCOLOR( 1.0f, 0.0f, 1.0f, 1.0f ) );	
	text_->DrawTextLine(GetStats());
	text_->SetInsertionPos(50, 140);
	text_->DrawFormattedTextLine("Shadow Camera Pos : %f, %f, %f Target : %f, %f, %f", 
	pShadowCam_->GetFrom().x, pShadowCam_->GetFrom().y, pShadowCam_->GetFrom().z, pShadowCam_->GetTo().x, pShadowCam_->GetTo().y, pShadowCam_->GetTo().z);
	text_->SetInsertionPos(50, 160);	
	noVec3 rayDir = rayTarget_ - rayOrig_;
	rayDir.Normalize();
	text_->DrawFormattedTextLine("Ray orig : %f, %f, %f Ray Dir : %f, %f, %f", rayOrig_.x, rayOrig_.y, rayOrig_.z,
		rayDir.x, rayDir.y, rayDir.z);
	
	if (pCascaded_)
	{	
	text_->SetInsertionPos(50, 180);
	text_->DrawFormattedTextLine("Bound Box : Min %f, %f, %f, Max : %f, %f, %f", 
		pCascaded_->GetSceneAABBMin().x, pCascaded_->GetSceneAABBMin().y, pCascaded_->GetSceneAABBMin().z,
		pCascaded_->GetSceneAABBMax().x,pCascaded_->GetSceneAABBMax().y, pCascaded_->GetSceneAABBMax().z);
	}
		
	 if (terrainload_)  {
		 CTerrainPatch* pPatch = g_TerrainDX11Render.GetCurrPatch();
		 noVec2 PatchXTY(-1.0f, -1.0f);
		 if (pPatch) {
			 PatchXTY = g_TerrainDX11Render.GetPatchCornerXY(pPatch);
		 }
		 extern int g_iPatchSize;
		 text_->SetInsertionPos(50, 200);
		 text_->DrawFormattedTextLine("Curr Patch : LeftTop ( %f, %f) RightBottom (%f, %f)", PatchXTY.x, PatchXTY.y, 
			 PatchXTY.x + g_iPatchSize, PatchXTY.y + g_iPatchSize);
	 }

	 CommandSystem::getInstance().Draw();
		
	
#if 0
	{
		WowActor* pActor = (WowActor*)g_database.Find(0);
		if (pActor)
		{	
			AnimationComponent* pAnim = GetGameObjectComponent(pActor, AnimationComponent);			
			text_->SetInsertionPos(50, 150);			
			text_->DrawFormattedTextLine("Player : %s, Curr Anim Id : %d Curr Anim Loop : %s,  CurrFrame : %4d, Current State Name : %s", 
				pActor->GetName(),
				pAnim->AnimSys()->GetCurrSeq(), pAnim->AnimSys()->IsLoop() ? "True" : "False",  pAnim->AnimSys()->GetFrame(), 
				StateNameTable[pActor->GetStateMachine()->GetState()] );
					

			text_->SetInsertionPos(50, 170);			
			if (CombatSystem::GetSingleton().target_)
				text_->DrawFormattedTextLine("Taret : %s",  CombatSystem::GetSingleton().target_->GetName());

			ActorController* pActrl = (ActorController*)pActor->GetStateMachine();
			GameObject* target;
			float dist = 100000;
			pActrl->GetClosestPlayer(target, dist);			
			noVec3 v;	
			noVec3 pos = pActor->GetTranslation();
			pcamera_->project(pos.x, pos.y, pos.z, 	getWidth(), getHeight(), v);
			RECT rc;
			rc.left = v.x;
			rc.top = getHeight() - v.y;
			text_->DrawFormattedTextLine(rc, 0, "Player : (%f, %f, %f) Dist : %f", pos.x, pos.y, pos.z, (pActrl->getPathEnd() - pActor->GetTranslation()).Length());
			
		}

		pActor = (WowActor*)g_database.Find(1);
		if (pActor)
		{
			AnimationComponent* pAnim = GetGameObjectComponent(pActor, AnimationComponent);			
			text_->SetInsertionPos(50, 190);			
			text_->DrawFormattedTextLine("Monster : %s Curr Anim Id : %d Curr Anim Loop : %s, CurrFrame : %4d, Current State Name : %s", 
				pActor->GetName(),
				pAnim->AnimSys()->GetCurrSeq(), pAnim->AnimSys()->IsLoop() ? "True" : "False",
				pAnim->AnimSys()->GetFrame(), StateNameTable[pActor->GetStateMachine()->GetState()] );

			ActorController* pActrl = (ActorController*)pActor->GetStateMachine();
			GameObject* target = NULL;
			float dist = 10000;
			pActrl->GetClosestPlayer(target, dist);
			text_->SetInsertionPos(50, 210);			
			if (target)
				text_->DrawFormattedTextLine("Enity Dist : %f ", dist);
			text_->SetInsertionPos(50, 230);			
			text_->DrawFormattedTextLine("Velocity : %f, %f Max Speed : %f", pActrl->Velocity().x, pActrl->Velocity().y, 
				pActrl->MaxSpeed());
		}		
	}
#endif
	RECT rc;
	rc.left = v.x;
	rc.top = getHeight() - v.y;
	text_->DrawFormattedTextLine(rc, 0, "(%d, %d)", rc.left, rc.top);

	text_->End();


	DrawTextShadowDebug();
	drawGUI();
#endif
#endif

	mMouseDelta.x = 0;
	mMouseDelta.y = 0;
}


void App::controls()
{
	D3D11App::controls();
}

void App::LoadTerrain()
{
	TerrainConfig cfg;
	pTerrain = new Terrain();
	const char* szAbsDir = "Data/Project";
	pTerrain->CreateTerrain(renderer, &cfg, NULL, false);
	pTerrain->m_SectorManager.EnsureSharedMeshesCreated();
	delete pTerrain;
}



void App::LoadModel( const noVec3 pos )
{
		
	M2Mesh* pMesh = new M2Mesh;
	loader = new M2Object(renderer, NULL); 
	loader->SetViewMatrix(g_View);
	pMesh->m2_ = loader;
	currMesh_ = pMesh;

	m2edit.Init(renderer, pMesh, NULL);
	//m2edit.cachedCombosedTex = TexPath("Skins\\Character\\BloodElf\\Female\\Warrior_Body.tga");	
	
	pMesh->SetScale(1.f);		
	noVec3 temp = pos;
	temp.y += 2.0f;
	pMesh->SetTrans(temp);

	
	//wxString fn("Creature\\AbyssalOutland\\Abyssal_Outland.m2");//("Character\\BloodElf\\female\\BloodElfFemale");
	//wxString fn(modelname[0]);
	wxString fn(Spells[spellIndex]);

	bool isChar =  ((fn.Lower().Find(wxT("char"))  != wxNOT_FOUND )|| 
		(fn.Lower().Find(wxT("alternate\\char")) != wxNOT_FOUND));

	//loader->Load(, true);		
	loader->Load(fn.wx_str(), true);	
	if (!isChar)
		loader->modelType = MT_NORMAL;
	
	loader->animManager->SetAnim(0, 0, true);
	AnimationSystem* animSys = (AnimationSystem*)loader->animManager;
	animSys->SetLoop(true);
	loader->animManager->Play();

	//loader->Load("Data/sword_1h_draenei_a_02.m2", true);
	CreateRenderModel(loader);

	loader->GetRenderData(gData);

	pMesh->CreateBuffers();	
	//std::string texturefile;
	//texturefile.assign("Data/Sword_1H_Draenei_A_02Blue.tga");
	//texturefile.assign("Data/Sword_1H_Draenei_A_02Blue.tga");
	//gData.baseTex = renderer->addTexture(texturefile.c_str(), true, gData.linear);				
}

void App::drawShadow( const RenderData& rd, Mesh* mesh, ShaderID shaderid /*= rd.shader*/ )
{
	renderer->reset();	
	renderer->setVertexFormat(rd.vf);
	renderer->setVertexBuffer(0, rd.vb);
	renderer->setIndexBuffer(rd.ib);
	renderer->setRasterizerState(rd.cull);
	renderer->setShader(gPssm);	
	//renderer->setSamplerState("samLinear", render_data[0].linear);
	//renderer->setTexture("txDiffuse", rd.baseTex);			
	if (rd.blendMode != -1) renderer->setBlendState(rd.blendMode);
	if (rd.depthMode != -1) renderer->setDepthState(rd.depthMode);

	renderer->setShaderConstant1i("g_iFirstSplit", mesh->m_iFirstSplit);
	renderer->setShaderConstant1i("g_iLastSplit", mesh->m_iLastSplit);								
	renderer->setShaderConstantArray4x4f("g_mCropMatrix", &mCropMatrix[0], 4);

	noMat4 tm(mesh->worldTM_.rotate_ * mesh->worldTM_.scale_, mesh->worldTM_.trans_);
	tm.TransposeSelf();	
	mat4 world = ToMat4(tm);
	renderer->setShaderConstant4x4f("g_mWorld", world);
	renderer->setShaderConstant4x4f("g_mViewProj", ToMat4(gSun.m_mView) * ToMat4(gSun.m_mProj));

	renderer->apply();				
	GetApp()->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	int iNumInstances = mesh->m_iLastSplit - mesh->m_iFirstSplit + 1;
	D3D11Context()->DrawIndexedInstanced(rd.endIndex, iNumInstances, rd.startIndex, 0, 0);				
}

void App::drawZOnly( const RenderData& rd, Mesh* mesh)
{
	renderer->reset();
	renderer->setVertexFormat(rd.vf);				
	renderer->setVertexBuffer(0, rd.vb);				
	renderer->setIndexBuffer(rd.ib);				
	noMat4 tm(mesh->worldTM_.rotate_ * mesh->worldTM_.scale_, mesh->worldTM_.trans_);
	tm.TransposeSelf();	
	
	renderer->apply();	

	if (AppSettings::ShadowModeEnum == SHADOW_SSAO)
		GetApp()->SSAORender_->SetDepthShader((noMat4*)&tm);
	else 
		LIDRenderer::GetSingletonPtr()->BeginDepthRender(GetApp()->ActiveCam_, D3D11Context(), tm.ToFloatPtr());									

	//renderer->setSamplerState("g_samLinear", render_data[0].linear);				
	//renderer->changeTexture(rd.shader, "g_txDiffuse", rd.baseTex);		
	renderer->drawElements(rd.prim, rd.startIndex, rd.endIndex, 0, rd.vertexEnd);		
}

void App::drawSSAO( const RenderData& rd, Mesh* mesh, ShaderID shaderid /*= rd.shader*/ ) {
	renderer->reset();	
	renderer->setShader(rd.shader);
	renderer->changeVertexFormat(rd.vf);
	renderer->changeVertexBuffer(0, rd.vb);
	renderer->changeIndexBuffer(rd.ib);			

	noMat4 tm(mesh->worldTM_.rotate_ * mesh->worldTM_.scale_, mesh->worldTM_.trans_);
	tm.TransposeSelf();	
	mat4 world = ToMat4(tm);

	renderer->setShaderConstant4x4f("World", world);
	//renderer->setShaderConstant4x4f("g_mViewProj", view * proj);
	renderer->setShaderConstant4x4f("View", g_View);

	//mat4 invView = transpose(view);
	//renderer->setShaderConstant4x4f("InvView", invView);
	//renderer->setShaderConstant3f("g_localEye", invView.getTrans());
	renderer->setShaderConstant4x4f("Projection",  g_Projection);

	renderer->applyConstants();

	Direct3D11Renderer* Dx11R = (Direct3D11Renderer*)renderer;
	ID3D11ShaderResourceView* srv = Dx11R->getTextureSRV(rd.baseTex);
	D3D11Context()->PSSetShaderResources(0, 1, &srv);		

	ID3D11ShaderResourceView* aoSRV[] ={ GetApp()->SSAORender_->GetAOSRV() };
	D3D11Context()->PSSetShaderResources(1, 1, aoSRV);					
		
	//renderer->setShaderConstantArray4x4f("g_matrices", &temp[0], header.nBones);				
	renderer->drawElements(rd.prim, rd.startIndex, rd.endIndex, 0, rd.vertexEnd);															
}

void App::drawShadowed( const RenderData& rd, Mesh* mesh, ShaderID shaderid /*= rd.shader*/ )
{
	//float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
	//renderer->clear(true, true, false, ClearColor, 1.f);
	renderer->resetToDefaults();	
	renderer->reset();
	renderer->setVertexFormat(rd.vf);
	renderer->setVertexBuffer(0, rd.vb);
	renderer->setIndexBuffer(rd.ib);
	//renderer->setRasterizerState(rd.cull);
	//renderer->setDepthState(rd.depthMode);
	renderer->setShader(shaderid);	
	if (rd.baseTex != -1)
	{
		renderer->setTexture("g_txDiffuse", rd.baseTex);
		renderer->setSamplerState("g_samLinear", rd.diffuseSampler);
	}
	
	renderer->setShaderConstantArray1f("g_fSplitPlane", &g_fSplitPos[1], g_iNumSplits);	

	noMat4 tm(mesh->worldTM_.rotate_ * mesh->worldTM_.scale_, mesh->worldTM_.trans_);
	tm.TransposeSelf();	
	mat4 world = ToMat4(tm);
		
	renderer->setShaderConstant4x4f("g_mWorld", world);
	renderer->setShaderConstant4x4f("g_mViewProj", g_View * g_Projection);
	renderer->setShaderConstant4x4f("g_mView", g_View);				
	renderer->setShaderConstant3f("g_vLightDir", gSun.GetDir().ToFloatPtr());
	renderer->setShaderConstant3f("g_vLightColor", vec3(gSun.m_diffuseColor.ToFloatPtr()));
	renderer->setShaderConstant3f("g_vAmbient", vec3(gSun.m_ambientColor.ToFloatPtr()));	
	renderer->setShaderConstant4x4f("g_mWorldViewProjection", world *g_View * g_Projection);	
	renderer->setShaderConstantArray4x4f("g_mTextureMatrix", &mTextureMatrix[0], g_iNumSplits);
	renderer->apply();	

	int slot = 0;
	if (rd.baseTex != -1)
		slot = 1;
	// DrawIndexed	
	ID3D11SamplerState* ss[] = { LIDRender_->GetSamplerState().ShadowMap() };
	D3D11Context()->PSSetSamplers(slot, 1, ss);
	D3D11Context()->PSSetShaderResources(slot, 1, &GetShadowMap<PSSMShadowMapDx11>()->m_pSRV);		
	// bind shadow map textures	
	/*ID3D11ShaderResourceView *pResources[NUM_SPLITS_IN_SHADER];
	for(int i = 0; i < g_iNumSplits; i++)
	{
		pResources[i] = GetShadowMap<PSSMShadowMapDx11>(i)->m_pSRV;
	}
	D3D11Context()->PSSetShaderResources(0, g_iNumSplits, pResources);		*/
	renderer->drawElements(rd.prim, rd.startIndex, rd.endIndex, 0, rd.vertexEnd);

	// Unbind render targets
	// Unbind shader resources
	ID3D11ShaderResourceView* nullViews[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	GetApp()->GetContext()->PSSetShaderResources(0, 8, nullViews);
	
}

void App::drawModel( const RenderData& rd, const mat4& world, const mat4& view, const mat4& proj )
{
	//float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
	//renderer->clear(true, true, false, ClearColor, 1.f);
	renderer->reset();
	renderer->setVertexFormat(rd.vf);
	renderer->setVertexBuffer(0, rd.vb);
	renderer->setIndexBuffer(rd.ib);

	if (rd.cull == -1) renderer->setRasterizerState(cullBack);
	else  renderer->setRasterizerState(rd.cull);
	if (rd.depthMode == -1) renderer->setDepthState(depthWrite_);
	else renderer->setDepthState(rd.depthMode);
	renderer->setShader(rd.shader);	
	if (rd.baseTex != -1)  renderer->setTexture("g_txDiffuse", rd.baseTex);
	if (rd.diffuseSampler == -1) renderer->setSamplerState("g_samLinear", linearClamp);
	else renderer->setSamplerState("g_samLinear", rd.diffuseSampler);
	renderer->setShaderConstant4x4f("World", world);
	renderer->setShaderConstant4x4f("View", view);
	renderer->setShaderConstant4x4f("Projection", proj);	
	vec3 eye;
	ActiveCam_->getFrom((float*)eye);
	/*renderer->setShaderConstant3f("vecEye", eye);
	vec3 Lightdir(gLight.GetDir().x, gLight.GetDir().y, gLight.GetDir().z);	
	renderer->setShaderConstant3f("vecLightDir", Lightdir);
	vec4 diffuse(gLight.m_diffuseColor.x, gLight.m_diffuseColor.y, gLight.m_diffuseColor.z, 1.0f);
	renderer->setShaderConstant4f("vDIC", diffuse);
	vec4 specular(gLight.m_specularColor.x, gLight.m_specularColor.y, gLight.m_specularColor.z, 10.f);
	renderer->setShaderConstant4f("vSpecIC", specular);	*/
	renderer->apply();

	// DrawIndexed	
	renderer->drawElements(rd.prim, rd.startIndex, rd.endIndex, 0, rd.vertexEnd);
}

void App::UpdateModelInfo(M2Loader* m2)
{	
	M2ActorDetails& cd = m2edit.cd;
	TabardDetails& td = m2edit.td;
	
	cd.reset();
	td.showCustom = false;

	// hide most geosets
	m2->HideGeosets();
		
	wxString raceName;
	wxString genderName;

	m2->GetRaceAndGenderName(raceName, genderName);

	unsigned int race, gender;

	try {
		// Okay for some reason Blizzard have removed the full racial names
		// out of the ChrRaces.dbc.  Going to have to hardcode the values.
		CharRacesDB::Record raceRec = racedb.getByName(raceName);
		race = raceRec.getUInt(CharRacesDB::RaceID);
		gender = (genderName.Lower() == wxT("female")) ? GENDER_FEMALE : GENDER_MALE;

	} catch (CharRacesDB::NotFound) {
		// wtf
		race = 0;
		gender = GENDER_MALE;
	}

	// Enable the use of NPC skins if its  a goblin.
	if (race == RACE_GOBLIN && gameVersion < VERSION_CATACLYSM)
		cd.useNPC=1;
	else
		cd.useNPC=0;

	if (race==RACE_TAUREN || race==RACE_TROLL || race==RACE_DRAENEI || race==RACE_NAGA || race==RACE_BROKEN) // If its a troll/tauren/dranei/naga/broken, show the feet (dont wear boots)
		cd.showFeet = true;
	else
		cd.showFeet = false;

	// hardcoded
	if (0 && race == RACE_WORGEN) {
		m2->showGeosets[3] = false;
		m2->showGeosets[4] = false;
		m2->showGeosets[72] = false;
		m2->showGeosets[81] = false;
	}

	// get max values
	cd.maxSkinColor = chardb.getColorsFor(race, gender, CharSectionsDB::SkinType, 0, cd.useNPC);
	if (cd.maxSkinColor==0 && cd.useNPC==1) {
		wxMessageBox(wxT("The selected character does not have any NPC skins!\nSwitching back to normal character skins."));
		cd.useNPC = 0;
		cd.maxSkinColor = chardb.getColorsFor(race, gender, CharSectionsDB::SkinType, 0, cd.useNPC);
	}
	cd.maxFaceType  = chardb.getSectionsFor(race, gender, CharSectionsDB::FaceType, 0, cd.useNPC);
	cd.maxHairColor = chardb.getColorsFor(race, gender, CharSectionsDB::HairType, 0, 0);
	cd.maxFacialHair = facialhairdb.getStylesFor(race, gender);
	cd.maxFacialColor = cd.maxHairColor;

	cd.race = race;
	cd.gender = gender;

	std::set<int> styles;
	for (CharHairGeosetsDB::Iterator it = hairdb.begin(); it != hairdb.end(); ++it) {
		if (it->getUInt(CharHairGeosetsDB::Race)==race && it->getUInt(CharHairGeosetsDB::Gender)==gender) {
			styles.insert(it->getUInt(CharHairGeosetsDB::Section));
		}
	}
	cd.maxHairStyle = (int)styles.size();

	if (cd.maxFaceType==0) cd.maxFaceType = 1;
	if (cd.maxSkinColor==0) cd.maxSkinColor = 1;
	if (cd.maxHairColor==0) cd.maxHairColor = 1;
	if (cd.maxHairStyle==0) cd.maxHairStyle = 1;
	if (cd.maxFacialHair==0) cd.maxFacialHair = 1;

	td.Icon = randint(0, td.maxIcon);
	td.IconColor = randint(0, td.maxIconColor);
	td.Border = randint(0, td.maxBorder);
	td.BorderColor = randint(0, td.maxBorderColor);
	td.Background = randint(0, td.maxBackground);
}

void App::CreateRenderModel(M2Loader* m2)
{
	wxString fn(m2->name.c_str());
	fn = fn.Lower();
	if (fn.substr(0, 4) == wxT("char")) {

		UpdateModelInfo(m2);

		{	
			std::ifstream f("Data/elf.chr");/*("Data/abyssal_outland.chr");*/ 

			std::string modelname;
			f >> modelname;

			f >> m2edit.cd.race >> m2edit.cd.gender;
			f >> m2edit.cd.skinColor >> m2edit.cd.faceType >> m2edit.cd.hairColor >> m2edit.cd.hairStyle >> m2edit.cd.facialHair >> m2edit.cd.facialColor;

			if (f.peek() != '\n') {
				f >> m2edit.cd.eyeGlowType;
			}
			else 
				m2edit.cd.eyeGlowType = EGT_DEFAULT;

			while (!f.eof()) {
				for (size_t i=0; i<NUM_CHAR_SLOTS; i++) {
					f >> m2edit.cd.equipment[i];
				}
				break;
			}

			// 5976 is the ID value for the Guild Tabard, 69209 for the Illustrious Guild Tabard, and 69210 for the Renowned Guild Tabard
			if (((m2edit.cd.equipment[CS_TABARD] == 5976) || (m2edit.cd.equipment[CS_TABARD] == 69209) || (m2edit.cd.equipment[CS_TABARD] == 69210)) && !f.eof()) {
				f >> m2edit.td.Background >> m2edit.td.Border >> m2edit.td.BorderColor >> m2edit.td.Icon >> m2edit.td.IconColor;
				m2edit.td.showCustom = true;
			}

			f.close();
		}

		m2edit.UpdateCharModel(m2);

		m2edit.UpdateEyeGlow(m2);

		m2edit.UpdateEquipment();
		
	}	
	else  if (fn.substr(0,8) == wxT("creature"))
	{
		m2edit.UpdateCreatureModel(m2);

	}
	else if (fn.substr(0,4) == wxT("item"))
	{
			
	}
		

	m2->CreateRenderPass();

	m2->SetViewMatrix(g_View);
}





float App::GetAspectRatio(void)
{
	RECT rcClient;
	GetClientRect(GetAppBase()->getWindow(), &rcClient);
	unsigned int iClientWidth = rcClient.right - rcClient.left;
	unsigned int iClientHeight = rcClient.bottom - rcClient.top;
	return iClientWidth / (float)iClientHeight;
}

void App::SetDefaultRenderTarget( void )
{
	float ClearColor[4] = { 0.0f, 0.f, 0.f, 1.0f }; // red,green,blue,alpha
	context->OMSetRenderTargets(1, &backBufferRTV, depthBufferDSV);
	renderer->clear(true, true, false, ClearColor, 1.f);


	RECT rcClient;
	GetClientRect(getWindow(), &rcClient);
	unsigned int iClientWidth = rcClient.right - rcClient.left;
	unsigned int iClientHeight = rcClient.bottom - rcClient.top;

	D3D11_VIEWPORT vp;
	vp.Width = iClientWidth;
	vp.Height = iClientHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	context->RSSetViewports( 1, &vp );
}

void App::exitAPI()
{
	QuadVS_.Detach();
	QuadPS_.Detach();
	QuadInputLayout_.Detach();
	QuadVB_.Detach();


	SAFE_DELETE(ddray);
	SAFE_DELETE(text_);
	SAFE_DELETE(Lightning_);
	SAFE_DELETE(gGameWorld);

	SAFE_DELETE(DefCam_);
	SAFE_DELETE(pShadowCam_);
	SAFE_DELETE(FpsCam_);
	SAFE_DELETE(FollowCam_);
	
	SAFE_RELEASE(main_color_resource);
	SAFE_RELEASE(main_color_resourceSRV);
	SAFE_RELEASE(main_color_resourceRTV);

	SAFE_RELEASE(main_color_resource_resolved);
	SAFE_RELEASE(main_color_resource_resolvedSRV);

	SAFE_RELEASE(main_depth_resource);
	SAFE_RELEASE(main_depth_resourceDSV);
	SAFE_RELEASE(main_depth_resourceSRV);

	SAFE_DELETE(timer_);
	SAFE_DELETE(msgRoute_);
	SAFE_DELETE(dLog_);
	SAFE_DELETE(combat_);
	SAFE_DELETE(m_SceneRoot);

	GameObjectManager::Destroy();

	SAFE_DELETE(msgRoute_);
	SAFE_DELETE(dLog_);
	SAFE_DELETE(timer_);

	SAFE_DELETE(m_navMesh);
	g_TerrainDX11Render.OnD3D11DestroyDevice();
	DestoryTerrainRes();
	DestorySSAO();
	SAFE_DELETE(loader);	
	SAFE_DELETE(m_SceneRoot);

	SAFE_DELETE(EffectSys_);
	for (size_t i=0; i < g_ShadowMaps.size() ;++i)
		delete g_ShadowMaps.at(i);
	g_ShadowMaps.clear();

	SAFE_DELETE(Lightning_);

	SAFE_DELETE(depthStencilStates_);
	SAFE_DELETE(blendStates_);
	SAFE_DELETE(rasterizerStates_);
	SAFE_DELETE(samplerStates_);
	
	SAFE_DELETE(SSAORender_);

	CMeshBundle::finalize();

	// shut down the string memory allocator
	idStr::ShutdownMemory();

	// shut down the SIMD engine
	idSIMD::Shutdown();

	// shut down the memory manager
	Mem_Shutdown();

	D3D11App::exitAPI();
}

void App::ReleaseRes()
{
	

	ReleaseTerrain2();
	ReleaseingSNBTerrain();
}

const char* App::GetStats()
{
	ZeroMemory(stats, 4096);	
	POINT pos;
	GetCursorPos(&pos);
	ScreenToClient(getWindow(), &pos);

	noVec3 pick = PickUtil::GetPickVert();
	sprintf_s(stats, 4096, "Camera Pos : %0.2f, %0.2f, %0.2f Mouse : Pos : ( %d, %d ) Picking Pos : %f, %f, %f",
		ActiveCam_->GetFrom().x, ActiveCam_->GetFrom().y, ActiveCam_->GetFrom().z, pos.x, pos.y, pick.x, pick.y, pick.z);

	return stats;
}

void App::SetupLight()
{
#if 0
	float fMaxLength = 1.414214f * 1000;
	float fHeight = (fMaxLength + 32.0f) / tanf(g_Light.m_fFOV * 0.5f);
	g_Light.m_vSource = noVec3(0, fHeight, 0);
	g_Light.m_vTarget = noVec3(0, 0, 0);
#endif	

	float fMaxLength = 1.414214f * 1000;
	float fHeight = (fMaxLength + 32.0f) / tanf(gSun.m_fFOV * 0.5f);
	gSun.m_vLitPos = noVec3(0, fHeight, 0);
	gSun.m_vTarget = noVec3(0, 0, 0);
	gSun.m_vLitUp = noVec3(0, 1, 0);	

	//gLight.m_vLitPos = noVec3(-10000.0f,6500.0f,10000.0f);
	

	// set light
	gSun.Initialize();
}

#define XY_PAN_SPEED 0.0005f
#define WHEEL_ZOOM_SPEED 0.0001f
#define ZOOM_SPEED 0.001f

bool App::onMouseMove( const int x, const int y, const int deltaX, const int deltaY )
{
	//Pick(x, y, m_SceneRoot);
	mMousePos.x = x;
	mMousePos.y = y;
	mMouseDelta.x = deltaX;
	mMouseDelta.y = deltaY;	

	int inv_y = getHeight() - y; // hkgWindowDx10PCProc 참조
	if ( (deltaX == 0) && (deltaY == 0))
		return false; // a few functions fail with 0 delta.

	/*int _deltaX = deltaX, _deltaY = deltaY;
	if (deltaX < -MAX_MOUSE_DELTA)
		_deltaX = -MAX_MOUSE_DELTA;
	else if (deltaX > MAX_MOUSE_DELTA)
		_deltaX = MAX_MOUSE_DELTA;

	if (deltaY < -MAX_MOUSE_DELTA)
		_deltaY = -MAX_MOUSE_DELTA;
	else if (deltaY > MAX_MOUSE_DELTA)
		_deltaY = MAX_MOUSE_DELTA;*/
	
	if (m_navMode == CAMERA_NAV_TRACKBALL)
	{
		bool doRotate = false;
		bool doPan = false;
		bool doZoom = false;

		interpretCurrentTrackballAction(doRotate, doPan, doZoom);

		if ( doRotate )
		{
			int vw = m_winUR[0] - m_winLL[0];
			int vh = m_winUR[1] - m_winLL[1];
			int mx = x - m_winLL[0];
			int my = inv_y - m_winLL[1];
			
			//printf("moving trackball to w%d h%d x%d y%d\n", vw, vh, mx, my);
			DefCam_->moveTrackball( vw, vh, mx, my );
			//pShadowCam_->moveTrackball( vw, vh, mx, my );
			//pcamera_->moveTrackball( 640, 500, 450, 64 );
		}

		if ( doPan )
		{
			// pan
			float speedX = -deltaX * XY_PAN_SPEED * m_speedMousePan;
			float speedY = -deltaY * XY_PAN_SPEED * m_speedMousePan;
			if (m_mouseAbsoluteSpeed)
			{
				DefCam_->pan(speedX * 10, speedY * 10, true, false ); //x10 as absolute ~ proportional the same at 10 meters from POI
			}
			else
			{
				DefCam_->pan(speedX, speedY, true, true); 
			}
		}

		// does this even when not in tackball
		if ( doZoom )
		{
			int delta;
			delta = deltaY;	
			
			float speed = delta * ZOOM_SPEED * m_speedMouseButtonZoom;
			ActiveCam_->dolly( speed, false, true); // no such thing as absolute Zoom.
		}

	}


	return D3D11App::onMouseMove(x, y, deltaX, deltaY);
}

bool App::onMouseButton( const int x, const int y, const MouseButton button, const bool pressed )
{
	mMousePos.x = x;
	mMousePos.y = y;
	int inv_y = getHeight() - y; // hkgWindowDx10PCProc 참조

	m_lbPressed = (button == MOUSE_LEFT && pressed);
	m_rbPressed = (button == MOUSE_RIGHT && pressed);
	m_mbPressed = (button == MOUSE_MIDDLE && pressed);

	if (m_lbPressed)
	{
		
		if (PickUtil::Picking(m_SceneRoot))
		{			
			gGameWorld->SetCrosshair(PickUtil::pickDatas[0].vert);
			//gGameWorld->HandleFindPath();			
			//gGameWorld->TestOffsetPursuit();
			//gGameWorld->TestPursuit();
			//gGameWorld->TestSeek();
		}
	}

	int vw = m_winUR[0] - m_winLL[0];
	int vh = m_winUR[1] - m_winLL[1];

	if (m_navMode == CAMERA_NAV_TRACKBALL)
	{
		bool doRotate = false;
		bool doPan = false;
		bool doZoom = false;
		interpretCurrentTrackballAction(doRotate, doPan, doZoom);

		if ( doRotate )
		{
			// printf(" starting trackball @ w%d h%d x%d y%d\n", vw, vh, x - m_winLL[0], y - m_winLL[1] );
			DefCam_->startTrackball( vw, vh, x - m_winLL[0],inv_y - m_winLL[1] );			
		}
	}
	
	return D3D11App::onMouseButton(x, y, button, pressed);
}

void App::interpretCurrentTrackballAction (bool& doRotate, bool& doPan, bool& doZoom )
{	
	const bool altPressed = GetKeyDown(VK_SHIFT);
	const bool ctrlPressed = GetKeyDown(VK_CONTROL);

		
	doZoom = m_mbPressed || (m_lbPressed && altPressed); // allow alt to mimic the middle mouse
	doRotate = m_lbPressed && !altPressed;
	doPan = m_rbPressed; 

}

bool App::Pick(SceneNode* pRoot, const noVec3& from, const noVec3& dir)
{
	

	Mesh* mesh = dynamic_cast<Mesh*>(pRoot);
	if (mesh)
	{
		M2Mesh* m2;
		m2 =dynamic_cast<M2Mesh*>(mesh);
		if (m2 && m2->GetBbox())
		{			
			if (PickUtil::Intersect(m2->GetBbox(), from, dir))			
			{			
				PickUtil::ms_pickData.pUserData = m2->m2_;
				PickUtil::SavePickData();
				return true;
			}
			
		}
		Mesh_D3D11* normalMesh;
		if (normalMesh = dynamic_cast<Mesh_D3D11*>(mesh))
		{
		if (PickUtil::Intersect(normalMesh, from, dir))			
		{					
		PickUtil::SavePickData();
		return true;
		}			
		}	
	}
	CDXUTSDKMesh* dxutmesh;
	if (dxutmesh = dynamic_cast<CDXUTSDKMesh*>(pRoot))
	{
		if (PickUtil::Intersect(dxutmesh, from, dir))			
		{					
			PickUtil::SavePickData();
			return true;
		}			
	}

	rcMeshLoaderObj* mesh2 = dynamic_cast<rcMeshLoaderObj*>(pRoot);
	if (mesh2)
		if (PickUtil::Intersect(&mesh2->objLoader, from, dir))
		{
			PickUtil::SavePickData();
			return true;
		}

	for (size_t i =0; i < pRoot->childNodes_.size(); ++i)
	{		
		if (Pick(pRoot->childNodes_[i], from, dir))
			return true;
	}	

	return false;
}


bool App::Pick(int x, int y, SceneNode* pRoot)
{
	noVec3 from;
	noVec3 dir;

	D3DXVECTOR3 vorig, vdir;
	GetRay(vdir, vorig);

	rayOrig_ = noVec3(vorig.x, vorig.y, vorig.z);

	//pcamera_->unProject(x, y, 0.0f, getWidth(), getHeight(), from);
	//pcamera_->unProject(x, y, 1.0f, getWidth(), getHeight(), to);
	from = noVec3(vorig.x, vorig.y, vorig.z);
	dir = noVec3(vdir.x, vdir.y, vdir.z);
	
	pickPos_ = rayOrig_ + 1000 * dir;
	//dir = rayOrig_ + 1000 * dir;;

	for (size_t i =0; i < pRoot->childNodes_.size(); ++i)
	{
		Mesh* mesh = dynamic_cast<Mesh*>(pRoot->childNodes_[i]);
		if (mesh)
			if (PickUtil::Intersect(mesh, from, dir))			
				return true;
		rcMeshLoaderObj* mesh2 = dynamic_cast<rcMeshLoaderObj*>(pRoot->childNodes_[i]);
		if (mesh2)
			if (PickUtil::Intersect(&mesh2->objLoader, from, dir))
				return true;
	}	
	
	return false;
}


