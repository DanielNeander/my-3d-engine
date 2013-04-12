#pragma once 

#include "AICommon.h"

//Macro trick to make message names enums
//rom the file msgnames.h
#define REGISTER_MESSAGE_NAME(x) x,
typedef enum
{
#include "messagenames.h"
	MSG_NUM
} MSG_Name;
#undef REGISTER_MESSAGE_NAME


//Delayed messages can be scoped with the following enum.
//
//Scoping explanation:
//A message scoped to a state or substate will not be delivered
//if the state or substate changes (only applies to delayed
//messages sent to yourself). If a delayed message scoped to the
//state is sent from state "A" and the state changes to "B" then 
//back to "A", the message will not be delivered since there was 
//a state change. By default, delayed messages are scoped to the 
//substate (which is equivalient to being scoped to the state if 
//there are no substates).
//
enum Scope_Rule { 
	NO_SCOPING,
	SCOPE_TO_STATE,
	SCOPE_TO_SUBSTATE
};

#define NEXT_FRAME 0.0001f

union MSG_Data_Union
{
	int intValue;
	float floatValue;
};

enum MSG_Data_Value
{
	MSG_DATA_INVALID,
	MSG_DATA_INT,
	MSG_DATA_FLOAT
};

class MSG_Data
{
public:
	MSG_Data( void )						{ m_valueType = MSG_DATA_INVALID; }
	MSG_Data( int data )					{ m_data.intValue = data; m_valueType = MSG_DATA_INT; }
	MSG_Data( float data )					{ m_data.floatValue = data; m_valueType = MSG_DATA_FLOAT; }
	inline void SetInt( int data )			{ m_data.intValue = data; m_valueType = MSG_DATA_INT; }
	inline int GetInt( void )				{ ASSERT_MSG( m_valueType == MSG_DATA_INT, "Message data not of correct type" ); return( m_data.intValue ); }
	inline void SetFloat( float data )		{ m_data.floatValue = data; m_valueType = MSG_DATA_FLOAT; }
	inline float GetFloat( void )			{ ASSERT_MSG( m_valueType == MSG_DATA_FLOAT, "Message data not of correct type" ); return( m_data.floatValue ); }

private:
	MSG_Data_Value m_valueType;
	MSG_Data_Union m_data;
};


class MSG_Object
{
public:

	MSG_Object( void );
	MSG_Object( float deliveryTime, MSG_Name name, 
		objectID sender, objectID receiver, 
		Scope_Rule rule, unsigned int scope, 
		MSG_Data data, bool timer, bool cc );

	~MSG_Object( void ) {}

	inline MSG_Name GetName( void )					{ return( m_name ); }
	inline void SetName( MSG_Name name )			{ m_name = name; }

	inline objectID GetSender( void )				{ return( m_sender ); }
	inline void SetSender( objectID sender )		{ m_sender = sender; }

	inline objectID GetReceiver( void )				{ return( m_receiver ); }
	inline void SetReceiver( objectID receiver )	{ m_receiver = receiver; }

	inline Scope_Rule GetScopeRule( void )			{ return( m_scopeRule ); }
	inline void SetScopeRule( Scope_Rule rule )		{ m_scopeRule = rule; }

	inline unsigned int GetScope( void )			{ return( m_scope ); }
	inline void SetScope( unsigned int scope )		{ m_scope = scope; }

	inline float GetDeliveryTime( void )			{ return( m_deliveryTime ); }
	inline void SetDeliveryTime( float time )		{ m_deliveryTime = time; }

	inline bool IsDelivered( void )					{ return( m_delivered ); }
	inline void SetDelivered( bool value )			{ m_delivered = value; }

	inline int GetIntData( void )					{ return( m_data.GetInt() ); }
	inline void SetIntData( int data )				{ m_data.SetInt( data ); }

	inline float GetFloatData( void )				{ return( m_data.GetFloat() ); }
	inline void SetFloatData( float data )			{ m_data.SetFloat( data ); }

	inline MSG_Data GetMsgData( void )				{ return( m_data ); }

	inline bool IsTimer( void )						{ return( m_timer ); }
	inline void SetTimer( bool value )				{ m_timer = value; }

	inline bool IsCC( void )						{ return( m_cc ); }
	inline void SetCC( bool value )					{ m_cc = value; }


private:

	MSG_Name m_name;		//Message name
	objectID m_sender;		//Object that sent the message
	objectID m_receiver;	//Object that will get the message
	unsigned int m_scope;	//State or substate instance in which the receiver is allowed to get the message
	Scope_Rule m_scopeRule;	//Rule for how to interpret scope
	MSG_Data m_data;		//Data that is passed with the message
	bool m_timer;			//Message is sent periodically
	bool m_cc;				//Message is a carbon copy that was received by someone else

	float m_deliveryTime;	//Time at which to send the message
	bool m_delivered;		//Whether the message has been delivered
};


inline MSG_Object::MSG_Object( void )
	: m_name( MSG_NULL ),
	m_sender( INVALID_OBJECT_ID ),
	m_receiver( INVALID_OBJECT_ID ),
	m_scopeRule( NO_SCOPING ),
	m_scope( 0 ),
	m_deliveryTime( 0.0f ),
	m_delivered( false ),
	m_timer( 0 ),
	m_cc( false )
{

}


inline MSG_Object::MSG_Object( float deliveryTime, MSG_Name name,
	objectID sender, objectID receiver,
	Scope_Rule rule, unsigned int scope,
	MSG_Data data, bool timer, bool cc )
{
	SetDeliveryTime( deliveryTime );
	SetName( name );
	SetSender( sender );
	SetReceiver( receiver );
	SetScopeRule( rule );
	SetScope( scope );
	SetDelivered( false );
	SetTimer( timer );
	SetCC( cc );
	m_data = data;
}

