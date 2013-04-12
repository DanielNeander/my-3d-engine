#pragma once 

#include "TrailEffect.h"

class EffectSystem : public Singleton<EffectSystem>
{
public:
	~EffectSystem();

	bool LoadAll();

	void BeginEffect();
	void EndEffect();

	void SetCurrEffect(TrailMesh* pEffect);

	TrailMesh* CurrTrailMesh_;
	
	TrailMesh* Weapon1hTrail_;
};

