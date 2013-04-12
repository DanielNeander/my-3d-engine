#include "stdafx.h"
#include "GameApp/Mesh.h"
#include "EffectSystem.h"



bool EffectSystem::LoadAll()
{
	Weapon1hTrail_ = new TrailMesh();

	return true;
}

void EffectSystem::BeginEffect()
{

}

void EffectSystem::EndEffect()
{

}

void EffectSystem::SetCurrEffect( TrailMesh* pEffect )
{

}

EffectSystem::~EffectSystem()
{
	SAFE_DELETE(Weapon1hTrail_);
}
