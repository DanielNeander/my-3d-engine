//#include <U2_3D/src/U23DLibPCH.h>
#include <stdafx.h>
#include "U2FileLoader.h"


U2FileLoader* U2FileLoader::ms_pSingleton = NULL;

U2FileLoader::U2FileLoader()
:m_iBytesRead(0),
m_iBytesWritten(0),
m_iNumSeeks(0)
{
	ms_pSingleton = this;

}


