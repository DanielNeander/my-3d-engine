#pragma once 

#include "Wall2D.h"
#include "AIPath.h"

class ActorController;
//class AIPath;

class SteeringBehavior 
{
public:

	enum summing_method{weighted_average, prioritized, dithered};

private:

	enum behavior_type
	{
		none               = 0x00000,
		seek               = 0x00002,
		flee               = 0x00004,
		arrive             = 0x00008,
		wander             = 0x00010,
		cohesion           = 0x00020,
		separation         = 0x00040,
		allignment         = 0x00080,
		obstacle_avoidance = 0x00100,
		wall_avoidance     = 0x00200,
		follow_path        = 0x00400,
		pursuit            = 0x00800,
		evade              = 0x01000,
		interpose          = 0x02000,
		hide               = 0x04000,
		flock              = 0x08000,
		offset_pursuit     = 0x10000,
	};

private:
	ActorController*		m_pOwner;

	noVec3    m_vSteeringForce;

	ActorController*     m_pTargetAgent1;
	ActorController*     m_pTargetAgent2;

	//the current target
	noVec3    m_vTarget;

	//length of the 'detection box' utilized in obstacle avoidance
	double                 m_dDBoxLength;


	//a vertex buffer to contain the feelers rqd for wall avoidance  
	std::vector<noVec3> m_Feelers;

	//the length of the 'feeler/s' used in wall detection
	double                 m_dWallDetectionFeelerLength;



	//the current position on the wander circle the agent is
	//attempting to steer towards
	noVec3     m_vWanderTarget; 

	//explained above
	double        m_dWanderJitter;
	double        m_dWanderRadius;
	double        m_dWanderDistance;


	//multipliers. These can be adjusted to effect strength of the  
	//appropriate behavior. Useful to get flocking the way you require
	//for example.
	double        m_dWeightSeparation;
	double        m_dWeightCohesion;
	double        m_dWeightAlignment;
	double        m_dWeightWander;
	double        m_dWeightObstacleAvoidance;
	double        m_dWeightWallAvoidance;
	double        m_dWeightSeek;
	double        m_dWeightFlee;
	double        m_dWeightArrive;
	double        m_dWeightPursuit;
	double        m_dWeightOffsetPursuit;
	double        m_dWeightInterpose;
	double        m_dWeightHide;
	double        m_dWeightEvade;
	double        m_dWeightFollowPath;

	//how far the agent can 'see'
	double        m_dViewDistance;

	  //pointer to any current path
  AIPath*          m_pPath;

  //the distance (squared) a GameObject has to be from a path waypoint before
  //it starts seeking to the next waypoint
  double        m_dWaypointSeekDistSq;


  //any offset used for formations or offset pursuit
  noVec3     m_vOffset;



  //binary flags to indicate whether or not a behavior should be active
  int           m_iFlags;

  
  //Arrive makes use of these to determine how quickly a GameObject
  //should decelerate to its target
  enum Deceleration{slow = 3, normal = 2, fast = 1};

  //default
  Deceleration m_Deceleration;

  //is cell space partitioning to be used or not?
  bool          m_bCellSpaceOn;
 
  //what type of method is used to sum any active behavior
  summing_method  m_SummingMethod;


  //this function tests if a specific bit of m_iFlags is set
  bool      On(behavior_type bt){return (m_iFlags & bt) == bt;}

  bool      AccumulateForce(noVec3 &sf, noVec3 ForceToAdd);

  //creates the antenna utilized by the wall avoidance behavior
  void      CreateFeelers();



   /* .......................................................

                    BEGIN BEHAVIOR DECLARATIONS

      .......................................................*/


  //this behavior moves the agent towards a target position
  noVec3 Seek(noVec3 TargetPos);

  //this behavior returns a vector that moves the agent away
  //from a target position
  noVec3 Flee(noVec3 TargetPos);

  //this behavior is similar to seek but it attempts to arrive 
  //at the target position with a zero velocity
  noVec3 Arrive(noVec3     TargetPos,
                  Deceleration deceleration);

  //this behavior predicts where an agent will be in time T and seeks
  //towards that point to intercept it.
  noVec3 Pursuit(const ActorController* agent);

  //this behavior maintains a position, in the direction of offset
  //from the target GameObject
  noVec3 OffsetPursuit(const ActorController* agent, const noVec3 offset);

  //this behavior attempts to evade a pursuer
  noVec3 Evade(const ActorController* agent);

  //this behavior makes the agent wander about randomly
  noVec3 Wander();

  //this returns a steering force which will attempt to keep the agent 
  //away from any obstacles it may encounter
  noVec3 ObstacleAvoidance(const std::vector<ActorController*>& obstacles);  
  //this returns a steering force which will keep the agent away from any
  //walls it may encounter
  noVec3 WallAvoidance(const std::vector<Wall2D> &walls);

  
  //given a series of noVec3s, this method produces a force that will
  //move the agent along the waypoints in order
  noVec3 FollowPath();

  //this results in a steering force that attempts to steer the MovingActor
  //to the center of the vector connecting two moving agents.
  noVec3 Interpose(const ActorController* GameObjectA, const ActorController* GameObjectB);

  //given another agent position to hide from and a list of Actors this
  //method attempts to put an obstacle between itself and its opponent
  noVec3 Hide(const ActorController* hunter, const std::vector<ActorController*>& obstacles);


  // -- Group Behaviors -- //

  noVec3 Cohesion(const std::vector<ActorController*> &agents);
  
  noVec3 Separation(const std::vector<ActorController*> &agents);

  noVec3 Alignment(const std::vector<ActorController*> &agents);

  //the following three are the same as above but they use cell-space
  //partitioning to find the neighbors
  noVec3 CohesionPlus(const std::vector<ActorController*> &agents);
  noVec3 SeparationPlus(const std::vector<ActorController*> &agents);
  noVec3 AlignmentPlus(const std::vector<ActorController*> &agents);

    /* .......................................................

                       END BEHAVIOR DECLARATIONS

      .......................................................*/

  //calculates and sums the steering forces from any active behaviors
  noVec3 CalculateWeightedSum();
  noVec3 CalculatePrioritized();
  noVec3 CalculateDithered();

  //helper method for Hide. Returns a position located on the other
  //side of an obstacle to the pursuer
  noVec3 GetHidingPosition(const noVec3& posOb,
                              const double     radiusOb,
                              const noVec3& posHunter);



  
  
public:
  SteeringBehavior(ActorController* agent);

  virtual ~SteeringBehavior();

  //calculates and sums the steering forces from any active behaviors
  noVec3 Calculate();

  //calculates the component of the steering force that is parallel
  //with the GameObject heading
  double    ForwardComponent();

  //calculates the component of the steering force that is perpendicuar
  //with the GameObject heading
  double    SideComponent();



  //renders visual aids and info for seeing how each behavior is
  //calculated
  void      RenderAids();

  void      SetTarget(const noVec3 t){m_vTarget = t;}

  void      SetTargetAgent1(ActorController* Agent){m_pTargetAgent1 = Agent;}
  void      SetTargetAgent2(ActorController* Agent){m_pTargetAgent2 = Agent;}

  void      SetOffset(const noVec3 offset){m_vOffset = offset;}
  noVec3  GetOffset()const{return m_vOffset;}

  void      SetPath(std::list<noVec3> new_path){m_pPath->Set(new_path);}
  void		SetPathLoopOn() { m_pPath->LoopOn(); }
  void		SetPathLoopOff() { m_pPath->LoopOff(); }
  void		SetPathLoop(bool _loop) { m_pPath->SetLoop(_loop); }
  void      CreateRandomPath(int num_waypoints, int mx, int my, int cx, int cy)const
            {m_pPath->CreateRandomPath(num_waypoints, mx, my, cx, cy);}
  bool		PathDone() { return m_pPath->GetPathRunThroughOnce(); }
  void		SetPathDone(bool _doneOnce) { m_pPath->SetPathRunThroughOnce(_doneOnce); }

  noVec3 Force()const{return m_vSteeringForce;}

  void      ToggleSpacePartitioningOnOff(){m_bCellSpaceOn = !m_bCellSpaceOn;}
  bool      isSpacePartitioningOn()const{return m_bCellSpaceOn;}

  void      SetSummingMethod(summing_method sm){m_SummingMethod = sm;}


  void FleeOn(){m_iFlags |= flee;}
  void SeekOn(){m_iFlags |= seek;}
  void ArriveOn(){m_iFlags |= arrive;}
  void WanderOn(){m_iFlags |= wander;}
  void PursuitOn(ActorController* v){m_iFlags |= pursuit; m_pTargetAgent1 = v;}
  void EvadeOn(ActorController* v){m_iFlags |= evade; m_pTargetAgent1 = v;}
  void CohesionOn(){m_iFlags |= cohesion;}
  void SeparationOn(){m_iFlags |= separation;}
  void AlignmentOn(){m_iFlags |= allignment;}
  void ObstacleAvoidanceOn(){m_iFlags |= obstacle_avoidance;}
  void WallAvoidanceOn(){m_iFlags |= wall_avoidance;}
  void FollowPathOn(){m_iFlags |= follow_path;}
  void InterposeOn(ActorController* v1, ActorController* v2){m_iFlags |= interpose; m_pTargetAgent1 = v1; m_pTargetAgent2 = v2;}
  void HideOn(ActorController* v){m_iFlags |= hide; m_pTargetAgent1 = v;}
  void OffsetPursuitOn(ActorController* v1, const noVec3 offset){m_iFlags |= offset_pursuit; m_vOffset = offset; m_pTargetAgent1 = v1;}  
  void FlockingOn(){CohesionOn(); AlignmentOn(); SeparationOn(); WanderOn();}

  void FleeOff()  {if(On(flee))   m_iFlags ^=flee;}
  void SeekOff()  {if(On(seek))   m_iFlags ^=seek;}
  void ArriveOff(){if(On(arrive)) m_iFlags ^=arrive;}
  void WanderOff(){if(On(wander)) m_iFlags ^=wander;}
  void PursuitOff(){if(On(pursuit)) m_iFlags ^=pursuit;}
  void EvadeOff(){if(On(evade)) m_iFlags ^=evade;}
  void CohesionOff(){if(On(cohesion)) m_iFlags ^=cohesion;}
  void SeparationOff(){if(On(separation)) m_iFlags ^=separation;}
  void AlignmentOff(){if(On(allignment)) m_iFlags ^=allignment;}
  void ObstacleAvoidanceOff(){if(On(obstacle_avoidance)) m_iFlags ^=obstacle_avoidance;}
  void WallAvoidanceOff(){if(On(wall_avoidance)) m_iFlags ^=wall_avoidance;}
  void FollowPathOff(){if(On(follow_path)) m_iFlags ^=follow_path;}
  void InterposeOff(){if(On(interpose)) m_iFlags ^=interpose;}
  void HideOff(){if(On(hide)) m_iFlags ^=hide;}
  void OffsetPursuitOff(){if(On(offset_pursuit)) m_iFlags ^=offset_pursuit;}
  void FlockingOff(){CohesionOff(); AlignmentOff(); SeparationOff(); WanderOff();}

  bool isFleeOn(){return On(flee);}
  bool isSeekOn(){return On(seek);}
  bool isArriveOn(){return On(arrive);}
  bool isWanderOn(){return On(wander);}
  bool isPursuitOn(){return On(pursuit);}
  bool isEvadeOn(){return On(evade);}
  bool isCohesionOn(){return On(cohesion);}
  bool isSeparationOn(){return On(separation);}
  bool isAlignmentOn(){return On(allignment);}
  bool isObstacleAvoidanceOn(){return On(obstacle_avoidance);}
  bool isWallAvoidanceOn(){return On(wall_avoidance);}
  bool isFollowPathOn(){return On(follow_path);}
  bool isInterposeOn(){return On(interpose);}
  bool isHideOn(){return On(hide);}
  bool isOffsetPursuitOn(){return On(offset_pursuit);}

  double DBoxLength()const{return m_dDBoxLength;}
  const std::vector<noVec3>& GetFeelers()const{return m_Feelers;}
  
  double WanderJitter()const{return m_dWanderJitter;}
  double WanderDistance()const{return m_dWanderDistance;}
  double WanderRadius()const{return m_dWanderRadius;}

  double SeparationWeight()const{return m_dWeightSeparation;}
  double AlignmentWeight()const{return m_dWeightAlignment;}
  double CohesionWeight()const{return m_dWeightCohesion;}
};