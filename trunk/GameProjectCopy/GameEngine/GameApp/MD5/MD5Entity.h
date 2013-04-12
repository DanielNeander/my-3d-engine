/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __GAME_ENTITY_H__
#define __GAME_ENTITY_H__

#include "Md5Class.h"
#include "MD5Renderer.h"

class idPhysics;
/*
===============================================================================

	Game entity base class.

===============================================================================
*/


static const int DELAY_DORMANT_TIME = 3000;

// Think flags
enum {
	TH_ALL					= -1,
	TH_THINK				= 1,		// run think function each frame
	TH_PHYSICS				= 2,		// run physics each frame
	TH_ANIMATE				= 4,		// update animation each frame
	TH_UPDATEVISUALS		= 8,		// update renderEntity
	TH_UPDATEPARTICLES		= 16
};

//
// Signals
// make sure to change script/doom_defs.script if you add any, or change their order
//
typedef enum {
	SIG_TOUCH,				// object was touched
	SIG_USE,				// object was used
	SIG_TRIGGER,			// object was activated
	SIG_REMOVED,			// object was removed from the game
	SIG_DAMAGE,				// object was damaged
	SIG_BLOCKED,			// object was blocked

	SIG_MOVER_POS1,			// mover at position 1 (door closed)
	SIG_MOVER_POS2,			// mover at position 2 (door open)
	SIG_MOVER_1TO2,			// mover changing from position 1 to 2
	SIG_MOVER_2TO1,			// mover changing from position 2 to 1

	NUM_SIGNALS
} signalNum_t;

// FIXME: At some point we may want to just limit it to one thread per signal, but
// for now, I'm allowing multiple threads.  We should reevaluate this later in the project
#define MAX_SIGNAL_THREADS 16		// probably overkill, but idList uses a granularity of 16



class idEntity : public idClass {
public:
	static const int		MAX_PVS_AREAS = 4;

	int						entityNumber;			// index into the entity list
	int						entityDefNumber;		// index into the entity def list

	idLinkList<idEntity>	spawnNode;				// for being linked into spawnedEntities list
	idLinkList<idEntity>	activeNode;				// for being linked into activeEntities list

	idStr					name;					// name of entity
	idDict					spawnArgs;				// key/value pairs used to spawn and initialize entity

	int						thinkFlags;				// TH_? flags


	int						health;					// FIXME: do all objects really need health?


	struct entityFlags_s {
		bool				notarget			:1;	// if true never attack or target this entity
		bool				noknockback			:1;	// if true no knockback from hits
		bool				takedamage			:1;	// if true this entity can be damaged
		bool				hidden				:1;	// if true this entity is not visible
		bool				bindOrientated		:1;	// if true both the master orientation is used for binding
		bool				solidForTeam		:1;	// if true this entity is considered solid when a physics team mate pushes entities
		bool				forcePhysicsUpdate	:1;	// if true always update from the physics whether the object moved or not
		bool				selected			:1;	// if true the entity is selected for editing
		bool				neverDormant		:1;	// if true the entity never goes dormant
		bool				isDormant			:1;	// if true the entity is dormant
		bool				hasAwakened			:1;	// before a monster has been awakened the first time, use full PVS for dormant instead of area-connected
		bool				networkSync			:1; // if true the entity is synchronized over the network
		bool				grabbed				:1;	// if true object is currently being grabbed
	} fl;

	virtual void			Think( void );


	// apply an impulse to the physics object, 'ent' is the entity applying the impulse
	virtual void			ApplyImpulse( idEntity *ent, int id, const noVec3 &point, const noVec3 &impulse );


public:
	ABSTRACT_PROTOTYPE( idEntity );

	idEntity();
	~idEntity();

	void					Spawn( void );

	void					SetName( const char *name );
	const char *			GetName( void ) const;

	virtual idAnimator *	GetAnimator( void );	// returns animator object used by this entity

	
	// physics
	// set a new physics object to be used by this entity
	void					SetPhysics( idPhysics *phys );
	// get the physics object used by this entity
	idPhysics *				GetPhysics( void ) const;

	bool					RunPhysics( void );
	
	void					SetOrigin( const noVec3 &org );
	// set the axis of the physics object (relative to bindMaster if not NULL)
	void					SetAxis( const noMat3 &axis );
	// use angles to set the axis of the physics object (relative to bindMaster if not NULL)
	void					SetAngles( const noAngles &ang );

	// get the floor position underneath the physics object
	bool					GetFloorPos( float max_dist, noVec3 &floorpos ) const;
	
	virtual void					ActivatePhysics( idEntity *ent );

	// returns true if the physics object is at rest
	virtual bool			IsAtRest( void ) const;

	// damage
	// returns true if this entity can be damaged from the given origin
	virtual bool			CanDamage( const noVec3 &origin, noVec3 &damagePoint ) const;
	// applies damage to this entity
	virtual	void			Damage( idEntity *inflictor, idEntity *attacker, const noVec3 &dir, const char *damageDefName, const float damageScale, const int location );
	// adds a damage effect like overlays, blood, sparks, debris etc.
	virtual void			AddDamageEffect( const trace_t &collision, const noVec3 &velocity, const char *damageDefName );
	// callback function for when another entity received damage from this entity.  damage can be adjusted and returned to the caller.
	virtual void			DamageFeedback( idEntity *victim, idEntity *inflictor, int &damage );
	// notifies this entity that it is in pain
	virtual bool			Pain( idEntity *inflictor, idEntity *attacker, int damage, const noVec3 &dir, int location );
	// notifies this entity that is has been killed
	virtual void			Killed( idEntity *inflictor, idEntity *attacker, int damage, const noVec3 &dir, int location );


	virtual void AddContactEntity( idEntity *ent );
	virtual void RemoveContactEntity( idEntity *ent );

	void					BecomeActive( int flags );
	void					BecomeInactive( int flags );
	bool IsActive( void ) const;
	bool Collide( const trace_t &collision, const noVec3 &velocity );
	void GetImpactInfo( idEntity *ent, int id, const noVec3 &point, impactInfo_t *info );
	bool GetMasterPosition( noVec3 &masterOrigin, noMat3 &masterAxis ) const;
	virtual void AddForce( idEntity *ent, int id, const noVec3 &point, const noVec3 &force );
	
	void UpdateVisuals( void );	
	void					UpdateModel( void );
	virtual void UpdateModelTransform( void );
	virtual bool GetPhysicsToVisualTransform( noVec3 &origin, noMat3 &axis );
	void ConvertLocalToWorldTransform( noVec3 &offset, noMat3 &axis );

	static bool				ModelCallback( renderEntity_s *renderEntity, const renderView_t *renderView );


	// visuals
	virtual void			Present( void );
	virtual renderEntity_t *GetRenderEntity( void );
	virtual int				GetModelDefHandle( void );
	virtual void			SetModel( const char *modelname );

	virtual bool			UpdateAnimationControllers( void );
	bool UpdateRenderEntity( renderEntity_s *renderEntity, const renderView_t *renderView );
	bool					IsHidden( void ) const;
	void UpdateFromPhysics( bool moveBack );
	void InitDefaultPhysics( const noVec3 &origin, const noMat3 &axis );
	idEntity *GetBindMaster( void ) const;
	jointHandle_t GetBindJoint( void ) const;
	int GetBindBody( void ) const;
	idEntity *GetTeamMaster( void ) const;
	idEntity *GetNextTeamEntity( void ) const;
	noVec3 GetLocalVector( const noVec3 &vec ) const;
	noVec3 GetWorldCoordinates( const noVec3 &vec ) const;
	noVec3 GetWorldVector( const noVec3 &vec ) const;

		
	// entity binding
	bool					InitBind( idEntity *master );		// initialize an entity binding
	void					FinishBind( void );	
	virtual void			PreBind( void );
	virtual void			PostBind( void );
	virtual void			PreUnbind( void );
	virtual void			PostUnbind( void );
	void					Bind( idEntity *master, bool orientated );
	void					BindToJoint( idEntity *master, const char *jointname, bool orientated );
	void					BindToJoint( idEntity *master, jointHandle_t jointnum, bool orientated );
	void					BindToBody( idEntity *master, int bodyId, bool orientated );
	void					Unbind( void );

	enum {
		EVENT_STARTSOUNDSHADER,
		EVENT_STOPSOUNDSHADER,
		EVENT_MAXEVENTS
	};

protected:
	renderEntity_t			renderEntity;						// used to present a model to the renderer
	int						modelDefHandle;						// handle to static renderer model


private:
	class idPhysics_Static	defaultPhysicsObj;					// default physics object
	class idPhysics *				physics;							// physics used for this entity
	idEntity *				bindMaster;							// entity bound to if unequal NULL
	jointHandle_t			bindJoint;							// joint bound to if unequal INVALID_JOINT
	int						bindBody;							// body bound to if unequal -1
	idEntity *				teamMaster;							// master of the physics team
	idEntity *				teamChain;	
	int						numPVSAreas;						// number of renderer areas the entity covers
	int						PVSAreas[MAX_PVS_AREAS];			// numbers of the renderer areas the entity covers
};


/*
===============================================================================

	Animated entity base class.
	
===============================================================================
*/

typedef struct damageEffect_s {
	jointHandle_t			jointNum;
	noVec3					localOrigin;
	noVec3					localNormal;
	int						time;
	//const idDeclParticle*	type;
	struct damageEffect_s *	next;
} damageEffect_t;

class idAnimatedEntity : public idEntity {
public:
	CLASS_PROTOTYPE( idAnimatedEntity );

							idAnimatedEntity();
							~idAnimatedEntity();

							virtual void			ClientPredictionThink( void );
							virtual void			Think( void );

							void					UpdateAnimation( void );

							virtual idAnimator *	GetAnimator( void );
							virtual void			SetModel( const char *modelname );

bool						GetJointWorldTransform( jointHandle_t jointHandle, int currentTime, noVec3 &offset, noMat3 &axis );
							bool					GetJointTransformForAnim( jointHandle_t jointHandle, int animNum, int currentTime, noVec3 &offset, noMat3 &axis ) const;

							virtual int				GetDefaultSurfaceType( void ) const;
							virtual void			AddDamageEffect( const trace_t &collision, const noVec3 &velocity, const char *damageDefName );
							void					AddLocalDamageEffect( jointHandle_t jointNum, const noVec3 &localPoint, const noVec3 &localNormal, const noVec3 &localDir, const idDeclEntityDef *def, const idMaterial *collisionMaterial );
							void					UpdateDamageEffects( void );

							virtual bool			ClientReceiveEvent( int event, int time, const idBitMsg &msg );

							enum {
								EVENT_ADD_DAMAGE_EFFECT = idEntity::EVENT_MAXEVENTS,
								EVENT_MAXEVENTS
							};

protected:
	idAnimator				animator;
	damageEffect_t *		damageEffects;


private:
	void					Event_GetJointHandle( const char *jointname );
	void 					Event_ClearAllJoints( void );
	void 					Event_ClearJoint( jointHandle_t jointnum );
	void 					Event_SetJointPos( jointHandle_t jointnum, jointModTransform_t transform_type, const noVec3 &pos );
	void 					Event_SetJointAngle( jointHandle_t jointnum, jointModTransform_t transform_type, const noAngles &angles );
	void 					Event_GetJointPos( jointHandle_t jointnum );
	void 					Event_GetJointAngle( jointHandle_t jointnum );
};

#ifdef _D3XP
class SetTimeState {
	bool					activated;
	bool					previousFast;
	bool					fast;

public:
	SetTimeState();
	SetTimeState( int timeGroup );
	~SetTimeState();

	void					PushState( int timeGroup );
};

ID_INLINE SetTimeState::SetTimeState() {
	activated = false;
}

ID_INLINE SetTimeState::SetTimeState( int timeGroup ) {
	activated = false;
	PushState( timeGroup );
}

ID_INLINE void SetTimeState::PushState( int timeGroup ) {

	// Don't mess with time in Multiplayer
	if ( !gameLocal.isMultiplayer ) {

		activated = true;

		// determine previous fast setting
		if ( gameLocal.time == gameLocal.slow.time ) {
			previousFast = false;
		}
		else {
			previousFast = true;
		}

		// determine new fast setting
		if ( timeGroup ) {
			fast = true;
		}
		else {
			fast = false;
		}

		// set correct time
		if ( fast ) {
			gameLocal.fast.Get( gameLocal.time, gameLocal.previousTime, gameLocal.msec, gameLocal.framenum, gameLocal.realClientTime );
		}
		else {
			gameLocal.slow.Get( gameLocal.time, gameLocal.previousTime, gameLocal.msec, gameLocal.framenum, gameLocal.realClientTime );
		}
	}
}

ID_INLINE SetTimeState::~SetTimeState() {
	if ( activated && !gameLocal.isMultiplayer ) {
		// set previous correct time
		if ( previousFast ) {
			gameLocal.fast.Get( gameLocal.time, gameLocal.previousTime, gameLocal.msec, gameLocal.framenum, gameLocal.realClientTime );
		}
		else {
			gameLocal.slow.Get( gameLocal.time, gameLocal.previousTime, gameLocal.msec, gameLocal.framenum, gameLocal.realClientTime );
		}
	}
}
#endif



#endif /* !__GAME_ENTITY_H__ */
