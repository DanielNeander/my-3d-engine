#include "stdafx.h"
#include "AIBrain.h"
#include "AIBehaviors.h"


// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
AIBehaviorBase::AIBehaviorBase(class AIBrain *pkBrain)
	: m_pkBrain(pkBrain)
	, m_bEnded(true)
{
}

bool AIBehaviorBase::Update()
{
	FSMObject::TickState();
	return true;
}

GameObject *AIBehaviorBase::GetGameObject()
{
	return m_pkBrain->GetGameObject();
}

noVec3 AIBehaviorBase::GetTranslation()
{
	return GetGameObject()->GetTranslation();
}

noMat3 AIBehaviorBase::GetRotation()
{
	return GetGameObject()->GetRotation();
}

noVec3 AIBehaviorBase::GetVelocity()
{
	return GetGameObject()->GetVelocity();
}

void AIBehaviorBase::SetTranslation(const noVec3 &translation)
{
	GetGameObject()->SetTranslation( translation );
}

void AIBehaviorBase::SetRotation(const noMat3 &rotation)
{
	GetGameObject()->SetRotation( rotation );
}

void AIBehaviorBase::SetVelocity(const noVec3 &velocity)
{
	GetGameObject()->SetVelocity( velocity );
}



// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
noImplementRTTI(AIBrain,GameObjectComponentBase);
AIBrain::AIBrain(GameObject *pkGameObject) : GameObjectComponentBase(pkGameObject)
{
	for (size_t ui=0;ui<BL_COUNT;ui++)
	{
		m_brainLevels[ui] = NULL;
	}

	m_behaviorLevel = BL_NONE;
}

AIBrain::~AIBrain()
{
	for (size_t ui=0;ui<m_behaviors.size();ui++)
	{
		delete m_behaviors.at(ui);
	}
	m_behaviors.clear();
}

bool AIBrain::Initialize()
{
	return true;
}

bool AIBrain::Update(float fDelta)
{
	UpdateBrainLevel();

	AIBehaviorBase	*pState = GetCurrentBehavior();

	if( pState )
	{
		if( pState->Update() )
		{
			return true;
		}
	}
	return false;
}

void AIBrain::AddBehavior(AIBehaviorBase *pBehavior)
{
	if( !pBehavior )
	{
		//		NIASSERT( pBehavior );
		return;
	}

	m_behaviors.push_back( pBehavior );
}

void AIBrain::UpdateBrainLevel()
{
	AIBehaviorBase		*pPrevBehavior	= GetCurrentBehavior();

	if( pPrevBehavior && pPrevBehavior->IsEnded() && !pPrevBehavior->ShouldSuspendPreviousBehavior() )
	{
		// we didn't suspend it, therefore we need to just end this one, and let the old brain-level naturally take over
		m_brainLevels[m_behaviorLevel]	= NULL;

		for(int i=m_behaviorLevel;i>=0;i--)
		{
			if( m_brainLevels[i] )
			{
				m_behaviorLevel	= (eBehaviorLevel)i;
				pPrevBehavior		= GetCurrentBehavior();
				break;
			}
		}
	}

	eBehaviorLevel	level			= BL_NONE;
	eBehaviorLevel	currLevel		= m_behaviorLevel;
	bool			behaviorEnded	= false;

	// End brain states that are "Ended"
	for( int i=currLevel;i>=0;i-- )
	{
		if( m_brainLevels[i] )
		{
			if( m_brainLevels[i]->IsEnded() )
			{
				m_brainLevels[i]->OnEnd();
				m_brainLevels[i]->SetEnded(false);

				if( pPrevBehavior==m_brainLevels[i] )
				{
					behaviorEnded	= true;
				}

				m_brainLevels[i]	= NULL;
				level				= BL_NONE;	// will get updated by next round... (if we have one)
			}
			else
			{
				level = (eBehaviorLevel)i;
				break;
			}
		}
	}

	m_behaviorLevel		= level;

	size_t	count				    = m_behaviors.size();
	AIBehaviorBase	*pCurrBehavior		= GetCurrentBehavior();
	AIBehaviorBase	*pNewBehavior		= NULL;
	AIBehaviorBase	*pTempBehavior;

	// See if we've got a new state to invoke...
	for( size_t i=0;i<count;i++ )
	{
		pTempBehavior		= m_behaviors.at(i);

		if( pCurrBehavior )
		{
			if( pTempBehavior==pCurrBehavior )
				continue;

			if( pTempBehavior->GetLevel() <= pCurrBehavior->GetLevel() )
				continue;

			if( pNewBehavior && pTempBehavior->GetLevel() < pNewBehavior->GetLevel() )
				continue;
		}

		if( pTempBehavior->ShouldInvoke() || (m_pkInvokedBehavior && (pTempBehavior==m_pkInvokedBehavior)) )
		{
			level						= pTempBehavior->GetLevel();
			m_behaviorLevel			= level;
			pNewBehavior				= pTempBehavior;
		}
	}

	if( pNewBehavior )
	{
		if( pCurrBehavior )
		{
			if( pCurrBehavior->GetLevel()==level )
			{
				// new behavior is forcing current (same level) behavior to end....
				pCurrBehavior->OnEnd();	
			}
			else
			{
				// if behaviorEnded is true, we just happen to be ending 1 behavior and starting another without ever resuming the lower behavior. 
				// Therefore don't suspend it as it is already suspended...
				if( !behaviorEnded && pNewBehavior->ShouldSuspendPreviousBehavior() )
					pCurrBehavior->OnSuspend(pNewBehavior);
			}
		}

		m_brainLevels[m_behaviorLevel]		= pNewBehavior;

		pNewBehavior->OnBegin(pPrevBehavior);
		return;
	}

	// if our new level is less than the level was when we entered this function, resume the new level.
	if( level!=BL_NONE && level < currLevel )
	{
		m_behaviorLevel = level;
		assert( m_brainLevels[level] );

		if( !pPrevBehavior || pPrevBehavior->ShouldSuspendPreviousBehavior() )
			m_brainLevels[level]->OnResume(pPrevBehavior);
	}
}

AIBehaviorBase *AIBrain::GetCurrentBehavior()
{
	if( m_behaviorLevel==BL_NONE )
		return NULL;

	return m_brainLevels[m_behaviorLevel];
}


void AIBrain::InvokeBehavior(AIBehaviorBase *pkBehavior)
{
	m_pkInvokedBehavior = pkBehavior;
}