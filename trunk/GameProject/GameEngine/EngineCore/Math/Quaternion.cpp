#include "Vector.h"
#include "Angles.h"
#include "Matrix.h"
#include "Rotation.h"
#include "Quaternion.h"
#include "EngineCore/Util/Str.h"

const noQuat  noQuat::Slerp( const noQuat &from, const noQuat &to, float t )
{
	// SLERP
	float dot = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;

	if (fabs(dot) > 0.9995f) {
		// fall back to LERP
		return noQuat::Lerp(t, from, to);
	}

	float a = acosf(dot) * t;
	noQuat q = (to - from * dot);
	q.Normalize();

	return from * cosf(a) + q * sinf(a);
#if 0 // very Slow!
	noQuat	temp;
	float	omega, cosom, sinom, scale0, scale1;

	if ( t <= 0.0f ) {
		*this = from;
		return *this;
	}

	if ( t >= 1.0f ) {
		*this = to;
		return *this;
	}

	if ( from == to ) {
		*this = to;
		return *this;
	}

	cosom = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;
	if ( cosom < 0.0f ) {
		temp = -to;
		cosom = -cosom;
	} else {
		temp = to;
	}

	if ( ( 1.0f - cosom ) > 1e-6f ) {
#if 0
		omega = acos( cosom );
		sinom = 1.0f / sin( omega );
		scale0 = sin( ( 1.0f - t ) * omega ) * sinom;
		scale1 = sin( t * omega ) * sinom;
#else
		scale0 = 1.0f - cosom * cosom;
		sinom = noMath::InvSqrt( scale0 );
		omega = noMath::ATan16( scale0 * sinom, cosom );
		scale0 = noMath::Sin16( ( 1.0f - t ) * omega ) * sinom;
		scale1 = noMath::Sin16( t * omega ) * sinom;
#endif
	} else {
		scale0 = 1.0f - t;
		scale1 = t;
	}

	*this = ( scale0 * from ) + ( scale1 * temp );
	return *this;
#endif
}

noMat3 noQuat::ToMat3( void ) const
{
	noMat3	mat;
	float	wx, wy, wz;
	float	xx, yy, yz;
	float	xy, xz, zz;
	float	x2, y2, z2;

	x2 = x + x;
	y2 = y + y;
	z2 = z + z;

	xx = x * x2;
	xy = x * y2;
	xz = x * z2;

	yy = y * y2;
	yz = y * z2;
	zz = z * z2;

	wx = w * x2;
	wy = w * y2;
	wz = w * z2;

	mat[ 0 ][ 0 ] = 1.0f - ( yy + zz );
	mat[ 0 ][ 1 ] = xy - wz;
	mat[ 0 ][ 2 ] = xz + wy;

	mat[ 1 ][ 0 ] = xy + wz;
	mat[ 1 ][ 1 ] = 1.0f - ( xx + zz );
	mat[ 1 ][ 2 ] = yz - wx;

	mat[ 2 ][ 0 ] = xz - wy;
	mat[ 2 ][ 1 ] = yz + wx;
	mat[ 2 ][ 2 ] = 1.0f - ( xx + yy );

	return mat;

}

noMat4 noQuat::ToMat4( void ) const
{
	return ToMat3().ToMat4();
}

void noQuat::FromAngleAxis( float angle, const noVec3& axis )
{
	w = noMath::Cos(0.5f * angle);
	x = axis.x;
	y = axis.y;
	z = axis.z;

	*this = *this * noMath::Sin(0.5f * angle);

	this->Normalize();
}

const noVec3 noQuat::RotateVec3(const noVec3& v)
{
	noVec3 result;
	float vs = ( (2*w*w) -1.0f);
	result = v * vs;

	float idv = x * v.x + y * v.y + z * v.z;

	noVec3 temp(x, y, z);
	noVec3 idvV;
	idvV = temp * idv;
	
	noVec3 icV;
	icV = temp.Cross(v);
	icV = icV * w;

	icV = idvV + icV;

	result.x += icV.x * 2.0f;
	result.y += icV.y * 2.0f;
	result.z += icV.z * 2.0f;
	
	return result;
}

/*
=====================
idQuat::ToCQuat
=====================
*/
idCQuat noQuat::ToCQuat( void ) const {
	if ( w < 0.0f ) {
		return idCQuat( -x, -y, -z );
	}
	return idCQuat( x, y, z );
}

noAngles noQuat::ToAngles( void ) const
{
	return ToMat3().ToAngles();

}

noRotation noQuat::ToRotation( void ) const
{
	noVec3 vec;
	float angle;

	vec.x = x;
	vec.y = y;
	vec.z = z;
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

noVec3 noQuat::ToAngularVelocity( void ) const
{
	noVec3 vec;

	vec.x = x;
	vec.y = y;
	vec.z = z;
	vec.Normalize();
	return vec * noMath::ACos( w );
}

noQuat noQuat::Identity = noQuat(0.f, 0.f, 0.f, 1.f);


/*
=============
idCQuat::ToMat3
=============
*/
noMat3 idCQuat::ToMat3( void ) const {
	return ToQuat().ToMat3();
}

/*
=============
idCQuat::ToMat4
=============
*/
noMat4 idCQuat::ToMat4( void ) const {
	return ToQuat().ToMat4();
}

/*
=============
idCQuat::ToString
=============
*/
const char *idCQuat::ToString( int precision ) const {
	return idStr::FloatArrayToString( ToFloatPtr(), GetDimension(), precision );
}

