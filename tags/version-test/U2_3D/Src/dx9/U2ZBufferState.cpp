#include <U2_3D/src/U23DLibPCH.h>
#include "U2ZBufferState.h"


IMPLEMENT_RTTI(U2ZBufferState, U2RenderState);
IMPLEMENT_INITIALIZE(U2ZBufferState);
IMPLEMENT_TERMINATE(U2ZBufferState);

void U2ZBufferState::Initialize()
{
	ms_aspDefault[ZBUFFER] = U2_NEW U2ZBufferState;
	FILE_LOG(logDEBUG) << _T("U2ZBufferState::Initialize - Success...");
}

void U2ZBufferState::Terminate()
{
	ms_aspDefault[ZBUFFER] = 0;
	FILE_LOG(logDEBUG) << _T("U2ZBufferState::Terminate - Success...");
}



U2ZBufferState::U2ZBufferState()
{
	
	m_bZEnabled = true;;
	m_bZWritable = true;
	m_eCompareFunc = CF_LESSEQUAL;
}


U2ZBufferState::~U2ZBufferState()
{

}

void	U2ZBufferState::SetZEnable(bool bZEnable)
{
	m_bZEnabled = bZEnable;
}


bool	U2ZBufferState::GetZEnable() const
{
	return m_bZEnabled;
}


void	U2ZBufferState::SetZWritable(bool bZWritable)
{
	m_bZWritable = bZWritable;
}


bool	U2ZBufferState::GetZWritable() const
{
	return m_bZWritable;
}


void	U2ZBufferState::SetZTestFunc(CompareFunc eFunc)
{
	m_eCompareFunc = eFunc;
}


U2ZBufferState::CompareFunc U2ZBufferState::GetZTestFunc() const
{	
	return m_eCompareFunc;
}