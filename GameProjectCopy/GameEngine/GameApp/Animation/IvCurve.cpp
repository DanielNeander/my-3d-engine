#include "stdafx.h"
#include "EngineCore/Math/GaussianElim.h"
#include "IvCurve.h"
#include "CurveLines.h"


const char eol = '\n';
//-------------------------------------------------------------------------------
//-- Static Members -------------------------------------------------------------
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
//-- Methods --------------------------------------------------------------------
//-------------------------------------------------------------------------------

//-------------------------------------------------------------------------------
// @ IvLinear::IvLinear()
//-------------------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------------------
IvLinear::IvLinear() :
mPositions( 0 ),
	mTimes( 0 ),
	mCount( 0 )
{
}   // End of IvLinear::IvLinear()


//-------------------------------------------------------------------------------
// @ IvLinear::IvLinear()
//-------------------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------------------
IvLinear::~IvLinear()
{
	delete [] mPositions;
	delete [] mTimes;

}   // End of IvLinear::~IvLinear()


//-------------------------------------------------------------------------------
// @ operator<<()
//-------------------------------------------------------------------------------
// Text output for debugging
//-------------------------------------------------------------------------------
std::ostream& 
	operator<<(std::ostream& out, const IvLinear& source)
{
	out << source.mCount << eol;
	for (unsigned int i = 0; i < source.mCount; ++i )
	{
		out << source.mTimes[i] << ':' << source.mPositions[i].x << ", " <<
			source.mPositions[i].y << ", " << source.mPositions[i].z << eol;
	}

	return out;

}   // End of operator<<()


//-------------------------------------------------------------------------------
// @ IvLinear::Initialize()
//-------------------------------------------------------------------------------
// Set up sample points
//-------------------------------------------------------------------------------
bool
	IvLinear::Initialize( const noVec3* samples, const float* times,
	unsigned int count )
{
	// make sure not already initialized
	if (mCount != 0)
		return false;

	// make sure data is valid
	if ( count < 2 || !samples || !times )
		return false;

	// set up arrays
	mPositions = new noVec3[count];
	mTimes = new float[count];
	mCount = count;

	// copy data
	for ( unsigned int i = 0; i < count; ++i )
	{
		mPositions[i] = samples[i];
		mTimes[i] = times[i];
	}

	return true;

}   // End of IvLinear::Initialize()


//-------------------------------------------------------------------------------
// @ IvLinear::Clean()
//-------------------------------------------------------------------------------
// Clean out data
//-------------------------------------------------------------------------------
void 
	IvLinear::Clean()
{
	delete [] mPositions;
	delete [] mTimes;
	mCount = 0;

}   // End of IvLinear::Clean()


//-------------------------------------------------------------------------------
// @ IvLinear::Evaluate()
//-------------------------------------------------------------------------------
// Evaluate spline
//-------------------------------------------------------------------------------
noVec3
	IvLinear::Evaluate( float t )
{
	// make sure data is valid
	assert( mCount >= 2 );
	if ( mCount < 2 )
		return vec3_zero;

	// handle boundary conditions
	if ( t <= mTimes[0] )
		return mPositions[0];
	else if ( t >= mTimes[mCount-1] )
		return mPositions[mCount-1];

	// find segment and parameter
	unsigned int i;
	for ( i = 0; i < mCount-1; ++i )
	{
		if ( t < mTimes[i+1] )
		{
			break;
		}
	}
	float t0 = mTimes[i];
	float t1 = mTimes[i+1];
	float u = (t - t0)/(t1 - t0);

	// evaluate
	return (1-u)*mPositions[i] + u*mPositions[i+1];

}   // End of IvLinear::Initialize()


//-------------------------------------------------------------------------------
// @ IvCatmullRom::IvCatmullRom()
//-------------------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------------------
IvCatmullRom::IvCatmullRom() :
mPositions( 0 ),
	mTimes( 0 ),
	mLengths( 0 ),
	mTotalLength( 0.0f ),
	mCount( 0 )
{
}   // End of IvCatmullRom::IvCatmullRom()


//-------------------------------------------------------------------------------
// @ IvCatmullRom::IvCatmullRom()
//-------------------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------------------
IvCatmullRom::~IvCatmullRom()
{
	Clean();

}   // End of IvCatmullRom::~IvCatmullRom()


//-------------------------------------------------------------------------------
// @ operator<<()
//-------------------------------------------------------------------------------
// Text output for debugging
//-------------------------------------------------------------------------------
std::ostream& 
	operator<<(std::ostream& out, const IvCatmullRom& source)
{
	out << source.mCount << eol;
	for (unsigned int i = 0; i < source.mCount; ++i )
	{
		out << source.mTimes[i] << ':' << source.mPositions[i].x << ", " <<
			source.mPositions[i].y << ", " << source.mPositions[i].z << eol;
	}

	return out;

}   // End of operator<<()


//-------------------------------------------------------------------------------
// @ IvCatmullRom::Initialize()
//-------------------------------------------------------------------------------
// Set up sample points
//-------------------------------------------------------------------------------
bool
	IvCatmullRom::Initialize( const noVec3* positions,
	const float* times,
	unsigned int count )

{
	// make sure not already initialized
	if (mCount != 0)
		return false;

	// make sure data is valid
	if ( count < 4 || !positions || !times )
		return false;

	// set up arrays
	mPositions = new noVec3[count];
	mTimes = new float[count];
	mCount = count;

	// copy data
	unsigned int i;
	for ( i = 0; i < count; ++i )
	{
		mPositions[i] = positions[i];
		mTimes[i] = times[i];
	}

	// set up curve segment lengths
	mLengths = new float[count-1];
	mTotalLength = 0.0f;
	for ( i = 0; i < count-1; ++i )
	{
		mLengths[i] = SegmentArcLength(i, 0.0f, 1.0f);
		mTotalLength += mLengths[i];
	}

	return true;

}   // End of IvCatmullRom::Initialize()


//-------------------------------------------------------------------------------
// @ IvCatmullRom::Clean()
//-------------------------------------------------------------------------------
// Clean out data
//-------------------------------------------------------------------------------
void IvCatmullRom::Clean()
{
	delete [] mPositions;
	delete [] mTimes;
	delete [] mLengths;
	mTotalLength = 0.0f;
	mCount = 0;

}   // End of IvCatmullRom::Clean()


//-------------------------------------------------------------------------------
// @ IvCatmullRom::Evaluate()
//-------------------------------------------------------------------------------
// Evaluate spline
//-------------------------------------------------------------------------------
noVec3
	IvCatmullRom::Evaluate( float t )
{
	// make sure data is valid
	assert( mCount >= 2 );
	if ( mCount < 2 )
		return vec3_zero;

	// handle boundary conditions
	if ( t <= mTimes[0] )
		return mPositions[0];
	else if ( t >= mTimes[mCount-1] )
		return mPositions[mCount-1];

	// find segment and parameter
	unsigned int i;  // segment #
	for ( i = 0; i < mCount-1; ++i )
	{
		if ( t <= mTimes[i+1] )
		{
			break;
		}
	}
	assert( i >= 0 && i < mCount );

	float t0 = mTimes[i];
	float t1 = mTimes[i+1];
	float u = (t - t0)/(t1 - t0);

	// quadratic Catmull-Rom for Q_0
	if (i == 0)
	{
		noVec3 A = mPositions[0] - 2.0f*mPositions[1] + mPositions[2];
		noVec3 B = 4.0f*mPositions[1] - 3.0f*mPositions[0] - mPositions[2];

		return mPositions[0] + (0.5f*u)*(B + u*A);
	}
	// quadratic Catmull-Rom for Q_n-1
	else if (i >= mCount-2)
	{
		i = mCount-2;
		noVec3 A = mPositions[i-1] - 2.0f*mPositions[i] + mPositions[i+1];
		noVec3 B = mPositions[i+1] - mPositions[i-1];

		return mPositions[i] + (0.5f*u)*(B + u*A);
	}
	// cubic Catmull-Rom for interior segments
	else
	{
		// evaluate
		noVec3 A = 3.0f*mPositions[i]
		- mPositions[i-1]
		- 3.0f*mPositions[i+1]
		+ mPositions[i+2];
		noVec3 B = 2.0f*mPositions[i-1]
		- 5.0f*mPositions[i]
		+ 4.0f*mPositions[i+1]
		- mPositions[i+2];
		noVec3 C = mPositions[i+1] - mPositions[i-1];

		return mPositions[i] + (0.5f*u)*(C + u*(B + u*A));
	}

}   // End of IvCatmullRom::Evaluate()


//-------------------------------------------------------------------------------
// @ IvCatmullRom::Velocity()
//-------------------------------------------------------------------------------
// Evaluate derivative at parameter t
//-------------------------------------------------------------------------------
noVec3
	IvCatmullRom::Velocity( float t )
{
	// make sure data is valid
	assert( mCount >= 2 );
	if ( mCount < 2 )
		return vec3_zero;

	// handle boundary conditions
	if ( t <= mTimes[0] )
		t = 0.0f;
	else if ( t > mTimes[mCount-1] )
		t = mTimes[mCount-1];

	// find segment and parameter
	unsigned int i;
	for ( i = 0; i < mCount-1; ++i )
	{
		if ( t <= mTimes[i+1] )
		{
			break;
		}
	}
	float t0 = mTimes[i];
	float t1 = mTimes[i+1];
	float u = (t - t0)/(t1 - t0);

	// evaluate
	// quadratic Catmull-Rom for Q_0
	if (i == 0)
	{
		noVec3 A = mPositions[0] - 2.0f*mPositions[1] + mPositions[2];
		noVec3 B = 4.0f*mPositions[1] - 3.0f*mPositions[0] - mPositions[2];

		return 0.5f*B + u*A;
	}
	// quadratic Catmull-Rom for Q_n-1
	else if (i >= mCount-2)
	{
		i = mCount-2;
		noVec3 A = mPositions[i-1] - 2.0f*mPositions[i] + mPositions[i+1];
		noVec3 B = mPositions[i+1] - mPositions[i-1];

		return 0.5f*B + u*A;
	}
	// cubic Catmull-Rom for interior segments
	else
	{
		// evaluate
		noVec3 A = 3.0f*mPositions[i]
		- mPositions[i-1]
		- 3.0f*mPositions[i+1]
		+ mPositions[i+2];
		noVec3 B = 2.0f*mPositions[i-1]
		- 5.0f*mPositions[i]
		+ 4.0f*mPositions[i+1]
		- mPositions[i+2];
		noVec3 C = mPositions[i+1] - mPositions[i-1];

		return 0.5f*C + u*(B + 1.5f*u*A);
	}

}   // End of IvCatmullRom::Velocity()


//-------------------------------------------------------------------------------
// @ IvCatmullRom::Acceleration()
//-------------------------------------------------------------------------------
// Evaluate second derivative at parameter t
//-------------------------------------------------------------------------------
noVec3
	IvCatmullRom::Acceleration( float t )
{
	// make sure data is valid
	assert( mCount >= 2 );
	if ( mCount < 2 )
		return vec3_zero;

	// handle boundary conditions
	if ( t <= mTimes[0] )
		t = 0.0f;
	else if ( t > mTimes[mCount-1] )
		t = mTimes[mCount-1];

	// find segment and parameter
	unsigned int i;
	for ( i = 0; i < mCount-1; ++i )
	{
		if ( t <= mTimes[i+1] )
		{
			break;
		}
	}
	float t0 = mTimes[i];
	float t1 = mTimes[i+1];
	float u = (t - t0)/(t1 - t0);

	// evaluate
	// quadratic Catmull-Rom for Q_0
	if (i == 0)
	{
		return mPositions[0] - 2.0f*mPositions[1] + mPositions[2];
	}
	// quadratic Catmull-Rom for Q_n-1
	else if (i >= mCount-2)
	{
		i = mCount-2;
		return mPositions[i-1] - 2.0f*mPositions[i] + mPositions[i+1];
	}
	// cubic Catmull-Rom for interior segments
	else
	{
		// evaluate
		noVec3 A = 3.0f*mPositions[i]
		- mPositions[i-1]
		- 3.0f*mPositions[i+1]
		+ mPositions[i+2];
		noVec3 B = 2.0f*mPositions[i-1]
		- 5.0f*mPositions[i]
		+ 4.0f*mPositions[i+1]
		- mPositions[i+2];

		return B + (3.0f*u)*A;
	}

}   // End of IvCatmullRom::Acceleration()


//-------------------------------------------------------------------------------
// @ IvCatmullRom::FindParameterByDistance()
//-------------------------------------------------------------------------------
// Find parameter s distance in arc length from Q(t1)
// Returns max float if can't find it
//
// This extends the approach in the text and uses a mixture of bisection and 
// Newton-Raphson to find the root.  The result is more stable than Newton-
// Raphson alone because a) we won't end up with a situation where we divide by 
// zero in the Newton-Raphson step and b) the end result converges faster.
//
// See Numerical Recipes or http://www.essentialmath.com/blog for more details.
//-------------------------------------------------------------------------------
float 
	IvCatmullRom::FindParameterByDistance( float t1, float s )
{
	// initialize bisection endpoints
	float a = t1;
	float b = mTimes[mCount-1];

	// ensure that we remain within valid parameter space
	if ( s >= ArcLength(t1, b) )
		return b;
	if ( s <= 0.0f )
		return a;

	// make first guess
	float p = t1 + s*(mTimes[mCount-1]-mTimes[0])/mTotalLength;

	// iterate and look for zeros
	for ( uint32 i = 0; i < 32; ++i )
	{
		// compute function value and test against zero
		float func = ArcLength(t1, p) - s;
		if ( noMath::Abs(func) < 1.0e-03f )
		{
			return p;
		}

		// update bisection endpoints
		if ( func < 0.0f )
		{
			a = p;
		}
		else
		{
			b = p;
		}

		// get speed along curve
		float speed = Velocity(p).Length();

		// if result will lie outside [a,b] 
		if ( ((p-a)*speed - func)*((p-b)*speed - func) > -1.0e-3f )
		{
			// do bisection
			p = 0.5f*(a+b);
		}    
		else
		{
			// otherwise Newton-Raphson
			p -= func/speed;
		}
	}

	// done iterating, return failure case
	return FLT_MAX;

}   // End of IvCatmullRom::FindParameterByDistance()


//-------------------------------------------------------------------------------
// @ IvCatmullRom::ArcLength()
//-------------------------------------------------------------------------------
// Find length of curve between parameters t1 and t2
//-------------------------------------------------------------------------------
float 
	IvCatmullRom::ArcLength( float t1, float t2 )
{
	if ( t2 <= t1 )
		return 0.0f;

	if ( t1 < mTimes[0] )
		t1 = mTimes[0];

	if ( t2 > mTimes[mCount-1] )
		t2 = mTimes[mCount-1];

	// find segment and parameter
	unsigned int seg1;
	for ( seg1 = 0; seg1 < mCount-1; ++seg1 )
	{
		if ( t1 <= mTimes[seg1+1] )
		{
			break;
		}
	}
	float u1 = (t1 - mTimes[seg1])/(mTimes[seg1+1] - mTimes[seg1]);

	// find segment and parameter
	unsigned int seg2;
	for ( seg2 = 0; seg2 < mCount-1; ++seg2 )
	{
		if ( t2 <= mTimes[seg2+1] )
		{
			break;
		}
	}
	float u2 = (t2 - mTimes[seg2])/(mTimes[seg2+1] - mTimes[seg2]);

	float result;
	// both parameters lie in one segment
	if ( seg1 == seg2 )
	{
		result = SegmentArcLength( seg1, u1, u2 );
	}
	// parameters cross segments
	else
	{
		result = SegmentArcLength( seg1, u1, 1.0f );
		for ( uint32 i = seg1+1; i < seg2; ++i )
			result += mLengths[i];
		result += SegmentArcLength( seg2, 0.0f, u2 );
	}

	return result;

}   // End of IvCatmullRom::ArcLength()


//-------------------------------------------------------------------------------
// @ IvCatmullRom::SegmentArcLength()
//-------------------------------------------------------------------------------
// Find length of curve segment between parameters u1 and u2
//-------------------------------------------------------------------------------
float 
	IvCatmullRom::SegmentArcLength( uint32 i, float u1, float u2 )
{
	static const float x[] =
	{
		0.0000000000, 0.5384693101, -0.5384693101, 0.9061798459, -0.9061798459 
	};

	static const float c[] =
	{
		0.5688888889, 0.4786286705, 0.4786286705, 0.2369268850, 0.2369268850
	};

	assert(i >= 0 && i < mCount-1);

	if ( u2 <= u1 )
		return 0.0f;

	if ( u1 < 0.0f )
		u1 = 0.0f;

	if ( u2 > 1.0f )
		u2 = 1.0f;

	// use Gaussian quadrature
	float sum = 0.0f;
	noVec3 A, B, C;
	if (i == 0)
	{
		A = mPositions[0] - 2.0f*mPositions[1] + mPositions[2];
		B = 4.0f*mPositions[1] - 3.0f*mPositions[0] - mPositions[2];

	}
	// quadratic Catmull-Rom for Q_n-1
	else if (i >= mCount-2)
	{
		i = mCount-2;
		A = mPositions[i-1] - 2.0f*mPositions[i] + mPositions[i+1];
		B = mPositions[i+1] - mPositions[i-1];
	}
	// cubic Catmull-Rom for interior segments
	else
	{
		A = 3.0f*mPositions[i]
		- mPositions[i-1]
		- 3.0f*mPositions[i+1]
		+ mPositions[i+2];
		B = 2.0f*mPositions[i-1]
		- 5.0f*mPositions[i]
		+ 4.0f*mPositions[i+1]
		- mPositions[i+2];
		C = mPositions[i+1] - mPositions[i-1];
	}

	for ( uint32 j = 0; j < 5; ++j )
	{
		float u = 0.5f*((u2 - u1)*x[j] + u2 + u1);
		noVec3 derivative;
		if ( i == 0 || i >= mCount-2)
			derivative = 0.5f*B + u*A;
		else
			derivative = 0.5f*C + u*(B + 1.5f*u*A);
		sum += c[j]*derivative.Length();
	}
	sum *= 0.5f*(u2-u1);

	return sum;

}   // End of IvCatmullRom::SegmentArcLength()


IvLagrange::IvLagrange() :
mPositions( 0 ),
	mTimes( 0 ),
	mDenomRecip( 0 ),
	mCount( 0 )
{
}   // End of IvLagrange::IvLagrange()


//-------------------------------------------------------------------------------
// @ IvLagrange::IvLagrange()
//-------------------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------------------
IvLagrange::~IvLagrange()
{
	delete [] mPositions;
	delete [] mTimes;
	delete [] mDenomRecip;

}   // End of IvLagrange::~IvLagrange()


//-------------------------------------------------------------------------------
// @ operator<<()
//-------------------------------------------------------------------------------
// Text output for debugging
//-------------------------------------------------------------------------------
std::ostream& 
	operator<<(std::ostream& out, const IvLagrange& source)
{
	out << source.mCount << eol;
	for (unsigned int i = 0; i < source.mCount; ++i )
	{
		out << source.mTimes[i] << ':' << source.mPositions[i].x << ", " <<
			source.mPositions[i].y << ", " << source.mPositions[i].z << eol;
	}

	return out;

}   // End of operator<<()


//-------------------------------------------------------------------------------
// @ IvLagrange::Initialize()
//-------------------------------------------------------------------------------
// Set up sample points
//-------------------------------------------------------------------------------
bool
	IvLagrange::Initialize( const noVec3* samples, const float* times,
	unsigned int count )
{
	// make sure not already initialized
	if (mCount != 0)
		return false;

	// make sure data is valid
	if ( count < 2 || !samples || !times )
		return false;

	// set up arrays
	mPositions = new noVec3[count];
	mTimes = new float[count];
	mDenomRecip = new float[count];
	mCount = count;

	// copy data
	for ( unsigned int k = 0; k < count; ++k )
	{
		mPositions[k] = samples[k];
		mTimes[k] = times[k];
		float denom = 1.0f;
		for ( unsigned int i = 0; i < count; ++i )
		{
			if (k != i)
			{
				denom *= (times[k] - times[i]);
			}
		}
		mDenomRecip[k] = 1.0f/denom;
	}

	return true;

}   // End of IvLagrange::Initialize()


//-------------------------------------------------------------------------------
// @ IvLagrange::Clean()
//-------------------------------------------------------------------------------
// Clean out data
//-------------------------------------------------------------------------------
void 
	IvLagrange::Clean()
{
	delete [] mPositions;
	delete [] mTimes;
	delete [] mDenomRecip;
	mCount = 0;

}   // End of IvLagrange::Clean()


//-------------------------------------------------------------------------------
// @ IvLagrange::Evaluate()
//-------------------------------------------------------------------------------
// Evaluate curve
//-------------------------------------------------------------------------------
noVec3
	IvLagrange::Evaluate( float t )
{
	// make sure data is valid
	assert( mCount >= 2 );
	if ( mCount < 2 )
		return vec3_zero;

	// handle boundary conditions
	if ( t <= mTimes[0] )
		return mPositions[0];
	else if ( t >= mTimes[mCount-1] )
		return mPositions[mCount-1];

	// evaluate
	noVec3 result(0.0f, 0.0f, 0.0f);
	for ( unsigned int k = 0; k < mCount; ++k )
	{
		float num = 1.0f;
		for ( unsigned int i = 0; i < mCount; ++i )
		{
			if (k != i)
			{
				num *= (t - mTimes[i]);
			}
		}

		result += num*mDenomRecip[k]*mPositions[k];
	}

	return result;

}   // End of IvLagrange::Initialize()

IvUniformBSpline::IvUniformBSpline() :
mPositions( 0 ),
	mTimes( 0 ),
	mCount( 0 )
{
}   // End of IvUniformBSpline::IvUniformBSpline()


//-------------------------------------------------------------------------------
// @ IvUniformBSpline::IvUniformBSpline()
//-------------------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------------------
IvUniformBSpline::~IvUniformBSpline()
{
	Clean();

}   // End of IvUniformBSpline::~IvUniformBSpline()


//-------------------------------------------------------------------------------
// @ operator<<()
//-------------------------------------------------------------------------------
// Text output for debugging
//-------------------------------------------------------------------------------
std::ostream& 
	operator<<(std::ostream& out, const IvUniformBSpline& source)
{
	out << source.mCount << eol;
	for (unsigned int i = 0; i < source.mCount; ++i )
	{
		out << source.mTimes[i] << ':' << source.mPositions[i].x << ", " <<
			source.mPositions[i].y << ", " << source.mPositions[i].z << eol;
	}

	return out;

}   // End of operator<<()


//-------------------------------------------------------------------------------
// @ IvUniformBSpline::Initialize()
//-------------------------------------------------------------------------------
// Set up sample points
//-------------------------------------------------------------------------------
bool
	IvUniformBSpline::Initialize( const noVec3* positions,
	unsigned int count,
	float startTime, float endTime )

{
	// make sure not already initialized
	if ( mCount != 0 )
		return false;

	// make sure data is valid
	if ( count < 2 || !positions )
		return false;

	// set up arrays
	mPositions = new noVec3[count+4];
	mTimes = new float[count+2];
	mCount = count+4;

	// copy position data
	// triplicate start and end points so that curve passes through them
	mPositions[0] = mPositions[1] = positions[0];
	unsigned int i;
	for ( i = 0; i < count; ++i )
	{
		mPositions[i+2] = positions[i];
	}
	mPositions[mCount-1] = mPositions[mCount-2] = positions[count-1];

	// now set up times
	// we subdivide interval to get arrival times at each knot location
	float dt = (endTime - startTime)/(float)(count+1);
	mTimes[0] = startTime;
	for ( i = 0; i < count; ++i )
	{
		mTimes[i+1] = mTimes[i]+dt;
	}
	mTimes[count+1] = endTime;

	// set up curve segment lengths
	mLengths = new float[mCount-3];
	mTotalLength = 0.0f;
	for ( i = 0; i < mCount-3; ++i )
	{
		mLengths[i] = SegmentArcLength(i, 0.0f, 1.0f);
		mTotalLength += mLengths[i];
	}

	return true;

}   // End of IvUniformBSpline::Initialize()


//-------------------------------------------------------------------------------
// @ IvUniformBSpline::Clean()
//-------------------------------------------------------------------------------
// Clean out data
//-------------------------------------------------------------------------------
void
	IvUniformBSpline::Clean()
{
	delete [] mPositions;
	delete [] mTimes;
	delete [] mLengths;
	mTotalLength = 0.0f;
	mCount = 0;

}   // End of IvUniformBSpline::~Clean()


//-------------------------------------------------------------------------------
// @ IvUniformBSpline::Evaluate()
//-------------------------------------------------------------------------------
// Evaluate spline
//-------------------------------------------------------------------------------
noVec3
	IvUniformBSpline::Evaluate( float t )
{
	// make sure data is valid
	assert( mCount >= 6 );
	if ( mCount < 6 )
		return vec3_zero;

	// handle boundary conditions
	if ( t <= mTimes[0] )
		return mPositions[0];
	else if ( t >= mTimes[mCount-3] )
		return mPositions[mCount-3];

	// find segment and parameter
	unsigned int i;  // segment #
	for ( i = 0; i < mCount-3; ++i )
	{
		if ( t <= mTimes[i+1] )
		{
			break;
		}
	}
	assert( i >= 0 && i < mCount-3 );

	float t0 = mTimes[i];
	float t1 = mTimes[i+1];
	float u = (t - t0)/(t1 - t0);

	// match segment index to standard B-spline terminology
	i += 3;

	// evaluate
	noVec3 A = mPositions[i]
	- 3.0f*mPositions[i-1]
	+ 3.0f*mPositions[i-2]
	- mPositions[i-3];
	noVec3 B = 3.0f*mPositions[i-1]
	- 6.0f*mPositions[i-2]
	+ 3.0f*mPositions[i-3];
	noVec3 C = 3.0f*mPositions[i-1] - 3.0f*mPositions[i-3];
	noVec3 D = mPositions[i-1]
	+ 4.0f*mPositions[i-2]
	+ mPositions[i-3];

	return (D + u*(C + u*(B + u*A)))/6.0f;

}   // End of IvUniformBSpline::Evaluate()


//-------------------------------------------------------------------------------
// @ IvUniformBSpline::Velocity()
//-------------------------------------------------------------------------------
// Evaluate derivative at parameter t
//-------------------------------------------------------------------------------
noVec3
	IvUniformBSpline::Velocity( float t )
{
	// make sure data is valid
	assert( mCount >= 2 );
	if ( mCount < 2 )
		return vec3_zero;

	// handle boundary conditions
	if ( t <= mTimes[0] )
		t = mTimes[0];
	else if ( t >= mTimes[mCount-3] )
		t = mTimes[mCount-3];

	// find segment and parameter
	unsigned int i;
	for ( i = 0; i < mCount-1; ++i )
	{
		if ( t <= mTimes[i+1] )
		{
			break;
		}
	}
	float t0 = mTimes[i];
	float t1 = mTimes[i+1];
	float u = (t - t0)/(t1 - t0);

	// match segment index to standard B-spline terminology
	i += 3;

	// evaluate
	noVec3 A = mPositions[i]
	- 3.0f*mPositions[i-1]
	+ 3.0f*mPositions[i-2]
	- mPositions[i-3];
	noVec3 B = 3.0f*mPositions[i-1]
	- 6.0f*mPositions[i-2]
	+ 3.0f*mPositions[i-3];
	noVec3 C = 3.0f*mPositions[i-1] - 3.0f*mPositions[i-3];

	noVec3 result = (C + u*(2.0f*B + 3.0f*u*A))/6.0f;

	return result;

}   // End of IvUniformBSpline::Velocity()


//-------------------------------------------------------------------------------
// @ IvUniformBSpline::Acceleration()
//-------------------------------------------------------------------------------
// Evaluate second derivative at parameter t
//-------------------------------------------------------------------------------
noVec3
	IvUniformBSpline::Acceleration( float t )
{
	// make sure data is valid
	assert( mCount >= 2 );
	if ( mCount < 2 )
		return vec3_zero;

	// handle boundary conditions
	if ( t <= mTimes[0] )
		t = 0.0f;
	else if ( t > mTimes[mCount-3] )
		t = mTimes[mCount-3];

	// find segment and parameter
	unsigned int i;
	for ( i = 0; i < mCount-1; ++i )
	{
		if ( t <= mTimes[i+1] )
		{
			break;
		}
	}
	float t0 = mTimes[i];
	float t1 = mTimes[i+1];
	float u = (t - t0)/(t1 - t0);

	// match segment index to standard B-spline terminology
	i += 3;

	// evaluate
	noVec3 A = mPositions[i]
	- 3.0f*mPositions[i-1]
	+ 3.0f*mPositions[i-2]
	- mPositions[i-3];
	noVec3 B = 3.0f*mPositions[i-1]
	- 6.0f*mPositions[i-2]
	+ 3.0f*mPositions[i-3];

	return 1.0f/3.0f*B + u*A;

}   // End of IvUniformBSpline::Acceleration()


//-------------------------------------------------------------------------------
// @ IvUniformBSpline::FindParameterByDistance()
//-------------------------------------------------------------------------------
// Find parameter s distance in arc length from Q(t1)
// Returns max float if can't find it
//
// This extends the approach in the text and uses a mixture of bisection and 
// Newton-Raphson to find the root.  The result is more stable than Newton-
// Raphson alone because a) we won't end up with a situation where we divide by 
// zero in the Newton-Raphson step and b) the end result converges faster.
//
// See Numerical Recipes or http://www.essentialmath.com/blog for more details.
//-------------------------------------------------------------------------------
float 
	IvUniformBSpline::FindParameterByDistance( float t1, float s )
{
	// initialize bisection endpoints
	float a = t1;
	float b = mTimes[mCount-3];

	// ensure that we remain within valid parameter space
	if ( s >= ArcLength(t1, b) )
		return b;
	if ( s <= 0.0f )
		return a;

	// make first guess
	float p = t1 + s*(mTimes[mCount-3]-mTimes[0])/mTotalLength;

	// iterate and look for zeros
	for ( uint32 i = 0; i < 32; ++i )
	{
		// compute function value and test against zero
		float func = ArcLength(t1, p) - s;
		if ( noMath::Abs(func) < 1.0e-03f )
		{
			return p;
		}

		// update bisection endpoints
		if ( func < 0.0f )
		{
			a = p;
		}
		else
		{
			b = p;
		}

		// get speed along curve
		float speed = Velocity(p).Length();

		// if result will lie outside [a,b] 
		if ( ((p-a)*speed - func)*((p-b)*speed - func) > -1.0e-3f )
		{
			// do bisection
			p = 0.5f*(a+b);
		}    
		else
		{
			// otherwise Newton-Raphson
			p -= func/speed;
		}
	}

	// done iterating, return failure case
	return FLT_MAX;

}   // End of IvUniformBSpline::FindParameterByDistance()


//-------------------------------------------------------------------------------
// @ IvUniformBSpline::ArcLength()
//-------------------------------------------------------------------------------
// Find length of curve between parameters t1 and t2
//-------------------------------------------------------------------------------
float 
	IvUniformBSpline::ArcLength( float t1, float t2 )
{
	if ( t2 <= t1 )
		return 0.0f;

	if ( t1 < mTimes[0] )
		t1 = mTimes[0];

	if ( t2 > mTimes[mCount-3] )
		t2 = mTimes[mCount-3];

	// find segment and parameter
	unsigned int seg1;
	for ( seg1 = 0; seg1 < mCount-1; ++seg1 )
	{
		if ( t1 <= mTimes[seg1+1] )
		{
			break;
		}
	}
	float u1 = (t1 - mTimes[seg1])/(mTimes[seg1+1] - mTimes[seg1]);

	// find segment and parameter
	unsigned int seg2;
	for ( seg2 = 0; seg2 < mCount-1; ++seg2 )
	{
		if ( t2 <= mTimes[seg2+1] )
		{
			break;
		}
	}
	float u2 = (t2 - mTimes[seg2])/(mTimes[seg2+1] - mTimes[seg2]);

	float result;
	// both parameters lie in one segment
	if ( seg1 == seg2 )
	{
		result = SegmentArcLength( seg1, u1, u2 );
	}
	// parameters cross segments
	else
	{
		result = SegmentArcLength( seg1, u1, 1.0f );
		for ( uint32 i = seg1+1; i < seg2; ++i )
			result += mLengths[i];
		result += SegmentArcLength( seg2, 0.0f, u2 );
	}

	return result;

}   // End of IvUniformBSpline::ArcLength()


//-------------------------------------------------------------------------------
// @ IvUniformBSpline::SegmentArcLength()
//-------------------------------------------------------------------------------
// Find length of curve segment between parameters u1 and u2
//-------------------------------------------------------------------------------
float 
	IvUniformBSpline::SegmentArcLength( uint32 i, float u1, float u2 )
{
	static const float x[] =
	{
		0.0000000000, 0.5384693101, -0.5384693101, 0.9061798459, -0.9061798459 
	};

	static const float c[] =
	{
		0.5688888889, 0.4786286705, 0.4786286705, 0.2369268850, 0.2369268850
	};

	assert(i >= 0 && i < mCount-3);

	if ( u2 <= u1 )
		return 0.0f;

	if ( u1 < 0.0f )
		u1 = 0.0f;

	if ( u2 > 1.0f )
		u2 = 1.0f;

	// reindex to use standard B-spline segment count
	i += 3;

	// use Gaussian quadrature
	float sum = 0.0f;
	// set up for computation of IvUniformBSpline derivative
	noVec3 A = mPositions[i]
	- 3.0f*mPositions[i-1]
	+ 3.0f*mPositions[i-2]
	- mPositions[i-3];
	noVec3 B = 3.0f*mPositions[i-1]
	- 6.0f*mPositions[i-2]
	+ 3.0f*mPositions[i-3];
	noVec3 C = 3.0f*mPositions[i-1] - 3.0f*mPositions[i-3];

	for ( uint32 j = 0; j < 5; ++j )
	{
		float u = 0.5f*((u2 - u1)*x[j] + u2 + u1);
		noVec3 derivative = (C + u*(2.0f*B + 3.0f*u*A))/6.0f;
		sum += c[j]*derivative.Length();
	}
	sum *= 0.5f*(u2-u1);

	return sum;

}   // End of IvUniformBSpline::SegmentArcLength()

IvBezier::IvBezier() :
mPositions( 0 ),
	mControls( 0 ),
	mTimes( 0 ),
	mLengths( 0 ),
	mTotalLength( 0.0f ),
	mCount( 0 ),
	mLength(1.0f)
{
}   // End of IvBezier::IvBezier()


//-------------------------------------------------------------------------------
// @ IvBezier::IvBezier()
//-------------------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------------------
IvBezier::~IvBezier()
{
	Clean();

}   // End of IvBezier::~IvBezier()


//-------------------------------------------------------------------------------
// @ operator<<()
//-------------------------------------------------------------------------------
// Text output for debugging
//-------------------------------------------------------------------------------
std::ostream& 
	operator<<(std::ostream& out, const IvBezier& source)
{
	out << source.mCount << eol;
	for (unsigned int i = 0; i < source.mCount; ++i )
	{
		out << source.mTimes[i] << ':' << source.mPositions[i].x << ", " <<
			source.mPositions[i].y << ", " << source.mPositions[i].z << eol;
		if ( i < source.mCount-1 )
			out <<source.mControls[2*i].x << ", " <<source.mControls[2*i].y << ". " 
			<< source.mControls[2*i].z << ',' << source.mControls[2*i+1].x << ", " <<
			source.mControls[2*i+1].y << ". " << source.mControls[2*i+1].z << eol;
	}

	return out;

}   // End of operator<<()


//-------------------------------------------------------------------------------
// @ IvBezier::Initialize()
//-------------------------------------------------------------------------------
// Set up sample points
//-------------------------------------------------------------------------------
bool
	IvBezier::Initialize( const noVec3* positions,
	const noVec3* controls,
	const float* times,
	unsigned int count )

{
	// make sure not already initialized
	if (mCount != 0)
		return false;

	// make sure data is valid
	if ( count < 2 || !positions || !times || !controls )
		return false;

	// set up arrays
	mPositions = new noVec3[count];
	mControls = new noVec3[2*(count-1)];
	mTimes = new float[count];
	mCount = count;

	// copy interpolant data
	unsigned int i;
	for ( i = 0; i < count; ++i )
	{
		mPositions[i] = positions[i];
		mTimes[i] = times[i];
	}

	// copy approximating control points
	for ( i = 0; i < 2*(count-1); ++i )
	{
		mControls[i] = controls[i];
	}

	// set up curve segment lengths
	mLengths = new float[count-1];
	mTotalLength = 0.0f;
	for ( i = 0; i < count-1; ++i )
	{
		mLengths[i] = SegmentArcLength(i, 0.0f, 1.0f);
		mTotalLength += mLengths[i];
	}

	return true;

}   // End of IvBezier::Initialize()


//-------------------------------------------------------------------------------
// @ IvBezier::Initialize()
//-------------------------------------------------------------------------------
// Set up sample points
//-------------------------------------------------------------------------------
bool
	IvBezier::Initialize( const noVec3* positions,
	const float* times,
	unsigned int count )

{
	// make sure not already initialized
	if (mCount != 0)
		return false;

	// make sure data is valid
	if ( count < 2 || !positions || !times )
		return false;

	// set up arrays
	mPositions = new noVec3[count];
	mControls = new noVec3[2*(count-1)];
	mTimes = new float[count];
	mCount = count;

	// copy interpolant data
	unsigned int i;
	for ( i = 0; i < count; ++i )
	{
		mPositions[i] = positions[i];
		mTimes[i] = times[i];
	}

	// create approximating control points
	for ( i = 0; i < count-1; ++i )
	{
		if ( i > 0 )
			mControls[2*i] = mPositions[i] + (mPositions[i+1]-mPositions[i-1])/3.0f;
		if ( i < count-2 )
			mControls[2*i+1] = mPositions[i+1] - (mPositions[i+2]-mPositions[i])/3.0f;
	}
	mControls[0] = mControls[1] - (mPositions[1] - mPositions[0])/3.0f;
	mControls[2*count-3] = 
		mControls[2*count-4] + (mPositions[count-1] - mPositions[count-2])/3.0f;

	// set up curve segment lengths
	mLengths = new float[count-1];
	mTotalLength = 0.0f;
	for ( i = 0; i < count-1; ++i )
	{
		mLengths[i] = SegmentArcLength(i, 0.0f, 1.0f);
		mTotalLength += mLengths[i];
	}

	return true;

}   // End of IvBezier::Initialize()


//-------------------------------------------------------------------------------
// @ IvBezier::Clean()
//-------------------------------------------------------------------------------
// Clean out data
//-------------------------------------------------------------------------------
void IvBezier::Clean()
{
	delete [] mPositions;
	delete [] mControls;
	delete [] mTimes;
	delete [] mLengths;
	mTotalLength = 0.0f;
	mCount = 0;

}   // End of IvBezier::Clean()


//-------------------------------------------------------------------------------
// @ IvBezier::Evaluate()
//-------------------------------------------------------------------------------
// Evaluate spline
//-------------------------------------------------------------------------------
noVec3
	IvBezier::Evaluate( float t )
{
	// make sure data is valid
	assert( mCount >= 2 );
	if ( mCount < 2 )
		return vec3_zero;

	// handle boundary conditions
	if ( t <= mTimes[0] )
		return mPositions[0];
	else if ( t >= mTimes[mCount-1] )
		return mPositions[mCount-1];

	// find segment and parameter
	unsigned int i;
	for ( i = 0; i < mCount-1; ++i )
	{
		if ( t < mTimes[i+1] )
		{
			break;
		}
	}
	float t0 = mTimes[i];
	float t1 = mTimes[i+1];
	float u = (t - t0)/(t1 - t0);

	// evaluate
	noVec3 A = mPositions[i+1]
	- 3.0f*mControls[2*i+1]
	+ 3.0f*mControls[2*i]
	- mPositions[i];
	noVec3 B = 3.0f*mControls[2*i+1]
	- 6.0f*mControls[2*i]
	+ 3.0f*mPositions[i];
	noVec3 C = 3.0f*mControls[2*i]
	- 3.0f*mPositions[i];

	return mPositions[i] + u*(C + u*(B + u*A));

}   // End of IvBezier::Evaluate()


//-------------------------------------------------------------------------------
// @ IvBezier::Velocity()
//-------------------------------------------------------------------------------
// Evaluate spline's derivative
//-------------------------------------------------------------------------------
noVec3
	IvBezier::Velocity( float t )
{
	// make sure data is valid
	assert( mCount >= 2 );
	if ( mCount < 2 )
		return vec3_zero;

	// handle boundary conditions
	if ( t <= mTimes[0] )
		return mPositions[0];
	else if ( t >= mTimes[mCount-1] )
		return mPositions[mCount-1];

	// find segment and parameter
	unsigned int i;
	for ( i = 0; i < mCount-1; ++i )
	{
		if ( t < mTimes[i+1] )
		{
			break;
		}
	}
	float t0 = mTimes[i];
	float t1 = mTimes[i+1];
	float u = (t - t0)/(t1 - t0);

	// evaluate
	noVec3 A = mPositions[i+1]
	- 3.0f*mControls[2*i+1]
	+ 3.0f*mControls[2*i]
	- mPositions[i];
	noVec3 B = 6.0f*mControls[2*i+1]
	- 12.0f*mControls[2*i]
	+ 6.0f*mPositions[i];
	noVec3 C = 3.0f*mControls[2*i]
	- 3.0f*mPositions[i];

	return C + u*(B + 3.0f*u*A);

}   // End of IvBezier::Velocity()


//-------------------------------------------------------------------------------
// @ IvBezier::Acceleration()
//-------------------------------------------------------------------------------
// Evaluate spline's second derivative
//-------------------------------------------------------------------------------
noVec3
	IvBezier::Acceleration( float t )
{
	// make sure data is valid
	assert( mCount >= 2 );
	if ( mCount < 2 )
		return vec3_zero;

	// handle boundary conditions
	if ( t <= mTimes[0] )
		return mPositions[0];
	else if ( t >= mTimes[mCount-1] )
		return mPositions[mCount-1];

	// find segment and parameter
	unsigned int i;
	for ( i = 0; i < mCount-1; ++i )
	{
		if ( t < mTimes[i+1] )
		{
			break;
		}
	}
	float t0 = mTimes[i];
	float t1 = mTimes[i+1];
	float u = (t - t0)/(t1 - t0);

	// evaluate
	noVec3 A = mPositions[i+1]
	- 3.0f*mControls[2*i+1]
	+ 3.0f*mControls[2*i]
	- mPositions[i];
	noVec3 B = 6.0f*mControls[2*i+1]
	- 12.0f*mControls[2*i]
	+ 6.0f*mPositions[i];

	return B + 6.0f*u*A;

}   // End of IvBezier::Acceleration()


//-------------------------------------------------------------------------------
// @ IvBezier::FindParameterByDistance()
//-------------------------------------------------------------------------------
// Find parameter s distance in arc length from Q(t1)
// Returns max float if can't find it
//
// This extends the approach in the text and uses a mixture of bisection and 
// Newton-Raphson to find the root.  The result is more stable than Newton-
// Raphson alone because a) we won't end up with a situation where we divide by 
// zero in the Newton-Raphson step and b) the end result converges faster.
//
// See Numerical Recipes or http://www.essentialmath.com/blog for more details.
//-------------------------------------------------------------------------------
float 
	IvBezier::FindParameterByDistance( float t1, float s )
{
	// initialize bisection endpoints
	float a = t1;
	float b = mTimes[mCount-1];

	// ensure that we remain within valid parameter space
	if ( s >= ArcLength(t1, b) )
		return b;
	if ( s <= 0.0f )
		return a;

	// make first guess
	float p = t1 + s*(mTimes[mCount-1]-mTimes[0])/mTotalLength;

	// iterate and look for zeros
	for ( uint32 i = 0; i < 32; ++i )
	{
		// compute function value and test against zero
		float func = ArcLength(t1, p) - s;
		if ( noMath::Abs(func) < 1.0e-03f )
		{
			return p;
		}

		// update bisection endpoints
		if ( func < 0.0f )
		{
			a = p;
		}
		else
		{
			b = p;
		}

		// get speed along curve
		float speed = Velocity(p).Length();

		// if result will lie outside [a,b] 
		if ( ((p-a)*speed - func)*((p-b)*speed - func) > -1.0e-3f )
		{
			// do bisection
			p = 0.5f*(a+b);
		}    
		else
		{
			// otherwise Newton-Raphson
			p -= func/speed;
		}
	}

	// done iterating, return failure case
	return FLT_MAX;

}   // End of IvBezier::FindParameterByDistance()


//-------------------------------------------------------------------------------
// @ IvBezier::ArcLength()
//-------------------------------------------------------------------------------
// Find length of curve between parameters t1 and t2
//-------------------------------------------------------------------------------
float 
	IvBezier::ArcLength( float t1, float t2 )
{
	if ( t2 <= t1 )
		return 0.0f;

	if ( t1 < mTimes[0] )
		t1 = mTimes[0];

	if ( t2 > mTimes[mCount-1] )
		t2 = mTimes[mCount-1];

	// find segment and parameter
	unsigned int seg1;
	for ( seg1 = 0; seg1 < mCount-1; ++seg1 )
	{
		if ( t1 < mTimes[seg1+1] )
		{
			break;
		}
	}
	float u1 = (t1 - mTimes[seg1])/(mTimes[seg1+1] - mTimes[seg1]);

	// find segment and parameter
	unsigned int seg2;
	for ( seg2 = 0; seg2 < mCount-1; ++seg2 )
	{
		if ( t2 <= mTimes[seg2+1] )
		{
			break;
		}
	}
	float u2 = (t2 - mTimes[seg2])/(mTimes[seg2+1] - mTimes[seg2]);

	float result;
	// both parameters lie in one segment
	if ( seg1 == seg2 )
	{
		result = SegmentArcLength( seg1, u1, u2 );
	}
	// parameters cross segments
	else
	{
		result = SegmentArcLength( seg1, u1, 1.0f );
		for ( uint32 i = seg1+1; i < seg2; ++i )
			result += mLengths[i];
		result += SegmentArcLength( seg2, 0.0f, u2 );
	}

	return result;

}   // End of IvBezier::ArcLength()


//-------------------------------------------------------------------------------
// @ IvBezier::SegmentArcLength()
//-------------------------------------------------------------------------------
// Find length of curve segment between parameters u1 and u2
//-------------------------------------------------------------------------------
float 
	IvBezier::SegmentArcLength( uint32 i, float u1, float u2 )
{
	assert(i >= 0 && i < mCount-1);

	if ( u2 <= u1 )
		return 0.0f;

	if ( u1 < 0.0f )
		u1 = 0.0f;

	if ( u2 > 1.0f )
		u2 = 1.0f;

	noVec3 P0 = mPositions[i];
	noVec3 P1 = mControls[2*i];
	noVec3 P2 = mControls[2*i+1];
	noVec3 P3 = mPositions[i+1];

	// get control points for subcurve from 0.0 to u2 (de Casteljau's method)
	float minus_u2 = (1.0f - u2);
	noVec3 L1 = minus_u2*P0 + u2*P1;
	noVec3 H = minus_u2*P1 + u2*P2;
	noVec3 L2 = minus_u2*L1 + u2*H;
	noVec3 L3 = minus_u2*L2 + u2*(minus_u2*H + u2*(minus_u2*P2 + u2*P3));

	// resubdivide to get control points for subcurve between u1 and u2
	float minus_u1 = (1.0f - u1);
	H = minus_u1*L1 + u1*L2;
	noVec3 R3 = L3;
	noVec3 R2 = minus_u1*L2 + u1*L3;
	noVec3 R1 = minus_u1*H + u1*R2;
	noVec3 R0 = minus_u1*(minus_u1*(minus_u1*P0 + u1*L1) + u1*H) + u1*R1;

	// get length through subdivision
	return SubdivideLength( R0, R1, R2, R3 );

}   // End of IvBezier::SegmentArcLength()


//-------------------------------------------------------------------------------
// @ IvBezier::SubdivideLength()
//-------------------------------------------------------------------------------
// Get length of Bezier curve using midpoint subdivision
//-------------------------------------------------------------------------------
float 
	IvBezier::SubdivideLength( const noVec3& P0, const noVec3& P1, 
	const noVec3& P2, const noVec3& P3 )
{
	// check to see if basically straight
	float Lmin = ( P0 - P3 ).Length();
	float Lmax = ( P0, P1 ).Length() + ( P1, P2 ).Length() + ( P2, P3 ).Length();
	float diff = Lmin - Lmax;

	if ( diff*diff < 1.0e-3f )
		return 0.5f*(Lmin + Lmax);

	// otherwise get control points for subdivision
	noVec3 L1 = (P0 + P1) * 0.5f;
	noVec3 H = (P1 + P2) * 0.5f;
	noVec3 L2 = (L1 + H) * 0.5f;
	noVec3 R2 = (P2 + P3) * 0.5f;
	noVec3 R1 = (H + R2) * 0.5f;
	noVec3 mid = (L2 + R1) * 0.5f;

	// subdivide
	return SubdivideLength( P0, L1, L2, mid ) + SubdivideLength( mid, R1, R2, P3 );

}   // End of IvBezier::SubdivideLength()

bool IvBezier::AddControlPoint( const noVec3& pt, float weight )
{
#if 0
	BezierControlPoint *ptr;
	int i=0;
	for (ptr = mPointList.Ptr(); i < mPointList.Num(); ptr++, ++i) {
		if (ptr->Point == pt && ptr->Weight == weight)
			return false;
	}
#endif

	BezierControlPoint p;
	p.Point = pt;
	p.Weight = weight;
	mPointList.Append(p);
	return true;
}

//Basic DeCasteljau bezier calculation. Accepts any amount of points, however it does
//not break the curve into sections. Doesnt account for weights. 
void IvBezier::CalcDeCasteljau( class CurveLines *Lines, int NumSamples ) {
	int numPoints = mPointList.Num();
	if (numPoints < 2) return;

	std::vector<noVec3>	controlP;
	controlP.resize(numPoints);
	BezierControlPoint* cp;
	int i;
		
	cp = mPointList.Ptr();
	noVec3 pos = cp->Point;
	int num = 0;
	for(i =0; i < mPointList.Num(); i++) {
		controlP[i] = cp[i].Point;			
		if (pos == cp[i].Point)
			++num;		
	}
	if (num != mPointList.Num())
	{	
		for (float t=0.f; t <=1.0f; t+= 1.0f/NumSamples) {
			i = numPoints;
			std::vector<noVec3> p;
			p.resize(numPoints);
			memcpy(&p[0], &controlP[0], sizeof(noVec3) * numPoints);

			//iterate through each set of line segments. creates a new line segment until there
			//is only one remaining point. this is the point we want on the curve.
			while (i != 1) {
				for( int k=0; k<i-1; k++) 
					p[k] = p[k] + t*(p[k+1] - p[k]);
				i--;
			}		
			Lines->AddPoint(p[0]);
		}
	}
	else 
	{
		Lines->AddPoint(pos);
	}
	
	//calculate the curve
	Lines->CalculateLine();
}

BezierControlPoint * IvBezier::GetControlPoint( int _Index ) {
	if (mPointList.Num() < _Index) 
		return NULL;
	return &mPointList[_Index];	
}

BezierControlPoint * IvBezier::GetCurrentControlPoint() {
	if (mPointList.Num() > 0)
		return &mPointList[mPointList.Num()-1];
	else return NULL;
}

void IvBezier::CalculateLength() {
	//calculate the length of the bezier by iterating through it 1000 times taking
	//line segments calculating their length then adding up the lengths

	BezierControlPoint* cp = mPointList.Ptr();
	//set the first point
	noVec3 lastPoint = cp->Point;
	float totalLen = 0.0f;
	for (float t=0.0f; t <= 1.0f; t+= 1.0f/1000.f) {
		noVec3 nextPoint = GetPoint(t);

		noVec3 diff = nextPoint - lastPoint;
		float len = diff.Length();
		totalLen += len;

		lastPoint = nextPoint;
	}
	mLength = totalLen;
}

float IvBezier::GetPointByDist( float d, float currU, noVec3* pos, noVec3* dir ) {
	//here _D denotes the distance allong the curve that is wanted from _U such that:
	//B(_U) = (1-_U)^3 * P0 + 3*_U*(1-_U)^2 * P1 + 3*_U^2*(1-_U) * P2 + _U^3 * P3
	//where P0-3 are the control points and B is the bezier curve

	//calculate the initial position
	noVec3 lastPos = GetPoint(currU);
	float u = currU;
	float stepSize = (0.01f * (d/noMath::Fabs(d))) / mLength;
	float tolerance = stepSize/2.0f;				 //the tolerance for the final distance al
	float totalLen = 0.0f;							 //length from from _CurrentU
	noVec3 diff = *dir;								 //the vector that describes the movement
													 //from one sample to the next
	bool closeEnough=false;
	//iterate through from _CurrentU until the distance value is close enough to
	//the intended value of distance and return this position
	while (closeEnough == false && d != 0.0f) {
		u += stepSize;

		//calculate the position at the new value of u
		noVec3 newPos = GetPoint(u);

		diff = newPos - lastPos;
		float len = diff.Length();

		//sum up the lengths
		totalLen += len;

		//set the current pos as the last pos
		lastPos = newPos;
		//if the total length is close enough to the intended point, then
		//accept this
		if(totalLen >= noMath::Fabs(d) || u >= 1.0f || u < 0.0f)
			closeEnough = true;
	}

	*dir = diff;
	dir->Normalize();
	*pos = lastPos;
	//return the current position allong the bezier
	return u;	
}

noVec3 IvBezier::GetPoint( float t ) {
	int numPoints = mPointList.Num();
	if (numPoints < 2) return vec3_zero;

	std::vector<noVec3>	controlP;
	controlP.resize(numPoints);
	BezierControlPoint* cp;
	int i;
	for(cp = mPointList.Ptr(), i =0; i < mPointList.Num(); i++) {
		controlP[i] = cp[i].Point;		
	}

	i = numPoints;
	std::vector<noVec3> p;
	p.resize(numPoints);
	for (int j=0; j < numPoints; j++) {		
		memcpy(&p[0], &controlP[0], sizeof(noVec3) * numPoints);
	}

		//iterate through each set of line segments. creates a new line segment until there
		//is only one remaining point. this is the point we want on the curve.
		while (i != 1) {
			for( int k=0; k<i; k++) 
				p[k] = p[k] + t*(p[k+1] - p[k]);
			i--;
		}

	noVec3 result = p[0];

	return result;
}


IvHermite::IvHermite() :
    mPositions( 0 ),
    mInTangents( 0 ),
    mOutTangents( 0 ),
    mTimes( 0 ),
    mLengths( 0 ),
    mTotalLength( 0.0f ),
    mCount( 0 )
{
}   // End of IvHermite::IvHermite()


//-------------------------------------------------------------------------------
// @ IvHermite::IvHermite()
//-------------------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------------------
IvHermite::~IvHermite()
{
    Clean();

}   // End of IvHermite::~IvHermite()


//-------------------------------------------------------------------------------
// @ operator<<()
//-------------------------------------------------------------------------------
// Text output for debugging
//-------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, const IvHermite& source)
{
    out << source.mCount << eol;
    for (unsigned int i = 0; i < source.mCount; ++i )
    {
        /*out << source.mTimes[i] << ':' << source.mPositions[i];
        if ( i < source.mCount-1 )
            out << ',' << source.mInTangents[i];
        if ( i > 0 )
            out << ',' << source.mOutTangents[i-1];
        out << eol;*/
    }

    return out;
    
}   // End of operator<<()


//-------------------------------------------------------------------------------
// @ IvHermite::Initialize()
//-------------------------------------------------------------------------------
// Set up sample points
//-------------------------------------------------------------------------------
bool
IvHermite::Initialize( const noVec3* positions,
                       const noVec3* inTangents,
                       const noVec3* outTangents,
                       const float* times,
                       unsigned int count )

{
    // make sure not already initialized
    if (mCount != 0)
        return false;

    // make sure data is valid
    if ( count < 2 || !positions || !times || !inTangents || !outTangents )
        return false;

    // set up arrays
    mPositions = new noVec3[count];
    mInTangents = new noVec3[count-1];
    mOutTangents = new noVec3[count-1];
    mTimes = new float[count];
    mCount = count;

    // copy data
    unsigned int i;
    for ( i = 0; i < count; ++i )
    {
        mPositions[i] = positions[i];
        if ( i < count-1 )
        {
            mInTangents[i] = inTangents[i];
            mOutTangents[i] = outTangents[i];
        }
        mTimes[i] = times[i];
    }

    // set up curve segment lengths
    mLengths = new float[count-1];
    mTotalLength = 0.0f;
    for ( i = 0; i < count-1; ++i )
    {
        mLengths[i] = SegmentArcLength(i, 0.0f, 1.0f);
        mTotalLength += mLengths[i];
    }

    return true;

}   // End of IvHermite::Initialize()


//-------------------------------------------------------------------------------
// @ IvHermite::InitializeClamped()
//-------------------------------------------------------------------------------
// Set up sample points for clamped spline
//-------------------------------------------------------------------------------
bool 
IvHermite::InitializeClamped( const noVec3* positions, 
                              const float* times,
                              unsigned int count,
                              const noVec3& inTangent,
                              const noVec3& outTangent )
{
    // make sure not already initialized
    if (mCount != 0)
        return false;

    // make sure data is valid
    if ( count < 3 || !positions || !times )
        return false;

    // build A
    unsigned int n = count;
    float* A = new float[n*n];
    memset(A, 0, sizeof(float)*n*n);

    A[0] = 1.0f;
    unsigned int i;
    for ( i = 1; i < n-1; ++i )
    {
        A[i + n*i - n] = 1.0f;
        A[i + n*i] = 4.0f;
        A[i + n*i + n] = 1.0f;
    }
    A[n*n-1] = 1.0f;
    
    // invert it
    // we'd might get better accuracy if we solve the linear system 3 times,
    // once each for x, y, and z, but this is more efficient
    if (!::InvertMatrix( A, n ))
    {
        delete [] A;
        return false;
    }

    // set up arrays
    mPositions = new noVec3[count];
    mInTangents = new noVec3[count-1];
    mOutTangents = new noVec3[count-1];
    mTimes = new float[count];
    mCount = count;

    // handle end conditions
    mPositions[0] = positions[0];
    mTimes[0] = times[0];
    mInTangents[0] = inTangent;
    mPositions[count-1] = positions[count-1];
    mTimes[count-1] = times[count-1];
    mOutTangents[count-2] = outTangent;

    // set up the middle
    for ( i = 1; i < count-1; ++i )
    {
        // copy position and time
        mPositions[i] = positions[i];
        mTimes[i] = times[i];

        // multiply b by inverse of A to get x
        mInTangents[i] = A[i]*inTangent + A[i + n*n-n]*outTangent;
        for ( uint32 j = 1; j < n-1; ++j )
        {
            noVec3 b_j = 3.0f*(positions[j+1]-positions[j-1]);
            mInTangents[i] += A[i + n*j]*b_j;
        }

        // out tangent is in tangent of next segment
        mOutTangents[i-1] = mInTangents[i];
    }

    // set up curve segment lengths
    mLengths = new float[count-1];
    mTotalLength = 0.0f;
    for ( i = 0; i < count-1; ++i )
    {
        mLengths[i] = SegmentArcLength(i, 0.0f, 1.0f);
        mTotalLength += mLengths[i];
    }

    delete [] A;

    return true;

}   // End of IvHermite::InitializeClamped()


//-------------------------------------------------------------------------------
// @ IvHermite::InitializeNatural()
//-------------------------------------------------------------------------------
// Set up sample points for natural spline
//-------------------------------------------------------------------------------
bool 
IvHermite::InitializeNatural( const noVec3* positions, 
                              const float* times,
                              unsigned int count )
{
    // make sure not already initialized
    if (mCount != 0)
        return false;

    // make sure data is valid
    if ( count < 3 || !positions || !times )
        return false;

    // build A
    unsigned int n = count;
    float* A = new float[n*n];
    memset(A, 0, sizeof(float)*n*n);

    A[0] = 2.0f;
    A[n] = 1.0f;
    unsigned int i;
    for ( i = 1; i < n-1; ++i )
    {
        A[i + n*i - n] = 1.0f;
        A[i + n*i] = 4.0f;
        A[i + n*i + n] = 1.0f;
    }
    A[n*(n-1)-1] = 1.0f;
    A[n*n-1] = 2.0f;
    
    // invert it
    // we'd might get better accuracy if we solve the linear system 3 times,
    // once each for x, y, and z, but this is more efficient
    if (!::InvertMatrix( A, n ))
    {
        delete [] A;
        return false;
    }

    // set up arrays
    mPositions = new noVec3[count];
    mInTangents = new noVec3[count-1];
    mOutTangents = new noVec3[count-1];
    mTimes = new float[count];
    mCount = count;

    // set up the tangents
    for ( i = 0; i < count; ++i )
    {
        // copy position and time
        mPositions[i] = positions[i];
        mTimes[i] = times[i];

        // multiply b by inverse of A to get tangents
        // compute count-1 incoming tangents
        if ( i < count-1 )
        {
            mInTangents[i] = 3.0f*A[i]*(positions[1]-positions[0])
                             + 3.0f*A[i + n*n-n]*(positions[n-1]-positions[n-2]);
            for ( uint32 j = 1; j < n-1; ++j )
            {
                noVec3 b_j = 3.0f*(positions[j+1]-positions[j-1]);
                mInTangents[i] += A[i + n*j]*b_j;
            }
            // out tangent is in tangent of next segment
            if (i > 0)
                mOutTangents[i-1] = mInTangents[i];
        }
        // compute final outgoing tangent
        else
        {
            mOutTangents[i-1] = 3.0f*A[i]*(positions[1]-positions[0])
                             + 3.0f*A[i + n*n-n]*(positions[n-1]-positions[n-2]);
            for ( uint32 j = 1; j < n-1; ++j )
            {
                noVec3 b_j = 3.0f*(positions[j+1]-positions[j-1]);
                mOutTangents[i-1] += A[i + n*j]*b_j;
            }
        }
    }

    // set up curve segment lengths
    mLengths = new float[count-1];
    mTotalLength = 0.0f;
    for ( i = 0; i < count-1; ++i )
    {
        mLengths[i] = SegmentArcLength(i, 0.0f, 1.0f);
        mTotalLength += mLengths[i];
    }

    delete [] A;

    return true;

}   // End of IvHermite::InitializeNatural()


//-------------------------------------------------------------------------------
// @ IvHermite::InitializeCyclic()
//-------------------------------------------------------------------------------
// Set up sample points for cyclic spline
//-------------------------------------------------------------------------------
bool 
IvHermite::InitializeCyclic( const noVec3* positions, 
                              const float* times,
                              unsigned int count )
{
    // make sure not already initialized
    if (mCount != 0)
        return false;

    // make sure data is valid
    if ( count < 4 || !positions || !times )
        return false;

    // build A
    unsigned int n = count-1;
    float* A = new float[n*n];
    memset(A, 0, sizeof(float)*n*n);

    A[0] = 4.0f;
    A[n] = 1.0f;
    A[n*n-n] = 1.0f;
    unsigned int i;
    for ( i = 1; i < n-1; ++i )
    {
        A[i + n*i - n] = 1.0f;
        A[i + n*i] = 4.0f;
        A[i + n*i + n] = 1.0f;
    }
    A[n*(n-2)-1] = 1.0f;
    A[n*(n-1)-1] = 1.0f;
    A[n*n-1] = 4.0f;
    
    // invert it
    // we'd might get better accuracy if we solve the linear system 3 times,
    // once each for x, y, and z, but this is more efficient
    if (!::InvertMatrix( A, n ))
    {
        delete [] A;
        return false;
    }

    // set up arrays
    mPositions = new noVec3[count];
    mInTangents = new noVec3[count-1];
    mOutTangents = new noVec3[count-1];
    mTimes = new float[count];
    mCount = count;

    noVec3 b0 = 3.0f*(positions[1]-positions[0] + positions[count-1]-positions[count-2]);

    // set up the tangents
    for ( i = 0; i < count; ++i )
    {
        // copy position and time
        mPositions[i] = positions[i];
        mTimes[i] = times[i];

        // multiply b by inverse of A to get tangents
        // compute count-1 incoming tangents
        if ( i < count-1 )
        {
            mInTangents[i] = A[i]*b0;
            for ( uint32 j = 1; j < n; ++j )
            {
                noVec3 b_j = 3.0f*(positions[j+1]-positions[j-1]);
                mInTangents[i] += A[i + n*j]*b_j;
            }
            // outgoing tangent is incoming tangent of next segment
            if (i > 0)
                mOutTangents[i-1] = mInTangents[i];
        }
    }
    mOutTangents[count-2] = mInTangents[0];

    // set up curve segment lengths
    mLengths = new float[count-1];
    mTotalLength = 0.0f;
    for ( i = 0; i < count-1; ++i )
    {
        mLengths[i] = SegmentArcLength(i, 0.0f, 1.0f);
        mTotalLength += mLengths[i];
    }

    delete [] A;

    return true;

}   // End of IvHermite::InitializeCyclic()


//-------------------------------------------------------------------------------
// @ IvHermite::InitializeAcyclic()
//-------------------------------------------------------------------------------
// Set up sample points for cyclic spline
//-------------------------------------------------------------------------------
bool 
IvHermite::InitializeAcyclic( const noVec3* positions, 
                              const float* times,
                              unsigned int count )
{
    // make sure not already initialized
    if (mCount != 0)
        return false;

    // make sure data is valid
    if ( count < 4 || !positions || !times )
        return false;

    // build A
    unsigned int n = count-1;
    float* A = new float[n*n];
    memset(A, 0, sizeof(float)*n*n);

    A[0] = 4.0f;
    A[n] = 1.0f;
    A[n*n-n] = -1.0f;
    unsigned int i;
    for ( i = 1; i < n-1; ++i )
    {
        A[i + n*i - n] = 1.0f;
        A[i + n*i] = 4.0f;
        A[i + n*i + n] = 1.0f;
    }
    A[n*(n-2)-1] = -1.0f;
    A[n*(n-1)-1] = 1.0f;
    A[n*n-1] = 4.0f;
    
    // invert it
    // we'd might get better accuracy if we solve the linear system 3 times,
    // once each for x, y, and z, but this is more efficient
    if (!::InvertMatrix( A, n ))
    {
        delete [] A;
        return false;
    }
	

    // set up arrays
    mPositions = new noVec3[count];
    mInTangents = new noVec3[count-1];
    mOutTangents = new noVec3[count-1];
    mTimes = new float[count];
    mCount = count;

    noVec3 b0 = 3.0f*(positions[1]-positions[0] - positions[count-1]+positions[count-2]);

    // set up the tangents
    for ( i = 0; i < count; ++i )
    {
        // copy position and time
        mPositions[i] = positions[i];
        mTimes[i] = times[i];

        // multiply b by inverse of A to get tangents
        // compute count-1 incoming tangents
        if ( i < count-1 )
        {
            mInTangents[i] = A[i]*b0;
            for ( uint32 j = 1; j < n; ++j )
            {
                noVec3 b_j = 3.0f*(positions[j+1]-positions[j-1]);
                mInTangents[i] += A[i + n*j]*b_j;
            }
            // outgoing tangent is incoming tangent of next segment
            if (i > 0)
                mOutTangents[i-1] = mInTangents[i];
        }
    }
    mOutTangents[count-2] = -mInTangents[0];

    // set up curve segment lengths
    mLengths = new float[count-1];
    mTotalLength = 0.0f;
    for ( i = 0; i < count-1; ++i )
    {
        mLengths[i] = SegmentArcLength(i, 0.0f, 1.0f);
        mTotalLength += mLengths[i];
    }

    delete [] A;

    return true;

}   // End of IvHermite::InitializeAcyclic()


//-------------------------------------------------------------------------------
// @ IvHermite::Clean()
//-------------------------------------------------------------------------------
// Clean out data
//-------------------------------------------------------------------------------
void IvHermite::Clean()
{
    delete [] mPositions;
    delete [] mInTangents;
    delete [] mOutTangents;
    delete [] mTimes;
    delete [] mLengths;
    mTotalLength = 0.0f;
    mCount = 0;

}   // End of IvHermite::Clean()


//-------------------------------------------------------------------------------
// @ IvHermite::Evaluate()
//-------------------------------------------------------------------------------
// Evaluate spline
//-------------------------------------------------------------------------------
noVec3
IvHermite::Evaluate( float t )
{
    // make sure data is valid
    assert( mCount >= 2 );
    if ( mCount < 2 )
        return vec3_zero;

    // handle boundary conditions
    if ( t <= mTimes[0] )
        return mPositions[0];
    else if ( t >= mTimes[mCount-1] )
        return mPositions[mCount-1];

    // find segment and parameter
    unsigned int i;
    for ( i = 0; i < mCount-1; ++i )
    {
        if ( t < mTimes[i+1] )
        {
            break;
        }
    }
    float t0 = mTimes[i];
    float t1 = mTimes[i+1];
    float u = (t - t0)/(t1 - t0);

    // evaluate
    noVec3 A = 2.0f*mPositions[i]
                - 2.0f*mPositions[i+1]
                + mInTangents[i]
                + mOutTangents[i];
    noVec3 B = -3.0f*mPositions[i]
                + 3.0f*mPositions[i+1]
                - 2.0f*mInTangents[i]
                - mOutTangents[i];
    
    return mPositions[i] + u*(mInTangents[i] + u*(B + u*A));

}   // End of IvHermite::Evaluate()


//-------------------------------------------------------------------------------
// @ IvHermite::Velocity()
//-------------------------------------------------------------------------------
// Evaluate derivative at parameter t
//-------------------------------------------------------------------------------
noVec3
IvHermite::Velocity( float t )
{
    // make sure data is valid
    assert( mCount >= 2 );
    if ( mCount < 2 )
        return vec3_zero;

    // handle boundary conditions
    if ( t <= mTimes[0] )
        return mInTangents[0];
    else if ( t >= mTimes[mCount-1] )
        return mOutTangents[mCount-2];

    // find segment and parameter
    unsigned int i;
    for ( i = 0; i < mCount-1; ++i )
    {
        if ( t < mTimes[i+1] )
        {
            break;
        }
    }
    float t0 = mTimes[i];
    float t1 = mTimes[i+1];
    float u = (t - t0)/(t1 - t0);

    // evaluate
    noVec3 A = 2.0f*mPositions[i]
                - 2.0f*mPositions[i+1]
                + mInTangents[i]
                + mOutTangents[i];
    noVec3 B = -3.0f*mPositions[i]
                + 3.0f*mPositions[i+1]
                - 2.0f*mInTangents[i]
                - mOutTangents[i];
    
    return mInTangents[i] + u*(2.0f*B + 3.0f*u*A);

}   // End of IvHermite::Velocity()


//-------------------------------------------------------------------------------
// @ IvHermite::Acceleration()
//-------------------------------------------------------------------------------
// Evaluate second derivative at parameter t
//-------------------------------------------------------------------------------
noVec3
IvHermite::Acceleration( float t )
{
    // make sure data is valid
    assert( mCount >= 2 );
    if ( mCount < 2 )
        return vec3_zero;

    // handle boundary conditions
    if ( t <= mTimes[0] )
        t = 0.0f;
    else if ( t > mTimes[mCount-1] )
        t = mTimes[mCount-1];

    // find segment and parameter
    unsigned int i;
    for ( i = 0; i < mCount-1; ++i )
    {
        if ( t <= mTimes[i+1] )
        {
            break;
        }
    }
    float t0 = mTimes[i];
    float t1 = mTimes[i+1];
    float u = (t - t0)/(t1 - t0);

    // evaluate
    noVec3 A = 2.0f*mPositions[i]
                - 2.0f*mPositions[i+1]
                + mInTangents[i]
                + mOutTangents[i];
    noVec3 B = -3.0f*mPositions[i]
                + 3.0f*mPositions[i+1]
                - 2.0f*mInTangents[i]
                - mOutTangents[i];
    
    return 2.0f*B + 6.0f*u*A;

}   // End of IvHermite::Acceleration()


//-------------------------------------------------------------------------------
// @ IvHermite::FindParameterByDistance()
//-------------------------------------------------------------------------------
// Find parameter s distance in arc length from Q(t1)
// Returns max float if can't find it
//
// This extends the approach in the text and uses a mixture of bisection and 
// Newton-Raphson to find the root.  The result is more stable than Newton-
// Raphson alone because a) we won't end up with a situation where we divide by 
// zero in the Newton-Raphson step and b) the end result converges faster.
//
// See Numerical Recipes or http://www.essentialmath.com/blog for more details.
//-------------------------------------------------------------------------------
float IvHermite::FindParameterByDistance( float t1, float s )
{
    // initialize bisection endpoints
    float a = t1;
    float b = mTimes[mCount-1];

    // ensure that we remain within valid parameter space
    if ( s >= ArcLength(t1, b) )
        return b;
    if ( s <= 0.0f )
        return a;

    // make first guess
    float p = t1 + s*(mTimes[mCount-1]-mTimes[0])/mTotalLength;

    // iterate and look for zeros
    for ( uint32 i = 0; i < 32; ++i )
    {
        // compute function value and test against zero
        float func = ArcLength(t1, p) - s;
        if ( noMath::Abs(func) < 1.0e-03f )
        {
            return p;
        }

         // update bisection endpoints
        if ( func < 0.0f )
        {
            a = p;
        }
        else
        {
            b = p;
        }

        // get speed along curve
        float speed = Velocity(p).Length();

        // if result will lie outside [a,b] 
        if ( ((p-a)*speed - func)*((p-b)*speed - func) > -1.0e-3f )
        {
            // do bisection
            p = 0.5f*(a+b);
        }    
        else
        {
            // otherwise Newton-Raphson
            p -= func/speed;
        }
    }

    // done iterating, return failure case
    return FLT_MAX;

}   // End of IvHermite::FindParameterByDistance()


//-------------------------------------------------------------------------------
// @ IvHermite::ArcLength()
//-------------------------------------------------------------------------------
// Find length of curve between parameters t1 and t2
//-------------------------------------------------------------------------------
float 
IvHermite::ArcLength( float t1, float t2 )
{
    if ( t2 <= t1 )
        return 0.0f;

    if ( t1 < mTimes[0] )
        t1 = mTimes[0];

    if ( t2 > mTimes[mCount-1] )
        t2 = mTimes[mCount-1];

    // find segment and parameter
    unsigned int seg1;
    for ( seg1 = 0; seg1 < mCount-1; ++seg1 )
    {
        if ( t1 < mTimes[seg1+1] )
        {
            break;
        }
    }
    float u1 = (t1 - mTimes[seg1])/(mTimes[seg1+1] - mTimes[seg1]);
    
    // find segment and parameter
    unsigned int seg2;
    for ( seg2 = 0; seg2 < mCount-1; ++seg2 )
    {
        if ( t2 <= mTimes[seg2+1] )
        {
            break;
        }
    }
    float u2 = (t2 - mTimes[seg2])/(mTimes[seg2+1] - mTimes[seg2]);
    
    float result;
    // both parameters lie in one segment
    if ( seg1 == seg2 )
    {
        result = SegmentArcLength( seg1, u1, u2 );
    }
    // parameters cross segments
    else
    {
        result = SegmentArcLength( seg1, u1, 1.0f );
        for ( uint32 i = seg1+1; i < seg2; ++i )
            result += mLengths[i];
        result += SegmentArcLength( seg2, 0.0f, u2 );
    }

    return result;

}   // End of IvHermite::ArcLength()


//-------------------------------------------------------------------------------
// @ IvHermite::SegmentArcLength()
//-------------------------------------------------------------------------------
// Find length of curve segment between parameters u1 and u2
//-------------------------------------------------------------------------------
float 
IvHermite::SegmentArcLength( uint32 i, float u1, float u2 )
{
    static const float x[] =
    {
        0.0000000000, 0.5384693101, -0.5384693101, 0.9061798459, -0.9061798459 
    };

    static const float c[] =
    {
        0.5688888889, 0.4786286705, 0.4786286705, 0.2369268850, 0.2369268850
    };

    assert(i >= 0 && i < mCount-1);

    if ( u2 <= u1 )
        return 0.0f;

    if ( u1 < 0.0f )
        u1 = 0.0f;

    if ( u2 > 1.0f )
        u2 = 1.0f;

    // use Gaussian quadrature
    float sum = 0.0f;
    // set up for computation of IvHermite derivative
    noVec3 A = 2.0f*mPositions[i]
                - 2.0f*mPositions[i+1]
                + mInTangents[i]
                + mOutTangents[i];
    noVec3 B = -3.0f*mPositions[i]
                + 3.0f*mPositions[i+1]
                - 2.0f*mInTangents[i]
                - mOutTangents[i];
    noVec3 C = mInTangents[i];
    
    for ( uint32 j = 0; j < 5; ++j )
    {
        float u = 0.5f*((u2 - u1)*x[j] + u2 + u1);
        noVec3 derivative = C + u*(2.0f*B + 3.0f*u*A);
        sum += c[j]*derivative.Length();
    }
    sum *= 0.5f*(u2-u1);
/*
    // sample length directly
    float result = 0.0f;
    noVec3 previous = mPositions[i] + u1*(mInTangents[i] + u1*(B + u1*A));
    for ( float t = u1 + 0.01f; t <= u2; t += 0.01f )
    {
        noVec3 current = mPositions[i] + t*(mInTangents[i] + t*(B + t*A));
        result += ::Distance(previous, current);
        previous = current;
    }
*/
    return sum;

}   // End of IvHermite::SegmentArcLength()