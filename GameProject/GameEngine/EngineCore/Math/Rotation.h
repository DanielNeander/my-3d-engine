/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 no Software LLC, a ZeniMax Media company. 

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

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from no Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing no Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __MATH_ROTATION_H__
#define __MATH_ROTATION_H__

/*
===============================================================================

	Describes a complete rotation in degrees about an abritray axis.
	A local rotation matrix is stored for fast rotation of multiple points.

===============================================================================
*/


class noAngles;
class noQuat;
class noMat3;

class noRotation {

	friend class noAngles;
	friend class noQuat;
	friend class noMat3;

public:
						noRotation( void );
						noRotation( const noVec3 &rotationOrigin, const noVec3 &rotationVec, const float rotationAngle );

	void				Set( const noVec3 &rotationOrigin, const noVec3 &rotationVec, const float rotationAngle );
	void				SetOrigin( const noVec3 &rotationOrigin );
	void				SetVec( const noVec3 &rotationVec );					// has to be normalized
	void				SetVec( const float x, const float y, const float z );	// has to be normalized
	void				SetAngle( const float rotationAngle );
	void				Scale( const float s );
	void				ReCalculateMatrix( void );
	const noVec3 &		GetOrigin( void ) const;
	const noVec3 &		GetVec( void ) const;
	float				GetAngle( void ) const;

	noRotation			operator-() const;										// flips rotation
	noRotation			operator*( const float s ) const;						// scale rotation
	noRotation			operator/( const float s ) const;						// scale rotation
	noRotation &		operator*=( const float s );							// scale rotation
	noRotation &		operator/=( const float s );							// scale rotation
	noVec3				operator*( const noVec3 &v ) const;						// rotate vector

	friend noRotation	operator*( const float s, const noRotation &r );		// scale rotation
	friend noVec3		operator*( const noVec3 &v, const noRotation &r );		// rotate vector
	friend noVec3 &		operator*=( noVec3 &v, const noRotation &r );			// rotate vector

	noAngles			ToAngles( void ) const;
	noQuat				ToQuat( void ) const;
	const noMat3 &		ToMat3( void ) const;
	noMat4				ToMat4( void ) const;
	noVec3				ToAngularVelocity( void ) const;

	void				RotatePoint( noVec3 &point ) const;

	void				Normalize180( void );
	void				Normalize360( void );

private:
	noVec3				origin;			// origin of rotation
	noVec3				vec;			// normalized vector to rotate around
	float				angle;			// angle of rotation in degrees
	mutable noMat3		axis;			// rotation axis
	mutable bool		axisValid;		// true if rotation axis is valno
};


NO_INLINE noRotation::noRotation( void ) {
}

NO_INLINE noRotation::noRotation( const noVec3 &rotationOrigin, const noVec3 &rotationVec, const float rotationAngle ) {
	origin = rotationOrigin;
	vec = rotationVec;
	angle = rotationAngle;
	axisValid = false;
}

NO_INLINE void noRotation::Set( const noVec3 &rotationOrigin, const noVec3 &rotationVec, const float rotationAngle ) {
	origin = rotationOrigin;
	vec = rotationVec;
	angle = rotationAngle;
	axisValid = false;
}

NO_INLINE void noRotation::SetOrigin( const noVec3 &rotationOrigin ) {
	origin = rotationOrigin;
}

NO_INLINE void noRotation::SetVec( const noVec3 &rotationVec ) {
	vec = rotationVec;
	axisValid = false;
}

NO_INLINE void noRotation::SetVec( float x, float y, float z ) {
	vec[0] = x;
	vec[1] = y;
	vec[2] = z;
	axisValid = false;
}

NO_INLINE void noRotation::SetAngle( const float rotationAngle ) {
	angle = rotationAngle;
	axisValid = false;
}

NO_INLINE void noRotation::Scale( const float s ) {
	angle *= s;
	axisValid = false;
}

NO_INLINE void noRotation::ReCalculateMatrix( void ) {
	axisValid = false;
	ToMat3();
}

NO_INLINE const noVec3 &noRotation::GetOrigin( void ) const {
	return origin;
}

NO_INLINE const noVec3 &noRotation::GetVec( void ) const  {
	return vec;
}

NO_INLINE float noRotation::GetAngle( void ) const  {
	return angle;
}

NO_INLINE noRotation noRotation::operator-() const {
	return noRotation( origin, vec, -angle );
}

NO_INLINE noRotation noRotation::operator*( const float s ) const {
	return noRotation( origin, vec, angle * s );
}

NO_INLINE noRotation noRotation::operator/( const float s ) const {
	assert( s != 0.0f );
	return noRotation( origin, vec, angle / s );
}

NO_INLINE noRotation &noRotation::operator*=( const float s ) {
	angle *= s;
	axisValid = false;
	return *this;
}

NO_INLINE noRotation &noRotation::operator/=( const float s ) {
	assert( s != 0.0f );
	angle /= s;
	axisValid = false;
	return *this;
}

NO_INLINE noVec3 noRotation::operator*( const noVec3 &v ) const {
	if ( !axisValid ) {
		ToMat3();
	}
	return ((v - origin) * axis + origin);
}

NO_INLINE noRotation operator*( const float s, const noRotation &r ) {
	return r * s;
}

NO_INLINE noVec3 operator*( const noVec3 &v, const noRotation &r ) {
	return r * v;
}

NO_INLINE noVec3 &operator*=( noVec3 &v, const noRotation &r ) {
	v = r * v;
	return v;
}

NO_INLINE void noRotation::RotatePoint( noVec3 &point ) const {
	if ( !axisValid ) {
		ToMat3();
	}
	point = ((point - origin) * axis + origin);
}

#endif /* !__MATH_ROTATION_H__ */
