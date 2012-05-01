#include <U2_3D/src/U23DLibPCH.h>
#include "U2AlphaState.h"


IMPLEMENT_RTTI(U2AlphaState, U2RenderState);
IMPLEMENT_INITIALIZE(U2AlphaState);
IMPLEMENT_TERMINATE(U2AlphaState);

void U2AlphaState::Initialize()
{
	ms_aspDefault[ALPHA] = U2_NEW U2AlphaState;
	FILE_LOG(logDEBUG) << _T("U2Dx9AlphaState::Initialize - Success...");
}

void U2AlphaState::Terminate()
{
	ms_aspDefault[ALPHA] = 0;
	FILE_LOG(logDEBUG) << _T("U2Dx9AlphaState::Terminate - Success...");
}

U2AlphaState::U2AlphaState()
{
	m_bBlendMode = false;
	m_eSrcBlend = BLEND_SRC_ALPHA;
	m_eDestBlend = BLEND_ONE_MINUS_SRC_ALPHA;
	m_bTestMode = false;
	m_eTestMode = TEST_ALWAYS;
	m_fTestRef = 0.f;
}

U2AlphaState::~U2AlphaState()
{

}

void U2AlphaState::SetAlphaBlending(bool bBlend)
{
	m_bBlendMode = bBlend;
}
bool U2AlphaState::GetAlphaBlending() const
{
	return m_bBlendMode;
}

void U2AlphaState::SetSrcBlendMode(BlendMode eSrcBlend)
{
	m_eSrcBlend = eSrcBlend;
}
U2AlphaState::BlendMode U2AlphaState::GetSrcBlendMode() const
{
	return m_eSrcBlend;
}

void U2AlphaState::SetDestBlendMode(BlendMode eDestBlend)
{
	m_eDestBlend = eDestBlend;
}

U2AlphaState::BlendMode U2AlphaState::GetDestBlendMode() const
{
	return m_eDestBlend;
}

void	U2AlphaState::SetAlphaTesting(bool bAlpah)
{
	m_bTestMode = bAlpah;
}

bool	U2AlphaState::GetAlphaTesting() const
{
	return m_bTestMode;
}

void	U2AlphaState::SetTestMode(TestMode eTestFunc)
{
	m_eTestMode = eTestFunc;	
}


U2AlphaState::TestMode U2AlphaState::GetTestMode() const
{
	return m_eTestMode;
}


void	U2AlphaState::SetTestRef(float fRef)
{
	m_fTestRef = fRef;
}
float	U2AlphaState::GetTestRef() const
{
	return m_fTestRef;
}

void	U2AlphaState::SetNoSort(bool bNoSort)
{
	m_bNoSort = bNoSort;
}


bool	U2AlphaState::GetNoSort() const
{
	return m_bNoSort;
}