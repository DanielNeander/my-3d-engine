#include "stdafx.h"
#include "EngineCore/Util/GameLog.h"
#include "BaseStates.h"
#include "../GameObjectUtil.h"
#include "../GameObjectAttribute.h"
#include "../GameObjectComponent.h"
#include "ModelLib/Model_M2Format.h"
#include "../M2Object.h"
#include "ModelLib/M2AnimationManager.h"
#include "../AnimationSystem.h"
#include "../AnimationComponent.h"
#include "../Mesh.h"
#include "../M2Mesh.h"


#include "../TestObjects.h"


SimpleStateManager::~SimpleStateManager()
{
	//hkMap<hkUint32, SimpleBipedState*>::iterator itr = m_registeredStates.begin();
	//while (itr < m_registeredStates.end())
	//{
	//	itr->second->removeReference();
	//	itr++;
	//}
	std::map<int, State* >::iterator itr = states_.begin();
	while (itr != states_.end())
	{
		SAFE_DELETE(itr->second);
		++itr;
	}
	states_.clear();
}



State* SimpleStateManager::GetState( int id ) const
{
	return states_.find(id)->second;
	
}




void SimpleStateManager::RegisterState( int id, State* state )
{
	//state->addReference();

	//if (m_registeredStates.find( stateId ))
	//{
	//	m_registeredStates[ stateId ]->removeReference();
	//}

	//m_registeredStates[stateId] = state;
	std::map<int, State* >::iterator iter = states_.find( id );	
	if (iter == states_.end())
	{ 
		states_[id] = state;
	}
}


void IdleState::Exit( GameObject* )
{
	bIdle_ = false;
}


void IdleState::Execute( GameObject* )
{

}


void IdleState::Enter( GameObject* actor)
{
	if (bIdle_) return;	
	currGroupAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	
	if (pkAnimAttrib) 	{		
		pkAnimAttrib->Idle();
		currGroupAnimState_ = (IdleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);				
		currGroupAnimState_->OnActivated(pAnimComp);
		pAnimComp->AnimSys()->SetLoop(1);	// 강제 변경..

		bIdle_ = true;
	}
}


IdleState::~IdleState()
{

}

IdleState::IdleState()
	:bIdle_(false)
{

}


void MoveState::Exit( GameObject* )
{
	
}


void MoveState::Execute( GameObject* )
{
	// Update Position
}


void MoveState::Enter( GameObject* actor)
{
	LOG_INFO << " Move Enter";
	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if (pkAnimAttrib) 	{
		pkAnimAttrib->Walk();
		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);				
		pAnimComp->PlayAnim(currAnimState_->GetSeqId(), 1);
	}
}


MoveState::~MoveState()
{

}

void MoveState::WalkForward(GameObject* actor)
{
	if (bMove) return;

	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if (pkAnimAttrib) 	{
		pkAnimAttrib->Walk();
		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);								
		pAnimComp->PlayAnim(currAnimState_->GetSeqId());				
		pAnimComp->AnimSys()->SetLoop(1);

		bMove = true;		
	}
}

void MoveState::WalkBackward(GameObject* actor)
{
	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if (pkAnimAttrib) 	{
		pkAnimAttrib->WalkBack();
		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);
		pAnimComp->PlayAnim(currAnimState_->GetSeqId());
	}
}

MoveState::MoveState()
{
	bMove = false;
}


void AttackState::Exit( GameObject* )
{

}


void AttackState::Execute( GameObject* actor)
{
	WowActor* pActor = (WowActor*)actor;

	AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
		
	switch(pActor->GetWeaponKind())
	{
	case WEAPONK_BOW:
		if (pAnimComp->AnimSys()->GetCurrSeq() == INVALID_SEQ_ID &&  currAnimState_ == &pkAnimAttrib->m_holdBow)
		{
			AttackBow(actor);
		}
		break;
	case WEAPONK_RIFLE:
		if (pAnimComp->AnimSys()->GetCurrSeq() == INVALID_SEQ_ID &&  currAnimState_ == &pkAnimAttrib->m_holdRifle)
		{
			AttackRifle(actor);
		}
		break;
	}
}

void AttackState::ReadyAttack1h( GameObject* actor )
{
	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if(  pkAnimAttrib )
	{
		pkAnimAttrib->ReadyAttack1h();		
		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);			
		pAnimComp->PlayAnim(currAnimState_->GetSeqId(), 1);		
	}
}

void AttackState::Enter( GameObject* obj)
{
	WowActor* actor = (WowActor*)obj;

	switch(actor->GetWeaponType())
	{
	case WEAPONT_1H:
		Attack1h(actor);
		break;
	case WEAPONT_2H:
		Attack2h(actor);
		break;	
	}

	switch(actor->GetWeaponKind())
	{
	case WEAPONK_BOW:
	case WEAPONK_RIFLE:
		//AttackBow(actor);
		Hold1Attack2h(actor);		
		//AttackRifle(actor);
		break;				
	}

	//AttackUnarmed(actor);
	

	//Attack2h(actor);
}


AttackState::~AttackState()
{

}

void AttackState::Attack1h( GameObject* actor )
{
	currGroupAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if (pkAnimAttrib) 	{
		pkAnimAttrib->Attack1h();
		currGroupAnimState_ = (AttackAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);				
		currGroupAnimState_->OnActivated(pAnimComp);		
	}
}

void AttackState::Attack2h( GameObject* actor )
{
	currGroupAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if (pkAnimAttrib) 	{
		pkAnimAttrib->Attack2h();
		currGroupAnimState_ = (AttackAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);				
		currGroupAnimState_->OnActivated(pAnimComp);		
	}
}

void AttackState::AttackBow( GameObject* actor )
{
	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if (pkAnimAttrib) 	{
		pkAnimAttrib->AttackBow();

		M2Mesh* pBow = (M2Mesh*)actor->GetNode()->GetChildByName("Bow");
		if (pBow)
		{
			// PULL
			pBow->m2_->animManager->SetAnim(0, 2, 0);
			pBow->m2_->animManager->Play();
		}
		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);		
		pAnimComp->PlayAnim(currAnimState_->GetSeqId());
	}
}

void AttackState::AttackUnarmed( GameObject* actor )
{
	currGroupAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if (pkAnimAttrib) 	{
		pkAnimAttrib->AttackUnarmed();
		currGroupAnimState_ = (AttackAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);		
		currGroupAnimState_->OnActivated(pAnimComp);		
	}
}

void AttackState::SpellCastOmni( GameObject* actor )
{
	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if(  pkAnimAttrib )
	{
		pkAnimAttrib->SpellCastOmni();		
		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);			
		pAnimComp->PlayAnim(currAnimState_->GetSeqId());		
	}
}

void AttackState::SpellCastDirected( GameObject* actor )
{
	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if(  pkAnimAttrib )
	{
		pkAnimAttrib->SpellCastDirected();		
		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);			
		pAnimComp->PlayAnim(currAnimState_->GetSeqId());		
	}
}

void AttackState::Hold1Attack1h( GameObject* actor )
{
	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if (pkAnimAttrib) 	{			

		switch(pActor->GetWeaponKind())
		{
		case WEAPONK_BOW:		
			{			
				pkAnimAttrib->HoldBow();				

				M2Mesh* pBow = (M2Mesh*)actor->GetNode()->GetChildByName("Bow");
				if (pBow)
				{
					// PULL
					pBow->m2_->animManager->SetAnim(0, 1, 1);
					pBow->m2_->animManager->Play();
				}
			}
			break;
		//case WEAPONK_RIFLE:
			//pkAnimAttrib->HoldRifle();
			//break;
		}

		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);		
		pAnimComp->PlayAnim(currAnimState_->GetSeqId());
	}
}

void AttackState::Hold1Attack2h( GameObject* actor )
{
	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if (pkAnimAttrib) 	{			
		
		switch(pActor->GetWeaponKind())
		{
			case WEAPONK_BOW:		
			{			
				pkAnimAttrib->HoldBow();				
				
			}
			break;
			case WEAPONK_RIFLE:
				pkAnimAttrib->HoldRifle();
				break;
		}
		
		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);		
		pAnimComp->PlayAnim(currAnimState_->GetSeqId());
	}
}

void AttackState::ReadyAttack2h( GameObject* actor )
{
	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if (pkAnimAttrib) 	{			

		pkAnimAttrib->ReadyAttack2h();
		switch(pActor->GetWeaponKind())
		{		
		case WEAPONK_BOW:		
			pkAnimAttrib->ReadyAttackBow();				
			break;
		case WEAPONK_RIFLE:
			pkAnimAttrib->ReadyAttackRifle();
			break;
		}

		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);				
		pAnimComp->PlayAnim(currAnimState_->GetSeqId(), 1);
	}
}

void AttackState::ReadyAttack2hl( GameObject* actor )
{
	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if (pkAnimAttrib) 	{			

		pkAnimAttrib->ReadyAttack2h();

		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);		
		pAnimComp->PlayAnim(currAnimState_->GetSeqId(), 1);
	}
}

void AttackState::ReadyAttackUnarmed( GameObject* actor )
{

}

void AttackState::AttackRifle( GameObject* actor )
{
	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if (pkAnimAttrib) 	{
		pkAnimAttrib->AttackRifle();
		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);		
		pAnimComp->PlayAnim(currAnimState_->GetSeqId());
	}
}

JumpState::~JumpState()
{

}

void JumpState::Enter( GameObject* actor)
{
	if (jumping_) return;

	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if( !jumping_ && pkAnimAttrib )
	{
		pkAnimAttrib->JumpStart();		
		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);			
		pAnimComp->PlayAnim(currAnimState_->GetSeqId());
	}
}

void JumpState::Execute( GameObject* actor)
{
	WowActor* pActor = (WowActor*)actor;
	AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	
		
	if (pAnimComp->AnimSys()->GetCurrSeq() == INVALID_SEQ_ID &&  currAnimState_ == &pkAnimAttrib->m_jumpStart)
	{
		if( pkAnimAttrib )		
			pkAnimAttrib->Jump();

		jumping_ = true;
		currAnimState_ = &pkAnimAttrib->m_jump;
		pAnimComp->PlayAnim(currAnimState_->GetSeqId());
	}

	if (pAnimComp->AnimSys()->GetCurrSeq() == INVALID_SEQ_ID &&  currAnimState_ == &pkAnimAttrib->m_jump)	
	{		
		if( pkAnimAttrib )
			pkAnimAttrib->JumpEnd();
		
		currAnimState_ = &pkAnimAttrib->m_jumpEnd;
		pAnimComp->PlayAnim(currAnimState_->GetSeqId());		
	}

	if (pAnimComp->AnimSys()->GetCurrSeq() == INVALID_SEQ_ID &&  currAnimState_ == &pkAnimAttrib->m_jumpEnd)
	{		
		currAnimState_ = NULL;
		jumping_ = false;
 		StateMachine* pStateMachine = pActor->GetStateMachine();
		//pStateMachine->SendMsgDelayedToMe(0.5f, MSG_Idle);
		pStateMachine->ChangeState(STATE_Idle);
	}
}

void JumpState::Exit( GameObject* )
{
	jumping_ = false;
}

JumpState::JumpState() : jumping_(false)
{

}

WoundState::~WoundState()
{

}

void WoundState::Enter( GameObject* actor)
{
	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if (pkAnimAttrib) 	{
		pkAnimAttrib->WoundCombat();
		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);
		pAnimComp->PlayAnim(currAnimState_->GetSeqId());
	}
}

void WoundState::Execute( GameObject* actor)
{
	WowActor* pActor = (WowActor*)actor;
	AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	
	if (pAnimComp->AnimSys()->GetCurrSeq() == INVALID_SEQ_ID &&  currAnimState_ == &pkAnimAttrib->m_combatWound)
	{
		pActor->GetStateMachine()->ChangeStateDelayed(0.1f, STATE_Idle);
	}
}

void WoundState::Exit( GameObject* )
{

}

DeathState::DeathState()
	:bDie_(false)
{

}

DeathState::~DeathState()
{

}

void DeathState::Enter( GameObject* actor)
{
	if (bDie_) return;

	currAnimState_ = NULL;	
	WowActor* pActor = (WowActor*)actor;
	WowAnimStateAttrib    *pkAnimAttrib = GetGameObjectAttribute(pActor,WowAnimStateAttrib);
	if (pkAnimAttrib) 	{
		pkAnimAttrib->Death();
		currAnimState_ = (SingleAnimState*)pkAnimAttrib->GetAnimState();
		AnimationComponent* pAnimComp = GetGameObjectComponent(pActor, AnimationComponent);
		pAnimComp->PlayAnim(currAnimState_->GetSeqId());
		bDie_ = true;
	}
}

void DeathState::Execute( GameObject* actor)
{

}

void DeathState::Exit( GameObject* )
{	
	bDie_ = false;
	
}
