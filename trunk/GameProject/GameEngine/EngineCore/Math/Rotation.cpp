#include "Math.h"
#include "Vector.h"
#include "Angles.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "Rotation.h"
/*
============
noRotation::ToAngles
============
*/
noAngles noRotation::ToAngles( void ) const {
	return ToMat3().ToAngles();
}

/*
============
noRotation::ToQuat
============
*/
noQuat noRotation::ToQuat( void ) const {
	float a, s, c;

	a = angle * ( noMath::M_DEG2RAD * 0.5f );
	noMath::SinCos( a, s, c );
	return noQuat( vec.x * s, vec.y * s, vec.z * s, c );
}

/*
============
noRotation::toMat3
============
*/
const noMat3 &noRotation::ToMat3( void ) const {
	float wx, wy, wz;
	float xx, yy, yz;
	float xy, xz, zz;
	float x2, y2, z2;
	float a, c, s, x, y, z;

	if ( axisValid ) {
		return axis;
	}

	a = angle * ( noMath::M_DEG2RAD * 0.5f );
	noMath::SinCos( a, s, c );

	x = vec[0] * s;
	y = vec[1] * s;
	z = vec[2] * s;

	x2 = x + x;
	y2 = y + y;
	z2 = z + z;

	xx = x * x2;
	xy = x * y2;
	xz = x * z2;

	yy = y * y2;
	yz = y * z2;
	zz = z * z2;

	wx = c * x2;
	wy = c * y2;
	wz = c * z2;

	axis[ 0 ][ 0 ] = 1.0f - ( yy + zz );
	axis[ 0 ][ 1 ] = xy - wz;
	axis[ 0 ][ 2 ] = xz + wy;

	axis[ 1 ][ 0 ] = xy + wz;
	axis[ 1 ][ 1 ] = 1.0f - ( xx + zz );
	axis[ 1 ][ 2 ] = yz - wx;

	axis[ 2 ][ 0 ] = xz - wy;
	axis[ 2 ][ 1 ] = yz + wx;
	axis[ 2 ][ 2 ] = 1.0f - ( xx + yy );

	axisValid = true;

	return axis;
}

/*
============
noRotation::ToMat4
============
*/
noMat4 noRotation::ToMat4( void ) const {
	return ToMat3().ToMat4();
}

/*
============
noRotation::ToAngularVelocity
============
*/
noVec3 noRotation::ToAngularVelocity( void ) const {
	return vec * DEG2RAD( angle );
}

/*
============
noRotation::Normalize180
============
*/
void noRotation::Normalize180( void ) {
	angle -= floor( angle / 360.0f ) * 360.0f;
	if ( angle > 180.0f ) {
		angle -= 360.0f;
	}
	else if ( angle < -180.0f ) {
		angle += 360.0f;
	}
}

/*
============
noRotation::Normalize360
============
*/
void noRotation::Normalize360( void ) {
	angle -= floor( angle / 360.0f ) * 360.0f;
	if ( angle > 360.0f ) {
		angle -= 360.0f;
	}
	else if ( angle < 0.0f ) {
		angle += 360.0f;
	}
}
