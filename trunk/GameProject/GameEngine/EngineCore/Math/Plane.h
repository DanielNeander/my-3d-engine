#ifndef __MATH_PLANE__H__
#define __MATH_PLANE__H__


/*
===============================================================================

	3D plane with equation: a * x + b * y + c * z + d = 0

===============================================================================
*/


class noVec3;
class noVec4;
class noMat3;

#define	ON_EPSILON					0.1f
#define DEGENERATE_DIST_EPSILON		1e-4f

#define	SIDE_FRONT					0
#define	SIDE_BACK					1
#define	SIDE_ON						2
#define	SIDE_CROSS					3

// plane sides
#define PLANESIDE_FRONT				0
#define PLANESIDE_BACK				1
#define PLANESIDE_ON				2
#define PLANESIDE_CROSS				3

// plane types
#define PLANETYPE_X					0
#define PLANETYPE_Y					1
#define PLANETYPE_Z					2
#define PLANETYPE_NEGX				3
#define PLANETYPE_NEGY				4
#define PLANETYPE_NEGZ				5
#define PLANETYPE_TRUEAXIAL			6	// all types < 6 are true axial planes
#define PLANETYPE_ZEROX				6
#define PLANETYPE_ZEROY				7
#define PLANETYPE_ZEROZ				8
#define PLANETYPE_NONAXIAL			9

class noPlane {
public:
					noPlane( void );
					noPlane( float a, float b, float c, float d );
					noPlane( const noVec3 &normal, const float dist );

	float			operator[]( int index ) const;
	float &			operator[]( int index );
	noPlane			operator-() const;						// flips plane
	noPlane &		operator=( const noVec3 &v );			// sets normal and sets noPlane::d to zero
	noPlane			operator+( const noPlane &p ) const;	// add plane equations
	noPlane			operator-( const noPlane &p ) const;	// subtract plane equations
	noPlane &		operator*=( const noMat3 &m );			// Normal() *= m

	bool			Compare( const noPlane &p ) const;						// exact compare, no epsilon
	bool			Compare( const noPlane &p, const float epsilon ) const;	// compare with epsilon
	bool			Compare( const noPlane &p, const float normalEps, const float distEps ) const;	// compare with epsilon
	bool			operator==(	const noPlane &p ) const;					// exact compare, no epsilon
	bool			operator!=(	const noPlane &p ) const;					// exact compare, no epsilon

	void			Zero( void );							// zero plane
	void			SetNormal( const noVec3 &normal );		// sets the normal
	const noVec3 &	Normal( void ) const;					// reference to const normal
	noVec3 &		Normal( void );							// reference to normal
	float			Normalize( bool fixDegenerate = true );	// only normalizes the plane normal, does not adjust d
	bool			FixDegenerateNormal( void );			// fix degenerate normal
	bool			FixDegeneracies( float distEpsilon );	// fix degenerate normal and dist
	float			Dist( void ) const;						// returns: -d
	void			SetDist( const float dist );			// sets: d = -dist
	int				Type( void ) const;						// returns plane type

	bool			FromPoints( const noVec3 &p1, const noVec3 &p2, const noVec3 &p3, bool fixDegenerate = true );
	bool			FromVecs( const noVec3 &dir1, const noVec3 &dir2, const noVec3 &p, bool fixDegenerate = true );
	void			FitThroughPoint( const noVec3 &p );	// assumes normal is valid
	bool			HeightFit( const noVec3 *points, const int numPoints );
	noPlane			Translate( const noVec3 &translation ) const;
	noPlane &		TranslateSelf( const noVec3 &translation );
	noPlane			Rotate( const noVec3 &origin, const noMat3 &axis ) const;
	noPlane &		RotateSelf( const noVec3 &origin, const noMat3 &axis );

	float			Distance( const noVec3 &v ) const;
	int				Side( const noVec3 &v, const float epsilon = 0.0f ) const;

	bool			LineIntersection( const noVec3 &start, const noVec3 &end ) const;
					// intersection point is start + dir * scale
	bool			RayIntersection( const noVec3 &start, const noVec3 &dir, float &scale ) const;
	bool			PlaneIntersection( const noPlane &plane, noVec3 &start, noVec3 &dir ) const;

	int				GetDimension( void ) const;

	const noVec4 &	ToVec4( void ) const;
	noVec4 &		ToVec4( void );
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;

private:
	float			a;
	float			b;
	float			c;
	float			d;
};

extern noPlane plane_origin;
#define plane_zero plane_origin

NO_INLINE noPlane::noPlane( void ) {
}

NO_INLINE noPlane::noPlane( float a, float b, float c, float d ) {
	this->a = a;
	this->b = b;
	this->c = c;
	this->d = d;
}

NO_INLINE noPlane::noPlane( const noVec3 &normal, const float dist ) {
	this->a = normal.x;
	this->b = normal.y;
	this->c = normal.z;
	this->d = -dist;
}

NO_INLINE float noPlane::operator[]( int index ) const {
	return ( &a )[ index ];
}

NO_INLINE float& noPlane::operator[]( int index ) {
	return ( &a )[ index ];
}

NO_INLINE noPlane noPlane::operator-() const {
	return noPlane( -a, -b, -c, -d );
}

NO_INLINE noPlane &noPlane::operator=( const noVec3 &v ) { 
	a = v.x;
	b = v.y;
	c = v.z;
	d = 0;
	return *this;
}

NO_INLINE noPlane noPlane::operator+( const noPlane &p ) const {
	return noPlane( a + p.a, b + p.b, c + p.c, d + p.d );
}

NO_INLINE noPlane noPlane::operator-( const noPlane &p ) const {
	return noPlane( a - p.a, b - p.b, c - p.c, d - p.d );
}

NO_INLINE noPlane &noPlane::operator*=( const noMat3 &m ) {
	Normal() *= m;
	return *this;
}

NO_INLINE bool noPlane::Compare( const noPlane &p ) const {
	return ( a == p.a && b == p.b && c == p.c && d == p.d );
}

NO_INLINE bool noPlane::Compare( const noPlane &p, const float epsilon ) const {
	if ( noMath::Fabs( a - p.a ) > epsilon ) {
		return false;
	}
			
	if ( noMath::Fabs( b - p.b ) > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( c - p.c ) > epsilon ) {
		return false;
	}

	if ( noMath::Fabs( d - p.d ) > epsilon ) {
		return false;
	}

	return true;
}

NO_INLINE bool noPlane::Compare( const noPlane &p, const float normalEps, const float distEps ) const {
	if ( noMath::Fabs( d - p.d ) > distEps ) {
		return false;
	}
	if ( !Normal().Compare( p.Normal(), normalEps ) ) {
		return false;
	}
	return true;
}

NO_INLINE bool noPlane::operator==( const noPlane &p ) const {
	return Compare( p );
}

NO_INLINE bool noPlane::operator!=( const noPlane &p ) const {
	return !Compare( p );
}

NO_INLINE void noPlane::Zero( void ) {
	a = b = c = d = 0.0f;
}

NO_INLINE void noPlane::SetNormal( const noVec3 &normal ) {
	a = normal.x;
	b = normal.y;
	c = normal.z;
}

NO_INLINE const noVec3 &noPlane::Normal( void ) const {
	return *reinterpret_cast<const noVec3 *>(&a);
}

NO_INLINE noVec3 &noPlane::Normal( void ) {
	return *reinterpret_cast<noVec3 *>(&a);
}

NO_INLINE float noPlane::Normalize( bool fixDegenerate ) {
	float length = reinterpret_cast<noVec3 *>(&a)->Normalize();

	if ( fixDegenerate ) {
		FixDegenerateNormal();
	}
	return length;
}

NO_INLINE bool noPlane::FixDegenerateNormal( void ) {
	return Normal().FixDegenerateNormal();
}

NO_INLINE bool noPlane::FixDegeneracies( float distEpsilon ) {
	bool fixedNormal = FixDegenerateNormal();
	// only fix dist if the normal was degenerate
	if ( fixedNormal ) {
		if ( noMath::Fabs( d - noMath::Rint( d ) ) < distEpsilon ) {
			d = noMath::Rint( d );
		}
	}
	return fixedNormal;
}

NO_INLINE float noPlane::Dist( void ) const {
	return -d;
}

NO_INLINE void noPlane::SetDist( const float dist ) {
	d = -dist;
}

NO_INLINE bool noPlane::FromPoints( const noVec3 &p1, const noVec3 &p2, const noVec3 &p3, bool fixDegenerate ) {
	Normal() = (p1 - p2).Cross( p3 - p2 );
	if ( Normalize( fixDegenerate ) == 0.0f ) {
		return false;
	}
	d = -( Normal() * p2 );
	return true;
}

NO_INLINE bool noPlane::FromVecs( const noVec3 &dir1, const noVec3 &dir2, const noVec3 &p, bool fixDegenerate ) {
	Normal() = dir1.Cross( dir2 );
	if ( Normalize( fixDegenerate ) == 0.0f ) {
		return false;
	}
	d = -( Normal() * p );
	return true;
}

NO_INLINE void noPlane::FitThroughPoint( const noVec3 &p ) {
	d = -( Normal() * p );
}

NO_INLINE noPlane noPlane::Translate( const noVec3 &translation ) const {
	return noPlane( a, b, c, d - translation * Normal() );
}

NO_INLINE noPlane &noPlane::TranslateSelf( const noVec3 &translation ) {
	d -= translation * Normal();
	return *this;
}

NO_INLINE noPlane noPlane::Rotate( const noVec3 &origin, const noMat3 &axis ) const {
	noPlane p;
	p.Normal() = Normal() * axis;
	p.d = d + origin * Normal() - origin * p.Normal();
	return p;
}

NO_INLINE noPlane &noPlane::RotateSelf( const noVec3 &origin, const noMat3 &axis ) {
	d += origin * Normal();
	Normal() *= axis;
	d -= origin * Normal();
	return *this;
}

NO_INLINE float noPlane::Distance( const noVec3 &v ) const {
	return a * v.x + b * v.y + c * v.z + d;
}

NO_INLINE int noPlane::Side( const noVec3 &v, const float epsilon ) const {
	float dist = Distance( v );
	if ( dist > epsilon ) {
		return PLANESIDE_FRONT;
	}
	else if ( dist < -epsilon ) {
		return PLANESIDE_BACK;
	}
	else {
		return PLANESIDE_ON;
	}
}

NO_INLINE bool noPlane::LineIntersection( const noVec3 &start, const noVec3 &end ) const {
	float d1, d2, fraction;

	d1 = Normal() * start + d;
	d2 = Normal() * end + d;
	if ( d1 == d2 ) {
		return false;
	}
	if ( d1 > 0.0f && d2 > 0.0f ) {
		return false;
	}
	if ( d1 < 0.0f && d2 < 0.0f ) {
		return false;
	}
	fraction = ( d1 / ( d1 - d2 ) );
	return ( fraction >= 0.0f && fraction <= 1.0f );
}

NO_INLINE bool noPlane::RayIntersection( const noVec3 &start, const noVec3 &dir, float &scale ) const {
	float d1, d2;

	d1 = Normal() * start + d;
	d2 = Normal() * dir;
	if ( d2 == 0.0f ) {
		return false;
	}
	scale = -( d1 / d2 );
	return true;
}

NO_INLINE int noPlane::GetDimension( void ) const {
	return 4;
}

NO_INLINE const noVec4 &noPlane::ToVec4( void ) const {
	return *reinterpret_cast<const noVec4 *>(&a);
}

NO_INLINE noVec4 &noPlane::ToVec4( void ) {
	return *reinterpret_cast<noVec4 *>(&a);
}

NO_INLINE const float *noPlane::ToFloatPtr( void ) const {
	return reinterpret_cast<const float *>(&a);
}

NO_INLINE float *noPlane::ToFloatPtr( void ) {
	return reinterpret_cast<float *>(&a);
}




#endif