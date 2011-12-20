/************************************************************************
module	: U2StopWatch
Author	: Yun sangyong
Desc	: 
************************************************************************/
#pragma once 
#ifndef U2_STOPWATCH_H
#define U2_STOPWATCH_H

#include <U2Lib/src/Memory/U2MemObj.h>
#include "U2Util.h"

class U2LIB U2StopWatch : public U2MemObj
{

public:
	U2StopWatch();

	void	Start();
	void	Stop();

	void	Suspend();
	void	Resume();

	void	Reset();
	uint32	GetTimeInMilisecs() const;
	float	GetTimeInSecs() const;

	uint32	ElapsedCnt() const;

private:
	uint32	m_uiElapsedCnt;
	uint32	m_uiInitTime;
	uint32	m_uiSuspendTime;
	bool	m_bSuspend;

};

#ifdef U2_PROFILE 
#define PROFILER_DECLARE(prof)		U2StopWatch prof
#define PROFILER_START(prof)		prof.Start()
#define PROFILER_STOP(prof)			prof.Stop()
#define PROFILER_RESET(prof)		prof.Reset()
#define PROFILER_OUTPUT(prof)		if(1) dbgout(prof.GetTimeInSecs())
#else 
#define PROFILER_DECLARE(prof)		
#define PROFILER_START(prof)		
#define PROFILER_STOP(prof)			
#define PROFILER_RESET(prof)		
#define PROFILER_OUTPUT(prof)
#endif


#include "U2StopWatch.inl"



#endif