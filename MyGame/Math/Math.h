/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company. 

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).  

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
#ifndef __MATH_MATH_H__
#define __MATH_MATH_H__

#include <assert.h>
#include <vector>
#include <Core/Types.h>
//#include <Core/Templates.h>

#ifdef INFINITY
#undef INFINITY
#endif

#ifdef FLT_EPSILON
#undef FLT_EPSILON
#endif


template< class T > inline T Max( const T A, const T B )
{
	return (A>=B) ? A : B;
}
template< class T > inline T Min( const T A, const T B )
{
	return (A<=B) ? A : B;
}

template< class T, class U > T Lerp( const T& A, const T& B, const U& Alpha )
{
	return (T)(A + Alpha * (B-A));
}

template<class T> T BiLerp(const T& P00,const T& P10,const T& P01,const T& P11,float FracX,float FracY)
{
	return Lerp(
		Lerp(P00,P10,FracX),
		Lerp(P01,P11,FracX),
		FracY
		);
}

// P - end points
// T - tangent directions at end points
// Alpha - distance along spline
template< class T, class U > T Cubicinterp( const T& P0, const T& T0, const T& P1, const T& T1, const U& A )
{
	const float A2 = A  * A;
	const float A3 = A2 * A;

	return (T)(((2*A3)-(3*A2)+1) * P0) + ((A3-(2*A2)+A) * T0) + ((A3-A2) * T1) + (((-2*A3)+(3*A2)) * P1);
}

template< class T, class U > T CubicinterpDerivative( const T& P0, const T& T0, const T& P1, const T& T1, const U& A )
{
	T a = 6.f*P0 + 3.f*T0 + 3.f*T1 - 6.f*P1;
	T b = -6.f*P0 - 4.f*T0 - 2.f*T1 + 6.f*P1;
	T c = T0;

	const float A2 = A  * A;

	return (a * A2) + (b * A) + c;
}

template< class T, class U > T CubicinterpSecondDerivative( const T& P0, const T& T0, const T& P1, const T& T1, const U& A )
{
	T a = 12.f*P0 + 6.f*T0 + 6.f*T1 - 12.f*P1;
	T b = -6.f*P0 - 4.f*T0 - 2.f*T1 + 6.f*P1;

	return (a * A) + b;
}

class noMath {
public:
	static void				Init( void );
		
	static float			Fabs( float f );
	static float			InvSqrt( float x );
	static float			Sqrt( float x );


	static const float			PI;							// pi
	static const float			TWO_PI;						// pi * 2
	static const float			HALF_PI;					// pi / 2
	static const float			ONEFOURTH_PI;				// pi / 4
	static const float			E;							// e
	static const float			SQRT_TWO;					// sqrt( 2 )
	static const float			SQRT_THREE;					// sqrt( 3 )
	static const float			SQRT_1OVER2;				// sqrt( 1 / 2 )
	static const float			SQRT_1OVER3;				// sqrt( 1 / 3 )
	static const float			M_DEG2RAD;					// degrees to radians multiplier
	static const float			M_RAD2DEG;					// radians to degrees multiplier
	static const float			M_SEC2MS;					// seconds to milliseconds multiplier
	static const float			M_MS2SEC;					// milliseconds to seconds multiplier
	static const float			INFINITY;					// huge number which should be larger than any valid number used
	static const float			FLT_EPSILON;				// smallest positive number such that 1.0+FLT_EPSILON != 1.0

private:
	enum {
		LOOKUP_BITS				= 8,							
		EXP_POS					= 23,							
		EXP_BIAS				= 127,							
		LOOKUP_POS				= (EXP_POS-LOOKUP_BITS),
		SEED_POS				= (EXP_POS-8),
		SQRT_TABLE_SIZE			= (2<<LOOKUP_BITS),
		LOOKUP_MASK				= (SQRT_TABLE_SIZE-1)
	};

	union _flint {
		uint32					i;
		float					f;
	};

	static uint32				iSqrt[SQRT_TABLE_SIZE];
	static bool					initialized;
};

NO_INLINE float noMath::Fabs( float f )
{
	int temp = *reinterpret_cast<char *>( &f );
	temp &= 0x7ffffff;
	return *reinterpret_cast<float *>( &temp );
}

NO_INLINE float noMath::InvSqrt( float x ) {

	uint32 a = ((union _flint*)(&x))->i;
	union _flint seed;

	assert( initialized );

	double y = x * 0.5f;
	seed.i = (( ( (3*EXP_BIAS-1) - ( (a >> EXP_POS) & 0xFF) ) >> 1)<<EXP_POS) | iSqrt[(a >> (EXP_POS-LOOKUP_BITS)) & LOOKUP_MASK];
	double r = seed.f;
	r = r * ( 1.5f - r * r * y );
	r = r * ( 1.5f - r * r * y );
	return (float) r;
}

NO_INLINE float noMath::Sqrt( float x ) {
	return x * InvSqrt( x );
}

typedef float LOOKUPVALUE;

enum ERawDistributionType
{
	RDT_float_Constant,

	DT_Vector_Constant,
};

enum ERawDistributionOperation
{
	RDO_Uninitialized,
	RDO_None,
	RDO_Random,
	RDO_Extreme,
};

enum ERawDistributionLockFlags
{
	RDL_None,
	RDL_XY,
	RDL_XZ,
	RDL_YZ,
	RDL_XYZ,
};

#define DIST_GET_LOCKFLAG_0(Type)				(Type & 0x07)
#define DIST_GET_LOCKFLAG_1(Type)				(Type & 0x48)
#define DIST_GET_LOCKFLAG(InIndex, Type)		((InIndex == 0) ? DIST_GET_LOCKFLAG_0(Type) : DIST_GET_LOCKFLAG_1(Type))
#define DIST_SET_LOCKFLAG_0(Flag, Type)			(Type |= (Flag & 0x07))
#define DIST_SET_LOCKFLAG_1(Flag, Type)			(Type |= ((Flag & 0x07) << 3))
#define DIST_SET_LOCKFLAG(InIndex, Flag, Type)	((InIndex == 0) ? DIST_SET_LOCKFLAG_0(Flag, Type) : DIST_SET_LOCKFLAG_1(Flag, Type))

#define DIST_UNIFORMCURVE_FLAG					0x80
#define DIST_IS_UNIFORMCURVE(Type)				(Type & DIST_UNIFORMCURVE_FLAG)
#define DIST_SET_UNIFORMCURVE(Flag, Type)		((Flag != 0) ? (Type |= DIST_UNIFORMCURVE_FLAG) : (Type &= ~DIST_UNIFORMCURVE_FLAG))

//struct noRawDistribution
//{
//	/**
//	 * Calcuate the float or vector value at the given time 
//	 * @param Time The time to evaluate
//	 * @param Value An array of (1 or 3) floats to receive the values
//	 * @param NumCoords The number of floats in the Value array
//	 * @param Extreme For distributions that use one of the extremes, this is which extreme to use
//	 */
//	void GetValue(float Time, float* Value, int NumCoords, int Extreme=0);
//
//	// prebaked versions of these
//	void GetValue1(float Time, float* Value, int Extreme=0);
//	void GetValue3(float Time, float* Value, int Extreme=0);
//	inline void GetValue1None(float Time, float* InValue) const
//	{
//		float* Value = InValue;
//		const LOOKUPVALUE* Entry1;
//		const LOOKUPVALUE* Entry2;
//		float LerpAlpha = 0.0f;
//		GetEntry(Time, Entry1, Entry2, LerpAlpha);
//		const LOOKUPVALUE* NewEntry1 = Entry1;
//		const LOOKUPVALUE* NewEntry2 = Entry2;
//		Value[0] = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
//	}
//	inline void GetValue3None(float Time, float* InValue) const
//	{
//		float* Value = InValue;
//		const LOOKUPVALUE* Entry1;
//		const LOOKUPVALUE* Entry2;
//		float LerpAlpha = 0.0f;
//		GetEntry(Time, Entry1, Entry2, LerpAlpha);
//		const LOOKUPVALUE* NewEntry1 = Entry1;
//		const LOOKUPVALUE* NewEntry2 = Entry2;
//		float T0 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
//		float T1 = Lerp(NewEntry1[1], NewEntry2[1], LerpAlpha);
//		float T2 = Lerp(NewEntry1[2], NewEntry2[2], LerpAlpha);
//		Value[0] = T0;
//		Value[1] = T1;
//		Value[2] = T2;
//	}
//	void GetValue1Extreme(float Time, float* Value, int Extreme=0);
//	void GetValue3Extreme(float Time, float* Value, int Extreme=0);
//	void GetValue1Random(float Time, float* Value);
//	void GetValue3Random(float Time, float* Value);
//
//	NO_INLINE BOOL IsSimple() 
//	{
//		return Op == RDO_None;
//	}
//
//	/**
//	 * Return the UDistribution* variable if the given StructProperty
//	 * points to a FRawDistribution* struct
//	 * @param Property Some UStructProperyy
//	 * @param Data Memory that owns the property
//	 * @return The UDisitribution* object if this is a FRawDistribution* struct, 
//	 *         or NULL otherwise
//	 */
//	//static UObject* TryGetDistributionObjectFromRawDistributionProperty(UStructProperty* Property, BYTE* Data);
//
//protected:
//	// let our masters access us
//	friend class noDistributionfloat;
//	friend class noDistributionVector;
//
//	/**
//	 * Get the entry for Time, and possibly the one after it for interpolating (along with 
//	 * an alpha for interpolation)
//	 * 
//	 * @param Time The time we are looking to retrieve
//	 * @param Entry1 Out variable that is the first (or only) entry
//	 * @param Entry2 Out variable that is the second entry (for interpolating) or NULL or Entry1 is all that's needed
//	 * @param LerpAlpha Out variable that is the alpha for interpolating between Entry1 and Entry2 (when Entry2 is valid)
//	 */
//	inline void GetEntry(float Time, const LOOKUPVALUE*& Entry1, const LOOKUPVALUE*& Entry2, float& LerpAlpha) const
//	{
//		// make time relative to start time
//		Time -= LookupTableStartTime;
//		Time *= LookupTableTimeScale;
//		Time = Max(Time,0.0f); // looks branch-free
//
//		// get the entry before or at the given time
//		DWORD Index = sysTrunc(Time);
//		DWORD Limit = LookupTable.size() - LookupTableChunkSize;
//
//		DWORD EntryIndex1 = Index * LookupTableChunkSize + 2;
//		DWORD EntryIndex2 = EntryIndex1 + LookupTableChunkSize;
//		EntryIndex1 = Min<DWORD>(EntryIndex1, Limit); 
//		EntryIndex2 = Min<DWORD>(EntryIndex2, Limit); 
//
//		Entry1 = &LookupTable[EntryIndex1];
//		Entry2 = &LookupTable[EntryIndex2];
//		// calculate the alpha to lerp between entry1 and entry2 
//		LerpAlpha = Time - float(Index);
//	}
//
//	/** Type of distribution, NOT NEEDED */
//	BYTE Type;
//
//	/** How to process the data in the lookup table */
//	BYTE Op;
//
//	/** How many elements per entry (ie, RDO_Random needs two elements to rand between) IMPLIED BY OP!!!*/
//	BYTE LookupTableNumElements;
//
//	/** The size of one element (1 for floats, 3 for vectors, etc) multiplied by number of elements */
//	BYTE LookupTableChunkSize;
//
//	/** Lookup table of values */
//	std::vector<LOOKUPVALUE> LookupTable;
//
//	/** Time between values in the lookup table */
//	float LookupTableTimeScale;
//
//	/** Absolute time of the first value */
//	float LookupTableStartTime;
//};

#endif