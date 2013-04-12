#pragma once 


#pragma once
#include "../FSM.h"
#include "../GameObjectComponent.h"


// ---------------------------------------------------------------------------------
// Each behavior has a Level associated with it.  These levels determine a priority
// for each behavior. For instance, if the current behavior is at level BL_COMBAT
// and a behavior of level BL_PAIN wants to invoke, the BL_COMBAT behavior will be
// suspended until the BL_PAIN behavior ends.
// ---------------------------------------------------------------------------------
enum eBehaviorLevel
{
	BL_NONE
	,   BL_IDLE
	,   BL_IDLE_TASK
	,   BL_COMBAT
	,   BL_PLAY_ANIM
	,   BL_SCRIPT_MOVE
	,   BL_PAIN
	,   BL_DEATH
	,   BL_COUNT
};

// ---------------------------------------------------------------------------------
// Abstract Base class for all AI Behaviors
// ---------------------------------------------------------------------------------
class AIBehaviorBase : public FSMObject
{
public:
	AIBehaviorBase(class AIBrain *pkBrain);

	virtual bool Update();

	virtual void OnBegin(AIBehaviorBase *pPrevBehavior)			    {}
	virtual void OnSuspend(AIBehaviorBase *pNextBehavior)		    {}
	virtual void OnResume(AIBehaviorBase *pPrevBehavior)		    {}
	virtual void OnEnd()									        {}

	virtual bool ShouldInvoke()=0;
	virtual const char *GetName()=0;
	virtual eBehaviorLevel GetLevel()=0;

	virtual bool IsEnded()                                      { return m_bEnded; }
	virtual void SetEnded(bool ended)                           { m_bEnded = ended; }
	virtual void EndBehavior()                                  { SetEnded(true); }
	virtual bool ShouldSuspendPreviousBehavior()                { return true; }

	virtual GameObject *GetGameObject();


protected:
	AIBrain                                 *m_pkBrain;


	// helper functions
	// helper functions
	noVec3 GetTranslation();
	noMat3 GetRotation();
	noVec3 GetVelocity();
	float GetScale();


	void SetTranslation(const noVec3 &translation);
	void SetRotation(const noMat3 &rotation);
	void SetVelocity(const noVec3 &velocity);
	void SetScale(float fScale);


private:
	bool                                    m_bEnded;

};


// ---------------------------------------------------------------------------------
// Manages AI Behaviors.
// ---------------------------------------------------------------------------------
class AIBrain : public GameObjectComponentBase
{
public:
	noDeclareRTTI;
	
	AIBrain(GameObject *pkGameObject);
	virtual ~AIBrain();

	virtual bool Initialize();
	virtual bool Update(float fDelta);

	virtual void AddBehavior(AIBehaviorBase *pBehavior);
	virtual void InvokeBehavior(AIBehaviorBase *pkBehavior);

protected:
	std::vector<AIBehaviorBase*>        m_behaviors;
	AIBehaviorBase*                         m_brainLevels[BL_COUNT];
	eBehaviorLevel                          m_behaviorLevel;
	AIBehaviorBase*                         m_pkInvokedBehavior;

	void UpdateBrainLevel();
	AIBehaviorBase *GetCurrentBehavior();

};