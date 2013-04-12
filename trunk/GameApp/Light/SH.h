/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#ifndef SH_H
#define SH_H

#include <math.h>
#include "sampling.h"

// Basic integer factorial
inline int Factorial(int v)
{
	if (v == 0)
		return (1);

	int result = v;
	while (--v > 0)
		result *= v;
	return (result);
}

// Defined as: n!! = n.(n - 2).(n - 4)..., n!!(0,-1)=1
inline int DoubleFactorial(int x)
{
	if (x == 0 || x == -1)
		return (1);

	int result = x;
	while ((x -= 2) > 0)
		result *= x;
	return (result);
}

// Re-normalisation constant for SH function
// Wonder what all this would look like in C++ TMP :)
inline float K(const int l, const int m)
{
	// Note that |m| is not used here as the SH function always passes positive m
	return (sqrt(((2 * l + 1) * Factorial(l - m)) / (4 * noMath::PI * Factorial(l + m))));
}

inline float P(const int l, const int m, const float x) {
// Rule 2 needs no previous results
if (l == m)
	return (powf(-1, m) * DoubleFactorial(2 * m - 1) * powf(sqrt(1 - x * x), m));

// Rule 3 requires the result for the same argument of the previous band
if (l == m + 1)
	return (x * (2 * m + 1) * P(m, m, x));

// Main reccurence used by rule 1 that uses result of the same argument from
// the previous two bands
return ((x * (2 * l - 1) * P(l - 1, m, x) - (l + m - 1) * P(l - 2, m, x)) / (l - m));
}

inline float calcSHCoeff(int l, int m, float theta, float phi) { //calculate an SH coeff
	if (m == 0)
		return (K(l, 0) * P(l, 0, cos(theta)));

	if (m > 0)
		return (sqrt(2.f) * K(l, m) * cos(m * phi) * P(l, m, cos(theta)));

	// m < 0, m is negated in call to K
	return (sqrt(2.f) * K(l, -m) * sin(-m * phi) * P(l, -m, cos(theta)));
}

inline void cartesianToSpherical(noVec3& vec, float* thetaphi) {
	float x = vec.x;
	float y = vec.y;
	float z = vec.z;

	float p = sqrtf(x*x + y*y + z*z);
	float S = sqrtf(x*x + z*z);

	thetaphi[0] = acos(x / p);
	if (z >= 0) {
		thetaphi[1] = asin(x / S);
	}
	else {
		thetaphi[1] = noMath::PI - asin(y / S);
	}
}

/// (l,m) => linear index. For indexing one set of coefficients in m_coef[].
int inline LinearIndex( int l, int m ) {
	return l*(l+1)+m;
}

inline float* calcSH(
	int numbands, 
	int numsamples, 
	Spheresample* samples, 
	bool isuniform,
	float* ranges
)
{
	float* thecoeffs = new float[numbands*numsamples]; //the array of all coeffs for all samples
	int coeffcounter = 0; //coeffs calculated

	for (int b=0; b < numbands; b++) {
		ranges[b*2+0] = FLT_MAX;
		ranges[b*2+1] = -FLT_MAX;
	}
	for (int i = 0; i < numsamples; ++i) {
		//cartesianToSpherical(samples[i], thetaphi);
		//theta = thetaphi[0];
		//phi = thetaphi[1];
		for (int l = 0; l < sqrtf(numbands); ++l) {
			for (int m = -l; m <= l; ++m) {
				//theta = PI / 2;
				//phi = PI / 2;
				thecoeffs[coeffcounter] = calcSHCoeff(l, m, samples[i].theta, samples[i].phi);
				if (isuniform) {
					thecoeffs[coeffcounter] *= sqrtf(sinf(samples[i].theta));
				}
				float v = thecoeffs[coeffcounter];
				int b = LinearIndex(l, m);
				ranges[b*2+0] = min(ranges[b*2+0], v);
				ranges[b*2+1] = max(ranges[b*2+1], v);
				coeffcounter++;
			}
		}
	}
	return thecoeffs;
}

#endif