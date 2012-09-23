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
#ifndef MATH_MATRIX_H
#define MATH_MATRIX_H

#include <Math/Vector.h>

#define MATRIX_INVERSE_EPSILON		1e-14
#define MATRIX_EPSILON				1e-6


class noAngle;
class noQuat;
class noMat4;
//===============================================================
//
//	noMat3 - 3x3 matrix
//
//	NOTE:	matrix is column-major
//
//===============================================================

class noMat3 {
public:
	noMat3( void );
	explicit noMat3( const noVec3 &x, const noVec3 &y, const noVec3 &z );
	explicit noMat3( const float xx, const float xy, const float xz, const float yx, const float yy, const float yz, const float zx, const float zy, const float zz );
	explicit noMat3( const float src[ 3 ][ 3 ] );

	const noVec3 &	operator[]( int index ) const;
	noVec3 &		operator[]( int index );
	noMat3			operator-() const;
	noMat3			operator*( const float a ) const;
	noVec3			operator*( const noVec3 &vec ) const;
	noMat3			operator*( const noMat3 &a ) const;
	noMat3			operator+( const noMat3 &a ) const;
	noMat3			operator-( const noMat3 &a ) const;
	noMat3 &		operator*=( const float a );
	noMat3 &		operator*=( const noMat3 &a );
	noMat3 &		operator+=( const noMat3 &a );
	noMat3 &		operator-=( const noMat3 &a );

	friend noMat3	operator*( const float a, const noMat3 &mat );
	friend noVec3	operator*( const noVec3 &vec, const noMat3 &mat );
	friend noVec3 &	operator*=( noVec3 &vec, const noMat3 &mat );

	bool			Compare( const noMat3 &a ) const;						// exact compare, no epsilon
	bool			Compare( const noMat3 &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==( const noMat3 &a ) const;					// exact compare, no epsilon
	bool			operator!=( const noMat3 &a ) const;					// exact compare, no epsilon

	void			Zero( void );
	void			Identity( void );
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsRotated( void ) const;

	void			ProjectVector( const noVec3 &src, noVec3 &dst ) const;
	void			UnprojectVector( const noVec3 &src, noVec3 &dst ) const;

	bool			FixDegeneracies( void );	// fix degenerate axial cases
	bool			FixDenormals( void );		// change tiny numbers to zero

	float			Trace( void ) const;
	float			Determinant( void ) const;
	noMat3			OrthoNormalize( void ) const;
	noMat3 &		OrthoNormalizeSelf( void );
	noMat3			Transpose( void ) const;	// returns transpose
	noMat3 &		TransposeSelf( void );
	noMat3			Inverse( void ) const;		// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseSelf( void );		// returns false if determinant is zero
	noMat3			InverseFast( void ) const;	// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseFastSelf( void );	// returns false if determinant is zero
	noMat3			TransposeMultiply( const noMat3 &b ) const;

	noMat3			InertiaTranslate( const float mass, const noVec3 &centerOfMass, const noVec3 &translation ) const;
	noMat3 &		InertiaTranslateSelf( const float mass, const noVec3 &centerOfMass, const noVec3 &translation );
	noMat3			InertiaRotate( const noMat3 &rotation ) const;
	noMat3 &		InertiaRotateSelf( const noMat3 &rotation );

	int				GetDimension( void ) const;

	/*idAngles		ToAngles( void ) const;
	idQuat			ToQuat( void ) const;
	idCQuat			ToCQuat( void ) const;
	idRotation		ToRotation( void ) const;*/
	noMat4			ToMat4( void ) const;
	noVec3			ToAngularVelocity( void ) const;
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

	friend void		TransposeMultiply( const noMat3 &inv, const noMat3 &b, noMat3 &dst );
	friend noMat3	SkewSymmetric( noVec3 const &src );

private:
	noVec3			mat[ 3 ];
};

extern noMat3 mat3_zero;
extern noMat3 mat3_identity;
#define mat3_default	mat3_identity

NO_INLINE noMat3::noMat3( void ) {
}

NO_INLINE noMat3::noMat3( const noVec3 &x, const noVec3 &y, const noVec3 &z ) {
	mat[ 0 ].x = x.x; mat[ 0 ].y = x.y; mat[ 0 ].z = x.z;
	mat[ 1 ].x = y.x; mat[ 1 ].y = y.y; mat[ 1 ].z = y.z;
	mat[ 2 ].x = z.x; mat[ 2 ].y = z.y; mat[ 2 ].z = z.z;
}

NO_INLINE noMat3::noMat3( const float xx, const float xy, const float xz, const float yx, const float yy, const float yz, const float zx, const float zy, const float zz ) {
	mat[ 0 ].x = xx; mat[ 0 ].y = xy; mat[ 0 ].z = xz;
	mat[ 1 ].x = yx; mat[ 1 ].y = yy; mat[ 1 ].z = yz;
	mat[ 2 ].x = zx; mat[ 2 ].y = zy; mat[ 2 ].z = zz;
}

NO_INLINE noMat3::noMat3( const float src[ 3 ][ 3 ] ) {
	memcpy( mat, src, 3 * 3 * sizeof( float ) );
}

NO_INLINE const noVec3 &noMat3::operator[]( int index ) const {
	//assert( ( index >= 0 ) && ( index < 3 ) );
	return mat[ index ];
}

NO_INLINE noVec3 &noMat3::operator[]( int index ) {
	//assert( ( index >= 0 ) && ( index < 3 ) );
	return mat[ index ];
}

NO_INLINE noMat3 noMat3::operator-() const {
	return noMat3(	-mat[0][0], -mat[0][1], -mat[0][2],
		-mat[1][0], -mat[1][1], -mat[1][2],
		-mat[2][0], -mat[2][1], -mat[2][2] );
}

NO_INLINE noVec3 noMat3::operator*( const noVec3 &vec ) const {
	return noVec3(
		mat[ 0 ].x * vec.x + mat[ 1 ].x * vec.y + mat[ 2 ].x * vec.z,
		mat[ 0 ].y * vec.x + mat[ 1 ].y * vec.y + mat[ 2 ].y * vec.z,
		mat[ 0 ].z * vec.x + mat[ 1 ].z * vec.y + mat[ 2 ].z * vec.z );
}

NO_INLINE noMat3 noMat3::operator*( const noMat3 &a ) const {
	int i, j;
	const float *m1Ptr, *m2Ptr;
	float *dstPtr;
	noMat3 dst;

	m1Ptr = reinterpret_cast<const float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);
	dstPtr = reinterpret_cast<float *>(&dst);

	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			*dstPtr = m1Ptr[0] * m2Ptr[ 0 * 3 + j ]
			+ m1Ptr[1] * m2Ptr[ 1 * 3 + j ]
			+ m1Ptr[2] * m2Ptr[ 2 * 3 + j ];
			dstPtr++;
		}
		m1Ptr += 3;
	}
	return dst;
}

NO_INLINE noMat3 noMat3::operator*( const float a ) const {
	return noMat3(
		mat[0].x * a, mat[0].y * a, mat[0].z * a,
		mat[1].x * a, mat[1].y * a, mat[1].z * a,
		mat[2].x * a, mat[2].y * a, mat[2].z * a );
}

NO_INLINE noMat3 noMat3::operator+( const noMat3 &a ) const {
	return noMat3(
		mat[0].x + a[0].x, mat[0].y + a[0].y, mat[0].z + a[0].z,
		mat[1].x + a[1].x, mat[1].y + a[1].y, mat[1].z + a[1].z,
		mat[2].x + a[2].x, mat[2].y + a[2].y, mat[2].z + a[2].z );
}

NO_INLINE noMat3 noMat3::operator-( const noMat3 &a ) const {
	return noMat3(
		mat[0].x - a[0].x, mat[0].y - a[0].y, mat[0].z - a[0].z,
		mat[1].x - a[1].x, mat[1].y - a[1].y, mat[1].z - a[1].z,
		mat[2].x - a[2].x, mat[2].y - a[2].y, mat[2].z - a[2].z );
}

NO_INLINE noMat3 &noMat3::operator*=( const float a ) {
	mat[0].x *= a; mat[0].y *= a; mat[0].z *= a;
	mat[1].x *= a; mat[1].y *= a; mat[1].z *= a; 
	mat[2].x *= a; mat[2].y *= a; mat[2].z *= a;

	return *this;
}

NO_INLINE noMat3 &noMat3::operator*=( const noMat3 &a ) {
	int i, j;
	const float *m2Ptr;
	float *m1Ptr, dst[3];

	m1Ptr = reinterpret_cast<float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);

	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			dst[j]  = m1Ptr[0] * m2Ptr[ 0 * 3 + j ]
			+ m1Ptr[1] * m2Ptr[ 1 * 3 + j ]
			+ m1Ptr[2] * m2Ptr[ 2 * 3 + j ];
		}
		m1Ptr[0] = dst[0]; m1Ptr[1] = dst[1]; m1Ptr[2] = dst[2];
		m1Ptr += 3;
	}
	return *this;
}

NO_INLINE noMat3 &noMat3::operator+=( const noMat3 &a ) {
	mat[0].x += a[0].x; mat[0].y += a[0].y; mat[0].z += a[0].z;
	mat[1].x += a[1].x; mat[1].y += a[1].y; mat[1].z += a[1].z;
	mat[2].x += a[2].x; mat[2].y += a[2].y; mat[2].z += a[2].z;

	return *this;
}

NO_INLINE noMat3 &noMat3::operator-=( const noMat3 &a ) {
	mat[0].x -= a[0].x; mat[0].y -= a[0].y; mat[0].z -= a[0].z;
	mat[1].x -= a[1].x; mat[1].y -= a[1].y; mat[1].z -= a[1].z;
	mat[2].x -= a[2].x; mat[2].y -= a[2].y; mat[2].z -= a[2].z;

	return *this;
}

NO_INLINE noVec3 operator*( const noVec3 &vec, const noMat3 &mat ) {
	return mat * vec;
}

NO_INLINE noMat3 operator*( const float a, const noMat3 &mat ) {
	return mat * a;
}

NO_INLINE noVec3 &operator*=( noVec3 &vec, const noMat3 &mat ) {
	float x = mat[ 0 ].x * vec.x + mat[ 1 ].x * vec.y + mat[ 2 ].x * vec.z;
	float y = mat[ 0 ].y * vec.x + mat[ 1 ].y * vec.y + mat[ 2 ].y * vec.z;
	vec.z = mat[ 0 ].z * vec.x + mat[ 1 ].z * vec.y + mat[ 2 ].z * vec.z;
	vec.x = x;
	vec.y = y;
	return vec;
}

NO_INLINE bool noMat3::Compare( const noMat3 &a ) const {
	if ( mat[0].Compare( a[0] ) &&
		mat[1].Compare( a[1] ) &&
		mat[2].Compare( a[2] ) ) {
			return true;
	}
	return false;
}

NO_INLINE bool noMat3::Compare( const noMat3 &a, const float epsilon ) const {
	if ( mat[0].Compare( a[0], epsilon ) &&
		mat[1].Compare( a[1], epsilon ) &&
		mat[2].Compare( a[2], epsilon ) ) {
			return true;
	}
	return false;
}

NO_INLINE bool noMat3::operator==( const noMat3 &a ) const {
	return Compare( a );
}

NO_INLINE bool noMat3::operator!=( const noMat3 &a ) const {
	return !Compare( a );
}

NO_INLINE void noMat3::Zero( void ) {
	memset( mat, 0, sizeof( noMat3 ) );
}

NO_INLINE void noMat3::Identity( void ) {
	*this = mat3_identity;
}

NO_INLINE bool noMat3::IsIdentity( const float epsilon ) const {
	return Compare( mat3_identity, epsilon );
}

NO_INLINE bool noMat3::IsSymmetric( const float epsilon ) const {
	if ( noMath::Fabs( mat[0][1] - mat[1][0] ) > epsilon ) {
		return false;
	}
	if ( noMath::Fabs( mat[0][2] - mat[2][0] ) > epsilon ) {
		return false;
	}
	if ( noMath::Fabs( mat[1][2] - mat[2][1] ) > epsilon ) {
		return false;
	}
	return true;
}

NO_INLINE bool noMat3::IsDiagonal( const float epsilon ) const {
	if ( noMath::Fabs( mat[0][1] ) > epsilon ||
		noMath::Fabs( mat[0][2] ) > epsilon ||
		noMath::Fabs( mat[1][0] ) > epsilon ||
		noMath::Fabs( mat[1][2] ) > epsilon ||
		noMath::Fabs( mat[2][0] ) > epsilon ||
		noMath::Fabs( mat[2][1] ) > epsilon ) {
			return false;
	}
	return true;
}

NO_INLINE bool noMat3::IsRotated( void ) const {
	return !Compare( mat3_identity );
}

NO_INLINE void noMat3::ProjectVector( const noVec3 &src, noVec3 &dst ) const {
	dst.x = src * mat[ 0 ];
	dst.y = src * mat[ 1 ];
	dst.z = src * mat[ 2 ];
}

NO_INLINE void noMat3::UnprojectVector( const noVec3 &src, noVec3 &dst ) const {
	dst = mat[ 0 ] * src.x + mat[ 1 ] * src.y + mat[ 2 ] * src.z;
}

NO_INLINE bool noMat3::FixDegeneracies( void ) {
	bool r = mat[0].FixDegenerateNormal();
	r |= mat[1].FixDegenerateNormal();
	r |= mat[2].FixDegenerateNormal();
	return r;
}

NO_INLINE bool noMat3::FixDenormals( void ) {
	bool r = mat[0].FixDenormals();
	r |= mat[1].FixDenormals();
	r |= mat[2].FixDenormals();
	return r;
}

NO_INLINE float noMat3::Trace( void ) const {
	return ( mat[0][0] + mat[1][1] + mat[2][2] );
}

NO_INLINE noMat3 noMat3::OrthoNormalize( void ) const {
	noMat3 ortho;

	ortho = *this;
	ortho[ 0 ].Normalize();
	ortho[ 2 ].Cross( mat[ 0 ], mat[ 1 ] );
	ortho[ 2 ].Normalize();
	ortho[ 1 ].Cross( mat[ 2 ], mat[ 0 ] );
	ortho[ 1 ].Normalize();
	return ortho;
}

NO_INLINE noMat3 &noMat3::OrthoNormalizeSelf( void ) {
	mat[ 0 ].Normalize();
	mat[ 2 ].Cross( mat[ 0 ], mat[ 1 ] );
	mat[ 2 ].Normalize();
	mat[ 1 ].Cross( mat[ 2 ], mat[ 0 ] );
	mat[ 1 ].Normalize();
	return *this;
}

NO_INLINE noMat3 noMat3::Transpose( void ) const {
	return noMat3(	mat[0][0], mat[1][0], mat[2][0],
		mat[0][1], mat[1][1], mat[2][1],
		mat[0][2], mat[1][2], mat[2][2] );
}

NO_INLINE noMat3 &noMat3::TransposeSelf( void ) {
	float tmp0, tmp1, tmp2;

	tmp0 = mat[0][1];
	mat[0][1] = mat[1][0];
	mat[1][0] = tmp0;
	tmp1 = mat[0][2];
	mat[0][2] = mat[2][0];
	mat[2][0] = tmp1;
	tmp2 = mat[1][2];
	mat[1][2] = mat[2][1];
	mat[2][1] = tmp2;

	return *this;
}

NO_INLINE noMat3 noMat3::Inverse( void ) const {
	noMat3 invMat;

	invMat = *this;
	int r = invMat.InverseSelf();
	assert( r );
	return invMat;
}

NO_INLINE noMat3 noMat3::InverseFast( void ) const {
	noMat3 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf();
	assert( r );
	return invMat;
}

NO_INLINE noMat3 noMat3::TransposeMultiply( const noMat3 &b ) const {
	return noMat3(	mat[0].x * b[0].x + mat[1].x * b[1].x + mat[2].x * b[2].x,
		mat[0].x * b[0].y + mat[1].x * b[1].y + mat[2].x * b[2].y,
		mat[0].x * b[0].z + mat[1].x * b[1].z + mat[2].x * b[2].z,
		mat[0].y * b[0].x + mat[1].y * b[1].x + mat[2].y * b[2].x,
		mat[0].y * b[0].y + mat[1].y * b[1].y + mat[2].y * b[2].y,
		mat[0].y * b[0].z + mat[1].y * b[1].z + mat[2].y * b[2].z,
		mat[0].z * b[0].x + mat[1].z * b[1].x + mat[2].z * b[2].x,
		mat[0].z * b[0].y + mat[1].z * b[1].y + mat[2].z * b[2].y,
		mat[0].z * b[0].z + mat[1].z * b[1].z + mat[2].z * b[2].z );
}

NO_INLINE void TransposeMultiply( const noMat3 &transpose, const noMat3 &b, noMat3 &dst ) {
	dst[0].x = transpose[0].x * b[0].x + transpose[1].x * b[1].x + transpose[2].x * b[2].x;
	dst[0].y = transpose[0].x * b[0].y + transpose[1].x * b[1].y + transpose[2].x * b[2].y;
	dst[0].z = transpose[0].x * b[0].z + transpose[1].x * b[1].z + transpose[2].x * b[2].z;
	dst[1].x = transpose[0].y * b[0].x + transpose[1].y * b[1].x + transpose[2].y * b[2].x;
	dst[1].y = transpose[0].y * b[0].y + transpose[1].y * b[1].y + transpose[2].y * b[2].y;
	dst[1].z = transpose[0].y * b[0].z + transpose[1].y * b[1].z + transpose[2].y * b[2].z;
	dst[2].x = transpose[0].z * b[0].x + transpose[1].z * b[1].x + transpose[2].z * b[2].x;
	dst[2].y = transpose[0].z * b[0].y + transpose[1].z * b[1].y + transpose[2].z * b[2].y;
	dst[2].z = transpose[0].z * b[0].z + transpose[1].z * b[1].z + transpose[2].z * b[2].z;
}

NO_INLINE noMat3 SkewSymmetric( noVec3 const &src ) {
	return noMat3( 0.0f, -src.z,  src.y, src.z,   0.0f, -src.x, -src.y,  src.x,   0.0f );
}

NO_INLINE int noMat3::GetDimension( void ) const {
	return 9;
}

NO_INLINE const float *noMat3::ToFloatPtr( void ) const {
	return mat[0].ToFloatPtr();
}

NO_INLINE float *noMat3::ToFloatPtr( void ) {
	return mat[0].ToFloatPtr();
}

//===============================================================
//
//	noMat4 - 4x4 matrix
//
//===============================================================

class noMat4 {
public:
	noMat4( void );
	explicit noMat4( const noVec4 &x, const noVec4 &y, const noVec4 &z, const noVec4 &w );
	explicit noMat4(const float xx, const float xy, const float xz, const float xw,
		const float yx, const float yy, const float yz, const float yw,
		const float zx, const float zy, const float zz, const float zw,
		const float wx, const float wy, const float wz, const float ww );
	explicit noMat4( const noMat3 &rotation, const noVec3 &translation );
	explicit noMat4( const float src[ 4 ][ 4 ] );

	const noVec4 &	operator[]( int index ) const;
	noVec4 &		operator[]( int index );
	noMat4			operator*( const float a ) const;
	noVec4			operator*( const noVec4 &vec ) const;
	noVec3			operator*( const noVec3 &vec ) const;
	noMat4			operator*( const noMat4 &a ) const;
	noMat4			operator+( const noMat4 &a ) const;
	noMat4			operator-( const noMat4 &a ) const;
	noMat4 &		operator*=( const float a );
	noMat4 &		operator*=( const noMat4 &a );
	noMat4 &		operator+=( const noMat4 &a );
	noMat4 &		operator-=( const noMat4 &a );

	friend noMat4	operator*( const float a, const noMat4 &mat );
	friend noVec4	operator*( const noVec4 &vec, const noMat4 &mat );
	friend noVec3	operator*( const noVec3 &vec, const noMat4 &mat );
	friend noVec4 &	operator*=( noVec4 &vec, const noMat4 &mat );
	friend noVec3 &	operator*=( noVec3 &vec, const noMat4 &mat );

	bool			Compare( const noMat4 &a ) const;						// exact compare, no epsilon
	bool			Compare( const noMat4 &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==( const noMat4 &a ) const;					// exact compare, no epsilon
	bool			operator!=( const noMat4 &a ) const;					// exact compare, no epsilon

	void			Zero( void );
	void			Identity( void );
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsRotated( void ) const;

	void			ProjectVector( const noVec4 &src, noVec4 &dst ) const;
	void			UnprojectVector( const noVec4 &src, noVec4 &dst ) const;

	float			Trace( void ) const;
	float			Determinant( void ) const;
	noMat4			Transpose( void ) const;	// returns transpose
	noMat4 &		TransposeSelf( void );
	noMat4			Inverse( void ) const;		// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseSelf( void );		// returns false if determinant is zero
	noMat4			InverseFast( void ) const;	// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseFastSelf( void );	// returns false if determinant is zero
	noMat4			TransposeMultiply( const noMat4 &b ) const;

	int				GetDimension( void ) const;

	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

private:
	noVec4			mat[ 4 ];
};

extern noMat4 mat4_zero;
extern noMat4 mat4_identity;
#define mat4_default	mat4_identity

NO_INLINE noMat4::noMat4( void ) {
}

NO_INLINE noMat4::noMat4( const noVec4 &x, const noVec4 &y, const noVec4 &z, const noVec4 &w ) {
	mat[ 0 ] = x;
	mat[ 1 ] = y;
	mat[ 2 ] = z;
	mat[ 3 ] = w;
}

NO_INLINE noMat4::noMat4( const float xx, const float xy, const float xz, const float xw,
							const float yx, const float yy, const float yz, const float yw,
							const float zx, const float zy, const float zz, const float zw,
							const float wx, const float wy, const float wz, const float ww ) {
	mat[0][0] = xx; mat[0][1] = xy; mat[0][2] = xz; mat[0][3] = xw;
	mat[1][0] = yx; mat[1][1] = yy; mat[1][2] = yz; mat[1][3] = yw;
	mat[2][0] = zx; mat[2][1] = zy; mat[2][2] = zz; mat[2][3] = zw;
	mat[3][0] = wx; mat[3][1] = wy; mat[3][2] = wz; mat[3][3] = ww;
}

NO_INLINE noMat4::noMat4( const noMat3 &rotation, const noVec3 &translation ) {
	// NOTE: noMat3 is transposed because it is column-major
	mat[ 0 ][ 0 ] = rotation[0][0];
	mat[ 0 ][ 1 ] = rotation[1][0];
	mat[ 0 ][ 2 ] = rotation[2][0];
	mat[ 0 ][ 3 ] = translation[0];
	mat[ 1 ][ 0 ] = rotation[0][1];
	mat[ 1 ][ 1 ] = rotation[1][1];
	mat[ 1 ][ 2 ] = rotation[2][1];
	mat[ 1 ][ 3 ] = translation[1];
	mat[ 2 ][ 0 ] = rotation[0][2];
	mat[ 2 ][ 1 ] = rotation[1][2];
	mat[ 2 ][ 2 ] = rotation[2][2];
	mat[ 2 ][ 3 ] = translation[2];
	mat[ 3 ][ 0 ] = 0.0f;
	mat[ 3 ][ 1 ] = 0.0f;
	mat[ 3 ][ 2 ] = 0.0f;
	mat[ 3 ][ 3 ] = 1.0f;
}

NO_INLINE noMat4::noMat4( const float src[ 4 ][ 4 ] ) {
	memcpy( mat, src, 4 * 4 * sizeof( float ) );
}

NO_INLINE const noVec4 &noMat4::operator[]( int index ) const {
	//assert( ( index >= 0 ) && ( index < 4 ) );
	return mat[ index ];
}

NO_INLINE noVec4 &noMat4::operator[]( int index ) {
	//assert( ( index >= 0 ) && ( index < 4 ) );
	return mat[ index ];
}

NO_INLINE noMat4 noMat4::operator*( const float a ) const {
	return noMat4(
		mat[0].x * a, mat[0].y * a, mat[0].z * a, mat[0].w * a,
		mat[1].x * a, mat[1].y * a, mat[1].z * a, mat[1].w * a,
		mat[2].x * a, mat[2].y * a, mat[2].z * a, mat[2].w * a,
		mat[3].x * a, mat[3].y * a, mat[3].z * a, mat[3].w * a );
}

NO_INLINE noVec4 noMat4::operator*( const noVec4 &vec ) const {
	return noVec4(
		mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z * vec.z + mat[ 0 ].w * vec.w,
		mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z * vec.z + mat[ 1 ].w * vec.w,
		mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].w * vec.w,
		mat[ 3 ].x * vec.x + mat[ 3 ].y * vec.y + mat[ 3 ].z * vec.z + mat[ 3 ].w * vec.w );
}

NO_INLINE noVec3 noMat4::operator*( const noVec3 &vec ) const {
	float s = mat[ 3 ].x * vec.x + mat[ 3 ].y * vec.y + mat[ 3 ].z * vec.z + mat[ 3 ].w;
	if ( s == 0.0f ) {
		return noVec3( 0.0f, 0.0f, 0.0f );
	}
	if ( s == 1.0f ) {
		return noVec3(
			mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z * vec.z + mat[ 0 ].w,
			mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z * vec.z + mat[ 1 ].w,
			mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].w );
	}
	else {
		float invS = 1.0f / s;
		return noVec3(
			(mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z * vec.z + mat[ 0 ].w) * invS,
			(mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z * vec.z + mat[ 1 ].w) * invS,
			(mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].w) * invS );
	}
}

NO_INLINE noMat4 noMat4::operator*( const noMat4 &a ) const {
	int i, j;
	const float *m1Ptr, *m2Ptr;
	float *dstPtr;
	noMat4 dst;

	m1Ptr = reinterpret_cast<const float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);
	dstPtr = reinterpret_cast<float *>(&dst);

	for ( i = 0; i < 4; i++ ) {
		for ( j = 0; j < 4; j++ ) {
			*dstPtr = m1Ptr[0] * m2Ptr[ 0 * 4 + j ]
					+ m1Ptr[1] * m2Ptr[ 1 * 4 + j ]
					+ m1Ptr[2] * m2Ptr[ 2 * 4 + j ]
					+ m1Ptr[3] * m2Ptr[ 3 * 4 + j ];
			dstPtr++;
		}
		m1Ptr += 4;
	}
	return dst;
}

NO_INLINE noMat4 noMat4::operator+( const noMat4 &a ) const {
	return noMat4( 
		mat[0].x + a[0].x, mat[0].y + a[0].y, mat[0].z + a[0].z, mat[0].w + a[0].w,
		mat[1].x + a[1].x, mat[1].y + a[1].y, mat[1].z + a[1].z, mat[1].w + a[1].w,
		mat[2].x + a[2].x, mat[2].y + a[2].y, mat[2].z + a[2].z, mat[2].w + a[2].w,
		mat[3].x + a[3].x, mat[3].y + a[3].y, mat[3].z + a[3].z, mat[3].w + a[3].w );
}
    
NO_INLINE noMat4 noMat4::operator-( const noMat4 &a ) const {
	return noMat4( 
		mat[0].x - a[0].x, mat[0].y - a[0].y, mat[0].z - a[0].z, mat[0].w - a[0].w,
		mat[1].x - a[1].x, mat[1].y - a[1].y, mat[1].z - a[1].z, mat[1].w - a[1].w,
		mat[2].x - a[2].x, mat[2].y - a[2].y, mat[2].z - a[2].z, mat[2].w - a[2].w,
		mat[3].x - a[3].x, mat[3].y - a[3].y, mat[3].z - a[3].z, mat[3].w - a[3].w );
}

NO_INLINE noMat4 &noMat4::operator*=( const float a ) {
	mat[0].x *= a; mat[0].y *= a; mat[0].z *= a; mat[0].w *= a;
	mat[1].x *= a; mat[1].y *= a; mat[1].z *= a; mat[1].w *= a;
	mat[2].x *= a; mat[2].y *= a; mat[2].z *= a; mat[2].w *= a;
	mat[3].x *= a; mat[3].y *= a; mat[3].z *= a; mat[3].w *= a;
    return *this;
}

NO_INLINE noMat4 &noMat4::operator*=( const noMat4 &a ) {
	*this = (*this) * a;
	return *this;
}

NO_INLINE noMat4 &noMat4::operator+=( const noMat4 &a ) {
	mat[0].x += a[0].x; mat[0].y += a[0].y; mat[0].z += a[0].z; mat[0].w += a[0].w;
	mat[1].x += a[1].x; mat[1].y += a[1].y; mat[1].z += a[1].z; mat[1].w += a[1].w;
	mat[2].x += a[2].x; mat[2].y += a[2].y; mat[2].z += a[2].z; mat[2].w += a[2].w;
	mat[3].x += a[3].x; mat[3].y += a[3].y; mat[3].z += a[3].z; mat[3].w += a[3].w;
    return *this;
}

NO_INLINE noMat4 &noMat4::operator-=( const noMat4 &a ) {
	mat[0].x -= a[0].x; mat[0].y -= a[0].y; mat[0].z -= a[0].z; mat[0].w -= a[0].w;
	mat[1].x -= a[1].x; mat[1].y -= a[1].y; mat[1].z -= a[1].z; mat[1].w -= a[1].w;
	mat[2].x -= a[2].x; mat[2].y -= a[2].y; mat[2].z -= a[2].z; mat[2].w -= a[2].w;
	mat[3].x -= a[3].x; mat[3].y -= a[3].y; mat[3].z -= a[3].z; mat[3].w -= a[3].w;
    return *this;
}

NO_INLINE noMat4 operator*( const float a, const noMat4 &mat ) {
	return mat * a;
}

NO_INLINE noVec4 operator*( const noVec4 &vec, const noMat4 &mat ) {
	return mat * vec;
}

NO_INLINE noVec3 operator*( const noVec3 &vec, const noMat4 &mat ) {
	return mat * vec;
}

NO_INLINE noVec4 &operator*=( noVec4 &vec, const noMat4 &mat ) {
	vec = mat * vec;
	return vec;
}

NO_INLINE noVec3 &operator*=( noVec3 &vec, const noMat4 &mat ) {
	vec = mat * vec;
	return vec;
}

NO_INLINE bool noMat4::Compare( const noMat4 &a ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for ( i = 0; i < 4*4; i++ ) {
		if ( ptr1[i] != ptr2[i] ) {
			return false;
		}
	}
	return true;
}

NO_INLINE bool noMat4::Compare( const noMat4 &a, const float epsilon ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for ( i = 0; i < 4*4; i++ ) {
		if ( noMath::Fabs( ptr1[i] - ptr2[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

NO_INLINE bool noMat4::operator==( const noMat4 &a ) const {
	return Compare( a );
}

NO_INLINE bool noMat4::operator!=( const noMat4 &a ) const {
	return !Compare( a );
}

NO_INLINE void noMat4::Zero( void ) {
	memset( mat, 0, sizeof( noMat4 ) );
}

NO_INLINE void noMat4::Identity( void ) {
	*this = mat4_identity;
}

NO_INLINE bool noMat4::IsIdentity( const float epsilon ) const {
	return Compare( mat4_identity, epsilon );
}

NO_INLINE bool noMat4::IsSymmetric( const float epsilon ) const {
	for ( int i = 1; i < 4; i++ ) {
		for ( int j = 0; j < i; j++ ) {
			if ( noMath::Fabs( mat[i][j] - mat[j][i] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

NO_INLINE bool noMat4::IsDiagonal( const float epsilon ) const {
	for ( int i = 0; i < 4; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			if ( i != j && noMath::Fabs( mat[i][j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

NO_INLINE bool noMat4::IsRotated( void ) const {
	if ( !mat[ 0 ][ 1 ] && !mat[ 0 ][ 2 ] &&
		!mat[ 1 ][ 0 ] && !mat[ 1 ][ 2 ] &&
		!mat[ 2 ][ 0 ] && !mat[ 2 ][ 1 ] ) {
		return false;
	}
	return true;
}

NO_INLINE void noMat4::ProjectVector( const noVec4 &src, noVec4 &dst ) const {
	dst.x = src * mat[ 0 ];
	dst.y = src * mat[ 1 ];
	dst.z = src * mat[ 2 ];
	dst.w = src * mat[ 3 ];
}

NO_INLINE void noMat4::UnprojectVector( const noVec4 &src, noVec4 &dst ) const {
	dst = mat[ 0 ] * src.x + mat[ 1 ] * src.y + mat[ 2 ] * src.z + mat[ 3 ] * src.w;
}

NO_INLINE float noMat4::Trace( void ) const {
	return ( mat[0][0] + mat[1][1] + mat[2][2] + mat[3][3] );
}

NO_INLINE noMat4 noMat4::Inverse( void ) const {
	noMat4 invMat;

	invMat = *this;
	int r = invMat.InverseSelf();
	assert( r );
	return invMat;
}

NO_INLINE noMat4 noMat4::InverseFast( void ) const {
	noMat4 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf();
	assert( r );
	return invMat;
}

NO_INLINE noMat4 noMat3::ToMat4( void ) const {
	// NOTE: noMat3 is transposed because it is column-major
	return noMat4(	mat[0][0],	mat[1][0],	mat[2][0],	0.0f,
					mat[0][1],	mat[1][1],	mat[2][1],	0.0f,
					mat[0][2],	mat[1][2],	mat[2][2],	0.0f,
					0.0f,		0.0f,		0.0f,		1.0f );
}

NO_INLINE int noMat4::GetDimension( void ) const {
	return 16;
}

NO_INLINE const float *noMat4::ToFloatPtr( void ) const {
	return mat[0].ToFloatPtr();
}

NO_INLINE float *noMat4::ToFloatPtr( void ) {
	return mat[0].ToFloatPtr();
}



#endif