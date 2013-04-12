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
#include "MD5Entity.h"

#include "MD5Game.h"
#include "MD5Scene.h"
#include "MD5Misc.h"

//idCmdSystem *				cmdSystem = NULL;
//idCVarSystem *				cvarSystem = NULL;

idGameLocal					gameLocal;


void idGameLocal::Clear( void ) {
	//int i;

	memset( usercmds, 0, sizeof( usercmds ) );
	memset( entities, 0, sizeof( entities ) );
	memset( spawnIds, -1, sizeof( spawnIds ) );
	firstFreeIndex = 0;
	num_entities = 0;
	spawnedEntities.Clear();
	activeEntities.Clear();
	numEntitiesToDeactivate = 0;
	sortPushers = false;
	sortTeamMasters = false;
	random.SetSeed( 0 );
	world = NULL;
	clip.Shutdown();
	entityHash.Clear( 1024, MAX_GENTITIES );
	framenum = 0;
	previousTime = 0;
	time = 0;
	mapFileName.Clear();
	mapFile = NULL;
	spawnCount = INITIAL_SPAWN_COUNT;
	spawnArgs.Clear();
	gravity.Set( 0, 0, -1 );
		
	isMultiplayer = false;
	isServer = false;
	isClient = false;
	realClientTime = 0;
	isNewFrame = true;
	clientSmoothing = 0.1f;
	
	nextGibTime = 0;
	//eventQueue.Init();
	mCurrScene = 0;

}

void idGameLocal::Warning( const char *fmt, ... ) const {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];
	//idThread *	thread;

	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	/*thread = idThread::CurrentThread();
	if ( thread ) {
		thread->Warning( "%s", text );
	} else {
		common->Warning( "%s", text );
	}*/
}


void idGameLocal::DWarning( const char *fmt, ... ) const {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];
	/*idThread *	thread;

	if ( !developer.GetBool() ) {
		return;
	}*/

	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	/*thread = idThread::CurrentThread();
	if ( thread ) {
		thread->Warning( "%s", text );
	} else {
		common->DWarning( "%s", text );
	}*/
}

void idGameLocal::Error( const char *fmt, ... ) const {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];
	//idThread *	thread;

	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	/*thread = idThread::CurrentThread();
	if ( thread ) {
		thread->Error( "%s", text );
	} else {
		common->Error( "%s", text );
	}*/
}

const noVec3 & idGameLocal::GetGravity( void ) const
{
	return gravity;
}

void idGameLocal::Printf( const char *fmt, ... ) const id_attribute((format(printf,2,3)))
{
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	//common->Printf( "%s", text );
}


void gameError( const char *fmt, ... ) {
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

	va_start( argptr, fmt );
	idStr::vsnPrintf( text, sizeof( text ), fmt, argptr );
	va_end( argptr );

	gameLocal.Error( "%s", text );
}

const idDeclEntityDef *idGameLocal::FindEntityDef( const char *name, bool makeDefault ) const {
	const idDecl *decl = NULL;
	/*if ( isMultiplayer ) {
		decl = declManager->FindType( DECL_ENTITYDEF, va( "%s_mp", name ), false );
	}*/
	if ( !decl ) {
		decl = declManager->FindType( DECL_ENTITYDEF, name, makeDefault );
	}
	return static_cast<const idDeclEntityDef *>( decl );
}

/*
================
idGameLocal::FindEntityDefDict
================
*/
const idDict *idGameLocal::FindEntityDefDict( const char *name, bool makeDefault ) const {
	const idDeclEntityDef *decl = FindEntityDef( name, makeDefault );
	return decl ? &decl->dict : NULL;
}

idEntity * idGameLocal::SpawnEntityType( const idTypeInfo &classdef, const idDict *args /*= NULL*/, bool bIsClientReadSnapshot /*= false */ )
{
	idClass *obj;

//#if _DEBUG
//	if ( isClient ) {
//		assert( bIsClientReadSnapshot );
//	}
//#endif

	if ( !classdef.IsType( idEntity::Type ) ) {
		Error( "Attempted to spawn non-entity class '%s'", classdef.classname );
	}

	try {
		if ( args ) {
			spawnArgs = *args;
		} else {
			spawnArgs.Clear();
		}
		obj = classdef.CreateInstance();
		obj->CallSpawn();
	}

	catch( idAllocError & ) {
		obj = NULL;
	}
	spawnArgs.Clear();

	return static_cast<idEntity *>(obj);
}

bool idGameLocal::SpawnEntityDef( const idDict &args, idEntity **ent /*= NULL*/, bool setDefaults /*= true */ ) {
	const char	*classname;
	const char	*spawn;
	idTypeInfo	*cls;
	idClass		*obj;
	idStr		error;
	const char  *name;

	if ( ent ) {
		*ent = NULL;
	}

	spawnArgs = args;

	if ( spawnArgs.GetString( "name", "", &name ) ) {
		sprintf( error, " on '%s'", name);
	}

	spawnArgs.GetString( "classname", NULL, &classname );

	const idDeclEntityDef *def = FindEntityDef( classname, false );

	if ( !def ) {
		Warning( "Unknown classname '%s'%s.", classname, error.c_str() );
		return false;
	}

	spawnArgs.SetDefaults( &def->dict );

#ifdef _D3XP
	if ( !spawnArgs.FindKey( "slowmo" ) ) {
		bool slowmo = true;

		for ( int i = 0; fastEntityList[i]; i++ ) {
			if ( !idStr::Cmp( classname, fastEntityList[i] ) ) {
				slowmo = false;
				break;
			}
		}

		if ( !slowmo ) {
			spawnArgs.SetBool( "slowmo", slowmo );
		}
	}
#endif

	// check if we should spawn a class object
	spawnArgs.GetString( "spawnclass", NULL, &spawn );
	if ( spawn ) {

		cls = idClass::GetClass( spawn );
		if ( !cls ) {
			Warning( "Could not spawn '%s'.  Class '%s' not found %s.", classname, spawn, error.c_str() );
			return false;
		}

		obj = cls->CreateInstance();
		if ( !obj ) {
			Warning( "Could not spawn '%s'. Instance could not be created %s.", classname, error.c_str() );
			return false;
		}

		obj->CallSpawn();

		if ( ent && obj->IsType( idEntity::Type ) ) {
			*ent = static_cast<idEntity *>(obj);
		}

		return true;
	}

	// check if we should call a script function to spawn
	/*spawnArgs.GetString( "spawnfunc", NULL, &spawn );
	if ( spawn ) {
		const function_t *func = program.FindFunction( spawn );
		if ( !func ) {
			Warning( "Could not spawn '%s'.  Script function '%s' not found%s.", classname, spawn, error.c_str() );
			return false;
		}
		idThread *thread = new idThread( func );
		thread->DelayedStart( 0 );
		return true;
	}*/

	Warning( "%s doesn't include a spawnfunc or spawnclass%s.", classname, error.c_str() );
	return false;
}

void idGameLocal::RegisterEntity( idEntity *ent )
{
	int spawn_entnum;

	if ( spawnCount >= ( 1 << ( 32 - GENTITYNUM_BITS ) ) ) {
		Error( "idGameLocal::RegisterEntity: spawn count overflow" );
	}

	if ( !spawnArgs.GetInt( "spawn_entnum", "0", spawn_entnum ) ) {
		while( entities[firstFreeIndex] && firstFreeIndex < ENTITYNUM_MAX_NORMAL ) {
			firstFreeIndex++;
		}
		if ( firstFreeIndex >= ENTITYNUM_MAX_NORMAL ) {
			Error( "no free entities" );
		}
		spawn_entnum = firstFreeIndex++;
	}

	entities[ spawn_entnum ] = ent;
	spawnIds[ spawn_entnum ] = spawnCount++;
	ent->entityNumber = spawn_entnum;
	ent->spawnNode.AddToEnd( spawnedEntities );
	ent->spawnArgs.TransferKeyValues( spawnArgs );

	if ( spawn_entnum >= num_entities ) {
		num_entities++;
	}
}

void idGameLocal::RunFrame() {
	idEntity *	ent;
	int			num;
	
		// update the game time
		framenum++;
		previousTime = time;
		time += msec * 0.1f;
		//realClientTime = time;


		// make sure the random number counter is used each frame so random events
		// are influenced by the player's actions
		random.RandomInt();
	

	num = 0;
	for( ent = activeEntities.Next(); ent != NULL; ent = ent->activeNode.Next() ) {
#ifdef _D3XP
		if ( ent->timeGroup != TIME_GROUP1 ) {
			continue;
		}
#endif
		ent->Think();
		num++;
	}

	// remove any entities that have stopped thinking
	if ( numEntitiesToDeactivate ) {
		idEntity *next_ent;
		int c = 0;
		for( ent = activeEntities.Next(); ent != NULL; ent = next_ent ) {
			next_ent = ent->activeNode.Next();
			if ( !ent->thinkFlags ) {
				ent->activeNode.Remove();
				c++;
			}
		}
		//assert( numEntitiesToDeactivate == c );
		numEntitiesToDeactivate = 0;
	}

	idEvent::ServiceEvents();

}

/*
================
idGameLocal::RemoveEntityFromHash
================
*/
bool idGameLocal::RemoveEntityFromHash( const char *name, idEntity *ent ) {
	int hash, i;

	hash = entityHash.GenerateKey( name, true );
	for ( i = entityHash.First( hash ); i != -1; i = entityHash.Next( i ) ) {
		if ( entities[i] && entities[i] == ent && entities[i]->name.Icmp( name ) == 0 ) {
			entityHash.Remove( hash, i );
			return true;
		}
	}
	return false;
}

void idGameLocal::UnregisterEntity( idEntity *ent ) {
	assert( ent );

	/*if ( editEntities ) {
	editEntities->RemoveSelectedEntity( ent );
	}*/

	if ( ( ent->entityNumber != ENTITYNUM_NONE ) && ( entities[ ent->entityNumber ] == ent ) ) {
		ent->spawnNode.Remove();
		entities[ ent->entityNumber ] = NULL;
		spawnIds[ ent->entityNumber ] = -1;
		if ( ent->entityNumber >= MAX_CLIENTS && ent->entityNumber < firstFreeIndex ) {
			firstFreeIndex = ent->entityNumber;
		}
		ent->entityNumber = ENTITYNUM_NONE;
	}
}

void idGameLocal::LoadMap( const char *mapName, int randseed ) {
	int i;
	bool sameMap = (mapFile && idStr::Icmp(mapFileName, mapName) == 0);
	//InitAsyncNetwork();
	if ( !sameMap || ( mapFile && mapFile->NeedsReload() ) ) {
		// load the .map file
		if ( mapFile ) {
			delete mapFile;
		}
		if (mCurrScene)
			delete mCurrScene;
		mCurrScene = new MD5Scene;
		mapFile = new idMapFile;
		if ( !mapFile->Parse( idStr( mapName ) + ".map", true ) ) {
			delete mapFile;
			mapFile = NULL;
			SAFE_DELETE(mCurrScene);
			Error( "Couldn't load %s", mapName );
		}
	}
	mapFileName = mapFile->GetName();
	collisionModelManager->LoadMap( mapFile );
//	numClients = 0;
	// initialize all entities for this game
	memset( entities, 0, sizeof( entities ) );
	memset( usercmds, 0, sizeof( usercmds ) );
	memset( spawnIds, -1, sizeof( spawnIds ) );
	spawnCount = INITIAL_SPAWN_COUNT;

	spawnedEntities.Clear();
	activeEntities.Clear();
	numEntitiesToDeactivate = 0;
	sortTeamMasters = false;
	sortPushers = false;

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	num_entities	= MAX_CLIENTS;
	firstFreeIndex	= MAX_CLIENTS;

	// reset the random number generator.
	random.SetSeed( isMultiplayer ? randseed : 0 );

	world			= NULL;

	previousTime	= 0;
	time			= 0;
	framenum		= 0;
	nextGibTime		= 0;

	gravity.Set( 0, 0, -g_gravity.GetFloat() );

	spawnArgs.Clear();
	clip.Init();
}

/*
=============
idGameLocal::GetTraceEntity

returns the master entity of a trace.  for example, if the trace entity is the player's head, it will return the player.
=============
*/
idEntity *idGameLocal::GetTraceEntity( const trace_t &trace ) const {
	idEntity *master;

	if ( !entities[ trace.c.entityNum ] ) {
		return NULL;
	}
	master = entities[ trace.c.entityNum ]->GetBindMaster();
	if ( master ) {
		return master;
	}
	return entities[ trace.c.entityNum ];
}

/*
===================
idGameLocal::MapPopulate
===================
*/
void idGameLocal::MapPopulate( void ) {
	// parse the key/value pairs and spawn entities
	SpawnMapEntities();

#if 0
	// mark location entities in all connected areas
	SpreadLocations();

	// prepare the list of randomized initial spawn spots
	RandomizeInitialSpawns();

	// spawnCount - 1 is the number of entities spawned into the map, their indexes started at MAX_CLIENTS (included)
	// mapSpawnCount is used as the max index of map entities, it's the first index of non-map entities
	mapSpawnCount = MAX_CLIENTS + spawnCount - 1;

	// execute pending events before the very first game frame
	// this makes sure the map script main() function is called
	// before the physics are run so entities can bind correctly
	Printf( "==== Processing events ====\n" );
	idEvent::ServiceEvents();
#endif
}

/*
==============
idGameLocal::SpawnMapEntities

Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/
void idGameLocal::SpawnMapEntities( void ) {
	int			i;
	int			num;
	int			inhibit;
	idMapEntity	*mapEnt;
	int			numEntities;
	idDict		args;

	Printf( "Spawning entities\n" );

	if ( mapFile == NULL ) {
		Printf("No mapfile present\n");
		return;
	}

	SetSkill( g_skill.GetInteger() );

	numEntities = mapFile->GetNumEntities();
	if ( numEntities == 0 ) {
		Error( "...no entities" );
	}

	// the worldspawn is a special that performs any global setup
	// needed by a level
	mapEnt = mapFile->GetEntity( 0 );
	args = mapEnt->epairs;
	args.SetInt( "spawn_entnum", ENTITYNUM_WORLD );
	if ( !SpawnEntityDef( args ) || !entities[ ENTITYNUM_WORLD ] || !entities[ ENTITYNUM_WORLD ]->IsType( idWorldspawn::Type ) ) {
		Error( "Problem spawning world entity" );
	}

	num = 1;
	inhibit = 0;
#if 0
	for ( i = 1 ; i < numEntities ; i++ ) {
		mapEnt = mapFile->GetEntity( i );
		args = mapEnt->epairs;

		if ( !InhibitEntitySpawn( args ) ) {
			// precache any media specified in the map entity
			CacheDictionaryMedia( &args );

			SpawnEntityDef( args );
			num++;
		} else {
			inhibit++;
		}
	}
#endif
	Printf( "...%i entities spawned, %i inhibited\n\n", num, inhibit );
}

/*
================
idGameLocal::SetSkill
================
*/
void idGameLocal::SetSkill( int value ) {
	int skill_level;

	if ( value < 0 ) {
		skill_level = 0;
	} else if ( value > 3 ) {
		skill_level = 3;
	} else {
		skill_level = value;
	}

	g_skill.SetInteger( skill_level );
}