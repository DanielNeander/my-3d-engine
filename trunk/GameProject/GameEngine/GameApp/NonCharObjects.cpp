#include "stdafx.h"
#include "NonCharObjects.h"
#include "Projectile.h"


Arrow::Arrow( const std::string& filename )
	:WowActor(filename)
{

}

void Arrow::Initialize()
{
	WowActor::Initialize();
	
	projectileAttrib_		 = new ProjectileAttributes;
	projectileAttrib_->m_damage              = 100.0f;
	projectileAttrib_->m_damageRadius        = 20.0f;
	projectileAttrib_->m_speed               = 150.0f;
	projectileAttrib_->m_explosionForce      = 100;
	projectileAttrib_->m_explosionRadius     = 20;
	projectileAttrib_->m_explosionEffectOffset = 0;
	AttachAttribute(projectileAttrib_);

	AttachComponent(new ProjectileHeatSeekingComponent(this));
	

}

