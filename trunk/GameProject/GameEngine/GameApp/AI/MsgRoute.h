#pragma once 

#include "Message.h"
#include "EngineCore/Singleton.h"
#include <list>

typedef std::list<MSG_Object*> MessageContainer;

class MsgRoute : public Singleton <MsgRoute>
{
public:

	MsgRoute( void ) {}
	~MsgRoute( void );

	void DeliverDelayedMessages( void );
	void SendMsg( float delay, MSG_Name name,
		objectID receiver, objectID sender, 
		Scope_Rule rule, unsigned int scope,
		MSG_Data data, bool timer, bool cc );

	void SendMsgBroadcast( MSG_Object & msg, unsigned int type = 0 );

	//Removing delayed messages
	void RemoveMsg( MSG_Name name, objectID receiver, objectID sender, bool timer );
	void PurgeScopedMsg( objectID receiver );

private:

	MessageContainer m_delayedMessages;

	void RouteMsg( MSG_Object & msg );

};
