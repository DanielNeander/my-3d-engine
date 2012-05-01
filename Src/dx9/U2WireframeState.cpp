#include <u2_3d/src/U23DLibPCH.h>
#include "U2WireframeState.h"

IMPLEMENT_RTTI(U2WireframeState, U2Object);

IMPLEMENT_INITIALIZE(U2WireframeState);
IMPLEMENT_TERMINATE(U2WireframeState);

void U2WireframeState::Initialize()
{
	ms_aspDefault[WIREFRAME] = U2_NEW U2WireframeState;
	FILE_LOG(logDEBUG) << _T("U2WireFrameState::Initialize - Success...");
}

void U2WireframeState::Terminate()
{
	ms_aspDefault[WIREFRAME] = 0;
	FILE_LOG(logDEBUG) << _T("U2WireFrameState::Terminate - Success...");
}



