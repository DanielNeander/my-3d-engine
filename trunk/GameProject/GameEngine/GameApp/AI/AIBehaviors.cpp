#include "stdafx.h"
#include "AIBehaviors.h"
#include "../GameObjectAttribute.h"
#include "../GameObjectComponent.h"
#include "../GameObject.h"
#include "../GameObjectUtil.h"

#include "PathManager.h"

DEFINE_STATE(STATE_IDLE,IdleBehavior,TickIdle,"idle");
IdleBehavior::IdleBehavior(AIBrain *pkBrain) : AIBehaviorBase(pkBrain)
{

}

void IdleBehavior::OnBegin(AIBehaviorBase *pPrevBehavior)
{
	GoIdle();
}

void IdleBehavior::OnResume(AIBehaviorBase *pPrevBehavior)
{
	GoIdle();
}

bool IdleBehavior::ShouldInvoke()
{
	return true;
}

bool IdleBehavior::Update()
{
	return AIBehaviorBase::Update();
}

void IdleBehavior::GoIdle()
{
	BasicCharacterAnimStateAttribute   *pkAttrib = GetGameObjectAttribute(GetGameObject(),BasicCharacterAnimStateAttribute);
	assert(pkAttrib);
	pkAttrib->Idle();

	CharacterMoverComponent *pkMover = GetGameObjectComponent(m_pkBrain->GetGameObject(),CharacterMoverComponent);
	pkMover->Stop();
}

bool IdleBehavior::TickIdle()
{
	return true;
}

DEFINE_STATE(STATE_FOLLOWPATH_START,FollowPathBehavior,TickStart,"start");
DEFINE_STATE(STATE_FOLLOWPATH_MOVE,FollowPathBehavior, TickMove, "move");
DEFINE_STATE(STATE_FOLLOWPATH_WAIT,FollowPathBehavior, TickWait, "wait");

FollowPathBehavior::FollowPathBehavior(AIBrain *pkBrain) : AIBehaviorBase(pkBrain)
	, m_pkPath(NULL)
{
	m_pathIndex         = 0;
	m_minWaitTime       = 5;
	m_maxWaitTime       = 5.5;
	m_idleChance        = 0;
}

void FollowPathBehavior::OnBegin(AIBehaviorBase *)
{
	BasicCharacterAnimStateAttribute   *pkAttrib = GetGameObjectAttribute(GetGameObject(),BasicCharacterAnimStateAttribute);
	assert(pkAttrib);
	pkAttrib->Idle();
	TransitionState(&STATE_FOLLOWPATH_START,GetRandom(m_minWaitTime,m_maxWaitTime));
}

void FollowPathBehavior::SetPath(Path *pkPath)
{
	m_pkPath        = pkPath;
	m_pathIndex     = 0;
	if( pkPath==NULL )
		EndBehavior();
}

bool FollowPathBehavior::ShouldInvoke()
{
	if( m_pkPath )
		return true;

	return false;
}

bool FollowPathBehavior::Update()
{
	if( m_pkPath==NULL )
	{
		EndBehavior();
		return false;
	}

	AIBehaviorBase::Update();

	return true;
}

void FollowPathBehavior::SetIdleAtPathPointChance(size_t chance)
{
	m_idleChance = chance;
}

void FollowPathBehavior::SetMinMaxIdleTime(float minTime, float maxTime)
{
	m_minWaitTime = minTime;
	m_maxWaitTime = maxTime;
}

bool FollowPathBehavior::TickStart()
{
	if( m_stateTime<GetTime() )
	{
		m_pathIndex = 0;
		DoMove(0);
	}
	return true;
}

bool FollowPathBehavior::TickMove()
{
	GameObject                      *pkGameObject = m_pkBrain->GetGameObject();
	CharacterMoverComponent        *pkMover = GetGameObjectComponent(pkGameObject,CharacterMoverComponent);
	assert(pkMover);

	if( pkMover->IsMoveDone() )
	{
		m_pathIndex++;

		if( m_pathIndex>= m_pkPath->size() )
		{
			m_pathIndex = 0;

			BasicCharacterAnimStateAttribute   *pkAttrib = GetGameObjectAttribute(GetGameObject(),BasicCharacterAnimStateAttribute);
			assert(pkAttrib);
			pkAttrib->Idle();

			TransitionState(&STATE_FOLLOWPATH_WAIT,GetRandom(m_minWaitTime,m_maxWaitTime));
			return true;
		}
		else
		{
			if( Wait() )
			{
				return true;
			}
		}

		DoMove(m_pathIndex);
	}
	return true;
}

bool FollowPathBehavior::TickWait()
{
	if( GetTime()>m_stateTime )
	{
		DoMove(m_pathIndex);
	}
	return true;
}

void FollowPathBehavior::DoMove(uint32 index)
{
	PathNode pt = m_pkPath->at(index);

	CharacterMoverComponent                    *pkMover = GetGameObjectComponent(m_pkBrain->GetGameObject(),CharacterMoverComponent);
	assert(pkMover);

	pkMover->MoveTo(pt.m_position);

	m_idleChance    = pt.m_pauseChance;

	GameObjectRotatorAttribute              *pkRotator = GetGameObjectAttribute(m_pkBrain->GetGameObject(),GameObjectRotatorAttribute);

	if( pkRotator )
	{
		noVec3    dir = pt.m_position - m_pkBrain->GetGameObject()->GetTranslation();
		MathHelpers::UP_COMPONENT(dir) = 0;
		dir.Normalize();
		pkRotator->FaceDir2D( dir );
	}

	BasicCharacterAnimStateAttribute   *pkAttrib = GetGameObjectAttribute(GetGameObject(),BasicCharacterAnimStateAttribute);
	assert(pkAttrib);
	pkAttrib->Walk();

	BasicCharacterMovementAttribute*          pkMovementAttrib = GetGameObjectAttribute(GetGameObject(),BasicCharacterMovementAttribute);
	if( pkMovementAttrib )
		pkMovementAttrib->Walk();

	TransitionState(&STATE_FOLLOWPATH_MOVE);
}

bool FollowPathBehavior::Wait()
{
	if( m_idleChance==0 )
		return false;

	uint32 random = MathHelpers::GetRandomInteger()%100;

	if( random > m_idleChance )
		return false;

	BasicCharacterAnimStateAttribute   *pkAttrib = GetGameObjectAttribute(GetGameObject(),BasicCharacterAnimStateAttribute);
	assert(pkAttrib);
	pkAttrib->Idle();

	TransitionState(&STATE_FOLLOWPATH_WAIT,GetRandom(m_minWaitTime,m_maxWaitTime));
	return true;
}

bool FollowPathBehavior::IsWaiting()
{
	if( m_state==&STATE_FOLLOWPATH_WAIT )
	{
		return true;
	}
	return false;
}


// -----------------------------------------------------------------
// -----------------------------------------------------------------
MoveToBehavior::MoveToBehavior(AIBrain *pkBrain) : AIBehaviorBase(pkBrain)
	, m_bHaveDestination(false)
	, m_destination(vec3_zero)
	, m_bRun(false)
{
}

void MoveToBehavior::RunTo(const noVec3 &pos)
{
	m_bHaveDestination = true;
	m_bRun = true;
	m_destination = pos;
}

void MoveToBehavior::WalkTo(const noVec3 &pos)
{
	m_bHaveDestination  = true;
	m_bRun              = false;
	m_destination       = pos;
}

void MoveToBehavior::Stop()
{
	m_bHaveDestination = false;
	EndBehavior();
}


void MoveToBehavior::OnBegin(AIBehaviorBase *pPrevBehavior)
{
	CharacterMoverComponent    *pkComp = GetGameObjectComponent(GetGameObject(),CharacterMoverComponent);
	assert(pkComp);
	pkComp->MoveTo(m_destination);

	BasicCharacterAnimStateAttribute    *pkAnimAttrib = GetGameObjectAttribute(GetGameObject(),BasicCharacterAnimStateAttribute);
	BasicCharacterMovementAttribute     *pkMoveAttrib = GetGameObjectAttribute(GetGameObject(),BasicCharacterMovementAttribute);

	if( m_bRun )
	{
		if( pkAnimAttrib )
			pkAnimAttrib->Run();

		if( pkMoveAttrib )
			pkMoveAttrib->Run();
	}
	else
	{
		if( pkAnimAttrib )
			pkAnimAttrib->Walk();

		if( pkMoveAttrib )
			pkMoveAttrib->Walk();
	}
}

bool MoveToBehavior::Update()
{
	CharacterMoverComponent    *pkComp = GetGameObjectComponent(GetGameObject(),CharacterMoverComponent);

	if( pkComp->IsMoveDone() )
	{
		m_bHaveDestination = false;
		EndBehavior();
	}

	return true;
}


//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
DeathBehavior::DeathBehavior(AIBrain *pkBrain)
	: AIBehaviorBase(pkBrain)
{

}

void DeathBehavior::OnBegin(AIBehaviorBase *pPrevBehavior)
{
	BasicCharacterMovementAttribute *pkMoveAttrib = GetGameObjectAttribute(GetGameObject(),BasicCharacterMovementAttribute);
	if( pkMoveAttrib )
		pkMoveAttrib->Stop();

	BasicCharacterAnimStateAttribute   *pkAttrib = GetGameObjectAttribute(GetGameObject(),BasicCharacterAnimStateAttribute);
	assert(pkAttrib);
	pkAttrib->Death();
}

bool DeathBehavior::Update()
{
	return true;
}

bool DeathBehavior::ShouldInvoke()
{
	return false;
}
