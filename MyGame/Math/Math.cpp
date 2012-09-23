#include "DXUT.h"
#include <vector>
#include "Math.h"



const float noMath::SQRT_1OVER3 = 0.57735026918962576450f;
const float noMath::SQRT_1OVER2 = 0.70710678118654752440f;
const float noMath::SQRT_THREE = 1.73205080756887729352f;
const float noMath::SQRT_TWO = 1.41421356237309504880f;
const float noMath::E = 2.71828182845904523536f;
const float noMath::PI = 3.14159265358979323846f;
const float noMath::ONEFOURTH_PI = 0.25 * PI;
const float noMath::HALF_PI = 0.5f * PI;
const float noMath::TWO_PI = 2.0f * PI;
const float noMath::M_DEG2RAD = PI / 180.f;
const float noMath::M_RAD2DEG = 180.0f / PI;
const float noMath::M_SEC2MS = 1000.f;
const float noMath::M_MS2SEC = 0.001f;
const float noMath::INFINITY = 1e30f;

const float noMath::FLT_EPSILON		= 1.192092896e-07f;

uint32 noMath::iSqrt[SQRT_TABLE_SIZE];
bool noMath::initialized = false;

void noMath::Init( void )
{
	union _flint fi, fo;

	for ( int i = 0; i < SQRT_TABLE_SIZE; i++ ) {
		fi.i	 = ((EXP_BIAS-1) << EXP_POS) | (i << LOOKUP_POS);
		fo.f	 = (float)( 1.0 / sqrt( fi.f ) );
		iSqrt[i] = ((dword)(((fo.i + (1<<(SEED_POS-2))) >> SEED_POS) & 0xFF))<<SEED_POS;
	}

	iSqrt[SQRT_TABLE_SIZE / 2] = ((dword)(0xFF))<<(SEED_POS); 
	initialized = true;
}

