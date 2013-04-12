
/* * * * * * * * * * * * * Author's note * * * * * * * * * * * *\
*   _       _   _       _   _       _   _       _     _ _ _ _   *
*  |_|     |_| |_|     |_| |_|_   _|_| |_|     |_|  _|_|_|_|_|  *
*  |_|_ _ _|_| |_|     |_| |_|_|_|_|_| |_|     |_| |_|_ _ _     *
*  |_|_|_|_|_| |_|     |_| |_| |_| |_| |_|     |_|   |_|_|_|_   *
*  |_|     |_| |_|_ _ _|_| |_|     |_| |_|_ _ _|_|  _ _ _ _|_|  *
*  |_|     |_|   |_|_|_|   |_|     |_|   |_|_|_|   |_|_|_|_|    *
*                                                               *
*                     http://www.humus.name                     *
*                                                                *
* This file is a part of the work done by Humus. You are free to   *
* use the code in any way you like, modified, unmodified or copied   *
* into your own work. However, I expect you to respect these points:  *
*  - If you use this file and its contents unmodified, or use a major *
*    part of this file, please credit the author and leave this note. *
*  - For use in anything commercial, please request my approval.     *
*  - Share your work and ideas too as much as you can.             *
*                                                                *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _MATHTOOLS_H_
#define _MATHTOOLS_H_

#include "Math.h"
#include "Vec.h"
#include "Vector.h"
#include "Matrix.h"
#include <windows.h>
#include <xnamath.h>
#include "GameMath.h"

// Generates Poisson disk samples using the Dart throwing method
bool generatePoissonSamples(vec2 *samples, const int nSamples, const float minDist, const int maxFailedTries, const int nRetries, const bool includeCenter);

const mat4 ToMat4(const XMMATRIX& mat);
const mat4 ToMat4(const noMat4& mat);
const mat4 ToMat4( const Matrix& mat );
const float2 ToVec2(const XMFLOAT2& src);
const float3 ToVec3(const XMFLOAT3& src);
const float4 ToVec4(const XMFLOAT4& src);


//returns a random integer between x and y
inline int   RandInt(int x,int y) {return rand()%(y-x+1)+x;}

//returns a random double between zero and 1
inline double RandFloat()      {return ((rand())/(RAND_MAX+1.0));}

inline double RandInRange(double x, double y)
{
	return x + RandFloat()*(y-x);
}

//returns a random bool
inline bool   RandBool()
{
	if (RandInt(0,1)) return true;

	else return false;
}

//returns a random double in the range -1 < n < 1
inline double RandomClamped()    {return RandFloat() - RandFloat();}

//returns a random number with a normal distribution. See method at
//http://www.taygeta.com/random/gaussian.html
inline double RandGaussian(double mean = 0.0, double standard_deviation = 1.0)
{				        
	double x1, x2, w, y1;
	static double y2;
	static int use_last = 0;

	if (use_last)		        /* use value from previous call */
	{
		y1 = y2;
		use_last = 0;
	}
	else
	{
		do 
		{
			x1 = 2.0 * RandFloat() - 1.0;
			x2 = 2.0 * RandFloat() - 1.0;
			w = x1 * x1 + x2 * x2;
		}
		while ( w >= 1.0 );

		w = sqrt( (-2.0 * log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;
		use_last = 1;
	}

	return( mean + y1 * standard_deviation );
}



//-----------------------------------------------------------------------
//  
//  some handy little functions
//-----------------------------------------------------------------------


inline double Sigmoid(double input, double response = 1.0)
{
	return ( 1.0 / ( 1.0 + exp(-input / response)));
}

//rounds a double up or down depending on its value
inline int Rounded(double val)
{
	int    integral = (int)val;
	double mantissa = val - integral;

	if (mantissa < 0.5)
	{
		return integral;
	}

	else
	{
		return integral + 1;
	}
}

//rounds a double up or down depending on whether its 
//mantissa is higher or lower than offset
inline int RoundUnderOffset(double val, double offset)
{
	int    integral = (int)val;
	double mantissa = val - integral;

	if (mantissa < offset)
	{
		return integral;
	}

	else
	{
		return integral + 1;
	}
}
void ComputeTangentVectors(const noVec3 &v0, const noVec3 &v1, const noVec3 &v2, const noVec2 &t0, const noVec2 &t1, const noVec2 &t2, noVec3 &sdir, noVec3 &tdir, noVec3 &normal);
#endif // _MATHTOOLS_H_

