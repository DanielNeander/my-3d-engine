#include "stdafx.h"
#pragma hdrstop
#include <GameApp/MD5/MD5Material.h>
#include <GameApp/MD5/MD5MapFile.h>
#include <GameApp/Collision/CollisionModel.h>
#include <GameApp/Collision/CollisionModel_local.h>
#include "GameApp/Physics/Physics.h"
#include "GameApp/Physics/Force.h"
#include "GameApp/Physics/Clip.h"
#include "GameApp/Physics/Physics_Base.h"
#include "GameApp/Physics/Physics_AF.h"
#include "MD5Model.h"
#include "MD5ModelManager.h"
#include "GameApp/Animation/MD5Anim.h"
#include "MD5Entity.h"
#include <GameApp/MD5/MD5Game.h>
#include <GameApp/MD5/DeclAF.h>
#include "MD5AF.h"
#include "MD5AFEntity.h"



/*
===============================================================================

  idAFEntity_Base

===============================================================================
*/

const idEventDef EV_SetConstraintPosition( "SetConstraintPosition", "sv" );

CLASS_DECLARATION( idAnimatedEntity, idAFEntity_Base )
	EVENT( EV_SetConstraintPosition,	idAFEntity_Base::Event_SetConstraintPosition )
END_CLASS

static const float BOUNCE_SOUND_MIN_VELOCITY	= 80.0f;
static const float BOUNCE_SOUND_MAX_VELOCITY	= 200.0f;

/*
================
idAFEntity_Base::idAFEntity_Base
================
*/
idAFEntity_Base::idAFEntity_Base( void ) {
	combatModel = NULL;
	combatModelContents = 0;
	nextSoundTime = 0;
	spawnOrigin.Zero();
	spawnAxis.Identity();
}

/*
================
idAFEntity_Base::~idAFEntity_Base
================
*/
idAFEntity_Base::~idAFEntity_Base( void ) {
	delete combatModel;
	combatModel = NULL;
}


/*
================
idAFEntity_Base::Spawn
================
*/
void idAFEntity_Base::Spawn( void ) {
	spawnOrigin = GetPhysics()->GetOrigin();
	spawnAxis = GetPhysics()->GetAxis();
	nextSoundTime = 0;
}

/*
================
idAFEntity_Base::Event_SetConstraintPosition
================
*/
void idAFEntity_Base::Event_SetConstraintPosition( const char *name, const noVec3 &pos ) {
	//af.SetConstraintPosition( name, pos );
}

bool idAFEntity_Base::LoadAF( void )
{
	idStr fileName;

	if ( !spawnArgs.GetString( "articulatedFigure", "*unknown*", fileName ) ) {
		return false;
	}

	af.SetAnimator( GetAnimator() );
	if ( !af.Load( this, fileName ) ) {
		gameLocal.Error( "idAFEntity_Base::LoadAF: Couldn't load af file '%s' on entity '%s'", fileName.c_str(), name.c_str() );
	}

	af.Start();

	af.GetPhysics()->Rotate( spawnAxis.ToRotation() );
	af.GetPhysics()->Translate( spawnOrigin );

	LoadState( spawnArgs );

	af.UpdateAnimation();
	animator.CreateFrame( gameLocal.time, true );
	//UpdateVisuals();

}

/*
================
idAFEntity_Base::SaveState
================
*/
void idAFEntity_Base::SaveState( idDict &args ) const {
	const idKeyValue *kv;

	// save the ragdoll pose
	af.SaveState( args );

	// save all the bind constraints
	kv = spawnArgs.MatchPrefix( "bindConstraint ", NULL );
	while ( kv ) {
		args.Set( kv->GetKey(), kv->GetValue() );
		kv = spawnArgs.MatchPrefix( "bindConstraint ", kv );
	}

	// save the bind if it exists
	kv = spawnArgs.FindKey( "bind" );
	if ( kv ) {
		args.Set( kv->GetKey(), kv->GetValue() );
	}
	kv = spawnArgs.FindKey( "bindToJoint" );
	if ( kv ) {
		args.Set( kv->GetKey(), kv->GetValue() );
	}
	kv = spawnArgs.FindKey( "bindToBody" );
	if ( kv ) {
		args.Set( kv->GetKey(), kv->GetValue() );
	}
}

/*
================
idAFEntity_Base::LoadState
================
*/
void idAFEntity_Base::LoadState( const idDict &args ) {
	af.LoadState( args );
}

/*
================
idAFEntity_Base::SetCombatModel
================
*/
void idAFEntity_Base::SetCombatModel( void ) {
	if ( combatModel ) {
		combatModel->Unlink();
		combatModel->LoadModel( modelDefHandle );
	} else {
		combatModel = new idClipModel( modelDefHandle );
	}
}

/*
================
idAFEntity_Base::LinkCombat
================
*/
void idAFEntity_Base::LinkCombat( void ) {
	if ( fl.hidden ) {
		return;
	}
	if ( combatModel ) {
		combatModel->Link( gameLocal.clip, this, 0, renderEntity.origin, renderEntity.axis, modelDefHandle );
	}
}

/*
================
idAFEntity_Base::UnlinkCombat
================
*/
void idAFEntity_Base::UnlinkCombat( void ) {
	if ( combatModel ) {
		combatModel->Unlink();
	}
}

/*
================
idAFEntity_Base::GetCombatModel
================
*/
idClipModel *idAFEntity_Base::GetCombatModel( void ) const {
	return combatModel;
}

/*
================
idAFEntity_Base::DropAFs

  The entity should have the following key/value pairs set:
	"def_drop<type>AF"		"af def"
	"drop<type>Skin"		"skin name"
  To drop multiple articulated figures the following key/value pairs can be used:
	"def_drop<type>AF*"		"af def"
  where * is an aribtrary string.
================
*/
void idAFEntity_Base::DropAFs( idEntity *ent, const char *type, idList<idEntity *> *list ) {
	const idKeyValue *kv;
	const char *skinName;
	idEntity *newEnt;
	idAFEntity_Base *af;
	idDict args;
	const idDeclSkin *skin;

	// drop the articulated figures
	kv = ent->spawnArgs.MatchPrefix( va( "def_drop%sAF", type ), NULL );
	while ( kv ) {

		args.Set( "classname", kv->GetValue() );
		gameLocal.SpawnEntityDef( args, &newEnt );

		if ( newEnt && newEnt->IsType( idAFEntity_Base::Type ) ) {
			af = static_cast<idAFEntity_Base *>(newEnt);
			af->GetPhysics()->SetOrigin( ent->GetPhysics()->GetOrigin() );
			af->GetPhysics()->SetAxis( ent->GetPhysics()->GetAxis() );
			af->af.SetupPose( ent, gameLocal.time );
			if ( list ) {
				list->Append( af );
			}
		}

		kv = ent->spawnArgs.MatchPrefix( va( "def_drop%sAF", type ), kv );
	}

	// change the skin to hide all the dropped articulated figures
	skinName = ent->spawnArgs.GetString( va( "skin_drop%s", type ) );
	if ( skinName[0] ) {
		skin = declManager->FindSkin( skinName );
		//ent->SetSkin( skin );
	}
}


/*
===============================================================================

idAFEntity_Gibbable

===============================================================================
*/

const idEventDef EV_Gib( "gib", "s" );
const idEventDef EV_Gibbed( "<gibbed>" );

CLASS_DECLARATION( idAFEntity_Base, idAFEntity_Gibbable )
	EVENT( EV_Gib,		idAFEntity_Gibbable::Event_Gib )
	//EVENT( EV_Gibbed,	idAFEntity_Base::Event_Remove )
END_CLASS

/*
================
idAFEntity_Gibbable::idAFEntity_Gibbable
================
*/
idAFEntity_Gibbable::idAFEntity_Gibbable( void ) {
	skeletonModel = NULL;
	skeletonModelDefHandle = -1;
	gibbed = false;
}

/*
================
idAFEntity_Gibbable::~idAFEntity_Gibbable
================
*/
idAFEntity_Gibbable::~idAFEntity_Gibbable() {
	if ( skeletonModelDefHandle != -1 ) {
		//gameRenderWorld->FreeEntityDef( skeletonModelDefHandle );
		skeletonModelDefHandle = -1;
	}
}

/*
================
idAFEntity_Gibbable::Spawn
================
*/
void idAFEntity_Gibbable::Spawn( void ) {
	InitSkeletonModel();

	gibbed = false;
}

/*
================
idAFEntity_Gibbable::InitSkeletonModel
================
*/
void idAFEntity_Gibbable::InitSkeletonModel( void ) {
	const char *modelName;
	const idDeclModelDef *modelDef;

	skeletonModel = NULL;
	skeletonModelDefHandle = -1;

	modelName = spawnArgs.GetString( "model_gib" );

	modelDef = NULL;
	if ( modelName[0] != '\0' ) {
		modelDef = static_cast<const idDeclModelDef *>( declManager->FindType( DECL_MODELDEF, modelName, false ) );
		if ( modelDef ) {
			skeletonModel = modelDef->ModelHandle();
		} else {
			skeletonModel = renderModelManager->FindModel( modelName );
		}
		if ( skeletonModel != NULL && renderEntity.hModel != NULL ) {
			if ( skeletonModel->NumJoints() != renderEntity.hModel->NumJoints() ) {
				gameLocal.Error( "gib model '%s' has different number of joints than model '%s'",
					skeletonModel->Name(), renderEntity.hModel->Name() );
			}
		}
	}
}

/*
============
idAFEntity_Gibbable::Event_Gib
============
*/
void idAFEntity_Gibbable::Event_Gib( const char *damageDefName ) {
	Gib( noVec3( 0, 0, 1 ), damageDefName );
}

void idAFEntity_Gibbable::Gib( const noVec3 &dir, const char *damageDefName ) {
	// only gib once
	if ( gibbed ) {
		return;
	}

	const idDict *damageDef = gameLocal.FindEntityDefDict( damageDefName );
	if ( !damageDef ) {
		gameLocal.Error( "Unknown damageDef '%s'", damageDefName );
	}

	if ( damageDef->GetBool( "gibNonSolid" ) ) {
		GetAFPhysics()->SetContents( 0 );
		GetAFPhysics()->SetClipMask( 0 );
		GetAFPhysics()->UnlinkClip();
		GetAFPhysics()->PutToRest();
	} else {
		GetAFPhysics()->SetContents( CONTENTS_CORPSE );
		GetAFPhysics()->SetClipMask( CONTENTS_SOLID );
	}

	UnlinkCombat();

	//if ( g_bloodEffects.GetBool() ) {
	//	if ( gameLocal.time > gameLocal.GetGibTime() ) {
	//		gameLocal.SetGibTime( gameLocal.time + GIB_DELAY );
	//		SpawnGibs( dir, damageDefName );
	//		renderEntity.noShadow = true;
	//		renderEntity.shaderParms[ SHADERPARM_TIME_OF_DEATH ] = gameLocal.time * 0.001f;
	//		//StartSound( "snd_gibbed", SND_CHANNEL_ANY, 0, false, NULL );
	//		gibbed = true;
	//	}
	//} else {
		gibbed = true;
	//}

}


/*
================
idAFEntity_Gibbable::Damage
================
*/
void idAFEntity_Gibbable::Damage( idEntity *inflictor, idEntity *attacker, const noVec3 &dir, const char *damageDefName, const float damageScale, const int location ) {
	if ( !fl.takedamage ) {
		return;
	}
	idAFEntity_Base::Damage( inflictor, attacker, dir, damageDefName, damageScale, location );
	if ( health < -20 && spawnArgs.GetBool( "gib" ) ) {
		Gib( dir, damageDefName );
	}
}

void idAFEntity_Gibbable::SpawnGibs( const noVec3 &dir, const char *damageDefName ) {
	int i;
	bool gibNonSolid;
	noVec3 entityCenter, velocity;
	idList<idEntity *> list;

	assert( !gameLocal.isClient );

	const idDict *damageDef = gameLocal.FindEntityDefDict( damageDefName );
	if ( !damageDef ) {
		gameLocal.Error( "Unknown damageDef '%s'", damageDefName );
	}
	// spawn gib articulated figures
	idAFEntity_Base::DropAFs( this, "gib", &list );

	// spawn gib items
	//idMoveableItem::DropItems( this, "gib", &list );

	// blow out the gibs in the given direction away from the center of the entity
	entityCenter = GetPhysics()->GetAbsBounds().GetCenter();
	gibNonSolid = damageDef->GetBool( "gibNonSolid" );
	for ( i = 0; i < list.Num(); i++ ) {
		if ( gibNonSolid ) {
			list[i]->GetPhysics()->SetContents( 0 );
			list[i]->GetPhysics()->SetClipMask( 0 );
			list[i]->GetPhysics()->UnlinkClip();
			list[i]->GetPhysics()->PutToRest();
		} else {
#ifdef _D3XP
			list[i]->GetPhysics()->SetContents( 0 );
#else
			list[i]->GetPhysics()->SetContents( CONTENTS_CORPSE );
#endif
			list[i]->GetPhysics()->SetClipMask( CONTENTS_SOLID );
			velocity = list[i]->GetPhysics()->GetAbsBounds().GetCenter() - entityCenter;
			velocity.NormalizeFast();
			velocity += ( i & 1 ) ? dir : -dir;
			list[i]->GetPhysics()->SetLinearVelocity( velocity * 75.0f );
		}
#ifdef _D3XP
		// Don't allow grabber to pick up temporary gibs
		list[i]->noGrab = true;
#endif
		list[i]->GetRenderEntity()->noShadow = true;
		list[i]->GetRenderEntity()->shaderParms[ SHADERPARM_TIME_OF_DEATH ] = gameLocal.time * 0.001f;
		//list[i]->PostEventSec( &EV_Remove, 4.0f );
	}
}



/*
================
idAFEntity_Base::UpdateAnimationControllers
================
*/
bool idAFEntity_Base::UpdateAnimationControllers( void ) {
	if ( af.IsActive() ) {
		if ( af.UpdateAnimation() ) {
			return true;
		}
	}
	return false;
}

void idAFEntity_Base::AddBindConstraints( void ) {
	af.AddBindConstraints();
}

void idAFEntity_Base::RemoveBindConstraints( void ) {
	af.RemoveBindConstraints();
}
