#include <U2_3D/src/U23DLibPCH.h>
#include "U2AnimFactory.h"
#include "U2MemAnimation.h"

U2AnimFactory* U2AnimFactory::ms_pSingleton = 0;

U2AnimFactory::U2AnimFactory()
{
	
}


U2AnimFactory::~U2AnimFactory()
{
	ms_pSingleton = 0;
}

U2AnimFactory* U2AnimFactory::Create()
{
	ms_pSingleton = U2_NEW U2AnimFactory();
	return ms_pSingleton;
}

void U2AnimFactory::Terminate()
{
	U2_DELETE ms_pSingleton;
}


U2Animation* U2AnimFactory::CreateMemAnim(const U2DynString& resName)
{
	return U2_NEW U2MemAnimation;
}





