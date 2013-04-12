#pragma once 
#include "AIBrain.h"

// ---------------------------------------------------------------------------------
// Basically just sets the idle animation state.
// ---------------------------------------------------------------------------------
class IdleBehavior : public AIBehaviorBase
{
public:
	IdleBehavior(AIBrain *pkBrain);

	void OnBegin(AIBehaviorBase *pPrevBehavior);
	void OnResume(AIBehaviorBase *pPrevBehavior);

	bool ShouldInvoke();
	const char *GetName()               { return "idle"; }
	virtual eBehaviorLevel GetLevel()   { return BL_IDLE; }

	bool Update();

	void GoIdle();
	bool TickIdle();
};


// ---------------------------------------------------------------------------------
// Simple path following behavior.
// ---------------------------------------------------------------------------------
class FollowPathBehavior : public AIBehaviorBase
{
public:
	FollowPathBehavior(AIBrain *pkBrain);

	void OnBegin(AIBehaviorBase *);

	void SetPath(class Path *pkPath);

	bool ShouldInvoke();
	const char *GetName()               { return "followpath"; }
	eBehaviorLevel GetLevel()           { return BL_IDLE_TASK; }

	virtual bool Update();

	virtual void SetIdleAtPathPointChance(size_t chance);
	virtual void SetMinMaxIdleTime(float minTime, float maxTime);

	bool TickStart();
	bool TickMove();
	bool TickWait();

	void DoMove(size_t index);

	bool Wait();

	bool IsWaiting();

protected:
	Path            *m_pkPath;
	uint32        m_pathIndex;
	bool            m_bFirstTime;
	float           m_minWaitTime;
	float           m_maxWaitTime;
	uint32        m_idleChance;
};

// ---------------------------------------------------------------------------------
// Behavior that moves a character to a destination point either by running or walking
// ---------------------------------------------------------------------------------
class MoveToBehavior : public AIBehaviorBase
{
public:
	MoveToBehavior(AIBrain *pkBrain);

	void RunTo(const noVec3 &pos);
	void WalkTo(const noVec3 &pos);
	void Stop();

	virtual void OnBegin(AIBehaviorBase *pPrevBehavior);
	virtual bool Update();

	virtual bool ShouldInvoke()                     { return m_bHaveDestination; }
	virtual const char *GetName()                   { return "moveto"; }
	virtual eBehaviorLevel GetLevel()               { return BL_SCRIPT_MOVE; }

private:
	bool        m_bHaveDestination;
	noVec3    m_destination;
	bool        m_bRun;
};


//-------------------------------------------------------------------------
// Stops other behaviors from running
// plays a death animation
//-------------------------------------------------------------------------
class DeathBehavior : public AIBehaviorBase
{
public:
	DeathBehavior(AIBrain *pkBrain);

	virtual void OnBegin(AIBehaviorBase *pPrevBehavior);
	virtual bool Update();

	virtual bool ShouldInvoke();
	virtual const char *GetName()                   { return "death"; }
	virtual eBehaviorLevel GetLevel()               { return BL_DEATH; }

};
