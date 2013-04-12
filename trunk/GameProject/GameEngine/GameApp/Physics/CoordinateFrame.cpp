#include "stdafx.h"
#include "CoordinateFrame.h"


//////////////////////
// ROTATION_MATRIX3 //
//////////////////////



///////////
// BASIS //
///////////




const noQuat BASIS::quat()
{
	noQuat q;
	float tr, s;

	tr = R[0][0] + R[1][1] + R[2][2];

	if( tr >= 0 )
	{
		s = sqrtf( tr + 1 );
		q.w = 0.5f * s;
		s = 0.5f / s;
		q.x = (R[2][1] - R[1][2]) * s;
		q.y = (R[0][2] - R[2][0]) * s;
		q.z = (R[1][0] - R[0][1]) * s;
	}
	else
	{
		if( R[1][1] > R[0][0] )//case 1
		{
			s = sqrtf( (R[1][1] - (R[2][2] + R[0][0])) + 1 );
			q.y = 0.5f * s;
			s = 0.5f / s;
			q.z = (R[1][2] + R[2][1]) * s;
			q.x = (R[0][1] + R[1][0]) * s;
			q.w = (R[0][2] - R[2][0]) * s;
		}
		else if( R[2][2] > R[1][1] )//case 2
		{
			s = sqrtf( (R[2][2] - (R[0][0] + R[1][1])) + 1 );
			q.z = 0.5f * s;
			s = 0.5f / s;
			q.x = (R[2][0] + R[0][2]) * s;
			q.y = (R[1][2] + R[2][1]) * s;
			q.w = (R[1][0] - R[0][1]) * s;
		}
		else//case 0
		{
			s = sqrtf( (R[0][0] - (R[1][1] + R[2][2])) + 1 );
			q.x = 0.5f * s;
			s = 0.5f / s;
			q.y = (R[0][1] + R[1][0]) * s;
			q.z = (R[2][0] + R[0][2]) * s;
			q.w = (R[2][1] - R[1][2]) * s;
		}
	}

	return q;
}


///////////////////////////////////////////////////////////////////////////
void BASIS::rotateAboutZ( const float a )
{
	if( 0 != a )//don't rotate by 0
	{
		//don't over-write basis before calculation is done
		noVec3 b0 = this->X()*cosf(a) + this->Y()*sinf(a);	//rotate x
		noVec3 b1 = -this->X()*sinf(a) + this->Y()*cosf(a);	//rotate y

		//set basis
		this->R[0] = b0;
		this->R[1] = b1;
		//z is unchanged
	}
}


///////////////////////////////////////////////////////////////////////////
void BASIS::rotateAboutX( const float a )
{
	if( 0 != a )//don't rotate by 0
	{
		noVec3 b1 = this->Y()*cosf(a) + this->Z()*sinf(a);
		noVec3 b2 = -this->Y()*sinf(a) + this->Z()*cosf(a);

		//set basis
		this->R[1] = b1;
		this->R[2] = b2;
		//x is unchanged
	}
}


///////////////////////////////////////////////////////////////////////////
void BASIS::rotateAboutY( const float a )
{
	if( 0 != a )//don't rotate by 0
	{
		noVec3 b2 = this->Z()*cosf(a) + this->X()*sinf(a);	//rotate z
		noVec3 b0 = -this->Z()*sinf(a) + this->X()*cosf(a);	//rotate x

		//set basis
		this->R[2] = b2;
		this->R[0] = b0;
		//y is unchanged
	}
}


///////////////////////////////////////////////////////////////////////////
void BASIS::rotate( const float theta, const noVec3& u )
{
	if( 0 != theta )//don't rotate by 0
	{
		noRotation rot(vec3_zero, u, theta);
		//const ROTATION_MATRIX r( theta, u );
		const noMat3 r = rot.ToMat3();			 

		//rotate each basis vector
		this->R[0] = r * this->R[0];
		this->R[1] = r * this->R[1];
		this->R[2] = r * this->R[2];
	}
}


///////////////////////////////////////////////////////////////////////////
void BASIS::rotate( const noVec3& v )
{
	float theta = v * v;//angle to rotate by

	if( 0 != theta )//don't rotate by 0
	{
		theta = sqrtf( theta );
		this->rotate( theta, v * (1/theta) );//unit vector is axis
	}
}

