// --------------------------------------------------------------------------
// Dingus project - a collection of subsystems for game/graphics applications
// --------------------------------------------------------------------------
#include "DXUT.h"

#include "RenderStats.h"

void CRenderStats::reset()
{
	// just set to zero...
	memset( this, 0, sizeof(*this) );
}
