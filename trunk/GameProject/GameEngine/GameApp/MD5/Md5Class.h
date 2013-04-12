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
/*

Base class for all game objects.  Provides fast run-time type checking and run-time
instancing of objects.

*/

#ifndef __SYS_CLASS_H__
#define __SYS_CLASS_H__

class idClass;
class idTypeInfo;

extern const idEventDef EV_Remove;
extern const idEventDef EV_SafeRemove;

typedef void ( idClass::*eventCallback_t )( void );

template< class Type >
struct idEventFunc {
	const idEventDef	*event;
	eventCallback_t		function;
};

// added & so gcc could compile this
#define EVENT( event, function )	{ &( event ), ( void ( idClass::* )( void ) )( &function ) },
#define END_CLASS					{ NULL, NULL } };

class idEventArg {
public:
	int			type;
	int			value;

	idEventArg()								{ type = D_EVENT_INTEGER; value = 0; };
	idEventArg( int data )						{ type = D_EVENT_INTEGER; value = data; };
	idEventArg( float data )					{ type = D_EVENT_FLOAT; value = *reinterpret_cast<int *>( &data ); };
	idEventArg( noVec3 &data )					{ type = D_EVENT_VECTOR; value = reinterpret_cast<int>( &data ); };
	idEventArg( const idStr &data )				{ type = D_EVENT_STRING; value = reinterpret_cast<int>( data.c_str() ); };
	idEventArg( const char *data )				{ type = D_EVENT_STRING; value = reinterpret_cast<int>( data ); };
	idEventArg( const class idEntity *data )	{ type = D_EVENT_ENTITY; value = reinterpret_cast<int>( data ); };
	idEventArg( const struct trace_s *data )	{ type = D_EVENT_TRACE; value = reinterpret_cast<int>( data ); };
};

class idAllocError {
public:
	char error[MAX_STRING_CHARS];

	idAllocError( const char *text = "" )  { strcpy( error, text ); }
};

/***********************************************************************

  idClass

***********************************************************************/

/*
================
CLASS_PROTOTYPE

This macro must be included in the definition of any subclass of idClass.
It prototypes variables used in class instanciation and type checking.
Use this on single inheritance concrete classes only.
================
*/
#define CLASS_PROTOTYPE( nameofclass )									\
public:																	\
	static	idTypeInfo						Type;						\
	static	idClass							*CreateInstance( void );	\
	virtual	idTypeInfo						*GetType( void ) const;		\
	static	idEventFunc<nameofclass>		eventCallbacks[]

/*
================
CLASS_DECLARATION

This macro must be included in the code to properly initialize variables
used in type checking and run-time instanciation.  It also defines the list
of events that the class responds to.  Take special care to ensure that the 
proper superclass is indicated or the run-time type information will be
incorrect.  Use this on concrete classes only.
================
*/
#define CLASS_DECLARATION( nameofsuperclass, nameofclass )											\
	idTypeInfo nameofclass::Type( #nameofclass, #nameofsuperclass,									\
		( idEventFunc<idClass> * )nameofclass::eventCallbacks,	nameofclass::CreateInstance, ( void ( idClass::* )( void ) )&nameofclass::Spawn);	\
	idClass *nameofclass::CreateInstance( void ) {													\
		try {																						\
			nameofclass *ptr = new nameofclass;														\
			ptr->FindUninitializedMemory();															\
			return ptr;																				\
		}																							\
		catch( idAllocError & ) {																	\
			return NULL;																			\
		}																							\
	}																								\
	idTypeInfo *nameofclass::GetType( void ) const {												\
		return &( nameofclass::Type );																\
	}																								\
idEventFunc<nameofclass> nameofclass::eventCallbacks[] = {

/*
================
ABSTRACT_PROTOTYPE

This macro must be included in the definition of any abstract subclass of idClass.
It prototypes variables used in class instanciation and type checking.
Use this on single inheritance abstract classes only.
================
*/
#define ABSTRACT_PROTOTYPE( nameofclass )								\
public:																	\
	static	idTypeInfo						Type;						\
	static	idClass							*CreateInstance( void );	\
	virtual	idTypeInfo						*GetType( void ) const;		\
	static	idEventFunc<nameofclass>		eventCallbacks[]

/*
================
ABSTRACT_DECLARATION

This macro must be included in the code to properly initialize variables
used in type checking.  It also defines the list of events that the class
responds to.  Take special care to ensure that the proper superclass is
indicated or the run-time tyep information will be incorrect.  Use this
on abstract classes only.
================
*/
#define ABSTRACT_DECLARATION( nameofsuperclass, nameofclass )										\
	idTypeInfo nameofclass::Type( #nameofclass, #nameofsuperclass,									\
	( idEventFunc<idClass> * )nameofclass::eventCallbacks, nameofclass::CreateInstance, ( void ( idClass::* )( void ) )&nameofclass::Spawn);	\
	idClass *nameofclass::CreateInstance( void ) {													\
	gameLocal.Error( "Cannot instanciate abstract class %s.", #nameofclass );					\
	return NULL;																				\
}																								\
	idTypeInfo *nameofclass::GetType( void ) const {												\
	return &( nameofclass::Type );																\
}																								\
	idEventFunc<nameofclass> nameofclass::eventCallbacks[] = {


typedef void ( idClass::*classSpawnFunc_t )( void );

class idClass {
public:
	ABSTRACT_PROTOTYPE( idClass );

#ifdef ID_REDIRECT_NEWDELETE
#undef new
#endif
	void *						operator new( size_t );
	void *						operator new( size_t s, int, int, char *, int );
	void						operator delete( void * );
	void						operator delete( void *, int, int, char *, int );
#ifdef ID_REDIRECT_NEWDELETE
#define new ID_DEBUG_NEW
#endif

	virtual						~idClass();

	void						Spawn( void );
	void						CallSpawn( void );
	bool						IsType( const idTypeInfo &c ) const;
	const char *				GetClassname( void ) const;
	const char *				GetSuperclass( void ) const;
	void						FindUninitializedMemory( void );
	

	bool						ProcessEvent( const idEventDef *ev );
	bool						ProcessEvent( const idEventDef *ev, idEventArg arg1 );

	bool ProcessEventArgs( const idEventDef *ev, int numargs, ... );

	bool						ProcessEventArgPtr( const idEventDef *ev, int *data );
	void						CancelEvents( const idEventDef *ev );
	void CancelEvents( const idClass *obj, const idEventDef *evdef );

	void						Event_Remove( void );


	static void					Init( void );
	static void					Shutdown( void );
	static idTypeInfo *			GetClass( const char *name );
	static idClass *			CreateInstance( const char *name );
	static int					GetNumTypes( void ) { return types.Num(); }
	static int					GetTypeNumBits( void ) { return typeNumBits; }
	static idTypeInfo *			GetType( int num );
	bool						PostEventArgs( const idEventDef *ev, int time, int numargs, ... );
	classSpawnFunc_t			CallSpawnFunc( idTypeInfo *cls );
private:

	static bool					initialized;
	static idList<idTypeInfo *>	types;
	static idList<idTypeInfo *>	typenums;
	static int					typeNumBits;
	static int					memused;
	static int					numobjects;

};

/***********************************************************************

  idTypeInfo

***********************************************************************/

class idTypeInfo {
public:
	const char *				classname;
	const char *				superclass;
	idClass *					( *CreateInstance )( void );
	void						( idClass::*Spawn )( void );
	

	idEventFunc<idClass> *		eventCallbacks;
	eventCallback_t *			eventMap;
	idTypeInfo *				super;
	idTypeInfo *				next;
	bool						freeEventMap;
	int							typeNum;
	int							lastChild;

	idHierarchy<idTypeInfo>		node;

	idTypeInfo( const char *classname, const char *superclass, 
		idEventFunc<idClass> *eventCallbacks, idClass *( *CreateInstance )( void ), void ( idClass::*Spawn )( void ));
	~idTypeInfo();

	void						Init( void );
	void						Shutdown( void );

	bool						IsType( const idTypeInfo &superclass ) const;
};


/*
================
idTypeInfo::IsType

Checks if the object's class is a subclass of the class defined by the 
passed in idTypeInfo.
================
*/
ID_INLINE bool idTypeInfo::IsType( const idTypeInfo &type ) const {
	return ( ( typeNum >= type.typeNum ) && ( typeNum <= type.lastChild ) );
}


/*
================
idClass::IsType

Checks if the object's class is a subclass of the class defined by the 
passed in idTypeInfo.
================
*/
ID_INLINE bool idClass::IsType( const idTypeInfo &superclass ) const {
	idTypeInfo *subclass;

	subclass = GetType();
	return subclass->IsType( superclass );
}

#endif
