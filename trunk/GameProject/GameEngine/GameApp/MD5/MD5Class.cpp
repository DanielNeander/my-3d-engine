#include "stdafx.h"
#pragma hdrstop
#include "MD5Event.h"
#include "Md5Class.h"
#include <GameApp/MD5/MD5Material.h>
#include <GameApp/MD5/MD5MapFile.h>
#include <GameApp/Collision/CollisionModel.h>
#include <GameApp/Collision/CollisionModel_local.h>
#include "GameApp/Physics/Physics.h"
#include "GameApp/Physics/Clip.h"
#include "GameApp/MD5/MD5Model.h"
#include "GameApp/Animation/MD5Anim.h"
#include "GameApp/Animation/MD5IK.h"
#include <GameApp/MD5/MD5Game.h>
#include <GameApp/MD5/DeclAF.h>
#include "GameApp/Physics/Physics_Base.h"
#include "GameApp/Physics/Physics_Actor.h"
#include <GameApp/Physics/Physics_AF.h>
#include <GameApp/MD5/MD5Af.h>
#include <GameApp/MD5/MD5Entity.h>
#include <GameApp/MD5/MD5AFEntity.h>
#include "GameApp/MD5/MD5Actor.h"



/***********************************************************************

  idTypeInfo

***********************************************************************/

// this is the head of a singly linked list of all the idTypes
static idTypeInfo				*typelist = NULL;
static idHierarchy<idTypeInfo>	classHierarchy;
static int						eventCallbackMemory	= 0;


/*
================
idTypeInfo::idClassType()

Constructor for class.  Should only be called from CLASS_DECLARATION macro.
Handles linking class definition into class hierarchy.  This should only happen
at startup as idTypeInfos are statically defined.  Since static variables can be
initialized in any order, the constructor must handle the case that subclasses
are initialized before superclasses.
================
*/
idTypeInfo::idTypeInfo( const char *classname, const char *superclass, idEventFunc<idClass> *eventCallbacks, idClass *( *CreateInstance )( void ), 
	void ( idClass::*Spawn )( void )) {

		idTypeInfo *type;
		idTypeInfo **insert;

		this->classname			= classname;
		this->superclass		= superclass;
		this->eventCallbacks	= eventCallbacks;
		this->eventMap			= NULL;
		this->Spawn				= Spawn;
		//this->Save				= Save;
		//this->Restore			= Restore;
		this->CreateInstance	= CreateInstance;
		this->super				= idClass::GetClass( superclass );
		this->freeEventMap		= false;
		typeNum					= 0;
		lastChild				= 0;

		// Check if any subclasses were initialized before their superclass
		for( type = typelist; type != NULL; type = type->next ) {
			if ( ( type->super == NULL ) && !idStr::Cmp( type->superclass, this->classname ) && 
				idStr::Cmp( type->classname, "idClass" ) ) {
					type->super	= this;
			}
		}

		// Insert sorted
		for ( insert = &typelist; *insert; insert = &(*insert)->next ) {
			assert( idStr::Cmp( classname, (*insert)->classname ) );
			if ( idStr::Cmp( classname, (*insert)->classname ) < 0 ) {
				next = *insert;
				*insert = this;
				break;
			}
		}
		if ( !*insert ) {
			*insert = this;
			next = NULL;
		}
}

/*
================
idTypeInfo::~idTypeInfo
================
*/
idTypeInfo::~idTypeInfo() {
	Shutdown();
}

/*
================
idTypeInfo::Init

Initializes the event callback table for the class.  Creates a 
table for fast lookups of event functions.  Should only be called once.
================
*/
void idTypeInfo::Init( void ) {
	idTypeInfo				*c;
	idEventFunc<idClass>	*def;
	int						ev;
	int						i;
	bool					*set;
	int						num;

	if ( eventMap ) {
		// we've already been initialized by a subclass
		return;
	}

	// make sure our superclass is initialized first
	if ( super && !super->eventMap ) {
		super->Init();
	}

	// add to our node hierarchy
	if ( super ) {
		node.ParentTo( super->node );
	} else {
		node.ParentTo( classHierarchy );
	}
	node.SetOwner( this );

	// keep track of the number of children below each class
	for( c = super; c != NULL; c = c->super ) {
		c->lastChild++;
	}

	// if we're not adding any new event callbacks, we can just use our superclass's table
	if ( ( !eventCallbacks || !eventCallbacks->event ) && super ) {
		eventMap = super->eventMap;
		return;
	}

	// set a flag so we know to delete the eventMap table
	freeEventMap = true;

	// Allocate our new table.  It has to have as many entries as there
	// are events.  NOTE: could save some space by keeping track of the maximum
	// event that the class responds to and doing range checking.
	num = idEventDef::NumEventCommands();
	eventMap = new eventCallback_t[ num ];
	memset( eventMap, 0, sizeof( eventCallback_t ) * num );
	eventCallbackMemory += sizeof( eventCallback_t ) * num;

	// allocate temporary memory for flags so that the subclass's event callbacks
	// override the superclass's event callback
	set = new bool[ num ];
	memset( set, 0, sizeof( bool ) * num );

	// go through the inheritence order and copies the event callback function into
	// a list indexed by the event number.  This allows fast lookups of
	// event functions.
	for( c = this; c != NULL; c = c->super ) {
		def = c->eventCallbacks;
		if ( !def ) {
			continue;
		}

		// go through each entry until we hit the NULL terminator
		for( i = 0; def[ i ].event != NULL; i++ )	{
			ev = def[ i ].event->GetEventNum();

			if ( set[ ev ] ) {
				continue;
			}
			set[ ev ] = true;
			eventMap[ ev ] = def[ i ].function;
		}
	}

	delete[] set;
}

/*
================
idTypeInfo::Shutdown

Should only be called when DLL or EXE is being shutdown.
Although it cleans up any allocated memory, it doesn't bother to remove itself 
from the class list since the program is shutting down.
================
*/
void idTypeInfo::Shutdown() {
	// free up the memory used for event lookups
	if ( eventMap ) {
		if ( freeEventMap ) {
			delete[] eventMap;
		}
		eventMap = NULL;
	}
	typeNum = 0;
	lastChild = 0;
}

/***********************************************************************

  idClass

***********************************************************************/
const idEventDef EV_Remove( "<immediateremove>", NULL );
//const idEventDef EV_SafeRemove( "remove", NULL );

ABSTRACT_DECLARATION( NULL, idClass )
	EVENT( EV_Remove,				idClass::Event_Remove )
	//EVENT( EV_SafeRemove,			idClass::Event_SafeRemove )
END_CLASS

int idClass::typeNumBits = 0;

idList<idTypeInfo *> idClass::typenums;

idList<idTypeInfo *> idClass::types;

bool idClass::initialized = false;


int idClass::numobjects = 0;

int idClass::memused  = 0 ;


idClass::~idClass()
{
	idEvent::CancelEvents( this );
}

/*
================
idClass::new
================
*/
#ifdef ID_DEBUG_MEMORY
#undef new
#endif

void * idClass::operator new( size_t s ) {
	int *p;

	s += sizeof( int );
	p = (int *)Mem_Alloc( s );
	*p = s;
	memused += s;
	numobjects++;

#ifdef ID_DEBUG_UNINITIALIZED_MEMORY
	unsigned long *ptr = (unsigned long *)p;
	int size = s;
	assert( ( size & 3 ) == 0 );
	size >>= 3;
	for ( int i = 1; i < size; i++ ) {
		ptr[i] = 0xcdcdcdcd;
	}
#endif

	return p + 1;
}

void * idClass::operator new( size_t s, int, int, char *, int ) {
	int *p;

	s += sizeof( int );
	p = (int *)Mem_Alloc( s );
	*p = s;
	memused += s;
	numobjects++;

#ifdef ID_DEBUG_UNINITIALIZED_MEMORY
	unsigned long *ptr = (unsigned long *)p;
	int size = s;
	assert( ( size & 3 ) == 0 );
	size >>= 3;
	for ( int i = 1; i < size; i++ ) {
		ptr[i] = 0xcdcdcdcd;
	}
#endif

	return p + 1;
}

#ifdef ID_DEBUG_MEMORY
#define new ID_DEBUG_NEW
#endif

/*
================
idClass::delete
================
*/
void idClass::operator delete( void *ptr ) {
	int *p;

	if ( ptr ) {
		p = ( ( int * )ptr ) - 1;
		memused -= *p;
		numobjects--;
		Mem_Free( p );
	}
}

void idClass::operator delete( void *ptr, int, int, char *, int ) {
	int *p;

	if ( ptr ) {
		p = ( ( int * )ptr ) - 1;
		memused -= *p;
		numobjects--;
		Mem_Free( p );
	}
}

idClass *idClass::CreateInstance( const char *name ) {
	const idTypeInfo	*type;
	idClass				*obj;

	type = idClass::GetClass( name );
	if ( !type ) {
		return NULL;
	}

	obj = type->CreateInstance();
	return obj;
}


idTypeInfo * idClass::GetType( int typeNum )
{
	idTypeInfo *c;

	if ( !initialized ) {
		for( c = typelist; c != NULL; c = c->next ) {
			if ( c->typeNum == typeNum ) {
				return c;
			}
		}
	} else if ( ( typeNum >= 0 ) && ( typeNum < types.Num() ) ) {
		return typenums[ typeNum ];
	}

	return NULL;
}

void idClass::Spawn( void )
{

}

void idClass::CallSpawn( void )
{
	idTypeInfo *type;

	type = GetType();
	CallSpawnFunc( type );
}

classSpawnFunc_t idClass::CallSpawnFunc( idTypeInfo *cls ) {
	classSpawnFunc_t func;

	if ( cls->super ) {
		func = CallSpawnFunc( cls->super );
		if ( func == cls->Spawn ) {
			// don't call the same function twice in a row.
			// this can happen when subclasses don't have their own spawn function.
			return func;
		}
	}

	( this->*cls->Spawn )();

	return cls->Spawn;
}


/*
================
idClass::PostEventArgs
================
*/
bool idClass::PostEventArgs( const idEventDef *ev, int time, int numargs, ... ) {
	idTypeInfo	*c;
	idEvent		*event;
	va_list		args;

	assert( ev );

	if ( !idEvent::initialized ) {
		return false;
	}

	c = GetType();
	if ( !c->eventMap[ ev->GetEventNum() ] ) {
		// we don't respond to this event, so ignore it
		return false;
	}

	// we service events on the client to avoid any bad code filling up the event pool
	// we don't want them processed usually, unless when the map is (re)loading.
	// we allow threads to run fine, though.
	/*if ( gameLocal.isClient && ( gameLocal.GameState() != GAMESTATE_STARTUP ) && !IsType( idThread::Type ) ) {
		return true;
	}*/

	va_start( args, numargs );
	event = idEvent::Alloc( ev, numargs, args );
	va_end( args );

	event->Schedule( this, c, time );

	return true;
}


const char * idClass::GetClassname( void ) const
{
	idTypeInfo *type;

	type = GetType();
	return type->classname;
}

const char * idClass::GetSuperclass( void ) const
{
	idTypeInfo *cls;

	cls = GetType();
	return cls->superclass;
}

void idClass::Init( void )
{
	idTypeInfo	*c;
	int			num;

	gameLocal.Printf( "Initializing class hierarchy\n" );

	if ( initialized ) {
		gameLocal.Printf( "...already initialized\n" );
		return;
	}

	// init the event callback tables for all the classes
	for( c = typelist; c != NULL; c = c->next ) {
		c->Init();
	}

	// number the types according to the class hierarchy so we can quickly determine if a class
	// is a subclass of another
	num = 0;
	for( c = classHierarchy.GetNext(); c != NULL; c = c->node.GetNext(), num++ ) {
		c->typeNum = num;
		c->lastChild += num;
	}

	// number of bits needed to send types over network
	typeNumBits = noMath::BitsForInteger( num );

	// create a list of the types so we can do quick lookups
	// one list in alphabetical order, one in typenum order
	types.SetGranularity( 1 );
	types.SetNum( num );
	typenums.SetGranularity( 1 );
	typenums.SetNum( num );
	num = 0;
	for( c = typelist; c != NULL; c = c->next, num++ ) {
		types[ num ] = c;
		typenums[ c->typeNum ] = c;
	}

	initialized = true;

	gameLocal.Printf( "...%i classes, %i bytes for event callbacks\n", types.Num(), eventCallbackMemory );
}

void idClass::Shutdown( void )
{
	idTypeInfo	*c;

	for( c = typelist; c != NULL; c = c->next ) {
		c->Shutdown();
	}
	types.Clear();
	typenums.Clear();

	initialized = false;
}

idTypeInfo * idClass::GetClass( const char *name )
{
	idTypeInfo	*c;
	int			order;
	int			mid;
	int			min;
	int			max;

	if ( !initialized ) {
		// idClass::Init hasn't been called yet, so do a slow lookup
		for( c = typelist; c != NULL; c = c->next ) {
			if ( !idStr::Cmp( c->classname, name ) ) {
				return c;
			}
		}
	} else {
		// do a binary search through the list of types
		min = 0;
		max = types.Num() - 1;
		while( min <= max ) {
			mid = ( min + max ) / 2;
			c = types[ mid ];
			order = idStr::Cmp( c->classname, name );
			if ( !order ) {
				return c;
			} else if ( order > 0 ) {
				max = mid - 1;
			} else {
				min = mid + 1;
			}
		}
	}

	return NULL;
}

void idClass::FindUninitializedMemory( void ) {
#ifdef ID_DEBUG_UNINITIALIZED_MEMORY
	unsigned long *ptr = ( ( unsigned long * )this ) - 1;
	int size = *ptr;
	assert( ( size & 3 ) == 0 );
	size >>= 2;
	for ( int i = 0; i < size; i++ ) {
		if ( ptr[i] == 0xcdcdcdcd ) {
			const char *varName = GetTypeVariableName( GetClassname(), i << 2 );
			gameLocal.Warning( "type '%s' has uninitialized variable %s (offset %d)", GetClassname(), varName, i << 2 );
		}
	}
#endif
}

bool idClass::ProcessEvent( const idEventDef *ev )
{
	return ProcessEventArgs(ev, 0);
}

/*
================
idClass::ProcessEvent
================
*/
bool idClass::ProcessEvent( const idEventDef *ev, idEventArg arg1 ) {
	return ProcessEventArgs( ev, 1, &arg1 );
}


bool idClass::ProcessEventArgs( const idEventDef *ev, int numargs, ... ) {
	idTypeInfo	*c;
	int			num;
	int			data[ D_EVENT_MAXARGS ];
	va_list		args;

	assert( ev );
	assert( idEvent::initialized );

	c = GetType();
	num = ev->GetEventNum();
	if ( !c->eventMap[ num ] ) {
		// we don't respond to this event, so ignore it
		return false;
	}

	va_start( args, numargs );
	idEvent::CopyArgs( ev, numargs, args, data );
	va_end( args );

	ProcessEventArgPtr( ev, data );

	return true;
}

bool idClass::ProcessEventArgPtr( const idEventDef *ev, int *data ) {
	idTypeInfo	*c;
	int			num;
	eventCallback_t	callback;

	assert( ev );
	assert( idEvent::initialized );

#ifdef _D3XP
	SetTimeState ts;

	if ( IsType( idEntity::Type ) ) {
		idEntity *ent = (idEntity*)this;
		ts.PushState( ent->timeGroup );
	}
#endif
	/*if ( g_debugTriggers.GetBool() && ( ev == &EV_Activate ) && IsType( idEntity::Type ) ) {
		const idEntity *ent = *reinterpret_cast<idEntity **>( data );
		gameLocal.Printf( "%d: '%s' activated by '%s'\n", gameLocal.framenum, static_cast<idEntity *>( this )->GetName(), ent ? ent->GetName() : "NULL" );
	}*/

	c = GetType();
	num = ev->GetEventNum();
	if ( !c->eventMap[ num ] ) {
		// we don't respond to this event, so ignore it
		return false;
	}

	callback = c->eventMap[ num ];

#if !CPU_EASYARGS

	/*
	on ppc architecture, floats are passed in a seperate set of registers
	the function prototypes must have matching float declaration

	http://developer.apple.com/documentation/DeveloperTools/Conceptual/MachORuntime/2rt_powerpc_abi/chapter_9_section_5.html
	*/

	switch( ev->GetFormatspecIndex() ) {
	case 1 << D_EVENT_MAXARGS :
		( this->*callback )();
		break;

		// generated file - see CREATE_EVENT_CODE
#include "Callbacks.cpp"

	default:
		gameLocal.Warning( "Invalid formatspec on event '%s'", ev->GetName() );
		break;
	}

#else

	assert( D_EVENT_MAXARGS == 8 );

	switch( ev->GetNumArgs() ) {
	case 0 :
		( this->*callback )();
		break;

	case 1 :
		typedef void ( idClass::*eventCallback_1_t )( const int );
		( this->*( eventCallback_1_t )callback )( data[ 0 ] );
		break;

	case 2 :
		typedef void ( idClass::*eventCallback_2_t )( const int, const int );
		( this->*( eventCallback_2_t )callback )( data[ 0 ], data[ 1 ] );
		break;

	case 3 :
		typedef void ( idClass::*eventCallback_3_t )( const int, const int, const int );
		( this->*( eventCallback_3_t )callback )( data[ 0 ], data[ 1 ], data[ 2 ] );
		break;

	case 4 :
		typedef void ( idClass::*eventCallback_4_t )( const int, const int, const int, const int );
		( this->*( eventCallback_4_t )callback )( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ] );
		break;

	case 5 :
		typedef void ( idClass::*eventCallback_5_t )( const int, const int, const int, const int, const int );
		( this->*( eventCallback_5_t )callback )( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ], data[ 4 ] );
		break;

	case 6 :
		typedef void ( idClass::*eventCallback_6_t )( const int, const int, const int, const int, const int, const int );
		( this->*( eventCallback_6_t )callback )( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ], data[ 4 ], data[ 5 ] );
		break;

	case 7 :
		typedef void ( idClass::*eventCallback_7_t )( const int, const int, const int, const int, const int, const int, const int );
		( this->*( eventCallback_7_t )callback )( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ], data[ 4 ], data[ 5 ], data[ 6 ] );
		break;

	case 8 :
		typedef void ( idClass::*eventCallback_8_t )( const int, const int, const int, const int, const int, const int, const int, const int );
		( this->*( eventCallback_8_t )callback )( data[ 0 ], data[ 1 ], data[ 2 ], data[ 3 ], data[ 4 ], data[ 5 ], data[ 6 ], data[ 7 ] );
		break;

	default:
		gameLocal.Warning( "Invalid formatspec on event '%s'", ev->GetName() );
		break;
	}

#endif	
	return true;
}

/*
================
idEvent::CancelEvents
================
*/
void idClass::CancelEvents( const idClass *obj, const idEventDef *evdef ) {
	idEvent::CancelEvents( this, evdef );	
}

/*
================
idClass::Event_Remove
================
*/
void idClass::Event_Remove( void ) {
	delete this;
}