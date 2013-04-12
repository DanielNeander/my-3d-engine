/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#ifndef SAMPLING_H
#define SAMPLING_H

//#include "vector.h"

struct Spheresample {
	float x, y, z; //sample in cartesian coordinates 
	float theta, phi; //theta, phi for spherical coordinates
};

inline float VanDerCorput(unsigned n, unsigned scramble) {
    n = (n << 16) | (n >> 16);
    n = ((n & 0x00ff00ff) << 8) | ((n & 0xff00ff00) >> 8);
    n = ((n & 0x0f0f0f0f) << 4) | ((n & 0xf0f0f0f0) >> 4);
    n = ((n & 0x33333333) << 2) | ((n & 0xcccccccc) >> 2);
    n = ((n & 0x55555555) << 1) | ((n & 0xaaaaaaaa) >> 1);
    n ^= scramble;
    return (float)n / (float)0x100000000L;
}

inline float Sobol2(unsigned n, unsigned scramble) {
    for (unsigned v = 1U << 31; n != 0; n >>= 1, v ^= v >> 1)
        if (n & 0x1) scramble ^= v;
    return (float)scramble / (float)0x100000000L;
}

inline float RadicalInverse(int n, int base) {
	double val = 0;
	double invBase = 1. / base, invBi = invBase;
	while (n > 0) {
		// Compute next digit of radical inverse
		int d_i = (n % base);
		val += d_i * invBi;
		n /= base;
		invBi *= invBase;
	}
	return (float)val;
}

inline void WorldToLocal( noVec3* wv, const noVec3& lv, const noVec3* basis )
{
	*wv = noVec3(lv*basis[1], lv*basis[0], lv*basis[2]);
}

void OrthoBasis(const noVec3& N, noVec3& X, noVec3& Y);
void GenerateSphericalSamples( Spheresample* samples, float* pdfs, int numsamples, int kOffset, float* factor );

#endif
