dict
dict
FreeData
#include "stdafx.h"

#include "GameApp.h"
#include "CascadedShadowsManager.h"
#include "GameApp/Mesh.h"
#include "GameApp/SceneNode.h"
#include "GameApp/M2Mesh.h"
#include "GameApp/Mesh_D3D11.h"
#include "GameApp/TestObjects.h"
#include "GameApp/GameObjectManager.h"
#include "GameApp/GameObjectUtil.h"
#include "GameApp/Common.h"
#include "ModelLib/M2Loader.h"
#include "GameApp/Actor.h"
#include "GameApp/Font/Dx11TextHelper.h"
#include "SDKMeshLoader.h"

#include "U2Light.h"
#include <GameApp/MD5/MD5Material.h>
#include <GameApp/MD5/MD5MapFile.h>
#include <GameApp/Collision/CollisionModel.h>
#include <GameApp/Collision/CollisionModel_local.h>
#include "GameApp/Physics/Physics.h"
#include "GameApp/Physics/Force.h"
#include "GameApp/Physics/Clip.h"
#include "GameApp/MD5/MD5Model.h"
#include "GameApp/Animation/MD5Anim.h"
#include "GameApp/Animation/MD5IK.h"
#include <GameApp/MD5/MD5Game.h>
#include "GameApp/Physics/Physics_Base.h"
#include "GameApp/Physics/Physics_Actor.h"
#include <GameApp/Physics/Physics_AF.h>
#include <GameApp/MD5/DeclAF.h>
#include <GameApp/MD5/MD5Af.h>
#include "GameApp/MD5/MD5Entity.h"
#include "GameApp/MD5/MD5AFEntity.h"
#include "GameApp/MD5/MD5Actor.h"
#include "GameApp/MD5/MD5AI.h"
#include "GameApp/MD5/MD5AI_define.h"
//idDeclManager *				declManager = NULL;

idAnimManager	animationLib;


static idRenderModelMD5 model;
static idAnimator	animator;

idAI* TestModel(const idStr& name)
{
	noVec3			offset;
	
	idActor *		player;
	const idDict *	entityDef;
	idDict			dict;

	entityDef = gameLocal.FindEntityDefDict( name, false );
	if ( entityDef ) {
		dict = *entityDef;
	} else {
		if ( declManager->FindType( DECL_MODELDEF, name, false ) ) {
			dict.Set( "model", name );
		} else {
			//// allow map models with underscore prefixes to be tested during development
			//// without appending an ase						

			//if ( !renderModelManager->CheckModel( name ) ) {
			//	gameLocal.Printf( "Can't register model\n" );
			//	return;
			//}
			//dict.Set( "model", name );
		}
	}
	//offset = player->GetPhysics()->GetOrigin() + player->viewAngles.ToForward() * 100.0f;
	//dict.Set( "origin", offset.ToString() );
	//dict.Set( "angle", va( "%f", player->viewAngles.yaw + 180.0f ) );
	return ( idAI * )gameLocal.SpawnEntityType( idAI::Type, &dict );
}

class Monster : public GameObject
{
public:
	Monster();
	virtual ~Monster();

	idAI*			self;

	float			sightEnemyTime;
	bool			in_melee;
	bool rightfoot;
	bool charge;


	float			pain_time;
	float			inactive_time;
	float			active_time;
	

	
	void 			destory();
	void 			addSeeker( idEntity* seeker );
	void 			sightEnemy();
	bool 		seekersAreDead();
		

	void 			Legs_Sight();
	void 			Legs_Death();
	void 			Legs_Pain();
	void			Legs_RangeAttack();
	void 			Legs_MeleeAttack();
	void 			Legs_Walk_Melee_Left();
	void 			Legs_Walk_Melee_Right();
	void 			Legs_Walk();
	void 			Legs_Run_Melee_Left();
	void 			Legs_Run_Melee_Right();
	void 			Legs_Charge();
	void 			Legs_Lost();
	void 			Legs_Idle();
	void 			Legs_SpawnSeeker();

	void 			state_Idle();
	void 			state_Killed();
	void 			state_Combat();
	void 			combat_melee();
	void			combat_range();
	void			combat_charge();

	virtual void Initialize();
	virtual void UpdateInternal(float fDelta);	

	bool TickIdle();
	bool TickWalk();
	bool TickDeath();
	bool TickPlayAnim();
};

DEFINE_STATE(STATE_IDLE, Monster, TickIdle, "idle");
DEFINE_STATE(STATE_WALK, Monster, TickWalk, "walk");
DEFINE_STATE(STATE_DEATH, Monster, TickDeath, "death");
DEFINE_STATE(STATE_PLAYANIM, Monster, TickPlayAnim, "playanim");
Monster::Monster() : GameObject("")
{
	
}


void Monster::Initialize() {
	active_time		= 0;
	inactive_time	= 0;
	pain_time		= 0;
	sightEnemyTime = -1;
	self = TestModel("monster_boss_guardian");
	
	rightfoot = true;
	self->SetAnimState(ANIMCHANNEL_LEGS, "Legs_Idle", 0 );
}

void Monster::UpdateInternal( float fDelta )
{
	self->Think();
}

void Monster::destory()
{

}

void Monster::addSeeker( idEntity* seeker )
{

}

void Monster::sightEnemy() {

}

bool Monster::seekersAreDead() {
	return false;
}

void Monster::Legs_Sight() {
	rightfoot = true;
	self->Event_PlayAnim(ANIMCHANNEL_LEGS, "sight");

}

void Monster::Legs_Death() {
	
	self->Event_PlayAnim(ANIMCHANNEL_LEGS, "death");

}

void Monster::Legs_Pain() {
	float nextpain;
	float currenttime;

	rightfoot = true;
	self->Event_PlayAnim( ANIMCHANNEL_LEGS, "pain" );
	nextpain = gameLocal.time + GUARDIAN_PAIN_DELAY;
}

void Monster::Legs_RangeAttack() {
	rightfoot = true;
	self->Event_PlayAnim( ANIMCHANNEL_LEGS, "range_attack" );
}

void Monster::Legs_MeleeAttack() {
	rightfoot = true;
	self->Event_PlayAnim( ANIMCHANNEL_LEGS, "melee_attack" );

}

void Monster::Legs_Walk_Melee_Left()
{

}

void Monster::Legs_Walk_Melee_Right()
{

}

void Monster::Legs_Walk() {
	if (rightfoot) {
		self->Event_PlayAnim( ANIMCHANNEL_LEGS, "walk_right" );		
	} else {
		self->Event_PlayAnim( ANIMCHANNEL_LEGS, "walk_left" );		
	}

	TransitionState(&STATE_WALK);
}

void Monster::Legs_Run_Melee_Left()
{

}

void Monster::Legs_Run_Melee_Right()
{

}

void Monster::Legs_Charge()
{

}

void Monster::Legs_Lost()
{

}

void Monster::Legs_Idle() {
	self->Event_IdleAnim(ANIMCHANNEL_LEGS, "Idle");
}

void Monster::Legs_SpawnSeeker()
{

}

void Monster::state_Idle() {
	//sightEnemyTime
	self->SetAnimState(ANIMCHANNEL_LEGS,  "Legs_Sight", GUARDIAN_IDLE_TO_SIGHT);
	self->SetWaitState("sight");

	TransitionState(&STATE_IDLE);	
}

void Monster::state_Killed() {
	self->AI_DEAD = true;
	self->Event_StopMove();
	self->SetAnimState(ANIMCHANNEL_LEGS, "Legs_Death", 0 );
	self->SetWaitState( "dead" );
	self->Event_StopThinking();	
	Legs_Death();

	TransitionState(&STATE_DEATH);
}

void Monster::state_Combat()
{

}

void Monster::combat_melee() {
	self->Event_StopMove();
	self->SetAnimState(ANIMCHANNEL_LEGS, "Legs_MeleeAttack", GUARDIAN_IDLE_TO_MELEE );
	Legs_MeleeAttack();
}

void Monster::combat_range() {
	self->FaceEnemy();
	self->Event_StopMove();
	self->SetAnimState( ANIMCHANNEL_LEGS, "Legs_RangeAttack", GUARDIAN_IDLE_TO_RANGE );
	Legs_RangeAttack();
}

void Monster::combat_charge()
{

}

bool Monster::TickIdle() {
	if (self->AI_PAIN) {
		self->SetAnimState( ANIMCHANNEL_LEGS, "Legs_Pain", GUARDIAN_WALK_TO_PAIN );
	}
	if (self->AI_FORWARD) {
		self->SetAnimState( ANIMCHANNEL_LEGS, "Legs_Walk", GUARDIAN_IDLE_TO_WALK );
	}
	if (self->GetEnemy()) {

	}	
	return true;
}

bool Monster::TickWalk() {
	if (self->AI_PAIN) {
		self->SetAnimState( ANIMCHANNEL_LEGS, "Legs_Pain", GUARDIAN_WALK_TO_PAIN );
	}
	if (!self->AI_FORWARD) {
		self->SetAnimState( ANIMCHANNEL_LEGS, "Legs_Idle", GUARDIAN_WALK_TO_IDLE );
	}

	if (self->Event_AnimDone(ANIMCHANNEL_LEGS, 0)) {
		/*if ( charge ) {
			animState( ANIMCHANNEL_LEGS, "Legs_Charge", GUARDIAN_WALK_TO_CHARGE );
		}*/
		rightfoot = !rightfoot;
		if (rightfoot) {
			self->Event_PlayAnim( ANIMCHANNEL_LEGS, "walk_right" );
		} else {
			self->Event_PlayAnim( ANIMCHANNEL_LEGS, "walk_left" );
		}
	}
	if (GetKeyDown('F'))
	{		
		//state_Killed();
		//self->Event_RealKill();
		//TransitionState(&STATE_DEATH);
		TransitionState(&STATE_PLAYANIM);
	}

	if (GetKeyDown('D'))
	{		
		//state_Killed();
		static int i =0;
		if (i & 2)
			self->Event_EnableWalkIK();
		else 
			self->Event_DisableWalkIK();
		i++;		
	}

	return true;
}

Monster::~Monster()
{

}

bool Monster::TickDeath()
{
	/*if (GetKeyDown('F'))
	{
		Legs_Walk();
		
	}*/
	return true;
}

bool Monster::TickPlayAnim()
{
	if (self->Event_AnimDone(ANIMCHANNEL_LEGS, GUARDIAN_RANGEATTACK_TO_IDLE))
		Legs_Idle();


	if (GetKeyDown('F')) {
	
		combat_range();
	}

	if (GetKeyDown('D'))
		combat_melee();


	return true;
}



void App::LoadMD5()
{
	// init console command system
	cmdSystem->Init();
	// init CVar system
	cvarSystem->Init();

	// register all static CVars
	idCVar::RegisterStaticVars();

	gameLocal.Clear();
	declManager->Init();

	declManager->RegisterDeclType( "model",				DECL_MODELDEF,		idDeclAllocator<idDeclModelDef> );

	declManager->LoadAndFindDecl("doom3/materials/base_door.mtr", DECL_MATERIAL);	
	declManager->LoadAndFindDecl("doom3/materials/base_floor.mtr", DECL_MATERIAL);	
	declManager->LoadAndFindDecl("doom3/materials/base_trim.mtr", DECL_MATERIAL);	
	declManager->LoadAndFindDecl("doom3/materials/base_light.mtr", DECL_MATERIAL);	

	declManager->LoadAndFindDecl("doom3/Def/misc.def", DECL_ENTITYDEF);	

	//tr.InitFromMap("doom3/maps/cpuboss");
	gameLocal.LoadMap("doom3/maps/cpuboss", 0);
	gameLocal.MapPopulate();

		
	//declManager->LoadAndFindDecl("Models/Def/monster_boss_guardian.def", DECL_MODELDEF);	
	declManager->LoadAndFindDecl("doom3/materials/monster_guardian.mtr", DECL_MATERIAL);	

	declManager->LoadAndFindDecl("doom3/Def/monster_default.def", DECL_ENTITYDEF);	
	declManager->LoadAndFindDecl("doom3/Def/monster_boss_guardian.def", DECL_ENTITYDEF);	
	declManager->LoadAndFindDecl("doom3/af/monster_boss_guardian.af", DECL_AF);	
		
	idAI*pActor = NULL;
	const idDict* entityDef = gameLocal.FindEntityDefDict("monster_boss_guardian", false );	

	//gameLocal.SpawnEntityDef(*entityDef, &pEntity, false);
			
	//pActor = TestModel("monster_boss_guardian");
	Monster* mon = new Monster;
	g_database.AddGameObject(mon);
	mon->Legs_Walk();

	//int				animNum = 1;	// 1ÀÌ»ף
	//animNum = pActor->GetAnimator()->GetAnim("lost_enemy");
	//pActor->GetAnimator()->CycleAnim(ANIMCHANNEL_ALL, animNum, gameLocal.time, FRAME2MS(50));
	//pActor->GetAnimator()->RemoveOriginOffset( true );
		
	float up[] = {0, 0, 1};
	ActiveCam_->setUp(up);
	ActiveCam_->computeModelView();		
}





