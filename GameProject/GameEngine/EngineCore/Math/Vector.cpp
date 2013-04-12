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
#include "Vector.h"
#include "Matrix.h"
#include "Rotation.h"
#include "Angles.h"
#include "Quaternion.h"
#include "EngineCore/Util/Str.h"

noVec2 vec2_origin( 0.0f, 0.0f );
noVec3 vec3_origin( 0.0f, 0.0f, 0.0f );
noVec4 vec4_origin( 0.0f, 0.0f, 0.0f, 0.0f );
idVec5 vec5_origin( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
idVec6 vec6_origin( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
idVec6 vec6_infinity( noMath::INFINITY, noMath::INFINITY, noMath::INFINITY, noMath::INFINITY, noMath::INFINITY, noMath::INFINITY );

const char * noVec2::ToString( int precision /*= 2 */ ) const {
	return idStr::FloatArrayToString( ToFloatPtr(), GetDimension(), precision );
}


void noVec2::Lerp( const noVec2 &v1, const noVec2 &v2, const float l ) {
	if ( l <= 0.0f ) {
		*this = v1;
	} else if ( l >= 1.0f ) {
		*this = v2;
	} else {
		(*this) = v1 + l * ( v2 - v1 );
	}
}




noQuat noVec3::getRotationTo( const noVec3& dest, const noVec3& fallbackAxis /*= vec3_zero*/ ) const
{
	// Based on Stan Melax's article in Game Programming Gems
	noQuat q;
	noVec3 v0 = *this;
	noVec3 v1 = dest;
	v0.Normalize();
	v1.Normalize();

	float d = v0 * v1;
	if (d >= 1.0f)
	{
		return noQuat(1.0f, 0.0f, 0.0f, 0.0f);
	}
	if (d < (1e-6f - 1.0f))
	{
		if (fallbackAxis != vec3_zero)
		{
			noRotation rot(vec3_zero, fallbackAxis, RAD2DEG(noMath::PI));
			q = rot.ToQuat();
		}
		else 
		{
			noVec3 axis = noVec3(1.0f, 0.0f, 0.0f).Cross(*this);
			if(axis.LengthSqr() < (1e-06 * 1e-06) )
				axis = noVec3(0.f, 1.f, 0.f).Cross(*this);
			axis.Normalize();

			noRotation rot(vec3_zero, axis, RAD2DEG(noMath::PI));
			q = rot.ToQuat();
		}
	}
	else 
	{
		float s = noMath::Sqrt( (1+d) * 2);
		float invs = 1 / s;

		noVec3 c = v0.Cross(v1);

		q.x = c.x * invs;
		q.y = c.y * invs;
		q.z = c.z * invs;
		q.w = s * 0.5f;
		q.Normalize();
	}

	return q;
}

void noVec3::Lerp( const noVec3 &v1, const noVec3 &v2, const float l ) {
	if ( l <= 0.0f ) {
		(*this) = v1;
	} else if ( l >= 1.0f ) {
		(*this) = v2;
	} else {
		(*this) = v1 + l * ( v2 - v1 );
	}
}

/*
=============
SLerp

Spherical linear interpolation from v1 to v2.
Vectors are expected to be normalized.
=============
*/
#define LERP_DELTA 1e-6

void noVec3::SLerp( const noVec3 &v1, const noVec3 &v2, const float t ) {
	float omega, cosom, sinom, scale0, scale1;

	if ( t <= 0.0f ) {
		(*this) = v1;
		return;
	} else if ( t >= 1.0f ) {
		(*this) = v2;
		return;
	}

	cosom = v1 * v2;
	if ( ( 1.0f - cosom ) > LERP_DELTA ) {
		omega = acos( cosom );
		sinom = sin( omega );
		scale0 = sin( ( 1.0f - t ) * omega ) / sinom;
		scale1 = sin( t * omega ) / sinom;
	} else {
		scale0 = 1.0f - t;
		scale1 = t;
	}

	(*this) = ( v1 * scale0 + v2 * scale1 );

}


/*
=============
noVec3::ToYaw
=============
*/
float noVec3::ToYaw( void ) const {
	float yaw;

	if ( ( y == 0.0f ) && ( x == 0.0f ) ) {
		yaw = 0.0f;
	} else {
		yaw = RAD2DEG( atan2( y, x ) );
		if ( yaw < 0.0f ) {
			yaw += 360.0f;
		}
	}

	return yaw;
}

/*
=============
noVec3::ToPitch
=============
*/
float noVec3::ToPitch( void ) const {
	float	forward;
	float	pitch;

	if ( ( x == 0.0f ) && ( y == 0.0f ) ) {
		if ( z > 0.0f ) {
			pitch = 90.0f;
		} else {
			pitch = 270.0f;
		}
	} else {
		forward = ( float )noMath::Sqrt( x * x + y * y );
		pitch = RAD2DEG( atan2( z, forward ) );
		if ( pitch < 0.0f ) {
			pitch += 360.0f;
		}
	}

	return pitch;
}

/*
=============
noVec3::ToAngles
=============
*/
noAngles noVec3::ToAngles( void ) const {
	float forward;
	float yaw;
	float pitch;

	if ( ( x == 0.0f ) && ( y == 0.0f ) ) {
		yaw = 0.0f;
		if ( z > 0.0f ) {
			pitch = 90.0f;
		} else {
			pitch = 270.0f;
		}
	} else {
		yaw = RAD2DEG( atan2( y, x ) );
		if ( yaw < 0.0f ) {
			yaw += 360.0f;
		}

		forward = ( float )noMath::Sqrt( x * x + y * y );
		pitch = RAD2DEG( atan2( z, forward ) );
		if ( pitch < 0.0f ) {
			pitch += 360.0f;
		}
	}

	return noAngles( -pitch, yaw, 0.0f );
}

/*
=============
noVec3::ToPolar
=============
*/
idPolar3 noVec3::ToPolar( void ) const {
	float forward;
	float yaw;
	float pitch;

	if ( ( x == 0.0f ) && ( y == 0.0f ) ) {
		yaw = 0.0f;
		if ( z > 0.0f ) {
			pitch = 90.0f;
		} else {
			pitch = 270.0f;
		}
	} else {
		yaw = RAD2DEG( atan2( y, x ) );
		if ( yaw < 0.0f ) {
			yaw += 360.0f;
		}

		forward = ( float )noMath::Sqrt( x * x + y * y );
		pitch = RAD2DEG( atan2( z, forward ) );
		if ( pitch < 0.0f ) {
			pitch += 360.0f;
		}
	}
	return idPolar3( noMath::Sqrt( x * x + y * y + z * z ), yaw, -pitch );
}

/*
=============
noVec3::ToMat3
=============
*/
noMat3 noVec3::ToMat3( void ) const {
	noMat3	mat;
	float	d;

	mat[0] = *this;
	d = x * x + y * y;
	if ( !d ) {
		mat[1][0] = 1.0f;
		mat[1][1] = 0.0f;
		mat[1][2] = 0.0f;
	} else {
		d = noMath::InvSqrt( d );
		mat[1][0] = -y * d;
		mat[1][1] = x * d;
		mat[1][2] = 0.0f;
	}
	mat[2] = Cross( mat[1] );

	return mat;
}

/*
=============
noVec3::ToString
=============
*/
const char *noVec3::ToString( int precision ) const {
	return idStr::FloatArrayToString( ToFloatPtr(), GetDimension(), precision );
}

/*
=============
ProjectSelfOntoSphere

Projects the z component onto a sphere.
=============
*/
void noVec3::ProjectSelfOntoSphere( const float radius ) {
	float rsqr = radius * radius;
	float len = Length();
	if ( len  < rsqr * 0.5f ) {
		z = sqrt( rsqr - len );
	} else {
		z = rsqr / ( 2.0f * sqrt( len ) );
	}
}



//===============================================================
//
//	idVec5
//
//===============================================================

/*
=============
idVec5::ToString
=============
*/
const char *idVec5::ToString( int precision ) const {
	return idStr::FloatArrayToString( ToFloatPtr(), GetDimension(), precision );
}

/*
=============
idVec5::Lerp
=============
*/
void idVec5::Lerp( const idVec5 &v1, const idVec5 &v2, const float l ) {
	if ( l <= 0.0f ) {
		(*this) = v1;
	} else if ( l >= 1.0f ) {
		(*this) = v2;
	} else {
		x = v1.x + l * ( v2.x - v1.x );
		y = v1.y + l * ( v2.y - v1.y );
		z = v1.z + l * ( v2.z - v1.z );
		s = v1.s + l * ( v2.s - v1.s );
		t = v1.t + l * ( v2.t - v1.t );
	}
}

//===============================================================
//
//	idVec6
//
//===============================================================

/*
=============
idVec6::ToString
=============
*/
const char *idVec6::ToString( int precision ) const {
	return idStr::FloatArrayToString( ToFloatPtr(), GetDimension(), precision );
}


//===============================================================
//
//	idVecX
//
//===============================================================

float	idVecX::temp[VECX_MAX_TEMP+4];
float *	idVecX::tempPtr = (float *) ( ( (int) idVecX::temp + 15 ) & ~15 );
int		idVecX::tempIndex = 0;

/*
=============
idVecX::ToString
=============
*/
const char *idVecX::ToString( int precision ) const {
	return idStr::FloatArrayToString( ToFloatPtr(), GetDimension(), precision );
}



