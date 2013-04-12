#include "stdafx.h"
#include "MsgRoute.h"
#include "StateMachine.h"



#define MAX_STATE_STACK_SIZE 10


StateMachine::StateMachine( GameObject * object )
: m_owner( object )
{
	Initialize();
}

/*---------------------------------------------------------------------------*
  Name:         Initialize

  Description:  Initializes the state machine.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::Initialize( void )
{
	m_scopeState = 0;
	m_scopeSubstate = 0;
	m_currentState = 0;
	m_currentSubstate = -1;
	m_stateChange = NO_STATE_CHANGE;
	m_nextState = 0;
	m_nextSubstate = 0;
	m_stateChangeAllowed = true;
	m_updateEventEnabled = true;
	m_timeOnEnter = 0.0f;
	m_ccMessagesToGameObject = 0;

	m_currentStateNameString[0] = 0;
	m_currentSubstateNameString[0] = 0;
  
	m_broadcastList.clear();
	m_stack.clear();
}

/*---------------------------------------------------------------------------*
  Name:         Reset

  Description:  Resets the state machine to the initial settings and default
                state. The state machine is aslo given the very first
				EVENT_Enter event.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::Reset( void )
{
	Initialize();
	Process( EVENT_Enter, 0 );
}

/*---------------------------------------------------------------------------*
  Name:         Update

  Description:  An update event is sent to the state machine.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::Update( void )
{
	if( m_updateEventEnabled )
	{
		Process( EVENT_Update, 0 );
	}
}

/*---------------------------------------------------------------------------*
  Name:         Process

  Description:  Processes an event in the state machine. It is first sent
                to the current substate of the current state. If not handled,
				it is sent to the current state. If not handled, it is then 
				sent to the global state. Finally, any state changes are
				propagated.

  Arguments:    event : the event to process
                msg   : an optional msg to process with the event

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::Process( State_Machine_Event event, MSG_Object * msg )
{
	
	if( event == EVENT_Message && msg && GetCCReceiver() > 0 )
	{	//CC this message
		SendCCMsg( msg->GetName(), GetCCReceiver(), msg->GetMsgData() );
	}

	//Process inside state machine
	bool handled = false;
	if( m_currentSubstate >= 0 )
	{	//Send to current substate
		handled = States( event, msg, m_currentState, m_currentSubstate );
	}
	if( !handled )
	{	//Send to current state
		handled = States( event, msg, m_currentState, -1 );
	}
	if( !handled )
	{	//Send to global state
		handled = States( event, msg, -1, -1 );
	}
	
	PerformStateChanges();	
}

/*---------------------------------------------------------------------------*
  Name:         PerformStateChanges

  Description:  Checks for a requested state change and executes it. This
                repeats until all requested state changes have completed. To
				avoid an infinite loop of state changes, it is stopped after
				50 times.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::PerformStateChanges( void )
{
	//Check for a state change
	int safetyCount = 20;
	while( m_stateChange != NO_STATE_CHANGE && (--safetyCount >= 0) )
	{
		ASSERT_MSG( safetyCount > 0, "StateMachine::PerformStateChanges - States are flip-flopping in an infinite loop." );

		m_stateChangeAllowed = false;

		//Let the last state clean-up
		if( m_currentSubstate > 0 )
		{	//Moving from a substate
			States( EVENT_Exit, 0, static_cast<int>( m_currentState ), m_currentSubstate );
		}
		if( m_nextSubstate < 0 )
		{	//Leaving current state
			States( EVENT_Exit, 0, static_cast<int>( m_currentState ), -1 );
		}
		

		//Perform state change
		switch( m_stateChange )
		{
			case STATE_CHANGE:
				if( m_nextSubstate < 0 )
				{	//This is a state change and not a substate change
					//Store the old state on the state stack
					m_stack.push_back( m_currentState );
					//Restrict stack size
					if( m_stack.size() > MAX_STATE_STACK_SIZE ) {
						m_stack.pop_front();
					}
				}
				//Set the new state
				m_currentState = m_nextState;
				m_currentSubstate = m_nextSubstate;
#ifdef DEBUG_STATE_MACHINE_MACROS
				g_debuglog.LogStateMachineStateChange( m_owner->GetID(), m_owner->GetName(), m_currentState, m_currentSubstate );
#endif
				break;
				
			case STATE_POP:
				if( !m_stack.empty() ) {
					//Get last state off stack and pop it
					m_currentState = m_stack.back();
					m_currentSubstate = -1;
					m_stack.pop_back();
				}
				else {
					ASSERT_MSG( 0, "StateMachine::PerformStateChanges - Hit bottom of state stack. Can't pop state." );
				}
#ifdef DEBUG_STATE_MACHINE_MACROS
				g_debuglog.LogStateMachineStateChange( m_owner->GetID(), m_owner->GetName(), m_currentState, m_currentSubstate );
#endif
				break;
			
			default:
				ASSERT_MSG( 0, "StateMachine::PerformStateChanges - Invalid state change." );
		}
				
		//Increment the scope (every state change gets a unique scope)
		m_scopeSubstate++;
		if( m_nextSubstate < 0 ) {
			m_scopeState++;
		}

		//Remember the time we entered this state
		m_timeOnEnter = g_time.GetCurTime();

		//Let the new state initialize
		m_stateChange = NO_STATE_CHANGE;
		m_stateChangeAllowed = true;
		States( EVENT_Enter, 0, static_cast<int>( m_currentState ), m_currentSubstate );
	}

}

/*---------------------------------------------------------------------------*
  Name:         ChangeState

  Description:  Requests a state change. The state change will occur once the
                state machine is done executing all code in the current state
				or substate.

  Arguments:    newState : the new destination state

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::ChangeState( unsigned int newState )
{
	ASSERT_MSG( m_stateChangeAllowed, "StateMachine::ChangeState - State change not allowed in OnExit." );
	ASSERT_MSG( m_stateChange == NO_STATE_CHANGE, "StateMachine::ChangeState - State change already requested." );
	if( m_stateChangeAllowed ) {
		m_stateChange = STATE_CHANGE;
		m_nextState = newState;
		m_nextSubstate = -1;		//Next state begins in "no particular" substate
	}
}

/*---------------------------------------------------------------------------*
  Name:         ChangeSubstate

  Description:  Requests a substate change. The substate change will occur 
                once the state machine is done executing all code in the 
				current state or substate.

  Arguments:    newSubstate : the new destination substate

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::ChangeSubstate( unsigned int newSubstate )
{
	ASSERT_MSG( m_stateChangeAllowed, "StateMachine::ChangeState - State change not allowed in OnExit." );
	ASSERT_MSG( m_stateChange == NO_STATE_CHANGE, "StateMachine::ChangeState - State change already requested." );
	if( m_stateChangeAllowed ) {
		m_stateChange = STATE_CHANGE;
		m_nextState = m_currentState;
		m_nextSubstate = static_cast<int>( newSubstate );
	}

}

/*---------------------------------------------------------------------------*
  Name:         ChangeStateDelayed

  Description:  Requests a state change at a specified time in the future.
  
  Arguments:    delay    : the number of seconds in which to execute the state change
                newState : the new destination state
				scope    : the scope of this request

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::ChangeStateDelayed( float delay, unsigned int newState, Scope_Rule scope )
{
	ASSERT_MSG( m_stateChangeAllowed, "StateMachine::ChangeStateDelayed - State change not allowed in OnExit." );
	if( m_stateChangeAllowed ) {
		SendMsgDelayedToMe( delay, MSG_CHANGE_STATE_DELAYED, scope, newState );
	}
}

/*---------------------------------------------------------------------------*
  Name:         ChangeSubstateDelayed

  Description:  Requests a substate change at a specified time in the future.
  
  Arguments:    delay       : the number of seconds in which to execute the state change
                newSubstate : the new destination substate
				scope       : the scope of this request

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::ChangeSubstateDelayed( float delay, unsigned int newSubstate, Scope_Rule scope )
{
	ASSERT_MSG( m_stateChangeAllowed, "StateMachine::ChangeSubstateDelayed - State change not allowed in OnExit." );
	if( m_stateChangeAllowed ) {
		SendMsgDelayedToMe( delay, MSG_CHANGE_SUBSTATE_DELAYED, scope, newSubstate );
	}
}

/*---------------------------------------------------------------------------*
  Name:         PopState

  Description:  Requests to pop the current state. The state change will occur 
                once the state machine is done executing all code in the current 
				state or substate.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::PopState( void )
{
	ASSERT_MSG( m_stateChangeAllowed, "StateMachine::PopState - State change not allowed in OnExit." );
	ASSERT_MSG( m_stateChange == NO_STATE_CHANGE, "StateMachine::PopState - State change already requested." );
	if( m_stateChangeAllowed ) {
		m_stateChange = STATE_POP;
	}
}

/*---------------------------------------------------------------------------*
  Name:         SendMsg

  Description:  General method for sending a message to another object.
  
  Arguments:    name     : the name of the message
                receiver : the receiver of the message
				data     : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsg( MSG_Name name, objectID receiver, int data )
{
	MSG_Data msgdata( data );
	g_msgroute.SendMsg( 0, name, receiver, m_owner->GetID(), NO_SCOPING, 0, msgdata, false, false );

}

/*---------------------------------------------------------------------------*
  Name:         SendMsgToMe

  Description:  Preferred way to send a message to yourself (the same object).
                Only the name is required, but the scoping rule and data can
				also be included. If no scope rule is specified, it defaults
				to the most restrictive scoping, making the message only apply
				to the substate (or state if no substates exist).
  
  Arguments:    name : the name of the message
                rule : the scoping rule for the message
				data : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgToMe( MSG_Name name, Scope_Rule rule, int data )
{
	SendMsgDelayedToMeHelper( 0, name, rule, data, false );
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgDelayed

  Description:  Sends a message at a specified time in the future.
  
  Arguments:    delay    : the number of seconds in the future to deliver the message
                name     : the name of the message
                receiver : the receiver of the message
				data     : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgDelayed( float delay, MSG_Name name, objectID receiver, int data )
{
	MSG_Data msgdata( data );
	g_msgroute.SendMsg( delay, name, receiver, m_owner->GetID(), NO_SCOPING, 0, msgdata, false, false );
	
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgDelayedToMe

  Description:  Preferred way to send a message to yourself at a specified 
                time in the future.
  
  Arguments:    delay : the number of seconds in the future to deliver the message
                name  : the name of the message
                rule  : the scoping rule for the message
				data  : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgDelayedToMe( float delay, MSG_Name name, Scope_Rule rule, int data )
{
	SendMsgDelayedToMeHelper( delay, name, rule, data, false );
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgBroadcast

  Description:  Preferred way to broadcast a message to many objects at once.
  
  Arguments:    name : the name of the message
                type : the type of object to broadcast the message to
                zone : the spatial zone to broadcast to
				data : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgBroadcast( MSG_Name name, unsigned int type, int data )
{
	MSG_Object msg( 0.0f, name, m_owner->GetID(), 0, NO_SCOPING, 0, data, false, false );
	g_msgroute.SendMsgBroadcast( msg, type );
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgBroadcastToList

  Description:  Broadcast a message to a pre-composed list of objects.
  
  Arguments:    name : the name of the message
				data : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgBroadcastToList( MSG_Name name, int data )
{
	ASSERT_MSG( m_broadcastList.size() > 0, "StateMachine::SendMsgBroadcast - No objects in broadcast list." );
	MSG_Data msgdata( data );

	BroadcastListContainer::iterator i;
	for( i=m_broadcastList.begin(); i!=m_broadcastList.end(); ++i )
	{
		objectID id = (objectID)(*i);
		if( id != m_owner->GetID() ) {
			g_msgroute.SendMsg( 0, name, id, m_owner->GetID(), NO_SCOPING, 0, msgdata, false, false );
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         BroadcastClearList

  Description:  Clear all objects from the broadcast list.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::BroadcastClearList( void )
{
	m_broadcastList.clear();
}

/*---------------------------------------------------------------------------*
  Name:         BroadcastAddToList

  Description:  Add an object to the broadcast list.
  
  Arguments:    id : an ID of an object to add to the list

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::BroadcastAddToList( objectID id )
{
	m_broadcastList.push_back( id );
}

/*---------------------------------------------------------------------------*
  Name:         SetTimer

  Description:  Set a timer to deliver a message periodically to yourself.
  
  Arguments:    delay : the number of seconds in the future to deliver the message
                name  : the name of the message
				rule  : the scoping rule for the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SetTimer( float delay, MSG_Name name, Scope_Rule rule )
{
	//pass in delay as data, so timer can be resent when received
	MSG_Data data( delay );
	SendMsgDelayedToMeHelper( delay, name, rule, data, true );
}

/*---------------------------------------------------------------------------*
  Name:         StopTimer

  Description:  Stop a timer based on the message name.
  
  Arguments:    name  : the name of the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::StopTimer( MSG_Name name )
{
	g_msgroute.RemoveMsg( name, m_owner->GetID(), m_owner->GetID(), true );
}

/*---------------------------------------------------------------------------*
  Name:         SendCCMsg

  Description:  Send a CCd message to the receiver. This is like a regular
                message, but the CC flag is on.
  
  Arguments:    name     : the name of the message
                receiver : the receiver of the message
				data     : associated data to deliver with the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendCCMsg( MSG_Name name, objectID receiver, MSG_Data data )
{
	g_msgroute.SendMsg( 0, name, receiver, m_owner->GetID(), NO_SCOPING, 0, data, false, true );
}

/*---------------------------------------------------------------------------*
  Name:         SendMsgDelayedToMeHelper

  Description:  Helper function for several other functions to send a message
                to yourself.
  
  Arguments:    delay : the number of seconds in the future to deliver the message
                name  : the name of the message
                rule  : the scoping rule for the message
				data  : associated data to deliver with the message
				timer : whether this is a periodic timer message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::SendMsgDelayedToMeHelper( float delay, MSG_Name name, Scope_Rule rule, MSG_Data data, bool timer )
{
	unsigned int scope = 0;
	
	if( rule == SCOPE_TO_SUBSTATE ) {	
		scope = m_scopeSubstate;
	}
	else if( rule == SCOPE_TO_STATE ) {
		scope = m_scopeState;
	}

	g_msgroute.SendMsg( delay, name, m_owner->GetID(), m_owner->GetID(), rule, scope, data, timer, false );
}

/*---------------------------------------------------------------------------*
  Name:         RandDelay

  Description:  Returns a random delay within the range [min,max].
  
  Arguments:    min : the lower bound
                max : the higher bound

  Returns:      random delay in range
 *---------------------------------------------------------------------------*/
float StateMachine::RandDelay( float min, float max )
{
	ASSERT_MSG( min <= max, "RandDelay - min must be <= to max" );
	float range = max - min;
	float value = (float)rand()*(1.0f/(float)(RAND_MAX));
	value *= range;
	value += min;

	return( value );
}

/*---------------------------------------------------------------------------*
  Name:         ResetStateMachine

  Description:  Requests that the state machine is reset. The reset will occur
                at the next update cycle.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::ResetStateMachine( void )
{
	m_owner->RequestStateMachineChange( 0, STATE_MACHINE_RESET );
}

/*---------------------------------------------------------------------------*
  Name:         ReplaceStateMachine

  Description:  Requests that the state machine be replaced. The replacement
                will occur at the next update cycle.
  
  Arguments:    mch : the new state machine

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::ReplaceStateMachine( StateMachine & mch )
{
	m_owner->RequestStateMachineChange( &mch, STATE_MACHINE_REPLACE );
}

/*---------------------------------------------------------------------------*
  Name:         QueueStateMachine

  Description:  Requests that a new state machine be queued. This will occur 
                at the next update cycle.
  
  Arguments:    mch : the new state machine

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::QueueStateMachine( StateMachine & mch )
{
	m_owner->RequestStateMachineChange( &mch, STATE_MACHINE_QUEUE );
}

/*---------------------------------------------------------------------------*
  Name:         RequeueStateMachine

  Description:  Requests that the current state machine be requeued. This will
                occur at the next update cycle.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::RequeueStateMachine( void )
{
	m_owner->RequestStateMachineChange( 0, STATE_MACHINE_REQUEUE );
}

/*---------------------------------------------------------------------------*
  Name:         PushStateMachine

  Description:  Requests that a new state machine be pushed to the top of the
                list and become the current state machine. This will
                occur at the next update cycle.
  
  Arguments:    mch : the new state machine

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::PushStateMachine( StateMachine & mch )
{
	m_owner->RequestStateMachineChange( &mch, STATE_MACHINE_PUSH );
}

/*---------------------------------------------------------------------------*
  Name:         PopStateMachine

  Description:  Requests that the current state machine be popped from the list.
                This will occur at the next update cycle.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void StateMachine::PopStateMachine( void )
{
	m_owner->RequestStateMachineChange( 0, STATE_MACHINE_POP );
}


