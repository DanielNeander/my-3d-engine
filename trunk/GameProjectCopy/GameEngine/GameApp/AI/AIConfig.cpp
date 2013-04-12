#include "stdafx.h"
#include "../ConfigFile.h"
#include "AIConfig.h"


AIConfig* AIConfig::Instance()
{
	static AIConfig instance;

	return &instance;
}


void AIConfig::load( const std::string& _cfgfile )
{
	static bool bFirst = true;
	
	if (bFirst == false)
		return;

	ConfigFile config(_cfgfile);
	NumAgents               = config.pInt("NumAgents");
	NumObstacles            = config.pInt("NumObstacles");
	MinObstacleRadius       = config.pFloat("MinObstacleRadius");
	MaxObstacleRadius       = config.pFloat("MaxObstacleRadius");

	NumCellsX               = config.pInt("NumCellsX");
	NumCellsY               = config.pInt("NumCellsY");

	NumSamplesForSmoothing  = config.pInt("NumSamplesForSmoothing");
	NumFrameSamplesForSmoothing = config.pInt("NumFrameSamplesForSmoothing");

	SteeringForceTweaker    = config.pFloat("SteeringForceTweaker");
	MaxSteeringForce        = config.pFloat("MaxSteeringForce") * SteeringForceTweaker;
	MaxSpeed                = config.pFloat("MaxSpeed");
	VehicleMass             = config.pFloat("VehicleMass");
	VehicleScale            = config.pFloat("VehicleScale");

	SeparationWeight        = config.pFloat("SeparationWeight") * SteeringForceTweaker;
	AlignmentWeight         = config.pFloat("AlignmentWeight") * SteeringForceTweaker;
	CohesionWeight          = config.pFloat("CohesionWeight") * SteeringForceTweaker;
	ObstacleAvoidanceWeight = config.pFloat("ObstacleAvoidanceWeight") * SteeringForceTweaker;
	WallAvoidanceWeight     = config.pFloat("WallAvoidanceWeight") * SteeringForceTweaker;
	WanderWeight            = config.pFloat("WanderWeight") * SteeringForceTweaker; 
	SeekWeight              = config.pFloat("SeekWeight") * SteeringForceTweaker;
	FleeWeight              = config.pFloat("FleeWeight") * SteeringForceTweaker;
	ArriveWeight            = config.pFloat("ArriveWeight") * SteeringForceTweaker;
	PursuitWeight           = config.pFloat("PursuitWeight") * SteeringForceTweaker;
	OffsetPursuitWeight     = config.pFloat("OffsetPursuitWeight") * SteeringForceTweaker;
	InterposeWeight         = config.pFloat("InterposeWeight") * SteeringForceTweaker;
	HideWeight              = config.pFloat("HideWeight") * SteeringForceTweaker;
	EvadeWeight             = config.pFloat("EvadeWeight") * SteeringForceTweaker;
	FollowPathWeight        = config.pFloat("FollowPathWeight") * SteeringForceTweaker;

	ViewDistance            = config.pFloat("ViewDistance");
	MinDetectionBoxLength   = config.pFloat("MinDetectionBoxLength");
	WallDetectionFeelerLength = config.pFloat("WallDetectionFeelerLength");

	prWallAvoidance         = config.pFloat("prWallAvoidance");
	prObstacleAvoidance     = config.pFloat("prObstacleAvoidance");
	prSeparation            = config.pFloat("prSeparation");
	prAlignment             = config.pFloat("prAlignment");
	prCohesion              = config.pFloat("prCohesion");
	prWander                = config.pFloat("prWander");
	prSeek                  = config.pFloat("prSeek");
	prFlee                  = config.pFloat("prFlee");
	prEvade                 = config.pFloat("prEvade");
	prHide                  = config.pFloat("prHide");
	prArrive                = config.pFloat("prArrive");

	MaxTurnRatePerSecond    = noMath::PI;

	bFirst = false;
}


