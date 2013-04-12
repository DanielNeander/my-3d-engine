#pragma once 

#include "EngineCore/Singleton.h"

class CombatSystem : public Singleton<CombatSystem>
{
public:
	CombatSystem();
	void SetTarget(class GameObject* pTarget) { target_ = pTarget; }
		

	 GameObject* target_;
};