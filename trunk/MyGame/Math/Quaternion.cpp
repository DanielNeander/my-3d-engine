#include "DXUT.h"
#include "Quaternion.h"

noQuat & noQuat::Slerp( const noQuat &from, const noQuat &to, float r )
{
	// SLERP
	float dot = 0.f;
	//float dot = from*to;

	if (fabs(dot) > 0.9995f) {
		// fall back to LERP
		return noQuat::Lerp(r, from, to);
	}

	float a = acosf(dot) * r;
	noQuat q = (to - from * dot);
	q.Normalize();

	return from * cosf(a) + q * sinf(a);
}

