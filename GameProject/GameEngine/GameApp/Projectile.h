#pragma once 

#include "GameObject.h"
#include "GameObjectAttribute.h"
#include "GameObjectComponent.h"
//#include "SoundManager.h"
//#include "ParticleSystem.h"
#include "GameApp/Particle/cyclone_physics.h"
#include "GameObjectUtil.h"


class ProjectileAttributes : public GameObjectAttributeBase
{
public:
	noDeclareRTTI;

	float               m_lifetime;
	float               m_speed;

	//NiFixedString       m_trailFactory;
	float               m_trailOffset;

	//NiFixedString       m_explosionFactory;
	float               m_explosionForce;       // used by physX
	float               m_explosionRadius;
	float               m_explosionEffectOffset;    // how far to push the effect in the collision normal direction away from collision point


	//SoundFXBuffer       *m_pkLaunchSound;
	//SoundFXBuffer       *m_pkExplosionSound;

	//NiTexturePtr        m_spTerrainDecalTexture;
	float               m_decalLifetime;
	float               m_decalFadetime;
	float               m_decalFadingDistance;
	uint32	            m_decalSize;
	uint32		        m_decalRatio;
	GameObjectWPtr      m_pkOwner;

	float               m_damage;
	float               m_damageRadius;

	ProjectileAttributes()
	{
		m_lifetime                      = 10;
		m_speed                         = 80;
		m_trailOffset                   = 1.2f;
		m_decalFadetime                 = 8;
		m_decalLifetime                 = 10;
		m_decalFadingDistance           = 400;
		m_decalSize                     = 4;
		m_decalRatio                    = 1;
		//m_pkLaunchSound                 = NULL;
		//m_pkExplosionSound              = NULL;
		m_damage                        = 0;
		m_damageRadius                  = 0;
		m_explosionForce                = 0;
		m_explosionRadius               = 0;
		m_explosionEffectOffset         = 0;
	}

	~ProjectileAttributes()
	{
		//m_spTerrainDecalTexture = NULL;
	}
};



//////////////////////////////////////////////////////////////////////////
// this attribute should be attached to a heat-seeking projectile
// This attribute is required by the ProjectileHeatSeekingComponent
// component
//////////////////////////////////////////////////////////////////////////
class ProjectileHeatSeekAttrib : public GameObjectAttributeBase
{
public:
	noDeclareRTTI;
	float           m_heatSeekRadius;
	float           m_heatSeekTurnRate;
	float           m_maxAngle;
	float           m_waitTime;
	float           m_kTargetCheckInterval;

	ProjectileHeatSeekAttrib()
		: m_heatSeekRadius(100.0f)
		, m_heatSeekTurnRate(DEG2RAD(180.0f))
		, m_maxAngle(0.707f)
		, m_waitTime(0.5f)
		, m_kTargetCheckInterval(0.2f)
	{
	}
};

//////////////////////////////////////////////////////////////////////////
//
// Attach this attribute to game objects that are valid heat-seeking 
// targets for heat-seeking projectiles....
//
//////////////////////////////////////////////////////////////////////////
class ProjectileHeatSeekingTargetAttrib : public GameObjectAttributeBase
{   
public:
	noDeclareRTTI;
	float           m_heatRadius;
	bool            m_enabled;

	ProjectileHeatSeekingTargetAttrib()
	{
		m_heatRadius    = 100.0f;
		m_enabled       = true;
	}

	bool IsEnabled()
	{
		return m_enabled;
	}
};

//////////////////////////////////////////////////////////////////////////
// This component implements the heat-seeking capability of the projectile
//////////////////////////////////////////////////////////////////////////
class ProjectileHeatSeekingComponent : public GameObjectComponentBase
{
public:
	noDeclareRTTI;

	ProjectileHeatSeekingComponent(GameObject *pkGameObject);

	bool Initialize();
	bool Update(float fDelta);

protected:
	GameObjectWPtr                      m_pkTarget;
	class GameObjectSlerpRotator*       m_pkSlerpRotator;
	float                               m_kNextTargetCheck;

	GameObject *FindTarget();
};

// --------------------------------------------------------------------------------
// This component does the majority of the work for a projectile including:
// translation, rotation, trail effect attachment, and explosion
// 
// However, this base class does not handle collision.  Collision is left up to 
// a subclass or possibly another component. (so we can use different collision
// methods for with or without PhysX)
//
//  note: for similar reasons, this class does not do the traversing of the projectile either. 
//  This is left up to another component of type GameObjectMoverBase.
// --------------------------------------------------------------------------------
class ProjectileComponentBase : public GameObjectComponentBase
{
public:
	noDeclareRTTI;

	ProjectileComponentBase(GameObject *pkGameObject);

	virtual bool Initialize();
	virtual void Launch(const noVec3 &direction, const noVec3 &inheritedVelocity);
	virtual bool Update(float fDelta);

	//virtual bool OnCollision(const GameObjectCollision &collision);

protected:
	ProjectileAttributes*               m_pkAttrib;
	//GameObjectWPtr                      m_pkTrailEffect;
	//GameObjectWPtr                      m_pkCollisonObject; // who did we collide with?

	//virtual void OnLifetimeExpired();
	//virtual void Explode(const noVec3 &explosionPoint);
	//virtual void Explode(const noVec3 &collisionPoint, const noVec3 &collisionNormal);

	//virtual void PlayLaunchSound();
	//virtual void PlayExplosionSound();
	virtual void CauseDamage();

	virtual bool IsOwner(GameObject *pkGameObject);

};


// --------------------------------------------------------------------------------
// This component does the majority of the work for a projectile including:
// translation, rotation, trail effect attachment, explosion, and terrain marking
// 
// For efficiency sake, the projectile's impact point is pre-computed so
// we only need to do one collision check.
// --------------------------------------------------------------------------------
class ProjectileComponent : public ProjectileComponentBase
{
public:
	noDeclareRTTI;

	ProjectileComponent(GameObject *pkGameObject);
	~ProjectileComponent();

	virtual void Launch(const noVec3 &direction, const noVec3 &inheritedVelocity);
	virtual bool Update(float fDelta);
	
	enum ShotType
	{
		UNUSED = 0,
		PISTOL,
		ARTILLERY,
		FIREBALL,
		LASER
	};

	ShotType currentShotType;
protected:
	cyclone::Particle particle;
	unsigned int						launchTime;

	noVec3                            m_collisionPoint;
	noVec3                            m_collisionNormal;
	bool                                m_haveCollisionCourse;
	//class NiTerrainInteractor*          m_pkTerrainInteractor;      // needed for placement of a decal...

	virtual void Explode(const noVec3 &collisionPoint, const noVec3 &collisionNormal);

};

//////////////////////////////////////////////////////////////////////////
// Utility function for launching a projectile
//////////////////////////////////////////////////////////////////////////
void LaunchProjectile(GameObject *pkOwner, GameObject *pkMissile, const noVec3 &kFrom
	, const noMat3 &rotation, const noVec3 &inheritedVelocity);


