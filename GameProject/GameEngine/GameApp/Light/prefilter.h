/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#ifndef __PREFILTER_H__
#define __PREFILTER_H__

void prefilterenvmap(float* envpixels, int width, int height, float* thecoeffs);
void tomatrix(void);

#endif