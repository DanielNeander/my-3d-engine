#include "DXUT.h"
#include "noaDefaultAnimControl.h"

noaDefaultAnimControl::noaDefaultAnimControl( const noaAnimBinding* binding, hkBool startEaseIn /*= true*/, hkInt32 maxCycles /*= -1*/ )
	:noaAnimControl(binding),	
	m_maxCycles(maxCycles)
{
	if (startEaseIn)
	{

	}
}

void noaDefaultAnimControl::Update( hkReal stepDelta )
{
	m_localTime += stepDelta;

}

