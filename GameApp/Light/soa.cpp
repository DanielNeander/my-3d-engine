#include "stdafx.h"
#include "string.h"
#include "malloc.h"
#include "soa.h"

// takes an AoS[height][width] <-> SoA[width][height]
void ToSoA( float* soa, const float* aos, int width, int height )
{
	memset( soa, 0, sizeof(float)*(width*height));
	for (int r=0; r < height; r++) {
		for (int c=0; c < width; c++) {
			soa[c*height + r] = aos[r*width + c];		
		}
	}
}

void ToSoa2( float* soa2, const float* soa )
{
	for (int r=0; r < 3; r++) {
		float* soa2ptr = soa2 + 4*3*r;
		const float* soaptr = soa + 3*3*r;

		soa2ptr[0] = *soaptr++;
		soa2ptr[1] = 0.f;
		soa2ptr[2] = 0.f;
		soa2ptr[3] = 0.f;
		soa2ptr += 4;

		memcpy( soa2ptr, soaptr, sizeof(float)*8 );
	}
}
