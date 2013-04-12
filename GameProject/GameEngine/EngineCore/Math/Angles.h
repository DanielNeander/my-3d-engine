#pragma once 
#ifndef __MATH_ANGLES_H__
#define __MATH_ANGLES_H__


/*
===============================================================================

	Euler angles

===============================================================================
*/

// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

class noVec3;
class noQuat;
class noRotation;
class noMat3;
class noMat4;


class noAngles {
public:
	float			pitch;
	float			yaw;
	float			roll;

	noAngles( void );
	noAngles( float pitch, float yaw, float roll );
	explicit noAngles( const noVec3 &v );

	void 			Set( float pitch, float yaw, float roll );
	noAngles &		Zero( void );

	float			operator[]( int index ) const;
	float &			operator[]( int index );
	noAngles		operator-() const;			// negate angles, in general not the inverse rotation
	noAngles &		operator=( const noAngles &a );
	noAngles		operator+( const noAngles &a ) const;
	noAngles &		operator+=( const noAngles &a );
	noAngles		operator-( const noAngles &a ) const;
	noAngles &		operator-=( const noAngles &a );
	noAngles		operator*( const float a ) const;
	noAngles &		operator*=( const float a );
	noAngles		operator/( const float a ) const;
	noAngles &		operator/=( const float a );

	friend noAngles	operator*( const float a, const noAngles &b );

	bool			Compare( const noAngles &a ) const;							// exact compare, no epsilon
	bool			Compare( const noAngles &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==(	const noAngles &a ) const;						// exact compare, no epsilon
	bool			operator!=(	const noAngles &a ) const;						// exact compare, no epsilon

	noAngles &		Normalize360( void );	// normalizes 'this'
	noAngles &		Normalize180( void );	// normalizes 'this'

	void			Clamp( const noAngles &min, const noAngles &max );

	int				GetDimension( void ) const;

	void			ToVectors( noVec3 *forward, noVec3 *right = NULL, noVec3 *up = NULL ) const;
	noVec3			ToForward( void ) const;
	noQuat			ToQuat( void ) const;
	noRotation		ToRotation( void ) const;
	noMat3			ToMat3( void ) const;
	noMat4			ToMat4( void ) const;
	noVec3			ToAngularVelocity( void ) const;
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;
};

extern noAngles ang_zero;

NO_INLINE noAngles::noAngles( void ) {
}

NO_INLINE noAngles::noAngles( float pitch, float yaw, float roll ) {
	this->pitch = pitch;
	this->yaw	= yaw;
	this->roll	= roll;
}

NO_INLINE noAngles::noAngles( const noVec3 &v ) {
	this->pitch = v[0];
	this->yaw	= v[1];
	this->roll	= v[2];
}

NO_INLINE void noAngles::Set( float pitch, float yaw, float roll ) {
	this->pitch = pitch;
	this->yaw	= yaw;
	this->roll	= roll;
}

NO_INLINE noAngles &noAngles::Zero( void ) {
	pitch = yaw = roll = 0.0f;
	return *this;
}

NO_INLINE float noAngles::operator[]( int index ) const {
	assert( ( index >= 0 ) && ( index < 3 ) );
	return ( &pitch )[ index ];
}

NO_INLINE float &noAngles::operator[]( int index ) {
	assert( ( index >= 0 ) && ( index < 3 ) );
	return ( &pitch )[ index ];
}

NO_INLINE noAngles noAngles::operator-() const {
	return noAngles( -pitch, -yaw, -roll );
}

NO_INLINE noAngles &noAngles::operator=( const noAngles &a ) {
	pitch	= a.pitch;
	yaw		= a.yaw;
	roll	= a.roll;
	return *this;
}

NO_INLINE noAngles noAngles::operator+( const noAngles &a ) const {
	return noAngles( pitch + a.pitch, yaw + a.yaw, roll + a.roll );
}

NO_INLINE noAngles& noAngles::operator+=( const noAngles &a ) {
	pitch	+= a.pitch;
	yaw		+= a.yaw;
	roll	+= a.roll;

	return *this;
}

NO_INLINE noAngles noAngles::operator-( const noAngles &a ) const {
	return noAngles( pitch - a.pitch, yaw - a.yaw, roll - a.roll );
}

NO_INLINE noAngles& noAngles::operator-=( const noAngles &a ) {
	pitch	-= a.pitch;
	yaw		-= a.yaw;
	roll	-= a.roll;

	return *this;
}

NO_INLINE noAngles noAngles::operator*( const float a ) const {
	return noAngles( pitch * a, yaw * a, roll * a );
}

NO_INLINE noAngles& noAngles::operator*=( float a ) {
	pitch	*= a;
	yaw		*= a;
	roll	*= a;
	return *this;
}

NO_INLINE noAngles noAngles::operator/( const float a ) const {
	float inva = 1.0f / a;
	return noAngles( pitch * inva, yaw * inva, roll * inva );
}

NO_INLINE noAngles& noAngles::operator/=( float a ) {
	float inva = 1.0f / a;
	pitch	*= inva;
	yaw		*= inva;
	roll	*= inva;
	return *this;
}

NO_INLINE noAngles operator*( const float a, const noAngles &b ) {
	return noAngles( a * b.pitch, a * b.yaw, a * b.roll );
}

NO_INLINE bool noAngles::Compare( const noAngles &a ) const {
	return ( ( a.pitch == pitch ) && ( a.yaw == yaw ) && ( a.roll == roll ) );
}

NO_INLINE bool noAngles::Compare( const noAngles &a, const float epsilon ) const {
	if ( noMath::Fabs( pitch - a.pitch ) > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( yaw - a.yaw ) > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( roll - a.roll ) > epsilon ) {
		return false;
	}

	return true;
}

NO_INLINE bool noAngles::operator==( const noAngles &a ) const {
	return Compare( a );
}

NO_INLINE bool noAngles::operator!=( const noAngles &a ) const {
	return !Compare( a );
}

NO_INLINE void noAngles::Clamp( const noAngles &min, const noAngles &max ) {
	if ( pitch < min.pitch ) {
		pitch = min.pitch;
	} else if ( pitch > max.pitch ) {
		pitch = max.pitch;
	}
	if ( yaw < min.yaw ) {
		yaw = min.yaw;
	} else if ( yaw > max.yaw ) {
		yaw = max.yaw;
	}
	if ( roll < min.roll ) {
		roll = min.roll;
	} else if ( roll > max.roll ) {
		roll = max.roll;
	}
}

NO_INLINE int noAngles::GetDimension( void ) const {
	return 3;
}

NO_INLINE const float *noAngles::ToFloatPtr( void ) const {
	return &pitch;
}

NO_INLINE float *noAngles::ToFloatPtr( void ) {
	return &pitch;
}

#endif /* !__MATH_ANGLES_H__ */
