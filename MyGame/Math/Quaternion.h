#pragma once 
#ifndef MATH_NOQUATERNION_H
#define MATH_NOQUATERNION_H

#include <Core/SysCommon.h>
#include <Math/Math.h>
class noVec3;
class noMat3;
class noCQuat;


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

	friend noQuat	operator*( const float a, const noQuat &b );
	friend noVec3	operator*( const noVec3& a, const noQuat &b );

	bool			Compare( const noQuat &a ) const;						// exact compare, no epsilon
	bool			Compare( const noQuat &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==(	const noQuat &a ) const;					// exact compare, no epsilon
	bool			operator!=(	const noQuat &a ) const;					// exact compare, no ep

	noQuat			Inverse( void ) const;
	float			Length( void ) const;
	noQuat &		Normalize( void );

	static noQuat &		Slerp( const noQuat &from, const noQuat &to, float t );

	NO_INLINE static	noQuat& Lerp(const float r, const noQuat &v1, const noQuat &v2)
	{
		return v1*(1.0f-r) + v2*r;
	}

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

NO_INLINE noQuat noQuat::operator*( const noQuat& a ) const {
	return noQuat( a );
}

NO_INLINE noQuat noQuat::operator*( float a ) const {
	return noQuat( x * a, y * a, z * a, w * a );
}

NO_INLINE noQuat operator*( const float a, const noQuat &b ) {
	return b * a;
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



#endif