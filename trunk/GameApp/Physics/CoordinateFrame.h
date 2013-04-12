#pragma once 


const noVec3 kX(1,0,0), kY(0,1,0), kZ(0,0,1);

// Set of orthonormal basis vectors with respect to a parent
class BASIS 
{
public:
	noMat3	R;

public:

	BASIS()
	{}

	BASIS( 
		const noVec3&	v0,
		const noVec3&	v1,
		const noVec3&	v2
		)
		:	R( v0, v1, v2 )
	{}

	BASIS( const noMat3& m )
		:	R( m )
	{}

	BASIS( const noQuat& q ) 
		:	R(	noVec3(	1 - 2*q.y*q.y - 2*q.z*q.z,	2*q.x*q.y - 2*q.w*q.z,		2*q.x*q.z + 2*q.w*q.y ),
		noVec3(	2*q.x*q.y + 2*q.w*q.z,		1 - 2*q.x*q.x - 2*q.z*q.z,	2*q.y*q.z - 2*q.w*q.x ),
		noVec3(	2*q.x*q.z - 2*q.w*q.y,		2*q.y*q.z + 2*q.w*q.x,		1 - 2*q.x*q.x - 2*q.y*q.y )	)
	{
	}

	const noVec3& operator [] ( long i ) const		{ return R.mat[i]; }

	const noVec3& X() const		{ return R.mat[0]; }
	const noVec3& Y() const		{ return R.mat[1]; }
	const noVec3& Z() const		{ return R.mat[2]; }

	const noQuat quat();
	const noMat3& matrix() const			{ return R; }

	void basis( const noVec3& v0, const noVec3& v1, const noVec3& v2 )
	{
		this->R[0] = v0;
		this->R[1] = v1;
		this->R[2] = v2;
	}

	// Right-Handed Rotations
	void rotateAboutX	( const float a );
	void rotateAboutZ	( const float a );
	void rotateAboutY	( const float a );

	//rotate the basis about the unit axis u by theta (radians)
	void rotate( const float theta, const noVec3& u );

	//rotate, length of da is theta, unit direction of da is u
	void rotate( const noVec3& da );


	// Transformations
	const noVec3 transformVectorToLocal( const noVec3& v ) const
	{
		return noVec3( R.mat[0]* v, R.mat[1] * (v), R.mat[2] * (v) );
	}

	const noVec3 transformVectorToParent( const noVec3& v ) const
	{
		return R.mat[0] * v.x + R.mat[1] * v.y + R.mat[2] * v.z;
	}

};


class CoordFrame : public BASIS 
{
public:
	noVec3 orig_;

public:
	CoordFrame() {}
	CoordFrame( const noVec3& o, const noVec3& v0, const noVec3& v1, const noVec3& v2)
		: orig_(o), BASIS(v0, v1, v2)
	{}
	CoordFrame( const noMat3& basis, const noVec3& orig_in)
		: orig_(orig_in), BASIS( basis )
	{}

	const noVec3& position() const { return orig_; }
	void position( float x, float y, float z )		{ orig_ = noVec3(x,y,z); }
	void position( const noVec3& p )				{ orig_ = p; }

	const noVec3 transformPointToLocal(  const noVec3& p ) const
	{
		//translate to this frame's orig_in, then project onto this basis
		return transformVectorToLocal( p - orig_ );
	}

	const noVec3 transformPointToParent( const noVec3& p ) const
	{
		//transform the coordinate vector and translate by this orig_in
		return transformVectorToParent( p ) + orig_;
	}

	//translate the orig_in by the given vector
	void translate( const noVec3& v )
	{
		orig_ += v;
	}
		

};