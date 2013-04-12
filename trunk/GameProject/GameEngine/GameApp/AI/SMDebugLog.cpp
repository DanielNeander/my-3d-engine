#include "stdafx.h"
#include "StateMachine.h"
#include "SMDebugLoig.h"
#include "../GameObjectManager.h"


#define MAX_DEBUG_LOG_SIZE 200


LogEntry::LogEntry( void )
{
	m_owner = INVALID_OBJECT_ID;
	m_name[0] = 0;
	m_handled = false;

	m_timestamp = -1.0f;
	m_statename[0] = 0;
	m_eventmsgname[0] = 0;
	m_msg = false;
	m_receiver = INVALID_OBJECT_ID;
	m_sender = INVALID_OBJECT_ID;
	m_data = 0;
	
}


/*---------------------------------------------------------------------------*
  Name:         ~DebugLog

  Description:  Destructor
 *---------------------------------------------------------------------------*/
DebugLog::~DebugLog( void )
{
	LoggingContainer::iterator i;
	for( i=m_log.begin(); i!=m_log.end(); ++i )
	{
		LogEntry* entry = *i;
		delete(entry);
	}

	m_log.clear();
}


/*---------------------------------------------------------------------------*
  Name:         LogStateMachineEvent

  Description:  Logs a state machine event, such as a received message.

  Arguments:    id           : ID of the object
                name         : string name of the object
                msg          : pointer to message object containing event
				statename    : current state of object
				substatename : current substate of object
				eventmsgname : the name of the event
				handled      : whether the event was handled by the object

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DebugLog::LogStateMachineEvent( objectID id, const char* name, MSG_Object * msg, const char* statename, const char* substatename, char* eventmsgname, bool handled )
{
	if( msg && ( strcmp( MessageNameText[msg->GetName()], "MSG_SET_STATE_DELAYED" ) == 0 ||
		strcmp( MessageNameText[msg->GetName()], "MSG_SET_SUBSTATE_DELAYED" ) == 0 ))
	{	//Don't log these events
		return;
	}

	LogEntry * entry = new LogEntry();

	entry->m_owner = id;
	strcpy(entry->m_name, name);
	entry->m_handled = handled;
	entry->m_timestamp = g_time.GetCurTime();
	strcpy( entry->m_statename, statename );
	strcpy( entry->m_substatename, substatename );

	if( !handled )
	{
		if( eventmsgname[0] == '1')
		{
			strcpy( entry->m_eventmsgname, "EVENT_Update" );
		}
		else if( eventmsgname[0] == '2' && msg ) 
		{
			strcpy( entry->m_eventmsgname, MessageNameText[msg->GetName()] );
		}
		else if( eventmsgname[0] == '3') 
		{
			strcpy( entry->m_eventmsgname, "EVENT_CCMessage" );
		}
		else if( eventmsgname[0] == '4') 
		{
			strcpy( entry->m_eventmsgname, "EVENT_Enter" );
		}
		else if( eventmsgname[0] == '5') 
		{
			strcpy( entry->m_eventmsgname, "EVENT_Exit" );
		}
		else
		{
			ASSERT_MSG( 0, "DebugLog::LogStateMachineEvent - eventmsgname not handled" );
			strcpy( entry->m_eventmsgname, "INVALID_EVENT" );
		}
	}
	else
	{
		strcpy( entry->m_eventmsgname, eventmsgname );
	}

	if( msg ) {
		entry->m_msg = true;
		entry->m_receiver = msg->GetReceiver();
		entry->m_sender = msg->GetSender();
		entry->m_data = msg->GetIntData();
	}

	if( (handled || eventmsgname[0] == '2') &&
		strcmp(entry->m_eventmsgname, "EVENT_Update") != 0 )
	{
		PrintLogEntry( *entry );
	}

	m_log.push_back( entry );
	if( m_log.size() > MAX_DEBUG_LOG_SIZE ) {
		delete( m_log.front() );
		m_log.pop_front();
	}
}

/*---------------------------------------------------------------------------*
  Name:         LogStateMachineStateChange

  Description:  Logs a state machine state change.

  Arguments:    id        : ID of the object
                name      : string name of the object
                state     : new state index
                substate  : new substate index
				statename : current state of object

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DebugLog::LogStateMachineStateChange( objectID id, const char* name, unsigned int state, int substate )
{
	LogEntry * entry = new LogEntry();

	entry->m_owner = id;
	strcpy(entry->m_name, name);
	entry->m_handled = true;
	entry->m_timestamp = g_time.GetCurTime();
	sprintf( entry->m_statename, "%d", state );
	sprintf( entry->m_substatename, "%d", substate );
	strcpy( entry->m_eventmsgname, "STATE_CHANGE" );
	entry->m_msg = false;

	PrintLogEntry( *entry );

	m_log.push_back( entry );
	if( m_log.size() > MAX_DEBUG_LOG_SIZE ) {
		delete( m_log.front() );
		m_log.pop_front();
	}
}

/*---------------------------------------------------------------------------*
  Name:         Dump

  Description:  Dumps the accumulated log of a particular object to the debug
                console.

  Arguments:    id : ID of the object

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DebugLog::Dump( objectID id )
{
	GameObject* obj = g_database.Find( id );
	printf( "DebugLog: %s, id=%d\n", obj->GetName(), id );

	LoggingContainer::iterator i;
	for( i=m_log.begin(); i!=m_log.end(); ++i )
	{
		LogEntry* log = *i;
		if( log && log->m_owner == id )
		{
			PrintLogEntry( *log );
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         PrintLogEntry

  Description:  Prints a single log entry.

  Arguments:    entry : the log entry to print

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DebugLog::PrintLogEntry( LogEntry& entry )
{
	char debug0[1024];
	char debug1[1024];
	char debug2[1024];
	char state[64];

	if( entry.m_statename[0] != 0 )
	{	//Use state
		strcpy( state, entry.m_statename );
	}
	else
	{	//Use substate
		strcpy( state, entry.m_substatename );
	}

	sprintf( debug0, "%.3f-[%s,%d] %s:%s ", entry.m_timestamp, entry.m_name, entry.m_owner, state, entry.m_eventmsgname );
	
	if( entry.m_msg )
	{
		sprintf( debug1, "from:%d to:%d data:%d ", entry.m_sender, entry.m_receiver, entry.m_data );
	}
	else
	{
		debug1[0] = 0;
	}

	if( entry.m_handled )
	{
		strcpy( debug2, "\n" );
	}
	else
	{
		strcpy( debug2, "(not handled)\n" );
	}

	printf( "%s%s%s", debug0, debug1, debug2 );
}