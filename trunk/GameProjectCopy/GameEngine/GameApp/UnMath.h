#pragma once 


#define SMALL_NUMBER		(1.e-8)
#define KINDA_SMALL_NUMBER	(1.e-4)
#define BIG_NUMBER			(3.4e+38f)
#define EULERS_NUMBER       (2.71828182845904523536)

#define DELTA			(0.00001f)

const FLOAT	SRandTemp = 1.f;


//
// MSM: Converts to an integer with truncation towards zero.
//
inline INT appTrunc( FLOAT F )
{
	__asm cvttss2si eax,[F]
	// return value in eax.
}
inline FLOAT	appTruncFloat( FLOAT F )
{
	// Convert to an integer
	//	__asm cvttss2si eax,[F]
	//	__asm cvtsi2ss  eax, eax
	// return value in eax

	return (FLOAT)appTrunc(F);
}

inline FLOAT appCopySign( FLOAT A, FLOAT B ) { return _copysign(A,B); }
inline FLOAT appExp( FLOAT Value ) { return expf(Value); }
inline FLOAT appLoge( FLOAT Value ) {	return logf(Value); }
inline FLOAT appFmod( FLOAT Y, FLOAT X ) { return fmodf(Y,X); }
inline FLOAT appSin( FLOAT Value ) { return sinf(Value); }
inline FLOAT appAsin( FLOAT Value ) { return asinf( (Value<-1.f) ? -1.f : ((Value<1.f) ? Value : 1.f) ); }
inline FLOAT appCos( FLOAT Value ) { return cosf(Value); }
inline FLOAT appAcos( FLOAT Value ) { return acosf( (Value<-1.f) ? -1.f : ((Value<1.f) ? Value : 1.f) ); }
inline FLOAT appTan( FLOAT Value ) { return tanf(Value); }
inline FLOAT appAtan( FLOAT Value ) { return atanf(Value); }
inline FLOAT appAtan2( FLOAT Y, FLOAT X ) { return atan2f(Y,X); }
inline FLOAT appPow( FLOAT A, FLOAT B ) { return powf(A,B); }
inline FLOAT appFractional( FLOAT Value ) { return Value - appTruncFloat( Value ); }
inline INT appCeil( FLOAT Value ) { return appTrunc(ceilf(Value)); }
inline INT appRand() { return rand(); }
inline void appRandInit(INT Seed) { srand( Seed ); }
inline FLOAT appFrand() { return rand() / (FLOAT)RAND_MAX; }
inline void appSRandInit( INT Seed ) { GSRandSeed = Seed; }


inline FLOAT appSRand() 
{ 
	GSRandSeed = (GSRandSeed * 196314165) + 907633515; 
	//@todo fix type aliasing
	FLOAT Result;
	*(INT*)&Result = (*(INT*)&SRandTemp & 0xff800000) | (GSRandSeed & 0x007fffff);
	return appFractional(Result); 
} 

//
//  MSM: Round (to nearest) a floating point number to an integer.
//
inline INT appRound( FLOAT F )
{
	__asm cvtss2si eax,[F]
	// return value in eax.
}

//
// MSM: Converts to integer equal to or less than.
//
inline INT appFloor( FLOAT F )
{
	const DWORD mxcsr_floor = 0x00003f80;
	const DWORD mxcsr_default = 0x00001f80;

	__asm ldmxcsr [mxcsr_floor]		// Round toward -infinity.
	__asm cvtss2si eax,[F]
	__asm ldmxcsr [mxcsr_default]	// Round to nearest
	// return value in eax.
}

//
// MSM: Fast float inverse square root using SSE.
// Accurate to within 1 LSB.
//
FORCEINLINE FLOAT appInvSqrt( FLOAT F )
{
#if ENABLE_VECTORINTRINSICS
	static const __m128 fThree = _mm_set_ss( 3.0f );
	static const __m128 fOneHalf = _mm_set_ss( 0.5f );
	__m128 Y0, X0, Temp;
	FLOAT temp;

	Y0 = _mm_set_ss( F );
	X0 = _mm_rsqrt_ss( Y0 );	// 1/sqrt estimate (12 bits)

	// Newton-Raphson iteration (X1 = 0.5*X0*(3-(Y*X0)*X0))
	Temp = _mm_mul_ss( _mm_mul_ss(Y0, X0), X0 );	// (Y*X0)*X0
	Temp = _mm_sub_ss( fThree, Temp );				// (3-(Y*X0)*X0)
	Temp = _mm_mul_ss( X0, Temp );					// X0*(3-(Y*X0)*X0)
	Temp = _mm_mul_ss( fOneHalf, Temp );			// 0.5*X0*(3-(Y*X0)*X0)
	_mm_store_ss( &temp, Temp );

#else
	const FLOAT fThree = 3.0f;
	const FLOAT fOneHalf = 0.5f;
	FLOAT temp;

	__asm
	{
		movss	xmm1,[F]
		rsqrtss	xmm0,xmm1			// 1/sqrt estimate (12 bits)

			// Newton-Raphson iteration (X1 = 0.5*X0*(3-(Y*X0)*X0))
			movss	xmm3,[fThree]
		movss	xmm2,xmm0
			mulss	xmm0,xmm1			// Y*X0
			mulss	xmm0,xmm2			// Y*X0*X0
			mulss	xmm2,[fOneHalf]		// 0.5*X0
		subss	xmm3,xmm0			// 3-Y*X0*X0
			mulss	xmm3,xmm2			// 0.5*X0*(3-Y*X0*X0)
			movss	[temp],xmm3
	}
#endif
	return temp;
}

inline FLOAT appInvSqrtEst( FLOAT F )
{
	return appInvSqrt( F );
}

/**
 * Convenience type for referring to axis by name instead of number.
 */
enum EAxis
{
	AXIS_None	= 0,
	AXIS_X		= 1,
	AXIS_Y		= 2,
	AXIS_Z		= 4,
	AXIS_XY		= AXIS_X|AXIS_Y,
	AXIS_XZ		= AXIS_X|AXIS_Z,
	AXIS_YZ		= AXIS_Y|AXIS_Z,
	AXIS_XYZ	= AXIS_X|AXIS_Y|AXIS_Z,
};

/*-----------------------------------------------------------------------------
	Global functions.
-----------------------------------------------------------------------------*/
/**
 * Helper function for rand implementations. Returns a random number in [0..A)
 */
inline INT RandHelper(INT A)
{
	// RAND_MAX+1 give interval [0..A) with even distribution.
	return A>0 ? appTrunc(appRand()/(FLOAT)((UINT)RAND_MAX+1) * A) : 0;
}

/**
 * Snaps a value to the nearest grid multiple.
 */
inline FLOAT FSnap( FLOAT Location, FLOAT Grid )
{
	if( Grid==0.f )	return Location;
	else			return appFloor((Location + 0.5*Grid)/Grid)*Grid;
}

/**
 * Add to a word angle, constraining it within a min (not to cross)
 * and a max (not to cross).  Accounts for funkyness of word angles.
 * Assumes that angle is initially in the desired range.
 */
inline WORD FAddAngleConfined( INT Angle, INT Delta, INT MinThresh, INT MaxThresh )
{
	if( Delta < 0 )
	{
		if ( Delta<=-0x10000L || Delta<=-(INT)((WORD)(Angle-MinThresh)))
			return MinThresh;
	}
	else if( Delta > 0 )
	{
		if( Delta>=0x10000L || Delta>=(INT)((WORD)(MaxThresh-Angle)))
			return MaxThresh;
	}
	return (WORD)(Angle+Delta);
}

/**
 * Eliminates all fractional precision from an angle.
 */
INT ReduceAngle( INT Angle );


/**
 * Fast 32-bit float evaluations.
 * Warning: likely not portable, and useful on Pentium class processors only.
 */
inline UBOOL IsNegativeFloat(const FLOAT& F1)
{
	return ( (*(DWORD*)&F1) >= (DWORD)0x80000000 ); // Detects sign bit.
}

inline FLOAT RangeByteToFloat(BYTE A)
{
	if( A < 128 )
	{
		return (A - 128.f) / 128.f;
	}
	else
	{
		return (A - 128.f) / 127.f;
	}
}

/**
 * Returns value based on comparand. The main purpose of this function is to avoid
 * branching based on floating point comparison which can be avoided via compiler
 * intrinsics. This is the platform agnostic implementation so we need to branch.
 *
 * Please note that we don't define what happens in the case of NaNs as there might
 * be platform specific differences.
 *
 * @param	Comparand		Comparand the results are based on
 * @param	ValueGEZero		Return value if Comparand >= 0
 * @param	ValueLTZero		Return value if Comparand > 0
 *
 * @return	ValueGEZero if Comparand >= 0, ValueLTZero otherwise
 */
FORCEINLINE DOUBLE FloatSelect( DOUBLE Comparand, DOUBLE ValueGEZero, DOUBLE ValueLTZero )
{
	return Comparand >= 0.f ? ValueGEZero : ValueLTZero;
}


inline BYTE FloatToRangeByte(FLOAT A)
{
	return appRound( 128.f + FloatSelect( A, Min( 127 * A, 127.f ), Max( 128 * A, -128.f ) ) );
}


/** Returns TRUE if A and B are equal within some epsilon */
inline UBOOL AreFloatsEqual( const FLOAT A, const FLOAT B, const FLOAT Tolerance = KINDA_SMALL_NUMBER )
{
	return ( Abs( A - B ) < Tolerance );
}

/** A logical exclusive or function. */
inline UBOOL XOR(UBOOL A, UBOOL B)
{
	return (A && !B) || (!A && B);
}

inline DWORD FLOAT_TO_DWORD(FLOAT Float)
{
	return *reinterpret_cast<DWORD*>(&Float);
}

#include "UnColor.h"

/*-----------------------------------------------------------------------------
	FIntPoint.
-----------------------------------------------------------------------------*/

struct FIntPoint
{
	INT X, Y;
	FIntPoint()
	{}

	FIntPoint( const FVector2D& xy)
	{
		X = appRound(xy.x);
		Y = appRound(xy.y);
	}

	FIntPoint( INT InX, INT InY )
		:	X( InX )
		,	Y( InY )
	{}
	explicit FORCEINLINE FIntPoint(EEventParm)
		: X(0), Y(0)
	{}
	static FIntPoint ZeroValue()
	{
		return FIntPoint(0,0);
	}
	static FIntPoint NoneValue()
	{
		return FIntPoint(INDEX_NONE,INDEX_NONE);
	}
	const INT& operator()( INT i ) const
	{
		return (&X)[i];
	}
	INT& operator()( INT i )
	{
		return (&X)[i];
	}
	static INT Num()
	{
		return 2;
	}
	UBOOL operator==( const FIntPoint& Other ) const
	{
		return X==Other.X && Y==Other.Y;
	}
	UBOOL operator!=( const FIntPoint& Other ) const
	{
		return X!=Other.X || Y!=Other.Y;
	}
	FIntPoint& operator*=( INT Scale )
	{
		X *= Scale;
		Y *= Scale;
		return *this;
	}
	FIntPoint& operator+=( const FIntPoint& Other )
	{
		X += Other.X;
		Y += Other.Y;
		return *this;
	}
	FIntPoint& operator-=( const FIntPoint& Other )
	{
		X -= Other.X;
		Y -= Other.Y;
		return *this;
	}
	FIntPoint operator*( INT Scale ) const
	{
		return FIntPoint(*this) *= Scale;
	}
	FIntPoint operator+( const FIntPoint& Other ) const
	{
		return FIntPoint(*this) += Other;
	}
	FIntPoint operator-( const FIntPoint& Other ) const
	{
		return FIntPoint(*this) -= Other;
	}
	INT Size() const
	{
		return INT( sqrt( FLOAT(X*X + Y*Y) ) );
	}
};

/*-----------------------------------------------------------------------------
	FIntRect.
-----------------------------------------------------------------------------*/

struct FIntRect
{
	FIntPoint Min, Max;
	FIntRect()
	{}
	FIntRect( INT X0, INT Y0, INT X1, INT Y1 )
		:	Min( X0, Y0 )
		,	Max( X1, Y1 )
	{}
	FIntRect( FIntPoint InMin, FIntPoint InMax )
		:	Min( InMin )
		,	Max( InMax )
	{}
	explicit FIntRect(EEventParm)
		: Min(EC_EventParm), Max(EC_EventParm)
	{}
	const FIntPoint& operator()( INT i ) const
	{
		return (&Min)[i];
	}
	FIntPoint& operator()( INT i )
	{
		return (&Min)[i];
	}
	static INT Num()
	{
		return 2;
	}
	UBOOL operator==( const FIntRect& Other ) const
	{
		return Min==Other.Min && Max==Other.Max;
	}
	UBOOL operator!=( const FIntRect& Other ) const
	{
		return Min!=Other.Min || Max!=Other.Max;
	}
	FIntRect Right( INT InWidth ) const
	{
		return FIntRect( ::Max(Min.X,Max.X-InWidth), Min.Y, Max.X, Max.Y );
	}
	FIntRect Bottom( INT InHeight ) const
	{
		return FIntRect( Min.X, ::Max(Min.Y,Max.Y-InHeight), Max.X, Max.Y );
	}
	FIntPoint Size() const
	{
		return FIntPoint( Max.X-Min.X, Max.Y-Min.Y );
	}
	INT Width() const
	{
		return Max.X-Min.X;
	}
	INT Height() const
	{
		return Max.Y-Min.Y;
	}
	FIntRect& operator*=( INT Scale )
	{
		Min *= Scale;
		Max *= Scale;
		return *this;
	}
	FIntRect& operator+=( const FIntPoint& P )
	{
		Min += P;
		Max += P;
		return *this;
	}
	FIntRect& operator-=( const FIntPoint& P )
	{
		Min -= P;
		Max -= P;
		return *this;
	}
	FIntRect operator*( INT Scale ) const
	{
		return FIntRect( Min*Scale, Max*Scale );
	}
	FIntRect operator+( const FIntPoint& P ) const
	{
		return FIntRect( Min+P, Max+P );
	}
	FIntRect operator-( const FIntPoint& P ) const
	{
		return FIntRect( Min-P, Max-P );
	}
	FIntRect operator+( const FIntRect& R ) const
	{
		return FIntRect( Min+R.Min, Max+R.Max );
	}
	FIntRect operator-( const FIntRect& R ) const
	{
		return FIntRect( Min-R.Min, Max-R.Max );
	}
	FIntRect Inner( FIntPoint P ) const
	{
		return FIntRect( Min+P, Max-P );
	}
	UBOOL Contains( FIntPoint P ) const
	{
		return P.X>=Min.X && P.X<Max.X && P.Y>=Min.Y && P.Y<Max.Y;
	}
	INT Area() const
	{
		return (Max.X - Min.X) * (Max.Y - Min.Y);
	}
	void GetCenterAndExtents(FIntPoint& Center, FIntPoint& Extent) const
	{
		Extent.X = (Max.X - Min.X) / 2;
		Extent.Y = (Max.Y - Min.Y) / 2;

		Center.X = Min.X + Extent.X;
		Center.Y = Min.Y + Extent.Y;
	}
	void Clip( const FIntRect& R )
	{
		Min.X = ::Max<INT>(Min.X, R.Min.X);
		Min.Y = ::Max<INT>(Min.Y, R.Min.Y);
		Max.X = ::Min<INT>(Max.X, R.Max.X);
		Max.Y = ::Min<INT>(Max.Y, R.Max.Y);

		// Adjust to zero area if the rects don't overlap.
		Max.X = ::Max<INT>(Min.X, Max.X);
		Max.Y = ::Max<INT>(Min.Y, Max.Y);
	}
};

/*-----------------------------------------------------------------------------
	FPlane.
	Stores the coeffecients as Ax+By+Cz=D.
	Note that this is different than many other Plane classes that use Ax+By+Cz+D=0.
-----------------------------------------------------------------------------*/

MS_ALIGN(16) class FPlane : public FVector
{
public:
	// Variables.
	FLOAT W;

	// Constructors.
	FORCEINLINE FPlane()
	{}
	FORCEINLINE FPlane( const FPlane& P )
	:	FVector(P)
	,	W(P.W)
	{}
	FORCEINLINE FPlane( const noVec4& V )
	:	FVector(V.x, V.y, V.z)
	,	W(V.w)
	{}
	FORCEINLINE FPlane( FLOAT InX, FLOAT InY, FLOAT InZ, FLOAT InW )
	:	FVector(InX,InY,InZ)
	,	W(InW)
	{}
	FORCEINLINE FPlane( FVector InNormal, FLOAT InW )
	:	FVector(InNormal), W(InW)
	{}
	FORCEINLINE FPlane( FVector InBase, const FVector &InNormal )
	:	FVector(InNormal)
	,	W(InBase * InNormal)
	{}
	FPlane( FVector A, FVector B, FVector C )
	:	FVector( ((B-A).Cross(C-A)).SafeNormal() )
	,	W( A * ((B-A).Cross(C-A)).SafeNormal() )
	{}
	explicit FORCEINLINE FPlane(EEventParm)
	: FVector(EC_EventParm), W(0.f)
	{}

	// Functions.
	FORCEINLINE FLOAT PlaneDot( const FVector &P ) const
	{
		return x*P.x + y*P.y + z*P.z - W;
	}
	FPlane Flip() const
	{
		return FPlane(-x,-y,-z,-W);
	}
	FPlane TransformPlaneByOrtho( const FMatrix& M ) const;
	FPlane TransformBy( const FMatrix& M ) const;
	FPlane TransformByUsingAdjointT( const FMatrix& M, FLOAT DetM, const FMatrix& TA ) const;
	UBOOL operator==( const FPlane& V ) const
	{
		return x==V.x && y==V.y && z==V.z && W==V.W;
	}
	UBOOL operator!=( const FPlane& V ) const
	{
		return x!=V.x || y!=V.y || z!=V.z || W!=V.W;
	}

	// Error-tolerant comparison.
	UBOOL Equals(const FPlane& V, FLOAT Tolerance=KINDA_SMALL_NUMBER) const
	{
		return noMath::Fabs(x-V.x) < Tolerance && 
			noMath::Fabs(y-V.y) < Tolerance && noMath::Fabs(z-V.z) < Tolerance && Abs(W-V.W) < Tolerance;
	}

	FORCEINLINE FLOAT operator|( const FPlane& V ) const
	{
		return x*V.x + y*V.y + z*V.z + W*V.W;
	}
	FPlane operator+( const FPlane& V ) const
	{
		return FPlane( x + V.x, y + V.y, z + V.z, W + V.W );
	}
	FPlane operator-( const FPlane& V ) const
	{
		return FPlane( x - V.x, y - V.y, z - V.z, W - V.W );
	}
	FPlane operator/( FLOAT Scale ) const
	{
		const FLOAT RScale = 1.f/Scale;
		return FPlane( x * RScale, y * RScale, z * RScale, W * RScale );
	}
	FPlane operator*( FLOAT Scale ) const
	{
		return FPlane( x * Scale, y * Scale, z * Scale, W * Scale );
	}
	FPlane operator*( const FPlane& V )
	{
		return FPlane ( x*V.x,y*V.y,z*V.z,W*V.W );
	}
	FPlane operator+=( const FPlane& V )
	{
		x += V.x; y += V.y; z += V.z; W += V.W;
		return *this;
	}
	FPlane operator-=( const FPlane& V )
	{
		x -= V.x; y -= V.y; z -= V.z; W -= V.W;
		return *this;
	}
	FPlane operator*=( FLOAT Scale )
	{
		x *= Scale; y *= Scale; z *= Scale; W *= Scale;
		return *this;
	}
	FPlane operator*=( const FPlane& V )
	{
		x *= V.x; y *= V.y; z *= V.z; W *= V.W;
		return *this;
	}
	FPlane operator/=( FLOAT V )
	{
		const FLOAT RV = 1.f/V;
		x *= RV; y *= RV; z *= RV; W *= RV;
		return *this;
	}

	noVec4	ToVec4() const 
	{
		return noVec4(x, y, z, W);
	}
				
	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FPlane &P )
	{
		return Ar << P.x << P.y << P.z << P.W;
	}
}  GCC_ALIGN(16);


class FPerspectiveMatrix : public FMatrix
{
public:

	FPerspectiveMatrix(float FOVX, float FOVY, float MultFOVX, float MultFOVY, float MinZ, float MaxZ) :
	  FMatrix(
		  FPlane(MultFOVX / appTan(FOVX),		0.0f,							0.0f,							0.0f).ToVec4(),
		  FPlane(0.0f,						MultFOVY / appTan(FOVY),		0.0f,							0.0f).ToVec4(),
		  FPlane(0.0f,						0.0f,							MaxZ / (MaxZ - MinZ),			1.0f).ToVec4(),
		  FPlane(0.0f,						0.0f,							-MinZ * (MaxZ / (MaxZ - MinZ)),	0.0f).ToVec4())
	  {
	  }

	  FPerspectiveMatrix(float FOV, float Width, float Height, float MinZ, float MaxZ) :
	  FMatrix(
		  FPlane(1.0f / appTan(FOV),	0.0f,							0.0f,							0.0f).ToVec4(),
		  FPlane(0.0f,				Width / appTan(FOV) / Height,	0.0f,							0.0f).ToVec4(),
		  FPlane(0.0f,				0.0f,							MaxZ / (MaxZ - MinZ),			1.0f).ToVec4(),
		  FPlane(0.0f,				0.0f,							-MinZ * (MaxZ / (MaxZ - MinZ)),	0.0f).ToVec4())
	  {
	  }

#define Z_PRECISION	0.001f

	  FPerspectiveMatrix(float FOV, float Width, float Height, float MinZ) :
	  FMatrix(
		  FPlane(1.0f / appTan(FOV),	0.0f,							0.0f,							0.0f).ToVec4(),
		  FPlane(0.0f,				Width / appTan(FOV) / Height,	0.0f,							0.0f).ToVec4(),
		  FPlane(0.0f,				0.0f,							(1.0f - Z_PRECISION),			1.0f).ToVec4(),
		  FPlane(0.0f,				0.0f,							-MinZ * (1.0f - Z_PRECISION),	0.0f).ToVec4())
	  {
	  }
};

class FOrthoMatrix : public FMatrix
{
public:

	FOrthoMatrix(float Width,float Height,float ZScale,float ZOffset) :
	  FMatrix(
		  FPlane(1.0f / Width,	0.0f,			0.0f,				0.0f).ToVec4(),
		  FPlane(0.0f,			1.0f / Height,	0.0f,				0.0f).ToVec4(),
		  FPlane(0.0f,			0.0f,			ZScale,				0.0f).ToVec4(),
		  FPlane(0.0f,			0.0f,			ZOffset * ZScale,	1.0f).ToVec4())
	  {
	  }
};

class FTranslationMatrix : public FMatrix
{
public:

	FTranslationMatrix(const FVector& Delta) :
	  FMatrix(
		  FPlane(1.0f,	0.0f,	0.0f,	0.0f).ToVec4(),
		  FPlane(0.0f,	1.0f,	0.0f,	0.0f).ToVec4(),
		  FPlane(0.0f,	0.0f,	1.0f,	0.0f).ToVec4(),
		  FPlane(Delta.x,	Delta.y,Delta.z,1.0f).ToVec4())
	  {
	  }
};

/*-----------------------------------------------------------------------------
	FDistribution functions
-----------------------------------------------------------------------------*/
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

struct noRawDistribution
{
	/**
	 * Calcuate the float or vector value at the given time 
	 * @param Time The time to evaluate
	 * @param Value An array of (1 or 3) floats to receive the values
	 * @param NumCoords The number of floats in the Value array
	 * @param Extreme For distributions that use one of the extremes, this is which extreme to use
	 */
	void GetValue(float Time, float* Value, int NumCoords, int Extreme=0);

	// prebaked versions of these
	void GetValue1(float Time, float* Value, int Extreme=0);
	void GetValue3(float Time, float* Value, int Extreme=0);
	inline void GetValue1None(float Time, float* InValue) const
	{
		float* Value = InValue;
		const LOOKUPVALUE* Entry1;
		const LOOKUPVALUE* Entry2;
		float LerpAlpha = 0.0f;
		GetEntry(Time, Entry1, Entry2, LerpAlpha);
		const LOOKUPVALUE* NewEntry1 = Entry1;
		const LOOKUPVALUE* NewEntry2 = Entry2;
		Value[0] = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
	}
	inline void GetValue3None(float Time, float* InValue) const
	{
		float* Value = InValue;
		const LOOKUPVALUE* Entry1;
		const LOOKUPVALUE* Entry2;
		float LerpAlpha = 0.0f;
		GetEntry(Time, Entry1, Entry2, LerpAlpha);
		const LOOKUPVALUE* NewEntry1 = Entry1;
		const LOOKUPVALUE* NewEntry2 = Entry2;
		float T0 = Lerp(NewEntry1[0], NewEntry2[0], LerpAlpha);
		float T1 = Lerp(NewEntry1[1], NewEntry2[1], LerpAlpha);
		float T2 = Lerp(NewEntry1[2], NewEntry2[2], LerpAlpha);
		Value[0] = T0;
		Value[1] = T1;
		Value[2] = T2;
	}
	void GetValue1Extreme(float Time, float* Value, int Extreme=0);
	void GetValue3Extreme(float Time, float* Value, int Extreme=0);
	void GetValue1Random(float Time, float* Value);
	void GetValue3Random(float Time, float* Value);

	__forceinline BOOL IsSimple() 
	{
		return Op == RDO_None;
	}

	/**
	 * Return the UDistribution* variable if the given StructProperty
	 * points to a FRawDistribution* struct
	 * @param Property Some UStructProperyy
	 * @param Data Memory that owns the property
	 * @return The UDisitribution* object if this is a FRawDistribution* struct, 
	 *         or NULL otherwise
	 */
	//static UObject* TryGetDistributionObjectFromRawDistributionProperty(UStructProperty* Property, BYTE* Data);

protected:
	// let our masters access us
	friend class noDistributionfloat;
	friend class noDistributionVector;

	/**
	 * Get the entry for Time, and possibly the one after it for interpolating (along with 
	 * an alpha for interpolation)
	 * 
	 * @param Time The time we are looking to retrieve
	 * @param Entry1 Out variable that is the first (or only) entry
	 * @param Entry2 Out variable that is the second entry (for interpolating) or NULL or Entry1 is all that's needed
	 * @param LerpAlpha Out variable that is the alpha for interpolating between Entry1 and Entry2 (when Entry2 is valid)
	 */
	inline void GetEntry(FLOAT Time, const LOOKUPVALUE*& Entry1, const LOOKUPVALUE*& Entry2, float& LerpAlpha) const
	{
		// make time relative to start time
		Time -= LookupTableStartTime;
		Time *= LookupTableTimeScale;
		Time = Max(Time,0.0f); // looks branch-free

		// get the entry before or at the given time
		DWORD Index = appTrunc(Time);
		DWORD Limit = LookupTable.Num() - LookupTableChunkSize;

		DWORD EntryIndex1 = Index * LookupTableChunkSize + 2;
		DWORD EntryIndex2 = EntryIndex1 + LookupTableChunkSize;
		EntryIndex1 = Min<DWORD>(EntryIndex1, Limit); 
		EntryIndex2 = Min<DWORD>(EntryIndex2, Limit); 

		Entry1 = &LookupTable(EntryIndex1);
		Entry2 = &LookupTable(EntryIndex2);
		// calculate the alpha to lerp between entry1 and entry2 
		LerpAlpha = Time - float(Index);
	}

	/** Type of distribution, NOT NEEDED */
	BYTE Type;

	/** How to process the data in the lookup table */
	BYTE Op;

	/** How many elements per entry (ie, RDO_Random needs two elements to rand between) IMPLIED BY OP!!!*/
	BYTE LookupTableNumElements;

	/** The size of one element (1 for floats, 3 for vectors, etc) multiplied by number of elements */
	BYTE LookupTableChunkSize;

	/** Lookup table of values */
	TArrayNoInit<LOOKUPVALUE> LookupTable;

	/** Time between values in the lookup table */
	float LookupTableTimeScale;

	/** Absolute time of the first value */
	float LookupTableStartTime;
};


//////////////////////////////////////////////////////////////////////////

enum EInterpCurveMode
{
	/** A straight line between two keypoint values. */
	CIM_Linear,

	/** A cubic-hermite curve between two keypoints, using Arrive/Leave tangents. These tangents will be automatically
	    updated when points are moved, etc.  Tangents are unclamped and will plateau at curve start and end points. */
	CIM_CurveAuto,

	/** The out value is held constant until the next key, then will jump to that value. */
	CIM_Constant,

	/** A smooth curve just like CIM_Curve, but tangents are not automatically updated so you can have manual control over them (eg. in Curve Editor). */
	CIM_CurveUser,

	/** A curve like CIM_Curve, but the arrive and leave tangents are not forced to be the same, so you can create a 'corner' at this key. */
	CIM_CurveBreak,

	/** A cubic-hermite curve between two keypoints, using Arrive/Leave tangents. These tangents will be automatically
	    updated when points are moved, etc.  Tangents are clamped and will plateau at curve start and end points. */
	CIM_CurveAutoClamped,

	/** Invalid or unknown curve type. */
	CIM_Unknown
};

/*-----------------------------------------------------------------------------
	FCurveEdInterface
-----------------------------------------------------------------------------*/


/** Interface that allows the CurveEditor to edit this type of object. */
class FCurveEdInterface
{
public:
	/** Get number of keyframes in curve. */
	virtual INT		GetNumKeys() { return 0; }

	/** Get number of 'sub curves' in this Curve. For example, a vector curve will have 3 sub-curves, for X, Y and Z. */
	virtual INT		GetNumSubCurves() { return 0; }

	/** Get the input value for the Key with the specified index. KeyIndex must be within range ie >=0 and < NumKeys. */
	virtual FLOAT	GetKeyIn(INT KeyIndex) { return 0.f; }

	/** 
	 *	Get the output value for the key with the specified index on the specified sub-curve. 
	 *	SubIndex must be within range ie >=0 and < NumSubCurves.
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual FLOAT	GetKeyOut(INT SubIndex, INT KeyIndex) { return 0.f; }

	/** Allows the Curve to specify a color for each keypoint. */
	virtual FColor	GetKeyColor(INT SubIndex, INT KeyIndex, const FColor& CurveColor) { return CurveColor; }

	/** Evaluate a subcurve at an arbitary point. Outside the keyframe range, curves are assumed to continue their end values. */
	virtual FLOAT	EvalSub(INT SubIndex, FLOAT InVal) { return 0.f; }

	/** 
	 *	Get the interpolation mode of the specified keyframe. This can be CIM_Constant, CIM_Linear or CIM_Curve. 
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual BYTE	GetKeyInterpMode(INT KeyIndex) { return CIM_Linear; }

	/** 
	 *	Get the incoming and outgoing tangent for the given subcurve and key.
	 *	SubIndex must be within range ie >=0 and < NumSubCurves.
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual void	GetTangents(INT SubIndex, INT KeyIndex, FLOAT& ArriveTangent, FLOAT& LeaveTangent) { ArriveTangent=0.f; LeaveTangent=0.f; }

	/** Get input range of keys. Outside this region curve continues constantly the start/end values. */
	virtual void	GetInRange(FLOAT& MinIn, FLOAT& MaxIn) { MinIn=0.f; MaxIn=0.f; }

	/** Get overall range of output values. */
	virtual void	GetOutRange(FLOAT& MinOut, FLOAT& MaxOut) { MinOut=0.f; MaxOut=0.f; }

	/** 
	 *	Add a new key to the curve with the specified input. Its initial value is set using EvalSub at that location. 
	 *	Returns the index of the new key.
	 */
	virtual INT		CreateNewKey(FLOAT KeyIn) { return INDEX_NONE; }

	/** 
	 *	Remove the specified key from the curve.
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual void	DeleteKey(INT KeyIndex) {}

	/** 
	 *	Set the input value of the specified Key. This may change the index of the key, so the new index of the key is retured. 
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual INT		SetKeyIn(INT KeyIndex, FLOAT NewInVal) { return KeyIndex; }

	/** 
	 *	Set the output values of the specified key.
	 *	SubIndex must be within range ie >=0 and < NumSubCurves.
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual void	SetKeyOut(INT SubIndex, INT KeyIndex, FLOAT NewOutVal) {}

	/** 
	 *	Set the method to use for interpolating between the give keyframe and the next one.
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual void	SetKeyInterpMode(INT KeyIndex, EInterpCurveMode NewMode) {}


	/** 
	 *	Set the incoming and outgoing tangent for the given subcurve and key.
	 *	SubIndex must be within range ie >=0 and < NumSubCurves.
	 *	KeyIndex must be within range ie >=0 and < NumKeys.
	 */
	virtual void	SetTangents(INT SubIndex, INT KeyIndex, FLOAT ArriveTangent, FLOAT LeaveTangent) {}


	/** Returns TRUE if this curve uses legacy tangent/interp algorithms and may be 'upgraded' */
	virtual UBOOL	UsingLegacyInterpMethod() const { return FALSE; }


	/** 'Upgrades' this curve to use the latest tangent/interp algorithms (usually, will 'bake' key tangents.) */
	virtual void	UpgradeInterpMethod() {}
};


/*-----------------------------------------------------------------------------
	FRotator.
-----------------------------------------------------------------------------*/

//
// Rotation.
//
class FRotator
{
public:
	// Variables.
	INT Pitch; // Looking up and down (0=Straight Ahead, +Up, -Down).
	INT Yaw;   // Rotating around (running in circles), 0=East, +North, -South.
	INT Roll;  // Rotation about axis of screen, 0=Straight, +Clockwise, -CCW.

	// Serializer.
	friend FArchive& operator<<( FArchive& Ar, FRotator& R )
	{
		return Ar << R.Pitch << R.Yaw << R.Roll;
	}

	// Constructors.
	FRotator() {}
	FRotator( INT InPitch, INT InYaw, INT InRoll )
	:	Pitch(InPitch), Yaw(InYaw), Roll(InRoll) {}
	explicit FORCEINLINE FRotator(EEventParm)
	: Pitch(0), Yaw(0), Roll(0)
	{}

	FRotator( const FQuat& Quat );

	// Binary arithmetic operators.
	FRotator operator+( const FRotator &R ) const
	{
		return FRotator( Pitch+R.Pitch, Yaw+R.Yaw, Roll+R.Roll );
	}
	FRotator operator-( const FRotator &R ) const
	{
		return FRotator( Pitch-R.Pitch, Yaw-R.Yaw, Roll-R.Roll );
	}
	FRotator operator*( FLOAT Scale ) const
	{
		return FRotator( appTrunc(Pitch*Scale), appTrunc(Yaw*Scale), appTrunc(Roll*Scale) );
	}
	FRotator operator*= (FLOAT Scale)
	{
		Pitch = appTrunc(Pitch*Scale); Yaw = appTrunc(Yaw*Scale); Roll = appTrunc(Roll*Scale);
		return *this;
	}
	// Binary comparison operators.
	UBOOL operator==( const FRotator &R ) const
	{
		return Pitch==R.Pitch && Yaw==R.Yaw && Roll==R.Roll;
	}
	UBOOL operator!=( const FRotator &V ) const
	{
		return Pitch!=V.Pitch || Yaw!=V.Yaw || Roll!=V.Roll;
	}
	// Assignment operators.
	FRotator operator+=( const FRotator &R )
	{
		Pitch += R.Pitch; Yaw += R.Yaw; Roll += R.Roll;
		return *this;
	}
	FRotator operator-=( const FRotator &R )
	{
		Pitch -= R.Pitch; Yaw -= R.Yaw; Roll -= R.Roll;
		return *this;
	}
	// Functions.
	FRotator Reduce() const
	{
		return FRotator( ReduceAngle(Pitch), ReduceAngle(Yaw), ReduceAngle(Roll) );
	}
	int IsZero() const
	{
		return ((Pitch&65535)==0) && ((Yaw&65535)==0) && ((Roll&65535)==0);
	}
	FRotator Add( INT DeltaPitch, INT DeltaYaw, INT DeltaRoll )
	{
		Yaw   += DeltaYaw;
		Pitch += DeltaPitch;
		Roll  += DeltaRoll;
		return *this;
	}
	FRotator AddBounded( INT DeltaPitch, INT DeltaYaw, INT DeltaRoll )
	{
		Yaw  += DeltaYaw;
		Pitch = FAddAngleConfined(Pitch,DeltaPitch,192*0x100,64*0x100);
		Roll  = FAddAngleConfined(Roll, DeltaRoll, 192*0x100,64*0x100);
		return *this;
	}
	FRotator GridSnap( const FRotator &RotGrid ) const
	{
		return FRotator
		(
			appTrunc(FSnap(Pitch,RotGrid.Pitch)),
			appTrunc(FSnap(Yaw,  RotGrid.Yaw)),
			appTrunc(FSnap(Roll, RotGrid.Roll))
		);
	}
	FVector Vector() const;
	FQuat Quaternion() const;
	FVector Euler() const;

	static FRotator MakeFromEuler(const FVector& Euler);

	// Resets the rotation values so they fall within the range -65535,65535
	FRotator Clamp() const
	{
		return FRotator( Pitch&65535, Yaw&65535, Roll&65535 );
	}
	FRotator ClampPos() const
	{
		return FRotator( Abs(Pitch)&65535, Abs(Yaw)&65535, Abs(Roll)&65535 );
	}

	static INT NormalizeAxis(INT Angle)
	{
		Angle &= 0xFFFF;
		if( Angle > 32767 ) 
		{
			Angle -= 0x10000;
		}
		return Angle;
	}

	FRotator GetNormalized() const
	{
		FRotator Rot = *this;
		Rot.Pitch = NormalizeAxis(Rot.Pitch);
		Rot.Roll = NormalizeAxis(Rot.Roll);
		Rot.Yaw = NormalizeAxis(Rot.Yaw);
		return Rot;
	}

	FRotator GetDenormalized() const
	{
		FRotator Rot = *this;
		Rot.Pitch	= Rot.Pitch & 0xFFFF;
		Rot.Yaw		= Rot.Yaw & 0xFFFF;
		Rot.Roll	= Rot.Roll & 0xFFFF;
		return Rot;
	}

	void MakeShortestRoute();
	void GetWindingAndRemainder(FRotator& Winding, FRotator& Remainder) const;

	FString ToString() const
	{
		return FString::Printf(TEXT("P=%i Y=%i R=%i"), Pitch, Yaw, Roll );
	}

};

inline FRotator operator*( FLOAT Scale, const FRotator &R )
{
	return R.operator*( Scale );
}


/**
 * Create an orthonormal basis from a basis with at least two orthogonal vectors.
 * It may change the directions of the X and Y axes to make the basis orthogonal,
 * but it won't change the direction of the Z axis.
 * All axes will be normalized.
 * @param XAxis - The input basis' XAxis, and upon return the orthonormal basis' XAxis.
 * @param YAxis - The input basis' YAxis, and upon return the orthonormal basis' YAxis.
 * @param ZAxis - The input basis' ZAxis, and upon return the orthonormal basis' ZAxis.
 */
extern void CreateOrthonormalBasis(FVector& XAxis,FVector& YAxis,FVector& ZAxis);

/**
 * Creates a hash value from a FVector. Uses pointers to the elements to
 * bypass any type conversion. This is a simple hash that just ORs the
 * raw 32bit data together
 *
 * @param Vector the vector to create a hash value for
 *
 * @return The hash value from the components
 */
inline DWORD GetTypeHash(const FVector& Vector)
{
	// Note: this assumes there's no padding in FVector that could contain uncompared data.
	return appMemCrc(&Vector,sizeof(FVector));
}

/*-----------------------------------------------------------------------------
	Bounds.
-----------------------------------------------------------------------------*/

//
// A rectangular minimum bounding volume.
//
class FBox
{
public:
	// Variables.
	FVector Min;
	FVector Max;
	BYTE IsValid;

	// Constructors.
	FBox() {}
	FBox(INT) { Init(); }
	FBox( const FVector& InMin, const FVector& InMax ) : Min(InMin), Max(InMax), IsValid(1) {}
	FBox( const FVector* Points, INT Count );

	/** Utility function to build an AABB from Origin and Extent */
	static FBox BuildAABB( const FVector& Origin, const FVector& Extent )
	{
		FBox NewBox(0);
		NewBox += Origin - Extent;
		NewBox += Origin + Extent;
		return NewBox;
	}

	// Accessors.
	FVector& GetExtrema( int i )
	{
		return (&Min)[i];
	}
	const FVector& GetExtrema( int i ) const
	{
		return (&Min)[i];
	}

	// Functions.
	void Init()
	{
		Min = Max = FVector(0,0,0);
		IsValid = 0;
	}
	FORCEINLINE FBox& operator+=( const FVector &Other )
	{
		if( IsValid )
		{
#if ASM_X86
			__asm
			{
				mov		eax,[Other]
				mov		ecx,[this]
				
				movss	xmm3,[eax]FVector.X
				movss	xmm4,[eax]FVector.Y
				movss	xmm5,[eax]FVector.Z

				movss	xmm0,[ecx]FBox.Min.x
				movss	xmm1,[ecx]FBox.Min.y
				movss	xmm2,[ecx]FBox.Min.z
				minss	xmm0,xmm3
				minss	xmm1,xmm4
				minss	xmm2,xmm5
				movss	[ecx]FBox.Min.x,xmm0
				movss	[ecx]FBox.Min.y,xmm1
				movss	[ecx]FBox.Min.z,xmm2

				movss	xmm0,[ecx]FBox.Max.x
				movss	xmm1,[ecx]FBox.Max.y
				movss	xmm2,[ecx]FBox.Max.z
				maxss	xmm0,xmm3
				maxss	xmm1,xmm4
				maxss	xmm2,xmm5
				movss	[ecx]FBox.Max.x,xmm0
				movss	[ecx]FBox.Max.y,xmm1
				movss	[ecx]FBox.Max.z,xmm2
			}
#else
			Min.x = ::Min( Min.x, Other.x );
			Min.y = ::Min( Min.y, Other.y );
			Min.z = ::Min( Min.z, Other.z );

			Max.x = ::Max( Max.x, Other.x );
			Max.y = ::Max( Max.y, Other.y );
			Max.z = ::Max( Max.z, Other.z );
#endif
		}
		else
		{
			Min = Max = Other;
			IsValid = 1;
		}
		return *this;
	}
	FBox operator+( const FVector& Other ) const
	{
		return FBox(*this) += Other;
	}
	FBox& operator+=( const FBox& Other )
	{
		if( IsValid && Other.IsValid )
		{
#if ASM_X86
			__asm
			{
				mov		eax,[Other]
				mov		ecx,[this]

				movss	xmm0,[ecx]FBox.Min.x
				movss	xmm1,[ecx]FBox.Min.y
				movss	xmm2,[ecx]FBox.Min.z
				minss	xmm0,[eax]FBox.Min.x
				minss	xmm1,[eax]FBox.Min.y
				minss	xmm2,[eax]FBox.Min.z
				movss	[ecx]FBox.Min.x,xmm0
				movss	[ecx]FBox.Min.y,xmm1
				movss	[ecx]FBox.Min.z,xmm2

				movss	xmm0,[ecx]FBox.Max.x
				movss	xmm1,[ecx]FBox.Max.y
				movss	xmm2,[ecx]FBox.Max.z
				maxss	xmm0,[eax]FBox.Max.x
				maxss	xmm1,[eax]FBox.Max.y
				maxss	xmm2,[eax]FBox.Max.z
				movss	[ecx]FBox.Max.x,xmm0
				movss	[ecx]FBox.Max.y,xmm1
				movss	[ecx]FBox.Max.z,xmm2
			}
#else
			Min.x = ::Min( Min.x, Other.Min.x );
			Min.y = ::Min( Min.y, Other.Min.y );
			Min.z = ::Min( Min.z, Other.Min.z );

			Max.x = ::Max( Max.x, Other.Max.x );
			Max.y = ::Max( Max.y, Other.Max.y );
			Max.z = ::Max( Max.z, Other.Max.z );
#endif
		}
		else if( Other.IsValid )
		{
			*this = Other;
		}
		return *this;
	}
	FBox operator+( const FBox& Other ) const
	{
		return FBox(*this) += Other;
	}
    FVector& operator[]( INT i )
	{
		assert(i>-1);
		assert(i<2);
		if( i == 0 )		return Min;
		else				return Max;
	}
	FBox TransformBy( const FMatrix& M ) const;
	FBox TransformProjectBy( const FMatrix& ProjM ) const;
	FBox ExpandBy( FLOAT W ) const
	{
		return FBox( Min - FVector(W,W,W), Max + FVector(W,W,W) );
	}

	// Returns the midpoint between the min and max points.
	FVector GetCenter() const
	{
		return FVector( ( Min + Max ) * 0.5f );
	}
	// Returns the extent around the center
	FVector GetExtent() const
	{
		return 0.5f*(Max - Min);
	}

	void GetCenterAndExtents( FVector & center, FVector & Extents ) const
	{
		Extents = GetExtent();
		center = Min + Extents;
	}

	UBOOL Intersect( const FBox & other ) const
	{
		if( Min.x > other.Max.x || other.Min.x > Max.x )
			return FALSE;
		if( Min.y > other.Max.y || other.Min.y > Max.y )
			return FALSE;
		if( Min.z > other.Max.z || other.Min.z > Max.z )
			return FALSE;
		return TRUE;
	}

	UBOOL IntersectXY( const FBox& other ) const
	{
		if( Min.x > other.Max.x || other.Min.x > Max.x )
			return FALSE;
		if( Min.y > other.Max.y || other.Min.y > Max.y )
			return FALSE;
		return TRUE;
	}

	// Checks to see if the location is inside this box
	UBOOL IsInside( const FVector& In ) const
	{
		return ( In.x > Min.x && In.x < Max.x
				&& In.y > Min.y && In.y < Max.y 
				&& In.z > Min.z && In.z < Max.z );
	}

	/** Calculate volume of this box. */
	FLOAT GetVolume() const
	{
		return ((Max.x-Min.x) * (Max.y-Min.y) * (Max.z-Min.z));
	}

	// Serializer.
	//friend FArchive& operator<<( FArchive& Ar, FBox& Bound )
	//{
	//	return Ar << Bound.Min << Bound.Max << Bound.IsValid;
	//}
};


/** a bounding cylinder */
struct FCylinder
{
	FLOAT Radius, Height;

	/** empty default constructor */
	FORCEINLINE FCylinder()
	{}

	/** initializing constructor */
	FORCEINLINE FCylinder(FLOAT InRadius, FLOAT InHeight)
		: Radius(InRadius), Height(InHeight)
	{}
	explicit FORCEINLINE FCylinder(EEventParm)
		: Radius(0.0f), Height(0.0f)
	{}

	/** returns the extent for the axis aligned box that most closely represents this cylinder */
	FORCEINLINE FVector GetExtent()
	{
		return FVector(Radius, Radius, Height);
	}
};

/*-----------------------------------------------------------------------------
	Interpolation functions
-----------------------------------------------------------------------------*/
FLOAT FInterpTo( FLOAT& Current, FLOAT& Target, FLOAT& DeltaTime, FLOAT InterpSpeed );

FLOAT FInterpEaseInOut( FLOAT A, FLOAT B, FLOAT Alpha, FLOAT Exp );



/** 
 * Fast Linear Quaternion Interpolation.
 * Result is NOT normalized.
 */
FORCEINLINE FQuat LerpQuat(const FQuat& A, const FQuat& B, const FLOAT Alpha)
{
	// To ensure the 'shortest route', we make sure the dot product between the both rotations is positive.
	if( (A | B) < 0.f )
	{
		return (B * Alpha) - (A * (1.f - Alpha));
	}

	// Then add on the second rotation..
	return (B * Alpha) + (A * (1.f - Alpha));
}

/** 
 * Bi-Linear Quaternion interpolation.
 * Result is NOT normalized.
 */
FORCEINLINE FQuat BiLerpQuat(const FQuat& P00, const FQuat& P10, const FQuat& P01, const FQuat& P11, FLOAT FracX, FLOAT FracY)
{
	return LerpQuat(
					LerpQuat(P00,P10,FracX),
					LerpQuat(P01,P11,FracX),
					FracY
					);
}


/**
 * Spherical interpolation. Will correct alignment. Output is not normalized.
 */
FQuat SlerpQuat(const FQuat &Quat1,const FQuat &Quat2, FLOAT Slerp);
FQuat SlerpQuatFullPath(const FQuat &quat1, const FQuat &quat2, FLOAT Alpha);
FQuat SquadQuat(const FQuat& quat1, const FQuat& tang1, const FQuat& quat2, const FQuat& tang2, FLOAT Alpha);

/*-----------------------------------------------------------------------------
	Bezier curves
-----------------------------------------------------------------------------*/

/**
 * Generates a list of sample points on a Bezier curve defined by 2 points.
 *
 * @param	ControlPoints	Array of 4 FVectors (vert1, controlpoint1, controlpoint2, vert2).
 * @param	NumPoints		Number of samples.
 * @param	OutPoints		Receives the output samples.
 * @return					Path length.
 */
FLOAT EvaluateBezier(const FVector* ControlPoints, INT NumPoints, TArray<FVector>& OutPoints);

/*-----------------------------------------------------------------------------
	FInterpCurve.
-----------------------------------------------------------------------------*/

/**
 * Clamps a tangent formed by the specified control point values
 */
FLOAT FClampFloatTangent( FLOAT PrevPointVal, FLOAT PrevTime, FLOAT CurPointVal, FLOAT CurTime, FLOAT NextPointVal, FLOAT NextTime );


template< class T, class U > void LegacyAutoCalcTangent( const T& PrevP, const T& P, const T& NextP, const U& Tension, T& OutTan )
{
	OutTan = 0.5f * (1.f - Tension) * ( (P - PrevP) + (NextP - P) );
}

template< class T, class U > void AutoCalcTangent( const T& PrevP, const T& P, const T& NextP, const U& Tension, T& OutTan )
{
	OutTan = (1.f - Tension) * ( (P - PrevP) + (NextP - P) );
}


//////////////////////////////////////////////////////////////////////////
// Support for InterpCurves of Quaternions
template< class U > FQuat Lerp( const FQuat& A, const FQuat& B, const U& Alpha)
{
	return SlerpQuat(A, B, Alpha);
}

inline FQuat BiLerp(const FQuat& P00, const FQuat& P10, const FQuat& P01, const FQuat& P11, FLOAT FracX, FLOAT FracY)
{
	FQuat Result;

	Result = Lerp(
				Lerp(P00,P10,FracX),
				Lerp(P01,P11,FracX),
				FracY
				);

	Result.Normalize();

	return Result;
}


/**
 * In the case of quaternions, we use a bezier like approach.
 * T - Actual 'control' orientations.
 */
template< class U > FQuat CubicInterp( const FQuat& P0, const FQuat& T0, const FQuat& P1, const FQuat& T1, const U& A)
{
	return SquadQuat(P0, T0, P1, T1, A);
}




/** Convert a direction vector into a 'heading' angle between +/-PI. 0 is pointing down +X. */
inline FLOAT HeadingAngle(FVector Dir)
{
	// Project Dir into Z plane.
	FVector PlaneDir = Dir;
	PlaneDir.z = 0.f;
	PlaneDir = PlaneDir.SafeNormal();

	FLOAT Angle = appAcos(PlaneDir.x);

	if(PlaneDir.y < 0.0f)
	{
		Angle *= -1.0f;
	}

	return Angle;
}

/** Find the smallest angle between two headings (in radians) */
inline FLOAT FindDeltaAngle(FLOAT A1, FLOAT A2)
{
	// Find the difference
	FLOAT Delta = A2 - A1;

	// If change is larger than PI
	if(Delta > noMath::PI)
	{
		// Flip to negative equivalent
		Delta = Delta - (noMath::PI * 2.0f);
	}
	else if(Delta < -noMath::PI)
	{
		// Otherwise, if change is smaller than -PI
		// Flip to positive equivalent
		Delta = Delta + (noMath::PI * 2.0f);
	}

	// Return delta in [-PI,PI] range
	return Delta;
}

/** Given a heading which may be outside the +/- PI range, 'unwind' it back into that range. */
inline FLOAT UnwindHeading(FLOAT A)
{
	while(A > noMath::PI)
	{
		A -= ((FLOAT)noMath::PI * 2.0f);
	}

	while(A < -noMath::PI)
	{
		A += ((FLOAT)noMath::PI * 2.0f);
	}

	return A;
}


/*-----------------------------------------------------------------------------
	Floating point constants.
-----------------------------------------------------------------------------*/

/**
 * Lengths of normalized vectors (These are half their maximum values
 * to assure that dot products with normalized vectors don't overflow).
 */
#define FLOAT_NORMAL_THRESH				(0.0001f)

//
// Magic numbers for numerical precision.
//
#define THRESH_POINT_ON_PLANE			(0.10f)		/* Thickness of plane for front/back/inside test */
#define THRESH_POINT_ON_SIDE			(0.20f)		/* Thickness of polygon side's side-plane for point-inside/outside/on side test */
#define THRESH_POINTS_ARE_SAME			(0.002f)	/* Two points are same if within this distance */
#define THRESH_POINTS_ARE_NEAR			(0.015f)	/* Two points are near if within this distance and can be combined if imprecise math is ok */
#define THRESH_NORMALS_ARE_SAME			(0.00002f)	/* Two normal points are same if within this distance */
													/* Making this too large results in incorrect CSG classification and disaster */
#define THRESH_VECTORS_ARE_NEAR			(0.0004f)	/* Two vectors are near if within this distance and can be combined if imprecise math is ok */
													/* Making this too large results in lighting problems due to inaccurate texture coordinates */
#define THRESH_SPLIT_POLY_WITH_PLANE	(0.25f)		/* A plane splits a polygon in half */
#define THRESH_SPLIT_POLY_PRECISELY		(0.01f)		/* A plane exactly splits a polygon */
#define THRESH_ZERO_NORM_SQUARED		(0.0001f)	/* Size of a unit normal that is considered "zero", squared */
#define THRESH_VECTORS_ARE_PARALLEL		(0.02f)		/* Vectors are parallel if dot product varies less than this */

/*-----------------------------------------------------------------------------
	FVector friends.
-----------------------------------------------------------------------------*/

/**
 * Compare two points and see if they're the same, using a threshold.
 * Returns 1=yes, 0=no.  Uses fast distance approximation.
 */
inline UBOOL FPointsAreSame( const FVector &P, const FVector &Q )
{
	FLOAT Temp;
	Temp=P.x-Q.x;
	if( (Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME) )
	{
		Temp=P.y-Q.y;
		if( (Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME) )
		{
			Temp=P.z-Q.z;
			if( (Temp > -THRESH_POINTS_ARE_SAME) && (Temp < THRESH_POINTS_ARE_SAME) )
			{
				return 1;
			}
		}
	}
	return 0;
}

/**
 * Compare two points and see if they're the same, using a threshold.
 * Returns 1=yes, 0=no.  Uses fast distance approximation.
 */
inline UBOOL FPointsAreNear( const FVector &Point1, const FVector &Point2, FLOAT Dist )
{
	FLOAT Temp;
	Temp=(Point1.x - Point2.x); if (Abs(Temp)>=Dist) return 0;
	Temp=(Point1.y - Point2.y); if (Abs(Temp)>=Dist) return 0;
	Temp=(Point1.z - Point2.z); if (Abs(Temp)>=Dist) return 0;
	return 1;
}

/**
 * Calculate the signed distance (in the direction of the normal) between
 * a point and a plane.
 */
inline FLOAT FPointPlaneDist
(
	const FVector &Point,
	const FVector &PlaneBase,
	const FVector &PlaneNormal
)
{
	return (Point - PlaneBase) * PlaneNormal;
}

/**
 * Euclidean distance between two points.
 */
inline FLOAT FDist( const FVector &V1, const FVector &V2 )
{
	return noMath::Sqrt( Square(V2.x-V1.x) + Square(V2.y-V1.y) + Square(V2.z-V1.z) );
}

/**
 * Squared distance between two points.
 */
inline FLOAT FDistSquared( const FVector &V1, const FVector &V2 )
{
	return Square(V2.x-V1.x) + Square(V2.y-V1.y) + Square(V2.z-V1.z);
}

/**
 * See if two normal vectors (or plane normals) are nearly parallel.
 */
inline UBOOL FParallel( const FVector &Normal1, const FVector &Normal2 )
{
	const FLOAT NormalDot = Normal1 * Normal2;
	return (Abs (NormalDot - 1.f) <= THRESH_VECTORS_ARE_PARALLEL);
}

/**
 * See if two planes are coplanar.
 */
inline UBOOL FCoplanar( const FVector &Base1, const FVector &Normal1, const FVector &Base2, const FVector &Normal2 )
{
	if      (!FParallel(Normal1,Normal2)) return 0;
	else if (FPointPlaneDist (Base2,Base1,Normal1) > THRESH_POINT_ON_PLANE) return 0;
	else    return 1;
}


class FQuatRotationTranslationMatrix : public FMatrix
{
public:
	FQuatRotationTranslationMatrix(const FQuat& Q, const FVector& Origin)
	{
		const FLOAT x2 = Q.x + Q.x;  const FLOAT y2 = Q.y + Q.y;  const FLOAT z2 = Q.z + Q.z;
		const FLOAT xx = Q.x * x2;   const FLOAT xy = Q.x * y2;   const FLOAT xz = Q.x * z2;
		const FLOAT yy = Q.y * y2;   const FLOAT yz = Q.y * z2;   const FLOAT zz = Q.z * z2;
		const FLOAT wx = Q.w * x2;   const FLOAT wy = Q.w * y2;   const FLOAT wz = Q.w * z2;

		mat[0][0] = 1.0f - (yy + zz);	mat[1][0] = xy - wz;				mat[2][0] = xz + wy;			mat[3][0] = Origin.x;
		mat[0][1] = xy + wz;			mat[1][1] = 1.0f - (xx + zz);		mat[2][1] = yz - wx;			mat[3][1] = Origin.y;
		mat[0][2] = xz - wy;			mat[1][2] = yz + wx;				mat[2][2] = 1.0f - (xx + yy);	mat[3][2] = Origin.z;
		mat[0][3] = 0.0f;				mat[1][3] = 0.0f;					mat[2][3] = 0.0f;				mat[3][3] = 1.0f;

		TransposeSelf();
	}
};