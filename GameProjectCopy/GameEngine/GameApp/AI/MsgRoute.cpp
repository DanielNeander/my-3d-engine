#include "stdafx.h"
#include "MsgRoute.h"
#include "StateMachine.h"
#include "../GameObjectManager.h"


#define MSGROUTE_ALLOW_INSTANTANEOUS_SEND_MSG

/*---------------------------------------------------------------------------*
  Name:         ~MsgRoute

  Description:  Destructor
 *---------------------------------------------------------------------------*/
MsgRoute::~MsgRoute( void )
{
	MessageContainer::iterator i;
	for( i=m_delayedMessages.begin(); i!=m_delayedMessages.end(); ++i )
	{
		MSG_Object * msg = *i;
		delete( msg );
	}

	m_delayedMessages.clear();
}


/*---------------------------------------------------------------------------*
  Name:         SendMsg

  Description:  Sends a message through the message router. This function
                determines if the message should be delivered immediately
				or should be held until the delivery time.

  Arguments:    delay    : the number of seconds to delay the message
                name     : the message name
				receiver : the ID of the receiver
				sender   : the ID of the sender
				rule     : the scoping rule for the message
				scope    : the scope of the message (a state index)
				data     : a piece of data
				timer    : if this message is a timer (sent periodically)
				cc       : if this message is a CC (a copy)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::SendMsg( float delay, MSG_Name name,
                        objectID receiver, objectID sender,
                        Scope_Rule rule, unsigned int scope,
                        MSG_Data data, bool timer, bool cc )
{

#ifdef MSGROUTE_ALLOW_INSTANTANEOUS_SEND_MSG
	if( delay <= 0.0f )
	{	//Deliver immediately
		MSG_Object msg( g_time.GetCurTime(), name, sender, receiver, rule, scope, data, timer, cc );
		RouteMsg( msg );
	}
	else
#endif
	{	//Check for duplicates - then store
		MessageContainer::iterator i;
		for( i=m_delayedMessages.begin(); i!=m_delayedMessages.end(); ++i )
		{
			if( (*i)->IsDelivered() == false &&
				(*i)->GetName() == name &&
				(*i)->GetReceiver() == receiver &&
				(*i)->GetSender() == sender &&
				(*i)->GetScopeRule() == rule &&
				(*i)->GetScope() == scope &&
				(*i)->IsTimer() == timer )
			{	//Already in list - don't add
				return;
			}
		}
		
		//Store in delivery list
		float deliveryTime = delay + g_time.GetCurTime();
		MSG_Object * msg = new MSG_Object( deliveryTime, name, sender, receiver, rule, scope, data, timer, false );
		m_delayedMessages.push_back( msg );
	}

}

/*---------------------------------------------------------------------------*
  Name:         SendMsgBroadcast

  Description:  Sends a message to every object of a certain type.

  Arguments:    msg    : the message to broadcast
                type   : the type of object (optional)

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::SendMsgBroadcast( MSG_Object & msg, unsigned int type )
{
	/*dbCompositionList list;
	g_database.ComposeList( list, type );

	dbCompositionList::iterator i;
	for( i=list.begin(); i!=list.end(); ++i )
	{
		if( msg.GetSender() != (*i)->GetID() )
		{
			if( (*i)->GetStateMachine() ) {
				(*i)->GetStateMachine()->Process( EVENT_Message, &msg );
			}
		}
	}*/
}

/*---------------------------------------------------------------------------*
  Name:         DeliverDelayedMessages

  Description:  Checks every delayed message and sends it if the time is right.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::DeliverDelayedMessages( void )
{
	MessageContainer::iterator i = m_delayedMessages.begin();
	while( i != m_delayedMessages.end() )
	{
		if( (*i)->GetDeliveryTime() <= g_time.GetCurTime() )
		{	//Deliver and delete msg
			MSG_Object * msg = *i;
			RouteMsg( *msg );
			delete( msg );
			i = m_delayedMessages.erase( i );
		}
		else
		{
			++i;
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         RouteMsg

  Description:  Routes the message to the receiver, only if the scoping rules
                allow it.

  Arguments:    msg : the message to route

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::RouteMsg( MSG_Object & msg )
{
	GameObject * object = g_database.Find( msg.GetReceiver() );

	if( object != 0 && object->GetStateMachine() )
	{
		Scope_Rule rule = msg.GetScopeRule();
		if( rule == NO_SCOPING ||
			( rule == SCOPE_TO_SUBSTATE && msg.GetScope() == object->GetStateMachine()->GetScopeSubstate() ) ||
			( rule == SCOPE_TO_STATE && msg.GetScope() == object->GetStateMachine()->GetScopeState() ) )
		{	//Scope matches
			msg.SetDelivered( true );	//Important to set as delivered since timer messages 
										//will resend themselves immediately (and would get
										//thrown away if we didn't set this, since it would look
										//like a redundant message)
			
			if( msg.IsTimer() )
			{	//Timer message that occurs periodically
				float delay = msg.GetFloatData();	//Timer value stored in data field
				msg.SetIntData( 0 );				//Zero out data field
				//Queue up next periodic msg
				object->GetStateMachine()->SetTimer( delay, msg.GetName(), rule );
			}
			
			if( msg.IsCC() ) {
				object->GetStateMachine()->Process( EVENT_CCMessage, &msg );
			}
			else {
				object->GetStateMachine()->Process( EVENT_Message, &msg );
			}
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         RemoveMsg

  Description:  Removes messages from the delayed message list that meet
                the criteria. This is useful to avoid duplicate delayed
				messages.

  Arguments:    name     : the name of the message
                receiver : the receiver ID of the message
				sender   : the sender ID of the message
				timer    : whether the message is a timer

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::RemoveMsg( MSG_Name name, objectID receiver, objectID sender, bool timer )
{
	MessageContainer::iterator i = m_delayedMessages.begin();
	while( i != m_delayedMessages.end() )
	{
		MSG_Object * msg = *i;
		if( msg->GetName() == name &&
			msg->GetReceiver() == receiver &&
			msg->GetSender() == sender &&
			msg->IsTimer() == timer &&
			!msg->IsDelivered() )
		{
			delete( msg );
			i = m_delayedMessages.erase( i );
		}
		else
		{
			++i;
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         PurgeScopedMsg

  Description:  Removes messages from the delayed message list for a given
                receiver if the message is scoped to a particular state. This
				is useful if the receiver changes state machines, since the 
				messages are no longer valid.

  Arguments:    receiver : the receiver ID of the message

  Returns:      None.
 *---------------------------------------------------------------------------*/
void MsgRoute::PurgeScopedMsg( objectID receiver )
{
	MessageContainer::iterator i = m_delayedMessages.begin();
	while( i != m_delayedMessages.end() )
	{
		MSG_Object * msg = *i;
		if( msg->GetReceiver() == receiver &&
			msg->GetScopeRule() != NO_SCOPING &&
			!msg->IsDelivered() )
		{
			delete( msg );
			i = m_delayedMessages.erase( i );
		}
		else
		{
			++i;
		}
	}
}


