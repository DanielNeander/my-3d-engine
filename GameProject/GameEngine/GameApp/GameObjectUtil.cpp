#include "stdafx.h"
#include "GameObjectUtil.h"
#include "EngineCore/Math/Frustum.h"

bool MathHelpers::ms_bYisUp = true;
float MathHelpers::ms_oneMeter = 1.0f;

noVec3 FORWARD_VECTOR(0,-1,0);
noVec3 UP_VECTOR(0,0,1);
noVec3 DOWN_VECTOR(0,0,-1);
noVec3 RIGHT_VECTOR(1,0,0);
noVec3 LEFT_VECTOR(-1,0,0);

static bool FloatEqual(float f1, float f2, float maxDelta=0.01f)
{
	if( fabs(f1-f2)<maxDelta )
		return true;

	return false;
}

float & MathHelpers::UP_COMPONENT( noVec3 &pos )
{
	if( MathHelpers::IsYUp() )
		return pos.y;

	return pos.z;
}

float MathHelpers::UP_COMPONENT( const noVec3 &pos )
{
	if( MathHelpers::IsYUp() )
		return pos.y;

	return pos.z;
}

float & MathHelpers::FORWARD_COMPONENT( noVec3 &pos )
{
	if( MathHelpers::IsYUp() )
		return pos.z;

	return pos.y;
}

float MathHelpers::FORWARD_COMPONENT( const noVec3 &pos )
{
	if( MathHelpers::IsYUp() )
		return pos.z;

	return pos.y;
}

float MathHelpers::GetOneMeter()
{
	return ms_oneMeter;

}

void MathHelpers::SetOneMeter( float fOneMeter )
{
	ms_oneMeter = fOneMeter;

}

void MathHelpers::SetYUp( bool b )
{
	ms_bYisUp = b;

	if (ms_bYisUp)
	{
		FORWARD_VECTOR      = noVec3(1,0,0);
		UP_VECTOR           = noVec3(0,1,0);
		RIGHT_VECTOR        = noVec3(0,0,-1);
	}
	else
	{
		FORWARD_VECTOR      = noVec3(0,-1,0);
		UP_VECTOR           = noVec3(0,0,1);
		RIGHT_VECTOR        = noVec3(1,0,0);
	}
}

float MathHelpers::GetCurrentYaw( const noMat3 &rotation )
{
	noVec3    forward = MathHelpers::GetForwardVectorFromMatrix(rotation);

	UP_COMPONENT(forward) = 0;

	float dot = FORWARD_VECTOR * forward;
	float yaw = acosf(dot);

	if ( forward * RIGHT_VECTOR > 0 )
		yaw *=-1;

	return yaw;
}



void MathHelpers::FaceDirection( noMat3 &Matrix, const noVec3 &cvDesiredFacing )
{
	noVec3 vDesiredFacing = cvDesiredFacing;
	vDesiredFacing.Normalize();

	// Quick method to orient the matrix.  This will "work", but cause some instability when facing within eight degrees of the y-axis.
	noVec3 vTempUp = UP_VECTOR;
	if(fabs(cvDesiredFacing * vTempUp) > 0.99f)
	{
		vTempUp = FORWARD_VECTOR;
	}

	// Figure out left and up vectors
	noVec3 vLeft;
	vLeft = vTempUp.Cross(cvDesiredFacing);
	vLeft.Normalize();

	noVec3 vRealUp;
	vRealUp = vDesiredFacing.Cross(vLeft);

	Matrix[0]=-vDesiredFacing;
	Matrix[1]=vRealUp;
	Matrix[2]=vLeft;
}

void MathHelpers::FaceDirectionLight( noMat3 &Matrix, const noVec3 &cvDesiredFacing )
{
	noVec3 vDesiredFacing = cvDesiredFacing;
	vDesiredFacing.Normalize();

	// Quick method to orient the matrix.  This will "work", but cause some instability when facing within eight degrees of the y-axis.
	noVec3 vTempUp = UP_VECTOR;
	if(fabs(cvDesiredFacing * vTempUp) > 0.99f)
	{
		vTempUp = FORWARD_VECTOR;
	}

	// Figure out left and up vectors
	noVec3 vLeft;
	vLeft = vTempUp.Cross(cvDesiredFacing);
	vLeft.Normalize();

	noVec3 vRealUp;
	vRealUp = vDesiredFacing.Cross(vLeft);

	// Set Mtx
	Matrix[0]=vDesiredFacing;
	Matrix[1]=vRealUp;
	Matrix[2]=-vLeft;
}

float MathHelpers::GetAngleBetweenVectors( const noVec3 &u, const noVec3 &v )
{
	float dot		= u * v ;

	float cosTheta = ( dot / (u.LengthSqr() * v.LengthSqr()) );

	if ( FloatEqual(cosTheta,1.0f) )
		return 0.0f;

	float arcCosTheta = acosf ( cosTheta );
	return arcCosTheta; // result is in radians..
}

float MathHelpers::GetAngleBetweenActorDirections( const noVec3 &vDir1, const noVec3 &vDir2 )
{
	float   fAmt,fAmt2;

	if ( IsYUp() )
	{
		fAmt	= noMath::ATan(vDir1.x, vDir1.z);
		fAmt2   = noMath::ATan(vDir2.x, vDir2.z);
	}
	else
	{
		fAmt	= noMath::ATan (vDir1.x, vDir1.y);
		fAmt2   = noMath::ATan (vDir2.x, vDir2.y);
	}

	float angle = fAmt - fAmt2;

	angle	= RAD2DEG(angle);

	if ( fabs(angle) > 180.0f )
	{
		// nope, turn the other way!!
		if ( angle>=0 )
			angle	= -(360.0f - angle);
		else
			angle	= 360.0f + angle;
	}

	if( IsYUp() )
		return -angle;  // todo: Figure out why this is necessary....

	return angle;
}

noVec3 MathHelpers::NormalizeVector( const noVec3 &vector )
{
	noVec3    temp = vector;
	temp.Normalize();
	return temp;
}

void MathHelpers::Slerp( noQuat &dest, const noQuat &q1, const noQuat &q2, float t )
{
	Slerp( (float*)&dest,(const float*)&q1, (const float*)&q2, t );
}

void MathHelpers::Slerp( float *pDest, const float *pQ1, const float *pQ2, float t )
{
	float	rot1q[4];
	float	omega, cosom, sinom;
	float	scalerot0, scalerot1;
	int		i, j;


	// Calculate the cosine
	cosom = pQ1[0]*pQ2[0] + pQ1[1]*pQ2[1] + pQ1[2]*pQ2[2] + pQ1[3]*pQ2[3];

	// adjust signs if necessary
	if(cosom < 0.0f)
	{
		cosom = -cosom;
		for(j = 0; j < 4; j++ )
			rot1q[j] = -pQ2[j];
	}
	else  
	{
		for (j = 0; j < 4; j++ )
			rot1q[j] = pQ2[j];
	}

	// calculate interpolating coeffs
	if ( (1.0 - cosom) > 0.00001f ) 
	{ 
		// standard case
		omega = (float)acos(cosom);
		sinom = (float)sin(omega);
		scalerot0 = (float)sin((1.0 - t) * omega) / sinom;
		scalerot1 = (float)sin(t * omega) / sinom;
	}
	else
	{ 
		// rot0 and rot1 very close - just do linear interp.
		scalerot0 = 1.0f - t;
		scalerot1 = t;
	}

	for (i = 0; i < 4; i++) // build the new quarternion
		pDest[i] = (float)(scalerot0 * pQ1[i] + scalerot1 * rot1q[i]);
}

float MathHelpers::Dist2D( const noVec3 &pos1, const noVec3 &pos2 )
{
	noVec3    p1 = pos1;
	noVec3    p2 = pos2;

	UP_COMPONENT(p1) = 0;
	UP_COMPONENT(p2) = 0;

	return (p1 - p2).Length();
}

bool MathHelpers::IsZero( const noVec3 &point, float fValue/*=0.0001f*/ )
{
	if( point.Length()<fValue )
		return true;

	return false;
}


#define ISNAN(x)  ((*(UINT*)&(x) & 0x7F800000) == 0x7F800000 && (*(UINT*)&(x) & 0x7FFFFF) != 0)
#define ISINF(x)  ((*(UINT*)&(x) & 0x7FFFFFFF) == 0x7F800000)

noVec4 MathHelpers::ReciprocalEst( noVec4 V)
{
	noVec4 Result;
	UINT     i;

	// Avoid C4701
	Result[0] = 0.0f;

	for (i = 0; i < 4; i++)
	{
		if (ISNAN(V[i]))
		{
			Result[i] = 0x7FC00000;
		}
		else if (V[i] == 0.0f || V[i] == -0.0f)
		{
			Result[i] = 0x7F800000 | ((uint32)V[i] & 0x80000000);
		}
		else
		{
			Result[i] = 1.f / V[i];
		}
	}
	return Result;
}


//-----------------------------------------------------------------------------
// Build a frustum from a persepective projection matrix.  The matrix may only
// contain a projection; any rotation, translation or scale will cause the
// constructed frustum to be incorrect.
//-----------------------------------------------------------------------------
VOID MathHelpers::ComputeFrustumFromProjection( Frustum* pOut, float* pProjection )
{
	assert( pOut );
	assert( pProjection );

	// Corners of the projection frustum in homogenous space.
	static noVec4 HomogenousPoints[6] =
	{
		noVec4(  1.0f,  0.0f, 1.0f, 1.0f ),   // right (at far plane)
		noVec4( -1.0f,  0.0f, 1.0f, 1.0f ),   // left
		noVec4(  0.0f,  1.0f, 1.0f, 1.0f ),   // top
		noVec4(  0.0f, -1.0f, 1.0f, 1.0f ),   // bottom

		noVec4( 0.0f, 0.0f, 0.0f, 1.0f ),     // near
		noVec4( 0.0f, 0.0f, 1.0f, 1.0f )      // far
	};

	noVec3 Determinant;
	noMat4 proj(pProjection);
	noMat4 matInverse = proj.Inverse();
	
	// Compute the frustum corners in world space.
	noVec4 Points[6];
	for( INT i = 0; i < 6; i++ )
	{
		// Transform point.
		Points[i] = matInverse * HomogenousPoints[i];
	}

	pOut->Origin = noVec3( 0.0f, 0.0f, 0.0f );
	pOut->Orientation = noVec4( 0.0f, 0.0f, 0.0f, 1.0f );

	// Compute the slopes.
	noVec4 pt;
	pt.SetAll( Points[0].z);
	Points[0] = Points[0].Multiply(ReciprocalEst( pt ));
	pt.SetAll( Points[1].z);	
	Points[1] = Points[1].Multiply( ReciprocalEst( pt) );
	pt.SetAll( Points[2].z);	
	Points[2] = Points[2].Multiply( ReciprocalEst( pt) );
	pt.SetAll( Points[3].z);	
	Points[3] = Points[3].Multiply( ReciprocalEst( pt) );

	pOut->RightSlope = ( Points[0] ).x;
	pOut->LeftSlope = ( Points[1] ).x;
	pOut->TopSlope = ( Points[2] ).y;
	pOut->BottomSlope = ( Points[3] ).y;

	// Compute near and far.
	pt.SetAll( Points[4].w);
	Points[4] = Points[4].Multiply( ReciprocalEst( pt ) );
	pt.SetAll( Points[5].w);
	Points[5] = Points[5].Multiply( ReciprocalEst( pt ) );

	pOut->Near = ( Points[4] ).z;
	pOut->Far =  ( Points[5] ).z;

	return;
}


	