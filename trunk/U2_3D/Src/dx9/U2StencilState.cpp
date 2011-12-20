#include <U2_3D/src/U23DLibPCH.h>
#include "U2StencilState.h"

IMPLEMENT_RTTI(U2StencilState, U2RenderState);
IMPLEMENT_INITIALIZE(U2StencilState);
IMPLEMENT_TERMINATE(U2StencilState);

void U2StencilState::Initialize()
{
	ms_aspDefault[STENCIL] = U2_NEW U2StencilState;
	FILE_LOG(logDEBUG) << _T("U2StencilState::Initialize - Success...");
}

void U2StencilState::Terminate()
{
	ms_aspDefault[STENCIL] = 0;
	FILE_LOG(logDEBUG) << _T("U2StencilState::Terminate - Success...");
}



U2StencilState::U2StencilState()
{
	m_bEnabled = false;
	m_eCompareFunc = CF_GREATER;
	m_uRefVal = 0;
	m_uMaskVal = 0xffffffff;
	m_uWriteMaskVal = 0xffffffff;
	m_eFail = OP_KEEP;		
	m_eZFail = OP_KEEP;		
	m_eZPass = OP_INCREMENT;	
	m_eCullMode = CM_CCW;
}

U2StencilState::~U2StencilState()
{

}

void U2StencilState::SetStencilFlag(bool bOn)
{	
	m_bEnabled = bOn;	
}


bool U2StencilState::GetStencilFlag() const
{
	return m_bEnabled;
}


void U2StencilState::SetStencilFunc(CompareFunc eFun)
{
	m_eCompareFunc = eFun;
}


U2StencilState::CompareFunc U2StencilState::GetStencilFunc() const
{
	return m_eCompareFunc;
}

void U2StencilState::SetStencilReference(UINT uRef)
{
	m_uRefVal = uRef;
}


uint32 U2StencilState::GetStencilReference() const
{
	return m_uRefVal;
}

void U2StencilState::SetStencilMask(UINT32 uMask)
{
	m_uMaskVal = uMask;
}


uint32 U2StencilState::GetStencilMask() const
{
	return m_uMaskVal;
}

void U2StencilState::SetStencilFailOp(OperationType eOpt)
{
	m_eFail = eOpt;
}

U2StencilState::OperationType U2StencilState::GetStencilFailOp() const
{
	return m_eFail;
}

void U2StencilState::SetStencilZFailOp(OperationType eOpt)
{
	m_eZFail = eOpt;
}

U2StencilState::OperationType U2StencilState::GetStencilZFailOp() const
{
	return m_eZFail;
}

void U2StencilState::SetStencilZPassaOp(OperationType eOpt)
{
	m_eZPass = eOpt;
}


U2StencilState::OperationType U2StencilState::GetStencilZPassOp() const
{
	return m_eZPass;
}


void	U2StencilState::SetCullMode(U2StencilState::CullMode eCull)
{
	m_eCullMode = eCull;
}


U2StencilState::CullMode U2StencilState::GetCullMode() const
{
	return m_eCullMode;
}
