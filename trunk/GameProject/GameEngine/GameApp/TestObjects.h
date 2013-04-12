#pragma once 

#include "Actor.h"
#include "GameObjectComponent.h"
#include "GameObjectAttribute.h"
#include "AnimationComponent.h"
#include "AnimationStateComponent.h"
#include "AI/AIBrain.h"
#include "AI/AIBehaviors.h"
#include "M2Equipment.h"
#include "M2Edit.h"

class M2Loader;


// ---------------------------------------------------------------------------------
// Idle Animation State will continually play all the animations in its group.
// ---------------------------------------------------------------------------------
class AttackAnimState : public GroupedAnimBase
{
public:
	virtual void Update(AnimationComponent *pkAnimComponent);
	virtual bool IsDone(AnimationComponent *pkAnimComponent);
};


class WowAnimStateAttrib : public BasicCharacterAnimStateAttribute
{
public:
	noDeclareRTTI;

	void Setup(class M2Object* pAnimSys);

	void JumpStart();
	void Jump();
	void JumpEnd();

	void Attack1h() { SetAnimState(&m_attack1h); }
	void Attack2h() { SetAnimState(&m_attack2h); }
	void AttackUnarmed()  { SetAnimState(&m_attackUnarmed); }
	void AttackBow() { SetAnimState(&m_attackBow);}
	void AttackRifle() { SetAnimState(&m_attackRifle);}

	void WoundStand() { SetAnimState(&m_standWound);}
	void WoundCombat() { SetAnimState(&m_combatWound);}

	void ReadyAttack1h() { SetAnimState(&m_ready1H); }
	void ReadyAttack2h() { SetAnimState(&m_ready2H); }
	void ReadyAttack2hl() { SetAnimState(&m_ready2HL); }
	void ReadyAttackBow() { SetAnimState(&m_readyBow); }
	void ReadyAttackRifle() { SetAnimState(&m_readyRifle); }

	void SpellCastOmni()	{ SetAnimState(&m_spellCastOmni); }
	void SpellCastDirected()	{ SetAnimState(&m_spellCastDirected); }
						
	void HoldBow()		{ SetAnimState(&m_holdBow); }
	void HoldRifle()		{ SetAnimState(&m_holdRifle); }
		

//protected:
	IdleAnimState                   m_idle;
	SingleAnimState                 m_walk;
	SingleAnimState					m_walkback;
	SingleAnimState                 m_run;
	SingleAnimState                 m_death;
	SingleAnimState					m_jumpStart;
	SingleAnimState					m_jump;
	SingleAnimState					m_jumpEnd;
	AttackAnimState					m_attack1h;
	AttackAnimState					m_attack2h;
	AttackAnimState					m_attackUnarmed;
		
	SingleAnimState					m_mount1;
	SingleAnimState					m_mount2;

	SingleAnimState					m_loadBow;	
	SingleAnimState					m_holdBow;
	SingleAnimState					m_holdRifle;
	SingleAnimState					m_loadRifle;	
	SingleAnimState					m_attackBow;
	SingleAnimState					m_attackRifle;
	
	SingleAnimState					m_standWound;
	SingleAnimState					m_stun;
	SingleAnimState					m_combatCritical;	
	SingleAnimState					m_combatWound;
	SingleAnimState					m_dodge;
	SingleAnimState					m_fall;
	SingleAnimState					m_kick;	

	SingleAnimState					m_swim;
	SingleAnimState					m_swimWalk;	
	SingleAnimState					m_swimIdle;	
	SingleAnimState					m_swimBackwards;	
	SingleAnimState					m_swimLeft;	
	SingleAnimState					m_swimRight;	

		
	SingleAnimState					m_spellCastOmni;
	SingleAnimState					m_spellCastDirected;

	SingleAnimState					m_useStandingStart;
	SingleAnimState					m_useStandingLoop;
	SingleAnimState					m_useStandingEnd;

	SingleAnimState					m_loot;

	SingleAnimState					m_ready1H;
	SingleAnimState					m_ready2H;
	SingleAnimState					m_ready2HL;
	SingleAnimState					m_readyBow;
	SingleAnimState					m_readyRifle;

	SingleAnimState					m_rollStart;
	SingleAnimState					m_roll;
	SingleAnimState					m_rollEnd;

	SingleAnimState					m_readySpellOmni;
	SingleAnimState					m_readySpellDirected;

	SingleAnimState					m_hipSheath;
	SingleAnimState					m_sheath;
};


class WowBrain : public AIBrain
{
public:
	WowBrain(GameObject* pkGameObject);

	virtual bool Initialize();
	void FollowPath(class Path *pkPath);
	void FollowPathStop();

	void RunTo(const noVec3 &pos);
	void WalkTo(const noVec3 &pos);
	void StopMoveTo();
	bool IsMoveToDone();

protected:
	class FollowPathBehavior	*m_pkFollowPath;
	class MoveToBehavior        *m_pkMoveToBehavior;
};


class WowActor : public Actor
{
public:	
	noDeclareRTTI;

	WowActor(const std::string& filename);

	void update (float elapsedTime);

	void AttachComponents();

	void AttachInput(class ActorInput* input);
	
	void StartPath();
	void FollowPath();

	void GoAI();
	void GoPlayer();
	void ResetPosition();
	void Stop();

	bool TickStartPath();

	int  GetWeaponType() const { return meshEdit_->Equip_->GetWeaponType(); }
	int	 GetWeaponKind() const { return meshEdit_->Equip_->GetWeaponKind();	}
	

protected:
	virtual void Initialize();
	virtual void LoadModel();
	virtual void UpdateInternal( float fDelta );	
	void UpdateModelInfo(M2Loader* m2);
	void CreateRenderModel(M2Loader* m2);
	void CreateTransformComponent();
public:
	WowBrain              *m_pkBrain;
	WowAnimStateAttrib   *m_pkAttrib;
		
	class M2Mesh* mesh_;
	class M2Edit* meshEdit_;
	ActorInput* input_;
	
};




