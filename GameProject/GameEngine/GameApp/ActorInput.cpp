#include "stdafx.h"
#include "ActorInput.h"
#include "TestObjects.h"
#include "Common.h"
#include "AI/Message.h"
#include "GameApp/GameApp.h"
#include "AI/BaseStates.h"

#include "AI/GameWorld.h"
#include "AI/CombatSystem.h"
#include "M2Object.h"
#include "GameObjectAttribute.h"
#include "GameObjectUtil.h"

extern GameWorld*	gGameWorld;


ActorInput::ActorInput( Actor* pActor )
	:Actor_(pActor)
{
	Actrl_ = (ActorController*)pActor->GetStateMachine();

}

ActorInput::~ActorInput()
{

}

WowActorInput::WowActorInput( WowActor* pActor ) : ActorInput( pActor )
{
}

WowActorInput::~WowActorInput()
{

}



void WowActorInput::HandleInput( float DeltaTime )
{
	if (!Actrl_)
		return;

	SimpleStateManager* pStateMgr = Actrl_->GetStateMgr();
	MoveState* move = (MoveState*)pStateMgr->GetState(STATE_Move);

	if(GetMouseDown(VK_LBUTTON))
	{	
		if (PickUtil::pickDatas.empty())
			return;
		PickData data = PickUtil::pickDatas[0];		
		gGameWorld->SetCrosshair(data.vert);		

		//Actrl_->SendMsgToMe(MSG_FindPath);
		//Actrl_->setPathEnd(PickUtil::GetPickVert());
		Actrl_->ChangeState(STATE_Attack);
		
		if (!PickUtil::pickDatas.empty())
		{
			//noVec3 dir = data.vert - Actor_->GetNode()->WorldTrans();
			//dir.Normalize();
			//GameObjectRotatorAttribute* roateAtt = GetGameObjectAttribute(Actor_, GameObjectRotatorAttribute);
			CharacterRotatorComponent* pkRotator = GetGameObjectComponent(Actor_, CharacterRotatorComponent);
			pkRotator->TurnToTarget(data.vert);
			

			GameObject* pTarget = NULL;
			//Actrl_->GetClosestTarget(PickUtil::GetPickVert(), pTarget);			 		 
			 
			 M2Object* pObj = reinterpret_cast<M2Object*> (data.pUserData);
			 if (pObj)
			 {
				 pTarget = pObj->Owner_;	
			 }			
		
			if (pTarget)
			{			
				CombatSystem::GetSingletonPtr()->SetTarget(pTarget);
			}
			else 
				CombatSystem::GetSingletonPtr()->SetTarget(NULL);
		}
	}

	if(GetKeyDown('W'))
	{
		if (Actrl_->GetState() != STATE_Jump)
		{
			//Actrl_->ChangeState(STATE_Move);						
		}
	}

	if(GetKeyDown('A'))
	{
		//Actrl_->SendMsgToMe(MSG_Move);
	}

	if(GetKeyDown('S'))
	{		
		//Actrl_->SendMsgToMe(MSG_Move);
		//Actrl_->ChangeState(STATE_Move);
		//move->WalkBackward(Actor_);
	}

	if(GetKeyDown('D'))
	{
		//Actrl_->SendMsgToMe(MSG_Move);
	}

	if(GetKeyDown(VK_SPACE))
	{
		//if (Actrl_->GetState() != STATE_Jump)
		//	Actrl_->ChangeState(STATE_Jump);
	}


}

