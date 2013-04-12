#pragma once 

//#include "gameobject.h"
#include "AICommon.h"
#include <list>

#define REGISTER_MESSAGE_NAME(x) #x,
static const char* MessageNameText[] =
{
#include "messagenames.h"
	"Invalid"
};
#undef REGISTER_MESSAGE_NAME


class LogEntry
{
public:

	LogEntry( void );
	~LogEntry( void ) {}

	objectID m_owner;
	char m_name[64];
	bool m_handled;

	float m_timestamp;
	char m_statename[64];
	char m_substatename[64];
	char m_eventmsgname[64];

	//msg only info
	bool m_msg;
	objectID m_receiver;
	objectID m_sender;
	unsigned int m_data;


};

class DebugLog : public Singleton <DebugLog>
{
public:

	DebugLog( void ) {}
	~DebugLog( void );

	void LogStateMachineEvent( objectID id, const char* name, MSG_Object * msg, const char* statename, const char* substatename, char* eventmsgname, bool handled ); 
	void LogStateMachineStateChange( objectID id, const char* name, unsigned int state, int substate ); 

	void Dump( objectID id );

private:

	typedef std::list<LogEntry*> LoggingContainer;

	LoggingContainer m_log;

	void PrintLogEntry( LogEntry& entry );

};
