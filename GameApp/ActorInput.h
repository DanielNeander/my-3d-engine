#pragma once 

#include "ActorInput.h"


class ActorInput 
{
public:
	ActorInput(class Actor* pActor);
	~ActorInput();

	Actor* Actor_;
	class ActorController* Actrl_;
	virtual void HandleInput(float fDeltaTime) {}
};

class WowActorInput : public ActorInput
{
public:
	WowActorInput(class WowActor* pActor);
	~WowActorInput();

	void HandleInput(float DeltaTime);


protected:	
};