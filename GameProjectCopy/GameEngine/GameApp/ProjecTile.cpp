#include "stdafx.h"
#include "ProjecTile.h"
#include "GameObjectUtil.h"
#include "GameObjectManager.h"
#include "GameApp.h"


noImplementRTTI(ProjectileAttributes,GameObjectAttributeBase);
noImplementRTTI(ProjectileHeatSeekAttrib,GameObjectAttributeBase);
noImplementRTTI(ProjectileHeatSeekingTargetAttrib,GameObjectAttributeBase);

noImplementRTTI(ProjectileComponentBase,GameObjectComponentBase);
DEFINE_SIMPLE_STATE(STATE_FLYING,ProjectileComponentBase,"flying");
DEFINE_SIMPLE_STATE(STATE_EXPLODED,ProjectileComponentBase,"exploded");
ProjectileComponentBase::ProjectileComponentBase(GameObject *pkGameObject) 
	: GameObjectComponentBase(pkGameObject)
{
	m_pkAttrib = GetGameObjectAttribute(pkGameObject,ProjectileAttributes);
	TransitionState(&STATE_NONE);
}


bool ProjectileComponentBase::Initialize()
{
	m_pkAttrib = GetGameObjectAttribute(GetGameObject(),ProjectileAttributes);

	if( !m_pkAttrib )
		return false;

	return true;
}

void ProjectileComponentBase::Launch(const noVec3 &direction, const noVec3 &inheritedVelocity)
{
	noVec3    velocity;
	noVec3    inheritedVelocityDir = inheritedVelocity;

	velocity = direction * m_pkAttrib->m_speed;

	inheritedVelocityDir.NormalizeFast();

	if( direction * inheritedVelocityDir > 0 )
	{
		velocity.x    += inheritedVelocity.x;
		velocity.y    += inheritedVelocity.y;
	}

	noVec3    trueDirection = velocity;
	trueDirection.Normalize();

	noMat3   rot;
	MathHelpers::FaceDirection(rot,trueDirection);
	SetRotation( rot );

	GetGameObject()->SetVelocity( velocity );

	// attach our trail effect...
	//m_pkTrailEffect = SpawnGameObject<ProjectileTrailEffect>(m_pkAttrib->m_trailFactory, & GetTranslation() );

	/*if( m_pkTrailEffect )
	{
		noMat3   rotation = GetRotation();
		noMat3   rotateOffset = m_pkTrailEffect->GetRotation();
		m_pkTrailEffect->SetRotation( rotation * rotateOffset );
	}

	PlayLaunchSound();*/

	TransitionState(&STATE_FLYING,m_pkAttrib->m_lifetime);
}

bool ProjectileComponentBase::Update(float fDelta)
{
	GameObjectComponentBase::Update(fDelta);

	noVec3 forward = MathHelpers::GetForwardVectorFromMatrix(GetRotation());

	/*if( m_pkTrailEffect )
	{
		noVec3 pos = GetTranslation();

		pos -= forward * m_pkAttrib->m_trailOffset;

		m_pkTrailEffect->SetTranslation(pos);
	}*/

	if (m_state==&STATE_FLYING)
	{
		if( m_pkAttrib->m_lifetime>0 && GetTime() > m_stateTime )
		{
			//OnLifetimeExpired();
			return true;
		}
	}

	return true;
}

//bool ProjectileComponentBase::OnCollision( const GameObjectCollision &collision )
//{
//	if( m_state!=&STATE_FLYING )
//		return false;
//
//	m_pkCollisonObject = collision.GetGameObject();
//
//	if( m_pkCollisonObject && IsOwner(m_pkCollisonObject) )
//		return false;
//
//	GameObjectCollisionData     collisionData = collision.GetAt(0);
//
//	Explode( collisionData.m_collisionPoint, collisionData.m_collisionNormal );
//	return true;
//}

//void ProjectileComponentBase::OnLifetimeExpired()
//{
//	Explode( GetTranslation() );
//}
//
//void ProjectileComponentBase::Explode(const noVec3 &explosionPoint)
//{
//	TransitionState( &STATE_EXPLODED );
//
//	GameObjectManager   *pkGameObjectMan = GameObjectManager::Get();
//
//	SpawnGameObject<Explosion>(m_pkAttrib->m_explosionFactory, &explosionPoint);
//	if( m_pkTrailEffect )
//	{
//		ParticleSystemComponent *pkComponent = GetGameObjectComponent(m_pkTrailEffect,ParticleSystemComponent);
//		NIASSERT(pkComponent);
//
//		pkComponent->StopEmitter();
//		pkComponent->SetRemoveWhenDone( true );
//	}
//
//	PlayExplosionSound();
//	CauseDamage();
//
//	pkGameObjectMan->DestroyGameObject(GetGameObject());
//}

//void ProjectileComponentBase::Explode(const noVec3 &collisionPoint, const noVec3 &collisionNormal)
//{
//	noVec3 spawnPoint = collisionPoint + (collisionNormal * m_pkAttrib->m_explosionEffectOffset);
//
//	Explode( spawnPoint );
//}
//
//void ProjectileComponentBase::PlayLaunchSound()
//{
//	NIASSERT(m_pkAttrib);
//
//	if( m_pkAttrib->m_pkLaunchSound )
//		m_pkAttrib->m_pkLaunchSound->Play(GetTranslation());
//}
//
//void ProjectileComponentBase::PlayExplosionSound()
//{
//	NIASSERT(m_pkAttrib);
//
//	if( m_pkAttrib->m_pkExplosionSound )
//	{
//		m_pkAttrib->m_pkExplosionSound->Play( GetTranslation() );
//	}
//}

bool ProjectileComponentBase::IsOwner(GameObject *pkGameObject)
{
	if( !m_pkAttrib )
		return false;

	if( pkGameObject==m_pkAttrib->m_pkOwner )
		return true;

	return false;
}

void ProjectileComponentBase::CauseDamage()
{
	if( m_pkAttrib->m_damage<=0 )
		return;

	//if( m_pkCollisonObject )
	//{
	//	GameObjectManager::Get()->CauseDamage(GetGameObject(), m_pkAttrib->m_pkOwner, m_pkCollisonObject
	//		, GetTranslation(), m_pkAttrib->m_damage);
	//}

	/*GameObjectManager::Get()->CauseDamage(GetGameObject(), m_pkAttrib->m_pkOwner, GetTranslation()
		, m_pkAttrib->m_damageRadius, m_pkAttrib->m_damage,m_pkCollisonObject);*/
}


noImplementRTTI(ProjectileComponent,ProjectileComponentBase);
ProjectileComponent::ProjectileComponent(GameObject *pkGameObject) 
	: ProjectileComponentBase(pkGameObject)
{
	//m_pkTerrainInteractor       = NULL;
}

ProjectileComponent::~ProjectileComponent()
{
}

void ProjectileComponent::Launch(const noVec3 &direction, const noVec3 &inheritedVelocity)
{
	ProjectileComponentBase::Launch( direction, inheritedVelocity );

	noVec3    velocity = GetGameObject()->GetVelocity();

	//m_haveCollisionCourse = CollisionManager::CastRayDirection(GetTranslation(),MathHelpers::NormalizeVector(velocity)
	//	,&m_collisionPoint,&m_collisionNormal);

	//if( m_haveCollisionCourse )
	//{
	//	// we save this for placing our terrain decal...
	//	m_pkTerrainInteractor = CollisionManager::GetLastInteractor();
	//}
	noVec3 dir = -GetRotation().ToAngles().ToForward();

	switch(currentShotType)
	{
	case PISTOL:
		particle.setMass(2.0f); // 2.0kg
		particle.setVelocity(dir * 35.0f); // 35m/s
		particle.setAcceleration(0.0f, -1.0f, 0.0f);
		particle.setDamping(0.99f);
		break;

	case ARTILLERY:
		{		
			noVec3 vel = dir * 40.0f; 
			vel.y = 30.0f;
			particle.setMass(200.0f); // 200.0kg
			particle.setVelocity(vel); // 50m/s
			particle.setAcceleration(0.0f, -20.0f, 0.0f);
			particle.setDamping(0.99f);
		}
		break;

	case FIREBALL:
		particle.setMass(1.0f); // 1.0kg - mostly blast damage
		particle.setVelocity(dir * 10.0f); // 5m/s
		particle.setAcceleration(0.0f, 0.6f, 0.0f); // Floats up
		particle.setDamping(0.9f);
		break;

	case LASER:
		// Note that this is the kind of laser bolt seen in films,
		// not a realistic laser beam!
		particle.setMass(0.1f); // 0.1kg - almost no weight
		particle.setVelocity(dir * 100.0f); // 100m/s
		particle.setAcceleration(0.0f, 0.0f, 0.0f); // No gravity
		particle.setDamping(0.99f);
		break;
	}
	// Set the data common to all particle types
	noVec3 pos = GetTranslation();
	pos.y += 1.5f;
	particle.setPosition(pos);
	launchTime = GetApp()->globalTime;

	// Clear the force accumulators
	particle.clearAccumulator();
}

//////////////////////////////////////////////////////////////////////////
// This update function updates the translation of the trail effect,
// checks to see if the projectile has reached its collision point,
// and destroys the projectile if its lifetime has expired.
//////////////////////////////////////////////////////////////////////////
bool ProjectileComponent::Update(float fDelta)
{
	ProjectileComponentBase::Update(fDelta);

	particle.integrate(fDelta);

	if (particle.getPosition().y < 0.0f || 
		launchTime + 5000 < GetApp()->globalTime || 
		particle.getPosition().z > 200.f)
	{
		//Explode()
	}

	SetTranslation(particle.getPosition());

	//if( m_haveCollisionCourse )
	//{
	//	noVec3    collisionDir = MathHelpers::NormalizeVector(m_collisionPoint - GetTranslation());
	//	noVec3    velocityDir = MathHelpers::NormalizeVector(GetVelocity());

	//	if( collisionDir * velocityDir < 0 )
	//	{
	//		// we've passed the collision point
	//		GameObjectCollision collision(NULL,NULL);
	//		GameObjectCollisionData collisionData(m_collisionPoint,m_collisionNormal);
	//		collision.Add(collisionData);

	//		SetTranslation(m_collisionPoint);
	//		OnCollision(collision);
	//		return true;
	//	}
	//}
	// Set the properties of the particle


	return true;
}

//////////////////////////////////////////////////////////////////////////
// Spawn the explosion effect, destroys the projectile,
// and applies a decal to the terrain if requested
//////////////////////////////////////////////////////////////////////////
void ProjectileComponent::Explode(const noVec3 &collisionPoint, const noVec3 &collisionNormal)
{
	/*if( m_pkTerrainInteractor )
	{
		NiTerrainDecal* pkDecal = m_pkTerrainInteractor->CreateDecal(m_pkAttrib->m_spTerrainDecalTexture,
			collisionPoint, m_pkAttrib->m_decalSize, m_pkAttrib->m_decalRatio,
			GetTime() + m_pkAttrib->m_decalLifetime, m_pkAttrib->m_decalFadetime);

		pkDecal->SetFadingDistance(m_pkAttrib->m_decalFadingDistance);
	}
	ProjectileComponentBase::Explode(collisionPoint,collisionNormal);*/
}


// ---------------------------------------------------------------------
// ---------------------------------------------------------------------
noImplementRTTI(ProjectileHeatSeekingComponent,GameObjectComponentBase);
ProjectileHeatSeekingComponent::ProjectileHeatSeekingComponent(GameObject *pkGameObject)
	: GameObjectComponentBase(pkGameObject)
{
	m_kNextTargetCheck = 0;
}

bool ProjectileHeatSeekingComponent::Initialize()
{
	ProjectileHeatSeekAttrib    *pkAttrib = GetGameObjectAttribute(GetGameObject(),ProjectileHeatSeekAttrib);
	if( !pkAttrib )
		return false;

	if( !GetGameObjectAttribute(GetGameObject(),ProjectileAttributes) )
		return false;

	m_pkSlerpRotator = GetGameObjectComponent(GetGameObject(),GameObjectSlerpRotator);

	if( !m_pkSlerpRotator )
		return false;

	m_stateStartTime    = GetTime();

	return true;
}

bool ProjectileHeatSeekingComponent::Update(float fDelta)
{
	ProjectileHeatSeekAttrib    *pkAttrib = GetGameObjectAttribute(GetGameObject(),ProjectileHeatSeekAttrib);

	m_pkTarget = FindTarget();

	if( m_pkTarget )
	{
		noVec3    direction   = m_pkTarget->GetTranslation() - GetTranslation();
		direction.Normalize();
		m_pkSlerpRotator->FaceDirection( direction, pkAttrib->m_heatSeekTurnRate );
	}
	else
	{
		m_pkSlerpRotator->Stop();
	}

	// now just make sure we're moving in the direction we're facing....
	noVec3 forward = MathHelpers::GetForwardVectorFromMatrix(GetRotation());

	ProjectileAttributes    *pkProjAttrib = GetGameObjectAttribute(GetGameObject(),ProjectileAttributes);

	noVec3    velocity = forward * pkProjAttrib->m_speed;
	SetVelocity( velocity );

	return true;
}

GameObject *ProjectileHeatSeekingComponent::FindTarget()
{
	dbCompositionList    rawlist;
	GameObjectManager::Get()->GetGameObjectsWithAttribute(ProjectileHeatSeekingTargetAttrib::ms_RTTI,rawlist);

	dbCompositionList    targetList;

	ProjectileHeatSeekAttrib    *pkAttrib = GetGameObjectAttribute(GetGameObject(),ProjectileHeatSeekAttrib);
	assert(pkAttrib);

	if( GetTime() < m_stateStartTime + pkAttrib->m_waitTime )
	{
		return NULL;
	}

	if( GetTime() < m_kNextTargetCheck )
	{
		return m_pkTarget;
	}

	m_kNextTargetCheck      = GetTime() + pkAttrib->m_kTargetCheckInterval;

	noVec3    kForward = MathHelpers::GetForwardVectorFromMatrix(GetRotation());

	GameObject *pkGameObject = NULL;

	ProjectileAttributes    *pkProjAttribs = GetGameObjectAttribute(GetGameObject(),ProjectileAttributes);
	assert(pkProjAttribs);

	noVec3    dirToTarget;

	for (uint32 ui=0;ui<rawlist.size();ui++)
	{
		pkGameObject = rawlist.at(ui);

		if( pkGameObject==pkProjAttribs->m_pkOwner )
			continue;

		ProjectileHeatSeekingTargetAttrib   *pkTargetAttrib = GetGameObjectAttribute(pkGameObject,ProjectileHeatSeekingTargetAttrib);
		assert( pkTargetAttrib );

		if( !pkTargetAttrib->IsEnabled() )
			continue;

		dirToTarget = pkGameObject->GetTranslation() - GetTranslation();

		float   dist = dirToTarget.Length();
		dirToTarget.Normalize();

		if( dist > pkAttrib->m_heatSeekRadius )
			continue;

		float angle = dirToTarget * kForward;

		if( angle < pkAttrib->m_maxAngle )
			continue;

		// for now...
		return pkGameObject;
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// Since any game object can be a projectile, this utility function makes
// launching the projectile a bit easier.
//////////////////////////////////////////////////////////////////////////
void LaunchProjectile(GameObject *pkOwner, GameObject *pkMissile, const noVec3 &kFrom
	, const noMat3 &rotation, const noVec3 &inheritedVelocity)
{
	ProjectileAttributes    *pkProjectileAttributes = GetGameObjectAttribute(pkMissile,ProjectileAttributes);
	assert(pkProjectileAttributes);
	pkProjectileAttributes->m_pkOwner   = pkOwner;

	ProjectileComponentBase	*pkProjectileComponent = GetGameObjectComponent(pkMissile,ProjectileComponentBase);
	assert(pkProjectileComponent);

	pkMissile->SetTranslation( kFrom );
	pkMissile->SetRotation( rotation );
	
	pkProjectileComponent->Launch( MathHelpers::GetForwardVectorFromMatrix(rotation), inheritedVelocity );
}
