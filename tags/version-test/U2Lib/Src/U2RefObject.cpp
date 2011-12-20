#include <src/U2LibPCH.h>
#include "U2RefObject.h"


unsigned int U2RefObject::ms_uObjs = 0;

void U2RefObject::Delete()
{
	U2_DELETE this;
}