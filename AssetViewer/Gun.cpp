#include "stdafx.h"
#include "Gun.h"

CGun::CGun()
:m_pMesh(0)
{

}

CGun::~CGun()
{
	InvalidateDeviceObjects();
}

HRESULT CGun::RestoreDeviceObjects()
{
	return S_OK;
}

void CGun::InvalidateDeviceObjects()
{
	
}