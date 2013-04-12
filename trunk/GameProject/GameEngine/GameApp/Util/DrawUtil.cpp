#include "stdafx.h"
#include "DrawUtil.h"

idCVar r_debugLineDepthTest( "r_debugLineDepthTest", "0", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "perform depth test on debug lines" );
idCVar r_debugLineWidth( "r_debugLineWidth", "1", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_BOOL, "width of debug lines" );
idCVar r_debugArrowStep( "r_debugArrowStep", "120", CVAR_RENDERER | CVAR_ARCHIVE | CVAR_INTEGER, "step size of arrow cone line rotation in degrees", 0, 120 );

noVec4	colorBlack	= noVec4( 0.00f, 0.00f, 0.00f, 1.00f );
noVec4	colorWhite	= noVec4( 1.00f, 1.00f, 1.00f, 1.00f );
noVec4	colorRed	= noVec4( 1.00f, 0.00f, 0.00f, 1.00f );
noVec4	colorGreen	= noVec4( 0.00f, 1.00f, 0.00f, 1.00f );
noVec4	colorBlue	= noVec4( 0.00f, 0.00f, 1.00f, 1.00f );
noVec4	colorYellow	= noVec4( 1.00f, 1.00f, 0.00f, 1.00f );
noVec4	colorMagenta= noVec4( 1.00f, 0.00f, 1.00f, 1.00f );
noVec4	colorCyan	= noVec4( 0.00f, 1.00f, 1.00f, 1.00f );
noVec4	colorOrange	= noVec4( 1.00f, 0.50f, 0.00f, 1.00f );
noVec4	colorPurple	= noVec4( 0.60f, 0.00f, 0.60f, 1.00f );
noVec4	colorPink	= noVec4( 0.73f, 0.40f, 0.48f, 1.00f );
noVec4	colorBrown	= noVec4( 0.40f, 0.35f, 0.08f, 1.00f );
noVec4	colorLtGrey	= noVec4( 0.75f, 0.75f, 0.75f, 1.00f );
noVec4	colorMdGrey	= noVec4( 0.50f, 0.50f, 0.50f, 1.00f );
noVec4	colorDkGrey	= noVec4( 0.25f, 0.25f, 0.25f, 1.00f );


debugLine_t		DrawUtil::rb_debugLines[ MAX_DEBUG_LINES ];
int				DrawUtil::rb_numDebugLines = 0;
int				DrawUtil::rb_debugLineTime = 0;

void DrawUtil::RB_ClearDebugLines( int time ) 
{

}

void DrawUtil::RB_AddDebugLine( const noVec4 &color, const noVec3 &start, const noVec3 &end, const int lifeTime, const bool depthTest ) {
	debugLine_t *line;

	if ( rb_numDebugLines < MAX_DEBUG_LINES ) {
		line = &rb_debugLines[ rb_numDebugLines++ ];
		line->rgb		= color;
		line->start		= start;
		line->end		= end;
		line->depthTest = depthTest;
		line->lifeTime	= rb_debugLineTime + lifeTime;
	}
}

void DrawUtil::DebugLine( const noVec4 &color, const noVec3 &start, const noVec3 &end, const int lifetime /*= 0*/, const bool depthTest /*= false */ ) {
	RB_AddDebugLine( color, start, end, lifetime, depthTest );
}

void DrawUtil::DebugArrow( const noVec4 &color, const noVec3 &start, const noVec3 &end, int size, const int lifetime /*= 0 */ ) {
	noVec3 forward, right, up, v1, v2;
	float a, s;
	int i;
	static float arrowCos[40];
	static float arrowSin[40];
	static int arrowStep;

	DebugLine( color, start, end, lifetime );

	if ( r_debugArrowStep.GetInteger() <= 10 ) {
		return;
	}
	// calculate sine and cosine when step size changes
	if ( arrowStep != r_debugArrowStep.GetInteger() ) {
		arrowStep = r_debugArrowStep.GetInteger();
		for (i = 0, a = 0; a < 360.0f; a += arrowStep, i++) {
			arrowCos[i] = noMath::Cos16( DEG2RAD( a ) );
			arrowSin[i] = noMath::Sin16( DEG2RAD( a ) );
		}
		arrowCos[i] = arrowCos[0];
		arrowSin[i] = arrowSin[0];
	}
	// draw a nice arrow
	forward = end - start;
	forward.Normalize();
	forward.NormalVectors( right, up);
	for (i = 0, a = 0; a < 360.0f; a += arrowStep, i++) {
		s = 0.5f * size * arrowCos[i];
		v1 = end - size * forward;
		v1 = v1 + s * right;
		s = 0.5f * size * arrowSin[i];
		v1 = v1 + s * up;

		s = 0.5f * size * arrowCos[i+1];
		v2 = end - size * forward;
		v2 = v2 + s * right;
		s = 0.5f * size * arrowSin[i+1];
		v2 = v2 + s * up;

		DebugLine( color, v1, end, lifetime );
		DebugLine( color, v1, v2, lifetime );
	}
}

void DrawUtil::DebugCone( const noVec4 &color, const noVec3 &apex, const noVec3 &dir, float radius1, float radius2, const int lifetime /*= 0 */ ) {
	int i;
	noMat3 axis;
	noVec3 top, p1, p2, lastp1, lastp2, d;

	axis[2] = dir;
	axis[2].Normalize();
	axis[2].NormalVectors( axis[0], axis[1] );
	axis[1] = -axis[1];

	top = apex + dir;
	lastp2 = top + radius2 * axis[1];

	if ( radius1 == 0.0f ) {
		for ( i = 20; i <= 360; i += 20 ) {
			d = noMath::Sin16( DEG2RAD(i) ) * axis[0] + noMath::Cos16( DEG2RAD(i) ) * axis[1];
			p2 = top + d * radius2;
			DebugLine( color, lastp2, p2, lifetime );
			DebugLine( color, p2, apex, lifetime );
			lastp2 = p2;
		}
	} else {
		lastp1 = apex + radius1 * axis[1];
		for ( i = 20; i <= 360; i += 20 ) {
			d = noMath::Sin16( DEG2RAD(i) ) * axis[0] + noMath::Cos16( DEG2RAD(i) ) * axis[1];
			p1 = apex + d * radius1;
			p2 = top + d * radius2;
			DebugLine( color, lastp1, p1, lifetime );
			DebugLine( color, lastp2, p2, lifetime );
			DebugLine( color, p1, p2, lifetime );
			lastp1 = p1;
			lastp2 = p2;
		}
	}
}

void DrawUtil::DebugAxis( const noVec3 &origin, const noMat3 &axis ) {
	noVec3 start = origin;
	noVec3 end = start + axis[0] * 20.0f;
	DebugArrow( colorWhite, start, end, 2 );
	end = start + axis[0] * -20.0f;
	DebugArrow( colorWhite, start, end, 2 );
	end = start + axis[1] * +20.0f;
	DebugArrow( colorGreen, start, end, 2 );
	end = start + axis[1] * -20.0f;
	DebugArrow( colorGreen, start, end, 2 );
	end = start + axis[2] * +20.0f;
	DebugArrow( colorBlue, start, end, 2 );
	end = start + axis[2] * -20.0f;
	DebugArrow( colorBlue, start, end, 2 );
}

debugPolygon_t DrawUtil::rb_debugPolygons[MAX_DEBUG_POLYGONS];



