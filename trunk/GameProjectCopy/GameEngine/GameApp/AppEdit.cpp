#include "stdafx.h"
#include <GameApp/GameApp.h>
#include "GameApp/Mesh.h"
#include "GameApp/SceneNode.h"
#include "GameApp/M2Mesh.h"
#include "GameApp/TestObjects.h"
#include "GameApp/GameObjectManager.h"
#include "GameApp/AI/GameWorld.h"
#include "GameApp/CurveEditor.h"
#include "GameCamera.h"
#include "Projectile.h"

#include ".\BaseTypes\BaseTypes.h"
#include ".\Interfaces\Interface.h"

#include "Trees/Tree.h"
#include "ProceduralTrees/TreeObject.h"

static const char* modelname[] = {
	"Character\\BloodElf\\female\\BloodElfFemale.m2",
	"Creature\\AbyssalOutland\\Abyssal_Outland.m2",
	"Creature\\alexstrasza\\alexstrasza.m2",
	"Creature\\arthas\\arthas.m2",
	"Creature\\FireSpirits\\firespiritsmall.m2"
};


static const char* ammonname[] = {
	"Item/ObjectComponents/Ammo/arrowfireflight_01.m2"
};
static noVec3 ammo_origPos;
static WowActor* pActor;

static Mesh* editGrid;
static RenderData grid;
static CurveEditor edit;
static WowActor* ammo;

void App::LoadEditObjs() {
	
	noVec3 corner(-50, 0, -50);
	noVec3 sideA(100, 0, 0);
	noVec3 sideB(0, 0, 100);

	editGrid = makeEditGrid(grid, corner, sideA, sideB, 100, 100, 1, true, false);
	m_WorldRoot->AddChild(editGrid);
	grid.cull = gRenderer->addRasterizerState(CULL_BACK, WIREFRAME);

	noVec3 eye = noVec3(0, 10, 10);
	noVec3 to(vec3_zero);
	noVec3 up(0, 1, 0);

	DefCam_->setFrom(eye.ToFloatPtr());
	DefCam_->setTo(to.ToFloatPtr());
	DefCam_->setUp(up.ToFloatPtr());
	DefCam_->computeModelView();

	FpsCam_->setFrom(eye.ToFloatPtr());
	FpsCam_->setTo(to.ToFloatPtr());
	FpsCam_->setUp(up.ToFloatPtr());
	//FpsCam_->computeModelView();

	//ActiveCam_ = FpsCam_;	
	ActiveCam_ == DefCam_;

	float x, y, z;
	noVec3 camPos = ActiveCam_->GetFrom();

	float targetOffset = -ONE_METER * 2.f;
	float camHeightOffset = ONE_METER * 10.f;
	float camTargetDist = ONE_METER* 5.0f;
		
	pActor = gGameWorld->AddNewActor(vec3_zero, m_ActorRoot, 0) ;			
	//noRotation rot(vec3_zero, noVec3(0, 1, 0), 180);
	//pActor->SetRotation(rot.ToMat3());
	edit.Init();

	ammo = new WowActor(ammonname[0]);
	ammo->SetID(g_database.GetNewObjectID());
	ammo->SetType(OBJECT_Item);
	GameObjectManager::Get()->AddGameObject(ammo);
	ammo->SetScale(1.f);		
	ammo->AttachAttribute(new ProjectileAttributes);

	ProjectileComponent* pComp = new ProjectileComponent(ammo);
	pComp->currentShotType = ProjectileComponent::LASER;
	ammo->AttachComponent(pComp);
	m_ActorRoot->AddChild(ammo->GetNode());
	
	noVec3 forward = pActor->GetRotation().ToAngles().ToForward();		
	ammo_origPos = pActor->GetTranslation() + forward * 0.5f;
	//ammo->SetTranslation(ammo_origPos);

	pActor->AttachGameObject(ammo, forward * 0.5f);

	noRotation rot(vec3_zero, noVec3(0, 1, 0), 180);
	ammo->SetRotation(rot.ToMat3());		
		
	
	//gShadowReceivers.push_back(ammo->mesh_);
}

void App::RenderEdit( float fDeltaTime, float fTime )
{
	if (GetKeyDown('F')) {
		ammo->SetTranslation(ammo_origPos);
		noVec3 forward = pActor->GetRotation().ToAngles().ToForward();	
		LaunchProjectile(pActor, ammo, ammo_origPos, mat3_identity, vec3_zero);
	}

	renderer->resetToDefaults();

	noMat4 tm(editGrid->worldTM_.rotate_ * editGrid->worldTM_.scale_, editGrid->worldTM_.trans_);
	tm.TransposeSelf();	
	mat4 world = ToMat4(tm);
	drawModel(grid, world, ViewMat(), ProjMat());	

	for (size_t i =0; i < m_ActorRoot->childNodes_.size(); ++i)
	{
		M2Mesh* pMesh = (M2Mesh*)m_ActorRoot->childNodes_[i];	
		pMesh->Draw();	
	}

	edit.Update(fDeltaTime);
	edit.Draw(fDeltaTime);
	
	//drawShadowed(grid, editGrid, gColorPssm);	

}