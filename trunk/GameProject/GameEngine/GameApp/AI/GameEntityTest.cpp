#include "stdafx.h"
#include <EngineCore/Util/Model.h>
#include <EngineCore/Util/GameLog.h>

#include "GameApp/GameApp.h"
#include "GameApp/Mesh.h"
#include "GameApp/SceneNode.h"
#include "GameApp/M2Mesh.h"
#include "GameApp/Mesh_D3D11.h"
#include "GameApp/TestObjects.h"
#include "GameApp/GameObjectManager.h"
#include "GameApp/GameObjectUtil.h"
#include "GameApp/Common.h"
#include "ModelLib/M2Loader.h"
#include "AICommon.h"
#include "AIConfig.h"
#include "PathManager.h"
#include "navigationmesh.h"
#include "GameWorld.h"
#include "GameApp/Actor.h"
#include "GameApp/Font/Dx11TextHelper.h"

#include "GameApp/Terrain/AdaptiveModelDX11Render.h"
#include "../PSSMShadowMapDx11.h"
#include "../DemoSetup.h"
#include "../U2Light.h"
#include "../ObjLoader.h"
#include "Gameapp/Effects/EffectSystem.h"
#include "Gameapp/Effects/LightningEffect.h"
#include "GameApp/GameCamera.h"
#include "GameApp/LIDRenderer.h"

enum { MAX_MESH = 10 };
extern GameWorld* gGameWorld;
static RenderData grid_rd;
static RenderData test_rd[10];
static ShaderID lighting[2];
static VertexFormatID lit_vf;
static RenderData light_rd;
static Model *sphere;
static CMeshLoader10 camera;


static const char* modelname[] = {
	"Character\\BloodElf\\female\\BloodElfFemale.m2",
	"Creature\\AbyssalOutland\\Abyssal_Outland.m2",
	"Creature\\alexstrasza\\alexstrasza.m2",
	"Creature\\arthas\\arthas.m2",
	"Creature\\FireSpiritsmall\\firespiritsmall.m2",
	"Environments/AZEROTH/Elwynn/elwynnfirtree01.m2",
	"World\\GENERIC\\HUMAN\\Passive Doodads\\ArcheryTargets\\ArcheryTarget_Solo.M2",
	"World\\GENERIC\\HUMAN\\Passive Doodads\\ArcheryTargets\\StormWindArcheryTarget01.m2"		
};

static const char* ammonname[] = {
	"Item/ObjectComponents/Ammo/arrowfireflight_01.m2"
};

 extern CAdaptiveModelDX11Render g_TerrainDX11Render;
 extern NavigationMesh			NaviMesh;		/* our navigation mesh */
 U2Light gSun;
 std::vector<Mesh*>	gShadowReceivers;


 bool App::CreateLightMesh()
 {
#if 0
	sphere = new Model();
	sphere->loadObj("Models/sphere.obj");		
	if ((lighting[0] = renderer->addShader("Data/Shaders/SimpleColor.hlsl")) == SHADER_NONE) return false;
	//if (!sphere->makeDrawable(renderer, true, lighting[0])) return false;
	light_rd.shader = lighting[0];
	if (!sphere->makeDrawable(renderer, true, lighting[0])) return false;
#endif
	//rd.cull = gRenderer->addRasterizerState(CULL_BACK);	
	if ((lighting[0] = renderer->addShader("Data/Shaders/SimpleColor.hlsl")) == SHADER_NONE) return false;

	FormatDesc vbFmt[] = {
		0, TYPE_VERTEX, FORMAT_FLOAT,  3,
		0, TYPE_NORMAL, FORMAT_FLOAT,   3,
		//0, TYPE_TANGENT, FORMAT_FLOAT, 3,
		//0, TYPE_BINORMAL,FORMAT_FLOAT, 3,
		0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,
		//0, TYPE_COLOR,	FORMAT_FLOAT,	4,
	};
	lit_vf = renderer->addVertexFormat(vbFmt, elementsOf(vbFmt), lighting[0]);
	Mesh* litMesh =  CMeshBundle::getInstance().getResourceById("ball");
	
	litMesh->SetTrans(gSun.m_vLitPos);;
	m_WorldRoot->AddChild(litMesh);

	return true;
}

 bool App::CreateCamera()
 {	 
	camera.Create(device, "Models/camera.obj");		
	return true;
 }
 
 void App::NewEntity( int nSlot )
 {	

	static bool bFirst = false;
	if (!bFirst)
	{
		//noRotation rot(vec3_zero, noVec3(0.f, 1.0f, 0.f), -90.f);
		//m_ActorRoot->SetRotate(rot.ToMat3());
		bFirst = true;
	}


	if (AppSettings::EditModeEnum == EDIT_TERRAIN)
	{
		if (terrainload_ )
		{
			noVec3 pos = Pick();	
			PickUtil::ms_pickData.vert = pos;
			if (AppSettings::AssetTypeEnum == ASSET_CHAR)
			{
					WowActor* pActor = gGameWorld->AddNewActor(pos, m_ActorRoot, nSlot) ;				
					if (pActor)
					{	
						ActorController* pActrl = (ActorController*)pActor->GetStateMachine();
					}			
					gShadowReceivers.push_back(pActor->mesh_);				
			}
			else if (AppSettings::AssetTypeEnum == ASSET_DECO)
			{
				WowActor* tree = new WowActor(modelname[5]);
				tree->SetID(g_database.GetNewObjectID());
				tree->SetType(OBJECT_Item);
				GameObjectManager::Get()->AddGameObject(tree);
				tree->SetScale(1.f);		

				tree->SetTranslation(pos);

				m_ActorRoot->AddChild(tree->GetNode());
				gShadowReceivers.push_back(tree->mesh_);
			}	

		}	
	}
	else if (AppSettings::EditModeEnum == EDIT_TEST_MODE)
	{
		
		if (PickUtil::Picking( m_WorldRoot))
		{		
		

			noVec3 target = PickUtil::pickDatas[0].vert;
						
			static bool bTest = false;
		
			if (AppSettings::AssetTypeEnum == ASSET_CHAR)
			{
				WowActor* pActor = gGameWorld->AddNewActor(target, m_ActorRoot, nSlot) ;			
				gShadowReceivers.push_back(pActor->mesh_);				

				if (pActor)
				{	
					ActorController* pActrl = (ActorController*)pActor->GetStateMachine();
				}			
				
			}
			else if (AppSettings::AssetTypeEnum == ASSET_DECO)
			{
				WowActor* tree = new WowActor(modelname[nSlot]);
				tree->SetID(g_database.GetNewObjectID());
				tree->SetType(OBJECT_Item);
				GameObjectManager::Get()->AddGameObject(tree);
				tree->SetScale(1.f);		

				tree->SetTranslation(target);

				m_ActorRoot->AddChild(tree->GetNode());
				gShadowReceivers.push_back(tree->mesh_);
			}	
		}
	}

		//	WowActor* pActor = (WowActor*)g_database.Find(0);
		//	if (pActor)
		//	{	
		//		//pActor->SetScale(5);
		//		ActorController* pActrl = (ActorController*)pActor->GetStateMachine();

		//		//pActrl->PathFinding()->Create(&NaviMesh, pActor->GetTranslation(), 0);
		//	}
		//	//gGameWorld->Hide();
		//	//gGameWorld->TestInterpose();
		//	//gGameWorld->TestOffsetPursuit();
		//
		//}
		//for (size_t i =0; i < m_ActorRoot->childNodes_.size(); ++i)
		//{
		//	noVec3 dir = target - m_ActorRoot->childNodes_[i]->worldTM_.trans_;			
		//	//root->childNodes_[i]->SetDir(dir, SceneNode::TS_WORLD, noVec3(1, 0, 0));											
		//	//noMat3 rot;
		//	//MathHelpers::FaceDirection(rot, dir);
		//	//root->childNodes_[i]->SetRotate(rot);
		//	//root->childNodes_[i]->UpdateTM();			
		//}

		/*WowActor* pActor = (WowActor*)GameObjectManager::Get()->Find(2);
		ActorController* pActrl = (ActorController*)(pActor->GetStateMachine());
		pActrl->RotateHeadingToFacePosition(noVec2(dir.x, dir.z));*/

	/*static int currAnim = 144;	
	
	for (size_t i =0; i < root->childNodes_.size(); ++i)
	{
		M2Mesh* pMesh = (M2Mesh*)root->childNodes_[i];

		if (pMesh->m2_->header.nAnimations <= currAnim)
			currAnim = 0;
		pMesh->m2_->animManager->SetAnim(0, currAnim, 0);
	}	*/
}

void App::AddPath()
{
	if (Pick(0, 0, m_WorldRoot))
	{	
	
		Path* path = PathManager::Get()->GetPath(0);
		path->push_back(PathNode(PickUtil::GetPickVert()));			
	}
}

void App::ExecuteAI()
{
	WowActor* pActor = (WowActor*)GameObjectManager::Get()->Find(0);
	pActor->StartPath();
}

void App::LoadEntity()
{
	TestMath();

	noVec3 corner(-150, 0, -150);
	noVec3 sideA(300, 0, 0);
	noVec3 sideB(0, 0, 300);
		
	//noQuat q;
	//q.FromAngleAxis(noMath::PI* 0.5f , noVec3(0.f, -1.0f, 0.f));		
	//root->SetRotate(q.ToMat3());
		
	//if (AppSettings::EditModeEnum == EDIT_TEST_MODE)
	{	
		gridMesh = makeGrid(grid_rd, corner, sideA, sideB, 10, 10, 1, true, false);	
		gridMesh->m_rd = grid_rd;
		gridMesh->SetName("Ground");
		m_WorldRoot->AddChild(gridMesh);
		gShadowReceivers.push_back(gridMesh);
		gridMesh->m_bOnlyReceiveShadows = true;	

		srand(1);
		float x, y, z;
		noVec3 camPos = ActiveCam_->GetFrom();
		
		float targetOffset = -ONE_METER * 2.f;
		float camHeightOffset = ONE_METER * 10.f;
		float camTargetDist = ONE_METER* 5.0f;
		
		x = camPos.x + randfloat(30, 50);
		y = 0.f;
		z = camPos.z + randfloat(0, 5);
#if 1
		WowActor* pActor = gGameWorld->AddNewActor(noVec3(x, y, z), m_ActorRoot, 0) ;			
		noRotation rot(noVec3(x, y, z), noVec3(0, 1, 0), 180);
		pActor->SetRotation(rot.ToMat3());				
		gShadowReceivers.push_back(pActor->mesh_);				

		//if (dynamic_cast<FollowCamera*>(ActiveCam_))
		{
			FollowCam_->Setup(pActor, targetOffset);
			FollowCam_->SetupFollowCam(camTargetDist,camHeightOffset);
			FollowCam_->SetYawOffset(DEG2RAD(180));
			FollowCam_->SetTarget(pActor);
		}
		
		// Sword1h
		if (pActor->GetNode()->GetChildByName("Weapon1h"))
		{
			pActor->GetNode()->GetChildByName("Weapon1h")->AddChild(EffectSystem::GetSingletonPtr()->Weapon1hTrail_);		
			pActor->GetNode()->GetChildByName("Weapon1h")->AddChild(pActor->meshEdit_->Equip_->RightHandBone_);
			EffectSystem::GetSingletonPtr()->Weapon1hTrail_->Init(NULL, pActor->meshEdit_->Equip_->RightHandBone_, GetSecTime());
			
		}

		if (pActor->GetWeaponKind() == WEAPONK_BOW)
		{
			WowActor* ammo = new WowActor(ammonname[0]);
			ammo->SetID(g_database.GetNewObjectID());
			ammo->SetType(OBJECT_Item);
			GameObjectManager::Get()->AddGameObject(ammo);
			ammo->SetScale(1.f);		
			m_ActorRoot->AddChild(ammo->GetNode());
			
			x = x + randfloat(0, 10);
			z = z;
			y = y + randfloat(0, 10);
			ammo->SetTranslation(noVec3(x, y, z));
			gShadowReceivers.push_back(ammo->mesh_);
		}
#endif
		
		{
			WowActor* ArcheryTarget = new WowActor(modelname[7]);
			ArcheryTarget->SetID(g_database.GetNewObjectID());
			ArcheryTarget->SetType(OBJECT_Item);
			GameObjectManager::Get()->AddGameObject(ArcheryTarget);
			ArcheryTarget->SetScale(1.f);		

			

			x = pActor->GetTranslation().x - 20.f;
			z = pActor->GetTranslation().z;
			y = pActor->GetTranslation().y;
			ArcheryTarget->SetTranslation(noVec3(x, y, z));

			m_ActorRoot->AddChild(ArcheryTarget->GetNode());
			gShadowReceivers.push_back(ArcheryTarget->mesh_);



			/*noVec3 start = pActor->mesh_->m_AABB.GetCenter();
			noVec3 end = ArcheryTarget->mesh_->m_AABB.GetCenter();


			lightning_->Test(start.TofloatPtr(), end.TofloatPtr(), 0x00);*/
			//lightning_->FirePathLightning(start.TofloatPtr(), end.TofloatPtr(), 0x00);
		}
#if 1
		for (int i = 0; i < 1; ++i)
		{	
			WowActor* tree = new WowActor(modelname[5]);
			tree->SetID(g_database.GetNewObjectID());
			tree->SetType(OBJECT_Item);
			GameObjectManager::Get()->AddGameObject(tree);
			tree->SetScale(1.f);		

			x = camPos.x + randfloat(10, 30);
			z = camPos.z + randfloat(10, 30);
			y = GetTerrainHeight(noVec3(x, 2, z));
			tree->SetTranslation(noVec3(x, y, z));

			m_ActorRoot->AddChild(tree->GetNode());
			gShadowReceivers.push_back(tree->mesh_);
		}
#endif

		m_ActorRoot->Update(0.f);
		

		/*Mesh* pMesh = makeMesh(test_rd[0], "Models/house11.lwo");
		pMesh->m_rd = test_rd[0];

		x = camPos.x + randfloat(0, 50) * 5;
		z = camPos.z + randfloat(0, 50) * 5;
		y = GetTerrainHeight(noVec3(x, 0, z));
		pMesh->SetTrans(noVec3(x, y, z));
		testMesh[0] = pMesh;
		m_WorldRoot->AddChild(pMesh);
		gShadowReceivers.push_back(pMesh);

		pMesh = makeMesh(test_rd[1], "Models/house11.lwo");
		pMesh->m_rd = test_rd[1];

		x = camPos.x + randfloat(0, 50) * 10;
		z = camPos.z + randfloat(0, 50) * 10;
		y = GetTerrainHeight(noVec3(x, 0, z));
		pMesh->SetTrans(noVec3(x, y, z));
		testMesh[1] = pMesh;
		gShadowReceivers.push_back(pMesh);
		m_WorldRoot->AddChild(pMesh);*/		
	}
		
	//CreateLightMesh();
	CreateCamera();
	/*GameObject* femaleElf = new WowActor(modelname[0]);
	femaleElf->SetID(g_database.GetNewObjectID());
	femaleElf->SetType(OBJECT_Enemy | OBJECT_Character | OBJECT_Player);
	GameObjectManager::Get()->AddGameObject(femaleElf);
			
	femaleElf->SetScale(1.f);
	femaleElf->SetTranslation(noVec3(0.0, 0.0f, -4.0f));
	
	m_ActorRoot->AddChild(femaleElf->GetNode());*/

	/*GameObject* monster = new WowActor(modelname[1]);
	monster->SetID(g_database.GetNewObjectID());
	monster->SetType(OBJECT_Enemy | OBJECT_Character | OBJECT_Player);
	
	GameObjectManager::Get()->AddGameObject(monster);		

	monster->SetScale(1.f);
	monster->SetTranslation(noVec3(0.0, 0.0f, 4.0f));

	m_ActorRoot->AddChild(monster->GetNode());
	*/

	// set light	
	//gLight.m_Type = U2Light::TYPE_PERSPECTIVE;	


	// set camera
	ActiveCam_->m_fNearMin = ActiveCam_->GetNear();
	ActiveCam_->m_fFarMax =  ActiveCam_->GetFar(); //fMaxLength + 5000.0f;
	
	
	
	//if(!pShadowMap->CreateAsTextureArray(g_iShadowMapSize, 6)) return;
	//if(!pShadowMap->Create(g_iShadowMapSize)) return;;
	
	



}

// Set rendering range for given casters
//
//
static void UpdateSplitRange(const std::vector<Mesh*> &casters, int iSplit)
{
	for(unsigned int i=0; i < casters.size(); i++)
	{
		Mesh *pCaster = casters[i];
		if(iSplit < pCaster->m_iFirstSplit) pCaster->m_iFirstSplit = iSplit;
		if(iSplit > pCaster->m_iLastSplit) pCaster->m_iLastSplit = iSplit;
	}
}

// Starts rendering to shadow maps
//
//
static void ActivateShadowMaps(int iSM)
{
	// unbind shadow map	
	ID3D11ShaderResourceView *pResources[NUM_SPLITS_IN_SHADER] = {NULL};	
	D3D11Context()->PSSetShaderResources(0, NUM_SPLITS_IN_SHADER, pResources);		

	// Enable rendering to shadow map
	GetShadowMap<PSSMShadowMapDx11>(iSM)->EnableRendering();

	// Clear texture
	float ClearSM[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	GetApp()->GetContext()->ClearDepthStencilView(GetShadowMap<PSSMShadowMapDx11>(iSM)->m_pDSV, D3D10_CLEAR_DEPTH, 1.0f, 0);
}

static void DeactivateShadowMaps(int iSM)
{
	// Disable rendering to shadow map
	GetShadowMap<PSSMShadowMapDx11>(iSM)->DisableRendering();
}


static void ActivateShadowMaps(void)
{
	// unbind shadow map	

	// Enable rendering to shadow map
	GetShadowMap<PSSMShadowMapDx11>()->EnableRendering();

	// Clear texture
	float ClearSM[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	GetApp()->GetContext()->ClearDepthStencilView(GetShadowMap<PSSMShadowMapDx11>()->m_pDSV, D3D10_CLEAR_DEPTH, 1.0f, 0);
}

// Stops rendering to shadow maps
//
//
static void DeactivateShadowMaps(void)
{
	// Disable rendering to shadow map
	GetShadowMap<PSSMShadowMapDx11>()->DisableRendering();
}

static void RenderCasters(std::vector<Mesh *> &Objects, const Matrix &mViewProj)
{
	Renderer* renderer = GetApp()->getRenderer();

	// for each object
	std::vector<Mesh *>::iterator it;
	M2Mesh* m2;
	for(it = Objects.begin(); it != Objects.end(); it++)
	{
		Mesh *pObject = (*it);
		pObject->DrawShadow();		
	}
}

static void RenderCasters(std::set<Mesh *> &Objects, const Matrix &mViewProj)
{
	Renderer* renderer = GetApp()->getRenderer();

	// for each object
	std::set<Mesh *>::iterator it;
	M2Mesh* m2;
	for(it = Objects.begin(); it != Objects.end(); it++)
	{
		Mesh *pObject = (*it);
		if (dynamic_cast<M2Mesh*>(pObject))
		{
			pObject->DrawShadow();	
		}
		else 		
			GetApp()->drawShadow(test_rd[0], pObject, test_rd[0].shader);
			
		// keep triangle count accurate
		//g_iTrisPerFrame += pObject->m_pMesh->m_iNumTris * (pObject->m_iLastSplit - pObject->m_iFirstSplit);

		//if(g_iRenderingMethod == METHOD_DX10_INST)
		{
			// activate pass
			//g_pTechniqueShadowMap_Inst->GetPassByIndex(p)->Apply(0, GetApp()->GetContext());
			// draw instanced
			int iNumInstances = pObject->m_iLastSplit - pObject->m_iFirstSplit + 1;
			

			//((Mesh_D3D11 *)pObject->m_pMesh)->DrawInstanced(iNumInstances);
		}
		//else
		//{
		//	// activate pass
		//	g_pTechniqueShadowMap_GSC->GetPassByIndex(p)->Apply(0);
		//	// draw
		//	pObject->m_pMesh->Draw();
		//}

		// reset variables
		pObject->m_iFirstSplit = INT_MAX;
		pObject->m_iLastSplit = INT_MIN;
	}
}

static Matrix GetTexScaleBiasMatrix(void)
{
	// Calculate a matrix to transform points to shadow map texture coordinates
	// (this should be exactly like in your standard shadow map implementation)
	//
	float fTexOffset = 0.5f + (0.5f / (float)g_ShadowMaps[0]->GetSize());

	return Matrix(       0.5f,        0.0f,   0.0f,  0.0f,
		0.0f,       -0.5f,   0.0f,  0.0f,
		0.0f,        0.0f,   1.0f,  0.0f,
		fTexOffset,  fTexOffset,   0.0f,  1.0f);
}

std::vector<mat4> mCropMatrix;
std::vector<mat4> mTextureMatrix;

static std::set<Mesh *> casters;

void App::PSSM_ShadowPass( float fDeltaTime )
{
	gSun.DoControls( fDeltaTime );

	//// Adjust split scheme weight
	////
	if(GetKeyDown(VK_ADD)) g_fSplitSchemeWeight += 0.1f * fDeltaTime;
	else if(GetKeyDown(VK_SUBTRACT)) g_fSplitSchemeWeight -= 0.1f * fDeltaTime;

	// find receivers
	std::vector<Mesh *> receivers;
	//std::vector<Mesh *> casters;


	receivers = ActiveCam_->FindReceivers();

	// adjust camera planes to contain scene tightly
	//ActiveCam_->AdjustPlanes(receivers);

	// calculate the distances of split planes
	ActiveCam_->CalculateSplitPositions(g_fSplitPos);

	// array of texture matrices	
	mTextureMatrix.resize(g_iNumSplits);	
	mCropMatrix.resize(g_iNumSplits);

	//for(int i = 0; i < g_iNumSplits; i++)
	//{
	//	// calculate frustum
	//	Frustum splitFrustum;
	//	splitFrustum = g_ShadowCamera.CalculateFrustum(g_fSplitPos[i], g_fSplitPos[i+1]);
	//	// find casters		
	//	casters = gLight.FindCasters(splitFrustum);
	//	
	//	// calculate crop matrix
	//	Matrix tempCrop= gLight.CalculateCropMatrix(casters, receivers, splitFrustum);
	//	memcpy(&mCropMatrix[i], &tempCrop, SIZE16);
	//	// calculate texture matrix
	//	Matrix tempTexture = gLight.m_mView * gLight.m_mProj * tempCrop;
	//	memcpy(&mTextureMatrix[i], &tempTexture, SIZE16); 

	//	// render shadow map
	//	ActivateShadowMaps(i);
	//	RenderCasters(casters, tempTexture);
	//	DeactivateShadowMaps(i);
	//}


	// for each split
	for(int i = 0; i < g_iNumSplits; i++)
	{
		// calculate frustum
		Frustum splitFrustum;
		splitFrustum = ActiveCam_->CalculateFrustum(g_fSplitPos[i], g_fSplitPos[i+1]);
		// find casters
		std::vector<Mesh*> castersInSplit;
		castersInSplit = gSun.FindCasters(splitFrustum);
		UpdateSplitRange(castersInSplit, i);
		casters.insert(castersInSplit.begin(), castersInSplit.end());

		// calculate crop matrix
		Matrix tempCrop= gSun.CalculateCropMatrix(castersInSplit, receivers, splitFrustum);		 

		// calculate texture matrix
		Matrix tempTexture = gSun.m_mView * gSun.m_mProj * tempCrop * GetTexScaleBiasMatrix();
		//tempCrop = Transpose(tempCrop);
		//tempTexture = Transpose(tempTexture);
		memcpy(&mCropMatrix[i], &tempCrop, SIZE16);
		memcpy(&mTextureMatrix[i], &tempTexture, SIZE16); 
		GetShadowMap<PSSMShadowMapDx11>()->mPssmConstants.TextureMatrixArray[i] = mTextureMatrix[i];
		GetShadowMap<PSSMShadowMapDx11>()->mPssmConstants.SplitPlane[i] = g_fSplitPos[i+1];
	}

	/*if (terrainload_)
	{	
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		DXCall(D3D11Context()->Map(GetShadowMap<PSSMShadowMapDx11>()->m_pD3DBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource));
		CopyMemory(mappedResource.pData, &GetShadowMap<PSSMShadowMapDx11>()->mPssmConstants, GetShadowMap<PSSMShadowMapDx11>()->m_ibufferSize);
		D3D11Context()->Unmap(GetShadowMap<PSSMShadowMapDx11>()->m_pD3DBuffer, 0);
	}*/

	// render shadow map
	if (!casters.empty())
	{	
		ActivateShadowMaps();
		RenderCasters(casters, gSun.m_mView * gSun.m_mProj);
		DeactivateShadowMaps();
	}
	
	
}

void App::RenderMeshes(bool zOnly) {

	noFrustum f;

	noMat4 view(ActiveCam_->getViewMatrix());
	noMat4 proj(ActiveCam_->getProjectionMatrix());
	f.loadFrustum(ToMat4(view) * ToMat4(proj));

	for (size_t i =0; i < m_ActorRoot->childNodes_.size(); ++i)
	{
		M2Mesh* pMesh = (M2Mesh*)m_ActorRoot->childNodes_[i];	
		//if (f.cubeInFrustum(pMesh->m_AABB.m_vMin.x, pMesh->m_AABB.m_vMax.x, pMesh->m_AABB.m_vMin.y, pMesh->m_AABB.m_vMax.y, pMesh->m_AABB.m_vMin.z, pMesh->m_AABB.m_vMax.z))
		{
			if (zOnly) pMesh->DrawZOnly();
			else pMesh->Draw();	
		}
	}
			

	for (size_t i =0; i < m_WorldRoot->childNodes_.size(); ++i) {
		CDXUTSDKMesh* mesh  = dynamic_cast<CDXUTSDKMesh*>(m_WorldRoot->childNodes_[i]);
		if (mesh) {

			renderer->reset();
			renderer->setShader(lighting[0]);
			renderer->setVertexFormat(lit_vf);

			mesh->worldTM_.trans_ = gSun.m_vLitPos;
			noMat4 tm(mesh->worldTM_.rotate_ * mesh->worldTM_.scale_, mesh->worldTM_.trans_);
			tm.TransposeSelf();	
			mat4 world = ToMat4(tm);			
			renderer->setShaderConstant4x4f("World", world);
			renderer->setShaderConstant4x4f("View", ViewMat());
			renderer->setShaderConstant4x4f("View", ViewMat());
			float4 MatDiffuse = float4(0.5f, 0.5, 0.f, 0.5f);
			renderer->setShaderConstant4f("MatDiffuse", MatDiffuse);
			renderer->setShaderConstant4x4f("Projection", ProjMat());	
			renderer->apply();
			
			mesh->Render(D3D11Context());
		}
			
	}

}

void App::RenderPSSMScenes( float fDeltaTime )
{	
	/*if (terrainload_)
	{	
		float x, y, z;
		for (size_t i = 0; i < gShadowReceivers.size(); ++i)
		{	
			noVec3 pos = gShadowReceivers.at(i)->WorldTrans();
			x = pos.x;
			z = pos.z;
			y = GetTerrainHeight(noVec3(x, 500, z), 10);
			gShadowReceivers.at(i)->SetTrans(noVec3(x, y, z));
			gShadowReceivers.at(i)->Update(0.f);
		}
	}*/

	// Global : 0	

	//drawModel(grid_rd);
	if (grid_rd.shader != -1)
	{

		if (AppSettings::ShadowModeEnum == SHADOW_SSAO)					
			drawSSAO(grid_rd, gridMesh, gColorSsao);
		else 		
			drawShadowed(grid_rd, gridMesh, gColorPssm);
	}	
		
	RenderMeshes(false);
		/*noMat4 tm(gridMesh->worldTM_.rotate_ * gridMesh->worldTM_.scale_, gridMesh->worldTM_.trans_);
		tm.TransposeSelf();	
		mat4 world = ToMat4(tm);
		drawModel(grid_rd, world, ViewMat(), ProjMat());*/
		//drawShadowed(test_rd[0], testMesh[0], test_rd[0].shader);
		//drawShadowed(test_rd[1], testMesh[1], test_rd[1].shader);	
	
	//mat4 world = translate(gLight.m_vLitPos.x, gLight.m_vLitPos.y, gLight.m_vLitPos.z);
	/*renderer->setShaderConstant4x4f("World", world);
	renderer->setShaderConstant4x4f("View", g_View);
	renderer->setShaderConstant4x4f("Projection", g_Projection);	
	sphere->draw(renderer);*/
	
	//noMat4 world(pShadowCam_->getViewMatrix());
	//world.TransposeSelf();		
	//drawModel(camera.rd, ToMat4(world), g_View, g_Projection );
		
	mCropMatrix.clear();
	mTextureMatrix.clear();	
}

void App::DrawTextShadowDebug()
{
	text_->Begin();
	text_->SetInsertionPos(10, 30);
	text_->DrawFormattedTextLine("Num Caster : %d  Light Pos : %f, %f, %f Light Target : %f. %f. %f", casters.size(), 
		gSun.m_vLitPos.x, gSun.m_vLitPos.y, gSun.m_vLitPos.z, gSun.m_vTarget.x, gSun.m_vTarget.y, gSun.m_vTarget.z);
	text_->End();

}

bool App::onKeyAI( const uint key, const bool pressed )
{
	return true;
}


//idAnimManager		animationLib;