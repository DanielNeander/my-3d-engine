/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#ifndef SOA_H
#define SOA_H

// AoS <-> SoA conversions:

// takes an AoS[height][width] <-> SoA[width][height]
void ToSoA( float* soa, const float* aos, int width, int height );
void ToSoa2( float* soa2, const float* soa );

#endif