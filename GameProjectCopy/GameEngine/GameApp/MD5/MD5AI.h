
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

#ifndef __ID_AI_H__
#define __ID_AI_H__

/*
===============================================================================

	idAI

===============================================================================
*/
const float	SQUARE_ROOT_OF_2			= 1.414213562f;
const float	AI_TURN_PREDICTION			= 0.2f;
const float	AI_TURN_SCALE				= 60.0f;
const float	AI_SEEK_PREDICTION			= 0.3f;
const float	AI_FLY_DAMPENING			= 0.15f;
const float	AI_HEARING_RANGE			= 2048.0f;
const int	DEFAULT_FLY_OFFSET			= 68;

#define ATTACK_IGNORE			0
#define ATTACK_ON_DAMAGE		1
#define ATTACK_ON_ACTIVATE		2
#define ATTACK_ON_SIGHT			4

// defined in script/ai_base.script.  please keep them up to date.
typedef enum {
	MOVETYPE_DEAD,
	MOVETYPE_ANIM,
	MOVETYPE_SLIDE,
	MOVETYPE_FLY,
	MOVETYPE_STATIC,
	NUM_MOVETYPES
} moveType_t;

typedef enum {
	MOVE_NONE,
	MOVE_FACE_ENEMY,
	MOVE_FACE_ENTITY,

	// commands < NUM_NONMOVING_COMMANDS don't cause a change in position
	NUM_NONMOVING_COMMANDS,

	MOVE_TO_ENEMY = NUM_NONMOVING_COMMANDS,
	MOVE_TO_ENEMYHEIGHT,
	MOVE_TO_ENTITY, 
	MOVE_OUT_OF_RANGE,
	MOVE_TO_ATTACK_POSITION,
	MOVE_TO_COVER,
	MOVE_TO_POSITION,
	MOVE_TO_POSITION_DIRECT,
	MOVE_SLIDE_TO_POSITION,
	MOVE_WANDER,
	NUM_MOVE_COMMANDS
} moveCommand_t;

typedef enum {
	TALK_NEVER,
	TALK_DEAD,
	TALK_OK,
	TALK_BUSY,
	NUM_TALK_STATES
} talkState_t;

//
// status results from move commands
// make sure to change script/doom_defs.script if you add any, or change their order
//
typedef enum {
	MOVE_STATUS_DONE,
	MOVE_STATUS_MOVING,
	MOVE_STATUS_WAITING,
	MOVE_STATUS_DEST_NOT_FOUND,
	MOVE_STATUS_DEST_UNREACHABLE,
	MOVE_STATUS_BLOCKED_BY_WALL,
	MOVE_STATUS_BLOCKED_BY_OBJECT,
	MOVE_STATUS_BLOCKED_BY_ENEMY,
	MOVE_STATUS_BLOCKED_BY_MONSTER
} moveStatus_t;

#define	DI_NODIR	-1

// obstacle avoidance
typedef struct obstaclePath_s {
	noVec3				seekPos;					// seek position avoiding obstacles
	idEntity *			firstObstacle;				// if != NULL the first obstacle along the path
	noVec3				startPosOutsideObstacles;	// start position outside obstacles
	idEntity *			startPosObstacle;			// if != NULL the obstacle containing the start position 
	noVec3				seekPosOutsideObstacles;	// seek position outside obstacles
	idEntity *			seekPosObstacle;			// if != NULL the obstacle containing the seek position 
} obstaclePath_t;

// path prediction
typedef enum {
	SE_BLOCKED			= BIT(0),
	SE_ENTER_LEDGE_AREA	= BIT(1),
	SE_ENTER_OBSTACLE	= BIT(2),
	SE_FALL				= BIT(3),
	SE_LAND				= BIT(4)
} stopEvent_t;

typedef struct predictedPath_s {
	noVec3				endPos;						// final position
	noVec3				endVelocity;				// velocity at end position
	noVec3				endNormal;					// normal of blocking surface
	int					endTime;					// time predicted
	int					endEvent;					// event that stopped the prediction
	const idEntity *	blockingEntity;				// entity that blocks the movement
} predictedPath_t;

//
// events
//
extern const idEventDef AI_BeginAttack;
extern const idEventDef AI_EndAttack;
extern const idEventDef AI_MuzzleFlash;
extern const idEventDef AI_CreateMissile;
extern const idEventDef AI_AttackMissile;
extern const idEventDef AI_FireMissileAtTarget;
extern const idEventDef AI_AttackMelee;
extern const idEventDef AI_DirectDamage;
extern const idEventDef AI_JumpFrame;
extern const idEventDef AI_EnableClip;
extern const idEventDef AI_DisableClip;
extern const idEventDef AI_EnableGravity;
extern const idEventDef AI_DisableGravity;
extern const idEventDef AI_TriggerParticles;
extern const idEventDef AI_RandomPath;

class idMoveState {
public:
	idMoveState();

	moveType_t				moveType;
	moveCommand_t			moveCommand;
	moveStatus_t			moveStatus;
	noVec3					moveDest;
	noVec3					moveDir;			// used for wandering and slide moves
	idEntityPtr<idEntity>	goalEntity;
	noVec3					goalEntityOrigin;	// move to entity uses this to avoid checking the floor position every frame
	int						toAreaNum;
	int						startTime;
	int						duration;
	float					speed;				// only used by flying creatures
	float					range;
	float					wanderYaw;
	int						nextWanderTime;
	int						blockTime;
	idEntityPtr<idEntity>	obstacle;
	noVec3					lastMoveOrigin;
	int						lastMoveTime;
	int						anim;
};


class idAI : public idActor {
public:
	CLASS_PROTOTYPE( idAI );

	idAI();
	~idAI();

	void					Spawn( void );
	void					HeardSound( idEntity *ent, const char *action );
	idActor					*GetEnemy( void ) const;

	void					TalkTo( idActor *actor );
	talkState_t				GetTalkState( void ) const;

	bool					GetAimDir( const noVec3 &firePos, idEntity *aimAtEnt, const idEntity *ignore, noVec3 &aimDir ) const;

	// Return true if the trajectory of the clip model is collision free.
	static bool				TestTrajectory( const noVec3 &start, const noVec3 &end, float zVel, float gravity, float time, float max_height, const idClipModel *clip, int clipmask, const idEntity *ignore, const idEntity *targetEntity, int drawtime );
	// Finds the best collision free trajectory for a clip model.
	static bool				PredictTrajectory( const noVec3 &firePos, const noVec3 &target, float projectileSpeed, const noVec3 &projGravity, const idClipModel *clip, int clipmask, float max_height, const idEntity *ignore, const idEntity *targetEntity, int drawtime, noVec3 &aimDir );

	void					Think( void );


		
	bool			AI_TALK;
	bool			AI_DAMAGE;
	bool			AI_PAIN;
	float			AI_SPECIAL_DAMAGE;
	bool			AI_DEAD;
	bool			AI_ENEMY_VISIBLE;
	bool			AI_ENEMY_IN_FOV;
	bool			AI_ENEMY_DEAD;
	bool			AI_MOVE_DONE;
	bool			AI_ONGROUND;
	bool			AI_ACTIVATED;
	bool			AI_FORWARD;
	bool			AI_JUMP;
	bool			AI_ENEMY_REACHABLE;
	bool			AI_BLOCKED;
	bool			AI_OBSTACLE_IN_PATH;
	bool			AI_DEST_UNREACHABLE;
	bool			AI_HIT_ENEMY;
	bool			AI_PUSHED;


protected:
	// navigation
	//idAAS *					aas;
	int						travelFlags;

	idMoveState				move;
	idMoveState				savedMove;

	float					kickForce;
	bool					ignore_obstacles;
	float					blockedRadius;
	int						blockedMoveTime;
	int						blockedAttackTime;

	// turning
	float					ideal_yaw;
	float					current_yaw;
	float					turnRate;
	float					turnVel;
	float					anim_turn_yaw;
	float					anim_turn_amount;
	float					anim_turn_angles;

	// physics
	idPhysics_Monster		physicsObj;

	// flying
	jointHandle_t			flyTiltJoint;
	float					fly_speed;
	float					fly_bob_strength;
	float					fly_bob_vert;
	float					fly_bob_horz;
	int						fly_offset;					// prefered offset from player's view
	float					fly_seek_scale;
	float					fly_roll_scale;
	float					fly_roll_max;
	float					fly_roll;
	float					fly_pitch_scale;
	float					fly_pitch_max;
	float					fly_pitch;

	bool					allowMove;					// disables any animation movement
	bool					allowHiddenMovement;		// allows character to still move around while hidden
	bool					disableGravity;				// disables gravity and allows vertical movement by the animation
	bool					af_push_moveables;			// allow the articulated figure to push moveable objects

	// weapon/attack vars
	bool					lastHitCheckResult;
	int						lastHitCheckTime;
	int						lastAttackTime;
	float					melee_range;
	float					projectile_height_to_distance_ratio;	// calculates the maximum height a projectile can be thrown
	idList<noVec3>			missileLaunchOffset;

	const idDict *			projectileDef;
	mutable idClipModel		*projectileClipModel;
	float					projectileRadius;
	float					projectileSpeed;
	noVec3					projectileVelocity;
	noVec3					projectileGravity;
	//idEntityPtr<idProjectile> projectile;
	idStr					attack;

	// chatter/talking
	//const idSoundShader		*chat_snd;
	int						chat_min;
	int						chat_max;
	int						chat_time;
	talkState_t				talk_state;
	idEntityPtr<idActor>	talkTarget;

	// cinematics
	int						num_cinematics;
	int						current_cinematic;

	bool					allowJointMod;
	idEntityPtr<idEntity>	focusEntity;
	noVec3					currentFocusPos;
	int						focusTime;
	int						alignHeadTime;
	int						forceAlignHeadTime;
	noAngles				eyeAng;
	noAngles				lookAng;
	noAngles				destLookAng;
	noAngles				lookMin;
	noAngles				lookMax;
	idList<jointHandle_t>	lookJoints;
	idList<noAngles>		lookJointAngles;
	float					eyeVerticalOffset;
	float					eyeHorizontalOffset;
	float					eyeFocusRate;
	float					headFocusRate;
	int						focusAlignTime;

	// special fx
	float					shrivel_rate;
	int						shrivel_start;

	bool					restartParticles;			// should smoke emissions restart
	bool					useBoneAxis;				// use the bone vs the model axis
	//idList<particleEmitter_t> particles;				// particle data

	renderLight_t			worldMuzzleFlash;			// positioned on world weapon bone
	int						worldMuzzleFlashHandle;
	jointHandle_t			flashJointWorld;
	int						muzzleFlashEnd;
	int						flashTime;

	// joint controllers
	noAngles				eyeMin;
	noAngles				eyeMax;
	jointHandle_t			focusJoint;
	jointHandle_t			orientationJoint;

	// enemy variables
	idEntityPtr<idActor>	enemy;
	noVec3					lastVisibleEnemyPos;
	noVec3					lastVisibleEnemyEyeOffset;
	noVec3					lastVisibleReachableEnemyPos;
	noVec3					lastReachableEnemyPos;
	bool					wakeOnFlashlight;


public:
	//
	// ai/ai.cpp
	//
	void					SetAAS( void );
	virtual	void			DormantBegin( void );	// called when entity becomes dormant
	virtual	void			DormantEnd( void );		// called when entity wakes from being dormant

	void					Activate( idEntity *activator );
	int						ReactionTo( const idEntity *ent );	
	void					EnemyDead( void );
	virtual void			Hide( void );
	virtual void			Show( void );
	void					CalculateAttackOffsets( void );

	// movement
	virtual void			ApplyImpulse( idEntity *ent, int id, const noVec3 &point, const noVec3 &impulse );
	void					GetMoveDelta( const noMat3 &oldaxis, const noMat3 &axis, noVec3 &delta );
	void					CheckObstacleAvoidance( const noVec3 &goalPos, noVec3 &newPos );
	void					DeadMove( void );
	void					AnimMove( void );
	void					SlideMove( void );
	void					AdjustFlyingAngles( void );
	void					AddFlyBob( noVec3 &vel );
	void					AdjustFlyHeight( noVec3 &vel, const noVec3 &goalPos );
	void					FlySeekGoal( noVec3 &vel, noVec3 &goalPos );
	void					AdjustFlySpeed( noVec3 &vel );
	void					FlyTurn( void );
	void					FlyMove( void );
	void					StaticMove( void );

	// damage
	virtual bool			Pain( idEntity *inflictor, idEntity *attacker, int damage, const noVec3 &dir, int location );
	virtual void			Killed( idEntity *inflictor, idEntity *attacker, int damage, const noVec3 &dir, int location );

	// navigation
	void					KickObstacles( const noVec3 &dir, float force, idEntity *alwaysKick );
	bool					ReachedPos( const noVec3 &pos, const moveCommand_t moveCommand ) const;
	bool					GetMovePos( noVec3 &seekPos );
	bool					MoveDone( void ) const;
	bool					EntityCanSeePos( idActor *actor, const noVec3 &actorOrigin, const noVec3 &pos );
	void					BlockedFailSafe( void );

	// movement control
	void					StopMove( moveStatus_t status );
	bool					FaceEnemy( void );
	bool					FaceEntity( idEntity *ent );
	bool					DirectMoveToPosition( const noVec3 &pos );
	bool					MoveToEnemyHeight( void );
	bool					MoveOutOfRange( idEntity *entity, float range );
	bool					MoveToAttackPosition( idEntity *ent, int attack_anim );
	bool					MoveToEnemy( void );
	bool					MoveToEntity( idEntity *ent );
	bool					MoveToPosition( const noVec3 &pos );
	bool					MoveToCover( idEntity *entity, const noVec3 &pos );
	bool					SlideToPosition( const noVec3 &pos, float time );
	bool					WanderAround( void );
	bool					StepDirection( float dir );
	bool					NewWanderDir( const noVec3 &dest );

	// turning
	bool					FacingIdeal( void );
	void					Turn( void );
	bool					TurnToward( float yaw );
	bool					TurnToward( const noVec3 &pos );

	// enemy management
	void					ClearEnemy( void );
	bool					EnemyPositionValid( void ) const;
	void					SetEnemyPosition( void );
	void					UpdateEnemyPosition( void );
	void					SetEnemy( idActor *newEnemy );
	virtual void			DamageFeedback( idEntity *victim, idEntity *inflictor, int &damage );
	void					DirectDamage( const char *meleeDefName, idEntity *ent );
	bool					TestMelee( void ) const;
	bool					AttackMelee( const char *meleeDefName );
	void					BeginAttack( const char *name );
	void					EndAttack( void );
	void					PushWithAF( void );
	bool UpdateAnimationControllers( void );
	

	void Event_StopMove( void );
	int Event_MoveStatus( void );
	void Event_StopThinking( void );
	void Event_RealKill( void );

};

#endif /* !__AI_H__ */
