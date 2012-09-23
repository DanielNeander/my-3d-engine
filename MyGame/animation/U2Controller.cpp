#include <U2_3D/src/U23DLibPCH.h>
#include "U2Controller.h"

IMPLEMENT_RTTI(U2Controller, U2Object);

U2Controller::Type 
U2Controller::GetControllerType() const
{
	return INVALID;
}


bool U2Controller::Update(float fTime, U2RenderContext* pCxt)
{
	return true;
}


U2Controller::U2Controller()
:m_eLoopType(U2AnimLoopType::LOOP),
m_bInit(false),
m_pTarget(0)
{
	m_channelVarHandle = U2VariableMgr::Instance()->
		GetVariableHandleByName(_T("time"));
	m_channelOffsetVarHandle = U2VariableMgr::Instance()->
		GetVariableHandleByName(_T("timeOffset"));
}

U2Controller::~U2Controller()
{

}

//------------------------------------------------------------------------------
/**
Sets the "animation channel" which drives this animation.
This could be something like "time", but the actual names are totally
up to the application. The actual channel value will be pulled from
the render context provided in the Animate() method.
*/
void U2Controller::SetChannel(const TCHAR* name)
{
	m_channelVarHandle = U2VariableMgr::Instance()->GetVariableHandleByName(name);
	m_channelOffsetVarHandle = U2VariableMgr::Instance()->GetVariableHandleByName(
		(U2DynString(name) + _T("Offset")).Str());
}

//------------------------------------------------------------------------------
/**
Return the animation channel which drives this animation.
*/
const TCHAR* U2Controller::GetChannel()
{
	if(U2Variable::InvalidHandle == m_channelVarHandle)
	{
		return 0;
	}
	else 
	{
		return U2VariableMgr::Instance()->GetVariableName(m_channelVarHandle);
	}
}


void U2Controller::SetTarget(U2Spatial* pTarget)
{
	m_pTarget = pTarget;
}

U2Spatial* U2Controller::GetTarget() const
{
	return m_pTarget;
}
