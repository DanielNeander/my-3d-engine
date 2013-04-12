#include "stdafx.h"
#pragma hdrstop
#include "MD5AI.h"

static const char *moveCommandString[ NUM_MOVE_COMMANDS ] = {
	"MOVE_NONE",
	"MOVE_FACE_ENEMY",
	"MOVE_FACE_ENTITY",
	"MOVE_TO_ENEMY",
	"MOVE_TO_ENEMYHEIGHT",
	"MOVE_TO_ENTITY",
	"MOVE_OUT_OF_RANGE",
	"MOVE_TO_ATTACK_POSITION",
	"MOVE_TO_COVER",
	"MOVE_TO_POSITION",
	"MOVE_TO_POSITION_DIRECT",
	"MOVE_SLIDE_TO_POSITION",
	"MOVE_WANDER"
};

/*
=====================
idMoveState::idMoveState
=====================
*/
idMoveState::idMoveState() {
	moveType			= MOVETYPE_ANIM;
	moveCommand			= MOVE_NONE;
	moveStatus			= MOVE_STATUS_DONE;
	moveDest.Zero();
	moveDir.Set( 1.0f, 0.0f, 0.0f );
	goalEntity			= NULL;
	goalEntityOrigin.Zero();
	toAreaNum			= 0;
	startTime			= 0;
	duration			= 0;
	speed				= 0.0f;
	range				= 0.0f;
	wanderYaw			= 0;
	nextWanderTime		= 0;
	blockTime			= 0;
	obstacle			= NULL;
	lastMoveOrigin		= vec3_origin;
	lastMoveTime		= 0;
	anim				= 0;
}




idAI::idAI()
{
	//aas					= NULL;
	//travelFlags			= TFL_WALK|TFL_AIR;

	kickForce			= 2048.0f;
	ignore_obstacles	= false;
	blockedRadius		= 0.0f;
	blockedMoveTime		= 750;
	blockedAttackTime	= 750;
	turnRate			= 360.0f;
	turnVel				= 0.0f;
	anim_turn_yaw		= 0.0f;
	anim_turn_amount	= 0.0f;
	anim_turn_angles	= 0.0f;
	fly_offset			= 0;
	fly_seek_scale		= 1.0f;
	fly_roll_scale		= 0.0f;
	fly_roll_max		= 0.0f;
	fly_roll			= 0.0f;
	fly_pitch_scale		= 0.0f;
	fly_pitch_max		= 0.0f;
	fly_pitch			= 0.0f;
	allowMove			= false;
	allowHiddenMovement	= false;
	fly_speed			= 0.0f;
	fly_bob_strength	= 0.0f;
	fly_bob_vert		= 0.0f;
	fly_bob_horz		= 0.0f;
	lastHitCheckResult	= false;
	lastHitCheckTime	= 0;
	lastAttackTime		= 0;
	melee_range			= 0.0f;
	projectile_height_to_distance_ratio = 1.0f;
	projectileDef		= NULL;
	//projectile			= NULL;
	projectileClipModel	= NULL;
	projectileRadius	= 0.0f;
	projectileVelocity	= vec3_origin;
	projectileGravity	= vec3_origin;
	projectileSpeed		= 0.0f;
	//chat_snd			= NULL;
	chat_min			= 0;
	chat_max			= 0;
	chat_time			= 0;
	talk_state			= TALK_NEVER;
	talkTarget			= NULL;

	//particles.Clear();
	restartParticles	= true;
	useBoneAxis			= false;

	wakeOnFlashlight	= false;
	memset( &worldMuzzleFlash, 0, sizeof ( worldMuzzleFlash ) );
	worldMuzzleFlashHandle = -1;

	enemy				= NULL;
	lastVisibleEnemyPos.Zero();
	lastVisibleEnemyEyeOffset.Zero();
	lastVisibleReachableEnemyPos.Zero();
	lastReachableEnemyPos.Zero();
	shrivel_rate		= 0.0f;
	shrivel_start		= 0;
	fl.neverDormant		= false;		// AI's can go dormant
	current_yaw			= 0.0f;
	ideal_yaw			= 0.0f;

	num_cinematics		= 0;
	current_cinematic	= 0;

	allowEyeFocus		= true;
	allowPain			= true;
	allowJointMod		= true;
	focusEntity			= NULL;
	focusTime			= 0;
	alignHeadTime		= 0;
	forceAlignHeadTime	= 0;

	currentFocusPos.Zero();
	eyeAng.Zero();
	lookAng.Zero();
	destLookAng.Zero();
	lookMin.Zero();
	lookMax.Zero();

	eyeMin.Zero();
	eyeMax.Zero();
	muzzleFlashEnd		= 0;
	flashTime			= 0;
	flashJointWorld		= INVALID_JOINT;

	focusJoint			= INVALID_JOINT;
	orientationJoint	= INVALID_JOINT;
	flyTiltJoint		= INVALID_JOINT;

	eyeVerticalOffset	= 0.0f;
	eyeHorizontalOffset = 0.0f;
	eyeFocusRate		= 0.0f;
	headFocusRate		= 0.0f;
	focusAlignTime		= 0;
}

idAI::~idAI()
{
	//delete projectileClipModel;

}

void idAI::Spawn( void ) {
	const char			*jointname;
	const idKeyValue	*kv;
	idStr				jointName;
	noAngles			jointScale;
	jointHandle_t		joint;
	noVec3				local_dir;
	bool				talks;

	/*if ( !g_monsters.GetBool() ) {
		PostEventMS( &EV_Remove, 0 );
		return;
	}*/

	spawnArgs.GetInt(	"team",					"1",		team );
	spawnArgs.GetInt(	"rank",					"0",		rank );
	spawnArgs.GetInt(	"fly_offset",			"0",		fly_offset );
	spawnArgs.GetFloat( "fly_speed",			"100",		fly_speed );
	spawnArgs.GetFloat( "fly_bob_strength",		"50",		fly_bob_strength );
	spawnArgs.GetFloat( "fly_bob_vert",			"2",		fly_bob_horz );
	spawnArgs.GetFloat( "fly_bob_horz",			"2.7",		fly_bob_vert );
	spawnArgs.GetFloat( "fly_seek_scale",		"4",		fly_seek_scale );
	spawnArgs.GetFloat( "fly_roll_scale",		"90",		fly_roll_scale );
	spawnArgs.GetFloat( "fly_roll_max",			"60",		fly_roll_max );
	spawnArgs.GetFloat( "fly_pitch_scale",		"45",		fly_pitch_scale );
	spawnArgs.GetFloat( "fly_pitch_max",		"30",		fly_pitch_max );

	spawnArgs.GetFloat( "melee_range",			"64",		melee_range );
	spawnArgs.GetFloat( "projectile_height_to_distance_ratio",	"1", projectile_height_to_distance_ratio );

	spawnArgs.GetFloat( "turn_rate",			"360",		turnRate );

	spawnArgs.GetBool( "talks",					"0",		talks );
	if ( spawnArgs.GetString( "npc_name", NULL ) != NULL ) {
		if ( talks ) {
			talk_state = TALK_OK;
		} else {
			talk_state = TALK_BUSY;
		}
	} else {
		talk_state = TALK_NEVER;
	}

	spawnArgs.GetBool( "animate_z",				"0",		disableGravity );
	spawnArgs.GetBool( "af_push_moveables",		"0",		af_push_moveables );
	spawnArgs.GetFloat( "kick_force",			"4096",		kickForce );
	spawnArgs.GetBool( "ignore_obstacles",		"0",		ignore_obstacles );
	spawnArgs.GetFloat( "blockedRadius",		"-1",		blockedRadius );
	spawnArgs.GetInt( "blockedMoveTime",		"750",		blockedMoveTime );
	spawnArgs.GetInt( "blockedAttackTime",		"750",		blockedAttackTime );

	spawnArgs.GetInt(	"num_cinematics",		"0",		num_cinematics );
	current_cinematic = 0;

	fl.takedamage		= !spawnArgs.GetBool( "noDamage" );
	enemy				= NULL;
	allowMove			= true;
	allowHiddenMovement = false;

	animator.RemoveOriginOffset( true );

	// create combat collision hull for exact collision detection
	SetCombatModel();

	AI_TALK = 0;
	AI_DAMAGE = 0;
	AI_PAIN = 0 ;
	AI_SPECIAL_DAMAGE = 0;
	AI_DEAD = 0;
	AI_ENEMY_VISIBLE = 0;
	AI_ENEMY_IN_FOV = 0;
	AI_ENEMY_DEAD = 0;
	AI_MOVE_DONE = 0;
	AI_ONGROUND = 0;
	AI_ACTIVATED = 0;
	AI_FORWARD = 0;
	AI_JUMP = 0;
	AI_ENEMY_REACHABLE = 0;
	AI_BLOCKED = 0;
	AI_OBSTACLE_IN_PATH = 0;
	AI_DEST_UNREACHABLE = 0;
	AI_HIT_ENEMY = 0;
	AI_PUSHED = 0;
	

	lookMin	= spawnArgs.GetAngles( "look_min", "-80 -75 0" );
	lookMax	= spawnArgs.GetAngles( "look_max", "80 75 0" );

	lookJoints.SetGranularity( 1 );
	lookJointAngles.SetGranularity( 1 );
	kv = spawnArgs.MatchPrefix( "look_joint", NULL );
	while( kv ) {
		jointName = kv->GetKey();
		jointName.StripLeadingOnce( "look_joint " );
		joint = animator.GetJointHandle( jointName );
		if ( joint == INVALID_JOINT ) {
			gameLocal.Warning( "Unknown look_joint '%s' on entity %s", jointName.c_str(), name.c_str() );
		} else {
			jointScale = spawnArgs.GetAngles( kv->GetKey(), "0 0 0" );
			jointScale.roll = 0.0f;

			// if no scale on any component, then don't bother adding it.  this may be done to
			// zero out rotation from an inherited entitydef.
			if ( jointScale != ang_zero ) {
				lookJoints.Append( joint );
				lookJointAngles.Append( jointScale );
			}
		}
		kv = spawnArgs.MatchPrefix( "look_joint", kv );
	}

	// calculate joint positions on attack frames so we can do proper "can hit" tests
	CalculateAttackOffsets();

	eyeMin				= spawnArgs.GetAngles( "eye_turn_min", "-10 -30 0" );
	eyeMax				= spawnArgs.GetAngles( "eye_turn_max", "10 30 0" );
	eyeVerticalOffset	= spawnArgs.GetFloat( "eye_verticle_offset", "5" );
	eyeHorizontalOffset = spawnArgs.GetFloat( "eye_horizontal_offset", "-8" );
	eyeFocusRate		= spawnArgs.GetFloat( "eye_focus_rate", "0.5" );
	headFocusRate		= spawnArgs.GetFloat( "head_focus_rate", "0.1" );
	focusAlignTime		= SEC2MS( spawnArgs.GetFloat( "focus_align_time", "1" ) );

	flashJointWorld = animator.GetJointHandle( "flash" );

	/*if ( head.GetEntity() ) {
		idAnimator *headAnimator = head.GetEntity()->GetAnimator();

		jointname = spawnArgs.GetString( "bone_focus" );
		if ( *jointname ) {
			focusJoint = headAnimator->GetJointHandle( jointname );
			if ( focusJoint == INVALID_JOINT ) {
				gameLocal.Warning( "Joint '%s' not found on head on '%s'", jointname, name.c_str() );
			}
		}
	} else */{
		jointname = spawnArgs.GetString( "bone_focus" );
		if ( *jointname ) {
			focusJoint = animator.GetJointHandle( jointname );
			if ( focusJoint == INVALID_JOINT ) {
				gameLocal.Warning( "Joint '%s' not found on '%s'", jointname, name.c_str() );
			}
		}
	}

	jointname = spawnArgs.GetString( "bone_orientation" );
	if ( *jointname ) {
		orientationJoint = animator.GetJointHandle( jointname );
		if ( orientationJoint == INVALID_JOINT ) {
			gameLocal.Warning( "Joint '%s' not found on '%s'", jointname, name.c_str() );
		}
	}

	jointname = spawnArgs.GetString( "bone_flytilt" );
	if ( *jointname ) {
		flyTiltJoint = animator.GetJointHandle( jointname );
		if ( flyTiltJoint == INVALID_JOINT ) {
			gameLocal.Warning( "Joint '%s' not found on '%s'", jointname, name.c_str() );
		}
	}
	//InitMuzzleFlash();

	physicsObj.SetSelf( this );
	physicsObj.SetClipModel( new idClipModel( GetPhysics()->GetClipModel() ), 1.0f );
	physicsObj.SetMass( spawnArgs.GetFloat( "mass", "100" ) );

	if ( spawnArgs.GetBool( "big_monster" ) ) {
		physicsObj.SetContents( 0 );
		physicsObj.SetClipMask( MASK_MONSTERSOLID & ~CONTENTS_BODY );
	} else {
		if ( use_combat_bbox ) {
			physicsObj.SetContents( CONTENTS_BODY|CONTENTS_SOLID );
		} else {
			physicsObj.SetContents( CONTENTS_BODY );
		}
		physicsObj.SetClipMask( MASK_MONSTERSOLID );
	}

	// move up to make sure the monster is at least an epsilon above the floor
	physicsObj.SetOrigin( GetPhysics()->GetOrigin() + noVec3( 0, 0, CM_CLIP_EPSILON ) );

	if ( num_cinematics ) {
		physicsObj.SetGravity( vec3_origin );
	} else {
		noVec3 gravity = spawnArgs.GetVector( "gravityDir", "0 0 -1" );
		gravity *= g_gravity.GetFloat();
		physicsObj.SetGravity( gravity );
	}

	SetPhysics( &physicsObj );

	physicsObj.GetGravityAxis().ProjectVector( viewAxis[ 0 ], local_dir );
	current_yaw		= local_dir.ToYaw();
	ideal_yaw		= noMath::AngleNormalize180( current_yaw );

	move.blockTime = 0;

	//SetAAS();

	/*projectile		= NULL;
	projectileDef	= NULL;
	projectileClipModel	= NULL;
	idStr projectileName;
	if ( spawnArgs.GetString( "def_projectile", "", projectileName ) && projectileName.Length() ) {
		projectileDef = gameLocal.FindEntityDefDict( projectileName );
		CreateProjectile( vec3_origin, viewAxis[ 0 ] );
		projectileRadius	= projectile.GetEntity()->GetPhysics()->GetClipModel()->GetBounds().GetRadius();
		projectileVelocity	= idProjectile::GetVelocity( projectileDef );
		projectileGravity	= idProjectile::GetGravity( projectileDef );
		projectileSpeed		= projectileVelocity.Length();
		delete projectile.GetEntity();
		projectile = NULL;
	}*/

	//particles.Clear();
	restartParticles = true;
	useBoneAxis = spawnArgs.GetBool( "useBoneAxis" );
	//SpawnParticles( "smokeParticleSystem" );

	if ( num_cinematics || spawnArgs.GetBool( "hide" ) || spawnArgs.GetBool( "teleport" ) || spawnArgs.GetBool( "trigger_anim" ) ) {
		fl.takedamage = false;
		physicsObj.SetContents( 0 );
		physicsObj.GetClipModel()->Unlink();
		Hide();
	} else {
		// play a looping ambient sound if we have one
		//StartSound( "snd_ambient", SND_CHANNEL_AMBIENT, 0, false, NULL );
	}

	if ( health <= 0 ) {
		gameLocal.Warning( "entity '%s' doesn't have health set", name.c_str() );
		health = 1;
	}

	// set up monster chatter
	//SetChatSound();

	BecomeActive( TH_THINK );

	if ( af_push_moveables ) {
		af.SetupPose( this, gameLocal.time );
		af.GetPhysics()->EnableClip();
	}

	// init the move variables
	StopMove( MOVE_STATUS_DONE );
}

void idAI::HeardSound( idEntity *ent, const char *action )
{

}

idActor					* idAI::GetEnemy( void ) const {
	return enemy.GetEntity();
}

void idAI::TalkTo( idActor *actor ) {
	if ( talk_state != TALK_OK ) {
		return;
	}

	talkTarget = actor;
	if ( actor ) {
		AI_TALK = true;
	} else {
		AI_TALK = false;
	}
}

talkState_t idAI::GetTalkState( void ) const {
	if ( ( talk_state != TALK_NEVER ) && AI_DEAD ) {
		return TALK_DEAD;
	}
	if ( IsHidden() ) {
		return TALK_NEVER;
	}
	return talk_state;
}


bool idAI::GetAimDir( const noVec3 &firePos, idEntity *aimAtEnt, const idEntity *ignore, noVec3 &aimDir ) const {
	noVec3	targetPos1;
	noVec3	targetPos2;
	noVec3	delta;
	float	max_height;
	bool	result;

	// if no aimAtEnt or projectile set
	if ( !aimAtEnt || !projectileDef ) {
		aimDir = viewAxis[ 0 ] * physicsObj.GetGravityAxis();
		return false;
	}
#if 0

	if ( projectileClipModel == NULL ) {
		CreateProjectileClipModel();
	}

	if ( aimAtEnt == enemy.GetEntity() ) {
		static_cast<idActor *>( aimAtEnt )->GetAIAimTargets( lastVisibleEnemyPos, targetPos1, targetPos2 );
	} else if ( aimAtEnt->IsType( idActor::Type ) ) {
		static_cast<idActor *>( aimAtEnt )->GetAIAimTargets( aimAtEnt->GetPhysics()->GetOrigin(), targetPos1, targetPos2 );
	} else {
		targetPos1 = aimAtEnt->GetPhysics()->GetAbsBounds().GetCenter();
		targetPos2 = targetPos1;
	}

	// try aiming for chest
	delta = firePos - targetPos1;
	max_height = delta.LengthFast() * projectile_height_to_distance_ratio;
	result = PredictTrajectory( firePos, targetPos1, projectileSpeed, projectileGravity, projectileClipModel, MASK_SHOT_RENDERMODEL, max_height, ignore, aimAtEnt, ai_debugTrajectory.GetBool() ? 1000 : 0, aimDir );
	if ( result || !aimAtEnt->IsType( idActor::Type ) ) {
		return result;
	}

	// try aiming for head
	delta = firePos - targetPos2;
	max_height = delta.LengthFast() * projectile_height_to_distance_ratio;
	result = PredictTrajectory( firePos, targetPos2, projectileSpeed, projectileGravity, projectileClipModel, MASK_SHOT_RENDERMODEL, max_height, ignore, aimAtEnt, ai_debugTrajectory.GetBool() ? 1000 : 0, aimDir );
#endif
	return result;

}

bool idAI::TestTrajectory( const noVec3 &start, const noVec3 &end, float zVel, float gravity, float time, float max_height, const idClipModel *clip, int clipmask, const idEntity *ignore, const idEntity *targetEntity, int drawtime ) {
	return true;
}

bool idAI::PredictTrajectory( const noVec3 &firePos, const noVec3 &target, float projectileSpeed, const noVec3 &projGravity, const idClipModel *clip, int clipmask, float max_height, const idEntity *ignore, const idEntity *targetEntity, int drawtime, noVec3 &aimDir ) {
	return true;
}

///*
//============
//ValidForBounds
//============
//*/
//bool ValidForBounds( const idAASSettings *settings, const idBounds &bounds ) {
//	int i;
//
//	for ( i = 0; i < 3; i++ ) {
//		if ( bounds[0][i] < settings->boundingBoxes[0][0][i] ) {
//			return false;
//		}
//		if ( bounds[1][i] > settings->boundingBoxes[0][1][i] ) {
//			return false;
//		}
//	}
//	return true;
//}


void idAI::SetAAS( void )
{

}

void idAI::DormantBegin( void )
{

}

void idAI::DormantEnd( void )
{

}

// AI 코드랑 다르므로 doom3 코드 확인
void idAI::Think( void ) {
	// if we are completely closed off from the player, don't do anything at all
	/*if ( CheckDormant() ) {
		return;
	}*/

	if ( thinkFlags & TH_THINK ) {
		// clear out the enemy when he dies or is hidden
		idActor *enemyEnt = enemy.GetEntity();
		if ( enemyEnt ) {
			if ( enemyEnt->health <= 0 ) {
				EnemyDead();
			}
		}

		current_yaw += deltaViewAngles.yaw;
		ideal_yaw = noMath::AngleNormalize180( ideal_yaw + deltaViewAngles.yaw );
		deltaViewAngles.Zero();
		viewAxis = noAngles( 0, current_yaw, 0 ).ToMat3();

		// clear the ik before we do anything else so the skeleton doesn't get updated twice
		walkIK.ClearJointMods();

		switch( move.moveType ) {
		case MOVETYPE_DEAD :
			DeadMove();
			break;

		case MOVETYPE_FLY :
			// flying monsters
			UpdateEnemyPosition();
			UpdateAnimState();
			FlyMove();
			CheckBlink();
			break;
		case MOVETYPE_STATIC :
			// static monsters
			UpdateEnemyPosition();
			//UpdateAIScript();
			UpdateAnimState();
			StaticMove();
			//PlayChatter();
			CheckBlink();
			break;

		case MOVETYPE_ANIM :
			// animation based movement
			UpdateEnemyPosition();
			//UpdateAIScript();
			UpdateAnimState();
			AnimMove();
			//PlayChatter();
			CheckBlink();
			break;

		case MOVETYPE_SLIDE :
			// velocity based movement
			UpdateEnemyPosition();
			//UpdateAIScript();
			UpdateAnimState();
			SlideMove();
			//PlayChatter();
			CheckBlink();
			break;
		}
	}  else if ( thinkFlags & TH_PHYSICS ) {
		RunPhysics();
	}
	if ( af_push_moveables ) {
		PushWithAF();
	}

	if ( fl.hidden && allowHiddenMovement ) {
		// UpdateAnimation won't call frame commands when hidden, so call them here when we allow hidden movement
		animator.ServiceAnims( gameLocal.previousTime, gameLocal.time );
	}
	//UpdateMuzzleFlash();
	UpdateAnimation();
	//UpdateParticles();
	Present();
	UpdateDamageEffects();
	//LinkCombat();
}

/*
=====================
idAI::Activate

Notifies the script that a monster has been activated by a trigger or flashlight
=====================
*/
void idAI::Activate( idEntity *activator ) {

}

int idAI::ReactionTo( const idEntity *ent ) {
	if ( ent->fl.hidden ) {
		// ignore hidden entities
		return ATTACK_IGNORE;
	}

	if ( !ent->IsType( idActor::Type ) ) {
		return ATTACK_IGNORE;
	}

	const idActor *actor = static_cast<const idActor *>( ent );
	//if ( actor->IsType( idPlayer::Type ) && static_cast<const idPlayer *>(actor)->noclip ) {
	//	// ignore players in noclip mode
	//	return ATTACK_IGNORE;
	//}

	// actors on different teams will always fight each other
	if ( actor->team != team ) {
		if ( actor->fl.notarget ) {
			// don't attack on sight when attacker is notargeted
			return ATTACK_ON_DAMAGE | ATTACK_ON_ACTIVATE;
		}
		return ATTACK_ON_SIGHT | ATTACK_ON_DAMAGE | ATTACK_ON_ACTIVATE;
	}

	// monsters will fight when attacked by lower ranked monsters.  rank 0 never fights back.
	if ( rank && ( actor->rank < rank ) ) {
		return ATTACK_ON_DAMAGE;
	}

	// don't fight back
	return ATTACK_IGNORE;
}

void idAI::EnemyDead( void ) {
	ClearEnemy();
	AI_ENEMY_DEAD = true;
}

void idAI::Hide( void )
{

}

void idAI::Show( void )
{

}

/*
===================
idAI::CalculateAttackOffsets

calculate joint positions on attack frames so we can do proper "can hit" tests
===================
*/
void idAI::CalculateAttackOffsets( void ) {
	const idDeclModelDef	*modelDef;
	int						num;
	int						i;
	int						frame;
	const frameCommand_t	*command;
	noMat3					axis;
	const idAnim			*anim;
	jointHandle_t			joint;

	modelDef = animator.ModelDef();
	if ( !modelDef ) {
		return;
	}
	num = modelDef->NumAnims();

	// needs to be off while getting the offsets so that we account for the distance the monster moves in the attack anim
	animator.RemoveOriginOffset( false );

	// anim number 0 is reserved for non-existant anims.  to avoid off by one issues, just allocate an extra spot for
	// launch offsets so that anim number can be used without subtracting 1.
	missileLaunchOffset.SetGranularity( 1 );
	missileLaunchOffset.SetNum( num + 1 );
	missileLaunchOffset[ 0 ].Zero();

	for( i = 1; i <= num; i++ ) {
		missileLaunchOffset[ i ].Zero();
		anim = modelDef->GetAnim( i );
		if ( anim ) {
			frame = anim->FindFrameForFrameCommand( FC_LAUNCHMISSILE, &command );
			if ( frame >= 0 ) {
				joint = animator.GetJointHandle( command->string->c_str() );
				if ( joint == INVALID_JOINT ) {
					gameLocal.Error( "Invalid joint '%s' on 'launch_missile' frame command on frame %d of model '%s'", command->string->c_str(), frame, modelDef->GetName() );
				}
				GetJointTransformForAnim( joint, i, FRAME2MS( frame ), missileLaunchOffset[ i ], axis );
			}
		}
	}

	animator.RemoveOriginOffset( true );
}


/***********************************************************************

	Movement

***********************************************************************/

/*
================
idAI::ApplyImpulse
================
*/
void idAI::ApplyImpulse( idEntity *ent, int id, const noVec3 &point, const noVec3 &impulse ) {
	// FIXME: Jim take a look at this and see if this is a reasonable thing to do
	// instead of a spawnArg flag.. Sabaoth is the only slide monster ( and should be the only one for D3 )
	// and we don't want him taking physics impulses as it can knock him off the path
	if ( move.moveType != MOVETYPE_STATIC && move.moveType != MOVETYPE_SLIDE ) {
		idActor::ApplyImpulse( ent, id, point, impulse );
	}
}

void idAI::GetMoveDelta( const noMat3 &oldaxis, const noMat3 &axis, noVec3 &delta ) {
	noVec3 oldModelOrigin;
	noVec3 modelOrigin;

	animator.GetDelta( gameLocal.time - gameLocal.msec, gameLocal.time, delta );
	delta = axis * delta;

	if ( modelOffset != vec3_zero ) {
		// the pivot of the monster's model is around its origin, and not around the bounding
		// box's origin, so we have to compensate for this when the model is offset so that
		// the monster still appears to rotate around it's origin.
		oldModelOrigin = modelOffset * oldaxis;
		modelOrigin = modelOffset * axis;
		delta += oldModelOrigin - modelOrigin;
	}

	delta *= physicsObj.GetGravityAxis();
}

void idAI::CheckObstacleAvoidance( const noVec3 &goalPos, noVec3 &newPos ) {

}

void idAI::DeadMove( void ) {
	noVec3				delta;
	monsterMoveResult_t	moveResult;

	noVec3 org = physicsObj.GetOrigin();

	GetMoveDelta( viewAxis, viewAxis, delta );
	physicsObj.SetDelta( delta );

	RunPhysics();

	moveResult = physicsObj.GetMoveResult();
	AI_ONGROUND = physicsObj.OnGround();
}

void idAI::AnimMove( void ) {
	noVec3				goalPos;
	noVec3				delta;
	noVec3				goalDelta;
	float				goalDist;
	monsterMoveResult_t	moveResult;
	noVec3				newDest;

	noVec3 oldorigin = physicsObj.GetOrigin();
	noMat3 oldaxis = viewAxis;

	AI_BLOCKED = false;

	if ( move.moveCommand < NUM_NONMOVING_COMMANDS ){ 
		move.lastMoveOrigin.Zero();
		move.lastMoveTime = gameLocal.time;
	}

	move.obstacle = NULL;
	if ( ( move.moveCommand == MOVE_FACE_ENEMY ) && enemy.GetEntity() ) {
		TurnToward( lastVisibleEnemyPos );
		goalPos = oldorigin;
	} else if ( ( move.moveCommand == MOVE_FACE_ENTITY ) && move.goalEntity.GetEntity() ) {
		TurnToward( move.goalEntity.GetEntity()->GetPhysics()->GetOrigin() );
		goalPos = oldorigin;
	} else if ( GetMovePos( goalPos ) ) {
		if ( move.moveCommand != MOVE_WANDER ) {
			CheckObstacleAvoidance( goalPos, newDest );
			TurnToward( newDest );
		} else {
			TurnToward( goalPos );
		}
	}

	Turn();

	if ( move.moveCommand == MOVE_SLIDE_TO_POSITION ) {
		if ( gameLocal.time < move.startTime + move.duration ) {
			goalPos = move.moveDest - move.moveDir * MS2SEC( move.startTime + move.duration - gameLocal.time );
			delta = goalPos - oldorigin;
			delta.z = 0.0f;
		} else {
			delta = move.moveDest - oldorigin;
			delta.z = 0.0f;
			StopMove( MOVE_STATUS_DONE );
		}
	} else if ( allowMove ) {
		GetMoveDelta( oldaxis, viewAxis, delta );
	} else {
		delta.Zero();
	}

	if ( move.moveCommand == MOVE_TO_POSITION ) {
		goalDelta = move.moveDest - oldorigin;
		goalDist = goalDelta.LengthFast();
		if ( goalDist < delta.LengthFast() ) {
			delta = goalDelta;
		}
	}

	physicsObj.SetDelta( delta );
	physicsObj.ForceDeltaMove( disableGravity );

	RunPhysics();

	if ( ai_debugMove.GetBool() ) {
		//gameRenderWorld->DebugLine( colorCyan, oldorigin, physicsObj.GetOrigin(), 5000 );
	}

	moveResult = physicsObj.GetMoveResult();
	if ( !af_push_moveables && attack.Length() && TestMelee() ) {
		DirectDamage( attack, enemy.GetEntity() );
	} else {
		idEntity *blockEnt = physicsObj.GetSlideMoveEntity();
		if ( blockEnt && /*blockEnt->IsType( idMoveable::Type ) &&*/ blockEnt->GetPhysics()->IsPushable() ) {
			KickObstacles( viewAxis[ 0 ], kickForce, blockEnt );
		}
	}

	BlockedFailSafe();

	AI_ONGROUND = physicsObj.OnGround();

	noVec3 org = physicsObj.GetOrigin();
	if ( oldorigin != org ) {
		//TouchTriggers();
	}

	if ( ai_debugMove.GetBool() ) {
	/*	gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), org, gameLocal.msec );
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), move.moveDest, gameLocal.msec );
		gameRenderWorld->DebugLine( colorYellow, org + EyeOffset(), org + EyeOffset() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 16.0f, gameLocal.msec, true );
		DrawRoute();*/
	}

}

noVec3 Seek( noVec3 &vel, const noVec3 &org, const noVec3 &goal, float prediction ) {
	noVec3 predictedPos;
	noVec3 goalDelta;
	noVec3 seekVel;

	// predict our position
	predictedPos = org + vel * prediction;
	goalDelta = goal - predictedPos;
	seekVel = goalDelta * MS2SEC( gameLocal.msec );

	return seekVel;
}

void idAI::SlideMove( void ) {
	noVec3				goalPos;
	noVec3				delta;
	noVec3				goalDelta;
	float				goalDist;
	monsterMoveResult_t	moveResult;
	noVec3				newDest;

	noVec3 oldorigin = physicsObj.GetOrigin();
	noMat3 oldaxis = viewAxis;

	AI_BLOCKED = false;

	if ( move.moveCommand < NUM_NONMOVING_COMMANDS ){ 
		move.lastMoveOrigin.Zero();
		move.lastMoveTime = gameLocal.time;
	}

	move.obstacle = NULL;
	if ( ( move.moveCommand == MOVE_FACE_ENEMY ) && enemy.GetEntity() ) {
		TurnToward( lastVisibleEnemyPos );
		goalPos = move.moveDest;
	} else if ( ( move.moveCommand == MOVE_FACE_ENTITY ) && move.goalEntity.GetEntity() ) {
		TurnToward( move.goalEntity.GetEntity()->GetPhysics()->GetOrigin() );
		goalPos = move.moveDest;
	} else if ( GetMovePos( goalPos ) ) {
		CheckObstacleAvoidance( goalPos, newDest );
		TurnToward( newDest );
		goalPos = newDest;
	}

	if ( move.moveCommand == MOVE_SLIDE_TO_POSITION ) {
		if ( gameLocal.time < move.startTime + move.duration ) {
			goalPos = move.moveDest - move.moveDir * MS2SEC( move.startTime + move.duration - gameLocal.time );
		} else {
			goalPos = move.moveDest;
			StopMove( MOVE_STATUS_DONE );
		}
	}

	if ( move.moveCommand == MOVE_TO_POSITION ) {
		goalDelta = move.moveDest - oldorigin;
		goalDist = goalDelta.LengthFast();
		if ( goalDist < delta.LengthFast() ) {
			delta = goalDelta;
		}
	}

	noVec3 vel = physicsObj.GetLinearVelocity();
	float z = vel.z;
	noVec3  predictedPos = oldorigin + vel * AI_SEEK_PREDICTION;

	// seek the goal position
	goalDelta = goalPos - predictedPos;
	vel -= vel * AI_FLY_DAMPENING * MS2SEC( gameLocal.msec );
	vel += goalDelta * MS2SEC( gameLocal.msec );

	// cap our speed
	vel.Truncate( fly_speed );
	vel.z = z;
	physicsObj.SetLinearVelocity( vel );
	physicsObj.UseVelocityMove( true );
	RunPhysics();

	if ( ( move.moveCommand == MOVE_FACE_ENEMY ) && enemy.GetEntity() ) {
		TurnToward( lastVisibleEnemyPos );
	} else if ( ( move.moveCommand == MOVE_FACE_ENTITY ) && move.goalEntity.GetEntity() ) {
		TurnToward( move.goalEntity.GetEntity()->GetPhysics()->GetOrigin() );
	} else if ( move.moveCommand != MOVE_NONE ) {
		if ( vel.ToVec2().LengthSqr() > 0.1f ) {
			TurnToward( vel.ToYaw() );
		}
	}
	Turn();

	if ( ai_debugMove.GetBool() ) {
		//gameRenderWorld->DebugLine( colorCyan, oldorigin, physicsObj.GetOrigin(), 5000 );
	}

	moveResult = physicsObj.GetMoveResult();
	if ( !af_push_moveables && attack.Length() && TestMelee() ) {
		DirectDamage( attack, enemy.GetEntity() );
	} else {
		idEntity *blockEnt = physicsObj.GetSlideMoveEntity();
		if ( blockEnt && /*blockEnt->IsType( idMoveable::Type ) &&*/ blockEnt->GetPhysics()->IsPushable() ) {
			KickObstacles( viewAxis[ 0 ], kickForce, blockEnt );
		}
	}

	BlockedFailSafe();

	AI_ONGROUND = physicsObj.OnGround();

	noVec3 org = physicsObj.GetOrigin();
	if ( oldorigin != org ) {
		//TouchTriggers();
	}

	if ( ai_debugMove.GetBool() ) {
		//gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), org, gameLocal.msec );
		//gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), move.moveDest, gameLocal.msec );
		//gameRenderWorld->DebugLine( colorYellow, org + EyeOffset(), org + EyeOffset() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 16.0f, gameLocal.msec, true );
		//DrawRoute();
	}

}

void idAI::AdjustFlyingAngles( void ) {
	noVec3	vel;
	float 	speed;
	float 	roll;
	float 	pitch;

	vel = physicsObj.GetLinearVelocity();

	speed = vel.Length();
	if ( speed < 5.0f ) {
		roll = 0.0f;
		pitch = 0.0f;
	} else {
		roll = vel * viewAxis[ 1 ] * -fly_roll_scale / fly_speed;
		if ( roll > fly_roll_max ) {
			roll = fly_roll_max;
		} else if ( roll < -fly_roll_max ) {
			roll = -fly_roll_max;
		}

		pitch = vel * viewAxis[ 2 ] * -fly_pitch_scale / fly_speed;
		if ( pitch > fly_pitch_max ) {
			pitch = fly_pitch_max;
		} else if ( pitch < -fly_pitch_max ) {
			pitch = -fly_pitch_max;
		}
	}

	fly_roll = fly_roll * 0.95f + roll * 0.05f;
	fly_pitch = fly_pitch * 0.95f + pitch * 0.05f;

	if ( flyTiltJoint != INVALID_JOINT ) {
		animator.SetJointAxis( flyTiltJoint, JOINTMOD_WORLD, noAngles( fly_pitch, 0.0f, fly_roll ).ToMat3() );
	} else {
		viewAxis = noAngles( fly_pitch, current_yaw, fly_roll ).ToMat3();
	}

}

void idAI::AddFlyBob( noVec3 &vel ) {
	noVec3	fly_bob_add;
	float	t;

	if ( fly_bob_strength ) {
		t = MS2SEC( gameLocal.time + entityNumber * 497 );
		fly_bob_add = ( viewAxis[ 1 ] * noMath::Sin16( t * fly_bob_horz ) + viewAxis[ 2 ] * noMath::Sin16( t * fly_bob_vert ) ) * fly_bob_strength;
		vel += fly_bob_add * MS2SEC( gameLocal.msec );
		if ( ai_debugMove.GetBool() ) {
			const noVec3 &origin = physicsObj.GetOrigin();
			//gameRenderWorld->DebugArrow( colorOrange, origin, origin + fly_bob_add, 0 );
		}
	}

}

void idAI::AdjustFlyHeight( noVec3 &vel, const noVec3 &goalPos ) {

}

void idAI::FlySeekGoal( noVec3 &vel, noVec3 &goalPos ) {
	noVec3 seekVel;

	// seek the goal position
	seekVel = Seek( vel, physicsObj.GetOrigin(), goalPos, AI_SEEK_PREDICTION );
	seekVel *= fly_seek_scale;
	vel += seekVel;
}

void idAI::AdjustFlySpeed( noVec3 &vel ) {
	float speed;

	// apply dampening
	vel -= vel * AI_FLY_DAMPENING * MS2SEC( gameLocal.msec );

	// gradually speed up/slow down to desired speed
	speed = vel.Normalize();
	speed += ( move.speed - speed ) * MS2SEC( gameLocal.msec );
	if ( speed < 0.0f ) {
		speed = 0.0f;
	} else if ( move.speed && ( speed > move.speed ) ) {
		speed = move.speed;
	}

	vel *= speed;
}

void idAI::FlyTurn( void ) {
	if ( move.moveCommand == MOVE_FACE_ENEMY ) {
		TurnToward( lastVisibleEnemyPos );
	} else if ( ( move.moveCommand == MOVE_FACE_ENTITY ) && move.goalEntity.GetEntity() ) {
		TurnToward( move.goalEntity.GetEntity()->GetPhysics()->GetOrigin() );
	} else if ( move.speed > 0.0f ) {
		const noVec3 &vel = physicsObj.GetLinearVelocity();
		if ( vel.ToVec2().LengthSqr() > 0.1f ) {
			TurnToward( vel.ToYaw() );
		}
	}
	Turn();
}

void idAI::FlyMove( void ) {
	noVec3	goalPos;
	noVec3	oldorigin;
	noVec3	newDest;

	AI_BLOCKED = false;
	if ( ( move.moveCommand != MOVE_NONE ) && ReachedPos( move.moveDest, move.moveCommand ) ) {
		StopMove( MOVE_STATUS_DONE );
	}

	if ( ai_debugMove.GetBool() ) {
		gameLocal.Printf( "%d: %s: %s, vel = %.2f, sp = %.2f, maxsp = %.2f\n", gameLocal.time, name.c_str(), moveCommandString[ move.moveCommand ], physicsObj.GetLinearVelocity().Length(), move.speed, fly_speed );
	}

	if ( move.moveCommand != MOVE_TO_POSITION_DIRECT ) {
		noVec3 vel = physicsObj.GetLinearVelocity();

		if ( GetMovePos( goalPos ) ) {
			CheckObstacleAvoidance( goalPos, newDest );
			goalPos = newDest;
		}

		if ( move.speed	) {
			FlySeekGoal( vel, goalPos );
		}

		// add in bobbing
		AddFlyBob( vel );

		if ( enemy.GetEntity() && ( move.moveCommand != MOVE_TO_POSITION ) ) {
			AdjustFlyHeight( vel, goalPos );
		}

		AdjustFlySpeed( vel );

		physicsObj.SetLinearVelocity( vel );
	}

	// turn
	FlyTurn();

	// run the physics for this frame
	oldorigin = physicsObj.GetOrigin();
	physicsObj.UseFlyMove( true );
	physicsObj.UseVelocityMove( false );
	physicsObj.SetDelta( vec3_zero );
	physicsObj.ForceDeltaMove( disableGravity );
	RunPhysics();

	monsterMoveResult_t	moveResult = physicsObj.GetMoveResult();
	if ( !af_push_moveables && attack.Length() && TestMelee() ) {
		DirectDamage( attack, enemy.GetEntity() );
	} else {
		idEntity *blockEnt = physicsObj.GetSlideMoveEntity();
		if ( blockEnt && /*blockEnt->IsType( idMoveable::Type ) &&*/ blockEnt->GetPhysics()->IsPushable() ) {
			KickObstacles( viewAxis[ 0 ], kickForce, blockEnt );
		} else if ( moveResult == MM_BLOCKED ) {
			move.blockTime = gameLocal.time + 500;
			AI_BLOCKED = true;
		}
	}

	noVec3 org = physicsObj.GetOrigin();
	if ( oldorigin != org ) {
		//TouchTriggers();
	}
#if 0
	if ( ai_debugMove.GetBool() ) {
		gameRenderWorld->DebugLine( colorCyan, oldorigin, physicsObj.GetOrigin(), 4000 );
		gameRenderWorld->DebugBounds( colorOrange, physicsObj.GetBounds(), org, gameLocal.msec );
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), move.moveDest, gameLocal.msec );
		gameRenderWorld->DebugLine( colorRed, org, org + physicsObj.GetLinearVelocity(), gameLocal.msec, true );
		gameRenderWorld->DebugLine( colorBlue, org, goalPos, gameLocal.msec, true );
		gameRenderWorld->DebugLine( colorYellow, org + EyeOffset(), org + EyeOffset() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 16.0f, gameLocal.msec, true );
		DrawRoute();
	}
#endif

}

void idAI::StaticMove( void ) {
	idActor	*enemyEnt = enemy.GetEntity();

	if ( AI_DEAD ) {
		return;
	}

	if ( ( move.moveCommand == MOVE_FACE_ENEMY ) && enemyEnt ) {
		TurnToward( lastVisibleEnemyPos );
	} else if ( ( move.moveCommand == MOVE_FACE_ENTITY ) && move.goalEntity.GetEntity() ) {
		TurnToward( move.goalEntity.GetEntity()->GetPhysics()->GetOrigin() );
	} else if ( move.moveCommand != MOVE_NONE ) {
		TurnToward( move.moveDest );
	}
	Turn();

	physicsObj.ForceDeltaMove( true ); // disable gravity
	RunPhysics();

	AI_ONGROUND = false;

	if ( !af_push_moveables && attack.Length() && TestMelee() ) {
		DirectDamage( attack, enemyEnt );
	}
#if 0
	if ( ai_debugMove.GetBool() ) {
		const noVec3 &org = physicsObj.GetOrigin();
		gameRenderWorld->DebugBounds( colorMagenta, physicsObj.GetBounds(), org, gameLocal.msec );
		gameRenderWorld->DebugLine( colorBlue, org, move.moveDest, gameLocal.msec, true );
		gameRenderWorld->DebugLine( colorYellow, org + EyeOffset(), org + EyeOffset() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 16.0f, gameLocal.msec, true );
	}
#endif

}

/***********************************************************************

	Damage

***********************************************************************/
bool idAI::Pain( idEntity *inflictor, idEntity *attacker, int damage, const noVec3 &dir, int location ) {
	idActor	*actor;

	AI_PAIN = idActor::Pain( inflictor, attacker, damage, dir, location );
	AI_DAMAGE = true;

	// force a blink
	blink_time = 0;

	// ignore damage from self
	if ( attacker != this ) {
		if ( inflictor ) {
			AI_SPECIAL_DAMAGE = inflictor->spawnArgs.GetInt( "special_damage" );
		} else {
			AI_SPECIAL_DAMAGE = 0;
		}

		if ( enemy.GetEntity() != attacker && attacker->IsType( idActor::Type ) ) {
			actor = ( idActor * )attacker;
			if ( ReactionTo( actor ) & ATTACK_ON_DAMAGE ) {
				//gameLocal.AlertAI( actor );
				SetEnemy( actor );
			}
		}
	}

	return ( AI_PAIN != 0 );

}

void idAI::Killed( idEntity *inflictor, idEntity *attacker, int damage, const noVec3 &dir, int location ) {
	noAngles ang;
	const char *modelDeath;

	// make sure the monster is activated
	EndAttack();

	/*if ( g_debugDamage.GetBool() ) {
		gameLocal.Printf( "Damage: joint: '%s', zone '%s'\n", animator.GetJointName( ( jointHandle_t )location ), 
			GetDamageGroup( location ) );
	}*/

	if ( inflictor ) {
		AI_SPECIAL_DAMAGE = inflictor->spawnArgs.GetInt( "special_damage" );
	} else {
		AI_SPECIAL_DAMAGE = 0;
	}

	if ( AI_DEAD ) {
		AI_PAIN = true;
		AI_DAMAGE = true;
		return;
	}

	// stop all voice sounds
	/*StopSound( SND_CHANNEL_VOICE, false );
	if ( head.GetEntity() ) {
		head.GetEntity()->StopSound( SND_CHANNEL_VOICE, false );
		head.GetEntity()->GetAnimator()->ClearAllAnims( gameLocal.time, 100 );
	}*/

	disableGravity = false;
	move.moveType = MOVETYPE_DEAD;
	af_push_moveables = false;

	physicsObj.UseFlyMove( false );
	physicsObj.ForceDeltaMove( false );

	// end our looping ambient sound
	//StopSound( SND_CHANNEL_AMBIENT, false );

	if ( attacker && attacker->IsType( idActor::Type ) ) {
		//gameLocal.AlertAI( ( idActor * )attacker );
	}

	// activate targets
	//ActivateTargets( attacker );

	//RemoveAttachments();
	//RemoveProjectile();
	StopMove( MOVE_STATUS_DONE );

	ClearEnemy();
	AI_DEAD	= true;

	// make monster nonsolid
	physicsObj.SetContents( 0 );
	physicsObj.GetClipModel()->Unlink();

	//Unbind();

	if ( StartRagdoll() ) {
		//StartSound( "snd_death", SND_CHANNEL_VOICE, 0, false, NULL );
	}

	if ( spawnArgs.GetString( "model_death", "", &modelDeath ) ) {
		// lost soul is only case that does not use a ragdoll and has a model_death so get the death sound in here
		//StartSound( "snd_death", SND_CHANNEL_VOICE, 0, false, NULL );
		renderEntity.shaderParms[ SHADERPARM_TIMEOFFSET ] = -MS2SEC( gameLocal.time );
		SetModel( modelDeath );
		physicsObj.SetLinearVelocity( vec3_zero );
		physicsObj.PutToRest();
		physicsObj.DisableImpact();
	}

	restartParticles = false;

	/*state = GetScriptFunction( "state_Killed" );
	SetState( state );
	SetWaitState( "" );*/
	AI_DEAD = true;
	Event_StopMove();
	SetAnimState(ANIMCHANNEL_LEGS, "Legs_Death", 0 );
	SetWaitState( "dead" );
	Event_StopThinking();	
	Event_PlayAnim(ANIMCHANNEL_LEGS, "death");

	const idKeyValue *kv = spawnArgs.MatchPrefix( "def_drops", NULL );
	while( kv ) {
		idDict args;

		args.Set( "classname", kv->GetValue() );
		args.Set( "origin", physicsObj.GetOrigin().ToString() );
		gameLocal.SpawnEntityDef( args );
		kv = spawnArgs.MatchPrefix( "def_drops", kv );
	}

	/*if ( ( attacker && attacker->IsType( idPlayer::Type ) ) && ( inflictor && !inflictor->IsType( idSoulCubeMissile::Type ) ) ) {
		static_cast< idPlayer* >( attacker )->AddAIKill();
	}*/

}

void idAI::KickObstacles( const noVec3 &dir, float force, idEntity *alwaysKick ) {
	int i, numListedClipModels;
	idBounds clipBounds;
	idEntity *obEnt;
	idClipModel *clipModel;
	idClipModel *clipModelList[ MAX_GENTITIES ];
	int clipmask;
	noVec3 org;
	noVec3 forceVec;
	noVec3 delta;
	noVec2 perpendicular;

	org = physicsObj.GetOrigin();

	// find all possible obstacles
	clipBounds = physicsObj.GetAbsBounds();
	clipBounds.TranslateSelf( dir * 32.0f );
	clipBounds.ExpandSelf( 8.0f );
	clipBounds.AddPoint( org );
	clipmask = physicsObj.GetClipMask();
	numListedClipModels = gameLocal.clip.ClipModelsTouchingBounds( clipBounds, clipmask, clipModelList, MAX_GENTITIES );
	for ( i = 0; i < numListedClipModels; i++ ) {
		clipModel = clipModelList[i];
		obEnt = clipModel->GetEntity();
		if ( obEnt == alwaysKick ) {
			// we'll kick this one outside the loop
			continue;
		}

		if ( !clipModel->IsTraceModel() ) {
			continue;
		}

		if ( /*obEnt->IsType( idMoveable::Type ) &&*/ obEnt->GetPhysics()->IsPushable() ) {
			delta = obEnt->GetPhysics()->GetOrigin() - org;
			delta.NormalizeFast();
			perpendicular.x = -delta.y;
			perpendicular.y = delta.x;
			delta.z += 0.5f;
			delta.ToVec2() += perpendicular * gameLocal.random.CRandomFloat() * 0.5f;
			forceVec = delta * force * obEnt->GetPhysics()->GetMass();
			obEnt->ApplyImpulse( this, 0, obEnt->GetPhysics()->GetOrigin(), forceVec );
		}
	}

	if ( alwaysKick ) {
		delta = alwaysKick->GetPhysics()->GetOrigin() - org;
		delta.NormalizeFast();
		perpendicular.x = -delta.y;
		perpendicular.y = delta.x;
		delta.z += 0.5f;
		delta.ToVec2() += perpendicular * gameLocal.random.CRandomFloat() * 0.5f;
		forceVec = delta * force * alwaysKick->GetPhysics()->GetMass();
		alwaysKick->ApplyImpulse( this, 0, alwaysKick->GetPhysics()->GetOrigin(), forceVec );
	}
}

bool idAI::ReachedPos( const noVec3 &pos, const moveCommand_t moveCommand ) const {
	if ( move.moveType == MOVETYPE_SLIDE ) {
		idBounds bnds( noVec3( -4, -4.0f, -8.0f ), noVec3( 4.0f, 4.0f, 64.0f ) );
		bnds.TranslateSelf( physicsObj.GetOrigin() );	
		if ( bnds.ContainsPoint( pos ) ) {
			return true;
		}
	} else {
		if ( ( moveCommand == MOVE_TO_ENEMY ) || ( moveCommand == MOVE_TO_ENTITY ) ) {
			if ( physicsObj.GetAbsBounds().IntersectsBounds( idBounds( pos ).Expand( 8.0f ) ) ) {
				return true;
			}
		} else {
			idBounds bnds( noVec3( -16.0, -16.0f, -8.0f ), noVec3( 16.0, 16.0f, 64.0f ) );
			bnds.TranslateSelf( physicsObj.GetOrigin() );	
			if ( bnds.ContainsPoint( pos ) ) {
				return true;
			}
		}
	}
	return false;
}

bool idAI::GetMovePos( noVec3 &seekPos ) {
	int			areaNum;
	//aasPath_t	path;
	bool		result;
	noVec3		org;

	org = physicsObj.GetOrigin();
	seekPos = org;

	switch( move.moveCommand ) {
	case MOVE_NONE :
		seekPos = move.moveDest;
		return false;
		break;

	case MOVE_FACE_ENEMY :
	case MOVE_FACE_ENTITY :
		seekPos = move.moveDest;
		return false;
		break;

	case MOVE_TO_POSITION_DIRECT :
		seekPos = move.moveDest;
		if ( ReachedPos( move.moveDest, move.moveCommand ) ) {
			StopMove( MOVE_STATUS_DONE );
		}
		return false;
		break;

	case MOVE_SLIDE_TO_POSITION :
		seekPos = org;
		return false;
		break;
	}

	if ( move.moveCommand == MOVE_TO_ENTITY ) {
		MoveToEntity( move.goalEntity.GetEntity() );
	}

	move.moveStatus = MOVE_STATUS_MOVING;
	result = false;
	if ( gameLocal.time > move.blockTime ) {
		if ( move.moveCommand == MOVE_WANDER ) {
			move.moveDest = org + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 256.0f;
		} else {
			if ( ReachedPos( move.moveDest, move.moveCommand ) ) {
				StopMove( MOVE_STATUS_DONE );
				seekPos	= org;
				return false;
			}
		}

		/*if ( aas && move.toAreaNum ) {
			areaNum	= PointReachableAreaNum( org );
			if ( PathToGoal( path, areaNum, org, move.toAreaNum, move.moveDest ) ) {
				seekPos = path.moveGoal;
				result = true;
				move.nextWanderTime = 0;
			} else {
				AI_DEST_UNREACHABLE = true;
			}
		}*/
	}

	if ( !result ) {
		// wander around
		if ( ( gameLocal.time > move.nextWanderTime ) || !StepDirection( move.wanderYaw ) ) {
			result = NewWanderDir( move.moveDest );
			if ( !result ) {
				StopMove( MOVE_STATUS_DEST_UNREACHABLE );
				AI_DEST_UNREACHABLE = true;
				seekPos	= org;
				return false;
			}
		} else {
			result = true;
		}

		seekPos = org + move.moveDir * 2048.0f;
		/*if ( ai_debugMove.GetBool() ) {
			gameRenderWorld->DebugLine( colorYellow, org, seekPos, gameLocal.msec, true );
		}*/
	} else {
		AI_DEST_UNREACHABLE = false;
	}

	/*if ( result && ( ai_debugMove.GetBool() ) ) {
		gameRenderWorld->DebugLine( colorCyan, physicsObj.GetOrigin(), seekPos );
	}*/

	return result;

}

bool idAI::MoveDone( void ) const {
	return ( move.moveCommand == MOVE_NONE );
}

bool idAI::EntityCanSeePos( idActor *actor, const noVec3 &actorOrigin, const noVec3 &pos )
{
	return true;
}

void idAI::BlockedFailSafe( void ) {
	if ( !ai_blockedFailSafe.GetBool() || blockedRadius < 0.0f ) {
		return;
	}
	if ( !physicsObj.OnGround() || enemy.GetEntity() == NULL ||
		( physicsObj.GetOrigin() - move.lastMoveOrigin ).LengthSqr() > Square( blockedRadius ) ) {
			move.lastMoveOrigin = physicsObj.GetOrigin();
			move.lastMoveTime = gameLocal.time;
	}
	if ( move.lastMoveTime < gameLocal.time - blockedMoveTime ) {
		if ( lastAttackTime < gameLocal.time - blockedAttackTime ) {
			AI_BLOCKED = true;
			move.lastMoveTime = gameLocal.time;
		}
	}

}

void idAI::StopMove( moveStatus_t status ) {
	AI_MOVE_DONE		= true;
	AI_FORWARD			= false;
	move.moveCommand	= MOVE_NONE;
	move.moveStatus		= status;
	move.toAreaNum		= 0;
	move.goalEntity		= NULL;
	move.moveDest		= physicsObj.GetOrigin();
	AI_DEST_UNREACHABLE	= false;
	AI_OBSTACLE_IN_PATH = false;
	AI_BLOCKED			= false;
	move.startTime		= gameLocal.time;
	move.duration		= 0;
	move.range			= 0.0f;
	move.speed			= 0.0f;
	move.anim			= 0;
	move.moveDir.Zero();
	move.lastMoveOrigin.Zero();
	move.lastMoveTime	= gameLocal.time;
}

bool idAI::FaceEnemy( void ) {
	idActor *enemyEnt = enemy.GetEntity();
	if ( !enemyEnt ) {
		StopMove( MOVE_STATUS_DEST_NOT_FOUND );
		return false;
	}

	TurnToward( lastVisibleEnemyPos );
	move.goalEntity		= enemyEnt;
	move.moveDest		= physicsObj.GetOrigin();
	move.moveCommand	= MOVE_FACE_ENEMY;
	move.moveStatus		= MOVE_STATUS_WAITING;
	move.startTime		= gameLocal.time;
	move.speed			= 0.0f;
	AI_MOVE_DONE		= true;
	AI_FORWARD			= false;
	AI_DEST_UNREACHABLE = false;

	return true;
}

/*
=====================
idAI::FaceEntity

Continually face the entity position.  MoveDone is always true in this case.
=====================
*/
bool idAI::FaceEntity( idEntity *ent ) {
	if ( !ent ) {
		StopMove( MOVE_STATUS_DEST_NOT_FOUND );
		return false;
	}

	noVec3 entityOrg = ent->GetPhysics()->GetOrigin();
	TurnToward( entityOrg );
	move.goalEntity		= ent;
	move.moveDest		= physicsObj.GetOrigin();
	move.moveCommand	= MOVE_FACE_ENTITY;
	move.moveStatus		= MOVE_STATUS_WAITING;
	move.startTime		= gameLocal.time;
	move.speed			= 0.0f;
	AI_MOVE_DONE		= true;
	AI_FORWARD			= false;
	AI_DEST_UNREACHABLE = false;

	return true;
}

bool idAI::DirectMoveToPosition( const noVec3 &pos ) {
	if ( ReachedPos( pos, move.moveCommand ) ) {
		StopMove( MOVE_STATUS_DONE );
		return true;
	}

	move.moveDest		= pos;
	move.goalEntity		= NULL;
	move.moveCommand	= MOVE_TO_POSITION_DIRECT;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.startTime		= gameLocal.time;
	move.speed			= fly_speed;
	AI_MOVE_DONE		= false;
	AI_DEST_UNREACHABLE = false;
	AI_FORWARD			= true;

	if ( move.moveType == MOVETYPE_FLY ) {
		noVec3 dir = pos - physicsObj.GetOrigin();
		dir.Normalize();
		dir *= fly_speed;
		physicsObj.SetLinearVelocity( dir );
	}

	return true;
}

bool idAI::MoveToEnemyHeight( void ) {
	idActor	*enemyEnt = enemy.GetEntity();

	if ( !enemyEnt || ( move.moveType != MOVETYPE_FLY ) ) {
		StopMove( MOVE_STATUS_DEST_NOT_FOUND );
		return false;
	}

	move.moveDest.z		= lastVisibleEnemyPos.z + enemyEnt->EyeOffset().z + fly_offset;
	move.goalEntity		= enemyEnt;
	move.moveCommand	= MOVE_TO_ENEMYHEIGHT;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.startTime		= gameLocal.time;
	move.speed			= 0.0f;
	AI_MOVE_DONE		= false;
	AI_DEST_UNREACHABLE = false;
	AI_FORWARD			= false;

	return true;
}

bool idAI::MoveOutOfRange( idEntity *entity, float range )
{
	return true;
}

bool idAI::MoveToAttackPosition( idEntity *ent, int attack_anim ) {
	return true;
}

bool idAI::MoveToEnemy( void ) {
	return true;
}

bool idAI::MoveToEntity( idEntity *ent ) {
	int			areaNum;
	//aasPath_t	path;
	noVec3		pos;

	if ( !ent ) {
		StopMove( MOVE_STATUS_DEST_NOT_FOUND );
		return false;
	}

	pos = ent->GetPhysics()->GetOrigin();
	if ( ( move.moveType != MOVETYPE_FLY ) && ( ( move.moveCommand != MOVE_TO_ENTITY ) || ( move.goalEntityOrigin != pos ) ) ) {
		ent->GetFloorPos( 64.0f, pos );
	}

	if ( ReachedPos( pos, MOVE_TO_ENTITY ) ) {
		StopMove( MOVE_STATUS_DONE );
		return true;
	}

	move.toAreaNum = 0;
	//if ( aas ) {
	//	move.toAreaNum = PointReachableAreaNum( pos );
	//	aas->PushPointIntoAreaNum( move.toAreaNum, pos );

	//	areaNum	= PointReachableAreaNum( physicsObj.GetOrigin() );
	//	if ( !PathToGoal( path, areaNum, physicsObj.GetOrigin(), move.toAreaNum, pos ) ) {
	//		AI_DEST_UNREACHABLE = true;
	//		return false;
	//	}
	//}

	if ( !move.toAreaNum ) {
		// if only trying to update the entity position
		if ( move.moveCommand == MOVE_TO_ENTITY ) {
			//if ( !aas ) {
				// keep the move destination up to date for wandering
				move.moveDest = pos;
			//}
			return false;
		}

		if ( !NewWanderDir( pos ) ) {
			StopMove( MOVE_STATUS_DEST_UNREACHABLE );
			AI_DEST_UNREACHABLE = true;
			return false;
		}
	}

	if ( ( move.moveCommand != MOVE_TO_ENTITY ) || ( move.goalEntity.GetEntity() != ent ) ) {
		move.startTime		= gameLocal.time;
		move.goalEntity		= ent;
		move.moveCommand	= MOVE_TO_ENTITY;
	}

	move.moveDest			= pos;
	move.goalEntityOrigin	= ent->GetPhysics()->GetOrigin();
	move.moveStatus			= MOVE_STATUS_MOVING;
	move.speed				= fly_speed;
	AI_MOVE_DONE			= false;
	AI_DEST_UNREACHABLE		= false;
	AI_FORWARD				= true;

	return true;
}

bool idAI::MoveToPosition( const noVec3 &pos ) {
	noVec3		org;
	int			areaNum;
	//aasPath_t	path;

	if ( ReachedPos( pos, move.moveCommand ) ) {
		StopMove( MOVE_STATUS_DONE );
		return true;
	}

	org = pos;
	move.toAreaNum = 0;
	/*if ( aas ) {
		move.toAreaNum = PointReachableAreaNum( org );
		aas->PushPointIntoAreaNum( move.toAreaNum, org );

		areaNum	= PointReachableAreaNum( physicsObj.GetOrigin() );
		if ( !PathToGoal( path, areaNum, physicsObj.GetOrigin(), move.toAreaNum, org ) ) {
			StopMove( MOVE_STATUS_DEST_UNREACHABLE );
			AI_DEST_UNREACHABLE = true;
			return false;
		}
	}*/

	if ( !move.toAreaNum && !NewWanderDir( org ) ) {
		StopMove( MOVE_STATUS_DEST_UNREACHABLE );
		AI_DEST_UNREACHABLE = true;
		return false;
	}

	move.moveDest		= org;
	move.goalEntity		= NULL;
	move.moveCommand	= MOVE_TO_POSITION;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.startTime		= gameLocal.time;
	move.speed			= fly_speed;
	AI_MOVE_DONE		= false;
	AI_DEST_UNREACHABLE = false;
	AI_FORWARD			= true;

	return true;
}

bool idAI::MoveToCover( idEntity *entity, const noVec3 &pos ) {
	return true;
}

bool idAI::SlideToPosition( const noVec3 &pos, float time ) {
	StopMove( MOVE_STATUS_DONE );

	move.moveDest		= pos;
	move.goalEntity		= NULL;
	move.moveCommand	= MOVE_SLIDE_TO_POSITION;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.startTime		= gameLocal.time;
	move.duration		= idPhysics::SnapTimeToPhysicsFrame( SEC2MS( time ) );
	AI_MOVE_DONE		= false;
	AI_DEST_UNREACHABLE = false;
	AI_FORWARD			= false;

	if ( move.duration > 0 ) {
		move.moveDir = ( pos - physicsObj.GetOrigin() ) / MS2SEC( move.duration );
		if ( move.moveType != MOVETYPE_FLY ) {
			move.moveDir.z = 0.0f;
		}
		move.speed = move.moveDir.LengthFast();
	}

	return true;
}

bool idAI::WanderAround( void ) {
	StopMove( MOVE_STATUS_DONE );

	move.moveDest = physicsObj.GetOrigin() + viewAxis[ 0 ] * physicsObj.GetGravityAxis() * 256.0f;
	if ( !NewWanderDir( move.moveDest ) ) {
		StopMove( MOVE_STATUS_DEST_UNREACHABLE );
		AI_DEST_UNREACHABLE = true;
		return false;
	}

	move.moveCommand	= MOVE_WANDER;
	move.moveStatus		= MOVE_STATUS_MOVING;
	move.startTime		= gameLocal.time;
	move.speed			= fly_speed;
	AI_MOVE_DONE		= false;
	AI_FORWARD			= true;

	return true;
}

bool idAI::StepDirection( float dir ) {
	return true;
}

bool idAI::NewWanderDir( const noVec3 &dest ) {

	float	deltax, deltay;
	float	d[ 3 ];
	float	tdir, olddir, turnaround;

	move.nextWanderTime = gameLocal.time + ( gameLocal.random.RandomFloat() * 500 + 500 );

	olddir = noMath::AngleNormalize360( ( int )( current_yaw / 45 ) * 45 );
	turnaround = noMath::AngleNormalize360( olddir - 180 );

	noVec3 org = physicsObj.GetOrigin();
	deltax = dest.x - org.x;
	deltay = dest.y - org.y;
	if ( deltax > 10 ) {
		d[ 1 ]= 0;
	} else if ( deltax < -10 ) {
		d[ 1 ] = 180;
	} else {
		d[ 1 ] = DI_NODIR;
	}

	if ( deltay < -10 ) {
		d[ 2 ] = 270;
	} else if ( deltay > 10 ) {
		d[ 2 ] = 90;
	} else {
		d[ 2 ] = DI_NODIR;
	}

	// try direct route
	if ( d[ 1 ] != DI_NODIR && d[ 2 ] != DI_NODIR ) {
		if ( d[ 1 ] == 0 ) {
			tdir = d[ 2 ] == 90 ? 45 : 315;
		} else {
			tdir = d[ 2 ] == 90 ? 135 : 215;
		}

		if ( tdir != turnaround && StepDirection( tdir ) ) {
			return true;
		}
	}

	// try other directions
	if ( ( gameLocal.random.RandomInt() & 1 ) || abs( deltay ) > abs( deltax ) ) {
		tdir = d[ 1 ];
		d[ 1 ] = d[ 2 ];
		d[ 2 ] = tdir;
	}

	if ( d[ 1 ] != DI_NODIR && d[ 1 ] != turnaround && StepDirection( d[1] ) ) {
		return true;
	}

	if ( d[ 2 ] != DI_NODIR && d[ 2 ] != turnaround	&& StepDirection( d[ 2 ] ) ) {
		return true;
	}

	// there is no direct path to the player, so pick another direction
	if ( olddir != DI_NODIR && StepDirection( olddir ) ) {
		return true;
	}

	// randomly determine direction of search
	if ( gameLocal.random.RandomInt() & 1 ) {
		for( tdir = 0; tdir <= 315; tdir += 45 ) {
			if ( tdir != turnaround && StepDirection( tdir ) ) {
				return true;
			}
		}
	} else {
		for ( tdir = 315; tdir >= 0; tdir -= 45 ) {
			if ( tdir != turnaround && StepDirection( tdir ) ) {
				return true;
			}
		}
	}

	if ( turnaround != DI_NODIR && StepDirection( turnaround ) ) {
		return true;
	}

	// can't move
	StopMove( MOVE_STATUS_DEST_UNREACHABLE );
	return false;
}

bool idAI::FacingIdeal( void ) {
	float diff;

	if ( !turnRate ) {
		return true;
	}

	diff = noMath::AngleNormalize180( current_yaw - ideal_yaw );
	if ( noMath::Fabs( diff ) < 0.01f ) {
		// force it to be exact
		current_yaw = ideal_yaw;
		return true;
	}

	return false;
}

void idAI::Turn( void ) {
	float diff;
	float diff2;
	float turnAmount;
	animFlags_t animflags;

	if ( !turnRate ) {
		return;
	}

	// check if the animator has marker this anim as non-turning
	if ( !legsAnim.Disabled() && !legsAnim.AnimDone( 0 ) ) {
		animflags = legsAnim.GetAnimFlags();
	} else {
		animflags = torsoAnim.GetAnimFlags();
	}
	if ( animflags.ai_no_turn ) {
		return;
	}

	if ( anim_turn_angles && animflags.anim_turn ) {
		noMat3 rotateAxis;

		// set the blend between no turn and full turn
		float frac = anim_turn_amount / anim_turn_angles;
		animator.CurrentAnim( ANIMCHANNEL_LEGS )->SetSyncedAnimWeight( 0, 1.0f - frac );
		animator.CurrentAnim( ANIMCHANNEL_LEGS )->SetSyncedAnimWeight( 1, frac );
		animator.CurrentAnim( ANIMCHANNEL_TORSO )->SetSyncedAnimWeight( 0, 1.0f - frac );
		animator.CurrentAnim( ANIMCHANNEL_TORSO )->SetSyncedAnimWeight( 1, frac );

		// get the total rotation from the start of the anim
		animator.GetDeltaRotation( 0, gameLocal.time, rotateAxis );
		current_yaw = noMath::AngleNormalize180( anim_turn_yaw + rotateAxis[ 0 ].ToYaw() );
	} else {
		diff = noMath::AngleNormalize180( ideal_yaw - current_yaw );
		turnVel += AI_TURN_SCALE * diff * MS2SEC( gameLocal.msec );
		if ( turnVel > turnRate ) {
			turnVel = turnRate;
		} else if ( turnVel < -turnRate ) {
			turnVel = -turnRate;
		}
		turnAmount = turnVel * MS2SEC( gameLocal.msec );
		if ( ( diff >= 0.0f ) && ( turnAmount >= diff ) ) {
			turnVel = diff / MS2SEC( gameLocal.msec );
			turnAmount = diff;
		} else if ( ( diff <= 0.0f ) && ( turnAmount <= diff ) ) {
			turnVel = diff / MS2SEC( gameLocal.msec );
			turnAmount = diff;
		}
		current_yaw += turnAmount;
		current_yaw = noMath::AngleNormalize180( current_yaw );
		diff2 = noMath::AngleNormalize180( ideal_yaw - current_yaw );
		if ( noMath::Fabs( diff2 ) < 0.1f ) {
			current_yaw = ideal_yaw;
		}
	}

	viewAxis = noAngles( 0, current_yaw, 0 ).ToMat3();

	if ( ai_debugMove.GetBool() ) {
		const noVec3 &org = physicsObj.GetOrigin();
		//gameRenderWorld->DebugLine( colorRed, org, org + noAngles( 0, ideal_yaw, 0 ).ToForward() * 64, gameLocal.msec );
		//gameRenderWorld->DebugLine( colorGreen, org, org + noAngles( 0, current_yaw, 0 ).ToForward() * 48, gameLocal.msec );
		//gameRenderWorld->DebugLine( colorYellow, org, org + noAngles( 0, current_yaw + turnVel, 0 ).ToForward() * 32, gameLocal.msec );
	}
}

bool idAI::TurnToward( float yaw ) {
	ideal_yaw = noMath::AngleNormalize180( yaw );
	bool result = FacingIdeal();
	return result;
}

bool idAI::TurnToward( const noVec3 &pos ) {
	noVec3 dir;
	noVec3 local_dir;
	float lengthSqr;

	dir = pos - physicsObj.GetOrigin();
	physicsObj.GetGravityAxis().ProjectVector( dir, local_dir );
	local_dir.z = 0.0f;
	lengthSqr = local_dir.LengthSqr();
	if ( lengthSqr > Square( 2.0f ) || ( lengthSqr > Square( 0.1f ) && enemy.GetEntity() == NULL ) ) {
		ideal_yaw = noMath::AngleNormalize180( local_dir.ToYaw() );
	}

	bool result = FacingIdeal();
	return result;
}

void idAI::ClearEnemy( void ) {
	if ( move.moveCommand == MOVE_TO_ENEMY ) {
		StopMove( MOVE_STATUS_DEST_NOT_FOUND );
	}

	enemyNode.Remove();
	enemy				= NULL;
	AI_ENEMY_IN_FOV		= false;
	AI_ENEMY_VISIBLE	= false;
	AI_ENEMY_DEAD		= true;

	//SetChatSound();
}

bool idAI::EnemyPositionValid( void ) const {
	trace_t	tr;
	noVec3	muzzle;
	noMat3	axis;

	if ( !enemy.GetEntity() ) {
		return false;
	}

	if ( AI_ENEMY_VISIBLE ) {
		return true;
	}

	gameLocal.clip.TracePoint( tr, GetEyePosition(), lastVisibleEnemyPos + lastVisibleEnemyEyeOffset, MASK_OPAQUE, this );
	if ( tr.fraction < 1.0f ) {
		// can't see the area yet, so don't know if he's there or not
		return true;
	}

	return false;
}

void idAI::SetEnemyPosition( void ) {
	idActor		*enemyEnt = enemy.GetEntity();
	int			enemyAreaNum;
	int			areaNum;
	int			lastVisibleReachableEnemyAreaNum;
	//aasPath_t	path;
	noVec3		pos;
	bool		onGround;

	if ( !enemyEnt ) {
		return;
	}

	lastVisibleReachableEnemyPos = lastReachableEnemyPos;
	lastVisibleEnemyEyeOffset = enemyEnt->EyeOffset();
	lastVisibleEnemyPos = enemyEnt->GetPhysics()->GetOrigin();
	if ( move.moveType == MOVETYPE_FLY ) {
		pos = lastVisibleEnemyPos;
		onGround = true;
	} else {
		onGround = enemyEnt->GetFloorPos( 64.0f, pos );
		if ( enemyEnt->OnLadder() ) {
			onGround = false;
		}
	}

	if ( !onGround ) {
		if ( move.moveCommand == MOVE_TO_ENEMY ) {
			AI_DEST_UNREACHABLE = true;
		}
		return;
	}
#if i0
	if ( !aas ) {
		lastVisibleReachableEnemyPos = lastVisibleEnemyPos;
		if ( move.moveCommand == MOVE_TO_ENEMY ) {
			AI_DEST_UNREACHABLE = false;
		}
		enemyAreaNum = 0;
		areaNum = 0;
	} else {
		lastVisibleReachableEnemyAreaNum = move.toAreaNum;
		enemyAreaNum = PointReachableAreaNum( lastVisibleEnemyPos, 1.0f );
		if ( !enemyAreaNum ) {
			enemyAreaNum = PointReachableAreaNum( lastReachableEnemyPos, 1.0f );
			pos = lastReachableEnemyPos;
		}
		if ( !enemyAreaNum ) {
			if ( move.moveCommand == MOVE_TO_ENEMY ) {
				AI_DEST_UNREACHABLE = true;
			}
			areaNum = 0;
		} else {
			const noVec3 &org = physicsObj.GetOrigin();
			areaNum = PointReachableAreaNum( org );
			if ( PathToGoal( path, areaNum, org, enemyAreaNum, pos ) ) {
				lastVisibleReachableEnemyPos = pos;
				lastVisibleReachableEnemyAreaNum = enemyAreaNum;
				if ( move.moveCommand == MOVE_TO_ENEMY ) {
					AI_DEST_UNREACHABLE = false;
				}
			} else if ( move.moveCommand == MOVE_TO_ENEMY ) {
				AI_DEST_UNREACHABLE = true;
			}
		}
	}
#endif
	lastVisibleReachableEnemyPos = lastVisibleEnemyPos;
	if ( move.moveCommand == MOVE_TO_ENEMY ) {
		AI_DEST_UNREACHABLE = false;
	}
	enemyAreaNum = 0;
	areaNum = 0;

	if ( move.moveCommand == MOVE_TO_ENEMY ) {
		//if ( !aas ) {
		//	// keep the move destination up to date for wandering
		//	move.moveDest = lastVisibleReachableEnemyPos;
		//} else if ( enemyAreaNum ) {
		//	move.toAreaNum = lastVisibleReachableEnemyAreaNum;
		//	move.moveDest = lastVisibleReachableEnemyPos;
		//}
		move.moveDest = lastVisibleReachableEnemyPos;

		if ( move.moveType == MOVETYPE_FLY ) {
			predictedPath_t path;
			noVec3 end = move.moveDest;
			end.z += enemyEnt->EyeOffset().z + fly_offset;
			//idAI::PredictPath( this, aas, move.moveDest, end - move.moveDest, 1000, 1000, SE_BLOCKED, path );
			move.moveDest = path.endPos;
			//move.toAreaNum = PointReachableAreaNum( move.moveDest, 1.0f );
		}
	}


}

void idAI::UpdateEnemyPosition( void ) {
	idActor *enemyEnt = enemy.GetEntity();
	int				enemyAreaNum;
	int				areaNum;
	//aasPath_t		path;
	predictedPath_t predictedPath;
	noVec3			enemyPos;
	bool			onGround;

	if ( !enemyEnt ) {
		return;
	}

	const noVec3 &org = physicsObj.GetOrigin();

	if ( move.moveType == MOVETYPE_FLY ) {
		enemyPos = enemyEnt->GetPhysics()->GetOrigin();
		onGround = true;
	} else {
		onGround = enemyEnt->GetFloorPos( 64.0f, enemyPos );
		if ( enemyEnt->OnLadder() ) {
			onGround = false;
		}
	}

	if ( onGround ) {
		// when we don't have an AAS, we can't tell if an enemy is reachable or not,
		// so just assume that he is.
#if 0
		if ( !aas ) {
			enemyAreaNum = 0;
			lastReachableEnemyPos = enemyPos;
		} else {
			enemyAreaNum = PointReachableAreaNum( enemyPos, 1.0f );
			if ( enemyAreaNum ) {
				areaNum = PointReachableAreaNum( org );
				if ( PathToGoal( path, areaNum, org, enemyAreaNum, enemyPos ) ) {
					lastReachableEnemyPos = enemyPos;
				}
			}
		}
#endif
		enemyAreaNum = 0;
		lastReachableEnemyPos = enemyPos;
	}

	AI_ENEMY_IN_FOV		= false;
	AI_ENEMY_VISIBLE	= false;

	if ( CanSee( enemyEnt, false ) ) {
		AI_ENEMY_VISIBLE = true;
		if ( CheckFOV( enemyEnt->GetPhysics()->GetOrigin() ) ) {
			AI_ENEMY_IN_FOV = true;
		}

		SetEnemyPosition();
	} else {
		// check if we heard any sounds in the last frame
		/*if ( enemyEnt == gameLocal.GetAlertEntity() ) {
			float dist = ( enemyEnt->GetPhysics()->GetOrigin() - org ).LengthSqr();
			if ( dist < Square( AI_HEARING_RANGE ) ) {
				SetEnemyPosition();
			}
		}*/
	}

	if ( ai_debugMove.GetBool() ) {
		//gameRenderWorld->DebugBounds( colorLtGrey, enemyEnt->GetPhysics()->GetBounds(), lastReachableEnemyPos, gameLocal.msec );
		//gameRenderWorld->DebugBounds( colorWhite, enemyEnt->GetPhysics()->GetBounds(), lastVisibleReachableEnemyPos, gameLocal.msec );
	}

}

void idAI::SetEnemy( idActor *newEnemy ) {
	int enemyAreaNum;

	if ( AI_DEAD ) {
		ClearEnemy();
		return;
	}

	AI_ENEMY_DEAD = false;
	if ( !newEnemy ) {
		ClearEnemy();
	} else if ( enemy.GetEntity() != newEnemy ) {
		enemy = newEnemy;
		enemyNode.AddToEnd( newEnemy->enemyList );
		if ( newEnemy->health <= 0 ) {
			EnemyDead();
			return;
		}
		// let the monster know where the enemy is
		//newEnemy->GetAASLocation( aas, lastReachableEnemyPos, enemyAreaNum );
		SetEnemyPosition();
		//SetChatSound();

		lastReachableEnemyPos = lastVisibleEnemyPos;
		lastVisibleReachableEnemyPos = lastReachableEnemyPos;
		/*enemyAreaNum = PointReachableAreaNum( lastReachableEnemyPos, 1.0f );
		if ( aas && enemyAreaNum ) {
		aas->PushPointIntoAreaNum( enemyAreaNum, lastReachableEnemyPos );
		lastVisibleReachableEnemyPos = lastReachableEnemyPos;
		}*/
	}

}

/*
================
idAI::DamageFeedback

callback function for when another entity received damage from this entity.  damage can be adjusted and returned to the caller.

FIXME: This gets called when we call idPlayer::CalcDamagePoints from idAI::AttackMelee, which then checks for a saving throw,
possibly forcing a miss.  This is harmless behavior ATM, but is not intuitive.
================
*/
void idAI::DamageFeedback( idEntity *victim, idEntity *inflictor, int &damage ) {
#if 0
	if ( ( victim == this ) && inflictor->IsType( idProjectile::Type ) ) {
		// monsters only get half damage from their own projectiles
		damage = ( damage + 1 ) / 2;  // round up so we don't do 0 damage

	} else if ( victim == enemy.GetEntity() ) {
		AI_HIT_ENEMY = true;
	}
#endif
	if ( victim == enemy.GetEntity() ) {
		AI_HIT_ENEMY = true;
	}
}

/*
=====================
idAI::DirectDamage

Causes direct damage to an entity

kickDir is specified in the monster's coordinate system, and gives the direction
that the view kick and knockback should go
=====================
*/
void idAI::DirectDamage( const char *meleeDefName, idEntity *ent ) {
	const idDict *meleeDef;
	const char *p;
	const idSoundShader *shader;

	meleeDef = gameLocal.FindEntityDefDict( meleeDefName, false );
	if ( !meleeDef ) {
		gameLocal.Error( "Unknown damage def '%s' on '%s'", meleeDefName, name.c_str() );
	}

	if ( !ent->fl.takedamage ) {
		//const idSoundShader *shader = declManager->FindSound(meleeDef->GetString( "snd_miss" ));
		//StartSoundShader( shader, SND_CHANNEL_DAMAGE, 0, false, NULL );
		return;
	}

	//
	// do the damage
	//
	p = meleeDef->GetString( "snd_hit" );
	if ( p && *p ) {
		//shader = declManager->FindSound( p );
		//StartSoundShader( shader, SND_CHANNEL_DAMAGE, 0, false, NULL );
	}

	noVec3	kickDir;
	meleeDef->GetVector( "kickDir", "0 0 0", kickDir );

	noVec3	globalKickDir;
	globalKickDir = ( viewAxis * physicsObj.GetGravityAxis() ) * kickDir;

	ent->Damage( this, this, globalKickDir, meleeDefName, 1.0f, INVALID_JOINT );

	// end the attack if we're a multiframe attack
	EndAttack();
}

bool idAI::TestMelee( void ) const {
	trace_t trace;
	idActor *enemyEnt = enemy.GetEntity();

	if ( !enemyEnt || !melee_range ) {
		return false;
	}

	//FIXME: make work with gravity vector
	noVec3 org = physicsObj.GetOrigin();
	const idBounds &myBounds = physicsObj.GetBounds();
	idBounds bounds;

	// expand the bounds out by our melee range
	bounds[0][0] = -melee_range;
	bounds[0][1] = -melee_range;
	bounds[0][2] = myBounds[0][2] - 4.0f;
	bounds[1][0] = melee_range;
	bounds[1][1] = melee_range;
	bounds[1][2] = myBounds[1][2] + 4.0f;
	bounds.TranslateSelf( org );

	noVec3 enemyOrg = enemyEnt->GetPhysics()->GetOrigin();
	idBounds enemyBounds = enemyEnt->GetPhysics()->GetBounds();
	enemyBounds.TranslateSelf( enemyOrg );

	if ( ai_debugMove.GetBool() ) {
		//gameRenderWorld->DebugBounds( colorYellow, bounds, vec3_zero, gameLocal.msec );
	}

	if ( !bounds.IntersectsBounds( enemyBounds ) ) {
		return false;
	}

	noVec3 start = GetEyePosition();
	noVec3 end = enemyEnt->GetEyePosition();

	gameLocal.clip.TracePoint( trace, start, end, MASK_SHOT_BOUNDINGBOX, this );
	if ( ( trace.fraction == 1.0f ) || ( gameLocal.GetTraceEntity( trace ) == enemyEnt ) ) {
		return true;
	}

	return false;

}

/*
=====================
idAI::AttackMelee

jointname allows the endpoint to be exactly specified in the model,
as for the commando tentacle.  If not specified, it will be set to
the facing direction + melee_range.

kickDir is specified in the monster's coordinate system, and gives the direction
that the view kick and knockback should go
=====================
*/
bool idAI::AttackMelee( const char *meleeDefName ) {
	const idDict *meleeDef;
	idActor *enemyEnt = enemy.GetEntity();
	const char *p;
	const idSoundShader *shader;

	meleeDef = gameLocal.FindEntityDefDict( meleeDefName, false );
	if ( !meleeDef ) {
		gameLocal.Error( "Unknown melee '%s'", meleeDefName );
	}

	if ( !enemyEnt ) {
		p = meleeDef->GetString( "snd_miss" );
		if ( p && *p ) {
			//shader = declManager->FindSound( p );
			//StartSoundShader( shader, SND_CHANNEL_DAMAGE, 0, false, NULL );
		}
		return false;
	}

	// check for the "saving throw" automatic melee miss on lethal blow
	// stupid place for this.
	bool forceMiss = false;
#if 0
	if ( enemyEnt->IsType( idPlayer::Type ) && g_skill.GetInteger() < 2 ) {
		int	damage, armor;
		idPlayer *player = static_cast<idPlayer*>( enemyEnt );
		player->CalcDamagePoints( this, this, meleeDef, 1.0f, INVALID_JOINT, &damage, &armor );

		if ( enemyEnt->health <= damage ) {
			int	t = gameLocal.time - player->lastSavingThrowTime;
			if ( t > SAVING_THROW_TIME ) {
				player->lastSavingThrowTime = gameLocal.time;
				t = 0;
			}
			if ( t < 1000 ) {
				gameLocal.Printf( "Saving throw.\n" );
				forceMiss = true;
			}
		}
	}
#endif
	// make sure the trace can actually hit the enemy
	if ( forceMiss || !TestMelee() ) {
		// missed
		p = meleeDef->GetString( "snd_miss" );
		if ( p && *p ) {
			//shader = declManager->FindSound( p );
			//StartSoundShader( shader, SND_CHANNEL_DAMAGE, 0, false, NULL );
		}
		return false;
	}

	//
	// do the damage
	//
	p = meleeDef->GetString( "snd_hit" );
	if ( p && *p ) {
		//shader = declManager->FindSound( p );
		//StartSoundShader( shader, SND_CHANNEL_DAMAGE, 0, false, NULL );
	}

	noVec3	kickDir;
	meleeDef->GetVector( "kickDir", "0 0 0", kickDir );

	noVec3	globalKickDir;
	globalKickDir = ( viewAxis * physicsObj.GetGravityAxis() ) * kickDir;

	enemyEnt->Damage( this, this, globalKickDir, meleeDefName, 1.0f, INVALID_JOINT );

	lastAttackTime = gameLocal.time;

	return true;
}


void idAI::BeginAttack( const char *name ) {
	attack = name;
	lastAttackTime = gameLocal.time;
}

void idAI::EndAttack( void ) {
	attack = "";
}

void idAI::PushWithAF( void ) {
	int i, j;
	afTouch_t touchList[ MAX_GENTITIES ];
	idEntity *pushed_ents[ MAX_GENTITIES ];
	idEntity *ent;
	noVec3 vel;
	int num_pushed;

	num_pushed = 0;
	af.ChangePose( this, gameLocal.time );
	int num = af.EntitiesTouchingAF( touchList );
	for( i = 0; i < num; i++ ) {
		//if ( touchList[ i ].touchedEnt->IsType( idProjectile::Type ) ) {
		//	// skip projectiles
		//	continue;
		//}

		// make sure we havent pushed this entity already.  this avoids causing double damage
		for( j = 0; j < num_pushed; j++ ) {
			if ( pushed_ents[ j ] == touchList[ i ].touchedEnt ) {
				break;
			}
		}
		if ( j >= num_pushed ) {
			ent = touchList[ i ].touchedEnt;
			pushed_ents[num_pushed++] = ent;
			vel = ent->GetPhysics()->GetAbsBounds().GetCenter() - touchList[ i ].touchedByBody->GetWorldOrigin();
			vel.Normalize();
			if ( attack.Length() && ent->IsType( idActor::Type ) ) {
				ent->Damage( this, this, vel, attack, 1.0f, INVALID_JOINT );
			} else {
				ent->GetPhysics()->SetLinearVelocity( 100.0f * vel, touchList[ i ].touchedClipModel->GetId() );
			}
		}
	}
}

/*
================
idAI::UpdateAnimationControllers
================
*/
bool idAI::UpdateAnimationControllers( void ) {
	noVec3		local;
	noVec3		focusPos;
	noQuat		jawQuat;
	noVec3		left;
	noVec3 		dir;
	noVec3 		orientationJointPos;
	noVec3 		localDir;
	noAngles 	newLookAng;
	noAngles	diff;
	noMat3		mat;
	noMat3		axis;
	noMat3		orientationJointAxis;
	//idAFAttachment	*headEnt = head.GetEntity();
	noVec3		eyepos;
	noVec3		pos;
	int			i;
	noAngles	jointAng;
	float		orientationJointYaw;

	if ( AI_DEAD ) {
		return idActor::UpdateAnimationControllers();
	}

	if ( orientationJoint == INVALID_JOINT ) {
		orientationJointAxis = viewAxis;
		orientationJointPos = physicsObj.GetOrigin();
		orientationJointYaw = current_yaw;
	} else {
		GetJointWorldTransform( orientationJoint, gameLocal.time, orientationJointPos, orientationJointAxis );
		orientationJointYaw = orientationJointAxis[ 2 ].ToYaw();
		orientationJointAxis = noAngles( 0.0f, orientationJointYaw, 0.0f ).ToMat3();
	}

	if ( focusJoint != INVALID_JOINT ) {
		/*if ( headEnt ) {
			headEnt->GetJointWorldTransform( focusJoint, gameLocal.time, eyepos, axis );
		} else*/ {
			GetJointWorldTransform( focusJoint, gameLocal.time, eyepos, axis );
		}
		eyeOffset.z = eyepos.z - physicsObj.GetOrigin().z;
		if ( ai_debugMove.GetBool() ) {
			//gameRenderWorld->DebugLine( colorRed, eyepos, eyepos + orientationJointAxis[ 0 ] * 32.0f, gameLocal.msec );
		}
	} else {
		eyepos = GetEyePosition();
	}

	//if ( headEnt ) {
	//	CopyJointsFromBodyToHead();
	//}

	// Update the IK after we've gotten all the joint positions we need, but before we set any joint positions.
	// Getting the joint positions causes the joints to be updated.  The IK gets joint positions itself (which
	// are already up to date because of getting the joints in this function) and then sets their positions, which
	// forces the heirarchy to be updated again next time we get a joint or present the model.  If IK is enabled,
	// or if we have a seperate head, we end up transforming the joints twice per frame.  Characters with no
	// head entity and no ik will only transform their joints once.  Set g_debuganim to the current entity number
	// in order to see how many times an entity transforms the joints per frame.
	idActor::UpdateAnimationControllers();

	idEntity *focusEnt = focusEntity.GetEntity();
	if ( !allowJointMod || !allowEyeFocus || ( gameLocal.time >= focusTime ) ) {
		focusPos = GetEyePosition() + orientationJointAxis[ 0 ] * 512.0f;
	} else if ( focusEnt == NULL ) {
		// keep looking at last position until focusTime is up
		focusPos = currentFocusPos;
	} else if ( focusEnt == enemy.GetEntity() ) {
		focusPos = lastVisibleEnemyPos + lastVisibleEnemyEyeOffset - eyeVerticalOffset * enemy.GetEntity()->GetPhysics()->GetGravityNormal();
	} else if ( focusEnt->IsType( idActor::Type ) ) {
		focusPos = static_cast<idActor *>( focusEnt )->GetEyePosition() - eyeVerticalOffset * focusEnt->GetPhysics()->GetGravityNormal();
	} else {
		focusPos = focusEnt->GetPhysics()->GetOrigin();
	}

	currentFocusPos = currentFocusPos + ( focusPos - currentFocusPos ) * eyeFocusRate;

	// determine yaw from origin instead of from focus joint since joint may be offset, which can cause us to bounce between two angles
	dir = focusPos - orientationJointPos;
	newLookAng.yaw = noMath::AngleNormalize180( dir.ToYaw() - orientationJointYaw );
	newLookAng.roll = 0.0f;
	newLookAng.pitch = 0.0f;

#if 0
	gameRenderWorld->DebugLine( colorRed, orientationJointPos, focusPos, gameLocal.msec );
	gameRenderWorld->DebugLine( colorYellow, orientationJointPos, orientationJointPos + orientationJointAxis[ 0 ] * 32.0f, gameLocal.msec );
	gameRenderWorld->DebugLine( colorGreen, orientationJointPos, orientationJointPos + newLookAng.ToForward() * 48.0f, gameLocal.msec );
#endif

	// determine pitch from joint position
	dir = focusPos - eyepos;
	dir.NormalizeFast();
	orientationJointAxis.ProjectVector( dir, localDir );
	newLookAng.pitch = -noMath::AngleNormalize180( localDir.ToPitch() );
	newLookAng.roll	= 0.0f;

	diff = newLookAng - lookAng;

	if ( eyeAng != diff ) {
		eyeAng = diff;
		eyeAng.Clamp( eyeMin, eyeMax );
		noAngles angDelta = diff - eyeAng;
		if ( !angDelta.Compare( ang_zero, 0.1f ) ) {
			alignHeadTime = gameLocal.time;
		} else {
			alignHeadTime = gameLocal.time + ( 0.5f + 0.5f * gameLocal.random.RandomFloat() ) * focusAlignTime;
		}
	}

	if ( noMath::Fabs( newLookAng.yaw ) < 0.1f ) {
		alignHeadTime = gameLocal.time;
	}

	if ( ( gameLocal.time >= alignHeadTime ) || ( gameLocal.time < forceAlignHeadTime ) ) {
		alignHeadTime = gameLocal.time + ( 0.5f + 0.5f * gameLocal.random.RandomFloat() ) * focusAlignTime;
		destLookAng = newLookAng;
		destLookAng.Clamp( lookMin, lookMax );
	}

	diff = destLookAng - lookAng;
	if ( ( lookMin.pitch == -180.0f ) && ( lookMax.pitch == 180.0f ) ) {
		if ( ( diff.pitch > 180.0f ) || ( diff.pitch <= -180.0f ) ) {
			diff.pitch = 360.0f - diff.pitch;
		}
	}
	if ( ( lookMin.yaw == -180.0f ) && ( lookMax.yaw == 180.0f ) ) {
		if ( diff.yaw > 180.0f ) {
			diff.yaw -= 360.0f;
		} else if ( diff.yaw <= -180.0f ) {
			diff.yaw += 360.0f;
		}
	}
	lookAng = lookAng + diff * headFocusRate;
	lookAng.Normalize180();

	jointAng.roll = 0.0f;
	for( i = 0; i < lookJoints.Num(); i++ ) {
		jointAng.pitch	= lookAng.pitch * lookJointAngles[ i ].pitch;
		jointAng.yaw	= lookAng.yaw * lookJointAngles[ i ].yaw;
		animator.SetJointAxis( lookJoints[ i ], JOINTMOD_WORLD, jointAng.ToMat3() );
	}

	if ( move.moveType == MOVETYPE_FLY ) {
		// lean into turns
		AdjustFlyingAngles();
	}

/*	if ( headEnt ) {
		idAnimator *headAnimator = headEnt->GetAnimator();

		if ( allowEyeFocus ) {
			noMat3 eyeAxis = ( lookAng + eyeAng ).ToMat3(); noMat3 headTranspose = headEnt->GetPhysics()->GetAxis().Transpose();
			axis =  eyeAxis * orientationJointAxis;
			left = axis[ 1 ] * eyeHorizontalOffset;
			eyepos -= headEnt->GetPhysics()->GetOrigin();
			headAnimator->SetJointPos( leftEyeJoint, JOINTMOD_WORLD_OVERRIDE, eyepos + ( axis[ 0 ] * 64.0f + left ) * headTranspose );
			headAnimator->SetJointPos( rightEyeJoint, JOINTMOD_WORLD_OVERRIDE, eyepos + ( axis[ 0 ] * 64.0f - left ) * headTranspose );
		} else {
			headAnimator->ClearJoint( leftEyeJoint );
			headAnimator->ClearJoint( rightEyeJoint );
		}
	} else*/ {
		if ( allowEyeFocus ) {
			noMat3 eyeAxis = ( lookAng + eyeAng ).ToMat3();
			axis =  eyeAxis * orientationJointAxis;
			left = axis[ 1 ] * eyeHorizontalOffset;
			eyepos += axis[ 0 ] * 64.0f - physicsObj.GetOrigin();
			animator.SetJointPos( leftEyeJoint, JOINTMOD_WORLD_OVERRIDE, eyepos + left );
			animator.SetJointPos( rightEyeJoint, JOINTMOD_WORLD_OVERRIDE, eyepos - left );
		} else {
			animator.ClearJoint( leftEyeJoint );
			animator.ClearJoint( rightEyeJoint );
		}
	}

	return true;
}