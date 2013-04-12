/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/
#include "stdafx.h"
#include <xutility>
#include <math.h>
#include <stdlib.h>

#include "Vector.h"
#include "sampling.h"

#define kInvPi	(1.f/PI)

void OrthoBasis(const noVec3& N, noVec3& X, noVec3& Y) {
	int min_index = 0;
	
	if (fabsf(N.x) > fabsf(N.y)) {
		min_index = 1;
	}
	if (fabsf(N.y) > fabsf(N.z)) {
		min_index = 2;
	}
	if (min_index == 0) {
		X = noVec3(0, -N.z, N.y);
	}
	else if (min_index == 1) {
		X = noVec3(-N.z, 0, N.x);
	}
	else if (min_index == 2) {
		X = noVec3(-N.y, N.x, 0);
	}

	Y = X.Multiply(N);

	X.Normalize();
	Y.Normalize();
}

void GenerateSphericalSamples( Spheresample* samples, float* pdfs, int numsamples, int kOffset, float* normfactor )
{
	float nextsample = 0;
	noVec3 basis[3];

	basis[0] = noVec3(1,0,0); //generate generic basis
	basis[1] = noVec3(0,1,0);
	basis[2] = noVec3(0,0,1);
	//now generate cos-weighted samples
	int index = 0;
	int sampledim = sqrtf(numsamples);

	for (int i = 0; i < sampledim; ++i) {
#if	0
		for (int j = 0; j < sampledim; j++) {
			float u1 = rand() / (float)RAND_MAX;
			int offset = kOffset + nextsample/2;
			u1 = VanDerCorput(offset, u1);
			float u2 = (i + 0.5f)/numsamples;
			nextsample += 2;
			noVec3& dir = *(noVec3*)&samples[i*sampledim+j];
			double theta=2.0*acos(sqrt(1.0-dir.x));
			double phi=2.0*PI*dir.y;
			samples[i*sampledim+j].theta = theta;
			samples[i*sampledim+j].phi = phi;
			pdfs[i*sampledim+j] = pdf;
		}
		*normfactor = ((PI) / numsamples);
#else
		for (int j = 0; j < sampledim; j++) {
			double x=(i+((double)rand()/RAND_MAX))/sampledim;
			double y=(j+((double)rand()/RAND_MAX))/sampledim;
			double theta=2.0*acos(sqrt(1.0-x));
			double phi=2.0 * noMath::PI * y;
			samples[index].theta = theta;
			samples[index].phi = phi;
			samples[index].x = sin(theta)*cos(phi);
			samples[index].y = sin(theta)*sin(phi);
			samples[index].z = cos(theta);
			pdfs[i*sampledim+j] = 1;

			index++;
		}
		*normfactor = ((4 * noMath::PI) / numsamples);
#endif
	}
}