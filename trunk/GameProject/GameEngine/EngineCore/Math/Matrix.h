#ifndef MATH_MATRIX_H
#define MATH_MATRIX_H

#include "Vector.h"

#define MATRIX_INVERSE_EPSILON		1e-14
#define MATRIX_EPSILON				1e-6


class noAngles;
class noRotation;	
class noQuat;
class idCQuat;
class noMat4;


//===============================================================
//
//	noMat2 - 2x2 matrix
//
//===============================================================

class noMat2 {
public:
	noMat2( void );
	explicit noMat2( const noVec2 &x, const noVec2 &y );
	explicit noMat2( const float xx, const float xy, const float yx, const float yy );
	explicit noMat2( const float src[ 2 ][ 2 ] );

	const noVec2 &	operator[]( int index ) const;
	noVec2 &		operator[]( int index );
	noMat2			operator-() const;
	noMat2			operator*( const float a ) const;
	noVec2			operator*( const noVec2 &vec ) const;
	noMat2			operator*( const noMat2 &a ) const;
	noMat2			operator+( const noMat2 &a ) const;
	noMat2			operator-( const noMat2 &a ) const;
	noMat2 &		operator*=( const float a );
	noMat2 &		operator*=( const noMat2 &a );
	noMat2 &		operator+=( const noMat2 &a );
	noMat2 &		operator-=( const noMat2 &a );

	friend noMat2	operator*( const float a, const noMat2 &mat );
	friend noVec2	operator*( const noVec2 &vec, const noMat2 &mat );
	friend noVec2 &	operator*=( noVec2 &vec, const noMat2 &mat );

	bool			Compare( const noMat2 &a ) const;						// exact compare, no epsilon
	bool			Compare( const noMat2 &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==( const noMat2 &a ) const;					// exact compare, no epsilon
	bool			operator!=( const noMat2 &a ) const;					// exact compare, no epsilon

	void			Zero( void );
	void			Identity( void );
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;

	float			Trace( void ) const;
	float			Determinant( void ) const;
	noMat2			Transpose( void ) const;	// returns transpose
	noMat2 &		TransposeSelf( void );
	noMat2			Inverse( void ) const;		// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseSelf( void );		// returns false if determinant is zero
	noMat2			InverseFast( void ) const;	// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseFastSelf( void );	// returns false if determinant is zero

	int				GetDimension( void ) const;

	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

private:
	noVec2			mat[ 2 ];
};

extern noMat2 mat2_zero;
extern noMat2 mat2_identity;
#define mat2_default	mat2_identity

NO_INLINE noMat2::noMat2( void ) {
}

NO_INLINE noMat2::noMat2( const noVec2 &x, const noVec2 &y ) {
	mat[ 0 ].x = x.x; mat[ 0 ].y = x.y;
	mat[ 1 ].x = y.x; mat[ 1 ].y = y.y;
}

NO_INLINE noMat2::noMat2( const float xx, const float xy, const float yx, const float yy ) {
	mat[ 0 ].x = xx; mat[ 0 ].y = xy;
	mat[ 1 ].x = yx; mat[ 1 ].y = yy;
}

NO_INLINE noMat2::noMat2( const float src[ 2 ][ 2 ] ) {
	memcpy( mat, src, 2 * 2 * sizeof( float ) );
}

NO_INLINE const noVec2 &noMat2::operator[]( int index ) const {
	//assert( ( index >= 0 ) && ( index < 2 ) );
	return mat[ index ];
}

NO_INLINE noVec2 &noMat2::operator[]( int index ) {
	//assert( ( index >= 0 ) && ( index < 2 ) );
	return mat[ index ];
}

NO_INLINE noMat2 noMat2::operator-() const {
	return noMat2(	-mat[0][0], -mat[0][1],
		-mat[1][0], -mat[1][1] );
}

NO_INLINE noVec2 noMat2::operator*( const noVec2 &vec ) const {
	return noVec2(
		mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y,
		mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y );
}

NO_INLINE noMat2 noMat2::operator*( const noMat2 &a ) const {
	return noMat2(
		mat[0].x * a[0].x + mat[0].y * a[1].x,
		mat[0].x * a[0].y + mat[0].y * a[1].y,
		mat[1].x * a[0].x + mat[1].y * a[1].x,
		mat[1].x * a[0].y + mat[1].y * a[1].y );
}

NO_INLINE noMat2 noMat2::operator*( const float a ) const {
	return noMat2(
		mat[0].x * a, mat[0].y * a, 
		mat[1].x * a, mat[1].y * a );
}

NO_INLINE noMat2 noMat2::operator+( const noMat2 &a ) const {
	return noMat2(
		mat[0].x + a[0].x, mat[0].y + a[0].y, 
		mat[1].x + a[1].x, mat[1].y + a[1].y );
}

NO_INLINE noMat2 noMat2::operator-( const noMat2 &a ) const {
	return noMat2(
		mat[0].x - a[0].x, mat[0].y - a[0].y,
		mat[1].x - a[1].x, mat[1].y - a[1].y );
}

NO_INLINE noMat2 &noMat2::operator*=( const float a ) {
	mat[0].x *= a; mat[0].y *= a;
	mat[1].x *= a; mat[1].y *= a;

	return *this;
}

NO_INLINE noMat2 &noMat2::operator*=( const noMat2 &a ) {
	float x, y;
	x = mat[0].x; y = mat[0].y;
	mat[0].x = x * a[0].x + y * a[1].x;
	mat[0].y = x * a[0].y + y * a[1].y;
	x = mat[1].x; y = mat[1].y;
	mat[1].x = x * a[0].x + y * a[1].x;
	mat[1].y = x * a[0].y + y * a[1].y;
	return *this;
}

NO_INLINE noMat2 &noMat2::operator+=( const noMat2 &a ) {
	mat[0].x += a[0].x; mat[0].y += a[0].y;
	mat[1].x += a[1].x; mat[1].y += a[1].y;

	return *this;
}

NO_INLINE noMat2 &noMat2::operator-=( const noMat2 &a ) {
	mat[0].x -= a[0].x; mat[0].y -= a[0].y;
	mat[1].x -= a[1].x; mat[1].y -= a[1].y;

	return *this;
}

NO_INLINE noVec2 operator*( const noVec2 &vec, const noMat2 &mat ) {
	return mat * vec;
}

NO_INLINE noMat2 operator*( const float a, noMat2 const &mat ) {
	return mat * a;
}

NO_INLINE noVec2 &operator*=( noVec2 &vec, const noMat2 &mat ) {
	vec = mat * vec;
	return vec;
}

NO_INLINE bool noMat2::Compare( const noMat2 &a ) const {
	if ( mat[0].Compare( a[0] ) &&
		mat[1].Compare( a[1] ) ) {
			return true;
	}
	return false;
}

NO_INLINE bool noMat2::Compare( const noMat2 &a, const float epsilon ) const {
	if ( mat[0].Compare( a[0], epsilon ) &&
		mat[1].Compare( a[1], epsilon ) ) {
			return true;
	}
	return false;
}

NO_INLINE bool noMat2::operator==( const noMat2 &a ) const {
	return Compare( a );
}

NO_INLINE bool noMat2::operator!=( const noMat2 &a ) const {
	return !Compare( a );
}

NO_INLINE void noMat2::Zero( void ) {
	mat[0].Zero();
	mat[1].Zero();
}

NO_INLINE void noMat2::Identity( void ) {
	*this = mat2_identity;
}

NO_INLINE bool noMat2::IsIdentity( const float epsilon ) const {
	return Compare( mat2_identity, epsilon );
}

NO_INLINE bool noMat2::IsSymmetric( const float epsilon ) const {
	return ( noMath::Fabs( mat[0][1] - mat[1][0] ) < epsilon );
}

NO_INLINE bool noMat2::IsDiagonal( const float epsilon ) const {
	if ( noMath::Fabs( mat[0][1] ) > epsilon ||
		noMath::Fabs( mat[1][0] ) > epsilon ) {
			return false;
	}
	return true;
}

NO_INLINE float noMat2::Trace( void ) const {
	return ( mat[0][0] + mat[1][1] );
}

NO_INLINE float noMat2::Determinant( void ) const {
	return mat[0][0] * mat[1][1] - mat[0][1] * mat[1][0];
}

NO_INLINE noMat2 noMat2::Transpose( void ) const {
	return noMat2(	mat[0][0], mat[1][0],
		mat[0][1], mat[1][1] );
}

NO_INLINE noMat2 &noMat2::TransposeSelf( void ) {
	float tmp;

	tmp = mat[0][1];
	mat[0][1] = mat[1][0];
	mat[1][0] = tmp;

	return *this;
}

NO_INLINE noMat2 noMat2::Inverse( void ) const {
	noMat2 invMat;

	invMat = *this;
	int r = invMat.InverseSelf();
	assert( r );
	return invMat;
}

NO_INLINE noMat2 noMat2::InverseFast( void ) const {
	noMat2 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf();
	assert( r );
	return invMat;
}

NO_INLINE int noMat2::GetDimension( void ) const {
	return 4;
}

NO_INLINE const float *noMat2::ToFloatPtr( void ) const {
	return mat[0].ToFloatPtr();
}

NO_INLINE float *noMat2::ToFloatPtr( void ) {
	return mat[0].ToFloatPtr();
}


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

	noAngles		ToAngles( void ) const;
	noQuat			ToQuat( void ) const;
	idCQuat			ToCQuat( void ) const;
	noRotation		ToRotation( void ) const;
	noMat4			ToMat4( void ) const;
	noVec3			ToAngularVelocity( void ) const;
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

	friend void		TransposeMultiply( const noMat3 &inv, const noMat3 &b, noMat3 &dst );
	friend noMat3	SkewSymmetric( noVec3 const &src );
	noVec3			mat[ 3 ];

private:
};

extern noMat3 mat3_zero;
extern noMat3 mat3_identity;
#define mat3_default	mat3_identity

__forceinline noMat3::noMat3( void ) {
}

__forceinline noMat3::noMat3( const noVec3 &x, const noVec3 &y, const noVec3 &z ) {
	mat[ 0 ].x = x.x; mat[ 0 ].y = x.y; mat[ 0 ].z = x.z;
	mat[ 1 ].x = y.x; mat[ 1 ].y = y.y; mat[ 1 ].z = y.z;
	mat[ 2 ].x = z.x; mat[ 2 ].y = z.y; mat[ 2 ].z = z.z;
}

__forceinline noMat3::noMat3( const float xx, const float xy, const float xz, const float yx, const float yy, const float yz, const float zx, const float zy, const float zz ) {
	mat[ 0 ].x = xx; mat[ 0 ].y = xy; mat[ 0 ].z = xz;
	mat[ 1 ].x = yx; mat[ 1 ].y = yy; mat[ 1 ].z = yz;
	mat[ 2 ].x = zx; mat[ 2 ].y = zy; mat[ 2 ].z = zz;
}

__forceinline noMat3::noMat3( const float src[ 3 ][ 3 ] ) {
	memcpy( mat, src, 3 * 3 * sizeof( float ) );
}

__forceinline const noVec3 &noMat3::operator[]( int index ) const {
	//assert( ( index >= 0 ) && ( index < 3 ) );
	return mat[ index ];
}

__forceinline noVec3 &noMat3::operator[]( int index ) {
	//assert( ( index >= 0 ) && ( index < 3 ) );
	return mat[ index ];
}

__forceinline noMat3 noMat3::operator-() const {
	return noMat3(	-mat[0][0], -mat[0][1], -mat[0][2],
		-mat[1][0], -mat[1][1], -mat[1][2],
		-mat[2][0], -mat[2][1], -mat[2][2] );
}

__forceinline noVec3 noMat3::operator*( const noVec3 &vec ) const {
	return noVec3(
		mat[ 0 ].x * vec.x + mat[ 1 ].x * vec.y + mat[ 2 ].x * vec.z,
		mat[ 0 ].y * vec.x + mat[ 1 ].y * vec.y + mat[ 2 ].y * vec.z,
		mat[ 0 ].z * vec.x + mat[ 1 ].z * vec.y + mat[ 2 ].z * vec.z );
}

__forceinline noMat3 noMat3::operator*( const noMat3 &a ) const {
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

__forceinline noMat3 noMat3::operator*( const float a ) const {
	return noMat3(
		mat[0].x * a, mat[0].y * a, mat[0].z * a,
		mat[1].x * a, mat[1].y * a, mat[1].z * a,
		mat[2].x * a, mat[2].y * a, mat[2].z * a );
}

__forceinline noMat3 noMat3::operator+( const noMat3 &a ) const {
	return noMat3(
		mat[0].x + a[0].x, mat[0].y + a[0].y, mat[0].z + a[0].z,
		mat[1].x + a[1].x, mat[1].y + a[1].y, mat[1].z + a[1].z,
		mat[2].x + a[2].x, mat[2].y + a[2].y, mat[2].z + a[2].z );
}

__forceinline noMat3 noMat3::operator-( const noMat3 &a ) const {
	return noMat3(
		mat[0].x - a[0].x, mat[0].y - a[0].y, mat[0].z - a[0].z,
		mat[1].x - a[1].x, mat[1].y - a[1].y, mat[1].z - a[1].z,
		mat[2].x - a[2].x, mat[2].y - a[2].y, mat[2].z - a[2].z );
}

__forceinline noMat3 &noMat3::operator*=( const float a ) {
	mat[0].x *= a; mat[0].y *= a; mat[0].z *= a;
	mat[1].x *= a; mat[1].y *= a; mat[1].z *= a; 
	mat[2].x *= a; mat[2].y *= a; mat[2].z *= a;

	return *this;
}

__forceinline noMat3 &noMat3::operator*=( const noMat3 &a ) {
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

__forceinline noMat3 &noMat3::operator+=( const noMat3 &a ) {
	mat[0].x += a[0].x; mat[0].y += a[0].y; mat[0].z += a[0].z;
	mat[1].x += a[1].x; mat[1].y += a[1].y; mat[1].z += a[1].z;
	mat[2].x += a[2].x; mat[2].y += a[2].y; mat[2].z += a[2].z;

	return *this;
}

__forceinline noMat3 &noMat3::operator-=( const noMat3 &a ) {
	mat[0].x -= a[0].x; mat[0].y -= a[0].y; mat[0].z -= a[0].z;
	mat[1].x -= a[1].x; mat[1].y -= a[1].y; mat[1].z -= a[1].z;
	mat[2].x -= a[2].x; mat[2].y -= a[2].y; mat[2].z -= a[2].z;

	return *this;
}

__forceinline noVec3 operator*( const noVec3 &vec, const noMat3 &mat ) {
	return mat * vec;
}

__forceinline noMat3 operator*( const float a, const noMat3 &mat ) {
	return mat * a;
}

__forceinline noVec3 &operator*=( noVec3 &vec, const noMat3 &mat ) {
	float x = mat[ 0 ].x * vec.x + mat[ 1 ].x * vec.y + mat[ 2 ].x * vec.z;
	float y = mat[ 0 ].y * vec.x + mat[ 1 ].y * vec.y + mat[ 2 ].y * vec.z;
	vec.z = mat[ 0 ].z * vec.x + mat[ 1 ].z * vec.y + mat[ 2 ].z * vec.z;
	vec.x = x;
	vec.y = y;
	return vec;
}

__forceinline bool noMat3::Compare( const noMat3 &a ) const {
	if ( mat[0].Compare( a[0] ) &&
		mat[1].Compare( a[1] ) &&
		mat[2].Compare( a[2] ) ) {
			return true;
	}
	return false;
}

__forceinline bool noMat3::Compare( const noMat3 &a, const float epsilon ) const {
	if ( mat[0].Compare( a[0], epsilon ) &&
		mat[1].Compare( a[1], epsilon ) &&
		mat[2].Compare( a[2], epsilon ) ) {
			return true;
	}
	return false;
}

__forceinline bool noMat3::operator==( const noMat3 &a ) const {
	return Compare( a );
}

__forceinline bool noMat3::operator!=( const noMat3 &a ) const {
	return !Compare( a );
}

__forceinline void noMat3::Zero( void ) {
	memset( mat, 0, sizeof( noMat3 ) );
}

__forceinline void noMat3::Identity( void ) {
	*this = mat3_identity;
}

__forceinline bool noMat3::IsIdentity( const float epsilon ) const {
	return Compare( mat3_identity, epsilon );
}

__forceinline bool noMat3::IsSymmetric( const float epsilon ) const {
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

__forceinline bool noMat3::IsDiagonal( const float epsilon ) const {
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

__forceinline bool noMat3::IsRotated( void ) const {
	return !Compare( mat3_identity );
}

__forceinline void noMat3::ProjectVector( const noVec3 &src, noVec3 &dst ) const {
	dst.x = src * mat[ 0 ];
	dst.y = src * mat[ 1 ];
	dst.z = src * mat[ 2 ];
}

__forceinline void noMat3::UnprojectVector( const noVec3 &src, noVec3 &dst ) const {
	dst = mat[ 0 ] * src.x + mat[ 1 ] * src.y + mat[ 2 ] * src.z;
}

__forceinline bool noMat3::FixDegeneracies( void ) {
	bool r = mat[0].FixDegenerateNormal();
	r |= mat[1].FixDegenerateNormal();
	r |= mat[2].FixDegenerateNormal();
	return r;
}

__forceinline bool noMat3::FixDenormals( void ) {
	bool r = mat[0].FixDenormals();
	r |= mat[1].FixDenormals();
	r |= mat[2].FixDenormals();
	return r;
}

__forceinline float noMat3::Trace( void ) const {
	return ( mat[0][0] + mat[1][1] + mat[2][2] );
}

__forceinline noMat3 noMat3::OrthoNormalize( void ) const {
	noMat3 ortho;

	ortho = *this;
	ortho[ 0 ].Normalize();
	ortho[ 2 ].Cross( mat[ 0 ], mat[ 1 ] );
	ortho[ 2 ].Normalize();
	ortho[ 1 ].Cross( mat[ 2 ], mat[ 0 ] );
	ortho[ 1 ].Normalize();
	return ortho;
}

__forceinline noMat3 &noMat3::OrthoNormalizeSelf( void ) {
	mat[ 0 ].Normalize();
	mat[ 2 ].Cross( mat[ 0 ], mat[ 1 ] );
	mat[ 2 ].Normalize();
	mat[ 1 ].Cross( mat[ 2 ], mat[ 0 ] );
	mat[ 1 ].Normalize();
	return *this;
}

__forceinline noMat3 noMat3::Transpose( void ) const {
	return noMat3(	mat[0][0], mat[1][0], mat[2][0],
		mat[0][1], mat[1][1], mat[2][1],
		mat[0][2], mat[1][2], mat[2][2] );
}

__forceinline noMat3 &noMat3::TransposeSelf( void ) {
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

__forceinline noMat3 noMat3::Inverse( void ) const {
	noMat3 invMat;

	invMat = *this;
	int r = invMat.InverseSelf();
	assert( r );
	return invMat;
}

__forceinline noMat3 noMat3::InverseFast( void ) const {
	noMat3 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf();
	assert( r );
	return invMat;
}

__forceinline noMat3 noMat3::TransposeMultiply( const noMat3 &b ) const {
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

__forceinline void TransposeMultiply( const noMat3 &transpose, const noMat3 &b, noMat3 &dst ) {
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

__forceinline noMat3 SkewSymmetric( noVec3 const &src ) {
	return noMat3( 0.0f, -src.z,  src.y, src.z,   0.0f, -src.x, -src.y,  src.x,   0.0f );
}

__forceinline int noMat3::GetDimension( void ) const {
	return 9;
}

__forceinline const float *noMat3::ToFloatPtr( void ) const {
	return mat[0].ToFloatPtr();
}

__forceinline float *noMat3::ToFloatPtr( void ) {
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
	explicit noMat4( const float src[ 16 ] );

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

	void			Translation(const noVec3& t);

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
	 void			Scale( const noVec3& t );

	 void			SetOrigin(const noVec3& v);
	 noVec3			GetOrigin() const;
	 void			RemoveScaling(FLOAT Tolerance =SMALL_NUMBER);

	 noVec4			mat[ 4 ];
private:
};

extern noMat4 mat4_zero;
extern noMat4 mat4_identity;
#define mat4_default	mat4_identity

__forceinline noMat4::noMat4( void ) {
}

__forceinline noMat4::noMat4( const noVec4 &x, const noVec4 &y, const noVec4 &z, const noVec4 &w ) {
	mat[ 0 ] = x;
	mat[ 1 ] = y;
	mat[ 2 ] = z;
	mat[ 3 ] = w;
}

__forceinline noMat4::noMat4( const float xx, const float xy, const float xz, const float xw,
							const float yx, const float yy, const float yz, const float yw,
							const float zx, const float zy, const float zz, const float zw,
							const float wx, const float wy, const float wz, const float ww ) {
	mat[0][0] = xx; mat[0][1] = xy; mat[0][2] = xz; mat[0][3] = xw;
	mat[1][0] = yx; mat[1][1] = yy; mat[1][2] = yz; mat[1][3] = yw;
	mat[2][0] = zx; mat[2][1] = zy; mat[2][2] = zz; mat[2][3] = zw;
	mat[3][0] = wx; mat[3][1] = wy; mat[3][2] = wz; mat[3][3] = ww;
}

__forceinline noMat4::noMat4( const noMat3 &rotation, const noVec3 &translation ) {
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

__forceinline noMat4::noMat4( const float src[ 4 ][ 4 ] ) {
	memcpy( mat, src, 4 * 4 * sizeof( float ) );
}

__forceinline noMat4::noMat4( const float src[ 16 ] ) {
	memcpy( mat, src, 4 * 4 * sizeof( float ) );
}

__forceinline const noVec4 &noMat4::operator[]( int index ) const {
	//assert( ( index >= 0 ) && ( index < 4 ) );
	return mat[ index ];
}

__forceinline noVec4 &noMat4::operator[]( int index ) {
	//assert( ( index >= 0 ) && ( index < 4 ) );
	return mat[ index ];
}

__forceinline noMat4 noMat4::operator*( const float a ) const {
	return noMat4(
		mat[0].x * a, mat[0].y * a, mat[0].z * a, mat[0].w * a,
		mat[1].x * a, mat[1].y * a, mat[1].z * a, mat[1].w * a,
		mat[2].x * a, mat[2].y * a, mat[2].z * a, mat[2].w * a,
		mat[3].x * a, mat[3].y * a, mat[3].z * a, mat[3].w * a );
}

__forceinline noVec4 noMat4::operator*( const noVec4 &vec ) const {
	return noVec4(
		mat[ 0 ].x * vec.x + mat[ 0 ].y * vec.y + mat[ 0 ].z * vec.z + mat[ 0 ].w * vec.w,
		mat[ 1 ].x * vec.x + mat[ 1 ].y * vec.y + mat[ 1 ].z * vec.z + mat[ 1 ].w * vec.w,
		mat[ 2 ].x * vec.x + mat[ 2 ].y * vec.y + mat[ 2 ].z * vec.z + mat[ 2 ].w * vec.w,
		mat[ 3 ].x * vec.x + mat[ 3 ].y * vec.y + mat[ 3 ].z * vec.z + mat[ 3 ].w * vec.w );
}

__forceinline noVec3 noMat4::operator*( const noVec3 &vec ) const {
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

__forceinline noMat4 noMat4::operator*( const noMat4 &a ) const {
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

__forceinline noMat4 noMat4::operator+( const noMat4 &a ) const {
	return noMat4( 
		mat[0].x + a[0].x, mat[0].y + a[0].y, mat[0].z + a[0].z, mat[0].w + a[0].w,
		mat[1].x + a[1].x, mat[1].y + a[1].y, mat[1].z + a[1].z, mat[1].w + a[1].w,
		mat[2].x + a[2].x, mat[2].y + a[2].y, mat[2].z + a[2].z, mat[2].w + a[2].w,
		mat[3].x + a[3].x, mat[3].y + a[3].y, mat[3].z + a[3].z, mat[3].w + a[3].w );
}
    
__forceinline noMat4 noMat4::operator-( const noMat4 &a ) const {
	return noMat4( 
		mat[0].x - a[0].x, mat[0].y - a[0].y, mat[0].z - a[0].z, mat[0].w - a[0].w,
		mat[1].x - a[1].x, mat[1].y - a[1].y, mat[1].z - a[1].z, mat[1].w - a[1].w,
		mat[2].x - a[2].x, mat[2].y - a[2].y, mat[2].z - a[2].z, mat[2].w - a[2].w,
		mat[3].x - a[3].x, mat[3].y - a[3].y, mat[3].z - a[3].z, mat[3].w - a[3].w );
}

__forceinline noMat4 &noMat4::operator*=( const float a ) {
	mat[0].x *= a; mat[0].y *= a; mat[0].z *= a; mat[0].w *= a;
	mat[1].x *= a; mat[1].y *= a; mat[1].z *= a; mat[1].w *= a;
	mat[2].x *= a; mat[2].y *= a; mat[2].z *= a; mat[2].w *= a;
	mat[3].x *= a; mat[3].y *= a; mat[3].z *= a; mat[3].w *= a;
    return *this;
}

__forceinline noMat4 &noMat4::operator*=( const noMat4 &a ) {
	*this = (*this) * a;
	return *this;
}

__forceinline noMat4 &noMat4::operator+=( const noMat4 &a ) {
	mat[0].x += a[0].x; mat[0].y += a[0].y; mat[0].z += a[0].z; mat[0].w += a[0].w;
	mat[1].x += a[1].x; mat[1].y += a[1].y; mat[1].z += a[1].z; mat[1].w += a[1].w;
	mat[2].x += a[2].x; mat[2].y += a[2].y; mat[2].z += a[2].z; mat[2].w += a[2].w;
	mat[3].x += a[3].x; mat[3].y += a[3].y; mat[3].z += a[3].z; mat[3].w += a[3].w;
    return *this;
}

__forceinline noMat4 &noMat4::operator-=( const noMat4 &a ) {
	mat[0].x -= a[0].x; mat[0].y -= a[0].y; mat[0].z -= a[0].z; mat[0].w -= a[0].w;
	mat[1].x -= a[1].x; mat[1].y -= a[1].y; mat[1].z -= a[1].z; mat[1].w -= a[1].w;
	mat[2].x -= a[2].x; mat[2].y -= a[2].y; mat[2].z -= a[2].z; mat[2].w -= a[2].w;
	mat[3].x -= a[3].x; mat[3].y -= a[3].y; mat[3].z -= a[3].z; mat[3].w -= a[3].w;
    return *this;
}

__forceinline noMat4 operator*( const float a, const noMat4 &mat ) {
	return mat * a;
}

__forceinline noVec4 operator*( const noVec4 &vec, const noMat4 &mat ) {
	return mat * vec;
}

__forceinline noVec3 operator*( const noVec3 &vec, const noMat4 &mat ) {
	return mat * vec;
}

__forceinline noVec4 &operator*=( noVec4 &vec, const noMat4 &mat ) {
	vec = mat * vec;
	return vec;
}

__forceinline noVec3 &operator*=( noVec3 &vec, const noMat4 &mat ) {
	vec = mat * vec;
	return vec;
}

__forceinline bool noMat4::Compare( const noMat4 &a ) const {
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

__forceinline bool noMat4::Compare( const noMat4 &a, const float epsilon ) const {
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

__forceinline bool noMat4::operator==( const noMat4 &a ) const {
	return Compare( a );
}

__forceinline bool noMat4::operator!=( const noMat4 &a ) const {
	return !Compare( a );
}

__forceinline void noMat4::Zero( void ) {
	memset( mat, 0, sizeof( noMat4 ) );
}

__forceinline void noMat4::Identity( void ) {
	*this = mat4_identity;
}

__forceinline bool noMat4::IsIdentity( const float epsilon ) const {
	return Compare( mat4_identity, epsilon );
}

__forceinline bool noMat4::IsSymmetric( const float epsilon ) const {
	for ( int i = 1; i < 4; i++ ) {
		for ( int j = 0; j < i; j++ ) {
			if ( noMath::Fabs( mat[i][j] - mat[j][i] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

__forceinline bool noMat4::IsDiagonal( const float epsilon ) const {
	for ( int i = 0; i < 4; i++ ) {
		for ( int j = 0; j < 4; j++ ) {
			if ( i != j && noMath::Fabs( mat[i][j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

__forceinline bool noMat4::IsRotated( void ) const {
	if ( !mat[ 0 ][ 1 ] && !mat[ 0 ][ 2 ] &&
		!mat[ 1 ][ 0 ] && !mat[ 1 ][ 2 ] &&
		!mat[ 2 ][ 0 ] && !mat[ 2 ][ 1 ] ) {
		return false;
	}
	return true;
}

__forceinline void noMat4::ProjectVector( const noVec4 &src, noVec4 &dst ) const {
	dst.x = src * mat[ 0 ];
	dst.y = src * mat[ 1 ];
	dst.z = src * mat[ 2 ];
	dst.w = src * mat[ 3 ];
}

__forceinline void noMat4::UnprojectVector( const noVec4 &src, noVec4 &dst ) const {
	dst = mat[ 0 ] * src.x + mat[ 1 ] * src.y + mat[ 2 ] * src.z + mat[ 3 ] * src.w;
}

__forceinline float noMat4::Trace( void ) const {
	return ( mat[0][0] + mat[1][1] + mat[2][2] + mat[3][3] );
}

__forceinline noMat4 noMat4::Inverse( void ) const {
	noMat4 invMat;

	invMat = *this;
	int r = invMat.InverseSelf();
	assert( r );
	return invMat;
}

__forceinline noMat4 noMat4::InverseFast( void ) const {
	noMat4 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf();
	assert( r );
	return invMat;
}

__forceinline noMat4 noMat3::ToMat4( void ) const {
	// NOTE: noMat3 is transposed because it is column-major
	return noMat4(	mat[0][0],	mat[1][0],	mat[2][0],	0.0f,
					mat[0][1],	mat[1][1],	mat[2][1],	0.0f,
					mat[0][2],	mat[1][2],	mat[2][2],	0.0f,
					0.0f,		0.0f,		0.0f,		1.0f );
}

__forceinline int noMat4::GetDimension( void ) const {
	return 16;
}

__forceinline const float *noMat4::ToFloatPtr( void ) const {
	return mat[0].ToFloatPtr();
}

__forceinline void noMat4::Translation( const noVec3& t )
{
	Identity();
	mat[0][3] = t.x;
	mat[1][3] = t.y;
	mat[2][3] = t.z;
}

__forceinline void noMat4::Scale( const noVec3& t )
{
	Zero();
	mat[0][0] = t.x;
	mat[1][1] = t.y;
	mat[2][2] = t.z;
	mat[3][3] = 1.0f;
}


__forceinline float *noMat4::ToFloatPtr( void ) {
	return mat[0].ToFloatPtr();
}

NO_INLINE void	noMat4::SetOrigin(const noVec3& v)
{
	mat[0][3] = v.x;
	mat[1][3] = v.y;
	mat[2][3] = v.z;
}


__forceinline  noVec3 noMat4::GetOrigin() const
{
	return noVec3(mat[0][3], mat[1][3], mat[2][3]);
}

// Remove any scaling from this matrix (ie magnitude of each row is 1)
__forceinline void noMat4::RemoveScaling(FLOAT Tolerance/*=SMALL_NUMBER*/)
{
	// For each row, find magnitude, and if its non-zero re-scale so its unit length.
	for(INT i=0; i<3; i++)
	{
		const FLOAT SquareSum = (mat[i][0] * mat[i][0]) + (mat[i][1] * mat[i][1]) + (mat[i][2] * mat[i][2]);
		if(SquareSum > Tolerance)
		{
			const FLOAT Scale = noMath::InvSqrt(SquareSum);
			mat[i][0] *= Scale; 
			mat[i][1] *= Scale; 
			mat[i][2] *= Scale; 
		}
	}
}



// Symmetric matrices can be optimized
template< class SCALAR, class V3, class T_MATRIX3 >
class T_SYMMETRIC_MATRIX3
{
public:

	SCALAR xx, yy, zz;	//diagonal elements
	SCALAR xy, xz, yz;	//off-diagonal elements

public:

	T_SYMMETRIC_MATRIX3()
	{
		//identity
		xx = yy = zz = 1;
		xy = xz = yz = 0;
	}

	T_SYMMETRIC_MATRIX3(
		const SCALAR xx,
		const SCALAR yy,
		const SCALAR zz,
		const SCALAR xy,
		const SCALAR xz,
		const SCALAR yz
		)
	{
		this->xx = xx;
		this->yy = yy;
		this->zz = zz;

		this->xy = xy;
		this->xz = xz;
		this->yz = yz;
	}

	//set equal to another matrix
	const T_SYMMETRIC_MATRIX3& operator = ( const T_SYMMETRIC_MATRIX3& m )
	{
		this->xx = m.xx;
		this->yy = m.yy;
		this->zz = m.zz;

		this->xy = m.xy;
		this->xz = m.xz;
		this->yz = m.yz;

		return *this;
	}

	//increment by another matrix
	const T_SYMMETRIC_MATRIX3& operator += ( const T_SYMMETRIC_MATRIX3& m )
	{
		this->xx += m.xx;
		this->yy += m.yy;
		this->zz += m.zz;

		this->xy += m.xy;
		this->xz += m.xz;
		this->yz += m.yz;

		return *this;
	}

	//decrement by another matrix
	const T_SYMMETRIC_MATRIX3& operator -=( const T_SYMMETRIC_MATRIX3& m ) 
	{
		this->xx -= m.xx;
		this->yy -= m.yy;
		this->zz -= m.zz;

		this->xy -= m.xy;
		this->xz -= m.xz;
		this->yz -= m.yz;

		return *this;
	}

	//add two matrices
	T_SYMMETRIC_MATRIX3 operator + ( const T_SYMMETRIC_MATRIX3& m ) const
	{
		return T_SYMMETRIC_MATRIX3(
			xx + m.xx,
			yy + m.yy,
			zz + m.zz,

			xy + m.xy,
			xz + m.xz,
			yz + m.yz
			);
	}

	//subtract two matrices
	T_SYMMETRIC_MATRIX3 operator - ( const T_SYMMETRIC_MATRIX3& m ) const
	{
		return T_SYMMETRIC_MATRIX3(
			xx - m.xx,
			yy - m.yy,
			zz - m.zz,

			xy - m.xy,
			xz - m.xz,
			yz - m.yz
			);
	}


	//post-multiply by a vector
	V3 operator * ( const V3& v ) const
	{
		return V3(	v.x*xx + v.y*xy + v.z*xz,
			v.x*xy + v.y*yy + v.z*yz,
			v.z*xz + v.y*yz + v.z*zz );
	}

	//pre-multiply by a vector
	friend inline V3 operator * ( const V3& v, const T_SYMMETRIC_MATRIX3& m )
	{
		return m * v;
	}

	//NOTE:  Can't do a self-multiply because the product of two symmetric matrices
	//is not necessarily a symmetric matrix

	//multiply two symmetric matrices
	//		T_SYMMETRIC_MATRIX3 operator * ( const T_SYMMETRIC_MATRIX3& m ) const
	//		{
	//			return T_MATRIX3();
	//		}

	// - matrix specific - //
	//a symmetric matrix is equal to its transpose

	//Is there a simplified formula for the inverse of a symmetric matrix?
};

typedef T_SYMMETRIC_MATRIX3< float, noVec3, noMat3 >		noSymmetricMat3;

//===============================================================
//
//	idMat5 - 5x5 matrix
//
//===============================================================

class idMat5 {
public:
	idMat5( void );
	explicit idMat5( const idVec5 &v0, const idVec5 &v1, const idVec5 &v2, const idVec5 &v3, const idVec5 &v4 );
	explicit idMat5( const float src[ 5 ][ 5 ] );

	const idVec5 &	operator[]( int index ) const;
	idVec5 &		operator[]( int index );
	idMat5			operator*( const float a ) const;
	idVec5			operator*( const idVec5 &vec ) const;
	idMat5			operator*( const idMat5 &a ) const;
	idMat5			operator+( const idMat5 &a ) const;
	idMat5			operator-( const idMat5 &a ) const;
	idMat5 &		operator*=( const float a );
	idMat5 &		operator*=( const idMat5 &a );
	idMat5 &		operator+=( const idMat5 &a );
	idMat5 &		operator-=( const idMat5 &a );

	friend idMat5	operator*( const float a, const idMat5 &mat );
	friend idVec5	operator*( const idVec5 &vec, const idMat5 &mat );
	friend idVec5 &	operator*=( idVec5 &vec, const idMat5 &mat );

	bool			Compare( const idMat5 &a ) const;						// exact compare, no epsilon
	bool			Compare( const idMat5 &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==( const idMat5 &a ) const;					// exact compare, no epsilon
	bool			operator!=( const idMat5 &a ) const;					// exact compare, no epsilon

	void			Zero( void );
	void			Identity( void );
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;

	float			Trace( void ) const;
	float			Determinant( void ) const;
	idMat5			Transpose( void ) const;	// returns transpose
	idMat5 &		TransposeSelf( void );
	idMat5			Inverse( void ) const;		// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseSelf( void );		// returns false if determinant is zero
	idMat5			InverseFast( void ) const;	// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseFastSelf( void );	// returns false if determinant is zero

	int				GetDimension( void ) const;

	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

private:
	idVec5			mat[ 5 ];
};

extern idMat5 mat5_zero;
extern idMat5 mat5_identity;
#define mat5_default	mat5_identity

ID_INLINE idMat5::idMat5( void ) {
}

ID_INLINE idMat5::idMat5( const float src[ 5 ][ 5 ] ) {
	memcpy( mat, src, 5 * 5 * sizeof( float ) );
}

ID_INLINE idMat5::idMat5( const idVec5 &v0, const idVec5 &v1, const idVec5 &v2, const idVec5 &v3, const idVec5 &v4 ) {
	mat[0] = v0;
	mat[1] = v1;
	mat[2] = v2;
	mat[3] = v3;
	mat[4] = v4;
}

ID_INLINE const idVec5 &idMat5::operator[]( int index ) const {
	//assert( ( index >= 0 ) && ( index < 5 ) );
	return mat[ index ];
}

ID_INLINE idVec5 &idMat5::operator[]( int index ) {
	//assert( ( index >= 0 ) && ( index < 5 ) );
	return mat[ index ];
}

ID_INLINE idMat5 idMat5::operator*( const idMat5 &a ) const {
	int i, j;
	const float *m1Ptr, *m2Ptr;
	float *dstPtr;
	idMat5 dst;

	m1Ptr = reinterpret_cast<const float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);
	dstPtr = reinterpret_cast<float *>(&dst);

	for ( i = 0; i < 5; i++ ) {
		for ( j = 0; j < 5; j++ ) {
			*dstPtr = m1Ptr[0] * m2Ptr[ 0 * 5 + j ]
			+ m1Ptr[1] * m2Ptr[ 1 * 5 + j ]
			+ m1Ptr[2] * m2Ptr[ 2 * 5 + j ]
			+ m1Ptr[3] * m2Ptr[ 3 * 5 + j ]
			+ m1Ptr[4] * m2Ptr[ 4 * 5 + j ];
			dstPtr++;
		}
		m1Ptr += 5;
	}
	return dst;
}

ID_INLINE idMat5 idMat5::operator*( const float a ) const {
	return idMat5(
		idVec5( mat[0][0] * a, mat[0][1] * a, mat[0][2] * a, mat[0][3] * a, mat[0][4] * a ),
		idVec5( mat[1][0] * a, mat[1][1] * a, mat[1][2] * a, mat[1][3] * a, mat[1][4] * a ),
		idVec5( mat[2][0] * a, mat[2][1] * a, mat[2][2] * a, mat[2][3] * a, mat[2][4] * a ),
		idVec5( mat[3][0] * a, mat[3][1] * a, mat[3][2] * a, mat[3][3] * a, mat[3][4] * a ),
		idVec5( mat[4][0] * a, mat[4][1] * a, mat[4][2] * a, mat[4][3] * a, mat[4][4] * a ) );
}

ID_INLINE idVec5 idMat5::operator*( const idVec5 &vec ) const {
	return idVec5(
		mat[0][0] * vec[0] + mat[0][1] * vec[1] + mat[0][2] * vec[2] + mat[0][3] * vec[3] + mat[0][4] * vec[4],
		mat[1][0] * vec[0] + mat[1][1] * vec[1] + mat[1][2] * vec[2] + mat[1][3] * vec[3] + mat[1][4] * vec[4],
		mat[2][0] * vec[0] + mat[2][1] * vec[1] + mat[2][2] * vec[2] + mat[2][3] * vec[3] + mat[2][4] * vec[4],
		mat[3][0] * vec[0] + mat[3][1] * vec[1] + mat[3][2] * vec[2] + mat[3][3] * vec[3] + mat[3][4] * vec[4],
		mat[4][0] * vec[0] + mat[4][1] * vec[1] + mat[4][2] * vec[2] + mat[4][3] * vec[3] + mat[4][4] * vec[4] );
}

ID_INLINE idMat5 idMat5::operator+( const idMat5 &a ) const {
	return idMat5(
		idVec5( mat[0][0] + a[0][0], mat[0][1] + a[0][1], mat[0][2] + a[0][2], mat[0][3] + a[0][3], mat[0][4] + a[0][4] ),
		idVec5( mat[1][0] + a[1][0], mat[1][1] + a[1][1], mat[1][2] + a[1][2], mat[1][3] + a[1][3], mat[1][4] + a[1][4] ),
		idVec5( mat[2][0] + a[2][0], mat[2][1] + a[2][1], mat[2][2] + a[2][2], mat[2][3] + a[2][3], mat[2][4] + a[2][4] ),
		idVec5( mat[3][0] + a[3][0], mat[3][1] + a[3][1], mat[3][2] + a[3][2], mat[3][3] + a[3][3], mat[3][4] + a[3][4] ),
		idVec5( mat[4][0] + a[4][0], mat[4][1] + a[4][1], mat[4][2] + a[4][2], mat[4][3] + a[4][3], mat[4][4] + a[4][4] ) );
}

ID_INLINE idMat5 idMat5::operator-( const idMat5 &a ) const {
	return idMat5(
		idVec5( mat[0][0] - a[0][0], mat[0][1] - a[0][1], mat[0][2] - a[0][2], mat[0][3] - a[0][3], mat[0][4] - a[0][4] ),
		idVec5( mat[1][0] - a[1][0], mat[1][1] - a[1][1], mat[1][2] - a[1][2], mat[1][3] - a[1][3], mat[1][4] - a[1][4] ),
		idVec5( mat[2][0] - a[2][0], mat[2][1] - a[2][1], mat[2][2] - a[2][2], mat[2][3] - a[2][3], mat[2][4] - a[2][4] ),
		idVec5( mat[3][0] - a[3][0], mat[3][1] - a[3][1], mat[3][2] - a[3][2], mat[3][3] - a[3][3], mat[3][4] - a[3][4] ),
		idVec5( mat[4][0] - a[4][0], mat[4][1] - a[4][1], mat[4][2] - a[4][2], mat[4][3] - a[4][3], mat[4][4] - a[4][4] ) );
}

ID_INLINE idMat5 &idMat5::operator*=( const float a ) {
	mat[0][0] *= a; mat[0][1] *= a; mat[0][2] *= a; mat[0][3] *= a; mat[0][4] *= a;
	mat[1][0] *= a; mat[1][1] *= a; mat[1][2] *= a; mat[1][3] *= a; mat[1][4] *= a;
	mat[2][0] *= a; mat[2][1] *= a; mat[2][2] *= a; mat[2][3] *= a; mat[2][4] *= a;
	mat[3][0] *= a; mat[3][1] *= a; mat[3][2] *= a; mat[3][3] *= a; mat[3][4] *= a;
	mat[4][0] *= a; mat[4][1] *= a; mat[4][2] *= a; mat[4][3] *= a; mat[4][4] *= a;
	return *this;
}

ID_INLINE idMat5 &idMat5::operator*=( const idMat5 &a ) {
	*this = *this * a;
	return *this;
}

ID_INLINE idMat5 &idMat5::operator+=( const idMat5 &a ) {
	mat[0][0] += a[0][0]; mat[0][1] += a[0][1]; mat[0][2] += a[0][2]; mat[0][3] += a[0][3]; mat[0][4] += a[0][4];
	mat[1][0] += a[1][0]; mat[1][1] += a[1][1]; mat[1][2] += a[1][2]; mat[1][3] += a[1][3]; mat[1][4] += a[1][4];
	mat[2][0] += a[2][0]; mat[2][1] += a[2][1]; mat[2][2] += a[2][2]; mat[2][3] += a[2][3]; mat[2][4] += a[2][4];
	mat[3][0] += a[3][0]; mat[3][1] += a[3][1]; mat[3][2] += a[3][2]; mat[3][3] += a[3][3]; mat[3][4] += a[3][4];
	mat[4][0] += a[4][0]; mat[4][1] += a[4][1]; mat[4][2] += a[4][2]; mat[4][3] += a[4][3]; mat[4][4] += a[4][4];
	return *this;
}

ID_INLINE idMat5 &idMat5::operator-=( const idMat5 &a ) {
	mat[0][0] -= a[0][0]; mat[0][1] -= a[0][1]; mat[0][2] -= a[0][2]; mat[0][3] -= a[0][3]; mat[0][4] -= a[0][4];
	mat[1][0] -= a[1][0]; mat[1][1] -= a[1][1]; mat[1][2] -= a[1][2]; mat[1][3] -= a[1][3]; mat[1][4] -= a[1][4];
	mat[2][0] -= a[2][0]; mat[2][1] -= a[2][1]; mat[2][2] -= a[2][2]; mat[2][3] -= a[2][3]; mat[2][4] -= a[2][4];
	mat[3][0] -= a[3][0]; mat[3][1] -= a[3][1]; mat[3][2] -= a[3][2]; mat[3][3] -= a[3][3]; mat[3][4] -= a[3][4];
	mat[4][0] -= a[4][0]; mat[4][1] -= a[4][1]; mat[4][2] -= a[4][2]; mat[4][3] -= a[4][3]; mat[4][4] -= a[4][4];
	return *this;
}

ID_INLINE idVec5 operator*( const idVec5 &vec, const idMat5 &mat ) {
	return mat * vec;
}

ID_INLINE idMat5 operator*( const float a, idMat5 const &mat ) {
	return mat * a;
}

ID_INLINE idVec5 &operator*=( idVec5 &vec, const idMat5 &mat ) {
	vec = mat * vec;
	return vec;
}

ID_INLINE bool idMat5::Compare( const idMat5 &a ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for ( i = 0; i < 5*5; i++ ) {
		if ( ptr1[i] != ptr2[i] ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMat5::Compare( const idMat5 &a, const float epsilon ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for ( i = 0; i < 5*5; i++ ) {
		if ( noMath::Fabs( ptr1[i] - ptr2[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMat5::operator==( const idMat5 &a ) const {
	return Compare( a );
}

ID_INLINE bool idMat5::operator!=( const idMat5 &a ) const {
	return !Compare( a );
}

ID_INLINE void idMat5::Zero( void ) {
	memset( mat, 0, sizeof( idMat5 ) );
}

ID_INLINE void idMat5::Identity( void ) {
	*this = mat5_identity;
}

ID_INLINE bool idMat5::IsIdentity( const float epsilon ) const {
	return Compare( mat5_identity, epsilon );
}

ID_INLINE bool idMat5::IsSymmetric( const float epsilon ) const {
	for ( int i = 1; i < 5; i++ ) {
		for ( int j = 0; j < i; j++ ) {
			if ( noMath::Fabs( mat[i][j] - mat[j][i] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMat5::IsDiagonal( const float epsilon ) const {
	for ( int i = 0; i < 5; i++ ) {
		for ( int j = 0; j < 5; j++ ) {
			if ( i != j && noMath::Fabs( mat[i][j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE float idMat5::Trace( void ) const {
	return ( mat[0][0] + mat[1][1] + mat[2][2] + mat[3][3] + mat[4][4] );
}

ID_INLINE idMat5 idMat5::Inverse( void ) const {
	idMat5 invMat;

	invMat = *this;
	int r = invMat.InverseSelf();
	assert( r );
	return invMat;
}

ID_INLINE idMat5 idMat5::InverseFast( void ) const {
	idMat5 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf();
	assert( r );
	return invMat;
}

ID_INLINE int idMat5::GetDimension( void ) const {
	return 25;
}

ID_INLINE const float *idMat5::ToFloatPtr( void ) const {
	return mat[0].ToFloatPtr();
}

ID_INLINE float *idMat5::ToFloatPtr( void ) {
	return mat[0].ToFloatPtr();
}


//===============================================================
//
//	idMat6 - 6x6 matrix
//
//===============================================================

class idMat6 {
public:
	idMat6( void );
	explicit idMat6( const idVec6 &v0, const idVec6 &v1, const idVec6 &v2, const idVec6 &v3, const idVec6 &v4, const idVec6 &v5 );
	explicit idMat6( const noMat3 &m0, const noMat3 &m1, const noMat3 &m2, const noMat3 &m3 );
	explicit idMat6( const float src[ 6 ][ 6 ] );

	const idVec6 &	operator[]( int index ) const;
	idVec6 &		operator[]( int index );
	idMat6			operator*( const float a ) const;
	idVec6			operator*( const idVec6 &vec ) const;
	idMat6			operator*( const idMat6 &a ) const;
	idMat6			operator+( const idMat6 &a ) const;
	idMat6			operator-( const idMat6 &a ) const;
	idMat6 &		operator*=( const float a );
	idMat6 &		operator*=( const idMat6 &a );
	idMat6 &		operator+=( const idMat6 &a );
	idMat6 &		operator-=( const idMat6 &a );

	friend idMat6	operator*( const float a, const idMat6 &mat );
	friend idVec6	operator*( const idVec6 &vec, const idMat6 &mat );
	friend idVec6 &	operator*=( idVec6 &vec, const idMat6 &mat );

	bool			Compare( const idMat6 &a ) const;						// exact compare, no epsilon
	bool			Compare( const idMat6 &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==( const idMat6 &a ) const;					// exact compare, no epsilon
	bool			operator!=( const idMat6 &a ) const;					// exact compare, no epsilon

	void			Zero( void );
	void			Identity( void );
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;

	noMat3			SubMat3( int n ) const;
	float			Trace( void ) const;
	float			Determinant( void ) const;
	idMat6			Transpose( void ) const;	// returns transpose
	idMat6 &		TransposeSelf( void );
	idMat6			Inverse( void ) const;		// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseSelf( void );		// returns false if determinant is zero
	idMat6			InverseFast( void ) const;	// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseFastSelf( void );	// returns false if determinant is zero

	int				GetDimension( void ) const;

	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

private:
	idVec6			mat[ 6 ];
};

extern idMat6 mat6_zero;
extern idMat6 mat6_identity;
#define mat6_default	mat6_identity

ID_INLINE idMat6::idMat6( void ) {
}

ID_INLINE idMat6::idMat6( const noMat3 &m0, const noMat3 &m1, const noMat3 &m2, const noMat3 &m3 ) {
	mat[0] = idVec6( m0[0][0], m0[0][1], m0[0][2], m1[0][0], m1[0][1], m1[0][2] );
	mat[1] = idVec6( m0[1][0], m0[1][1], m0[1][2], m1[1][0], m1[1][1], m1[1][2] );
	mat[2] = idVec6( m0[2][0], m0[2][1], m0[2][2], m1[2][0], m1[2][1], m1[2][2] );
	mat[3] = idVec6( m2[0][0], m2[0][1], m2[0][2], m3[0][0], m3[0][1], m3[0][2] );
	mat[4] = idVec6( m2[1][0], m2[1][1], m2[1][2], m3[1][0], m3[1][1], m3[1][2] );
	mat[5] = idVec6( m2[2][0], m2[2][1], m2[2][2], m3[2][0], m3[2][1], m3[2][2] );
}

ID_INLINE idMat6::idMat6( const idVec6 &v0, const idVec6 &v1, const idVec6 &v2, const idVec6 &v3, const idVec6 &v4, const idVec6 &v5 ) {
	mat[0] = v0;
	mat[1] = v1;
	mat[2] = v2;
	mat[3] = v3;
	mat[4] = v4;
	mat[5] = v5;
}

ID_INLINE idMat6::idMat6( const float src[ 6 ][ 6 ] ) {
	memcpy( mat, src, 6 * 6 * sizeof( float ) );
}

ID_INLINE const idVec6 &idMat6::operator[]( int index ) const {
	//assert( ( index >= 0 ) && ( index < 6 ) );
	return mat[ index ];
}

ID_INLINE idVec6 &idMat6::operator[]( int index ) {
	//assert( ( index >= 0 ) && ( index < 6 ) );
	return mat[ index ];
}

ID_INLINE idMat6 idMat6::operator*( const idMat6 &a ) const {
	int i, j;
	const float *m1Ptr, *m2Ptr;
	float *dstPtr;
	idMat6 dst;

	m1Ptr = reinterpret_cast<const float *>(this);
	m2Ptr = reinterpret_cast<const float *>(&a);
	dstPtr = reinterpret_cast<float *>(&dst);

	for ( i = 0; i < 6; i++ ) {
		for ( j = 0; j < 6; j++ ) {
			*dstPtr = m1Ptr[0] * m2Ptr[ 0 * 6 + j ]
			+ m1Ptr[1] * m2Ptr[ 1 * 6 + j ]
			+ m1Ptr[2] * m2Ptr[ 2 * 6 + j ]
			+ m1Ptr[3] * m2Ptr[ 3 * 6 + j ]
			+ m1Ptr[4] * m2Ptr[ 4 * 6 + j ]
			+ m1Ptr[5] * m2Ptr[ 5 * 6 + j ];
			dstPtr++;
		}
		m1Ptr += 6;
	}
	return dst;
}

ID_INLINE idMat6 idMat6::operator*( const float a ) const {
	return idMat6(
		idVec6( mat[0][0] * a, mat[0][1] * a, mat[0][2] * a, mat[0][3] * a, mat[0][4] * a, mat[0][5] * a ),
		idVec6( mat[1][0] * a, mat[1][1] * a, mat[1][2] * a, mat[1][3] * a, mat[1][4] * a, mat[1][5] * a ),
		idVec6( mat[2][0] * a, mat[2][1] * a, mat[2][2] * a, mat[2][3] * a, mat[2][4] * a, mat[2][5] * a ),
		idVec6( mat[3][0] * a, mat[3][1] * a, mat[3][2] * a, mat[3][3] * a, mat[3][4] * a, mat[3][5] * a ),
		idVec6( mat[4][0] * a, mat[4][1] * a, mat[4][2] * a, mat[4][3] * a, mat[4][4] * a, mat[4][5] * a ),
		idVec6( mat[5][0] * a, mat[5][1] * a, mat[5][2] * a, mat[5][3] * a, mat[5][4] * a, mat[5][5] * a ) );
}

ID_INLINE idVec6 idMat6::operator*( const idVec6 &vec ) const {
	return idVec6(
		mat[0][0] * vec[0] + mat[0][1] * vec[1] + mat[0][2] * vec[2] + mat[0][3] * vec[3] + mat[0][4] * vec[4] + mat[0][5] * vec[5],
		mat[1][0] * vec[0] + mat[1][1] * vec[1] + mat[1][2] * vec[2] + mat[1][3] * vec[3] + mat[1][4] * vec[4] + mat[1][5] * vec[5],
		mat[2][0] * vec[0] + mat[2][1] * vec[1] + mat[2][2] * vec[2] + mat[2][3] * vec[3] + mat[2][4] * vec[4] + mat[2][5] * vec[5],
		mat[3][0] * vec[0] + mat[3][1] * vec[1] + mat[3][2] * vec[2] + mat[3][3] * vec[3] + mat[3][4] * vec[4] + mat[3][5] * vec[5],
		mat[4][0] * vec[0] + mat[4][1] * vec[1] + mat[4][2] * vec[2] + mat[4][3] * vec[3] + mat[4][4] * vec[4] + mat[4][5] * vec[5],
		mat[5][0] * vec[0] + mat[5][1] * vec[1] + mat[5][2] * vec[2] + mat[5][3] * vec[3] + mat[5][4] * vec[4] + mat[5][5] * vec[5] );
}

ID_INLINE idMat6 idMat6::operator+( const idMat6 &a ) const {
	return idMat6(
		idVec6( mat[0][0] + a[0][0], mat[0][1] + a[0][1], mat[0][2] + a[0][2], mat[0][3] + a[0][3], mat[0][4] + a[0][4], mat[0][5] + a[0][5] ),
		idVec6( mat[1][0] + a[1][0], mat[1][1] + a[1][1], mat[1][2] + a[1][2], mat[1][3] + a[1][3], mat[1][4] + a[1][4], mat[1][5] + a[1][5] ),
		idVec6( mat[2][0] + a[2][0], mat[2][1] + a[2][1], mat[2][2] + a[2][2], mat[2][3] + a[2][3], mat[2][4] + a[2][4], mat[2][5] + a[2][5] ),
		idVec6( mat[3][0] + a[3][0], mat[3][1] + a[3][1], mat[3][2] + a[3][2], mat[3][3] + a[3][3], mat[3][4] + a[3][4], mat[3][5] + a[3][5] ),
		idVec6( mat[4][0] + a[4][0], mat[4][1] + a[4][1], mat[4][2] + a[4][2], mat[4][3] + a[4][3], mat[4][4] + a[4][4], mat[4][5] + a[4][5] ),
		idVec6( mat[5][0] + a[5][0], mat[5][1] + a[5][1], mat[5][2] + a[5][2], mat[5][3] + a[5][3], mat[5][4] + a[5][4], mat[5][5] + a[5][5] ) );
}

ID_INLINE idMat6 idMat6::operator-( const idMat6 &a ) const {
	return idMat6(
		idVec6( mat[0][0] - a[0][0], mat[0][1] - a[0][1], mat[0][2] - a[0][2], mat[0][3] - a[0][3], mat[0][4] - a[0][4], mat[0][5] - a[0][5] ),
		idVec6( mat[1][0] - a[1][0], mat[1][1] - a[1][1], mat[1][2] - a[1][2], mat[1][3] - a[1][3], mat[1][4] - a[1][4], mat[1][5] - a[1][5] ),
		idVec6( mat[2][0] - a[2][0], mat[2][1] - a[2][1], mat[2][2] - a[2][2], mat[2][3] - a[2][3], mat[2][4] - a[2][4], mat[2][5] - a[2][5] ),
		idVec6( mat[3][0] - a[3][0], mat[3][1] - a[3][1], mat[3][2] - a[3][2], mat[3][3] - a[3][3], mat[3][4] - a[3][4], mat[3][5] - a[3][5] ),
		idVec6( mat[4][0] - a[4][0], mat[4][1] - a[4][1], mat[4][2] - a[4][2], mat[4][3] - a[4][3], mat[4][4] - a[4][4], mat[4][5] - a[4][5] ),
		idVec6( mat[5][0] - a[5][0], mat[5][1] - a[5][1], mat[5][2] - a[5][2], mat[5][3] - a[5][3], mat[5][4] - a[5][4], mat[5][5] - a[5][5] ) );
}

ID_INLINE idMat6 &idMat6::operator*=( const float a ) {
	mat[0][0] *= a; mat[0][1] *= a; mat[0][2] *= a; mat[0][3] *= a; mat[0][4] *= a; mat[0][5] *= a;
	mat[1][0] *= a; mat[1][1] *= a; mat[1][2] *= a; mat[1][3] *= a; mat[1][4] *= a; mat[1][5] *= a;
	mat[2][0] *= a; mat[2][1] *= a; mat[2][2] *= a; mat[2][3] *= a; mat[2][4] *= a; mat[2][5] *= a;
	mat[3][0] *= a; mat[3][1] *= a; mat[3][2] *= a; mat[3][3] *= a; mat[3][4] *= a; mat[3][5] *= a;
	mat[4][0] *= a; mat[4][1] *= a; mat[4][2] *= a; mat[4][3] *= a; mat[4][4] *= a; mat[4][5] *= a;
	mat[5][0] *= a; mat[5][1] *= a; mat[5][2] *= a; mat[5][3] *= a; mat[5][4] *= a; mat[5][5] *= a;
	return *this;
}

ID_INLINE idMat6 &idMat6::operator*=( const idMat6 &a ) {
	*this = *this * a;
	return *this;
}

ID_INLINE idMat6 &idMat6::operator+=( const idMat6 &a ) {
	mat[0][0] += a[0][0]; mat[0][1] += a[0][1]; mat[0][2] += a[0][2]; mat[0][3] += a[0][3]; mat[0][4] += a[0][4]; mat[0][5] += a[0][5];
	mat[1][0] += a[1][0]; mat[1][1] += a[1][1]; mat[1][2] += a[1][2]; mat[1][3] += a[1][3]; mat[1][4] += a[1][4]; mat[1][5] += a[1][5];
	mat[2][0] += a[2][0]; mat[2][1] += a[2][1]; mat[2][2] += a[2][2]; mat[2][3] += a[2][3]; mat[2][4] += a[2][4]; mat[2][5] += a[2][5];
	mat[3][0] += a[3][0]; mat[3][1] += a[3][1]; mat[3][2] += a[3][2]; mat[3][3] += a[3][3]; mat[3][4] += a[3][4]; mat[3][5] += a[3][5];
	mat[4][0] += a[4][0]; mat[4][1] += a[4][1]; mat[4][2] += a[4][2]; mat[4][3] += a[4][3]; mat[4][4] += a[4][4]; mat[4][5] += a[4][5];
	mat[5][0] += a[5][0]; mat[5][1] += a[5][1]; mat[5][2] += a[5][2]; mat[5][3] += a[5][3]; mat[5][4] += a[5][4]; mat[5][5] += a[5][5];
	return *this;
}

ID_INLINE idMat6 &idMat6::operator-=( const idMat6 &a ) {
	mat[0][0] -= a[0][0]; mat[0][1] -= a[0][1]; mat[0][2] -= a[0][2]; mat[0][3] -= a[0][3]; mat[0][4] -= a[0][4]; mat[0][5] -= a[0][5];
	mat[1][0] -= a[1][0]; mat[1][1] -= a[1][1]; mat[1][2] -= a[1][2]; mat[1][3] -= a[1][3]; mat[1][4] -= a[1][4]; mat[1][5] -= a[1][5];
	mat[2][0] -= a[2][0]; mat[2][1] -= a[2][1]; mat[2][2] -= a[2][2]; mat[2][3] -= a[2][3]; mat[2][4] -= a[2][4]; mat[2][5] -= a[2][5];
	mat[3][0] -= a[3][0]; mat[3][1] -= a[3][1]; mat[3][2] -= a[3][2]; mat[3][3] -= a[3][3]; mat[3][4] -= a[3][4]; mat[3][5] -= a[3][5];
	mat[4][0] -= a[4][0]; mat[4][1] -= a[4][1]; mat[4][2] -= a[4][2]; mat[4][3] -= a[4][3]; mat[4][4] -= a[4][4]; mat[4][5] -= a[4][5];
	mat[5][0] -= a[5][0]; mat[5][1] -= a[5][1]; mat[5][2] -= a[5][2]; mat[5][3] -= a[5][3]; mat[5][4] -= a[5][4]; mat[5][5] -= a[5][5];
	return *this;
}

ID_INLINE idVec6 operator*( const idVec6 &vec, const idMat6 &mat ) {
	return mat * vec;
}

ID_INLINE idMat6 operator*( const float a, idMat6 const &mat ) {
	return mat * a;
}

ID_INLINE idVec6 &operator*=( idVec6 &vec, const idMat6 &mat ) {
	vec = mat * vec;
	return vec;
}

ID_INLINE bool idMat6::Compare( const idMat6 &a ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for ( i = 0; i < 6*6; i++ ) {
		if ( ptr1[i] != ptr2[i] ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMat6::Compare( const idMat6 &a, const float epsilon ) const {
	dword i;
	const float *ptr1, *ptr2;

	ptr1 = reinterpret_cast<const float *>(mat);
	ptr2 = reinterpret_cast<const float *>(a.mat);
	for ( i = 0; i < 6*6; i++ ) {
		if ( noMath::Fabs( ptr1[i] - ptr2[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMat6::operator==( const idMat6 &a ) const {
	return Compare( a );
}

ID_INLINE bool idMat6::operator!=( const idMat6 &a ) const {
	return !Compare( a );
}

ID_INLINE void idMat6::Zero( void ) {
	memset( mat, 0, sizeof( idMat6 ) );
}

ID_INLINE void idMat6::Identity( void ) {
	*this = mat6_identity;
}

ID_INLINE bool idMat6::IsIdentity( const float epsilon ) const {
	return Compare( mat6_identity, epsilon );
}

ID_INLINE bool idMat6::IsSymmetric( const float epsilon ) const {
	for ( int i = 1; i < 6; i++ ) {
		for ( int j = 0; j < i; j++ ) {
			if ( noMath::Fabs( mat[i][j] - mat[j][i] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMat6::IsDiagonal( const float epsilon ) const {
	for ( int i = 0; i < 6; i++ ) {
		for ( int j = 0; j < 6; j++ ) {
			if ( i != j && noMath::Fabs( mat[i][j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE noMat3 idMat6::SubMat3( int n ) const {
	assert( n >= 0 && n < 4 );
	int b0 = ((n & 2) >> 1) * 3;
	int b1 = (n & 1) * 3;
	return noMat3(
		mat[b0 + 0][b1 + 0], mat[b0 + 0][b1 + 1], mat[b0 + 0][b1 + 2],
		mat[b0 + 1][b1 + 0], mat[b0 + 1][b1 + 1], mat[b0 + 1][b1 + 2],
		mat[b0 + 2][b1 + 0], mat[b0 + 2][b1 + 1], mat[b0 + 2][b1 + 2] );
}

ID_INLINE float idMat6::Trace( void ) const {
	return ( mat[0][0] + mat[1][1] + mat[2][2] + mat[3][3] + mat[4][4] + mat[5][5] );
}

ID_INLINE idMat6 idMat6::Inverse( void ) const {
	idMat6 invMat;

	invMat = *this;
	int r = invMat.InverseSelf();
	assert( r );
	return invMat;
}

ID_INLINE idMat6 idMat6::InverseFast( void ) const {
	idMat6 invMat;

	invMat = *this;
	int r = invMat.InverseFastSelf();
	assert( r );
	return invMat;
}

ID_INLINE int idMat6::GetDimension( void ) const {
	return 36;
}

ID_INLINE const float *idMat6::ToFloatPtr( void ) const {
	return mat[0].ToFloatPtr();
}

ID_INLINE float *idMat6::ToFloatPtr( void ) {
	return mat[0].ToFloatPtr();
}


//===============================================================
//
//	idMatX - arbitrary sized dense real matrix
//
//  The matrix lives on 16 byte aligned and 16 byte padded memory.
//
//	NOTE: due to the temporary memory pool idMatX cannot be used by multiple threads.
//
//===============================================================

#define MATX_MAX_TEMP		1024
#define MATX_QUAD( x )		( ( ( ( x ) + 3 ) & ~3 ) * sizeof( float ) )
#define MATX_CLEAREND()		int s = numRows * numColumns; while( s < ( ( s + 3 ) & ~3 ) ) { mat[s++] = 0.0f; }
#define MATX_ALLOCA( n )	( (float *) _alloca16( MATX_QUAD( n ) ) )
#define MATX_SIMD

class idMatX {
public:
	idMatX( void );
	explicit idMatX( int rows, int columns );
	explicit idMatX( int rows, int columns, float *src );
	~idMatX( void );

	void			Set( int rows, int columns, const float *src );
	void			Set( const noMat3 &m1, const noMat3 &m2 );
	void			Set( const noMat3 &m1, const noMat3 &m2, const noMat3 &m3, const noMat3 &m4 );

	const float *	operator[]( int index ) const;
	float *			operator[]( int index );
	idMatX &		operator=( const idMatX &a );
	idMatX			operator*( const float a ) const;
	idVecX			operator*( const idVecX &vec ) const;
	idMatX			operator*( const idMatX &a ) const;
	idMatX			operator+( const idMatX &a ) const;
	idMatX			operator-( const idMatX &a ) const;
	idMatX &		operator*=( const float a );
	idMatX &		operator*=( const idMatX &a );
	idMatX &		operator+=( const idMatX &a );
	idMatX &		operator-=( const idMatX &a );

	friend idMatX	operator*( const float a, const idMatX &m );
	friend idVecX	operator*( const idVecX &vec, const idMatX &m );
	friend idVecX &	operator*=( idVecX &vec, const idMatX &m );

	bool			Compare( const idMatX &a ) const;								// exact compare, no epsilon
	bool			Compare( const idMatX &a, const float epsilon ) const;			// compare with epsilon
	bool			operator==( const idMatX &a ) const;							// exact compare, no epsilon
	bool			operator!=( const idMatX &a ) const;							// exact compare, no epsilon

	void			SetSize( int rows, int columns );								// set the number of rows/columns
	void			ChangeSize( int rows, int columns, bool makeZero = false );		// change the size keeping data intact where possible
	int				GetNumRows( void ) const { return numRows; }					// get the number of rows
	int				GetNumColumns( void ) const { return numColumns; }				// get the number of columns
	void			SetData( int rows, int columns, float *data );					// set float array pointer
	void			Zero( void );													// clear matrix
	void			Zero( int rows, int columns );									// set size and clear matrix
	void			Identity( void );												// clear to identity matrix
	void			Identity( int rows, int columns );								// set size and clear to identity matrix
	void			Diag( const idVecX &v );										// create diagonal matrix from vector
	void			Random( int seed, float l = 0.0f, float u = 1.0f );				// fill matrix with random values
	void			Random( int rows, int columns, int seed, float l = 0.0f, float u = 1.0f );
	void			Negate( void );													// (*this) = - (*this)
	void			Clamp( float min, float max );									// clamp all values
	idMatX &		SwapRows( int r1, int r2 );										// swap rows
	idMatX &		SwapColumns( int r1, int r2 );									// swap columns
	idMatX &		SwapRowsColumns( int r1, int r2 );								// swap rows and columns
	idMatX &		RemoveRow( int r );												// remove a row
	idMatX &		RemoveColumn( int r );											// remove a column
	idMatX &		RemoveRowColumn( int r );										// remove a row and column
	void			ClearUpperTriangle( void );										// clear the upper triangle
	void			ClearLowerTriangle( void );										// clear the lower triangle
	void			SquareSubMatrix( const idMatX &m, int size );					// get square sub-matrix from 0,0 to size,size
	float			MaxDifference( const idMatX &m ) const;							// return maximum element difference between this and m

	bool			IsSquare( void ) const { return ( numRows == numColumns ); }
	bool			IsZero( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsIdentity( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsTriDiagonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetric( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsOrthogonal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsOrthonormal( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsPMatrix( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsZMatrix( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsPositiveDefinite( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetricPositiveDefinite( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsPositiveSemiDefinite( const float epsilon = MATRIX_EPSILON ) const;
	bool			IsSymmetricPositiveSemiDefinite( const float epsilon = MATRIX_EPSILON ) const;

	float			Trace( void ) const;											// returns product of diagonal elements
	float			Determinant( void ) const;										// returns determinant of matrix
	idMatX			Transpose( void ) const;										// returns transpose
	idMatX &		TransposeSelf( void );											// transposes the matrix itself
	idMatX			Inverse( void ) const;											// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseSelf( void );											// returns false if determinant is zero
	idMatX			InverseFast( void ) const;										// returns the inverse ( m * m.Inverse() = identity )
	bool			InverseFastSelf( void );										// returns false if determinant is zero

	bool			LowerTriangularInverse( void );									// in-place inversion, returns false if determinant is zero
	bool			UpperTriangularInverse( void );									// in-place inversion, returns false if determinant is zero

	idVecX			Multiply( const idVecX &vec ) const;							// (*this) * vec
	idVecX			TransposeMultiply( const idVecX &vec ) const;					// this->Transpose() * vec

	idMatX			Multiply( const idMatX &a ) const;								// (*this) * a
	idMatX			TransposeMultiply( const idMatX &a ) const;						// this->Transpose() * a

	void			Multiply( idVecX &dst, const idVecX &vec ) const;				// dst = (*this) * vec
	void			MultiplyAdd( idVecX &dst, const idVecX &vec ) const;			// dst += (*this) * vec
	void			MultiplySub( idVecX &dst, const idVecX &vec ) const;			// dst -= (*this) * vec
	void			TransposeMultiply( idVecX &dst, const idVecX &vec ) const;		// dst = this->Transpose() * vec
	void			TransposeMultiplyAdd( idVecX &dst, const idVecX &vec ) const;	// dst += this->Transpose() * vec
	void			TransposeMultiplySub( idVecX &dst, const idVecX &vec ) const;	// dst -= this->Transpose() * vec

	void			Multiply( idMatX &dst, const idMatX &a ) const;					// dst = (*this) * a
	void			TransposeMultiply( idMatX &dst, const idMatX &a ) const;		// dst = this->Transpose() * a

	int				GetDimension( void ) const;										// returns total number of values in matrix

	const idVec6 &	SubVec6( int row ) const;										// interpret beginning of row as a const idVec6
	idVec6 &		SubVec6( int row );												// interpret beginning of row as an idVec6
	const idVecX	SubVecX( int row ) const;										// interpret complete row as a const idVecX
	idVecX			SubVecX( int row );												// interpret complete row as an idVecX
	const float *	ToFloatPtr( void ) const;										// pointer to const matrix float array
	float *			ToFloatPtr( void );												// pointer to matrix float array
	const char *	ToString( int precision = 2 ) const;

	void			Update_RankOne( const idVecX &v, const idVecX &w, float alpha );
	void			Update_RankOneSymmetric( const idVecX &v, float alpha );
	void			Update_RowColumn( const idVecX &v, const idVecX &w, int r );
	void			Update_RowColumnSymmetric( const idVecX &v, int r );
	void			Update_Increment( const idVecX &v, const idVecX &w );
	void			Update_IncrementSymmetric( const idVecX &v );
	void			Update_Decrement( int r );

	bool			Inverse_GaussJordan( void );					// invert in-place with Gauss-Jordan elimination
	bool			Inverse_UpdateRankOne( const idVecX &v, const idVecX &w, float alpha );
	bool			Inverse_UpdateRowColumn( const idVecX &v, const idVecX &w, int r );
	bool			Inverse_UpdateIncrement( const idVecX &v, const idVecX &w );
	bool			Inverse_UpdateDecrement( const idVecX &v, const idVecX &w, int r );
	void			Inverse_Solve( idVecX &x, const idVecX &b ) const;

	bool			LU_Factor( int *index, float *det = NULL );		// factor in-place: L * U
	bool			LU_UpdateRankOne( const idVecX &v, const idVecX &w, float alpha, int *index );
	bool			LU_UpdateRowColumn( const idVecX &v, const idVecX &w, int r, int *index );
	bool			LU_UpdateIncrement( const idVecX &v, const idVecX &w, int *index );
	bool			LU_UpdateDecrement( const idVecX &v, const idVecX &w, const idVecX &u, int r, int *index );
	void			LU_Solve( idVecX &x, const idVecX &b, const int *index ) const;
	void			LU_Inverse( idMatX &inv, const int *index ) const;
	void			LU_UnpackFactors( idMatX &L, idMatX &U ) const;
	void			LU_MultiplyFactors( idMatX &m, const int *index ) const;

	bool			QR_Factor( idVecX &c, idVecX &d );				// factor in-place: Q * R
	bool			QR_UpdateRankOne( idMatX &R, const idVecX &v, const idVecX &w, float alpha );
	bool			QR_UpdateRowColumn( idMatX &R, const idVecX &v, const idVecX &w, int r );
	bool			QR_UpdateIncrement( idMatX &R, const idVecX &v, const idVecX &w );
	bool			QR_UpdateDecrement( idMatX &R, const idVecX &v, const idVecX &w, int r );
	void			QR_Solve( idVecX &x, const idVecX &b, const idVecX &c, const idVecX &d ) const;
	void			QR_Solve( idVecX &x, const idVecX &b, const idMatX &R ) const;
	void			QR_Inverse( idMatX &inv, const idVecX &c, const idVecX &d ) const;
	void			QR_UnpackFactors( idMatX &Q, idMatX &R, const idVecX &c, const idVecX &d ) const;
	void			QR_MultiplyFactors( idMatX &m, const idVecX &c, const idVecX &d ) const;

	bool			SVD_Factor( idVecX &w, idMatX &V );				// factor in-place: U * Diag(w) * V.Transpose()
	void			SVD_Solve( idVecX &x, const idVecX &b, const idVecX &w, const idMatX &V ) const;
	void			SVD_Inverse( idMatX &inv, const idVecX &w, const idMatX &V ) const;
	void			SVD_MultiplyFactors( idMatX &m, const idVecX &w, const idMatX &V ) const;

	bool			Cholesky_Factor( void );						// factor in-place: L * L.Transpose()
	bool			Cholesky_UpdateRankOne( const idVecX &v, float alpha, int offset = 0 );
	bool			Cholesky_UpdateRowColumn( const idVecX &v, int r );
	bool			Cholesky_UpdateIncrement( const idVecX &v );
	bool			Cholesky_UpdateDecrement( const idVecX &v, int r );
	void			Cholesky_Solve( idVecX &x, const idVecX &b ) const;
	void			Cholesky_Inverse( idMatX &inv ) const;
	void			Cholesky_MultiplyFactors( idMatX &m ) const;

	bool			LDLT_Factor( void );							// factor in-place: L * D * L.Transpose()
	bool			LDLT_UpdateRankOne( const idVecX &v, float alpha, int offset = 0 );
	bool			LDLT_UpdateRowColumn( const idVecX &v, int r );
	bool			LDLT_UpdateIncrement( const idVecX &v );
	bool			LDLT_UpdateDecrement( const idVecX &v, int r );
	void			LDLT_Solve( idVecX &x, const idVecX &b ) const;
	void			LDLT_Inverse( idMatX &inv ) const;
	void			LDLT_UnpackFactors( idMatX &L, idMatX &D ) const;
	void			LDLT_MultiplyFactors( idMatX &m ) const;

	void			TriDiagonal_ClearTriangles( void );
	bool			TriDiagonal_Solve( idVecX &x, const idVecX &b ) const;
	void			TriDiagonal_Inverse( idMatX &inv ) const;

	bool			Eigen_SolveSymmetricTriDiagonal( idVecX &eigenValues );
	bool			Eigen_SolveSymmetric( idVecX &eigenValues );
	bool			Eigen_Solve( idVecX &realEigenValues, idVecX &imaginaryEigenValues );
	void			Eigen_SortIncreasing( idVecX &eigenValues );
	void			Eigen_SortDecreasing( idVecX &eigenValues );

	static void		Test( void );

private:
	int				numRows;				// number of rows
	int				numColumns;				// number of columns
	int				alloced;				// floats allocated, if -1 then mat points to data set with SetData
	float *			mat;					// memory the matrix is stored

	static float	temp[MATX_MAX_TEMP+4];	// used to store intermediate results
	static float *	tempPtr;				// pointer to 16 byte aligned temporary memory
	static int		tempIndex;				// index into memory pool, wraps around

private:
	void			SetTempSize( int rows, int columns );
	float			DeterminantGeneric( void ) const;
	bool			InverseSelfGeneric( void );
	void			QR_Rotate( idMatX &R, int i, float a, float b );
	float			Pythag( float a, float b ) const;
	void			SVD_BiDiag( idVecX &w, idVecX &rv1, float &anorm );
	void			SVD_InitialWV( idVecX &w, idMatX &V, idVecX &rv1 );
	void			HouseholderReduction( idVecX &diag, idVecX &subd );
	bool			QL( idVecX &diag, idVecX &subd );
	void			HessenbergReduction( idMatX &H );
	void			ComplexDivision( float xr, float xi, float yr, float yi, float &cdivr, float &cdivi );
	bool			HessenbergToRealSchur( idMatX &H, idVecX &realEigenValues, idVecX &imaginaryEigenValues );
};

ID_INLINE idMatX::idMatX( void ) {
	numRows = numColumns = alloced = 0;
	mat = NULL;
}

ID_INLINE idMatX::~idMatX( void ) {
	// if not temp memory
	if ( mat != NULL && ( mat < idMatX::tempPtr || mat > idMatX::tempPtr + MATX_MAX_TEMP ) && alloced != -1 ) {
		Mem_Free16( mat );
	}
}

ID_INLINE idMatX::idMatX( int rows, int columns ) {
	numRows = numColumns = alloced = 0;
	mat = NULL;
	SetSize( rows, columns );
}

ID_INLINE idMatX::idMatX( int rows, int columns, float *src ) {
	numRows = numColumns = alloced = 0;
	mat = NULL;
	SetData( rows, columns, src );
}

ID_INLINE void idMatX::Set( int rows, int columns, const float *src ) {
	SetSize( rows, columns );
	memcpy( this->mat, src, rows * columns * sizeof( float ) );
}

ID_INLINE void idMatX::Set( const noMat3 &m1, const noMat3 &m2 ) {
	int i, j;

	SetSize( 3, 6 );
	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			mat[(i+0) * numColumns + (j+0)] = m1[i][j];
			mat[(i+0) * numColumns + (j+3)] = m2[i][j];
		}
	}
}

ID_INLINE void idMatX::Set( const noMat3 &m1, const noMat3 &m2, const noMat3 &m3, const noMat3 &m4 ) {
	int i, j;

	SetSize( 6, 6 );
	for ( i = 0; i < 3; i++ ) {
		for ( j = 0; j < 3; j++ ) {
			mat[(i+0) * numColumns + (j+0)] = m1[i][j];
			mat[(i+0) * numColumns + (j+3)] = m2[i][j];
			mat[(i+3) * numColumns + (j+0)] = m3[i][j];
			mat[(i+3) * numColumns + (j+3)] = m4[i][j];
		}
	}
}

ID_INLINE const float *idMatX::operator[]( int index ) const {
	assert( ( index >= 0 ) && ( index < numRows ) );
	return mat + index * numColumns;
}

ID_INLINE float *idMatX::operator[]( int index ) {
	assert( ( index >= 0 ) && ( index < numRows ) );
	return mat + index * numColumns;
}

ID_INLINE idMatX &idMatX::operator=( const idMatX &a ) {
	SetSize( a.numRows, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Copy16( mat, a.mat, a.numRows * a.numColumns );
#else
	memcpy( mat, a.mat, a.numRows * a.numColumns * sizeof( float ) );
#endif
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX idMatX::operator*( const float a ) const {
	idMatX m;

	m.SetTempSize( numRows, numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Mul16( m.mat, mat, a, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		m.mat[i] = mat[i] * a;
	}
#endif
	return m;
}

ID_INLINE idVecX idMatX::operator*( const idVecX &vec ) const {
	idVecX dst;

	assert( numColumns == vec.GetSize() );

	dst.SetTempSize( numRows );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyVecX( dst, *this, vec );
#else
	Multiply( dst, vec );
#endif
	return dst;
}

ID_INLINE idMatX idMatX::operator*( const idMatX &a ) const {
	idMatX dst;

	assert( numColumns == a.numRows );

	dst.SetTempSize( numRows, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyMatX( dst, *this, a );
#else
	Multiply( dst, a );
#endif
	return dst;
}

ID_INLINE idMatX idMatX::operator+( const idMatX &a ) const {
	idMatX m;

	assert( numRows == a.numRows && numColumns == a.numColumns );
	m.SetTempSize( numRows, numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Add16( m.mat, mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		m.mat[i] = mat[i] + a.mat[i];
	}
#endif
	return m;
}

ID_INLINE idMatX idMatX::operator-( const idMatX &a ) const {
	idMatX m;

	assert( numRows == a.numRows && numColumns == a.numColumns );
	m.SetTempSize( numRows, numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Sub16( m.mat, mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		m.mat[i] = mat[i] - a.mat[i];
	}
#endif
	return m;
}

ID_INLINE idMatX &idMatX::operator*=( const float a ) {
#ifdef MATX_SIMD
	SIMDProcessor->MulAssign16( mat, a, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		mat[i] *= a;
	}
#endif
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX &idMatX::operator*=( const idMatX &a ) {
	*this = *this * a;
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX &idMatX::operator+=( const idMatX &a ) {
	assert( numRows == a.numRows && numColumns == a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->AddAssign16( mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		mat[i] += a.mat[i];
	}
#endif
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX &idMatX::operator-=( const idMatX &a ) {
	assert( numRows == a.numRows && numColumns == a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->SubAssign16( mat, a.mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		mat[i] -= a.mat[i];
	}
#endif
	idMatX::tempIndex = 0;
	return *this;
}

ID_INLINE idMatX operator*( const float a, idMatX const &m ) {
	return m * a;
}

ID_INLINE idVecX operator*( const idVecX &vec, const idMatX &m ) {
	return m * vec;
}

ID_INLINE idVecX &operator*=( idVecX &vec, const idMatX &m ) {
	vec = m * vec;
	return vec;
}

ID_INLINE bool idMatX::Compare( const idMatX &a ) const {
	int i, s;

	assert( numRows == a.numRows && numColumns == a.numColumns );

	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		if ( mat[i] != a.mat[i] ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMatX::Compare( const idMatX &a, const float epsilon ) const {
	int i, s;

	assert( numRows == a.numRows && numColumns == a.numColumns );

	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		if ( noMath::Fabs( mat[i] - a.mat[i] ) > epsilon ) {
			return false;
		}
	}
	return true;
}

ID_INLINE bool idMatX::operator==( const idMatX &a ) const {
	return Compare( a );
}

ID_INLINE bool idMatX::operator!=( const idMatX &a ) const {
	return !Compare( a );
}

ID_INLINE void idMatX::SetSize( int rows, int columns ) {
	assert( mat < idMatX::tempPtr || mat > idMatX::tempPtr + MATX_MAX_TEMP );
	int alloc = ( rows * columns + 3 ) & ~3;
	if ( alloc > alloced && alloced != -1 ) {
		if ( mat != NULL ) {
			Mem_Free16( mat );
		}
		mat = (float *) Mem_Alloc16( alloc * sizeof( float ) );
		alloced = alloc;
	}
	numRows = rows;
	numColumns = columns;
	MATX_CLEAREND();
}

ID_INLINE void idMatX::SetTempSize( int rows, int columns ) {
	int newSize;

	newSize = ( rows * columns + 3 ) & ~3;
	assert( newSize < MATX_MAX_TEMP );
	if ( idMatX::tempIndex + newSize > MATX_MAX_TEMP ) {
		idMatX::tempIndex = 0;
	}
	mat = idMatX::tempPtr + idMatX::tempIndex;
	idMatX::tempIndex += newSize;
	alloced = newSize;
	numRows = rows;
	numColumns = columns;
	MATX_CLEAREND();
}

ID_INLINE void idMatX::SetData( int rows, int columns, float *data ) {
	assert( mat < idMatX::tempPtr || mat > idMatX::tempPtr + MATX_MAX_TEMP );
	if ( mat != NULL && alloced != -1 ) {
		Mem_Free16( mat );
	}
	assert( ( ( (int) data ) & 15 ) == 0 ); // data must be 16 byte aligned
	mat = data;
	alloced = -1;
	numRows = rows;
	numColumns = columns;
	MATX_CLEAREND();
}

ID_INLINE void idMatX::Zero( void ) {
#ifdef MATX_SIMD
	SIMDProcessor->Zero16( mat, numRows * numColumns );
#else
	memset( mat, 0, numRows * numColumns * sizeof( float ) );
#endif
}

ID_INLINE void idMatX::Zero( int rows, int columns ) {
	SetSize( rows, columns );
#ifdef MATX_SIMD
	SIMDProcessor->Zero16( mat, numRows * numColumns );
#else
	memset( mat, 0, rows * columns * sizeof( float ) );
#endif
}

ID_INLINE void idMatX::Identity( void ) {
	assert( numRows == numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->Zero16( mat, numRows * numColumns );
#else
	memset( mat, 0, numRows * numColumns * sizeof( float ) );
#endif
	for ( int i = 0; i < numRows; i++ ) {
		mat[i * numColumns + i] = 1.0f;
	}
}

ID_INLINE void idMatX::Identity( int rows, int columns ) {
	assert( rows == columns );
	SetSize( rows, columns );
	idMatX::Identity();
}

ID_INLINE void idMatX::Diag( const idVecX &v ) {
	Zero( v.GetSize(), v.GetSize() );
	for ( int i = 0; i < v.GetSize(); i++ ) {
		mat[i * numColumns + i] = v[i];
	}
}

ID_INLINE void idMatX::Random( int seed, float l, float u ) {
	int i, s;
	float c;
	idRandom rnd(seed);

	c = u - l;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		mat[i] = l + rnd.RandomFloat() * c;
	}
}

ID_INLINE void idMatX::Random( int rows, int columns, int seed, float l, float u ) {
	int i, s;
	float c;
	idRandom rnd(seed);

	SetSize( rows, columns );
	c = u - l;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		mat[i] = l + rnd.RandomFloat() * c;
	}
}

ID_INLINE void idMatX::Negate( void ) {
#ifdef MATX_SIMD
	SIMDProcessor->Negate16( mat, numRows * numColumns );
#else
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		mat[i] = -mat[i];
	}
#endif
}

ID_INLINE void idMatX::Clamp( float min, float max ) {
	int i, s;
	s = numRows * numColumns;
	for ( i = 0; i < s; i++ ) {
		if ( mat[i] < min ) {
			mat[i] = min;
		} else if ( mat[i] > max ) {
			mat[i] = max;
		}
	}
}

ID_INLINE idMatX &idMatX::SwapRows( int r1, int r2 ) {
	float *ptr;

	ptr = (float *) _alloca16( numColumns * sizeof( float ) );
	memcpy( ptr, mat + r1 * numColumns, numColumns * sizeof( float ) );
	memcpy( mat + r1 * numColumns, mat + r2 * numColumns, numColumns * sizeof( float ) );
	memcpy( mat + r2 * numColumns, ptr, numColumns * sizeof( float ) );

	return *this;
}

ID_INLINE idMatX &idMatX::SwapColumns( int r1, int r2 ) {
	int i;
	float tmp, *ptr;

	for ( i = 0; i < numRows; i++ ) {
		ptr = mat + i * numColumns;
		tmp = ptr[r1];
		ptr[r1] = ptr[r2];
		ptr[r2] = tmp;
	}

	return *this;
}

ID_INLINE idMatX &idMatX::SwapRowsColumns( int r1, int r2 ) {

	SwapRows( r1, r2 );
	SwapColumns( r1, r2 );
	return *this;
}

ID_INLINE void idMatX::ClearUpperTriangle( void ) {
	assert( numRows == numColumns );
	for ( int i = numRows-2; i >= 0; i-- ) {
		memset( mat + i * numColumns + i + 1, 0, (numColumns - 1 - i) * sizeof(float) );
	}
}

ID_INLINE void idMatX::ClearLowerTriangle( void ) {
	assert( numRows == numColumns );
	for ( int i = 1; i < numRows; i++ ) {
		memset( mat + i * numColumns, 0, i * sizeof(float) );
	}
}

ID_INLINE void idMatX::SquareSubMatrix( const idMatX &m, int size ) {
	int i;
	assert( size <= m.numRows && size <= m.numColumns );
	SetSize( size, size );
	for ( i = 0; i < size; i++ ) {
		memcpy( mat + i * numColumns, m.mat + i * m.numColumns, size * sizeof( float ) );
	}
}

ID_INLINE float idMatX::MaxDifference( const idMatX &m ) const {
	int i, j;
	float diff, maxDiff;

	assert( numRows == m.numRows && numColumns == m.numColumns );

	maxDiff = -1.0f;
	for ( i = 0; i < numRows; i++ ) {
		for ( j = 0; j < numColumns; j++ ) {
			diff = noMath::Fabs( mat[ i * numColumns + j ] - m[i][j] );
			if ( maxDiff < 0.0f || diff > maxDiff ) {
				maxDiff = diff;
			}
		}
	}
	return maxDiff;
}

ID_INLINE bool idMatX::IsZero( const float epsilon ) const {
	// returns true if (*this) == Zero
	for ( int i = 0; i < numRows; i++ ) {
		for ( int j = 0; j < numColumns; j++ ) {
			if ( noMath::Fabs( mat[i * numColumns + j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMatX::IsIdentity( const float epsilon ) const {
	// returns true if (*this) == Identity
	assert( numRows == numColumns );
	for ( int i = 0; i < numRows; i++ ) {
		for ( int j = 0; j < numColumns; j++ ) {
			if ( noMath::Fabs( mat[i * numColumns + j] - (float)( i == j ) ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMatX::IsDiagonal( const float epsilon ) const {
	// returns true if all elements are zero except for the elements on the diagonal
	assert( numRows == numColumns );
	for ( int i = 0; i < numRows; i++ ) {
		for ( int j = 0; j < numColumns; j++ ) {
			if ( i != j && noMath::Fabs( mat[i * numColumns + j] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMatX::IsTriDiagonal( const float epsilon ) const {
	// returns true if all elements are zero except for the elements on the diagonal plus or minus one column

	if ( numRows != numColumns ) {
		return false;
	}
	for ( int i = 0; i < numRows-2; i++ ) {
		for ( int j = i+2; j < numColumns; j++ ) {
			if ( noMath::Fabs( (*this)[i][j] ) > epsilon ) {
				return false;
			}
			if ( noMath::Fabs( (*this)[j][i] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE bool idMatX::IsSymmetric( const float epsilon ) const {
	// (*this)[i][j] == (*this)[j][i]
	if ( numRows != numColumns ) {
		return false;
	}
	for ( int i = 0; i < numRows; i++ ) {
		for ( int j = 0; j < numColumns; j++ ) {
			if ( noMath::Fabs( mat[ i * numColumns + j ] - mat[ j * numColumns + i ] ) > epsilon ) {
				return false;
			}
		}
	}
	return true;
}

ID_INLINE float idMatX::Trace( void ) const {
	float trace = 0.0f;

	assert( numRows == numColumns );

	// sum of elements on the diagonal
	for ( int i = 0; i < numRows; i++ ) {
		trace += mat[i * numRows + i];
	}
	return trace;
}

ID_INLINE float idMatX::Determinant( void ) const {

	assert( numRows == numColumns );

	switch( numRows ) {
	case 1:
		return mat[0];
	case 2:
		return reinterpret_cast<const noMat2 *>(mat)->Determinant();
	case 3:
		return reinterpret_cast<const noMat3 *>(mat)->Determinant();
	case 4:
		return reinterpret_cast<const noMat4 *>(mat)->Determinant();
	case 5:
		return reinterpret_cast<const idMat5 *>(mat)->Determinant();
	case 6:
		return reinterpret_cast<const idMat6 *>(mat)->Determinant();
	default:
		return DeterminantGeneric();
	}
	return 0.0f;
}

ID_INLINE idMatX idMatX::Transpose( void ) const {
	idMatX transpose;
	int i, j;

	transpose.SetTempSize( numColumns, numRows );

	for ( i = 0; i < numRows; i++ ) {
		for ( j = 0; j < numColumns; j++ ) {
			transpose.mat[j * transpose.numColumns + i] = mat[i * numColumns + j];
		}
	}

	return transpose;
}

ID_INLINE idMatX &idMatX::TransposeSelf( void ) {
	*this = Transpose();
	return *this;
}

ID_INLINE idMatX idMatX::Inverse( void ) const {
	idMatX invMat;

	invMat.SetTempSize( numRows, numColumns );
	memcpy( invMat.mat, mat, numRows * numColumns * sizeof( float ) );
	int r = invMat.InverseSelf();
	assert( r );
	return invMat;
}

ID_INLINE bool idMatX::InverseSelf( void ) {

	assert( numRows == numColumns );

	switch( numRows ) {
	case 1:
		if ( noMath::Fabs( mat[0] ) < MATRIX_INVERSE_EPSILON ) {
			return false;
		}
		mat[0] = 1.0f / mat[0];
		return true;
	case 2:
		return reinterpret_cast<noMat2 *>(mat)->InverseSelf();
	case 3:
		return reinterpret_cast<noMat3 *>(mat)->InverseSelf();
	case 4:
		return reinterpret_cast<noMat4 *>(mat)->InverseSelf();
	case 5:
		return reinterpret_cast<idMat5 *>(mat)->InverseSelf();
	case 6:
		return reinterpret_cast<idMat6 *>(mat)->InverseSelf();
	default:
		return InverseSelfGeneric();
	}
}

ID_INLINE idMatX idMatX::InverseFast( void ) const {
	idMatX invMat;

	invMat.SetTempSize( numRows, numColumns );
	memcpy( invMat.mat, mat, numRows * numColumns * sizeof( float ) );
	int r = invMat.InverseFastSelf();
	assert( r );
	return invMat;
}

ID_INLINE bool idMatX::InverseFastSelf( void ) {

	assert( numRows == numColumns );

	switch( numRows ) {
	case 1:
		if ( noMath::Fabs( mat[0] ) < MATRIX_INVERSE_EPSILON ) {
			return false;
		}
		mat[0] = 1.0f / mat[0];
		return true;
	case 2:
		return reinterpret_cast<noMat2 *>(mat)->InverseFastSelf();
	case 3:
		return reinterpret_cast<noMat3 *>(mat)->InverseFastSelf();
	case 4:
		return reinterpret_cast<noMat4 *>(mat)->InverseFastSelf();
	case 5:
		return reinterpret_cast<idMat5 *>(mat)->InverseFastSelf();
	case 6:
		return reinterpret_cast<idMat6 *>(mat)->InverseFastSelf();
	default:
		return InverseSelfGeneric();
	}
	return false;
}

ID_INLINE idVecX idMatX::Multiply( const idVecX &vec ) const {
	idVecX dst;

	assert( numColumns == vec.GetSize() );

	dst.SetTempSize( numRows );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyVecX( dst, *this, vec );
#else
	Multiply( dst, vec );
#endif
	return dst;
}

ID_INLINE idMatX idMatX::Multiply( const idMatX &a ) const {
	idMatX dst;

	assert( numColumns == a.numRows );

	dst.SetTempSize( numRows, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyMatX( dst, *this, a );
#else
	Multiply( dst, a );
#endif
	return dst;
}

ID_INLINE idVecX idMatX::TransposeMultiply( const idVecX &vec ) const {
	idVecX dst;

	assert( numRows == vec.GetSize() );

	dst.SetTempSize( numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyVecX( dst, *this, vec );
#else
	TransposeMultiply( dst, vec );
#endif
	return dst;
}

ID_INLINE idMatX idMatX::TransposeMultiply( const idMatX &a ) const {
	idMatX dst;

	assert( numRows == a.numRows );

	dst.SetTempSize( numColumns, a.numColumns );
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyMatX( dst, *this, a );
#else
	TransposeMultiply( dst, a );
#endif
	return dst;
}

ID_INLINE void idMatX::Multiply( idVecX &dst, const idVecX &vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	mPtr = mat;
	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for ( i = 0; i < numRows; i++ ) {
		float sum = mPtr[0] * vPtr[0];
		for ( j = 1; j < numColumns; j++ ) {
			sum += mPtr[j] * vPtr[j];
		}
		dstPtr[i] = sum;
		mPtr += numColumns;
	}
#endif
}

ID_INLINE void idMatX::MultiplyAdd( idVecX &dst, const idVecX &vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyAddVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	mPtr = mat;
	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for ( i = 0; i < numRows; i++ ) {
		float sum = mPtr[0] * vPtr[0];
		for ( j = 1; j < numColumns; j++ ) {
			sum += mPtr[j] * vPtr[j];
		}
		dstPtr[i] += sum;
		mPtr += numColumns;
	}
#endif
}

ID_INLINE void idMatX::MultiplySub( idVecX &dst, const idVecX &vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplySubVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	mPtr = mat;
	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for ( i = 0; i < numRows; i++ ) {
		float sum = mPtr[0] * vPtr[0];
		for ( j = 1; j < numColumns; j++ ) {
			sum += mPtr[j] * vPtr[j];
		}
		dstPtr[i] -= sum;
		mPtr += numColumns;
	}
#endif
}

ID_INLINE void idMatX::TransposeMultiply( idVecX &dst, const idVecX &vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for ( i = 0; i < numColumns; i++ ) {
		mPtr = mat + i;
		float sum = mPtr[0] * vPtr[0];
		for ( j = 1; j < numRows; j++ ) {
			mPtr += numColumns;
			sum += mPtr[0] * vPtr[j];
		}
		dstPtr[i] = sum;
	}
#endif
}

ID_INLINE void idMatX::TransposeMultiplyAdd( idVecX &dst, const idVecX &vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyAddVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for ( i = 0; i < numColumns; i++ ) {
		mPtr = mat + i;
		float sum = mPtr[0] * vPtr[0];
		for ( j = 1; j < numRows; j++ ) {
			mPtr += numColumns;
			sum += mPtr[0] * vPtr[j];
		}
		dstPtr[i] += sum;
	}
#endif
}

ID_INLINE void idMatX::TransposeMultiplySub( idVecX &dst, const idVecX &vec ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplySubVecX( dst, *this, vec );
#else
	int i, j;
	const float *mPtr, *vPtr;
	float *dstPtr;

	vPtr = vec.ToFloatPtr();
	dstPtr = dst.ToFloatPtr();
	for ( i = 0; i < numColumns; i++ ) {
		mPtr = mat + i;
		float sum = mPtr[0] * vPtr[0];
		for ( j = 1; j < numRows; j++ ) {
			mPtr += numColumns;
			sum += mPtr[0] * vPtr[j];
		}
		dstPtr[i] -= sum;
	}
#endif
}

ID_INLINE void idMatX::Multiply( idMatX &dst, const idMatX &a ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_MultiplyMatX( dst, *this, a );
#else
	int i, j, k, l, n;
	float *dstPtr;
	const float *m1Ptr, *m2Ptr;
	double sum;

	assert( numColumns == a.numRows );

	dstPtr = dst.ToFloatPtr();
	m1Ptr = ToFloatPtr();
	m2Ptr = a.ToFloatPtr();
	k = numRows;
	l = a.GetNumColumns();

	for ( i = 0; i < k; i++ ) {
		for ( j = 0; j < l; j++ ) {
			m2Ptr = a.ToFloatPtr() + j;
			sum = m1Ptr[0] * m2Ptr[0];
			for ( n = 1; n < numColumns; n++ ) {
				m2Ptr += l;
				sum += m1Ptr[n] * m2Ptr[0];
			}
			*dstPtr++ = sum;
		}
		m1Ptr += numColumns;
	}
#endif
}

ID_INLINE void idMatX::TransposeMultiply( idMatX &dst, const idMatX &a ) const {
#ifdef MATX_SIMD
	SIMDProcessor->MatX_TransposeMultiplyMatX( dst, *this, a );
#else
	int i, j, k, l, n;
	float *dstPtr;
	const float *m1Ptr, *m2Ptr;
	double sum;

	assert( numRows == a.numRows );

	dstPtr = dst.ToFloatPtr();
	m1Ptr = ToFloatPtr();
	k = numColumns;
	l = a.numColumns;

	for ( i = 0; i < k; i++ ) {
		for ( j = 0; j < l; j++ ) {
			m1Ptr = ToFloatPtr() + i;
			m2Ptr = a.ToFloatPtr() + j;
			sum = m1Ptr[0] * m2Ptr[0];
			for ( n = 1; n < numRows; n++ ) {
				m1Ptr += numColumns;
				m2Ptr += a.numColumns;
				sum += m1Ptr[0] * m2Ptr[0];
			}
			*dstPtr++ = sum;
		}
	}
#endif
}

ID_INLINE int idMatX::GetDimension( void ) const {
	return numRows * numColumns;
}

ID_INLINE const idVec6 &idMatX::SubVec6( int row ) const {
	assert( numColumns >= 6 && row >= 0 && row < numRows );
	return *reinterpret_cast<const idVec6 *>(mat + row * numColumns);
}

ID_INLINE idVec6 &idMatX::SubVec6( int row ) {
	assert( numColumns >= 6 && row >= 0 && row < numRows );
	return *reinterpret_cast<idVec6 *>(mat + row * numColumns);
}

ID_INLINE const idVecX idMatX::SubVecX( int row ) const {
	idVecX v;
	assert( row >= 0 && row < numRows );
	v.SetData( numColumns, mat + row * numColumns );
	return v;
}

ID_INLINE idVecX idMatX::SubVecX( int row ) {
	idVecX v;
	assert( row >= 0 && row < numRows );
	v.SetData( numColumns, mat + row * numColumns );
	return v;
}

ID_INLINE const float *idMatX::ToFloatPtr( void ) const {
	return mat;
}

ID_INLINE float *idMatX::ToFloatPtr( void ) {
	return mat;
}


#endif