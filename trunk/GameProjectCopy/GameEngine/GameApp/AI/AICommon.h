#pragma once 

#include <assert.h>
#include "Mmsystem.h"
#include "EngineCore/Singleton.h"

#include "RecastLog.h"
#include "DetourNavMesh.h"

class Time : public Singleton <Time>
{
public:

	Time( void );
	~Time( void ) {}

	void MarkTimeThisTick( void );
	float GetElapsedTime( void )				{ return( m_timeLastTick ); }
	float GetCurTime( void )					{ return( m_currentTime ); }

private:

	unsigned int m_startTime;
	float m_currentTime;
	float m_timeLastTick;

};



inline Time::Time( void )
{
	m_currentTime = 0.0f;
	m_timeLastTick = 0.001f;
	m_startTime = timeGetTime();
}

/*---------------------------------------------------------------------------*
  Name:         MarkTimeThisTick

  Description:  Marks the current time for this tick (frame). This can be
                referenced during the frame to simulate a consistent moment 
				in time.
  
  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
inline void Time::MarkTimeThisTick( void )
{
	unsigned int timeInMS = timeGetTime();
	float newTime = (timeInMS-m_startTime)  / 1000.0f;

	m_timeLastTick = newTime - m_currentTime;
	m_currentTime = newTime;

	if( m_timeLastTick <= 0.0f ) {
		m_timeLastTick = 0.001f;
	}

}


#define g_time Time::GetSingleton()
#define g_database (*GameObjectManager::Get())
#define g_msgroute MsgRoute::GetSingleton()
#define g_debuglog DebugLog::GetSingleton()

#define INVALID_OBJECT_ID 0


#define ASSERT_MSG(a,b) assert(a && b)

#define MAX_LAYERS_ALLOWED 6


typedef unsigned int objectID;


/** This is the Query Flag for any other MovableObject */
const unsigned int QUERYFLAG_MOVABLE = 2;

enum QueryFlags
{
	AGENT_MASK						= 1<<0,
	SINBAD_MASK						= 1<<1,
	GEOMETRY_QUERY_MASK				= 1<<2,
	VEHICLE_QUERY_MASK				= 1<<3,
	STATIC_GEOMETRY_QUERY_MASK		= 1<<4
};


enum SamplePolyAreas
{
	SAMPLE_POLYAREA_GROUND,
	SAMPLE_POLYAREA_WATER,
	SAMPLE_POLYAREA_ROAD,
	SAMPLE_POLYAREA_DOOR,
	SAMPLE_POLYAREA_GRASS,
	SAMPLE_POLYAREA_JUMP,
};
enum SamplePolyFlags
{
	SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
	SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
	SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
	SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
	SAMPLE_POLYFLAGS_ALL = 0xffff		// All abilities.
};
