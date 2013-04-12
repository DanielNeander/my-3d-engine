#include "stdafx.h"
#include "UnMath.h"


FLOAT FInterpTo( FLOAT& Current, FLOAT& Target, FLOAT& DeltaTime, FLOAT InterpSpeed )
{
	// If no interp speed, jump to target value
	if( InterpSpeed == 0.f )
	{
		return Target;
	}

	// Distance to reach
	const FLOAT Dist = Target - Current;

	// If distance is too small, just set the desired location
	if( Square(Dist) < SMALL_NUMBER )
	{
		return Target;
	}

	// Delta Move, Clamp so we do not over shoot.
	const FLOAT DeltaMove = Dist * Clamp<FLOAT>(DeltaTime * InterpSpeed, 0.f, 1.f);

	return Current + DeltaMove;
}

/** Interpolate between A and B, applying an ease in/out function.  Exp controls the degree of the curve. */
FLOAT FInterpEaseInOut( FLOAT A, FLOAT B, FLOAT Alpha, FLOAT Exp )
{
	FLOAT ModifiedAlpha;

	if( Alpha < 0.5f )
	{
		ModifiedAlpha = 0.5f * appPow(2.f * Alpha, Exp);
	}
	else
	{
		ModifiedAlpha = 1.f - 0.5f * appPow(2.f * (1.f - Alpha), Exp);
	}

	return Lerp<FLOAT>(A, B, ModifiedAlpha);
}


/**
 * Clamps a tangent formed by the specified control point values
 */
FLOAT FClampFloatTangent( FLOAT PrevPointVal, FLOAT PrevTime, FLOAT CurPointVal, FLOAT CurTime, FLOAT NextPointVal, FLOAT NextTime )
{
	const FLOAT PrevToNextTimeDiff = Max< DOUBLE >( KINDA_SMALL_NUMBER, NextTime - PrevTime );
	const FLOAT PrevToCurTimeDiff = Max< DOUBLE >( KINDA_SMALL_NUMBER, CurTime - PrevTime );
	const FLOAT CurToNextTimeDiff = Max< DOUBLE >( KINDA_SMALL_NUMBER, NextTime - CurTime );

	FLOAT OutTangentVal = 0.0f;

	const FLOAT PrevToNextHeightDiff = NextPointVal - PrevPointVal;
	const FLOAT PrevToCurHeightDiff = CurPointVal - PrevPointVal;
	const FLOAT CurToNextHeightDiff = NextPointVal - CurPointVal;

	// Check to see if the current point is crest
	if( ( PrevToCurHeightDiff >= 0.0f && CurToNextHeightDiff <= 0.0f ) ||
		( PrevToCurHeightDiff <= 0.0f && CurToNextHeightDiff >= 0.0f ) )
	{
		// Neighbor points are both both on the same side, so zero out the tangent
		OutTangentVal = 0.0f;
	}
	else
	{
		// The three points form a slope

		// Constants
		const FLOAT ClampThreshold = 0.333f;

		// Compute height deltas
		const FLOAT CurToNextTangent = CurToNextHeightDiff / CurToNextTimeDiff;
		const FLOAT PrevToCurTangent = PrevToCurHeightDiff / PrevToCurTimeDiff;
		const FLOAT PrevToNextTangent = PrevToNextHeightDiff / PrevToNextTimeDiff;

		// Default to not clamping
		const FLOAT UnclampedTangent = PrevToNextTangent;
		FLOAT ClampedTangent = UnclampedTangent;

		const FLOAT LowerClampThreshold = ClampThreshold;
		const FLOAT UpperClampThreshold = 1.0f - ClampThreshold;

		// @todo: Would we get better results using percentange of TIME instead of HEIGHT?
		const FLOAT CurHeightAlpha = PrevToCurHeightDiff / PrevToNextHeightDiff;

		if( PrevToNextHeightDiff > 0.0f )
		{
			if( CurHeightAlpha < LowerClampThreshold )
			{
				// 1.0 = maximum clamping (flat), 0.0 = minimal clamping (don't touch)
				const FLOAT ClampAlpha = 1.0f - CurHeightAlpha / ClampThreshold;
				const FLOAT LowerClamp = Lerp( PrevToNextTangent, PrevToCurTangent, ClampAlpha );
				ClampedTangent = Min( ClampedTangent, LowerClamp );
			}

			if( CurHeightAlpha > UpperClampThreshold )
			{
				// 1.0 = maximum clamping (flat), 0.0 = minimal clamping (don't touch)
				const FLOAT ClampAlpha = ( CurHeightAlpha - UpperClampThreshold ) / ClampThreshold;
				const FLOAT UpperClamp = Lerp( PrevToNextTangent, CurToNextTangent, ClampAlpha );
				ClampedTangent = Min( ClampedTangent, UpperClamp );
			}
		}
		else
		{

			if( CurHeightAlpha < LowerClampThreshold )
			{
				// 1.0 = maximum clamping (flat), 0.0 = minimal clamping (don't touch)
				const FLOAT ClampAlpha = 1.0f - CurHeightAlpha / ClampThreshold;
				const FLOAT LowerClamp = Lerp( PrevToNextTangent, PrevToCurTangent, ClampAlpha );
				ClampedTangent = Max( ClampedTangent, LowerClamp );
			}

			if( CurHeightAlpha > UpperClampThreshold )
			{
				// 1.0 = maximum clamping (flat), 0.0 = minimal clamping (don't touch)
				const FLOAT ClampAlpha = ( CurHeightAlpha - UpperClampThreshold ) / ClampThreshold;
				const FLOAT UpperClamp = Lerp( PrevToNextTangent, CurToNextTangent, ClampAlpha );
				ClampedTangent = Max( ClampedTangent, UpperClamp );
			}
		}

		OutTangentVal = ClampedTangent;
	}

	return OutTangentVal;
}

/**
 * Spherical interpolation. Will correct alignment. Output is not normalized.
 */
FQuat SlerpQuat( const FQuat &Quat1,const FQuat &Quat2, FLOAT Slerp )
{
	// Get cosine of angle between quats.
	const FLOAT RawCosom = 
		Quat1.x * Quat2.x +
		Quat1.y * Quat2.y +
		Quat1.z * Quat2.z +
		Quat1.w * Quat2.w;
	// Unaligned quats - compensate, results in taking shorter route.
	const FLOAT Cosom = FloatSelect( RawCosom, RawCosom, -RawCosom );

	FLOAT Scale0, Scale1;

	if( Cosom < 0.9999f )
	{	
		const FLOAT Omega = appAcos(Cosom);
		const FLOAT InvSin = 1.f/appSin(Omega);
		Scale0 = appSin( (1.f - Slerp) * Omega ) * InvSin;
		Scale1 = appSin( Slerp * Omega ) * InvSin;
	}
	else
	{
		// Use linear interpolation.
		Scale0 = 1.0f - Slerp;
		Scale1 = Slerp;	
	}

	// In keeping with our flipped Cosom:
	Scale1 = FloatSelect( RawCosom, Scale1, -Scale1 );

	FQuat Result;

	Result.x = Scale0 * Quat1.x + Scale1 * Quat2.x;
	Result.y = Scale0 * Quat1.y + Scale1 * Quat2.y;
	Result.z = Scale0 * Quat1.z + Scale1 * Quat2.z;
	Result.w = Scale0 * Quat1.w + Scale1 * Quat2.w;

	return Result;
}

// Simpler Slerp that doesn't do any checks for 'shortest distance' etc.
// We need this for the cubic interpolation stuff so that the multiple Slerps dont go in different directions.
FQuat SlerpQuatFullPath( const FQuat &quat1, const FQuat &quat2, FLOAT Alpha )
{
	const FLOAT CosAngle = Clamp<FLOAT>(quat1 | quat2, -1.f, 1.f);
	const FLOAT Angle = appAcos(CosAngle);

	//debugf( TEXT("CosAngle: %f Angle: %f"), CosAngle, Angle );

	if ( Abs(Angle) < KINDA_SMALL_NUMBER )
	{
		return quat1;
	}

	const FLOAT SinAngle = appSin(Angle);
	const FLOAT InvSinAngle = 1.f/SinAngle;

	const FLOAT Scale0 = appSin((1.0f-Alpha)*Angle)*InvSinAngle;
	const FLOAT Scale1 = appSin(Alpha*Angle)*InvSinAngle;

	return quat1*Scale0 + quat2*Scale1;
}

// Given start and end quaternions of quat1 and quat2, and tangents at those points tang1 and tang2, calculate the point at Alpha (between 0 and 1) between them.
FQuat SquadQuat( const FQuat& quat1, const FQuat& tang1, const FQuat& quat2, const FQuat& tang2, FLOAT Alpha )
{
	const FQuat Q1 = SlerpQuatFullPath(quat1, quat2, Alpha);
	//debugf(TEXT("Q1: %f %f %f %f"), Q1.X, Q1.Y, Q1.Z, Q1.W);

	const FQuat Q2 = SlerpQuatFullPath(tang1, tang2, Alpha);
	//debugf(TEXT("Q2: %f %f %f %f"), Q2.X, Q2.Y, Q2.Z, Q2.W);

	const FQuat Result = SlerpQuatFullPath(Q1, Q2, 2.f * Alpha * (1.f - Alpha));
	//FQuat Result = SlerpQuat(Q1, Q2, 2.f * Alpha * (1.f - Alpha));
	//debugf(TEXT("Result: %f %f %f %f"), Result.X, Result.Y, Result.Z, Result.W);

	return Result;
}

/*-----------------------------------------------------------------------------
	Bezier curves
-----------------------------------------------------------------------------*/

/**
 * Generates a list of sample points on a Bezier curve defined by 2 points.
 *
 * @param	ControlPoints	Array of 4 FVectors (vert1, controlpoint1, controlpoint2, vert2).
 * @param	NumPoints		Number of samples.
 * @param	OutPoints		Receives the output samples.
 * @return					Path length.
 */

FLOAT EvaluateBezier( const FVector* ControlPoints, INT NumPoints, TArray<FVector>& OutPoints )
{
	assert( ControlPoints );
	assert( NumPoints >= 2 );

	// var q is the change in t between successive evaluations.
	const FLOAT q = 1.f/(NumPoints-1); // q is dependent on the number of GAPS = POINTS-1

	// recreate the names used in the derivation
	const FVector& P0 = ControlPoints[0];
	const FVector& P1 = ControlPoints[1];
	const FVector& P2 = ControlPoints[2];
	const FVector& P3 = ControlPoints[3];

	// coefficients of the cubic polynomial that we're FDing -
	const FVector a = P0;
	const FVector b = 3*(P1-P0);
	const FVector c = 3*(P2-2*P1+P0);
	const FVector d = P3-3*P2+3*P1-P0;

	// initial values of the poly and the 3 diffs -
	FVector S  = a;						// the poly value
	FVector U  = b*q + c*q*q + d*q*q*q;	// 1st order diff (quadratic)
	FVector V  = 2*c*q*q + 6*d*q*q*q;	// 2nd order diff (linear)
	FVector W  = 6*d*q*q*q;				// 3rd order diff (constant)

	// Path length.
	FLOAT Length = 0.f;

	FVector OldPos = P0;
	OutPoints.AddItem( P0 );	// first point on the curve is always P0.

	for( INT i = 1 ; i < NumPoints ; ++i )
	{
		// calculate the next value and update the deltas
		S += U;			// update poly value
		U += V;			// update 1st order diff value
		V += W;			// update 2st order diff value
		// 3rd order diff is constant => no update needed.

		// Update Length.
		Length += FDist( S, OldPos );
		OldPos  = S;

		OutPoints.AddItem( S );
	}

	// Return path length as experienced in sequence (linear interpolation between points).
	return Length;
}
