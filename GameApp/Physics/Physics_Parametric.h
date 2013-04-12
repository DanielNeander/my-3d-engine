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

#ifndef __PHYSICS_PARAMETRIC_H__
#define __PHYSICS_PARAMETRIC_H__

/*
===================================================================================

	Parametric physics

	Used for predefined or scripted motion. The motion of an object is completely
	parametrized. By adjusting the parameters an object is forced to follow a
	predefined path. The parametric physics is typically used for doors, bridges,
	rotating fans etc.

===================================================================================
*/

typedef struct parametricPState_s {
	int										time;					// physics time
	int										atRest;					// set when simulation is suspended
	noVec3									origin;					// world origin
	noAngles								angles;					// world angles
	noMat3									axis;					// world axis
	noVec3									localOrigin;			// local origin
	noAngles								localAngles;			// local angles
	idExtrapolate<noVec3>					linearExtrapolation;	// extrapolation based description of the position over time
	idExtrapolate<noAngles>					angularExtrapolation;	// extrapolation based description of the orientation over time
	idInterpolateAccelDecelLinear<noVec3>	linearInterpolation;	// interpolation based description of the position over time
	idInterpolateAccelDecelLinear<noAngles>	angularInterpolation;	// interpolation based description of the orientation over time
	idCurve_Spline<noVec3> *				spline;					// spline based description of the position over time
	idInterpolateAccelDecelLinear<float>	splineInterpolate;		// position along the spline over time
	bool									useSplineAngles;		// set the orientation using the spline
} parametricPState_t;

class idPhysics_Parametric : public idPhysics_Base {

public:
	CLASS_PROTOTYPE( idPhysics_Parametric );

							idPhysics_Parametric( void );
							~idPhysics_Parametric( void );

	//void					Save( idSaveGame *savefile ) const;
	//void					Restore( idRestoreGame *savefile );

	void					SetPusher( int flags );
	bool					IsPusher( void ) const;

	void					SetLinearExtrapolation( extrapolation_t type, int time, int duration, const noVec3 &base, const noVec3 &speed, const noVec3 &baseSpeed );
	void					SetAngularExtrapolation( extrapolation_t type, int time, int duration, const noAngles &base, const noAngles &speed, const noAngles &baseSpeed );
	extrapolation_t			GetLinearExtrapolationType( void ) const;
	extrapolation_t			GetAngularExtrapolationType( void ) const;

	void					SetLinearInterpolation( int time, int accelTime, int decelTime, int duration, const noVec3 &startPos, const noVec3 &endPos );
	void					SetAngularInterpolation( int time, int accelTime, int decelTime, int duration, const noAngles &startAng, const noAngles &endAng );

	void					SetSpline( idCurve_Spline<noVec3> *spline, int accelTime, int decelTime, bool useSplineAngles );
	idCurve_Spline<noVec3> *GetSpline( void ) const;
	int						GetSplineAcceleration( void ) const;
	int						GetSplineDeceleration( void ) const;
	bool					UsingSplineAngles( void ) const;

	void					GetLocalOrigin( noVec3 &curOrigin ) const;
	void					GetLocalAngles( noAngles &curAngles ) const;

	void					GetAngles( noAngles &curAngles ) const;

public:	// common physics interface
	void					SetClipModel( idClipModel *model, float density, int id = 0, bool freeOld = true );
	idClipModel *			GetClipModel( int id = 0 ) const;
	int						GetNumClipModels( void ) const;

	void					SetMass( float mass, int id = -1 );
	float					GetMass( int id = -1 ) const;

	void					SetContents( int contents, int id = -1 );
	int						GetContents( int id = -1 ) const;

	const idBounds &		GetBounds( int id = -1 ) const;
	const idBounds &		GetAbsBounds( int id = -1 ) const;

	bool					Evaluate( int timeStepMSec, int endTimeMSec );
	void					UpdateTime( int endTimeMSec );
	int						GetTime( void ) const;

	void					Activate( void );
	bool					IsAtRest( void ) const;
	int						GetRestStartTime( void ) const;
	bool					IsPushable( void ) const;

	void					SaveState( void );
	void					RestoreState( void );

	void					SetOrigin( const noVec3 &newOrigin, int id = -1 );
	void					SetAxis( const noMat3 &newAxis, int id = -1 );

	void					Translate( const noVec3 &translation, int id = -1 );
	void					Rotate( const noRotation &rotation, int id = -1 );

	const noVec3 &			GetOrigin( int id = 0 ) const;
	const noMat3 &			GetAxis( int id = 0 ) const;

	void					SetLinearVelocity( const noVec3 &newLinearVelocity, int id = 0 );
	void					SetAngularVelocity( const noVec3 &newAngularVelocity, int id = 0 );

	const noVec3 &			GetLinearVelocity( int id = 0 ) const;
	const noVec3 &			GetAngularVelocity( int id = 0 ) const;

	void					DisableClip( void );
	void					EnableClip( void );

	void					UnlinkClip( void );
	void					LinkClip( void );

	void					SetMaster( idEntity *master, const bool orientated = true );

	const trace_t *			GetBlockingInfo( void ) const;
	idEntity *				GetBlockingEntity( void ) const;

	int						GetLinearEndTime( void ) const;
	int						GetAngularEndTime( void ) const;

	void					WriteToSnapshot( idBitMsgDelta &msg ) const;
	void					ReadFromSnapshot( const idBitMsgDelta &msg );

private:
	// parametric physics state
	parametricPState_t		current;
	parametricPState_t		saved;

	// pusher
	bool					isPusher;
	idClipModel *			clipModel;
	int						pushFlags;

	// results of last evaluate
	trace_t					pushResults;
	bool					isBlocked;

	// master
	bool					hasMaster;
	bool					isOrientated;

private:
	bool					TestIfAtRest( void ) const;
	void					Rest( void );
};

#endif /* !__PHYSICS_PARAMETRIC_H__ */
