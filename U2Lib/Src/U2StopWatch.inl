


inline
U2StopWatch::U2StopWatch()
	:m_uiElapsedCnt(0), 
	m_uiInitTime(0), 
	m_uiSuspendTime(0)
	,m_bSuspend(false)
{

}


inline 
void U2StopWatch::Start()
{

	if(m_uiElapsedCnt == 0)
		m_uiInitTime = GetPerformanceCounter();
	m_uiElapsedCnt++;
}

inline 
void U2StopWatch::Stop()
{
	U2ASSERT(m_uiElapsedCnt > 0);
	m_uiElapsedCnt--;

	if(m_uiElapsedCnt == 0)
	{
		m_uiInitTime = GetPerformanceCounter() - m_uiInitTime;
	}
}


inline 
void U2StopWatch::Reset()
{
	m_uiElapsedCnt = m_uiInitTime = 0;
}


inline 
uint32 U2StopWatch::GetTimeInMilisecs() const 
{	
	return m_uiInitTime;
}


inline 
float U2StopWatch::GetTimeInSecs() const 
{
	return GetTimeInMilisecs() / GetPerformaceCounterHz();
}


inline 
uint32 U2StopWatch::ElapsedCnt() const
{
	return m_uiElapsedCnt;
}


inline 
void U2StopWatch::Suspend()
{
	m_uiSuspendTime = GetPerformanceCounter();
}


inline
void U2StopWatch::Resume()
{

	U2ASSERT(m_bSuspend);

	m_uiInitTime += GetPerformanceCounter() - m_uiSuspendTime;
	m_uiSuspendTime = 0;
	m_bSuspend = false;
}


