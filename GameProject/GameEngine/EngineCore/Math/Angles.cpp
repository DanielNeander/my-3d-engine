#include "Math.h"
#include "Vector.h"
#include "Angles.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "Rotation.h"
#include "EngineCore/Util/Str.h"



#include <float.h>

noAngles ang_zero( 0.0f, 0.0f, 0.0f );


/*
=================
noAngles::Normalize360

returns angles normalized to the range [0 <= angle < 360]
=================
*/
noAngles& noAngles::Normalize360( void ) {
	int i;

	for ( i = 0; i < 3; i++ ) {
		if ( ( (*this)[i] >= 360.0f ) || ( (*this)[i] < 0.0f ) ) {
			(*this)[i] -= floor( (*this)[i] / 360.0f ) * 360.0f;

			if ( (*this)[i] >= 360.0f ) {
				(*this)[i] -= 360.0f;
			}
			if ( (*this)[i] < 0.0f ) {
				(*this)[i] += 360.0f;
			}
		}
	}

	return *this;
}

/*
=================
noAngles::Normalize180

returns angles normalized to the range [-180 < angle <= 180]
=================
*/
noAngles& noAngles::Normalize180( void ) {
	Normalize360();

	if ( pitch > 180.0f ) {
		pitch -= 360.0f;
	}

	if ( yaw > 180.0f ) {
		yaw -= 360.0f;
	}

	if ( roll > 180.0f ) {
		roll -= 360.0f;
	}
	return *this;
}

/*
=================
noAngles::ToVectors
=================
*/
void noAngles::ToVectors( noVec3 *forward, noVec3 *right, noVec3 *up ) const {
	float sr, sp, sy, cr, cp, cy;

	noMath::SinCos( DEG2RAD( yaw ), sy, cy );
	noMath::SinCos( DEG2RAD( pitch ), sp, cp );
	noMath::SinCos( DEG2RAD( roll ), sr, cr );

	if ( forward ) {
		forward->Set( cp * cy, cp * sy, -sp );
	}

	if ( right ) {
		right->Set( -sr * sp * cy + cr * sy, -sr * sp * sy + -cr * cy, -sr * cp );
	}

	if ( up ) {
		up->Set( cr * sp * cy + -sr * -sy, cr * sp * sy + -sr * cy, cr * cp );
	}
}

/*
=================
noAngles::ToForward
=================
*/
noVec3 noAngles::ToForward( void ) const {
	float sp, sy, cp, cy;

	noMath::SinCos( DEG2RAD( yaw ), sy, cy );
	noMath::SinCos( DEG2RAD( pitch ), sp, cp );

	return noVec3( cp * cy, cp * sy, -sp );
}

/*
=================
noAngles::ToQuat
=================
*/
noQuat noAngles::ToQuat( void ) const {
	float sx, cx, sy, cy, sz, cz;
	float sxcy, cxcy, sxsy, cxsy;

	noMath::SinCos( DEG2RAD( yaw ) * 0.5f, sz, cz );
	noMath::SinCos( DEG2RAD( pitch ) * 0.5f, sy, cy );
	noMath::SinCos( DEG2RAD( roll ) * 0.5f, sx, cx );

	sxcy = sx * cy;
	cxcy = cx * cy;
	sxsy = sx * sy;
	cxsy = cx * sy;

	return noQuat( cxsy*sz - sxcy*cz, -cxsy*cz - sxcy*sz, sxsy*cz - cxcy*sz, cxcy*cz + sxsy*sz );
}

/*
=================
noAngles::ToRotation
=================
*/
noRotation noAngles::ToRotation( void ) const {
	noVec3 vec;
	float angle, w;
	float sx, cx, sy, cy, sz, cz;
	float sxcy, cxcy, sxsy, cxsy;

	if ( pitch == 0.0f ) {
		if ( yaw == 0.0f ) {
			return noRotation( vec3_origin, noVec3( -1.0f, 0.0f, 0.0f ), roll );
		}
		if ( roll == 0.0f ) {
			return noRotation( vec3_origin, noVec3( 0.0f, 0.0f, -1.0f ), yaw );
		}
	} else if ( yaw == 0.0f && roll == 0.0f ) {
		return noRotation( vec3_origin, noVec3( 0.0f, -1.0f, 0.0f ), pitch );
	}

	noMath::SinCos( DEG2RAD( yaw ) * 0.5f, sz, cz );
	noMath::SinCos( DEG2RAD( pitch ) * 0.5f, sy, cy );
	noMath::SinCos( DEG2RAD( roll ) * 0.5f, sx, cx );

	sxcy = sx * cy;
	cxcy = cx * cy;
	sxsy = sx * sy;
	cxsy = cx * sy;

	vec.x =  cxsy * sz - sxcy * cz;
	vec.y = -cxsy * cz - sxcy * sz;
	vec.z =  sxsy * cz - cxcy * sz;
	w =		 cxcy * cz + sxsy * sz;
	angle = noMath::ACos( w );
	if ( angle == 0.0f ) {
		vec.Set( 0.0f, 0.0f, 1.0f );
	} else {
		//vec *= (1.0f / sin( angle ));
		vec.Normalize();
		vec.FixDegenerateNormal();
		angle *= 2.0f * noMath::M_RAD2DEG;
	}
	return noRotation( vec3_origin, vec, angle );
}

/*
=================
noAngles::ToMat3
=================
*/
noMat3 noAngles::ToMat3( void ) const {
	noMat3 mat;
	float sr, sp, sy, cr, cp, cy;

	noMath::SinCos( DEG2RAD( yaw ), sy, cy );
	noMath::SinCos( DEG2RAD( pitch ), sp, cp );
	noMath::SinCos( DEG2RAD( roll ), sr, cr );

	mat[ 0 ].Set( cp * cy, cp * sy, -sp );
	mat[ 1 ].Set( sr * sp * cy + cr * -sy, sr * sp * sy + cr * cy, sr * cp );
	mat[ 2 ].Set( cr * sp * cy + -sr * -sy, cr * sp * sy + -sr * cy, cr * cp );

	return mat;
}

/*
=================
noAngles::ToMat4
=================
*/
noMat4 noAngles::ToMat4( void ) const {
	return ToMat3().ToMat4();
}

/*
=================
noAngles::ToAngularVelocity
=================
*/
noVec3 noAngles::ToAngularVelocity( void ) const {
	noRotation rotation = noAngles::ToRotation();
	return rotation.GetVec() * DEG2RAD( rotation.GetAngle() );
}

/*
=============
noAngles::ToString
=============
*/
const char *noAngles::ToString( int precision ) const {
	return idStr::FloatArrayToString( ToFloatPtr(), GetDimension(), precision );
}
