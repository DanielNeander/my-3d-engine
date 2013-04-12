#include "stdafx.h"
#include <GameApp/MD5/MD5Material.h>
#include <GameApp/MD5/MD5MapFile.h>
#include <GameApp/Collision/CollisionModel.h>
#include <GameApp/Collision/CollisionModel_local.h>
#include "GameApp/Physics/Physics.h"
#include "GameApp/Physics/Force.h"
#include "GameApp/Physics/Clip.h"
#include "MD5Model.h"
#include "GameApp/Animation/MD5Anim.h"
#include "GameApp/Animation/MD5IK.h"
#include <GameApp/MD5/MD5Game.h>
#include "GameApp/Physics/Physics_Base.h"
#include "GameApp/Physics/Physics_Actor.h"
#include "GameApp/Physics/Physics_Parametric.h"
#include <GameApp/Physics/Physics_AF.h>
#include <GameApp/MD5/DeclAF.h>
#include <GameApp/MD5/MD5Af.h>
#include "MD5Entity.h"
#include "MD5AFEntity.h"
#include "MD5Actor.h"
#include "MD5ModelManager.h"
#include "MD5Misc.h"

ABSTRACT_DECLARATION( idClass, idEntity )
END_CLASS

static void ParseSpawnArgsToRenderEntity( const idDict *args, renderEntity_t *renderEntity );


void idEntity::SetPhysics( idPhysics *phys )
{
	// clear any contacts the current physics object has
	if ( physics ) {
		physics->ClearContacts();
	}
	// set new physics object or set the default physics if NULL
	if ( phys != NULL ) {
		//defaultPhysicsObj.SetClipModel( NULL, 1.0f );
		physics = phys;
		physics->Activate();
	} else {
		//physics = &defaultPhysicsObj;
	}
	physics->UpdateTime( gameLocal.time );
	physics->SetMaster( bindMaster, fl.bindOrientated );
}

/*
================
idEntity::ActivatePhysics
================
*/
void idEntity::ActivatePhysics( idEntity *ent ) {
	GetPhysics()->Activate();
}

idEntity::idEntity() {

		entityNumber	= ENTITYNUM_NONE;
		entityDefNumber = -1;

		spawnNode.SetOwner( this );
		activeNode.SetOwner( this );

		/*	snapshotNode.SetOwner( this );
		snapshotSequence = -1;
		snapshotBits = 0;
		*/
		thinkFlags		= 0;
		//dormantStart	= 0;
		//cinematic		= false;
		//renderView		= NULL;
		//cameraTarget	= NULL;
		health			= 0;

		physics			= NULL;
		bindMaster		= NULL;
		bindJoint		= INVALID_JOINT;
		bindBody		= -1;
		teamMaster		= NULL;
		teamChain		= NULL;
//		signals			= NULL;

		memset( PVSAreas, 0, sizeof( PVSAreas ) );
		numPVSAreas		= -1;

		memset( &fl, 0, sizeof( fl ) );
		fl.neverDormant	= true;			// most entities never go dormant

		memset( &renderEntity, 0, sizeof( renderEntity ) );
		modelDefHandle	= -1;
}


idEntity::~idEntity()
{
	if ( thinkFlags ) {
		BecomeInactive( thinkFlags );
	}
	activeNode.Remove();

	//Signal( SIG_REMOVED );

	// we have to set back the default physics object before unbinding because the entity
	// specific physics object might be an entity variable and as such could already be destroyed.
	SetPhysics( NULL );
	gameLocal.RemoveEntityFromHash( name.c_str(), this );
	gameLocal.UnregisterEntity( this );
}


void idEntity::Spawn( void )
{
	int					i;
	const char			*temp;
	noVec3				origin;
	noMat3				axis;
	const idKeyValue	*networkSync;
	const char			*classname;
	const char			*scriptObjectName;

	gameLocal.RegisterEntity( this );

	spawnArgs.GetString( "classname", NULL, &classname );
	const idDeclEntityDef *def = gameLocal.FindEntityDef( classname, false );
	if ( def ) {
		entityDefNumber = def->Index();
	}
	ParseSpawnArgsToRenderEntity( &spawnArgs, &renderEntity );
	renderEntity.entityNum = entityNumber;
	origin = renderEntity.origin;
	axis = renderEntity.axis;

	// every object will have a unique name
	temp = spawnArgs.GetString( "name", va( "%s_%s_%d", GetClassname(), spawnArgs.GetString( "classname" ), entityNumber ) );
	SetName( temp );

	health = spawnArgs.GetInt( "health" );
	InitDefaultPhysics( origin, axis );
	SetOrigin( origin );
	SetAxis( axis );

	temp = spawnArgs.GetString( "model" );
	if ( temp && *temp ) {
		SetModel( temp );
	}		
}



void idEntity::SetName( const char *_name )
{
	name = _name;
}

const char * idEntity::GetName( void ) const
{
	return name.c_str();

}

/*
================
idEntity::Collide
================
*/
bool idEntity::Collide( const trace_t &collision, const noVec3 &velocity ) {
	// this entity collides with collision.c.entityNum
	return false;
}

/*
================
idEntity::GetImpactInfo
================
*/
void idEntity::GetImpactInfo( idEntity *ent, int id, const noVec3 &point, impactInfo_t *info ) {
	GetPhysics()->GetImpactInfo( id, point, info );
}

idPhysics * idEntity::GetPhysics( void ) const
{
	return physics;

}

bool idEntity::RunPhysics( void )
{
	int			i, reachedTime, startTime, endTime;
	idEntity *	part, *blockedPart, *blockingEntity;
	trace_t		results;
	bool		moved;

	// don't run physics if not enabled
	if ( !( thinkFlags & TH_PHYSICS ) ) {
		// however do update any animation controllers
		if ( UpdateAnimationControllers() ) {
			BecomeActive( TH_ANIMATE );
		}
		return false;
	}

	// if this entity is a team slave don't do anything because the team master will handle everything
	if ( teamMaster && teamMaster != this ) {
		return false;
	}

	startTime = gameLocal.previousTime;
	endTime = gameLocal.time;

	gameLocal.push.InitSavingPushedEntityPositions();
	blockedPart = NULL;

	// save the physics state of the whole team and disable the team for collision detection
	for ( part = this; part != NULL; part = part->teamChain ) {
		if ( part->physics ) {
			if ( !part->fl.solidForTeam ) {
				part->physics->DisableClip();
			}
			part->physics->SaveState();
		}
	}

	// move the whole team
	for ( part = this; part != NULL; part = part->teamChain ) {

		if ( part->physics ) {

			// run physics
			moved = part->physics->Evaluate( endTime - startTime, endTime );

			// check if the object is blocked
			blockingEntity = part->physics->GetBlockingEntity();
			if ( blockingEntity ) {
				blockedPart = part;
				break;
			}

			// if moved or forced to update the visual position and orientation from the physics
			if ( moved || part->fl.forcePhysicsUpdate ) {
				part->UpdateFromPhysics( false );
			}

			// update any animation controllers here so an entity bound
			// to a joint of this entity gets the correct position
			if ( part->UpdateAnimationControllers() ) {
				part->BecomeActive( TH_ANIMATE );
			}
		}
	}

	// enable the whole team for collision detection
	for ( part = this; part != NULL; part = part->teamChain ) {
		if ( part->physics ) {
			if ( !part->fl.solidForTeam ) {
				part->physics->EnableClip();
			}
		}
	}

	// if one of the team entities is a pusher and blocked
	if ( blockedPart ) {
		// move the parts back to the previous position
		for ( part = this; part != blockedPart; part = part->teamChain ) {

			if ( part->physics ) {

				// restore the physics state
				part->physics->RestoreState();

				// move back the visual position and orientation
				part->UpdateFromPhysics( true );
			}
		}
		for ( part = this; part != NULL; part = part->teamChain ) {
			if ( part->physics ) {
				// update the physics time without moving
				part->physics->UpdateTime( endTime );
			}
		}

		// restore the positions of any pushed entities
		gameLocal.push.RestorePushedEntityPositions();

		/*if ( gameLocal.isClient ) {
			return false;
		}*/

		// if the master pusher has a "blocked" function, call it
		//Signal( SIG_BLOCKED );
		//ProcessEvent( &EV_TeamBlocked, blockedPart, blockingEntity );
		// call the blocked function on the blocked part
		//blockedPart->ProcessEvent( &EV_PartBlocked, blockingEntity );
		return false;
	}

	// set pushed
	for ( i = 0; i < gameLocal.push.GetNumPushedEntities(); i++ ) {
		idEntity *ent = gameLocal.push.GetPushedEntity( i );
		ent->physics->SetPushed( endTime - startTime );
	}

	/*if ( gameLocal.isClient ) {
		return true;
	}*/

	// post reached event if the current time is at or past the end point of the motion
	/*for ( part = this; part != NULL; part = part->teamChain ) {

	if ( part->physics ) {

	reachedTime = part->physics->GetLinearEndTime();
	if ( startTime < reachedTime && endTime >= reachedTime ) {
	part->ProcessEvent( &EV_ReachedPos );
	}
	reachedTime = part->physics->GetAngularEndTime();
	if ( startTime < reachedTime && endTime >= reachedTime ) {
	part->ProcessEvent( &EV_ReachedAng );
	}
	}
	}*/

	return true;
}

/*
================
idEntity::UpdateFromPhysics
================
*/
void idEntity::UpdateFromPhysics( bool moveBack ) {

	if ( IsType( idActor::Type ) ) {
		idActor *actor = static_cast<idActor *>( this );

		// set master delta angles for actors
		if ( bindMaster ) {
			noAngles delta = actor->GetDeltaViewAngles();
			if ( moveBack ) {
				delta.yaw -= static_cast<idPhysics_Actor *>(physics)->GetMasterDeltaYaw();
			} else {
				delta.yaw += static_cast<idPhysics_Actor *>(physics)->GetMasterDeltaYaw();
			}
			actor->SetDeltaViewAngles( delta );
		}
	}

	//UpdateVisuals();
}

void idEntity::ConvertLocalToWorldTransform( noVec3 &offset, noMat3 &axis ) {
	UpdateModelTransform();

	offset = renderEntity.origin + offset * renderEntity.axis;
	axis *= renderEntity.axis;
}

/*
================
idEntity::AddContactEntity
================
*/
void idEntity::AddContactEntity( idEntity *ent ) {
	GetPhysics()->AddContactEntity( ent );
}

/*
================
idEntity::RemoveContactEntity
================
*/
void idEntity::RemoveContactEntity( idEntity *ent ) {
	GetPhysics()->RemoveContactEntity( ent );
}

void idEntity::BecomeActive( int flags ) {
	if ( ( flags & TH_PHYSICS ) ) {
		// enable the team master if this entity is part of a physics team
		if ( teamMaster && teamMaster != this ) {
			teamMaster->BecomeActive( TH_PHYSICS );
		} else if ( !( thinkFlags & TH_PHYSICS ) ) {
			// if this is a pusher
			if ( physics->IsType( idPhysics_Parametric::Type ) || physics->IsType( idPhysics_Actor::Type ) ) {
				gameLocal.sortPushers = true;
			}
		}		
	}

	int oldFlags = thinkFlags;
	thinkFlags |= flags;
	if ( thinkFlags ) {
		if ( !IsActive() ) {
			activeNode.AddToEnd( gameLocal.activeEntities );
		} else if ( !oldFlags ) {
			// we became inactive this frame, so we have to decrease the count of entities to deactivate
			gameLocal.numEntitiesToDeactivate--;
		}
	}
}

/*
================
idEntity::BecomeInactive
================
*/
void idEntity::BecomeInactive( int flags ) {
	if ( ( flags & TH_PHYSICS ) ) {
		// may only disable physics on a team master if no team members are running physics or bound to a joints
		if ( teamMaster == this ) {
			for ( idEntity *ent = teamMaster->teamChain; ent; ent = ent->teamChain ) {
				if ( ( ent->thinkFlags & TH_PHYSICS ) || ( ( ent->bindMaster == this ) && ( ent->bindJoint != INVALID_JOINT ) ) ) {
					flags &= ~TH_PHYSICS;
					break;
				}
			}
		}
	}

	if ( thinkFlags ) {
		thinkFlags &= ~flags;
		if ( !thinkFlags && IsActive() ) {
			gameLocal.numEntitiesToDeactivate++;
		}
	}

	if ( ( flags & TH_PHYSICS ) ) {
		// if this entity has a team master
		if ( teamMaster && teamMaster != this ) {
			// if the team master is at rest
			if ( teamMaster->IsAtRest() ) {
				teamMaster->BecomeInactive( TH_PHYSICS );
			}
		}
	}
}

/*
================
idEntity::IsActive
================
*/
bool idEntity::IsActive( void ) const {
	return activeNode.InList();
}




void idEntity::ApplyImpulse( idEntity *ent, int id, const noVec3 &point, const noVec3 &impulse )
{
	GetPhysics()->ApplyImpulse( id, point, impulse );

}

/*
================
idEntity::AddForce
================
*/
void idEntity::AddForce( idEntity *ent, int id, const noVec3 &point, const noVec3 &force ) {
	GetPhysics()->AddForce( id, point, force );
}

idAnimator * idEntity::GetAnimator( void )
{
	return NULL;
}

bool idEntity::IsAtRest( void ) const
{
	return GetPhysics()->IsAtRest();
}

bool idEntity::CanDamage( const noVec3 &origin, noVec3 &damagePoint ) const
{
	return true;
}

void idEntity::Damage( idEntity *inflictor, idEntity *attacker, const noVec3 &dir, const char *damageDefName, const float damageScale, const int location )
{
	if ( !fl.takedamage ) {
		return;
	}

#ifdef _D3XP
	SetTimeState ts( timeGroup );
#endif

	if ( !inflictor ) {
		inflictor = gameLocal.world;
	}

	if ( !attacker ) {
		attacker = gameLocal.world;
	}

	const idDict *damageDef = gameLocal.FindEntityDefDict( damageDefName );
	if ( !damageDef ) {
		gameLocal.Error( "Unknown damageDef '%s'\n", damageDefName );
	}

	int	damage = damageDef->GetInt( "damage" );

	// inform the attacker that they hit someone
	attacker->DamageFeedback( this, inflictor, damage );
	if ( damage ) {
		// do the damage
		health -= damage;
		if ( health <= 0 ) {
			if ( health < -999 ) {
				health = -999;
			}

			Killed( inflictor, attacker, damage, dir, location );
		} else {
			Pain( inflictor, attacker, damage, dir, location );
		}
	}
}

void idEntity::AddDamageEffect( const trace_t &collision, const noVec3 &velocity, const char *damageDefName )
{

}

void idEntity::DamageFeedback( idEntity *victim, idEntity *inflictor, int &damage )
{

}

bool idEntity::Pain( idEntity *inflictor, idEntity *attacker, int damage, const noVec3 &dir, int location )
{
	return true;
}

void idEntity::Killed( idEntity *inflictor, idEntity *attacker, int damage, const noVec3 &dir, int location )
{

}

/*
================
idEntity::GetPhysicsToVisualTransform
================
*/
bool idEntity::GetPhysicsToVisualTransform( noVec3 &origin, noMat3 &axis ) {
	return false;
}


/*
================
idEntity::UpdateModelTransform
================
*/
void idEntity::UpdateModelTransform( void ) {
	noVec3 origin;
	noMat3 axis;

	if ( GetPhysicsToVisualTransform( origin, axis ) ) {
		renderEntity.axis = axis * GetPhysics()->GetAxis();
		renderEntity.origin = GetPhysics()->GetOrigin() + origin * renderEntity.axis;
	} else {
		renderEntity.axis = GetPhysics()->GetAxis();
		renderEntity.origin = GetPhysics()->GetOrigin();
	}
}

renderEntity_t * idEntity::GetRenderEntity( void )
{
	return &renderEntity;

}

int idEntity::GetModelDefHandle( void )
{
	return modelDefHandle;

}

void idEntity::SetModel( const char *modelname )
{
	assert( modelname );

	//FreeModelDef();

	renderEntity.hModel = renderModelManager->FindModel( modelname );

	if ( renderEntity.hModel ) {
		//renderEntity.hModel->Reset();
	}

	renderEntity.callback = NULL;
	renderEntity.numJoints = 0;
	renderEntity.joints = NULL;
	if ( renderEntity.hModel ) {
		renderEntity.bounds = renderEntity.hModel->Bounds( &renderEntity );
	} else {
		renderEntity.bounds.Zero();
	}

	//UpdateVisuals();
}

/*
================
idEntity::UpdateRenderEntity
================
*/
bool idEntity::UpdateRenderEntity( renderEntity_s *renderEntity, const renderView_t *renderView ) {
	/*if ( gameLocal.inCinematic && gameLocal.skipCinematic ) {
		return false;
	}*/

	idAnimator *animator = GetAnimator();
	if ( animator ) {
#ifdef _D3XP
		SetTimeState ts( timeGroup );
#endif

		return animator->CreateFrame( gameLocal.time, false );
	}

	return false;
}

bool idEntity::IsHidden( void ) const
{
	return fl.hidden;
}

void idEntity::Think( void )
{
	RunPhysics();
	Present();
}

bool idEntity::ModelCallback( renderEntity_s *renderEntity, const renderView_t *renderView ) {

	idEntity *ent;

		ent = gameLocal.entities[ renderEntity->entityNum ];
		if ( !ent ) {
			gameLocal.Error( "idEntity::ModelCallback: callback with NULL game entity" );
		}

		return ent->UpdateRenderEntity( renderEntity, renderView );

}

bool idEntity::UpdateAnimationControllers( void )
{
	return false;
}

const idEventDef EV_GetJointHandle( "getJointHandle", "s", 'd' );
const idEventDef EV_ClearAllJoints( "clearAllJoints" );
const idEventDef EV_ClearJoint( "clearJoint", "d" );
const idEventDef EV_SetJointPos( "setJointPos", "ddv" );
const idEventDef EV_SetJointAngle( "setJointAngle", "ddv" );
const idEventDef EV_GetJointPos( "getJointPos", "d", 'v' );
const idEventDef EV_GetJointAngle( "getJointAngle", "d", 'v' );

CLASS_DECLARATION( idEntity, idAnimatedEntity )
	EVENT( EV_GetJointHandle,		idAnimatedEntity::Event_GetJointHandle )
	EVENT( EV_ClearAllJoints,		idAnimatedEntity::Event_ClearAllJoints )
	EVENT( EV_ClearJoint,			idAnimatedEntity::Event_ClearJoint )
	EVENT( EV_SetJointPos,			idAnimatedEntity::Event_SetJointPos )
	EVENT( EV_SetJointAngle,		idAnimatedEntity::Event_SetJointAngle )
	EVENT( EV_GetJointPos,			idAnimatedEntity::Event_GetJointPos )
	EVENT( EV_GetJointAngle,		idAnimatedEntity::Event_GetJointAngle )
END_CLASS


idAnimatedEntity::idAnimatedEntity()
{
	animator.SetEntity( this );
	damageEffects = NULL;
}


idAnimatedEntity::~idAnimatedEntity()
{
	damageEffect_t	*de;

	for ( de = damageEffects; de; de = damageEffects ) {
		damageEffects = de->next;
		delete de;
	}
}

void idAnimatedEntity::ClientPredictionThink( void )
{

}

/*
================
idEntity::UpdateVisuals
================
*/
void idEntity::UpdateVisuals( void ) {
	UpdateModel();
	//UpdateSound();
}


void idAnimatedEntity::Think( void )
{
	RunPhysics();
	UpdateAnimation();
	Present();
	//UpdateDamageEffects();
}

void idAnimatedEntity::UpdateAnimation( void )
{
	// don't do animations if they're not enabled
	if ( !( thinkFlags & TH_ANIMATE ) ) {
		return;
	}

	// is the model an MD5?
	if ( !animator.ModelHandle() ) {
		// no, so nothing to do
		return;
	}

	// call any frame commands that have happened in the past frame
	if ( !fl.hidden ) {
		animator.ServiceAnims( gameLocal.previousTime, gameLocal.time );
	}

	// if the model is animating then we have to update it
	if ( !animator.FrameHasChanged( gameLocal.time ) ) {
		// still fine the way it was
		return;
	}

	// get the latest frame bounds
	animator.GetBounds( gameLocal.time, renderEntity.bounds );
	if ( renderEntity.bounds.IsCleared() && !fl.hidden ) {
		//gameLocal.DPrintf( "%d: inside out bounds\n", gameLocal.time );
	}

	// update the renderEntity
	UpdateVisuals();

	// the animation is updated
	animator.ClearForceUpdate();
}

idAnimator * idAnimatedEntity::GetAnimator( void )
{
	return &animator;

}

void idAnimatedEntity::SetModel( const char *modelname )
{
	//FreeModelDef();

	renderEntity.hModel = animator.SetModel( modelname );
	if ( !renderEntity.hModel ) {
		idEntity::SetModel( modelname );
		return;
	}

	/*if ( !renderEntity.customSkin ) {
	renderEntity.customSkin = animator.ModelDef()->GetDefaultSkin();
	}*/

	// set the callback to update the joints
	renderEntity.callback = idEntity::ModelCallback;
	animator.GetJoints( &renderEntity.numJoints, &renderEntity.joints );
	animator.GetBounds( gameLocal.time, renderEntity.bounds );

	//UdateVisuals();
}

/*
=====================
idAnimatedEntity::GetJointWorldTransform
=====================
*/
bool idAnimatedEntity::GetJointWorldTransform( jointHandle_t jointHandle, int currentTime, noVec3 &offset, noMat3 &axis ) {
	if ( !animator.GetJointTransform( jointHandle, currentTime, offset, axis ) ) {
		return false;
	}

	ConvertLocalToWorldTransform( offset, axis );
	return true;
}


bool idAnimatedEntity::GetJointTransformForAnim( jointHandle_t jointHandle, int animNum, int frameTime, noVec3 &offset, noMat3 &axis ) const
{
	const idAnim	*anim;
	int				numJoints;
	idJointMat		*frame;

	anim = animator.GetAnim( animNum );
	if ( !anim ) {
		assert( 0 );
		return false;
	}

	numJoints = animator.NumJoints();
	if ( ( jointHandle < 0 ) || ( jointHandle >= numJoints ) ) {
		assert( 0 );
		return false;
	}

	frame = ( idJointMat * )_alloca16( numJoints * sizeof( idJointMat ) );
	::ANIM_CreateAnimFrame( animator.ModelHandle(), anim->MD5Anim( 0 ), renderEntity.numJoints, frame, frameTime, animator.ModelDef()->GetVisualOffset(), animator.RemoveOrigin() );

	offset = frame[ jointHandle ].ToVec3();
	axis = frame[ jointHandle ].ToMat3();

	return true;
}

int idAnimatedEntity::GetDefaultSurfaceType( void ) const
{
	return SURFTYPE_METAL;
}

void idAnimatedEntity::AddDamageEffect( const trace_t &collision, const noVec3 &velocity, const char *damageDefName )
{

}

void idAnimatedEntity::AddLocalDamageEffect( jointHandle_t jointNum, const noVec3 &localPoint, const noVec3 &localNormal, const noVec3 &localDir, const idDeclEntityDef *def, const idMaterial *collisionMaterial )
{

}

void idAnimatedEntity::UpdateDamageEffects( void )
{

}

bool idAnimatedEntity::ClientReceiveEvent( int event, int time, const idBitMsg &msg )
{
	return false;
}

/*
================
idAnimatedEntity::Event_GetJointHandle

looks up the number of the specified joint.  returns INVALID_JOINT if the joint is not found.
================
*/
void idAnimatedEntity::Event_GetJointHandle( const char *jointname )
{
	jointHandle_t joint;

	joint = animator.GetJointHandle( jointname );
	//idThread::ReturnInt( joint );
}

void idAnimatedEntity::Event_ClearAllJoints( void )
{
	animator.ClearAllJoints();

}

void idAnimatedEntity::Event_ClearJoint( jointHandle_t jointnum )
{
	animator.ClearJoint( jointnum );

}

void idAnimatedEntity::Event_SetJointPos( jointHandle_t jointnum, jointModTransform_t transform_type, const noVec3 &pos )
{
	animator.SetJointPos( jointnum, transform_type, pos );

}

void idAnimatedEntity::Event_SetJointAngle( jointHandle_t jointnum, jointModTransform_t transform_type, const noAngles &angles )
{
	noMat3 mat;

	mat = angles.ToMat3();
	animator.SetJointAxis( jointnum, transform_type, mat );
}

void idAnimatedEntity::Event_GetJointPos( jointHandle_t jointnum )
{
	/*noVec3 offset;
	noMat3 axis;

	if ( !GetJointWorldTransform( jointnum, gameLocal.time, offset, axis ) ) {
	gameLocal.Warning( "Joint # %d out of range on entity '%s'",  jointnum, name.c_str() );
	}

	idThread::ReturnVector( offset );*/
}

void idAnimatedEntity::Event_GetJointAngle( jointHandle_t jointnum )
{	
	//idThread::ReturnVector( offset );
}

void idEntity::InitDefaultPhysics( const noVec3 &origin, const noMat3 &axis ) {
	const char *temp;
	idClipModel *clipModel = NULL;

	// check if a clipmodel key/value pair is set
	if ( spawnArgs.GetString( "clipmodel", "", &temp ) ) {
		if ( idClipModel::CheckModel( temp ) ) {
			clipModel = new idClipModel( temp );
		}
	}

	if ( !spawnArgs.GetBool( "noclipmodel", "0" ) ) {

		// check if mins/maxs or size key/value pairs are set
		if ( !clipModel ) {
			noVec3 size;
			idBounds bounds;
			bool setClipModel = false;

			if ( spawnArgs.GetVector( "mins", NULL, bounds[0] ) &&
				spawnArgs.GetVector( "maxs", NULL, bounds[1] ) ) {
					setClipModel = true;
					if ( bounds[0][0] > bounds[1][0] || bounds[0][1] > bounds[1][1] || bounds[0][2] > bounds[1][2] ) {
						//gameLocal.Error( "Invalid bounds '%s'-'%s' on entity '%s'", bounds[0].ToString(), bounds[1].ToString(), name.c_str() );
					}
			} else if ( spawnArgs.GetVector( "size", NULL, size ) ) {
				if ( ( size.x < 0.0f ) || ( size.y < 0.0f ) || ( size.z < 0.0f ) ) {
					//gameLocal.Error( "Invalid size '%s' on entity '%s'", size.ToString(), name.c_str() );
				}
				bounds[0].Set( size.x * -0.5f, size.y * -0.5f, 0.0f );
				bounds[1].Set( size.x * 0.5f, size.y * 0.5f, size.z );
				setClipModel = true;
			}

			if ( setClipModel ) {
				int numSides;
				idTraceModel trm;

				if ( spawnArgs.GetInt( "cylinder", "0", numSides ) && numSides > 0 ) {
					trm.SetupCylinder( bounds, numSides < 3 ? 3 : numSides );
				} else if ( spawnArgs.GetInt( "cone", "0", numSides ) && numSides > 0 ) {
					trm.SetupCone( bounds, numSides < 3 ? 3 : numSides );
				} else {
					trm.SetupBox( bounds );
				}
				clipModel = new idClipModel( trm );
			}
		}

		// check if the visual model can be used as collision model
		if ( !clipModel ) {
			temp = spawnArgs.GetString( "model" );
			if ( ( temp != NULL ) && ( *temp != 0 ) ) {
				if ( idClipModel::CheckModel( temp ) ) {
					clipModel = new idClipModel( temp );
				}
			}
		}
	}

	defaultPhysicsObj.SetSelf( this );
	defaultPhysicsObj.SetClipModel( clipModel, 1.0f );
	defaultPhysicsObj.SetOrigin( origin );
	defaultPhysicsObj.SetAxis( axis );

	physics = &defaultPhysicsObj;
}

void idEntity::SetOrigin( const noVec3 &org )
{
	GetPhysics()->SetOrigin( org );

}

void idEntity::SetAxis( const noMat3 &axis )
{
	if ( GetPhysics()->IsType( idPhysics_Actor::Type ) ) {
		static_cast<idActor *>(this)->viewAxis = axis;
	} else {
		GetPhysics()->SetAxis( axis );
	}
}

void idEntity::SetAngles( const noAngles &ang )
{
	SetAxis( ang.ToMat3() );

}

void idEntity::UpdateModel( void ) {
#ifdef _D3XP
	renderEntity.timeGroup = timeGroup;
#endif

	UpdateModelTransform();

	// check if the entity has an MD5 model
	idAnimator *animator = GetAnimator();
	if ( animator && animator->ModelHandle() ) {
		// set the callback to update the joints
		renderEntity.callback = idEntity::ModelCallback;
	}

	// set to invalid number to force an update the next time the PVS areas are retrieved
	//ClearPVSAreas();

	// ensure that we call Present this frame
	BecomeActive( TH_UPDATEVISUALS );

#ifdef _D3XP
	// If the entity has an xray skin, go ahead and add it
	if ( xraySkin != NULL ) {
		xrayEntity = renderEntity;
		xrayEntity.xrayIndex = 2;
		xrayEntity.customSkin = xraySkin;

		if ( xrayEntityHandle == -1 ) {
			xrayEntityHandle = gameRenderWorld->AddEntityDef( &xrayEntity );
		} else {
			gameRenderWorld->UpdateEntityDef( xrayEntityHandle, &xrayEntity );
		}
	}
#endif
}

/*
================
idEntity::Present

Present is called to allow entities to generate refEntities, lights, etc for the renderer.
================
*/
void idEntity::Present( void ) {

	/*if ( !gameLocal.isNewFrame ) {
	return;
	}*/

	// don't present to the renderer if the entity hasn't changed
	if ( !( thinkFlags & TH_UPDATEVISUALS ) ) {
		return;
	}
	BecomeInactive( TH_UPDATEVISUALS );

	// camera target for remote render views
	/*if ( cameraTarget && gameLocal.InPlayerPVS( this ) ) {
		renderEntity.remoteRenderView = cameraTarget->GetRenderView();
	}*/

	// if set to invisible, skip
	if ( !renderEntity.hModel || IsHidden() ) {
		return;
	}

	//// add to refresh list
	//if ( modelDefHandle == -1 ) {
	//	modelDefHandle = gameRenderWorld->AddEntityDef( &renderEntity );
	//} else {
	//	gameRenderWorld->UpdateEntityDef( modelDefHandle, &renderEntity );
	//}
	if (renderEntity.callback)
	{
		noMat4 modelMat;
		R_AxisToModelMatrix( renderEntity.axis, renderEntity.origin,  modelMat.ToFloatPtr());

		bool update;
		/*if(tr.viewDef)
			update = renderEntity.callback(&renderEntity, &tr.viewDef->renderView );
		else */
			update = renderEntity.callback(&renderEntity, NULL );

	}	
}

void ParseSpawnArgsToRenderEntity( const idDict *args, renderEntity_t *renderEntity ) {
	int			i;
	const char	*temp;
	noVec3		color;
	float		angle;
	const idDeclModelDef *modelDef;

	memset( renderEntity, 0, sizeof( *renderEntity ) );

	temp = args->GetString( "model" );

	modelDef = NULL;
	if ( temp[0] != '\0' ) {
		modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, temp, false ) );
		if ( modelDef ) {
			renderEntity->hModel = modelDef->ModelHandle();
		}
		if ( !renderEntity->hModel ) {
			renderEntity->hModel = renderModelManager->FindModel( temp );
		}
	}
	if ( renderEntity->hModel ) {
		renderEntity->bounds = renderEntity->hModel->Bounds( renderEntity );
	} else {
		renderEntity->bounds.Zero();
	}

	temp = args->GetString( "skin" );
	if ( temp[0] != '\0' ) {
		renderEntity->customSkin = declManager->FindSkin( temp );
	} else if ( modelDef ) {
		renderEntity->customSkin = modelDef->GetDefaultSkin();
	}

	temp = args->GetString( "shader" );
	if ( temp[0] != '\0' ) {
		renderEntity->customShader = declManager->FindMaterial( temp );
	}

	args->GetVector( "origin", "0 0 0", renderEntity->origin );

	// get the rotation matrix in either full form, or single angle form
	if ( !args->GetMatrix( "rotation", "1 0 0 0 1 0 0 0 1", renderEntity->axis ) ) {
		angle = args->GetFloat( "angle" );
		if ( angle != 0.0f ) {
			renderEntity->axis = noAngles( 0.0f, angle, 0.0f ).ToMat3();
		} else {
			renderEntity->axis.Identity();
		}
	}

	//renderEntity->referenceSound = NULL;

	// get shader parms
	args->GetVector( "_color", "1 1 1", color );
	renderEntity->shaderParms[ SHADERPARM_RED ]		= color[0];
	renderEntity->shaderParms[ SHADERPARM_GREEN ]	= color[1];
	renderEntity->shaderParms[ SHADERPARM_BLUE ]	= color[2];
	renderEntity->shaderParms[ 3 ]					= args->GetFloat( "shaderParm3", "1" );
	renderEntity->shaderParms[ 4 ]					= args->GetFloat( "shaderParm4", "0" );
	renderEntity->shaderParms[ 5 ]					= args->GetFloat( "shaderParm5", "0" );
	renderEntity->shaderParms[ 6 ]					= args->GetFloat( "shaderParm6", "0" );
	renderEntity->shaderParms[ 7 ]					= args->GetFloat( "shaderParm7", "0" );
	renderEntity->shaderParms[ 8 ]					= args->GetFloat( "shaderParm8", "0" );
	renderEntity->shaderParms[ 9 ]					= args->GetFloat( "shaderParm9", "0" );
	renderEntity->shaderParms[ 10 ]					= args->GetFloat( "shaderParm10", "0" );
	renderEntity->shaderParms[ 11 ]					= args->GetFloat( "shaderParm11", "0" );

	// check noDynamicInteractions flag
	renderEntity->noDynamicInteractions = args->GetBool( "noDynamicInteractions" );

	// check noshadows flag
	renderEntity->noShadow = args->GetBool( "noshadows" );

	// check noselfshadows flag
	renderEntity->noSelfShadow = args->GetBool( "noselfshadows" );

	// init any guis, including entity-specific states
	//for( i = 0; i < MAX_RENDERENTITY_GUI; i++ ) {
	//	temp = args->GetString( i == 0 ? "gui" : va( "gui%d", i + 1 ) );
	//	if ( temp[ 0 ] != '\0' ) {
	//		AddRenderGui( temp, &renderEntity->gui[ i ], args );
	//	}
	//}
}

/*
================
idEntity::GetBindMaster
================
*/
idEntity *idEntity::GetBindMaster( void ) const {
	return bindMaster;
}

/*
================
idEntity::GetBindJoint
================
*/
jointHandle_t idEntity::GetBindJoint( void ) const {
	return bindJoint;
}

/*
================
idEntity::GetBindBody
================
*/
int idEntity::GetBindBody( void ) const {
	return bindBody;
}

/*
================
idEntity::GetTeamMaster
================
*/
idEntity *idEntity::GetTeamMaster( void ) const {
	return teamMaster;
}

/*
================
idEntity::GetNextTeamEntity
================
*/
idEntity *idEntity::GetNextTeamEntity( void ) const {
	return teamChain;
}

/*
================
idEntity::GetLocalVector

Takes a vector in worldspace and transforms it into the parent
object's localspace.

Note: Does not take origin into acount.  Use getLocalCoordinate to
convert coordinates.
================
*/
noVec3 idEntity::GetLocalVector( const noVec3 &vec ) const {
	noVec3	pos;

	if ( !bindMaster ) {
		return vec;
	}

	noVec3	masterOrigin;
	noMat3	masterAxis;

	GetMasterPosition( masterOrigin, masterAxis );
	masterAxis.ProjectVector( vec, pos );

	return pos;
}

/*
================
idEntity::GetWorldVector

Takes a vector in the parent object's local coordinates and transforms
it into world coordinates.

Note: Does not take origin into acount.  Use getWorldCoordinate to
convert coordinates.
================
*/
noVec3 idEntity::GetWorldVector( const noVec3 &vec ) const {
	noVec3	pos;

	if ( !bindMaster ) {
		return vec;
	}

	noVec3	masterOrigin;
	noMat3	masterAxis;

	GetMasterPosition( masterOrigin, masterAxis );
	masterAxis.UnprojectVector( vec, pos );

	return pos;
}

/*
================
idEntity::GetWorldCoordinates

Takes a vector in the parent object's local coordinates and transforms
it into world coordinates.
================
*/
noVec3 idEntity::GetWorldCoordinates( const noVec3 &vec ) const {
	noVec3	pos;

	if ( !bindMaster ) {
		return vec;
	}

	noVec3	masterOrigin;
	noMat3	masterAxis;

	GetMasterPosition( masterOrigin, masterAxis );
	masterAxis.UnprojectVector( vec, pos );
	pos += masterOrigin;

	return pos;
}


bool idEntity::GetMasterPosition( noVec3 &masterOrigin, noMat3 &masterAxis ) const {
	noVec3		localOrigin;
	noMat3		localAxis;
	idAnimator	*masterAnimator;

	if ( bindMaster ) {
		// if bound to a joint of an animated model
		if ( bindJoint != INVALID_JOINT ) {
			masterAnimator = bindMaster->GetAnimator();
			if ( !masterAnimator ) {
				masterOrigin = vec3_origin;
				masterAxis = mat3_identity;
				return false;
			} else {
				masterAnimator->GetJointTransform( bindJoint, gameLocal.time, masterOrigin, masterAxis );
				masterAxis *= bindMaster->renderEntity.axis;
				masterOrigin = bindMaster->renderEntity.origin + masterOrigin * bindMaster->renderEntity.axis;
			}
		} else if ( bindBody >= 0 && bindMaster->GetPhysics() ) {
			masterOrigin = bindMaster->GetPhysics()->GetOrigin( bindBody );
			masterAxis = bindMaster->GetPhysics()->GetAxis( bindBody );
		} else {
			masterOrigin = bindMaster->renderEntity.origin;
			masterAxis = bindMaster->renderEntity.axis;
		}
		return true;
	} else {
		masterOrigin = vec3_origin;
		masterAxis = mat3_identity;
		return false;
	}
}

bool idEntity::GetFloorPos( float max_dist, noVec3 &floorpos ) const {
	trace_t result;

	if ( !GetPhysics()->HasGroundContacts() ) {
		GetPhysics()->ClipTranslation( result, GetPhysics()->GetGravityNormal() * max_dist, NULL );
		if ( result.fraction < 1.0f ) {
			floorpos = result.endpos;
			return true;
		} else {
			floorpos = GetPhysics()->GetOrigin();
			return false;
		}
	} else {
		floorpos = GetPhysics()->GetOrigin();
		return true;
	}
}

void idEntity::PreBind( void )
{

}

void idEntity::PostBind( void )
{

}

void idEntity::PreUnbind( void )
{

}

void idEntity::PostUnbind( void )
{

}

void idEntity::Bind( idEntity *master, bool orientated ) {
	if ( !InitBind( master ) ) {
		return;
	}

	PreBind();

	bindJoint = INVALID_JOINT;
	bindBody = -1;
	bindMaster = master;
	fl.bindOrientated = orientated;

	FinishBind();

	PostBind( );
}

bool idEntity::InitBind( idEntity *master ) {
	if ( master == this ) {
		gameLocal.Error( "Tried to bind an object to itself." );
		return false;
	}

	if ( this == gameLocal.world ) {
		gameLocal.Error( "Tried to bind world to another entity" );
		return false;
	}

	// unbind myself from my master
	Unbind();

	// add any bind constraints to an articulated figure
	if ( master && IsType( idAFEntity_Base::Type ) ) {
		static_cast<idAFEntity_Base *>(this)->AddBindConstraints();
	}

	if ( !master || master == gameLocal.world ) {
		// this can happen in scripts, so safely exit out.
		return false;
	}

	return true;
}

/*
================
idEntity::FinishBind
================
*/
void idEntity::FinishBind( void ) {

	// set the master on the physics object
	physics->SetMaster( bindMaster, fl.bindOrientated );

	// We are now separated from our previous team and are either
	// an individual, or have a team of our own.  Now we can join
	// the new bindMaster's team.  Bindmaster must be set before
	// joining the team, or we will be placed in the wrong position
	// on the team.
	//JoinTeam( bindMaster );

	// if our bindMaster is enabled during a cinematic, we must be, too
	//cinematic = bindMaster->cinematic;

	// make sure the team master is active so that physics get run
	teamMaster->BecomeActive( TH_PHYSICS );
}

void idEntity::Unbind( void ) {
	idEntity *	prev;
	idEntity *	next;
	idEntity *	last;
	idEntity *	ent;

	// remove any bind constraints from an articulated figure
	if ( IsType( idAFEntity_Base::Type ) ) {
		static_cast<idAFEntity_Base *>(this)->RemoveBindConstraints();
	}

	if ( !bindMaster ) {
		return;
	}

	if ( !teamMaster ) {
		// Teammaster already has been freed
		bindMaster = NULL;
		return;
	}

	PreUnbind();

	if ( physics ) {
		physics->SetMaster( NULL, fl.bindOrientated );
	}

	// We're still part of a team, so that means I have to extricate myself
	// and any entities that are bound to me from the old team.
	// Find the node previous to me in the team
	prev = teamMaster;
	for( ent = teamMaster->teamChain; ent && ( ent != this ); ent = ent->teamChain ) {
		prev = ent;
	}

	assert( ent == this ); // If ent is not pointing to this, then something is very wrong.

	// Find the last node in my team that is bound to me.
	// Also find the first node not bound to me, if one exists.
	last = this;
	for( next = teamChain; next != NULL; next = next->teamChain ) {
		//if ( !next->IsBoundTo( this ) ) {
		//	break;
		//}

		// Tell them I'm now the teamMaster
		next->teamMaster = this;
		last = next;
	}

	// disconnect the last member of our team from the old team
	last->teamChain = NULL;

	// connect up the previous member of the old team to the node that
	// follow the last node bound to me (if one exists).
	if ( teamMaster != this ) {
		prev->teamChain = next;
		if ( !next && ( teamMaster == prev ) ) {
			prev->teamMaster = NULL;
		}
	} else if ( next ) {
		// If we were the teamMaster, then the nodes that were not bound to me are now
		// a disconnected chain.  Make them into their own team.
		for( ent = next; ent->teamChain != NULL; ent = ent->teamChain ) {
			ent->teamMaster = next;
		}
		next->teamMaster = next;
	}

	// If we don't have anyone on our team, then clear the team variables.
	if ( teamChain ) {
		// make myself my own team
		teamMaster = this;
	} else {
		// no longer a team
		teamMaster = NULL;
	}

	bindJoint = INVALID_JOINT;
	bindBody = -1;
	bindMaster = NULL;

	PostUnbind();
}

/*
================
idEntity::BindToJoint

  bind relative to a joint of the md5 model used by the master
================
*/
void idEntity::BindToJoint( idEntity *master, const char *jointname, bool orientated ) {
	jointHandle_t	jointnum;
	idAnimator		*masterAnimator;

	if ( !InitBind( master ) ) {
		return;
	}

	masterAnimator = master->GetAnimator();
	if ( !masterAnimator ) {
		gameLocal.Warning( "idEntity::BindToJoint: entity '%s' cannot support skeletal models.", master->GetName() );
		return;
	}

	jointnum = masterAnimator->GetJointHandle( jointname );
	if ( jointnum == INVALID_JOINT ) {
		gameLocal.Warning( "idEntity::BindToJoint: joint '%s' not found on entity '%s'.", jointname, master->GetName() );
	}

	PreBind();

	bindJoint = jointnum;
	bindBody = -1;
	bindMaster = master;
	fl.bindOrientated = orientated;

	FinishBind();

	PostBind();
}

void idEntity::BindToJoint( idEntity *master, jointHandle_t jointnum, bool orientated ) {
	if ( !InitBind( master ) ) {
		return;
	}

	PreBind();

	bindJoint = jointnum;
	bindBody = -1;
	bindMaster = master;
	fl.bindOrientated = orientated;

	FinishBind();

	PostBind();
}

void idEntity::BindToBody( idEntity *master, int bodyId, bool orientated ) {

	if ( !InitBind( master ) ) {
		return;
	}

	if ( bodyId < 0 ) {
		gameLocal.Warning( "idEntity::BindToBody: body '%d' not found.", bodyId );
	}

	PreBind();

	bindJoint = INVALID_JOINT;
	bindBody = bodyId;
	bindMaster = master;
	fl.bindOrientated = orientated;

	FinishBind();

	PostBind();
}


