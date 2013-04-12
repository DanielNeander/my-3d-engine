/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <math.h>
#include "basictypes.h"

namespace MATH
{
	int inline round( float f ) { return (int)(f + 0.5f); }
	float inline fract( float f ) { return (f - floorf(f)); }

	U32 inline MakeU32ByteSwap( Byte inx, Byte iny, Byte inz, Byte inw ) // intel byte order
	{
		return (inw << 24) | (inx << 16) | (iny << 8) | (inz << 0);
	}
};

#endif
