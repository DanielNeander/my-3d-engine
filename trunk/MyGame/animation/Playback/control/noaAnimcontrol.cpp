#include "DXUT.h"
#include "noaAnimControl.h"


noaAnimControl::noaAnimControl( const noaAnimBinding* binding )
	:m_binding(binding), 
	m_localTime(0.f),
	m_weight(0.f)
{

}

noaAnimControl::~noaAnimControl()
{

}
