#pragma once 
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
#ifndef __MATH_VECTOR_H__
#define __MATH_VECTOR_H__

#include <Core/SysCommon.h>
#include <Math/Math.h>

/*
===============================================================================

  Vector classes

===============================================================================
*/



#define VECTOR_EPSILON		0.001f

class noPolar3;

class noVec2 {
public:
	float x,y;

	noVec2();
	explicit noVec2( const float x, const float y );

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

	int				GetDimension( void ) const;

	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

	void			Lerp( const noVec2 &v1, const noVec2 &v2, const float l );	
};

extern noVec2 vec2_origin;
#define vec2_zero vec2_origin

NO_INLINE noVec2::noVec2( void ) {}

NO_INLINE noVec2::noVec2( const float x, const float y) {
	this->x = x;
	this->y = y;
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


class noVec3 {
public:
	float	x,y,z;

	noVec3( void );
	explicit noVec3( const float x, const float y, const float z );

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

	int				GetDimension( void ) const;
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
};

extern noVec3 vec3_origin;
#define vec3_zero vec3_origin;

NO_INLINE noVec3::noVec3( void ) {}

NO_INLINE noVec3::noVec3( const float x, const float y, const float z) {
	this->x = x;
	this->y = y;
	this->z = z;
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
	return x * a.x + y * a.y;
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

NO_INLINE noVec3 operator*( const float a, const noVec3& b ) {
	return noVec3( b.x * a, b.y * a, b.z * a);
}

NO_INLINE noVec3 noVec3::operator-( const noVec3& a ) const {
	return noVec3( x  - a.x, y - a.y, z - a.z );
}

NO_INLINE noVec3& noVec3::operator+=( const noVec3& a )	{
	x += a.x;
	y += a.y;
	y += a.z;
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

NO_INLINE const float * noVec3::ToFloatPtr( void ) const {
	return &x;
}

NO_INLINE float * noVec3::ToFloatPtr( void ) {
	return &x;
}


class noPolar3 {
public:
	float			radius, theta, phi;

	noPolar3( void );
	explicit noPolar3( const float radius, const float theta, const float phi );

	void 			Set( const float radius, const float theta, const float phi );

	float			operator[]( const int index ) const;
	float &			operator[]( const int index );
	noPolar3		operator-() const;
	noPolar3 &		operator=( const noPolar3 &a );

	noVec3			ToVec3( void ) const;
};

class noVec4 
{
public:
	noVec4() {}
	noVec4(float x, float y, float z, float w);

	
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


	int				GetDimension( void ) const;
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );

	//NO_ALIGN16( float x );
	float x ;
	float y,z,w;
};

extern noVec4 vec4_origin;
#define vec4_zero vec4_origin

NO_INLINE noVec4::noVec4( float x, float y, float z, float w ) {
	Set(x, y, z, w);
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

NO_INLINE const float * noVec4::ToFloatPtr( void ) const {
	return &x;
}

NO_INLINE float * noVec4::ToFloatPtr( void ) {
	return &x;
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


#endif /* !__MATH_VECTOR_H__ */
