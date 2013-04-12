#include "stdafx.h"
#include <ModelLib/mpq_libmpq.h>
#include <ModelLib/Model_M2.h>
#include <ModelLib/M2Loader.h>
#include "M2Object.h"
#include "M2Edit.h"
#include "EngineCore/Util/GameLog.h"

#include "AI/PathManager.h"

#include "SceneNode.h"
#include "Mesh.h"
#include "M2Mesh.h"

#include "GameObjectUtil.h"
#include "GameApp.h"
#include "FSM.h"
#include "AnimationComponent.h"
#include "AnimationSystem.h"
#include "TestObjects.h"

#include "ActorInput.h"
#include "Common.h"

noImplementRTTI(WowActor,Actor);
DEFINE_STATE(STATE_STARTPATH,WowActor,TickStartPath,"startpath");
DEFINE_SIMPLE_STATE(STATE_FOLLOWPATH,WowActor,"followpath");
DEFINE_SIMPLE_STATE(STATE_PLAYERCONTROL,WowActor,"PlayerControl");
WowActor::WowActor( const std::string& filename )
	:Actor(filename),m_pkAttrib(0), input_(NULL)
{
	
}

void WowActor::LoadModel()
{
	mesh_ = new M2Mesh;
	M2Loader* loader = new M2Object(gRenderer, this); 
	mesh_->m2_ = loader;
	loader->SetViewMatrix(ViewMat());
	mesh_->owner_ = this;
	
	m_spNode = mesh_;
		
	wxString fn(m_filename.c_str());

	bool isChar = (fn.Lower().Find(wxT("char")) || fn.Lower().Find(wxT("alternate\\char")));
		
	loader->Load(fn.wx_str(), true);	
	if (!isChar)
		loader->modelType = MT_NORMAL;


	/*LOG_INFO << " Bone Start";

	for (size_t i=0; i< loader->header.nBones; i++) {		
		if (loader->bones[i].parent != -1)
		{		
					
			LOG_INFO << " Parent Bone Id : " << loader->bones[i].parent <<  "  Bone Id" << i ;
			
		}
	}
	LOG_INFO << " Bone End";*/


	meshEdit_ = new M2Edit;
	meshEdit_->Init(gRenderer, mesh_, this);
	
	CreateRenderModel(loader);
	
	if (!GetStateMachine())
	{
		AnimationSystem* animSys = (AnimationSystem*)mesh_->m2_->animManager;		
		AnimationSystem::InitStand(mesh_->m2_, true);
	}
	
	mesh_->CreateBuffers();	
}

void WowActor::UpdateModelInfo(M2Loader* m2)
{	
	M2Edit& m2edit = *meshEdit_;
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

void WowActor::CreateRenderModel(M2Loader* m2)
{
	M2Edit& m2edit = *meshEdit_;

	wxString fn(m2->name.c_str());
	fn = fn.Lower();
	if (fn.substr(0, 4) == wxT("char")) {

		UpdateModelInfo(m2);


		{	
			std::ifstream f("Data/elf3.chr");/*("Data/abyssal_outland.chr");*/ 

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
		meshEdit_->cachedCombosedTex = TexPath("Skins\\Character\\BloodElf\\Female\\Warrior_Body2.tga");


		m2->replaceTextures[TEXTURE_BUMP] = GetRenderer()->addNormalMap("Textures/Skins/Character/BloodElf/Female/Warrior_Body_SSBump.tga", 
			FORMAT_RGBA8, true);

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

	m2->SetViewMatrix(ViewMat());
}

void WowActor::CreateTransformComponent()
{
	ITransformComponent	*pkComponent = NULL;		
	pkComponent = new ActorTransformComponent(this);
	if( pkComponent )
		SetTransformComponent(pkComponent);
}

void WowActor::Initialize()
{
	LoadModel();

	CreateTransformComponent();

	GameObjectRotatorAttribute      *pkRotateAttrib = new GameObjectRotatorAttribute;
	pkRotateAttrib->m_yawRate                       = DEG2RAD(360);	
	AttachAttribute( pkRotateAttrib );			
	if (mesh_->m2_->IsSKinned())
	{
		

		m_pkBrain = new WowBrain(this);

		m_pkAttrib = new WowAnimStateAttrib();
		AttachAttribute(m_pkAttrib);
		m_pkAttrib->Setup((M2Object*)mesh_->m2_);
		m_pkAttrib->Idle();		

		BasicCharacterMovementAttribute     *pkMoveAttrib = new BasicCharacterMovementAttribute(1.3f*1.7f,3.5f*1.7f);
		pkMoveAttrib->Walk();   // default to walk speed...
		AttachAttribute(pkMoveAttrib);		
	}

	GameObject::Initialize();
}

void WowActor::update( float elapsedTime )
{	
	Actor::Update(elapsedTime);
}

void WowActor::UpdateInternal( float fDelta )
{
	if (input_)
		input_->HandleInput(fDelta);

	if( GetStateMachine() ) {
		ActorController* pActrl = (ActorController*)GetStateMachine();
		pActrl->update( fDelta );
	}
}

void WowActor::StartPath()
{
	Path	*pkPath		= PathManager::Get()->GetPath(0);

	m_pkBrain->WalkTo(PickUtil::GetPickVert());
	TransitionState(&STATE_STARTPATH);

}

void WowActor::FollowPath()
{
	Path	*pkPath		= PathManager::Get()->GetPath(0);
	noVec3 pos = GetTranslation();
	pkPath->SetHeight(MathHelpers::UP_COMPONENT(pos));
	pkPath->SetPauseChance(4,100);
	pkPath->SetPauseChance(10, 100);

	m_pkBrain->FollowPath(pkPath);
}

void WowActor::GoAI()
{

}

void WowActor::GoPlayer()
{

}

void WowActor::ResetPosition()
{

}

void WowActor::Stop()
{
	m_pkBrain->FollowPathStop();
	m_pkBrain->StopMoveTo();
}

bool WowActor::TickStartPath()
{
	if( m_pkBrain->IsMoveToDone() )
	{
		FollowPath();
		TransitionState(&STATE_FOLLOWPATH);
	}
	return true;
}

void WowActor::AttachComponents()
{
	//AttachComponent( m_pkBrain );
	//AttachComponent( new CharacterMoverComponent(this) );

	CharacterRotatorComponent  *pkRotator = new CharacterRotatorComponent(this);
	AttachComponent(pkRotator);
	if (mesh_->m2_->IsSKinned())
	{
		AnimationComponent  *pkAnimComponent = new AnimationComponent(this);
		AttachComponent( pkAnimComponent );
		AnimStateComponent *pkAnim = new AnimStateComponent(this);
		AttachComponent( pkAnim );

	}

	//FloorMoverComponent     *pkFloor = new FloorMoverComponent(this);
	//pkFloor->Setup(5,8,PHOENIX_STEP_HEIGHT,0);
	//AttachComponent(pkFloor);
}

void WowActor::AttachInput( class ActorInput* input )
{
	input_ = input;
}


WowBrain::WowBrain( GameObject* pkGameObject ) : AIBrain(pkGameObject)
{
	m_pkFollowPath = new FollowPathBehavior(this);
	m_pkMoveToBehavior = new MoveToBehavior(this);	
}


bool WowBrain::Initialize()
{
	AIBrain::Initialize();
	AddBehavior( new IdleBehavior(this) );
	AddBehavior( m_pkMoveToBehavior );
	AddBehavior( m_pkFollowPath );
	return true;
}

void WowBrain::FollowPath( class Path *pkPath )
{
	m_pkFollowPath->SetPath(pkPath);
}

void WowBrain::FollowPathStop()
{
	m_pkFollowPath->SetPath(NULL);

	CharacterMoverComponent    *pkMover = GetGameObjectComponent(GetGameObject(),CharacterMoverComponent);
	if( pkMover )
		pkMover->Stop();

	BasicCharacterAnimStateAttribute *pkAttrib = GetGameObjectAttribute(GetGameObject(),BasicCharacterAnimStateAttribute);
	if( pkAttrib )
		pkAttrib->Idle();
}

void WowBrain::RunTo( const noVec3 &pos )
{
	m_pkMoveToBehavior->RunTo(pos);
}

void WowBrain::WalkTo( const noVec3 &pos )
{
	m_pkMoveToBehavior->WalkTo(pos);
}

void WowBrain::StopMoveTo()
{
	m_pkMoveToBehavior->Stop();

}

bool WowBrain::IsMoveToDone()
{
	if( GetCurrentBehavior()==m_pkMoveToBehavior )
		return false;

	return true;
}


noImplementRTTI(WowAnimStateAttrib,BasicCharacterAnimStateAttribute);
void WowAnimStateAttrib::Setup( M2Object* pOwner )
{
	AnimationSystem* pAnimSys = (AnimationSystem*)pOwner->animManager;

	//pAnimSys->DebugIds();
	
	m_pkIdle = &m_idle;
	m_pkWalk = &m_walk;
	m_pkWalkBack = &m_walkback;
	m_pkRun  = &m_run;
	m_pkDeath = &m_death;
	m_pkKick = &m_kick;

	m_kick.Setup(pAnimSys->FindSeqId("Kick"));
		
	int i;
	m_idle.AddAnimation(pAnimSys->FindSeqIdAndBaseIndex("Stand",i), 30);	// stand[3]
	//m_idle.AddAnimation(pAnimSys->FindSeqId("Stand", i+38), 30);	// stand[41]
	//m_idle.AddAnimation(pAnimSys->FindSeqId("Stand", i+142), 30);	// stand[145]

	m_walk.Setup(pAnimSys->FindSeqId("Walk"));
	m_walkback.Setup(pAnimSys->FindSeqId("WalkBackwards"));
	m_run.Setup(pAnimSys->FindSeqId("Run"));
	m_death.Setup(pAnimSys->FindSeqId("Death"));

	pAnimSys->FindSeqIdAndBaseIndex("JumpStart", i);
	m_jumpStart.Setup(pAnimSys->FindSeqId("JumpStart",i+1));	// JumpStart 113(È¸Àü), 114
	m_jump.Setup(pAnimSys->FindSeqId("Jump"));	// Jump
	m_jumpEnd.Setup(pAnimSys->FindSeqId("JumpEnd"));	// JumpEnd

	m_attack1h.AddAnimation(pAnimSys->FindSeqIdAndBaseIndex("Attack1H", i));
	m_attack1h.AddAnimation(pAnimSys->FindSeqId("Attack1H", i+1));
	m_attack1h.AddAnimation(pAnimSys->FindSeqId("Attack1HPierce"));

	m_attack2h.AddAnimation(pAnimSys->FindSeqIdAndBaseIndex("Attack2H",i));
	m_attack2h.AddAnimation(pAnimSys->FindSeqId("Attack2H",i+1));
	m_attack2h.AddAnimation(pAnimSys->FindSeqIdAndBaseIndex("Attack2HL", i));
	m_attack2h.AddAnimation(pAnimSys->FindSeqId("Attack2H", i+1));
	m_attack2h.AddAnimation(pAnimSys->FindSeqId("Attack2H", i+3));
	m_attack2h.AddAnimation(pAnimSys->FindSeqId("Attack2HLoosePierce"));

	m_attackBow.Setup(pAnimSys->FindSeqId("AttackBow"));
	m_attackRifle.Setup(pAnimSys->FindSeqId("AttackRifle"));
	

	m_attackUnarmed.AddAnimation(pAnimSys->FindSeqIdAndBaseIndex("AttackUnarmed", i));
	m_attackUnarmed.AddAnimation(pAnimSys->FindSeqId("AttackUnarmed", i+2));

	m_standWound.Setup(pAnimSys->FindSeqId("StandWound"));
	m_combatCritical.Setup(pAnimSys->FindSeqId("CombatCritical"));
	m_combatWound.Setup(pAnimSys->FindSeqId("CombatWound"));
	m_dodge.Setup(pAnimSys->FindSeqId("Dodge"));

	m_loadBow.Setup(pAnimSys->FindSeqId("LoadBow"));
	m_loadRifle.Setup(pAnimSys->FindSeqId("LoadRifle"));

	m_mount1.Setup(pAnimSys->FindSeqIdAndBaseIndex("Mount", i));
	m_mount2.Setup(pAnimSys->FindSeqId("Mount", i+1));

	m_holdBow.Setup(pAnimSys->FindSeqId("HoldBow"));
	m_holdRifle.Setup(pAnimSys->FindSeqId("HoldRifle"));

	m_fall.Setup(pAnimSys->FindSeqId("Fall"));
	m_stun.Setup(pAnimSys->FindSeqId("Stun"));

	m_loot.Setup(pAnimSys->FindSeqId("Loot"));


	m_spellCastDirected.Setup(pAnimSys->FindSeqId("SpellCastDirected"));
	m_spellCastOmni.Setup(pAnimSys->FindSeqId("SpellCastOmni"));
	
	m_useStandingStart.Setup(pAnimSys->FindSeqId("UseStandingStart"));
	m_useStandingLoop.Setup(pAnimSys->FindSeqId("UseStandingLoop"));
	m_useStandingEnd.Setup(pAnimSys->FindSeqId("UseStandingEnd"));


	m_ready1H.Setup(pAnimSys->FindSeqId("Ready1H"));
	m_ready2H.Setup(pAnimSys->FindSeqId("Ready2H"));
	m_ready2HL.Setup(pAnimSys->FindSeqId("Ready2HL"));
	m_readyBow.Setup(pAnimSys->FindSeqId("ReadyBow"));
	m_readyRifle.Setup(pAnimSys->FindSeqId("ReadyRifle"));

	m_rollStart.Setup(pAnimSys->FindSeqId("RollStart"));
	m_roll.Setup(pAnimSys->FindSeqId("Roll"));
	m_rollEnd.Setup(pAnimSys->FindSeqId("RollEnd"));

	m_sheath.Setup(pAnimSys->FindSeqId("Sheath"));
	m_hipSheath.Setup(pAnimSys->FindSeqId("HipSheath"));
}

void WowAnimStateAttrib::JumpStart()
{	
	SetAnimState(&m_jumpStart);
}

void WowAnimStateAttrib::JumpEnd()
{
	SetAnimState(&m_jumpEnd);
}

void WowAnimStateAttrib::Jump()
{
	SetAnimState(&m_jump);
}



void AttackAnimState::Update( AnimationComponent *pkAnimComponent )
{
	// if current anim is done, play the next one...
	if( pkAnimComponent->IsAnimDone(m_seqId) )
	{
		m_seqId = m_group.GetNext();
		pkAnimComponent->PlayAnim(m_seqId);
	}
}

bool AttackAnimState::IsDone( AnimationComponent *pkAnimComponent )
{
	return true;
}
