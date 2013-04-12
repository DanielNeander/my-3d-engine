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

#ifndef __GAME_LOCAL_H__
#define	__GAME_LOCAL_H__


#include "UsercmdGen.h"
/*
===============================================================================

	Local implementation of the public game interface.

===============================================================================
*/

class idEntity;
class idClip;
class idPush;
class idDeclEntityDef;
class idMapFile;
class idWorldspawn;

#define	MAX_CLIENTS				32
#define	GENTITYNUM_BITS			12
#define	MAX_GENTITIES			(1<<GENTITYNUM_BITS)
#define	ENTITYNUM_NONE			(MAX_GENTITIES-1)
#define	ENTITYNUM_WORLD			(MAX_GENTITIES-2)
#define	ENTITYNUM_MAX_NORMAL	(MAX_GENTITIES-2)

#include "GameApp/Physics/Push.h"

// content masks
#define	MASK_ALL					(-1)
#define	MASK_SOLID					(CONTENTS_SOLID)
#define	MASK_MONSTERSOLID			(CONTENTS_SOLID|CONTENTS_MONSTERCLIP|CONTENTS_BODY)
#define	MASK_PLAYERSOLID			(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY)
#define	MASK_DEADSOLID				(CONTENTS_SOLID|CONTENTS_PLAYERCLIP)
#define	MASK_WATER					(CONTENTS_WATER)
#define	MASK_OPAQUE					(CONTENTS_OPAQUE)
#define	MASK_SHOT_RENDERMODEL		(CONTENTS_SOLID|CONTENTS_RENDERMODEL)
#define	MASK_SHOT_BOUNDINGBOX		(CONTENTS_SOLID|CONTENTS_BODY)

const float DEFAULT_GRAVITY			= 1066.0f;
#define DEFAULT_GRAVITY_STRING		"1066"
const noVec3 DEFAULT_GRAVITY_VEC3( 0, 0, -DEFAULT_GRAVITY );

template< class type >
class idEntityPtr {
public:
	idEntityPtr();

	// save games
	//void					Save( idSaveGame *savefile ) const;					// archives object for save game file
	//void					Restore( idRestoreGame *savefile );					// unarchives object from save game file

	idEntityPtr<type> &		operator=( type *ent );

	// synchronize entity pointers over the network
	int						GetSpawnId( void ) const { return spawnId; }
	bool					SetSpawnId( int id );
	bool					UpdateSpawnId( void );

	bool					IsValid( void ) const;
	type *					GetEntity( void ) const;
	int						GetEntityNum( void ) const;

private:
	int						spawnId;
};

class idGameLocal {
public:

	usercmd_t				usercmds[MAX_CLIENTS];	// client input commands
	idEntity *				entities[MAX_GENTITIES];// index to entities
	int						spawnIds[MAX_GENTITIES];// for use in idEntityPtr
	int						num_entities;			// current number <= MAX_GENTITIES
	int						firstFreeIndex;			// first free index in the entities array
	idHashIndex				entityHash;				// hash table to quickly find entities by name
	idWorldspawn *			world;					// world entity
	
	void					Printf( const char *fmt, ... ) const id_attribute((format(printf,2,3)));	
	void Error( const char *fmt, ... ) const;
	void DWarning( const char *fmt, ... ) const;
	void Warning( const char *fmt, ... ) const;

	// Initializes all map variables common to both save games and spawned games
	void					LoadMap( const char *mapName, int randseed );

	const noVec3 &			GetGravity( void ) const;
	const idDeclEntityDef *FindEntityDef( const char *name, bool makeDefault ) const;
	const idDict *FindEntityDefDict( const char *name, bool makeDefault = true ) const;

	void					RunFrame();
	
	class MD5Scene*			mCurrScene;
	

	noVec3					gravity;				// global gravity vector

	idLinkList<idEntity>	spawnedEntities;		// all spawned entities
	idLinkList<idEntity>	activeEntities;			// all thinking entities (idEntity::thinkFlags != 0)

	idClip					clip;					// collision detection
	idPush					push;

	int						numEntitiesToDeactivate;// number of entities that became inactive in current frame
	bool					sortPushers;			// true if active lists needs to be reordered to place pushers at the front
	bool					sortTeamMasters;		// true if active lists needs to be reordered to place physics team masters before their slaves
	int						framenum;
	int						previousTime;			// time in msec of last frame
	int						time;					// in msec
	static const int		msec = USERCMD_MSEC;	// time since last update in milliseconds

	idRandom				random;					// random number generator used throughout the game

	bool					isMultiplayer;			// set if the game is run in multiplayer mode
	bool					isServer;				// set if the game is run for a dedicated or listen server
	bool					isClient;				// set if the game is run for a client


	idEntity *				SpawnEntityType( const idTypeInfo &classdef, const idDict *args = NULL, bool bIsClientReadSnapshot = false );
	bool					SpawnEntityDef( const idDict &args, idEntity **ent = NULL, bool setDefaults = true );

	void					RegisterEntity( idEntity *ent );
	void					UnregisterEntity( idEntity *ent );
	void Clear( void );
	void ComputeSlowMsec();
	bool RemoveEntityFromHash( const char *name, idEntity *ent );
	idEntity *GetTraceEntity( const trace_t &trace ) const;
	void MapPopulate( void );
	void SpawnMapEntities( void );
	void SetSkill( int value );

private:
	const static int		INITIAL_SPAWN_COUNT = 1;

	idStr					mapFileName;			// name of the map, empty string if no map loaded
	idMapFile *				mapFile;				// will be NULL during the game unless in-game editing is used
	int						spawnCount;
	int						mapSpawnCount;			// it's handy to know which entities are part of the map
	int						nextGibTime;

	idDict					spawnArgs;				// spawn args used during entity spawning  FIXME: shouldn't be necessary anymore

	//idEventQueue			eventQueue;
	//idEventQueue			savedEventQueue;
	

	int						realClientTime;			// real client time
	bool					isNewFrame;				// true if this is a new game frame, not a rerun due to prediction
	float					clientSmoothing;		// smoothing of other clients in the view
};



extern idGameLocal			gameLocal;


template< class type >
ID_INLINE idEntityPtr<type>::idEntityPtr() {
	spawnId = 0;
}

//template< class type >
//ID_INLINE void idEntityPtr<type>::Save( idSaveGame *savefile ) const {
//	savefile->WriteInt( spawnId );
//}
//
//template< class type >
//ID_INLINE void idEntityPtr<type>::Restore( idRestoreGame *savefile ) {
//	savefile->ReadInt( spawnId );
//}

template< class type >
ID_INLINE idEntityPtr<type> &idEntityPtr<type>::operator=( type *ent ) {
	if ( ent == NULL ) {
		spawnId = 0;
	} else {
		spawnId = ( gameLocal.spawnIds[ent->entityNumber] << GENTITYNUM_BITS ) | ent->entityNumber;
	}
	return *this;
}

template< class type >
ID_INLINE bool idEntityPtr<type>::SetSpawnId( int id ) {
	// the reason for this first check is unclear:
	// the function returning false may mean the spawnId is already set right, or the entity is missing
	if ( id == spawnId ) {
		return false;
	}
	if ( ( id >> GENTITYNUM_BITS ) == gameLocal.spawnIds[ id & ( ( 1 << GENTITYNUM_BITS ) - 1 ) ] ) {
		spawnId = id;
		return true;
	}
	return false;
}

template< class type >
ID_INLINE bool idEntityPtr<type>::IsValid( void ) const {
	return ( gameLocal.spawnIds[ spawnId & ( ( 1 << GENTITYNUM_BITS ) - 1 ) ] == ( spawnId >> GENTITYNUM_BITS ) );
}

template< class type >
ID_INLINE type *idEntityPtr<type>::GetEntity( void ) const {
	int entityNum = spawnId & ( ( 1 << GENTITYNUM_BITS ) - 1 );
	if ( ( gameLocal.spawnIds[ entityNum ] == ( spawnId >> GENTITYNUM_BITS ) ) ) {
		return static_cast<type *>( gameLocal.entities[ entityNum ] );
	}
	return NULL;
}

template< class type >
ID_INLINE int idEntityPtr<type>::GetEntityNum( void ) const {
	return ( spawnId & ( ( 1 << GENTITYNUM_BITS ) - 1 ) );
}
void gameError( const char *fmt, ... );


#endif