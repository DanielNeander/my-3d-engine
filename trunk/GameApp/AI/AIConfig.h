#pragma once 



#define AICON (*AIConfig::Instance())

class AIConfig 
{
public:	
	std::string mFilename;	

	AIConfig()
	{		
		load("AI.cfg");
	}

	AIConfig(const std::string& filename) :mFilename(filename)
	{		
		load(filename);
	}

	static AIConfig* Instance();
	void load(const std::string& _cfgfile);


	int	NumAgents;
	int	NumObstacles;
	double MinObstacleRadius;
	double MaxObstacleRadius;

	//number of horizontal cells used for spatial partitioning
	int   NumCellsX;
	//number of vertical cells used for spatial partitioning
	int   NumCellsY;

	//how many samples the smoother will use to average a value
	int   NumSamplesForSmoothing;

	//how many samples the smoother will use to average the AI an animation values
	int	  NumFrameSamplesForSmoothing;

	//used to tweak the combined steering force (simply altering the MaxSteeringForce
	//will NOT work!This tweaker affects all the steering force multipliers
	//too).
	double SteeringForceTweaker;

	double MaxSteeringForce;
	double MaxSpeed;
	double VehicleMass;

	double VehicleScale;
	double MaxTurnRatePerSecond;

	double SeparationWeight;
	double AlignmentWeight;
	double CohesionWeight;
	double ObstacleAvoidanceWeight;
	double WallAvoidanceWeight;
	double WanderWeight;
	double SeekWeight;
	double FleeWeight;
	double ArriveWeight;
	double PursuitWeight;
	double OffsetPursuitWeight;
	double InterposeWeight;
	double HideWeight;
	double EvadeWeight;
	double FollowPathWeight;

	//how close a neighbour must be before an agent perceives it (considers it
	//to be within its neighborhood)
	double ViewDistance;

	//used in obstacle avoidance
	double MinDetectionBoxLength;

	//used in wall avoidance
	double WallDetectionFeelerLength;

	//these are the probabilities that a steering behavior will be used
	//when the prioritized dither calculate method is used
	double prWallAvoidance;
	double prObstacleAvoidance;
	double prSeparation;
	double prAlignment;
	double prCohesion;
	double prWander;
	double prSeek;
	double prFlee;
	double prEvade;
	double prHide;
	double prArrive;
};

