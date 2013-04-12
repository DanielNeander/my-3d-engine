#pragma once 
#ifndef MATH_NOQUATERNION_H
#define MATH_NOQUATERNION_H

//#include <Core/SysCommon.h>
#include "Math.h"
class noVec3;
class noMat3;
class noMat4;
class idCQuat;
class noAngles;
class noRotation;


class noQuat {
public:
	float x, y, z, w;

	noQuat( void );
	noQuat( float x, float y, float z, float w );

	void Set( float x, float y, float z, float w );

	float			operator[]( int index ) const;
	float &			operator[]( int index );
	noQuat			operator-() const;
	noQuat &		operator=( const noQuat &a );
	noQuat			operator+( const noQuat &a ) const;
	noQuat &		operator+=( const noQuat &a );
	noQuat			operator-( const noQuat &a ) const;
	noQuat &		operator-=( const noQuat &a );
	noQuat			operator*( const noQuat &a ) const;
	noVec3			operator*( const noVec3 &a ) const;
	noQuat			operator*( float a ) const;
	noQuat &		operator*=( const noQuat &a );
	noQuat &		operator*=( float a );
	float			operator| (const noQuat& q) const;

	friend noQuat	operator*( const float a, const noQuat &b );
	friend noVec3	operator*( const noVec3& a, const noQuat &b );

	bool			Compare( const noQuat &a ) const;						// exact compare, no epsilon
	bool			Compare( const noQuat &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==(	const noQuat &a ) const;					// exact compare, no epsilon
	bool			operator!=(	const noQuat &a ) const;					// exact compare, no ep

	noQuat			Inverse( void ) const;
	float			Length( void ) const;
	noQuat &		Normalize( void );

	

	noAngles		ToAngles( void ) const;
	noRotation		ToRotation( void ) const;
	noMat3			ToMat3( void ) const;
	noMat4			ToMat4( void ) const;
	noVec3			ToAngularVelocity( void ) const;
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
				
	void			FromAngleAxis(float angle, const noVec3& axis); // Error!

	static const noQuat	Slerp( const noQuat &from, const noQuat &to, float t );

	NO_INLINE static const noQuat Lerp(const float r, const noQuat &v1, const noQuat &v2)
	{
		return v1*(1.0f-r) + v2*r;
	}
	const noVec3 RotateVec3(const noVec3& v);
	float CalcW( void ) const;
	idCQuat ToCQuat( void ) const;
	static noQuat Identity;
};

NO_INLINE noQuat::noQuat( void ) {
}

NO_INLINE noQuat::noQuat( float x, float y, float z, float w ) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

NO_INLINE float noQuat::operator[]( int index ) const {
	assert( ( index >= 0 ) && ( index < 4 ) );
	return ( &x )[ index ];
}

NO_INLINE float& noQuat::operator[]( int index ) {
	assert( ( index >= 0 ) && ( index < 4 ) );
	return ( &x )[ index ];
}

NO_INLINE noQuat noQuat::operator-() const {
	return noQuat( -x, -y, -z, -w );
}

NO_INLINE noQuat &noQuat::operator=( const noQuat &a ) {
	x = a.x;
	y = a.y;
	z = a.z;
	w = a.w;

	return *this;
}

NO_INLINE noQuat noQuat::operator+( const noQuat &a ) const {
	return noQuat( x + a.x, y + a.y, z + a.z, w + a.w );
}

NO_INLINE noQuat& noQuat::operator+=( const noQuat &a ) {
	x += a.x;
	y += a.y;
	z += a.z;
	w += a.w;

	return *this;
}

NO_INLINE noQuat noQuat::operator-( const noQuat &a ) const {
	return noQuat( x - a.x, y - a.y, z - a.z, w - a.w );
}

NO_INLINE noQuat& noQuat::operator-=( const noQuat &a ) {
	x -= a.x;
	y -= a.y;
	z -= a.z;
	w -= a.w;

	return *this;
}

NO_INLINE noQuat noQuat::operator*( float a ) const {
	return noQuat( x * a, y * a, z * a, w * a );
}

NO_INLINE noQuat operator*( const float a, const noQuat &b ) {
	return b * a;
}

NO_INLINE noQuat noQuat::operator*( const noQuat &a ) const {
	return noQuat(	w*a.x + x*a.w + y*a.z - z*a.y,
		w*a.y + y*a.w + z*a.x - x*a.z,
		w*a.z + z*a.w + x*a.y - y*a.x,
		w*a.w - x*a.x - y*a.y - z*a.z );
}

NO_INLINE noVec3 noQuat::operator*( const noVec3 &a ) const {
#if 0
	// it's faster to do the conversion to a 3x3 matrix and multiply the vector by this 3x3 matrix
	return ( ToMat3() * a );
#else
	// result = this->Inverse() * noQuat( a.x, a.y, a.z, 0.0f ) * (*this)
	float xxzz = x*x - z*z;
	float wwyy = w*w - y*y;

	float xw2 = x*w*2.0f;
	float xy2 = x*y*2.0f;
	float xz2 = x*z*2.0f;
	float yw2 = y*w*2.0f;
	float yz2 = y*z*2.0f;
	float zw2 = z*w*2.0f;

	return noVec3(
		(xxzz + wwyy)*a.x		+ (xy2 + zw2)*a.y		+ (xz2 - yw2)*a.z,
		(xy2 - zw2)*a.x			+ (y*y+w*w-x*x-z*z)*a.y	+ (yz2 + xw2)*a.z,
		(xz2 + yw2)*a.x			+ (yz2 - xw2)*a.y		+ (wwyy - xxzz)*a.z
		);
#endif
}

NO_INLINE noQuat& noQuat::operator*=( const noQuat &a ) {
	*this = *this * a;

	return *this;
}

NO_INLINE noQuat& noQuat::operator*=( float a ) {
	x *= a;
	y *= a;
	z *= a;
	w *= a;

	return *this;
}

NO_INLINE float noQuat::operator| (const noQuat& q) const {
	return x *q.x + y*q.y + z*q.z + w * q.w;
}



NO_INLINE bool noQuat::Compare( const noQuat &a ) const {
	return ( ( x == a.x ) && ( y == a.y ) && ( z == a.z ) && ( w == a.w ) );
}

NO_INLINE bool noQuat::Compare( const noQuat &a, const float epsilon ) const {
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

NO_INLINE bool noQuat::operator==( const noQuat &a ) const {
	return Compare( a );
}

NO_INLINE bool noQuat::operator!=( const noQuat &a ) const {
	return !Compare( a );
}

NO_INLINE void noQuat::Set( float x, float y, float z, float w ) {
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

NO_INLINE noQuat noQuat::Inverse( void ) const {
	return noQuat( -x, -y, -z, w );
}

NO_INLINE float noQuat::Length( void ) const {
	float len;

	len = x * x + y * y + z * z + w * w;
	return noMath::Sqrt( len );
}

NO_INLINE noQuat & noQuat::Normalize( void ) {
	float len;
	float ilength;

	len = this->Length();
	if ( len ) {
		ilength = 1 / len;
		x *= ilength;
		y *= ilength;
		z *= ilength;
		w *= ilength;
	}
	return *this;

}

NO_INLINE float noQuat::CalcW( void ) const {
	// take the absolute value because floating point rounding may cause the dot of x,y,z to be larger than 1
	return sqrt( fabs( 1.0f - ( x * x + y * y + z * z ) ) );
}

NO_INLINE const float * noQuat::ToFloatPtr( void ) const
{
	return &x;

}

NO_INLINE float * noQuat::ToFloatPtr( void )
{
	return &x;
}

/*
===============================================================================

	Compressed quaternion

===============================================================================
*/

class idCQuat {
public:
	float			x;
	float			y;
	float			z;

					idCQuat( void );
					idCQuat( float x, float y, float z );

	void 			Set( float x, float y, float z );

	float			operator[]( int index ) const;
	float &			operator[]( int index );

	bool			Compare( const idCQuat &a ) const;						// exact compare, no epsilon
	bool			Compare( const idCQuat &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==(	const idCQuat &a ) const;					// exact compare, no epsilon
	bool			operator!=(	const idCQuat &a ) const;					// exact compare, no epsilon

	int				GetDimension( void ) const;

	noAngles		ToAngles( void ) const;
	noRotation		ToRotation( void ) const;
	noMat3			ToMat3( void ) const;
	noMat4			ToMat4( void ) const;
	noQuat			ToQuat( void ) const;
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;
};

ID_INLINE idCQuat::idCQuat( void ) {
}

ID_INLINE idCQuat::idCQuat( float x, float y, float z ) {
	this->x = x;
	this->y = y;
	this->z = z;
}

ID_INLINE void idCQuat::Set( float x, float y, float z ) {
	this->x = x;
	this->y = y;
	this->z = z;
}

ID_INLINE float idCQuat::operator[]( int index ) const {
	assert( ( index >= 0 ) && ( index < 3 ) );
	return ( &x )[ index ];
}

ID_INLINE float& idCQuat::operator[]( int index ) {
	assert( ( index >= 0 ) && ( index < 3 ) );
	return ( &x )[ index ];
}

ID_INLINE bool idCQuat::Compare( const idCQuat &a ) const {
	return ( ( x == a.x ) && ( y == a.y ) && ( z == a.z ) );
}

ID_INLINE bool idCQuat::Compare( const idCQuat &a, const float epsilon ) const {
	if ( noMath::Fabs( x - a.x ) > epsilon ) {
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

ID_INLINE bool idCQuat::operator==( const idCQuat &a ) const {
	return Compare( a );
}

ID_INLINE bool idCQuat::operator!=( const idCQuat &a ) const {
	return !Compare( a );
}

ID_INLINE int idCQuat::GetDimension( void ) const {
	return 3;
}

ID_INLINE noQuat idCQuat::ToQuat( void ) const {
	// take the absolute value because floating point rounding may cause the dot of x,y,z to be larger than 1
	return noQuat( x, y, z, sqrt( fabs( 1.0f - ( x * x + y * y + z * z ) ) ) );
}

ID_INLINE const float *idCQuat::ToFloatPtr( void ) const {
	return &x;
}

ID_INLINE float *idCQuat::ToFloatPtr( void ) {
	return &x;
}


#endif