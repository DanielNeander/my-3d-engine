#pragma once 

//------------------------------------------------------------------------------------
// ENTITY STATES

//Add new states here
#define  REGISTER_STATE_NAME(x)		x,
enum StateName 
{
#include "FSMStateNames.h"
	State_Num
};

#undef REGISTER_STATE_NAME

#define REGISTER_STATE_NAME(x)	#x,
static const char* StateNameTable[] = {
#include 	"FSMStateNames.h"
};

#undef REGISTER_STATE_NAME

//Add new substates here
enum SubstateName 
{
	SUBSTATE_WalkPath_Walk,
	SUBSTATE_WalkPath_Stop,
	SUBSTATE_WalkPath_Turn,
	SUBSTATE_WalkPath_Jump,
};

class GameObject;
class SingleAnimState;
class GroupedAnimBase;

class State
{
public:

	virtual ~State(){}

	//this will execute when the state is entered
	virtual void Enter(GameObject*)=0;

	//this is the states normal update function
	virtual void Execute(GameObject*)=0;

	//this will execute when the state is exited. 
	virtual void Exit(GameObject*)=0;

	//this executes if the agent receives a message from the 
	//message dispatcher
	//virtual bool OnMessage(GameObject*, const Telegram&)=0;

	SingleAnimState* currAnimState_;
	GroupedAnimBase* currGroupAnimState_;
};


class SimpleStateManager 
{
public:
	~SimpleStateManager();
	void RegisterState(int id, State* state);
	State* GetState(int id) const; 

	std::map<int, State* >	states_;
};






class IdleState : public State
{
public:
	IdleState();

	virtual ~IdleState();

	//this will execute when the state is entered
	virtual void Enter(GameObject*);

	//this is the states normal update function
	virtual void Execute(GameObject*);

	//this will execute when the state is exited. 
	virtual void Exit(GameObject*);

	bool bIdle_;
};


class WoundState : public State
{
public:
	virtual ~WoundState();

	//this will execute when the state is entered
	virtual void Enter(GameObject*);

	//this is the states normal update function
	virtual void Execute(GameObject*);

	//this will execute when the state is exited. 
	virtual void Exit(GameObject*);
};


class MoveState : public State
{
public:
	MoveState();
	virtual ~MoveState();

	//this will execute when the state is entered
	virtual void Enter(GameObject*);

	//this is the states normal update function
	virtual void Execute(GameObject*);

	//this will execute when the state is exited. 
	virtual void Exit(GameObject*);

	void WalkForward(GameObject* actor);
	void WalkBackward(GameObject* actor);

	bool bMove;
};

class JumpState : public State
{
public:
	JumpState();
	virtual ~JumpState();

	//this will execute when the state is entered
	virtual void Enter(GameObject*);

	//this is the states normal update function
	virtual void Execute(GameObject*);

	//this will execute when the state is exited. 
	virtual void Exit(GameObject*);

	bool	jumping_;	
};



class AttackState : public State
{
public:
	virtual ~AttackState();

	
	//this will execute when the state is entered
	virtual void Enter(GameObject*);

	//this is the states normal update function
	virtual void Execute(GameObject*);

	//this will execute when the state is exited. 
	virtual void Exit(GameObject*);

		
	void Attack1h(GameObject* actor);
	void Attack2h(GameObject* actor);
	void AttackBow(GameObject* actor);
	void AttackRifle(GameObject* actor);
	void AttackUnarmed(GameObject* actor);

	void Hold1Attack1h(GameObject* actor);
	void Hold1Attack2h(GameObject* actor);	
	
	void SpellCastOmni(GameObject* actor);
	void SpellCastDirected(GameObject* actor);
	void ReadyAttack1h( GameObject* actor );
	void ReadyAttack2h( GameObject* actor );
	void ReadyAttackUnarmed( GameObject* actor );
	void ReadyAttack2hl( GameObject* actor );

};


class DeathState : public State
{
public:
	DeathState();

	virtual ~DeathState();

	//this will execute when the state is entered
	virtual void Enter(GameObject*);

	//this is the states normal update function
	virtual void Execute(GameObject*);

	//this will execute when the state is exited. 
	virtual void Exit(GameObject*);

	bool bDie_;
};
