#include "stdafx.h"
#include "FSM.h"
#include "GameApp.h"

FSMState<FSMObject>		        STATE_NONE("none");

FSMObject::FSMObject()
{
	m_state = NULL;
	m_stateTime = 0;
	m_stateStartTime = 0;
}

bool FSMObject::TickState()
{
	if( m_state )
	{
		return m_state->Update(this);
	}

	return false;
}

bool FSMObject::TickInput()
{
	if(m_state)
	{
		return m_state->UpdateInput(this);
	}
	return false;
}

void FSMObject::TransitionState( IFSMState *pkState, float time/*=0*/ )
{
	m_state                 = pkState;
	m_stateStartTime        = GetTime();
	m_stateTime             = m_stateStartTime + time;
}

float FSMObject::GetTime()
{
	return ::GetTime(); 	
}

float FSMObject::GetRandom( float min, float max )
{
	if( max<=min )
		return min;

	int32 imin = (int) (min * 1000.0f);
	int32 imax = (int) (max * 1000.0f);
	int32 random = imin + rand()%(imax - imin);

	return (float)( random / 1000.0f );
}
