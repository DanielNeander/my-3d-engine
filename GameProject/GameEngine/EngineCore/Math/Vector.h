#pragma once 
#ifndef __MATH_VECTOR_H__
#define __MATH_VECTOR_H__

//#include <Core/SysCommon.h>
#include "Math.h"
#include "Enginecore/Heap.h"
/*
===============================================================================

  Vector classes

===============================================================================
*/



#define VECTOR_EPSILON		0.001f

class idPolar3;
class noQuat;

class noVec2 {
public:
	float x,y;

	noVec2();
	explicit noVec2( const float x, const float y );
	explicit noVec2( const float x);

	void		Set( const float x, const float y);
	void		Zero( void );

	float		operator[]( int index ) const;
	float&		operator[]( int index );
	noVec2		operator-() const;
	float		operator*( const noVec2& a ) const;
	noVec2		operator*( const float a ) const;
	noVec2		operator/( const float a ) const;
	noVec2		operator+( const noVec2& a ) const;
	noVec2		operator-( const noVec2& a ) const;
	noVec2&		operator+=( const noVec2& a);
	noVec2&		operator-=( const noVec2& a);
	noVec2&		operator/=( const noVec2& a);
	noVec2&		operator/=( const float a);
	noVec2&		operator*=( const float a);

	friend noVec2	operator*( const float a, const noVec2 b );

	bool		Compare( const noVec2& a ) const;
	bool		Compare( const noVec2& a, const float epsilon ) const;
	bool		operator==( const noVec2& a ) const;
	bool		operator!=( const noVec2& a ) const;

	float		Length( void ) const;
	float		LengthFast( void ) const;
	float		LengthSqr( void ) const;
	float		Normalize( void );
	float		NormalizeFast( void );
	noVec2&		Truncate( float length );
	void		Clamp( const noVec2& min, const noVec2& max );
	void		Snap( void );
	void		SnapInt( void );

	//returns the vector that is perpendicular to this one.
	noVec2  Perp()const;

	int				GetDimension( void ) const;

	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

	void			Lerp( const noVec2 &v1, const noVec2 &v2, const float l );	
	int				Sign(const noVec2& v2)	const;
	int				IsNearlyZero(FLOAT Tolerance=KINDA_SMALL_NUMBER) const;

};

extern noVec2 vec2_origin;
#define vec2_zero vec2_origin

NO_INLINE noVec2::noVec2( void ) {}

NO_INLINE noVec2::noVec2( const float x, const float y) {
	this->x = x;
	this->y = y;
}

NO_INLINE noVec2::noVec2( const float _x )
	:x(_x), y(_x)
{

}

NO_INLINE void noVec2::Set( const float x, const float y ) {
	this->x = x;
	this->y = y;
}

NO_INLINE void noVec2::Zero( void ) {
	x =  y = 0.0f;
}

NO_INLINE float noVec2::operator[]( int index ) const {
	return (&x)[index];
}

NO_INLINE float& noVec2::operator[]( int index ) {
	return (&x)[index];
}

NO_INLINE noVec2 noVec2::operator-() const {
	return noVec2(-x, -y);
}

NO_INLINE float noVec2::operator*( const noVec2& a ) const {
	return x * a.x + y * a.y;
}

NO_INLINE noVec2 noVec2::operator*( const float a ) const {
	return noVec2( x * a, y * a);
}

NO_INLINE noVec2 operator*( const float a, const noVec2 b ) {
	return noVec2( b.x * a, b.y * a);
}

NO_INLINE noVec2 noVec2::operator/( const float a ) const {
	float inva = 1.0f / a;
	return noVec2( x * inva, y * inva );
}

NO_INLINE noVec2 noVec2::operator+( const noVec2& a ) const {
	return noVec2( x + a.x, y + a.y );
}

NO_INLINE noVec2 noVec2::operator-( const noVec2& a ) const {
	return noVec2( x  - a.x, y - a.y );
}

NO_INLINE int noVec2::IsNearlyZero(FLOAT Tolerance) const
{
	return	noMath::Fabs(x)<Tolerance 
		&&	noMath::Fabs(y)<Tolerance;
}

NO_INLINE noVec2& noVec2::operator+=( const noVec2& a )	{
	x += a.x;
	y += a.y;
	return *this;
}

NO_INLINE noVec2& noVec2::operator-=( const noVec2& a ) {
	x -= a.x;
	y -= a.y;
	return *this;	
}

NO_INLINE noVec2& noVec2::operator/=( const noVec2& a ) {
	x /= a.x;
	y /= a.y;
	return *this;

}

NO_INLINE noVec2& noVec2::operator/=( const float a ) {
	float inva = 1.0f / a;
	x *= inva;
	y *= inva;
	return *this;
}

NO_INLINE noVec2& noVec2::operator*=( const float a ) {
	x *= a;
	y *= a;
	return *this;
}

NO_INLINE bool noVec2::Compare( const noVec2& a ) const
{
	return ( x == a.x) && ( y == a.y);
}

NO_INLINE bool noVec2::Compare( const noVec2& a, const float epsilon ) const
{
	if ( noMath::Fabs( x - a.x )  > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( y - a.y ) > epsilon ) {
		return false;
	}

	return true;
}

NO_INLINE bool noVec2::operator==( const noVec2& a ) const
{
	return Compare( a );
}

NO_INLINE bool noVec2::operator!=( const noVec2& a ) const
{
	return !Compare( a );
}

NO_INLINE float noVec2::Length( void ) const {
	return ( float )noMath::Sqrt( x * x + y * y );
}

NO_INLINE float noVec2::LengthFast( void ) const
{
	return 0.f;
}

NO_INLINE float noVec2::LengthSqr( void ) const	{
	return ( x * x + y * y );	
}

NO_INLINE float noVec2::Normalize( void ) {
	float sqrLen, invLen;
	sqrLen = x * x + y * y;
	invLen = noMath::InvSqrt( sqrLen );
	x *= invLen;
	y *= invLen;
	return invLen * sqrLen;
}

NO_INLINE float noVec2::NormalizeFast( void )	{
	return 0.f;
}

NO_INLINE noVec2& noVec2::Truncate( float length ) {
	float len2;
	float ilen;
	if ( !length ) {
		Zero();
	}
	else {
		len2 = LengthSqr();
		if ( len2 > length * length ) {
			ilen = length * noMath::InvSqrt( len2 );
			x *= ilen;
			y *= ilen;
		}
	}
	return *this;
}

NO_INLINE noVec2 noVec2::Perp() const
{
	return noVec2(-y, x);
}


NO_INLINE void noVec2::Clamp( const noVec2& min, const noVec2& max ) {
	if ( x < min.x ) {
		x = min.x;
	} else if ( x > max.x ) {
		x = max.x;
	}
	if ( y < min.y ) {
		y = min.y;
	} else if ( y > max.y ) {
		y = max.y;
	}
}

NO_INLINE void noVec2::Snap( void ) {
	x = floor( x + 0.5f );
	y = floor( y + 0.5f );
}

NO_INLINE void noVec2::SnapInt( void ) {
	x = float( int( x ) );
	y = float( int( y ) );
}

NO_INLINE int noVec2::GetDimension( void ) const {
	return 2;
}

NO_INLINE const float * noVec2::ToFloatPtr( void ) const {
	return &x;
}

NO_INLINE float * noVec2::ToFloatPtr( void ) {
	return &x;
}

//------------------------ Sign ------------------------------------------
//
//  returns positive if v2 is clockwise of this vector,
//  minus if anticlockwise (Y axis pointing down, X axis to right)
//------------------------------------------------------------------------
enum {clockwise = 1, anticlockwise = -1};

NO_INLINE int noVec2::Sign(const noVec2& v2)const
{
	if (y*v2.x > x*v2.y)
	{ 
		return anticlockwise;
	}
	else 
	{
		return clockwise;
	}
}

class noVec3 {
public:
	float	x,y,z;

	noVec3( void );
	explicit noVec3( const float x, const float y, const float z );
	explicit noVec3( const float x);

	void 			Set( const float x, const float y, const float z );
	void			Zero( void );

	float			operator[]( const int index ) const;
	float &			operator[]( const int index );
	noVec3			operator-() const;
	noVec3 &		operator=( const noVec3 &a );		// required because of a msvc 6 & 7 bug
	float			operator*( const noVec3 &a ) const;
	noVec3			operator*( const float a ) const;
	noVec3			operator/( const float a ) const;
	noVec3			operator+( const noVec3 &a ) const;
	noVec3			operator-( const noVec3 &a ) const;
	noVec3 &		operator+=( const noVec3 &a );
	noVec3 &		operator-=( const noVec3 &a );
	noVec3 &		operator/=( const noVec3 &a );
	noVec3 &		operator/=( const float a );
	noVec3 &		operator*=( const float a );
	bool            operator<(const noVec3& vIn) const;	
	
	friend noVec3	operator*( const float a, const noVec3 b );

	bool			Compare( const noVec3 &a ) const;							// exact compare, no epsilon
	bool			Compare( const noVec3 &a, const float epsilon ) const;		// compare with epsilon
	bool			operator==(	const noVec3 &a ) const;						// exact compare, no epsilon
	bool			operator!=(	const noVec3 &a ) const;						// exact compare, no epsilon

	bool			FixDegenerateNormal( void );	// fix degenerate axial cases
	bool			FixDenormals( void );			// change tiny numbers to zero

	noVec3			Cross( const noVec3 &a ) const;
	noVec3 &		Cross( const noVec3 &a, const noVec3 &b );
	float			Length( void ) const;
	float			LengthSqr( void ) const;
	float			LengthFast( void ) const;
	float			Normalize( void );				// returns length
	float			NormalizeFast( void );			// returns length
	noVec3 &		Truncate( float length );		// cap length
	void			Clamp( const noVec3 &min, const noVec3 &max );
	void			Snap( void );					// snap to closest integer value
	void			SnapInt( void );				// snap towards integer (floor)

	void			Lerp( const noVec3 &v1, const noVec3 &v2, const float l );
	void			SLerp( const noVec3 &v1, const noVec3 &v2, const float l );

	int				GetDimension( void ) const;
	noMat3			ToMat3( void ) const;	
	float			ToYaw( void ) const;
	float			ToPitch( void ) const;
	noAngles		ToAngles( void ) const;
	idPolar3		ToPolar( void ) const;
	const noVec2 &	ToVec2( void ) const;
	noVec2 &		ToVec2( void );
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );

	float      Distance(const noVec3& vIn) const;
	float      DistanceSqr(const noVec3& vIn) const;

	void			Max(const noVec3 &other );
	void			Min(const noVec3 &other );

	FLOAT GetMax() const
	{
		return ::Max(::Max(x,y),z);
	}
	FLOAT GetAbsMax() const
	{
		return ::Max(::Max(abs(x),abs(y)),abs(z));
	}
	FLOAT GetMin() const
	{
		return ::Min(::Min(x,y),z);
	}

	noQuat getRotationTo(const noVec3& dest, const noVec3& fallbackAxis = noVec3(0.0f, 0.0f, 0.0f)) const;
	int				Sign(const noVec3& v2)const;
	noVec3			Perp() const;
	noVec3	NormalizeCopy(void) const;

	/** Calculates a reflection vector to the plane with the given normal .
        @remarks NB assumes 'this' is pointing AWAY FROM the plane, invert if it is not.
        */
    inline noVec3	Reflect(const noVec3& normal) const
    {
         return noVec3( *this - ( 2 * (*this) * (normal) * normal ) );
    }
	const noVec3	Multiply(const noVec3& vIn) const;
	noVec3			Perpendicular(void) const;

	noVec3			SafeNormal2D(FLOAT Tolerance=1e-8) const;
	noVec3			SafeNormal(FLOAT Tolerance=1e-8) const;

	int				IsNearlyZero(FLOAT Tolerance=KINDA_SMALL_NUMBER) const;
	void			ProjectSelfOntoSphere( const float radius );
	
	bool			ProjectAlongPlane( const noVec3 &normal, const float epsilon, const float overBounce );
	void			ProjectOntoPlane( const noVec3 &normal, const float overBounce );
	void			OrthogonalBasis( noVec3 &left, noVec3 &up ) const;
	void			NormalVectors( noVec3 &left, noVec3 &down ) const;
	const char *ToString( int precision = 2) const;
};

extern noVec3 vec3_origin;

extern noVec3 vec3_origin;
#define vec3_zero vec3_origin


NO_INLINE noVec3::noVec3( void ) {}

NO_INLINE noVec3::noVec3( const float x, const float y, const float z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

NO_INLINE noVec3::noVec3( const float _x )
	:x(_x), y(_x), z(_x)
{

}

NO_INLINE void noVec3::Set( const float x, const float y, const float z ) {
	this->x = x;
	this->y = y;
	this->z = z;
}

NO_INLINE void noVec3::Zero( void ) {
	x =  y = z = 0.0f;
}

NO_INLINE float noVec3::operator[]( int index ) const {
	return (&x)[index];
}

NO_INLINE float& noVec3::operator[]( int index ) {
	return (&x)[index];
}

NO_INLINE noVec3 & noVec3::operator=( const noVec3 &a ) {
	x = a.x;
	y = a.y; 
	z = a.z;
	return *this;
}


NO_INLINE noVec3 noVec3::operator-() const {
	return noVec3(-x, -y, -z);
}

NO_INLINE float noVec3::operator*( const noVec3& a ) const {
	return x * a.x + y * a.y + z * a.z;
}

NO_INLINE noVec3 noVec3::operator*( const float a ) const {
	return noVec3( x * a, y * a, z * a );
}

NO_INLINE noVec3 noVec3::operator/( const float a ) const {
	float inva = 1.0f / a;
	return noVec3( x * inva, y * inva, z  * inva );
}

NO_INLINE noVec3 noVec3::operator+( const noVec3& a ) const {
	return noVec3( x + a.x, y + a.y, z + a.z );
}

NO_INLINE noVec3 operator*( const float a, const noVec3 b ) {
	return noVec3( b.x * a, b.y * a, b.z * a);
}

NO_INLINE noVec3 noVec3::operator-( const noVec3& a ) const {
	return noVec3( x  - a.x, y - a.y, z - a.z );
}

NO_INLINE noVec3& noVec3::operator+=( const noVec3& a )	{
	x += a.x;
	y += a.y;
	z += a.z;
	return *this;
}

NO_INLINE noVec3& noVec3::operator-=( const noVec3& a ) {
	x -= a.x;
	y -= a.y;
	z -= a.z;
	return *this;	
}

NO_INLINE noVec3& noVec3::operator/=( const noVec3& a ) {
	x /= a.x;
	y /= a.y;
	z /= a.z;
	return *this;

}

NO_INLINE noVec3& noVec3::operator/=( const float a ) {
	float inva = 1.0f / a;
	x *= inva;
	y *= inva;
	z *= inva;
	return *this;
}

NO_INLINE noVec3& noVec3::operator*=( const float a ) {
	x *= a;
	y *= a;
	z *= a;
	return *this;
}

NO_INLINE bool noVec3::operator<(const noVec3& vIn) const
{
	if (x == vIn.x)
	{
		if (y == vIn.y)
			return z < vIn.z;
		else 
			return y < vIn.y;
	}
	else
		return x < vIn.x;
}

NO_INLINE bool noVec3::Compare( const noVec3& a ) const
{
	return ( x == a.x) && ( y == a.y) && ( z == a.z );
}

NO_INLINE bool noVec3::Compare( const noVec3& a, const float epsilon ) const
{
	if ( noMath::Fabs( x - a.x )  > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( y - a.y ) > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( z - a.z ) > epsilon ) {
		return false;
	}

	return true;
}

NO_INLINE bool noVec3::operator==( const noVec3& a ) const
{
	return Compare( a );
}

NO_INLINE bool noVec3::operator!=( const noVec3& a ) const
{
	return !Compare( a );
}

NO_INLINE float noVec3::Length( void ) const {
	return ( float )noMath::Sqrt( x * x + y * y + z * z );
}

NO_INLINE float noVec3::LengthFast( void ) const {
	return 0.f;
}

NO_INLINE float noVec3::LengthSqr( void ) const	{
	return ( x * x + y * y + z * z );	
}

NO_INLINE float noVec3::DistanceSqr( const noVec3& vIn ) const
{
	return ((*this - vIn).LengthSqr());    
}

NO_INLINE float noVec3::Distance( const noVec3& vIn ) const
{
	return ((*this - vIn).Length());    
}


NO_INLINE float noVec3::Normalize( void ) {
	float sqrLen, invLen;
	sqrLen = x * x + y * y + z * z;
	invLen = noMath::InvSqrt( sqrLen );
	x *= invLen;
	y *= invLen;
	z *= invLen;
	return invLen * sqrLen;
}

NO_INLINE float noVec3::NormalizeFast( void )	{
	return 0.f;
}

NO_INLINE noVec3 noVec3::SafeNormal(FLOAT Tolerance) const
{
	const float SquareSum = x*x + y*y + z*z;

	// Not sure if it's safe to add tolerance in there. Might introduce too many errors
	if( SquareSum == 1.f )
	{
		return *this;
	}		
	else if( SquareSum < Tolerance )
	{
		return noVec3(0.f);
	}
	const FLOAT Scale = noMath::InvSqrt(SquareSum);
	return noVec3(x*Scale, y*Scale, z*Scale);
}

NO_INLINE noVec3 noVec3::SafeNormal2D(FLOAT Tolerance) const
{
	const FLOAT SquareSum = x*x + y*y;

	// Not sure if it's safe to add tolerance in there. Might introduce too many errors
	if( SquareSum == 1.f )
	{
		if( z == 0.f )
		{
			return *this;
		}
		else
		{
			return noVec3(x, y, 0.f);
		}
	}
	else if( SquareSum < Tolerance )
	{
		return noVec3(0.f);
	}

	const FLOAT Scale = noMath::InvSqrt(SquareSum);
	return noVec3(x*Scale, y*Scale, 0.f);
}


NO_INLINE noVec3& noVec3::Truncate( float length ) {
	float len2;
	float ilen;
	if ( !length ) {
		Zero();
	}
	else {
		len2 = LengthSqr();
		if ( len2 > length * length ) {
			ilen = length * noMath::InvSqrt( len2 );
			x *= ilen;
			y *= ilen;
			z *= ilen;
		}
	}
	return *this;
}

NO_INLINE void noVec3::Clamp( const noVec3& min, const noVec3& max ) {
	if ( x < min.x ) {
		x = min.x;
	} else if ( x > max.x ) {
		x = max.x;
	}
	if ( y < min.y ) {
		y = min.y;
	} else if ( y > max.y ) {
		y = max.y;
	}
	if ( z < min.z ) {
		z = min.z;
	} else if ( z > max.z ) {
		z = max.z;
	}
}

NO_INLINE void noVec3::Snap( void ) {
	x = floor( x + 0.5f );
	y = floor( y + 0.5f );
	z = floor( z + 0.5f );
}

NO_INLINE void noVec3::SnapInt( void ) {
	x = float( int( x ) );
	y = float( int( y ) );
	z = float( int( z ) );
}

NO_INLINE int noVec3::GetDimension( void ) const {
	return 3;
}

NO_INLINE const noVec2 &	noVec3::ToVec2( void ) const {
	return *reinterpret_cast<const noVec2 *>(this);
}
NO_INLINE noVec2 &		noVec3::ToVec2( void ) {
	return *reinterpret_cast<noVec2 *>(this);
}

NO_INLINE const float * noVec3::ToFloatPtr( void ) const {
	return &x;
}

NO_INLINE float * noVec3::ToFloatPtr( void ) {
	return &x;
}

NO_INLINE void noVec3::Min(const noVec3 &other )
{
	x = __min(x,other.x);
	y = __min(y,other.y);
	z = __min(z,other.z);
}

NO_INLINE void noVec3::Max(const noVec3 &other )
{
	x = __max(x,other.x);
	y = __max(y,other.y);
	z = __max(z,other.z);
}

NO_INLINE int noVec3::Sign(const noVec3& v2)const
{
	if (z*v2.x > x*v2.z)
	{ 
		return anticlockwise;
	}
	else 
	{
		return clockwise;
	}
}

NO_INLINE noVec3 noVec3::Perp() const
{
	return noVec3(-z, y, x);
}

NO_INLINE noVec3 noVec3::NormalizeCopy(void) const
{
	noVec3 ret = *this;
	ret.Normalize();
	return ret;
}

NO_INLINE const noVec3 noVec3::Multiply(const noVec3& vIn) const
{
	return noVec3(x * vIn.x, y * vIn.y, z * vIn.z);
}

NO_INLINE int noVec3::IsNearlyZero( FLOAT Tolerance/*=KINDA_SMALL_NUMBER*/ ) const
{
	return noMath::Fabs(x) < Tolerance 
		&& noMath::Fabs(y) < Tolerance 
		&& noMath::Fabs(z) < Tolerance;
}

NO_INLINE void noVec3::NormalVectors( noVec3 &left, noVec3 &down ) const {
	float d;

	d = x * x + y * y;
	if ( !d ) {
		left[0] = 1;
		left[1] = 0;
		left[2] = 0;
	} else {
		d = noMath::InvSqrt( d );
		left[0] = -y * d;
		left[1] = x * d;
		left[2] = 0;
	}
	down = left.Cross( *this );
}

NO_INLINE void noVec3::OrthogonalBasis( noVec3 &left, noVec3 &up ) const {
	float l, s;

	if ( noMath::Fabs( z ) > 0.7f ) {
		l = y * y + z * z;
		s = noMath::InvSqrt( l );
		up[0] = 0;
		up[1] = z * s;
		up[2] = -y * s;
		left[0] = l * s;
		left[1] = -x * up[2];
		left[2] = x * up[1];
	}
	else {
		l = x * x + y * y;
		s = noMath::InvSqrt( l );
		left[0] = -y * s;
		left[1] = x * s;
		left[2] = 0;
		up[0] = -z * left[1];
		up[1] = z * left[0];
		up[2] = l * s;
	}
}

NO_INLINE void noVec3::ProjectOntoPlane( const noVec3 &normal, const float overBounce ) {
	float backoff;

	backoff = *this * normal;

	if ( overBounce != 1.0 ) {
		if ( backoff < 0 ) {
			backoff *= overBounce;
		} else {
			backoff /= overBounce;
		}
	}

	*this -= backoff * normal;
}

NO_INLINE bool noVec3::ProjectAlongPlane( const noVec3 &normal, const float epsilon, const float overBounce ) {
	noVec3 cross;
	float len;

	cross = this->Cross( normal ).Cross( (*this) );
	// normalize so a fixed epsilon can be used
	cross.Normalize();
	len = normal * cross;
	if ( noMath::Fabs( len ) < epsilon ) {
		return false;
	}
	cross *= overBounce * ( normal * (*this) ) / len;
	(*this) -= cross;
	return true;
}


 /** Generates a vector perpendicular to this vector (eg an 'up' vector).
            @remarks
                This method will return a vector which is perpendicular to this
                vector. There are an infinite number of possibilities but this
                method will guarantee to generate one of them. If you need more
                control you should use the Quaternion class.
        */
        NO_INLINE noVec3 noVec3::Perpendicular(void) const
        {
            static const Real fSquareZero = (Real)(1e-06 * 1e-06);

            noVec3 perp = this->Cross( noVec3(1.f, 0.f, 0.f) );

            // Check length
            if( perp.LengthSqr() < fSquareZero )
            {
                /* This vector is the Y axis multiplied by a scalar, so we have
                   to use another axis.
                */
                perp = this->Cross( noVec3(0.f, 1.0f, 0.f) );
            }
			perp.Normalize();

            return perp;
        }

class noVec4 
{
public:
	noVec4() {}
	noVec4(float x, float y, float z, float w);
	noVec4(const noVec3& v, float w);

	
	NO_INLINE void Set(float x, float y, float z, float w=0);
	NO_INLINE void SetAll( float x );
	NO_INLINE void SetAll3( float x );
	NO_INLINE void SetZero4( void );	
	float			operator[]( const int index ) const;
	float &			operator[]( const int index );
	noVec4			operator-() const;
	noVec4 &		operator=( const noVec4 &a );		// required because of a msvc 6 & 7 bug
	float			operator*( const noVec4 &a ) const;
	noVec4			operator*( const float a ) const;
	noVec4			operator/( const float a ) const;
	noVec4			operator+( const noVec4 &a ) const;
	noVec4			operator-( const noVec4 &a ) const;
	noVec4 &		operator+=( const noVec4 &a );
	noVec4 &		operator-=( const noVec4 &a );
	noVec4 &		operator/=( const noVec4 &a );
	noVec4 &		operator/=( const float a );
	noVec4 &		operator*=( const float a );

	friend noVec4	operator*( const float a, const noVec4 b );

	bool Compare( const noVec4 &a, const float epsilon ) const;	

	float			Normalize();
	float			Length( void ) const;
	
	int				GetDimension( void ) const;
	
	const noVec2 &	ToVec2( void ) const;
	noVec2 &		ToVec2( void );
	const noVec3 &	ToVec3( void ) const;
	noVec3 &		ToVec3( void );
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const noVec4	Multiply(const noVec4& vIn) const;
	void Min(const noVec4 &other );
	void Max(const noVec4 &other );
	//NO_ALIGN16( float x );
	float x ;
	float y,z,w;
};

extern noVec4 vec4_origin;
#define vec4_zero vec4_origin

NO_INLINE noVec4::noVec4( float x, float y, float z, float w ) {
	Set(x, y, z, w);
}


NO_INLINE noVec4::noVec4( const noVec3& v, float _w )	
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = _w;
}


NO_INLINE void noVec4::Set( float x, float y, float z, float w/*=0*/ ) {
	this->x = x; 
	this->y = y; 
	this->z = z; 
	this->w = w;
}

NO_INLINE void noVec4::SetAll( float a ) {
	x = y = z = w = a;
}

NO_INLINE void noVec4::SetAll3( float a ) {
	x = y = z = a;
}

NO_INLINE void noVec4::SetZero4( void ) {
	x = y = z = w = 0.f;
}

NO_INLINE float noVec4::operator[]( const int index ) const {
	return (&x)[ index ];
}

NO_INLINE float & noVec4::operator[]( const int index ) {
	return (&x)[ index ];
}

NO_INLINE noVec4 & noVec4::operator=( const noVec4 &a ) {
	x = a.x;
	y = a.y; 
	z = a.z;
	w = a.w;
	return *this;
}

ID_INLINE bool noVec4::Compare( const noVec4 &a, const float epsilon ) const {
	if ( noMath::Fabs( x - a.x ) > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( y - a.y ) > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( z - a.z ) > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( w - a.w ) > epsilon ) {
		return false;
	}

	return true;
}


NO_INLINE noVec4 noVec4::operator-() const {
	return noVec4(-x, -y, -z, -w);
}

NO_INLINE float noVec4::operator*( const noVec4 &a ) const {
	return (x * a.x) + (y * a.y) + (z * a.z) + ( w * a.w);	
}

NO_INLINE noVec4 noVec4::operator*( const float a ) const {
	return noVec4(x * a, y * a, z * a, w * a);	
}

NO_INLINE noVec4 noVec4::operator/( const float a ) const {
	return noVec4(x / a, y / a, z / a, w / a);	
}

NO_INLINE noVec4 noVec4::operator+( const noVec4 &a ) const {
	return noVec4(x + a.x, y + a.y, z + a.z, w + a.w);	
}

NO_INLINE noVec4 noVec4::operator-( const noVec4 &a ) const {
	return noVec4(x - a.x, y - a.y, z - a.z, w - a.w);		
}

NO_INLINE noVec4 & noVec4::operator+=( const noVec4 &a ) {
	x += a.x;
	y += a.y;
	z += a.z;
	w += a.w;
	return *this;
}

NO_INLINE noVec4 & noVec4::operator-=( const noVec4 &a ) {
	x -= a.x;
	y -= a.y;
	z -= a.z;
	w -= a.w;
	return *this;
}

NO_INLINE noVec4 & noVec4::operator/=( const noVec4 &a ) {
	x /= a.x;
	y /= a.y;
	z /= a.z;
	w /= a.w;
	return *this;	
}

NO_INLINE noVec4 & noVec4::operator/=( const float a ) {
	x /= a;
	y /= a;
	z /= a;
	w /= a;
	return *this;	
}

NO_INLINE noVec4 & noVec4::operator*=( const float a ) {
	x *= a;
	y *= a;
	z *= a;
	w *= a;
	return *this;
}

NO_INLINE noVec4 operator*( const float a, const noVec4 b ) {
	return noVec4( b.x * a, b.y * a, b.z * a, b.w * a );
}

NO_INLINE const noVec4 noVec4::Multiply(const noVec4& vIn) const
{
	return noVec4(x * vIn.x, y * vIn.y, z * vIn.z, w * vIn.w);
}

ID_INLINE const noVec2 &noVec4::ToVec2( void ) const {
	return *reinterpret_cast<const noVec2 *>(this);
}

ID_INLINE noVec2 &noVec4::ToVec2( void ) {
	return *reinterpret_cast<noVec2 *>(this);
}

ID_INLINE const noVec3 &noVec4::ToVec3( void ) const {
	return *reinterpret_cast<const noVec3 *>(this);
}

ID_INLINE noVec3 &noVec4::ToVec3( void ) {
	return *reinterpret_cast<noVec3 *>(this);
}


NO_INLINE const float * noVec4::ToFloatPtr( void ) const {
	return &x;
}

NO_INLINE float * noVec4::ToFloatPtr( void ) {
	return &x;
}

NO_INLINE float noVec4::Length( void ) const {
	return ( float )noMath::Sqrt( x * x + y * y + z * z + w * w);
}

NO_INLINE bool noVec3::FixDegenerateNormal( void )
{
	if ( x == 0.0f ) {
		if ( y == 0.0f ) {
			if ( z > 0.0f ) {
				if ( z != 1.0f ) {
					z = 1.0f;
					return true;
				}
			} else {
				if ( z != -1.0f ) {
					z = -1.0f;
					return true;
				}
			}
			return false;
		} else if ( z == 0.0f ) {
			if ( y > 0.0f ) {
				if ( y != 1.0f ) {
					y = 1.0f;
					return true;
				}
			} else {
				if ( y != -1.0f ) {
					y = -1.0f;
					return true;
				}
			}
			return false;
		}
	} else if ( y == 0.0f ) {
		if ( z == 0.0f ) {
			if ( x > 0.0f ) {
				if ( x != 1.0f ) {
					x = 1.0f;
					return true;
				}
			} else {
				if ( x != -1.0f ) {
					x = -1.0f;
					return true;
				}
			}
			return false;
		}
	}
	if ( noMath::Fabs( x ) == 1.0f ) {
		if ( y != 0.0f || z != 0.0f ) {
			y = z = 0.0f;
			return true;
		}
		return false;
	} else if ( noMath::Fabs( y ) == 1.0f ) {
		if ( x != 0.0f || z != 0.0f ) {
			x = z = 0.0f;
			return true;
		}
		return false;
	} else if ( noMath::Fabs( z ) == 1.0f ) {
		if ( x != 0.0f || y != 0.0f ) {
			x = y = 0.0f;
			return true;
		}
		return false;
	}
	return false;

}

NO_INLINE bool noVec3::FixDenormals( void ) {
	bool denormal = false;
	if ( fabs( x ) < 1e-30f ) {
		x = 0.0f;
		denormal = true;
	}
	if ( fabs( y ) < 1e-30f ) {
		y = 0.0f;
		denormal = true;
	}
	if ( fabs( z ) < 1e-30f ) {
		z = 0.0f;
		denormal = true;
	}
	return denormal;

}

NO_INLINE noVec3 noVec3::Cross( const noVec3 &a ) const {
	return noVec3( y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x );	 
}

NO_INLINE noVec3 & noVec3::Cross( const noVec3 &a, const noVec3 &b ) {
	x = a.y * b.z - a.z * b.y;
	y = a.z * b.x - a.x * b.z;
	z = a.x * b.y - a.y * b.x;
	return *this;
}

NO_INLINE float noVec4::Normalize( void ) {
	float sqrLen, invLen;
	sqrLen = x * x + y * y + z * z + w * w;
	invLen = noMath::InvSqrt( sqrLen );
	x *= invLen;
	y *= invLen;
	z *= invLen;
	w *= invLen;
	return invLen * sqrLen;
}

NO_INLINE void noVec4::Min(const noVec4 &other )
{
	x = __min(x,other.x);
	y = __min(y,other.y);
	z = __min(z,other.z);
	w = __min(w,other.w);
}

NO_INLINE void noVec4::Max(const noVec4 &other )
{
	x = __max(x,other.x);
	y = __max(y,other.y);
	z = __max(z,other.z);
	w = __max(w,other.w);
}


//===============================================================
//
//	idVec5 - 5D vector
//
//===============================================================

class idVec5 {
public:
	float			x;
	float			y;
	float			z;
	float			s;
	float			t;

	idVec5( void );
	explicit idVec5( const noVec3 &xyz, const noVec2 &st );
	explicit idVec5( const float x, const float y, const float z, const float s, const float t );

	float			operator[]( int index ) const;
	float &			operator[]( int index );
	idVec5 &		operator=( const noVec3 &a );

	int				GetDimension( void ) const;

	const noVec3 &	ToVec3( void ) const;
	noVec3 &		ToVec3( void );
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

	void			Lerp( const idVec5 &v1, const idVec5 &v2, const float l );
};

extern idVec5 vec5_origin;
#define vec5_zero vec5_origin

ID_INLINE idVec5::idVec5( void ) {
}

ID_INLINE idVec5::idVec5( const noVec3 &xyz, const noVec2 &st ) {
	x = xyz.x;
	y = xyz.y;
	z = xyz.z;
	s = st[0];
	t = st[1];
}

ID_INLINE idVec5::idVec5( const float x, const float y, const float z, const float s, const float t ) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->s = s;
	this->t = t;
}

ID_INLINE float idVec5::operator[]( int index ) const {
	return ( &x )[ index ];
}

ID_INLINE float& idVec5::operator[]( int index ) {
	return ( &x )[ index ];
}

ID_INLINE idVec5 &idVec5::operator=( const noVec3 &a ) { 
	x = a.x;
	y = a.y;
	z = a.z;
	s = t = 0;
	return *this;
}

ID_INLINE int idVec5::GetDimension( void ) const {
	return 5;
}

ID_INLINE const noVec3 &idVec5::ToVec3( void ) const {
	return *reinterpret_cast<const noVec3 *>(this);
}

ID_INLINE noVec3 &idVec5::ToVec3( void ) {
	return *reinterpret_cast<noVec3 *>(this);
}

ID_INLINE const float *idVec5::ToFloatPtr( void ) const {
	return &x;
}

ID_INLINE float *idVec5::ToFloatPtr( void ) {
	return &x;
}


//===============================================================
//
//	idVec6 - 6D vector
//
//===============================================================

class idVec6 {
public:	
	idVec6( void );
	explicit idVec6( const float *a );
	explicit idVec6( const float a1, const float a2, const float a3, const float a4, const float a5, const float a6 );

	void 			Set( const float a1, const float a2, const float a3, const float a4, const float a5, const float a6 );
	void			Zero( void );

	float			operator[]( const int index ) const;
	float &			operator[]( const int index );
	idVec6			operator-() const;
	idVec6			operator*( const float a ) const;
	idVec6			operator/( const float a ) const;
	float			operator*( const idVec6 &a ) const;
	idVec6			operator-( const idVec6 &a ) const;
	idVec6			operator+( const idVec6 &a ) const;
	idVec6 &		operator*=( const float a );
	idVec6 &		operator/=( const float a );
	idVec6 &		operator+=( const idVec6 &a );
	idVec6 &		operator-=( const idVec6 &a );

	friend idVec6	operator*( const float a, const idVec6 b );

	bool			Compare( const idVec6 &a ) const;							// exact compare, no epsilon
	bool			Compare( const idVec6 &a, const float epsilon ) const;		// compare with epsilon
	bool			operator==(	const idVec6 &a ) const;						// exact compare, no epsilon
	bool			operator!=(	const idVec6 &a ) const;						// exact compare, no epsilon

	float			Length( void ) const;
	float			LengthSqr( void ) const;
	float			Normalize( void );			// returns length
	float			NormalizeFast( void );		// returns length

	int				GetDimension( void ) const;

	const noVec3 &	SubVec3( int index ) const;
	noVec3 &		SubVec3( int index );
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

private:
	float			p[6];
};

extern idVec6 vec6_origin;
#define vec6_zero vec6_origin
extern idVec6 vec6_infinity;

ID_INLINE idVec6::idVec6( void ) {
}

ID_INLINE idVec6::idVec6( const float *a ) {
	memcpy( p, a, 6 * sizeof( float ) );
}

ID_INLINE idVec6::idVec6( const float a1, const float a2, const float a3, const float a4, const float a5, const float a6 ) {
	p[0] = a1;
	p[1] = a2;
	p[2] = a3;
	p[3] = a4;
	p[4] = a5;
	p[5] = a6;
}

ID_INLINE idVec6 idVec6::operator-() const {
	return idVec6( -p[0], -p[1], -p[2], -p[3], -p[4], -p[5] );
}

ID_INLINE float idVec6::operator[]( const int index ) const {
	return p[index];
}

ID_INLINE float &idVec6::operator[]( const int index ) {
	return p[index];
}

ID_INLINE idVec6 idVec6::operator*( const float a ) const {
	return idVec6( p[0]*a, p[1]*a, p[2]*a, p[3]*a, p[4]*a, p[5]*a );
}

ID_INLINE float idVec6::operator*( const idVec6 &a ) const {
	return p[0] * a[0] + p[1] * a[1] + p[2] * a[2] + p[3] * a[3] + p[4] * a[4] + p[5] * a[5];
}

ID_INLINE idVec6 idVec6::operator/( const float a ) const {
	float inva;

	assert( a != 0.0f );
	inva = 1.0f / a;
	return idVec6( p[0]*inva, p[1]*inva, p[2]*inva, p[3]*inva, p[4]*inva, p[5]*inva );
}

ID_INLINE idVec6 idVec6::operator+( const idVec6 &a ) const {
	return idVec6( p[0] + a[0], p[1] + a[1], p[2] + a[2], p[3] + a[3], p[4] + a[4], p[5] + a[5] );
}

ID_INLINE idVec6 idVec6::operator-( const idVec6 &a ) const {
	return idVec6( p[0] - a[0], p[1] - a[1], p[2] - a[2], p[3] - a[3], p[4] - a[4], p[5] - a[5] );
}

ID_INLINE idVec6 &idVec6::operator*=( const float a ) {
	p[0] *= a;
	p[1] *= a;
	p[2] *= a;
	p[3] *= a;
	p[4] *= a;
	p[5] *= a;
	return *this;
}

ID_INLINE idVec6 &idVec6::operator/=( const float a ) {
	float inva;

	assert( a != 0.0f );
	inva = 1.0f / a;
	p[0] *= inva;
	p[1] *= inva;
	p[2] *= inva;
	p[3] *= inva;
	p[4] *= inva;
	p[5] *= inva;
	return *this;
}

ID_INLINE idVec6 &idVec6::operator+=( const idVec6 &a ) {
	p[0] += a[0];
	p[1] += a[1];
	p[2] += a[2];
	p[3] += a[3];
	p[4] += a[4];
	p[5] += a[5];
	return *this;
}

ID_INLINE idVec6 &idVec6::operator-=( const idVec6 &a ) {
	p[0] -= a[0];
	p[1] -= a[1];
	p[2] -= a[2];
	p[3] -= a[3];
	p[4] -= a[4];
	p[5] -= a[5];
	return *this;
}

ID_INLINE idVec6 operator*( const float a, const idVec6 b ) {
	return b * a;
}

ID_INLINE bool idVec6::Compare( const idVec6 &a ) const {
	return ( ( p[0] == a[0] ) && ( p[1] == a[1] ) && ( p[2] == a[2] ) &&
		( p[3] == a[3] ) && ( p[4] == a[4] ) && ( p[5] == a[5] ) );
}

ID_INLINE bool idVec6::Compare( const idVec6 &a, const float epsilon ) const {
	if ( noMath::Fabs( p[0] - a[0] ) > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( p[1] - a[1] ) > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( p[2] - a[2] ) > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( p[3] - a[3] ) > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( p[4] - a[4] ) > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( p[5] - a[5] ) > epsilon ) {
		return false;
	}

	return true;
}

ID_INLINE bool idVec6::operator==( const idVec6 &a ) const {
	return Compare( a );
}

ID_INLINE bool idVec6::operator!=( const idVec6 &a ) const {
	return !Compare( a );
}

ID_INLINE void idVec6::Set( const float a1, const float a2, const float a3, const float a4, const float a5, const float a6 ) {
	p[0] = a1;
	p[1] = a2;
	p[2] = a3;
	p[3] = a4;
	p[4] = a5;
	p[5] = a6;
}

ID_INLINE void idVec6::Zero( void ) {
	p[0] = p[1] = p[2] = p[3] = p[4] = p[5] = 0.0f;
}

ID_INLINE float idVec6::Length( void ) const {
	return ( float )noMath::Sqrt( p[0] * p[0] + p[1] * p[1] + p[2] * p[2] + p[3] * p[3] + p[4] * p[4] + p[5] * p[5] );
}

ID_INLINE float idVec6::LengthSqr( void ) const {
	return ( p[0] * p[0] + p[1] * p[1] + p[2] * p[2] + p[3] * p[3] + p[4] * p[4] + p[5] * p[5] );
}

ID_INLINE float idVec6::Normalize( void ) {
	float sqrLength, invLength;

	sqrLength = p[0] * p[0] + p[1] * p[1] + p[2] * p[2] + p[3] * p[3] + p[4] * p[4] + p[5] * p[5];
	invLength = noMath::InvSqrt( sqrLength );
	p[0] *= invLength;
	p[1] *= invLength;
	p[2] *= invLength;
	p[3] *= invLength;
	p[4] *= invLength;
	p[5] *= invLength;
	return invLength * sqrLength;
}

ID_INLINE float idVec6::NormalizeFast( void ) {
	float sqrLength, invLength;

	sqrLength = p[0] * p[0] + p[1] * p[1] + p[2] * p[2] + p[3] * p[3] + p[4] * p[4] + p[5] * p[5];
	invLength = noMath::RSqrt( sqrLength );
	p[0] *= invLength;
	p[1] *= invLength;
	p[2] *= invLength;
	p[3] *= invLength;
	p[4] *= invLength;
	p[5] *= invLength;
	return invLength * sqrLength;
}

ID_INLINE int idVec6::GetDimension( void ) const {
	return 6;
}

ID_INLINE const noVec3 &idVec6::SubVec3( int index ) const {
	return *reinterpret_cast<const noVec3 *>(p + index * 3);
}

ID_INLINE noVec3 &idVec6::SubVec3( int index ) {
	return *reinterpret_cast<noVec3 *>(p + index * 3);
}

ID_INLINE const float *idVec6::ToFloatPtr( void ) const {
	return p;
}

ID_INLINE float *idVec6::ToFloatPtr( void ) {
	return p;
}


//===============================================================
//
//	idVecX - arbitrary sized vector
//
//  The vector lives on 16 byte aligned and 16 byte padded memory.
//
//	NOTE: due to the temporary memory pool idVecX cannot be used by multiple threads
//
//===============================================================

#define VECX_MAX_TEMP		1024
#define VECX_QUAD( x )		( ( ( ( x ) + 3 ) & ~3 ) * sizeof( float ) )
#define VECX_CLEAREND()		int s = size; while( s < ( ( s + 3) & ~3 ) ) { p[s++] = 0.0f; }
#define VECX_ALLOCA( n )	( (float *) _alloca16( VECX_QUAD( n ) ) )
#define VECX_SIMD

class idVecX {
	friend class idMatX;

public:	
	idVecX( void );
	explicit idVecX( int length );
	explicit idVecX( int length, float *data );
	~idVecX( void );

	float			operator[]( const int index ) const;
	float &			operator[]( const int index );
	idVecX			operator-() const;
	idVecX &		operator=( const idVecX &a );
	idVecX			operator*( const float a ) const;
	idVecX			operator/( const float a ) const;
	float			operator*( const idVecX &a ) const;
	idVecX			operator-( const idVecX &a ) const;
	idVecX			operator+( const idVecX &a ) const;
	idVecX &		operator*=( const float a );
	idVecX &		operator/=( const float a );
	idVecX &		operator+=( const idVecX &a );
	idVecX &		operator-=( const idVecX &a );

	friend idVecX	operator*( const float a, const idVecX b );

	bool			Compare( const idVecX &a ) const;							// exact compare, no epsilon
	bool			Compare( const idVecX &a, const float epsilon ) const;		// compare with epsilon
	bool			operator==(	const idVecX &a ) const;						// exact compare, no epsilon
	bool			operator!=(	const idVecX &a ) const;						// exact compare, no epsilon

	void			SetSize( int size );
	void			ChangeSize( int size, bool makeZero = false );
	int				GetSize( void ) const { return size; }
	void			SetData( int length, float *data );
	void			Zero( void );
	void			Zero( int length );
	void			Random( int seed, float l = 0.0f, float u = 1.0f );
	void			Random( int length, int seed, float l = 0.0f, float u = 1.0f );
	void			Negate( void );
	void			Clamp( float min, float max );
	idVecX &		SwapElements( int e1, int e2 );

	float			Length( void ) const;
	float			LengthSqr( void ) const;
	idVecX			Normalize( void ) const;
	float			NormalizeSelf( void );

	int				GetDimension( void ) const;

	const noVec3 &	SubVec3( int index ) const;
	noVec3 &		SubVec3( int index );
	const idVec6 &	SubVec6( int index ) const;
	idVec6 &		SubVec6( int index );
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

private:
	int				size;					// size of the vector
	int				alloced;				// if -1 p points to data set with SetData
	float *			p;						// memory the vector is stored

	static float	temp[VECX_MAX_TEMP+4];	// used to store intermediate results
	static float *	tempPtr;				// pointer to 16 byte aligned temporary memory
	static int		tempIndex;				// index into memory pool, wraps around

private:
	void			SetTempSize( int size );
};


ID_INLINE idVecX::idVecX( void ) {
	size = alloced = 0;
	p = NULL;
}

ID_INLINE idVecX::idVecX( int length ) {
	size = alloced = 0;
	p = NULL;
	SetSize( length );
}

ID_INLINE idVecX::idVecX( int length, float *data ) {
	size = alloced = 0;
	p = NULL;
	SetData( length, data );
}

ID_INLINE idVecX::~idVecX( void ) {
	// if not temp memory
	if ( p && ( p < idVecX::tempPtr || p >= idVecX::tempPtr + VECX_MAX_TEMP ) && alloced != -1 ) {
		Mem_Free16( p );
	}
}

ID_INLINE float idVecX::operator[]( const int index ) const {
	assert( index >= 0 && index < size );
	return p[index];
}

ID_INLINE float &idVecX::operator[]( const int index ) {
	assert( index >= 0 && index < size );
	return p[index];
}

ID_INLINE idVecX idVecX::operator-() const {
	int i;
	idVecX m;

	m.SetTempSize( size );
	for ( i = 0; i < size; i++ ) {
		m.p[i] = -p[i];
	}
	return m;
}

ID_INLINE idVecX &idVecX::operator=( const idVecX &a ) { 
	SetSize( a.size );
#ifdef VECX_SIMD
	SIMDProcessor->Copy16( p, a.p, a.size );
#else
	memcpy( p, a.p, a.size * sizeof( float ) );
#endif
	idVecX::tempIndex = 0;
	return *this;
}

ID_INLINE idVecX idVecX::operator+( const idVecX &a ) const {
	idVecX m;

	assert( size == a.size );
	m.SetTempSize( size );
#ifdef VECX_SIMD
	SIMDProcessor->Add16( m.p, p, a.p, size );
#else
	int i;
	for ( i = 0; i < size; i++ ) {
		m.p[i] = p[i] + a.p[i];
	}
#endif
	return m;
}

ID_INLINE idVecX idVecX::operator-( const idVecX &a ) const {
	idVecX m;

	assert( size == a.size );
	m.SetTempSize( size );
#ifdef VECX_SIMD
	SIMDProcessor->Sub16( m.p, p, a.p, size );
#else
	int i;
	for ( i = 0; i < size; i++ ) {
		m.p[i] = p[i] - a.p[i];
	}
#endif
	return m;
}

ID_INLINE idVecX &idVecX::operator+=( const idVecX &a ) {
	assert( size == a.size );
#ifdef VECX_SIMD
	SIMDProcessor->AddAssign16( p, a.p, size );
#else
	int i;
	for ( i = 0; i < size; i++ ) {
		p[i] += a.p[i];
	}
#endif
	idVecX::tempIndex = 0;
	return *this;
}

ID_INLINE idVecX &idVecX::operator-=( const idVecX &a ) {
	assert( size == a.size );
#ifdef VECX_SIMD
	SIMDProcessor->SubAssign16( p, a.p, size );
#else
	int i;
	for ( i = 0; i < size; i++ ) {
		p[i] -= a.p[i];
	}
#endif
	idVecX::tempIndex = 0;
	return *this;
}

ID_INLINE idVecX idVecX::operator*( const float a ) const {
	idVecX m;

	m.SetTempSize( size );
#ifdef VECX_SIMD
	SIMDProcessor->Mul16( m.p, p, a, size );
#else
	int i;
	for ( i = 0; i < size; i++ ) {
		m.p[i] = p[i] * a;
	}
#endif
	return m;
}

ID_INLINE idVecX &idVecX::operator*=( const float a ) {
#ifdef VECX_SIMD
	SIMDProcessor->MulAssign16( p, a, size );
#else
	int i;
	for ( i = 0; i < size; i++ ) {
		p[i] *= a;
	}
#endif
	return *this;
}

ID_INLINE idVecX idVecX::operator/( const float a ) const {
	assert( a != 0.0f );
	return (*this) * ( 1.0f / a );
}

ID_INLINE idVecX &idVecX::operator/=( const float a ) {
	assert( a != 0.0f );
	(*this) *= ( 1.0f / a );
	return *this;
}

ID_INLINE idVecX operator*( const float a, const idVecX b ) {
	return b * a;
}

ID_INLINE float idVecX::operator*( const idVecX &a ) const {
	int i;
	float sum = 0.0f;

	assert( size == a.size );
	for ( i = 0; i < size; i++ ) {
		sum += p[i] * a.p[i];
	}
	return sum;
}

ID_INLINE bool idVecX::Compare( const idVecX &a ) const {
	int i;

	assert( size == a.size );
	for ( i = 0; i < size; i++ ) {
		if ( p[i] != a.p[i] ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idVecX::Compare( const idVecX &a, const float epsilon ) const {
	int i;

	assert( size == a.size );
	for ( i = 0; i < size; i++ ) {
		if ( noMath::Fabs( p[i] - a.p[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idVecX::operator==( const idVecX &a ) const {
	return Compare( a );
}

ID_INLINE bool idVecX::operator!=( const idVecX &a ) const {
	return !Compare( a );
}

ID_INLINE void idVecX::SetSize( int newSize ) {
	int alloc = ( newSize + 3 ) & ~3;
	if ( alloc > alloced && alloced != -1 ) {
		if ( p ) {
			Mem_Free16( p );
		}
		p = (float *) Mem_Alloc16( alloc * sizeof( float ) );
		alloced = alloc;
	}
	size = newSize;
	VECX_CLEAREND();
}

ID_INLINE void idVecX::ChangeSize( int newSize, bool makeZero ) {
	int alloc = ( newSize + 3 ) & ~3;
	if ( alloc > alloced && alloced != -1 ) {
		float *oldVec = p;
		p = (float *) Mem_Alloc16( alloc * sizeof( float ) );
		alloced = alloc;
		if ( oldVec ) {
			for ( int i = 0; i < size; i++ ) {
				p[i] = oldVec[i];
			}
			Mem_Free16( oldVec );
		}
		if ( makeZero ) {
			// zero any new elements
			for ( int i = size; i < newSize; i++ ) {
				p[i] = 0.0f;
			}
		}
	}
	size = newSize;
	VECX_CLEAREND();
}

ID_INLINE void idVecX::SetTempSize( int newSize ) {

	size = newSize;
	alloced = ( newSize + 3 ) & ~3;
	assert( alloced < VECX_MAX_TEMP );
	if ( idVecX::tempIndex + alloced > VECX_MAX_TEMP ) {
		idVecX::tempIndex = 0;
	}
	p = idVecX::tempPtr + idVecX::tempIndex;
	idVecX::tempIndex += alloced;
	VECX_CLEAREND();
}

ID_INLINE void idVecX::SetData( int length, float *data ) {
	if ( p && ( p < idVecX::tempPtr || p >= idVecX::tempPtr + VECX_MAX_TEMP ) && alloced != -1 ) {
		Mem_Free16( p );
	}
	assert( ( ( (int) data ) & 15 ) == 0 ); // data must be 16 byte aligned
	p = data;
	size = length;
	alloced = -1;
	VECX_CLEAREND();
}

ID_INLINE void idVecX::Zero( void ) {
#ifdef VECX_SIMD
	SIMDProcessor->Zero16( p, size );
#else
	memset( p, 0, size * sizeof( float ) );
#endif
}

ID_INLINE void idVecX::Zero( int length ) {
	SetSize( length );
#ifdef VECX_SIMD
	SIMDProcessor->Zero16( p, length );
#else
	memset( p, 0, size * sizeof( float ) );
#endif
}

ID_INLINE void idVecX::Random( int seed, float l, float u ) {
	int i;
	float c;
	idRandom rnd( seed );

	c = u - l;
	for ( i = 0; i < size; i++ ) {
		p[i] = l + rnd.RandomFloat() * c;
	}
}

ID_INLINE void idVecX::Random( int length, int seed, float l, float u ) {
	int i;
	float c;
	idRandom rnd( seed );

	SetSize( length );
	c = u - l;
	for ( i = 0; i < size; i++ ) {
		p[i] = l + rnd.RandomFloat() * c;
	}
}

ID_INLINE void idVecX::Negate( void ) {
#ifdef VECX_SIMD
	SIMDProcessor->Negate16( p, size );
#else
	int i;
	for ( i = 0; i < size; i++ ) {
		p[i] = -p[i];
	}
#endif
}

ID_INLINE void idVecX::Clamp( float min, float max ) {
	int i;
	for ( i = 0; i < size; i++ ) {
		if ( p[i] < min ) {
			p[i] = min;
		} else if ( p[i] > max ) {
			p[i] = max;
		}
	}
}

ID_INLINE idVecX &idVecX::SwapElements( int e1, int e2 ) {
	float tmp;
	tmp = p[e1];
	p[e1] = p[e2];
	p[e2] = tmp;
	return *this;
}

ID_INLINE float idVecX::Length( void ) const {
	int i;
	float sum = 0.0f;

	for ( i = 0; i < size; i++ ) {
		sum += p[i] * p[i];
	}
	return noMath::Sqrt( sum );
}

ID_INLINE float idVecX::LengthSqr( void ) const {
	int i;
	float sum = 0.0f;

	for ( i = 0; i < size; i++ ) {
		sum += p[i] * p[i];
	}
	return sum;
}

ID_INLINE idVecX idVecX::Normalize( void ) const {
	int i;
	idVecX m;
	float invSqrt, sum = 0.0f;

	m.SetTempSize( size );
	for ( i = 0; i < size; i++ ) {
		sum += p[i] * p[i];
	}
	invSqrt = noMath::InvSqrt( sum );
	for ( i = 0; i < size; i++ ) {
		m.p[i] = p[i] * invSqrt;
	}
	return m;
}

ID_INLINE float idVecX::NormalizeSelf( void ) {
	float invSqrt, sum = 0.0f;
	int i;
	for ( i = 0; i < size; i++ ) {
		sum += p[i] * p[i];
	}
	invSqrt = noMath::InvSqrt( sum );
	for ( i = 0; i < size; i++ ) {
		p[i] *= invSqrt;
	}
	return invSqrt * sum;
}

ID_INLINE int idVecX::GetDimension( void ) const {
	return size;
}

ID_INLINE noVec3 &idVecX::SubVec3( int index ) {
	assert( index >= 0 && index * 3 + 3 <= size );
	return *reinterpret_cast<noVec3 *>(p + index * 3);
}

ID_INLINE const noVec3 &idVecX::SubVec3( int index ) const {
	assert( index >= 0 && index * 3 + 3 <= size );
	return *reinterpret_cast<const noVec3 *>(p + index * 3);
}

ID_INLINE idVec6 &idVecX::SubVec6( int index ) {
	assert( index >= 0 && index * 6 + 6 <= size );
	return *reinterpret_cast<idVec6 *>(p + index * 6);
}

ID_INLINE const idVec6 &idVecX::SubVec6( int index ) const {
	assert( index >= 0 && index * 6 + 6 <= size );
	return *reinterpret_cast<const idVec6 *>(p + index * 6);
}

ID_INLINE const float *idVecX::ToFloatPtr( void ) const {
	return p;
}

ID_INLINE float *idVecX::ToFloatPtr( void ) {
	return p;
}

//===============================================================
//
//	idPolar3
//
//===============================================================

class idPolar3 {
public:	
	float			radius, theta, phi;

					idPolar3( void );
					explicit idPolar3( const float radius, const float theta, const float phi );

	void 			Set( const float radius, const float theta, const float phi );

	float			operator[]( const int index ) const;
	float &			operator[]( const int index );
	idPolar3		operator-() const;
	idPolar3 &		operator=( const idPolar3 &a );

	noVec3			ToVec3( void ) const;
};

ID_INLINE idPolar3::idPolar3( void ) {
}

ID_INLINE idPolar3::idPolar3( const float radius, const float theta, const float phi ) {
	assert( radius > 0 );
	this->radius = radius;
	this->theta = theta;
	this->phi = phi;
}
	
ID_INLINE void idPolar3::Set( const float radius, const float theta, const float phi ) {
	assert( radius > 0 );
	this->radius = radius;
	this->theta = theta;
	this->phi = phi;
}

ID_INLINE float idPolar3::operator[]( const int index ) const {
	return ( &radius )[ index ];
}

ID_INLINE float &idPolar3::operator[]( const int index ) {
	return ( &radius )[ index ];
}

ID_INLINE idPolar3 idPolar3::operator-() const {
	return idPolar3( radius, -theta, -phi );
}

ID_INLINE idPolar3 &idPolar3::operator=( const idPolar3 &a ) { 
	radius = a.radius;
	theta = a.theta;
	phi = a.phi;
	return *this;
}

ID_INLINE noVec3 idPolar3::ToVec3( void ) const {
	float sp, cp, st, ct;
	noMath::SinCos( phi, sp, cp );
	noMath::SinCos( theta, st, ct );
 	return noVec3( cp * radius * ct, cp * radius * st, radius * sp );
}


/*
===============================================================================

	Old 3D vector macros, should no longer be used.

===============================================================================
*/

#define DotProduct( a, b)			((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VectorSubtract2( a, b, c )	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd( a, b, c )		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define	VectorScale( v, s, o )		((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define	VectorMA( v, s, b, o )		((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))
#define VectorCopy( a, b )			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])

#endif /* !__MATH_VECTOR_H__ */
