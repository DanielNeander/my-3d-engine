#include "stdafx.h"
#include "Transform2DFuncs.h"
#include "Intersection2D.h"
#include "AIPath.h"
#include "AIConfig.h"
#include "../Actor.h"
#include "Wall2D.h"
#include "GameWorld.h"
#include "SteeringBehavior.h"

using std::string;
using std::vector;

//--------------------------- Constants ----------------------------------

//the radius of the constraining circle for the wander behavior
static const double WanderRad    = 2.0;
//distance the wander circle is projected in front of the agent
static const double WanderDist   = 2.0;
//the maximum amount of displacement along the circle each frame
static const double WanderJitterPerSec = 80.0;

//used in path following
static const double WaypointSeekDist   = 10;                                          



SteeringBehavior::SteeringBehavior(ActorController* agent):                         
             m_pOwner(agent),
             m_iFlags(0),
             m_dDBoxLength(AICON.MinDetectionBoxLength),
             m_dWeightCohesion(AICON.CohesionWeight),
             m_dWeightAlignment(AICON.AlignmentWeight),
			 m_dWeightSeparation(AICON.SeparationWeight),
			 m_dWeightObstacleAvoidance(AICON.ObstacleAvoidanceWeight),
             m_dWeightWander(AICON.WanderWeight),
             m_dWeightWallAvoidance(AICON.WallAvoidanceWeight),
             m_dViewDistance(AICON.ViewDistance),
             m_dWallDetectionFeelerLength(AICON.WallDetectionFeelerLength),
             m_Feelers(3),
             m_Deceleration(normal),
             m_pTargetAgent1(NULL),
             m_pTargetAgent2(NULL),
             m_dWanderDistance(WanderDist),
             m_dWanderJitter(WanderJitterPerSec),
             m_dWanderRadius(WanderRad),
             m_dWaypointSeekDistSq(WaypointSeekDist*WaypointSeekDist),
             m_dWeightSeek(AICON.SeekWeight),
             m_dWeightFlee(AICON.FleeWeight),
             m_dWeightArrive(AICON.ArriveWeight),
             m_dWeightPursuit(AICON.PursuitWeight),
             m_dWeightOffsetPursuit(AICON.OffsetPursuitWeight),
             m_dWeightInterpose(AICON.InterposeWeight),
             m_dWeightHide(AICON.HideWeight),
             m_dWeightEvade(AICON.EvadeWeight),
             m_dWeightFollowPath(AICON.FollowPathWeight),
             m_bCellSpaceOn(false),
             m_SummingMethod(prioritized)


{
  //stuff for the wander behavior
  double theta = RandFloat() * noMath::TWO_PI;

  //create a vector to a target position on the wander circle
  m_vWanderTarget = noVec3(m_dWanderRadius * cos(theta), 0.0f, 
                              m_dWanderRadius * sin(theta));

  //create a Path
  m_pPath = new AIPath();
  m_pPath->LoopOn();

}

//---------------------------------dtor ----------------------------------
SteeringBehavior::~SteeringBehavior(){delete m_pPath;}


/////////////////////////////////////////////////////////////////////////////// CALCULATE METHODS 


//----------------------- Calculate --------------------------------------
//
//  calculates the accumulated steering force according to the method set
//  in m_SummingMethod
//------------------------------------------------------------------------
noVec3 SteeringBehavior::Calculate()
{ 
  //reset the steering force
  m_vSteeringForce.Zero();

  //use space partitioning to calculate the neighbours of this vehicle
  //if switched on. If not, use the standard tagging system
  if (!isSpacePartitioningOn())
  {
    //tag neighbors if any of the following 3 group behaviors are switched on
    if (On(separation) || On(allignment) || On(cohesion))
    {
      m_pOwner->World()->TagVehiclesWithinViewRange(m_pOwner, m_dViewDistance);
    }
  }
  else
  {
    //calculate neighbours in cell-space if any of the following 3 group
    //behaviors are switched on
    if (On(separation) || On(allignment) || On(cohesion))
    {
      m_pOwner->World()->CellSpace()->CalculateNeighbors(m_pOwner->Pos(), m_dViewDistance);
    }
  }

  switch (m_SummingMethod)
  {
  case weighted_average:
    
    m_vSteeringForce = CalculateWeightedSum(); break;

  case prioritized:

    m_vSteeringForce = CalculatePrioritized(); break;

  case dithered:
    
    m_vSteeringForce = CalculateDithered();break;

  default:m_vSteeringForce = vec3_zero;

  }//end switch

  return m_vSteeringForce;
}

//------------------------- ForwardComponent -----------------------------
//
//  returns the forward oomponent of the steering force
//------------------------------------------------------------------------
double SteeringBehavior::ForwardComponent()
{
  return m_pOwner->Heading() * m_vSteeringForce;
}

//--------------------------- SideComponent ------------------------------
//  returns the side component of the steering force
//------------------------------------------------------------------------
double SteeringBehavior::SideComponent()
{
  return m_pOwner->Side() * m_vSteeringForce;
}


//--------------------- AccumulateForce ----------------------------------
//
//  This function calculates how much of its max steering force the 
//  vehicle has left to apply and then applies that amount of the
//  force to add.
//------------------------------------------------------------------------
bool SteeringBehavior::AccumulateForce(noVec3 &RunningTot,
                                       noVec3 ForceToAdd)
{
  
  //calculate how much steering force the vehicle has used so far
  double MagnitudeSoFar = RunningTot.Length();

  //calculate how much steering force remains to be used by this vehicle
  double MagnitudeRemaining = m_pOwner->MaxForce() - MagnitudeSoFar;

  //return false if there is no more force left to use
  if (MagnitudeRemaining <= 0.0) return false;

  //calculate the magnitude of the force we want to add
  double MagnitudeToAdd = ForceToAdd.Length();
  
  //if the magnitude of the sum of ForceToAdd and the running total
  //does not exceed the maximum force available to this vehicle, just
  //add together. Otherwise add as much of the ForceToAdd vector is
  //possible without going over the max.
  if (MagnitudeToAdd < MagnitudeRemaining)
  {
    RunningTot += ForceToAdd;
  }

  else
  {
    //add it to the steering force
	ForceToAdd.Normalize();
    RunningTot += (ForceToAdd) * MagnitudeRemaining; 
  }

  return true;
}



//---------------------- CalculatePrioritized ----------------------------
//
//  this method calls each active steering behavior in order of priority
//  and acumulates their forces until the max steering force magnitude
//  is reached, at which time the function returns the steering force 
//  accumulated to that  point
//------------------------------------------------------------------------
noVec3 SteeringBehavior::CalculatePrioritized()
{       
  noVec3 force;
  
   if (On(wall_avoidance))
  {
    force = WallAvoidance(m_pOwner->World()->Walls()) *
            m_dWeightWallAvoidance;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }
   
  if (On(obstacle_avoidance))
  {
    force = ObstacleAvoidance(m_pOwner->World()->Obstacles()) * 
            m_dWeightObstacleAvoidance;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(evade))
  {
    assert(m_pTargetAgent1 && "Evade target not assigned");
    
    force = Evade(m_pTargetAgent1) * m_dWeightEvade;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  
  if (On(flee))
  {
    force = Flee(m_pOwner->World()->Crosshair()) * m_dWeightFlee;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }


 
  //these next three can be combined for flocking behavior (wander is
  //also a good behavior to add into this mix)
  if (!isSpacePartitioningOn())
  {
    if (On(separation))
    {
      force = Separation(m_pOwner->World()->Agents()) * m_dWeightSeparation;

      if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
    }

    if (On(allignment))
    {
      force = Alignment(m_pOwner->World()->Agents()) * m_dWeightAlignment;

      if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
    }

    if (On(cohesion))
    {
      force = Cohesion(m_pOwner->World()->Agents()) * m_dWeightCohesion;

      if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
    }
  }

  else
  {

    if (On(separation))
    {
      force = SeparationPlus(m_pOwner->World()->Agents()) * m_dWeightSeparation;

      if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
    }

    if (On(allignment))
    {
      force = AlignmentPlus(m_pOwner->World()->Agents()) * m_dWeightAlignment;

      if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
    }

    if (On(cohesion))
    {
      force = CohesionPlus(m_pOwner->World()->Agents()) * m_dWeightCohesion;

      if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
    }
  }

  if (On(seek))
  {
    force = Seek(m_pOwner->World()->Crosshair()) * m_dWeightSeek;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }


  if (On(arrive))
  {
    force = Arrive(m_pOwner->World()->Crosshair(), m_Deceleration) * m_dWeightArrive;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(wander))
  {
    force = Wander() * m_dWeightWander;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(pursuit))
  {
    assert(m_pTargetAgent1 && "pursuit target not assigned");

    force = Pursuit(m_pTargetAgent1) * m_dWeightPursuit;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(offset_pursuit))
  {
    assert (m_pTargetAgent1 && "pursuit target not assigned");
    assert (m_vOffset != vec3_zero && "No offset assigned");

    force = OffsetPursuit(m_pTargetAgent1, m_vOffset);

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(interpose))
  {
    assert (m_pTargetAgent1 && m_pTargetAgent2 && "Interpose agents not assigned");

    force = Interpose(m_pTargetAgent1, m_pTargetAgent2) * m_dWeightInterpose;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(hide))
  {
    assert(m_pTargetAgent1 && "Hide target not assigned");

    force = Hide(m_pTargetAgent1, m_pOwner->World()->Obstacles()) * m_dWeightHide;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }
  
  if (On(follow_path))
  {
    force = FollowPath() * m_dWeightFollowPath;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  return m_vSteeringForce;
}


//---------------------- CalculateWeightedSum ----------------------------
//
//  this simply sums up all the active behaviors X their weights and 
//  truncates the result to the max available steering force before 
//  returning
//------------------------------------------------------------------------
noVec3 SteeringBehavior::CalculateWeightedSum()
{        
  if (On(wall_avoidance))
  {
    m_vSteeringForce += WallAvoidance(m_pOwner->World()->Walls()) *
                         m_dWeightWallAvoidance;
  }
   
  if (On(obstacle_avoidance))
  {
    m_vSteeringForce += ObstacleAvoidance(m_pOwner->World()->Obstacles()) * 
            m_dWeightObstacleAvoidance;
  }

  if (On(evade))
  {
    assert(m_pTargetAgent1 && "Evade target not assigned");
    
    m_vSteeringForce += Evade(m_pTargetAgent1) * m_dWeightEvade;
  }


  //these next three can be combined for flocking behavior (wander is
  //also a good behavior to add into this mix)
  if (!isSpacePartitioningOn())
  {
    if (On(separation))
    {
      m_vSteeringForce += Separation(m_pOwner->World()->Agents()) * m_dWeightSeparation;
    }

    if (On(allignment))
    {
      m_vSteeringForce += Alignment(m_pOwner->World()->Agents()) * m_dWeightAlignment;
    }

    if (On(cohesion))
    {
      m_vSteeringForce += Cohesion(m_pOwner->World()->Agents()) * m_dWeightCohesion;
    }
  }
  else
  {
    if (On(separation))
    {
      m_vSteeringForce += SeparationPlus(m_pOwner->World()->Agents()) * m_dWeightSeparation;
    }

    if (On(allignment))
    {
      m_vSteeringForce += AlignmentPlus(m_pOwner->World()->Agents()) * m_dWeightAlignment;
    }

    if (On(cohesion))
    {
      m_vSteeringForce += CohesionPlus(m_pOwner->World()->Agents()) * m_dWeightCohesion;
    }
  }


  if (On(wander))
  {
    m_vSteeringForce += Wander() * m_dWeightWander;
  }

  if (On(seek))
  {
    m_vSteeringForce += Seek(m_pOwner->World()->Crosshair()) * m_dWeightSeek;
  }

  if (On(flee))
  {
    m_vSteeringForce += Flee(m_pOwner->World()->Crosshair()) * m_dWeightFlee;
  }

  if (On(arrive))
  {
    m_vSteeringForce += Arrive(m_pOwner->World()->Crosshair(), m_Deceleration) * m_dWeightArrive;
  }

  if (On(pursuit))
  {
    assert(m_pTargetAgent1 && "pursuit target not assigned");

    m_vSteeringForce += Pursuit(m_pTargetAgent1) * m_dWeightPursuit;
  }

  if (On(offset_pursuit))
  {
    assert (m_pTargetAgent1 && "pursuit target not assigned");
    assert (m_vOffset != vec3_zero && "No offset assigned");

    m_vSteeringForce += OffsetPursuit(m_pTargetAgent1, m_vOffset) * m_dWeightOffsetPursuit;
  }

  if (On(interpose))
  {
    assert (m_pTargetAgent1 && m_pTargetAgent2 && "Interpose agents not assigned");

    m_vSteeringForce += Interpose(m_pTargetAgent1, m_pTargetAgent2) * m_dWeightInterpose;
  }

  if (On(hide))
  {
    assert(m_pTargetAgent1 && "Hide target not assigned");

    m_vSteeringForce += Hide(m_pTargetAgent1, m_pOwner->World()->Obstacles()) * m_dWeightHide;
  }

  if (On(follow_path))
  {
    m_vSteeringForce += FollowPath() * m_dWeightFollowPath;
  }

  m_vSteeringForce.Truncate(m_pOwner->MaxForce());
  /*if (m_vSteeringForce.Length() > m_pOwner->MaxForce())
  {
	  m_vSteeringForce.Normalize();
	  m_vSteeringForce *= m_pOwner->MaxForce();
  }*/
 
  return m_vSteeringForce;
}


//---------------------- CalculateDithered ----------------------------
//
//  this method sums up the active behaviors by assigning a probabilty
//  of being calculated to each behavior. It then tests the first priority
//  to see if it should be calcukated this simulation-step. If so, it
//  calculates the steering force resulting from this behavior. If it is
//  more than zero it returns the force. If zero, or if the behavior is
//  skipped it continues onto the next priority, and so on.
//
//  NOTE: Not all of the behaviors have been implemented in this method,
//        just a few, so you get the general idea
//------------------------------------------------------------------------
noVec3 SteeringBehavior::CalculateDithered()
{  
  //reset the steering force
   m_vSteeringForce.Zero();

  if (On(wall_avoidance) && RandFloat() < AICON.prWallAvoidance)
  {
    m_vSteeringForce = WallAvoidance(m_pOwner->World()->Walls()) *
                         m_dWeightWallAvoidance / AICON.prWallAvoidance;

    if (m_vSteeringForce != vec3_zero)
    {
      m_vSteeringForce.Truncate(m_pOwner->MaxForce()); 
      
      return m_vSteeringForce;
    }
  }
   
  if (On(obstacle_avoidance) && RandFloat() < AICON.prObstacleAvoidance)
  {
    m_vSteeringForce += ObstacleAvoidance(m_pOwner->World()->Obstacles()) * 
            m_dWeightObstacleAvoidance / AICON.prObstacleAvoidance;

    if (m_vSteeringForce != vec3_zero)
    {
      m_vSteeringForce.Truncate(m_pOwner->MaxForce()); 
      
      return m_vSteeringForce;
    }
  }

  if (!isSpacePartitioningOn())
  {
    if (On(separation) && RandFloat() < AICON.prSeparation)
    {
      m_vSteeringForce += Separation(m_pOwner->World()->Agents()) * 
                          m_dWeightSeparation / AICON.prSeparation;

      if (m_vSteeringForce != vec3_zero)
      {
        m_vSteeringForce.Truncate(m_pOwner->MaxForce()); 
      
        return m_vSteeringForce;
      }
    }
  }

  else
  {
    if (On(separation) && RandFloat() < AICON.prSeparation)
    {
      m_vSteeringForce += SeparationPlus(m_pOwner->World()->Agents()) * 
                          m_dWeightSeparation / AICON.prSeparation;

      if (m_vSteeringForce  != vec3_zero)
      {
        m_vSteeringForce.Truncate(m_pOwner->MaxForce()); 
      
        return m_vSteeringForce;
      }
    }
  }


  if (On(flee) && RandFloat() < AICON.prFlee)
  {
    m_vSteeringForce += Flee(m_pOwner->World()->Crosshair()) * m_dWeightFlee / AICON.prFlee;

    if (m_vSteeringForce != vec3_zero)
    {
      m_vSteeringForce.Truncate(m_pOwner->MaxForce()); 
      
      return m_vSteeringForce;
    }
  }

  if (On(evade) && RandFloat() < AICON.prEvade)
  {
    assert(m_pTargetAgent1 && "Evade target not assigned");
    
    m_vSteeringForce += Evade(m_pTargetAgent1) * m_dWeightEvade / AICON.prEvade;

    if (m_vSteeringForce != vec3_zero)
    {
      m_vSteeringForce.Truncate(m_pOwner->MaxForce()); 
      
      return m_vSteeringForce;
    }
  }


  if (!isSpacePartitioningOn())
  {
    if (On(allignment) && RandFloat() < AICON.prAlignment)
    {
      m_vSteeringForce += Alignment(m_pOwner->World()->Agents()) *
                          m_dWeightAlignment / AICON.prAlignment;

      if (m_vSteeringForce != vec3_zero)
      {
        m_vSteeringForce.Truncate(m_pOwner->MaxForce()); 
      
        return m_vSteeringForce;
      }
    }

    if (On(cohesion) && RandFloat() < AICON.prCohesion)
    {
      m_vSteeringForce += Cohesion(m_pOwner->World()->Agents()) * 
                          m_dWeightCohesion / AICON.prCohesion;

      if (m_vSteeringForce != vec3_zero)
      {
        m_vSteeringForce.Truncate(m_pOwner->MaxForce()); 
      
        return m_vSteeringForce;
      }
    }
  }
  else
  {
    if (On(allignment) && RandFloat() < AICON.prAlignment)
    {
      m_vSteeringForce += AlignmentPlus(m_pOwner->World()->Agents()) *
                          m_dWeightAlignment / AICON.prAlignment;

      if (m_vSteeringForce != vec3_zero)
      {
        m_vSteeringForce.Truncate(m_pOwner->MaxForce()); 
      
        return m_vSteeringForce;
      }
    }

    if (On(cohesion) && RandFloat() < AICON.prCohesion)
    {
      m_vSteeringForce += CohesionPlus(m_pOwner->World()->Agents()) *
                          m_dWeightCohesion / AICON.prCohesion;

      if (m_vSteeringForce != vec3_zero)
      {
        m_vSteeringForce.Truncate(m_pOwner->MaxForce()); 
      
        return m_vSteeringForce;
      }
    }
  }

  if (On(wander) && RandFloat() < AICON.prWander)
  {
    m_vSteeringForce += Wander() * m_dWeightWander / AICON.prWander;

    if (m_vSteeringForce != vec3_zero)
    {
      m_vSteeringForce.Truncate(m_pOwner->MaxForce()); 
      
      return m_vSteeringForce;
    }
  }

  if (On(seek) && RandFloat() < AICON.prSeek)
  {
    m_vSteeringForce += Seek(m_pOwner->World()->Crosshair()) * m_dWeightSeek / AICON.prSeek;

    if (m_vSteeringForce != vec3_zero)
    {
      m_vSteeringForce.Truncate(m_pOwner->MaxForce()); 
      
      return m_vSteeringForce;
    }
  }

  if (On(arrive) && RandFloat() < AICON.prArrive)
  {
    m_vSteeringForce += Arrive(m_pOwner->World()->Crosshair(), m_Deceleration) * 
                        m_dWeightArrive / AICON.prArrive;

    if (m_vSteeringForce != vec3_zero)
    {
      m_vSteeringForce.Truncate(m_pOwner->MaxForce()); 
      
      return m_vSteeringForce;
    }
  }
 
  return m_vSteeringForce;
}



/////////////////////////////////////////////////////////////////////////////// START OF BEHAVIORS

//------------------------------- Seek -----------------------------------
//
//  Given a target, this behavior returns a steering force which will
//  direct the agent towards the target
//------------------------------------------------------------------------
noVec3 SteeringBehavior::Seek(noVec3 TargetPos)
{
	noVec3 temp = TargetPos - m_pOwner->Pos();
	temp.Normalize();
	noVec3 DesiredVelocity = temp * m_pOwner->MaxSpeed();

	return (DesiredVelocity - m_pOwner->Velocity());
}

//----------------------------- Flee -------------------------------------
//
//  Does the opposite of Seek
//------------------------------------------------------------------------
noVec3 SteeringBehavior::Flee(noVec3 TargetPos)
{
  //only flee if the target is within 'panic distance'. Work in distance
  //squared space.
  const double PanicDistanceSq = 100.0f * 100.0;
  if ((m_pOwner->Pos() - TargetPos).LengthSqr() > PanicDistanceSq)
  {
    return vec3_zero;
  }
  
  noVec3 temp = m_pOwner->Pos() - TargetPos;
  temp.Normalize();
  noVec3 DesiredVelocity = temp * m_pOwner->MaxSpeed();

  return (DesiredVelocity - m_pOwner->Velocity());
}

//--------------------------- Arrive -------------------------------------
//
//  This behavior is similar to seek but it attempts to arrive at the
//  target with a zero velocity
//------------------------------------------------------------------------
noVec3 SteeringBehavior::Arrive(noVec3     TargetPos,
                                  Deceleration deceleration)
{
  noVec3 ToTarget = TargetPos - m_pOwner->Pos();

  //calculate the distance to the target
  double dist = ToTarget.Length();

  if (dist > 0)
  {
    //because Deceleration is enumerated as an int, this value is required
    //to provide fine tweaking of the deceleration..
    const double DecelerationTweaker = 0.3;

    //calculate the speed required to reach the target given the desired
    //deceleration
    double speed =  dist / ((double)deceleration * DecelerationTweaker);     

    //make sure the velocity does not exceed the max
    speed = min(speed, m_pOwner->MaxSpeed());

    //from here proceed just like Seek except we don't need to normalize 
    //the ToTarget vector because we have already gone to the trouble
    //of calculating its length: dist. 
    noVec3 DesiredVelocity =  ToTarget * speed / dist;

    return (DesiredVelocity - m_pOwner->Velocity());
  }

  return vec3_zero;
}

//------------------------------ Pursuit ---------------------------------
//
//  this behavior creates a force that steers the agent towards the 
//  evader
//------------------------------------------------------------------------
noVec3 SteeringBehavior::Pursuit(const ActorController* evader)
{
  //if the evader is ahead and facing the agent then we can just seek
  //for the evader's current position.
  noVec3 ToEvader = evader->Pos() - m_pOwner->Pos();

  double RelativeHeading = m_pOwner->Heading() * evader->Heading();

  if ( (ToEvader * m_pOwner->Heading() > 0) &&  
       (RelativeHeading < -0.95))  //acos(0.95)=18 degs
  {
    return Seek(evader->Pos());
  }

  //Not considered ahead so we predict where the evader will be.
 
  //the lookahead time is propotional to the distance between the evader
  //and the pursuer; and is inversely proportional to the sum of the
  //agent's velocities
  double LookAheadTime = ToEvader.Length() / 
                        (m_pOwner->MaxSpeed() + evader->Speed());
  
  //now seek to the predicted future position of the evader
  return Seek(evader->Pos() + evader->Velocity() * LookAheadTime);
}


//----------------------------- Evade ------------------------------------
//
//  similar to pursuit except the agent Flees from the estimated future
//  position of the pursuer
//------------------------------------------------------------------------
noVec3 SteeringBehavior::Evade(const ActorController* pursuer)
{
  /* Not necessary to include the check for facing direction this time */

  //noVec3 diff = pursuer->GetTranslation() - m_pOwner->GetTranslation();
  noVec3 ToPursuer = pursuer->Pos() - m_pOwner->Pos();//noVec3(diff.x, diff.y);

  //uncomment the following two lines to have Evade only consider pursuers 
  //within a 'threat range'
  const double ThreatRange = 100.0;
  if (ToPursuer.LengthSqr() > ThreatRange * ThreatRange) return noVec3();
 
  //the lookahead time is propotional to the distance between the pursuer
  //and the pursuer; and is inversely proportional to the sum of the
  //agents' velocities
  double LookAheadTime = ToPursuer.Length() / 
                         (m_pOwner->MaxSpeed() + pursuer->Speed());
  
  //now flee away from predicted future position of the pursuer
  return Flee(pursuer->Pos() + pursuer->Velocity() * LookAheadTime);
}


//--------------------------- Wander -------------------------------------
//
//  This behavior makes the agent wander about randomly
//------------------------------------------------------------------------
noVec3 SteeringBehavior::Wander()
{ 
  //this behavior is dependent on the update rate, so this line must
  //be included when using time independent framerate.
  double JitterThisTimeSlice = m_dWanderJitter * m_pOwner->TimeElapsed();

  //first, add a small random vector to the target's position
  m_vWanderTarget += noVec3(RandomClamped() * JitterThisTimeSlice, 0.0f, 
                              RandomClamped() * JitterThisTimeSlice);

  //reproject this new vector back on to a unit circle
  m_vWanderTarget.Normalize();

  //increase the length of the vector to the same as the radius
  //of the wander circle
  m_vWanderTarget *= m_dWanderRadius;

  //move the target into a position WanderDist in front of the agent
  noVec3 target = m_vWanderTarget + noVec3(m_dWanderDistance, 0.f, 0.f);

  //project the target into world space
  noVec3 Target = PointToWorldSpace(target,
                                       m_pOwner->Heading(),
                                       m_pOwner->Side(), 
                                       m_pOwner->Pos());

  //and steer towards it
  return Target - m_pOwner->Pos(); 
}


//---------------------- ObstacleAvoidance -------------------------------
//
//  Given a vector of CObstacles, this method returns a steering force
//  that will prevent the agent colliding with the closest obstacle
//------------------------------------------------------------------------
noVec3 SteeringBehavior::ObstacleAvoidance(const std::vector<ActorController*>& obstacles)
{
  //the detection box length is proportional to the agent's velocity
  m_dDBoxLength = AICON.MinDetectionBoxLength + 
                  (m_pOwner->Speed()/m_pOwner->MaxSpeed()) *
                  AICON.MinDetectionBoxLength;

  //tag all obstacles within range of the box for processing
  m_pOwner->World()->TagObstaclesWithinViewRange(m_pOwner, m_dDBoxLength);

  //this will keep track of the closest intersecting obstacle (CIB)
  ActorController* ClosestIntersectingObstacle = NULL;
 
  //this will be used to track the distance to the CIB
  double DistToClosestIP = DBL_MAX;

  //this will record the transformed local coordinates of the CIB
  noVec3 LocalPosOfClosestObstacle;

  std::vector<ActorController*>::const_iterator curOb = obstacles.begin();

  while(curOb != obstacles.end())
  {
    //if the obstacle has been tagged within range proceed
    if ((*curOb)->IsTagged())
    {
      //calculate this obstacle's position in local space
      noVec3 LocalPos = PointToLocalSpace((*curOb)->Pos(),
                                             m_pOwner->Heading(),
                                             m_pOwner->Side(),
                                             m_pOwner->Pos());

      //if the local position has a negative x value then it must lay
      //behind the agent. (in which case it can be ignored)
      if (LocalPos.x >= 0)
      {
        //if the distance from the x axis to the object's position is less
        //than its radius + half the width of the detection box then there
        //is a potential intersection.
        double ExpandedRadius = (*curOb)->BRadius() + m_pOwner->BRadius();

        if (fabs(LocalPos.y) < ExpandedRadius)
        {
          //now to do a line/circle intersection test. The center of the 
          //circle is represented by (cX, cY). The intersection points are 
          //given by the formula x = cX +/-sqrt(r^2-cY^2) for y=0. 
          //We only need to look at the smallest positive value of x because
          //that will be the closest point of intersection.
          double cX = LocalPos.x;
          double cY = LocalPos.y;
          
          //we only need to calculate the sqrt part of the above equation once
          double SqrtPart = sqrt(ExpandedRadius*ExpandedRadius - cY*cY);

          double ip = cX - SqrtPart;

          if (ip <= 0.0)
          {
            ip = cX + SqrtPart;
          }

          //test to see if this is the closest so far. If it is keep a
          //record of the obstacle and its local coordinates
          if (ip < DistToClosestIP)
          {
            DistToClosestIP = ip;

            ClosestIntersectingObstacle = *curOb;

            LocalPosOfClosestObstacle = LocalPos;
          }         
        }
      }
    }

    ++curOb;
  }

  //if we have found an intersecting obstacle, calculate a steering 
  //force away from it
  noVec3 SteeringForce(vec3_zero);

  if (ClosestIntersectingObstacle)
  {
    //the closer the agent is to an object, the stronger the 
    //steering force should be
    double multiplier = 1.0 + (m_dDBoxLength - LocalPosOfClosestObstacle.x) /
                        m_dDBoxLength;

    //calculate the lateral force
    SteeringForce.y = (ClosestIntersectingObstacle->BRadius()-
                       LocalPosOfClosestObstacle.y)  * multiplier;   

    //apply a braking force proportional to the obstacles distance from
    //the vehicle. 
    const double BrakingWeight = 0.2;

    SteeringForce.x = (ClosestIntersectingObstacle->BRadius() - 
                       LocalPosOfClosestObstacle.x) * 
                       BrakingWeight;
  }

  //finally, convert the steering vector from local to world space
  return VectorToWorldSpace(SteeringForce,
                            m_pOwner->Heading(),
                            m_pOwner->Side());
}


//--------------------------- WallAvoidance --------------------------------
//
//  This returns a steering force that will keep the agent away from any
//  walls it may encounter
//------------------------------------------------------------------------
noVec3 SteeringBehavior::WallAvoidance(const std::vector<Wall2D>& walls)
{
  //the feelers are contained in a std::vector, m_Feelers
  CreateFeelers();
  
  double DistToThisIP    = 0.0;
  double DistToClosestIP = DBL_MAX;

  //this will hold an index into the vector of walls
  int ClosestWall = -1;

  noVec3 SteeringForce,
            point,         //used for storing temporary info
            ClosestPoint;  //holds the closest intersection point

  //examine each feeler in turn
  for (unsigned int flr=0; flr<m_Feelers.size(); ++flr)
  {
    //run through each wall checking for any intersection points
    for (unsigned int w=0; w<walls.size(); ++w)
    {
      if (LineIntersection2D(m_pOwner->Pos(),
                             m_Feelers[flr],
                             walls[w].From(),
                             walls[w].To(),
                             DistToThisIP,
                             point))
      {
        //is this the closest found so far? If so keep a record
        if (DistToThisIP < DistToClosestIP)
        {
          DistToClosestIP = DistToThisIP;

          ClosestWall = w;

          ClosestPoint = point;
        }
      }
    }//next wall

  
    //if an intersection point has been detected, calculate a force  
    //that will direct the agent away
    if (ClosestWall >=0)
    {
      //calculate by what distance the projected position of the agent
      //will overshoot the wall
      noVec3 OverShoot = m_Feelers[flr] - ClosestPoint;

      //create a force in the direction of the wall normal, with a 
      //magnitude of the overshoot
      SteeringForce = walls[ClosestWall].Normal() * OverShoot.Length();
    }
		
  }//next feeler

  return SteeringForce;
}

//------------------------------- CreateFeelers --------------------------
//
//  Creates the antenna utilized by WallAvoidance
//------------------------------------------------------------------------
void SteeringBehavior::CreateFeelers()
{
  //feeler pointing straight in front
  m_Feelers[0] = m_pOwner->Pos() + m_dWallDetectionFeelerLength * m_pOwner->Heading();

  //feeler to left
  noVec3 temp = m_pOwner->Heading();
  Vec2DRotateAroundOrigin(temp, noMath::HALF_PI * 3.5f);
  m_Feelers[1] = m_pOwner->Pos() + m_dWallDetectionFeelerLength/2.0f * temp;

  //feeler to right
  temp = m_pOwner->Heading();
  Vec2DRotateAroundOrigin(temp, noMath::HALF_PI * 0.5f);
  m_Feelers[2] = m_pOwner->Pos() + m_dWallDetectionFeelerLength/2.0f * temp;
}


//---------------------------- Separation --------------------------------
//
// this calculates a force repelling from the other neighbors
//------------------------------------------------------------------------
noVec3 SteeringBehavior::Separation(const vector<ActorController*> &neighbors)
{  
  noVec3 SteeringForce(vec3_zero);

  for (unsigned int a=0; a<neighbors.size(); ++a)
  {
    //make sure this agent isn't included in the calculations and that
    //the agent being examined is close enough. ***also make sure it doesn't
    //include the evade target ***
    if((neighbors[a] != m_pOwner) && neighbors[a]->IsTagged() &&
      (neighbors[a] != m_pTargetAgent1))
    {
      noVec3 ToAgent = m_pOwner->Pos() - neighbors[a]->Pos();

      //scale the force inversely proportional to the agents distance  
      //from its neighbor.
	  ToAgent.Normalize();
      SteeringForce += ToAgent /ToAgent.Length();
    }
  }

  return SteeringForce;
}


//---------------------------- Alignment ---------------------------------
//
//  returns a force that attempts to align this agents heading with that
//  of its neighbors
//------------------------------------------------------------------------
noVec3 SteeringBehavior::Alignment(const vector<ActorController*>& neighbors)
{
  //used to record the average heading of the neighbors
  noVec3 AverageHeading;
  AverageHeading.Zero();

  //used to count the number of vehicles in the neighborhood
  int    NeighborCount = 0;

  //iterate through all the tagged vehicles and sum their heading vectors  
  for (unsigned int a=0; a<neighbors.size(); ++a)
  {
    //make sure *this* agent isn't included in the calculations and that
    //the agent being examined  is close enough ***also make sure it doesn't
    //include any evade target ***
    if((neighbors[a] != m_pOwner) && neighbors[a]->IsTagged() &&
      (neighbors[a] != m_pTargetAgent1))
    {
      AverageHeading += neighbors[a]->Heading();

      ++NeighborCount;
    }
  }

  //if the neighborhood contained one or more vehicles, average their
  //heading vectors.
  if (NeighborCount > 0)
  {
    AverageHeading /= (double)NeighborCount;

    AverageHeading -= m_pOwner->Heading();
  }
  
  return AverageHeading;
}

//-------------------------------- Cohesion ------------------------------
//
//  returns a steering force that attempts to move the agent towards the
//  center of mass of the agents in its immediate area
//------------------------------------------------------------------------
noVec3 SteeringBehavior::Cohesion(const vector<ActorController*> &neighbors)
{
  //first find the center of mass of all the agents
  noVec3 CenterOfMass, SteeringForce;
  CenterOfMass.Zero();
  SteeringForce.Zero();

  int NeighborCount = 0;

  //iterate through the neighbors and sum up all the position vectors
  for (unsigned int a=0; a<neighbors.size(); ++a)
  {
    //make sure *this* agent isn't included in the calculations and that
    //the agent being examined is close enough ***also make sure it doesn't
    //include the evade target ***
    if((neighbors[a] != m_pOwner) && neighbors[a]->IsTagged() &&
      (neighbors[a] != m_pTargetAgent1))
    {
      CenterOfMass += neighbors[a]->Pos();

      ++NeighborCount;
    }
  }

  if (NeighborCount > 0)
  {
    //the center of mass is the average of the sum of positions
    CenterOfMass /= (double)NeighborCount;

    //now seek towards that position
    SteeringForce = Seek(CenterOfMass);
  }

  //the magnitude of cohesion is usually much larger than separation or
  //allignment so it usually helps to normalize it.
  SteeringForce.Normalize();
  return (SteeringForce);
}


/* NOTE: the next three behaviors are the same as the above three, except
          that they use a cell-space partition to find the neighbors
*/


//---------------------------- Separation --------------------------------
//
// this calculates a force repelling from the other neighbors
//
//  USES SPACIAL PARTITIONING
//------------------------------------------------------------------------
noVec3 SteeringBehavior::SeparationPlus(const vector<ActorController*> &neighbors)
{  
  noVec3 SteeringForce;

  //iterate through the neighbors and sum up all the position vectors
  for (ActorController* pV = m_pOwner->World()->CellSpace()->begin();
                         !m_pOwner->World()->CellSpace()->end();     
                       pV = m_pOwner->World()->CellSpace()->next())
  {    
    //make sure this agent isn't included in the calculations and that
    //the agent being examined is close enough
    if(pV != m_pOwner)
    {
      noVec3 ToAgent = m_pOwner->Pos() - pV->Pos();

      //scale the force inversely proportional to the agents distance  
      //from its neighbor.
	  ToAgent.Normalize();
      SteeringForce += (ToAgent)/ToAgent.Length();
    }

  }

  return SteeringForce;
}
//---------------------------- Alignment ---------------------------------
//
//  returns a force that attempts to align this agents heading with that
//  of its neighbors
//
//  USES SPACIAL PARTITIONING
//------------------------------------------------------------------------
noVec3 SteeringBehavior::AlignmentPlus(const vector<ActorController*> &neighbors)
{
  //This will record the average heading of the neighbors
  noVec3 AverageHeading;

  //This count the number of vehicles in the neighborhood
  double    NeighborCount = 0.0;

  //iterate through the neighbors and sum up all the position vectors
  for (ActorController* pV = m_pOwner->World()->CellSpace()->begin();
                         !m_pOwner->World()->CellSpace()->end();     
                     pV = m_pOwner->World()->CellSpace()->next())
  {
    //make sure *this* agent isn't included in the calculations and that
    //the agent being examined  is close enough
    if(pV != m_pOwner)
    {
      AverageHeading += pV->Heading();

      ++NeighborCount;
    }

  }

  //if the neighborhood contained one or more vehicles, average their
  //heading vectors.
  if (NeighborCount > 0.0)
  {
    AverageHeading /= NeighborCount;

    AverageHeading -= m_pOwner->Heading();
  }
  
  return AverageHeading;
}


//-------------------------------- Cohesion ------------------------------
//
//  returns a steering force that attempts to move the agent towards the
//  center of mass of the agents in its immediate area
//
//  USES SPACIAL PARTITIONING
//------------------------------------------------------------------------
noVec3 SteeringBehavior::CohesionPlus(const vector<ActorController*> &neighbors)
{
  //first find the center of mass of all the agents
  noVec3 CenterOfMass, SteeringForce;

  int NeighborCount = 0;

  //iterate through the neighbors and sum up all the position vectors
  for (ActorController* pV = m_pOwner->World()->CellSpace()->begin();
                         !m_pOwner->World()->CellSpace()->end();     
                       pV = m_pOwner->World()->CellSpace()->next())
  {
    //make sure *this* agent isn't included in the calculations and that
    //the agent being examined is close enough
    if(pV != m_pOwner)
    {
      CenterOfMass += pV->Pos();

      ++NeighborCount;
    }
  }

  if (NeighborCount > 0)
  {
    //the center of mass is the average of the sum of positions
    CenterOfMass /= (double)NeighborCount;

    //now seek towards that position
    SteeringForce = Seek(CenterOfMass);
  }

  //the magnitude of cohesion is usually much larger than separation or
  //allignment so it usually helps to normalize it.
  SteeringForce.Normalize();
  return (SteeringForce);
}


//--------------------------- Interpose ----------------------------------
//
//  Given two agents, this method returns a force that attempts to 
//  position the vehicle between them
//------------------------------------------------------------------------
noVec3 SteeringBehavior::Interpose(const ActorController* AgentA,
                                     const ActorController* AgentB)
{
  //first we need to figure out where the two agents are going to be at 
  //time T in the future. This is approximated by determining the time
  //taken to reach the mid way point at the current time at at max speed.
  noVec3 MidPoint = (AgentA->Pos() + AgentB->Pos()) / 2.0;

  double TimeToReachMidPoint = (m_pOwner->Pos() - MidPoint).Length() /
                               m_pOwner->MaxSpeed();

  //now we have T, we assume that agent A and agent B will continue on a
  //straight trajectory and extrapolate to get their future positions
  noVec3 APos = AgentA->Pos() + AgentA->Velocity() * TimeToReachMidPoint;
  noVec3 BPos = AgentB->Pos() + AgentB->Velocity() * TimeToReachMidPoint;

  //calculate the mid point of these predicted positions
  MidPoint = (APos + BPos) / 2.0;

  //then steer to Arrive at it
  return Arrive(MidPoint, fast);
}

//--------------------------- Hide ---------------------------------------
//
//------------------------------------------------------------------------
noVec3 SteeringBehavior::Hide(const ActorController*           hunter,
                                 const vector<ActorController*>& obstacles)
{
  double    DistToClosest = DBL_MAX;
  noVec3 BestHidingSpot;

  std::vector<ActorController*>::const_iterator curOb = obstacles.begin();
  std::vector<ActorController*>::const_iterator closest;

  while(curOb != obstacles.end())
  {
    //calculate the position of the hiding spot for this obstacle
    noVec3 HidingSpot = GetHidingPosition((*curOb)->Pos(),
                                             (*curOb)->BRadius(),
                                              hunter->Pos());
            
    //work in distance-squared space to find the closest hiding
    //spot to the agent
    double dist = (HidingSpot - m_pOwner->Pos()).LengthSqr();

    if (dist < DistToClosest)
    {
      DistToClosest = dist;

      BestHidingSpot = HidingSpot;

      closest = curOb;
    }  
            
    ++curOb;

  }//end while
  
  //if no suitable obstacles found then Evade the hunter
  if (DistToClosest == noMath::INFINITY)
  {
    return Evade(hunter);
  }
      
  //else use Arrive on the hiding spot
  return Arrive(BestHidingSpot, fast);
}

//------------------------- GetHidingPosition ----------------------------
//
//  Given the position of a hunter, and the position and radius of
//  an obstacle, this method calculates a position DistanceFromBoundary 
//  away from its bounding radius and directly opposite the hunter
//------------------------------------------------------------------------
noVec3 SteeringBehavior::GetHidingPosition(const noVec3& posOb,
                                              const double     radiusOb,
                                              const noVec3& posHunter)
{
  //calculate how far away the agent is to be from the chosen obstacle's
  //bounding radius
  const double DistanceFromBoundary = 30.0;
  double       DistAway    = radiusOb + DistanceFromBoundary;

  //calculate the heading toward the object from the hunter
  noVec3 temp = posOb - posHunter;
  temp.Normalize();
  noVec3 ToOb = temp;
  
  //scale it to size and add to the obstacles position to get
  //the hiding spot.
  return (ToOb * DistAway) + posOb;
}


//------------------------------- FollowPath -----------------------------
//
//  Given a series of noVec3s, this method produces a force that will
//  move the agent along the waypoints in order. The agent uses the
// 'Seek' behavior to move to the next waypoint - unless it is the last
//  waypoint, in which case it 'Arrives'
//------------------------------------------------------------------------
noVec3 SteeringBehavior::FollowPath()
{ 
  //move to next target if close enough to current target (working in
  //distance squared space)
  if( ((m_pPath->CurrentWaypoint() - m_pOwner->Pos()).LengthSqr()) <
     m_dWaypointSeekDistSq)
  {
    m_pPath->SetNextWaypoint();
  }

  if (!m_pPath->Finished())
  {
    return Seek(m_pPath->CurrentWaypoint());
  }

  else
  {
    return Arrive(m_pPath->CurrentWaypoint(), normal);
  }
}

//------------------------- Offset Pursuit -------------------------------
//
//  Produces a steering force that keeps a vehicle at a specified offset
//  from a leader vehicle
//------------------------------------------------------------------------
noVec3 SteeringBehavior::OffsetPursuit(const ActorController*  leader,
                                          const noVec3 offset)
{
  //calculate the offset's position in world space
  noVec3 WorldOffsetPos = PointToWorldSpace(offset,
                                                  leader->Heading(),
                                                  leader->Side(),
                                                  leader->Pos());

  noVec3 ToOffset = WorldOffsetPos - m_pOwner->Pos();

  //the lookahead time is propotional to the distance between the leader
  //and the pursuer; and is inversely proportional to the sum of both
  //agent's velocities
  double LookAheadTime = ToOffset.Length() / 
                        (m_pOwner->MaxSpeed() + leader->Speed());
  
  //now Arrive at the predicted future position of the offset
  return Arrive(WorldOffsetPos + leader->Velocity() * LookAheadTime, fast);
}



//for receiving keyboard input from user
#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
//----------------------------- RenderAids -------------------------------
//
//------------------------------------------------------------------------
void SteeringBehavior::RenderAids( )
{ 
  
#if 0
  gdi->TransparentText();
  gdi->TextColor(Cgdi::grey);

  int NextSlot = 0; int SlotSize = 20;

  if (KEYDOWN(VK_INSERT)){m_pOwner->SetMaxForce(m_pOwner->MaxForce() + 1000.0f*m_pOwner->TimeElapsed());} 
  if (KEYDOWN(VK_DELETE)){if (m_pOwner->MaxForce() > 0.2f) m_pOwner->SetMaxForce(m_pOwner->MaxForce() - 1000.0f*m_pOwner->TimeElapsed());}
  if (KEYDOWN(VK_HOME)){m_pOwner->SetMaxSpeed(m_pOwner->MaxSpeed() + 50.0f*m_pOwner->TimeElapsed());}
  if (KEYDOWN(VK_END)){if (m_pOwner->MaxSpeed() > 0.2f) m_pOwner->SetMaxSpeed(m_pOwner->MaxSpeed() - 50.0f*m_pOwner->TimeElapsed());}

  if (m_pOwner->MaxForce() < 0) m_pOwner->SetMaxForce(0.0f);
  if (m_pOwner->MaxSpeed() < 0) m_pOwner->SetMaxSpeed(0.0f);
  
  if (m_pOwner->ID() == 0){ gdi->TextAtPos(5,NextSlot,"MaxForce(Ins/Del):"); gdi->TextAtPos(160,NextSlot,ttos(m_pOwner->MaxForce()/Prm.SteeringForceTweaker)); NextSlot+=SlotSize;}
  if (m_pOwner->ID() == 0){ gdi->TextAtPos(5,NextSlot,"MaxSpeed(Home/End):"); gdi->TextAtPos(160,NextSlot,ttos(m_pOwner->MaxSpeed()));NextSlot+=SlotSize;}

  //render the steering force
  if (m_pOwner->World()->RenderSteeringForce())
  {  
    gdi->RedPen();
    noVec3 F = (m_vSteeringForce / Prm.SteeringForceTweaker) * Prm.VehicleScale ;
    gdi->Line(m_pOwner->Pos(), m_pOwner->Pos() + F);
  }

  //render wander stuff if relevant
  if (On(wander) && m_pOwner->World()->RenderWanderCircle())
  {    
    if (KEYDOWN('F')){m_dWanderJitter+=1.0f*m_pOwner->TimeElapsed(); Clamp(m_dWanderJitter, 0.0f, 100.0f);}
    if (KEYDOWN('V')){m_dWanderJitter-=1.0f*m_pOwner->TimeElapsed(); Clamp(m_dWanderJitter, 0.0f, 100.0f );}
    if (KEYDOWN('G')){m_dWanderDistance+=2.0f*m_pOwner->TimeElapsed(); Clamp(m_dWanderDistance, 0.0f, 50.0f);}
    if (KEYDOWN('B')){m_dWanderDistance-=2.0f*m_pOwner->TimeElapsed(); Clamp(m_dWanderDistance, 0.0f, 50.0f);}
    if (KEYDOWN('H')){m_dWanderRadius+=2.0f*m_pOwner->TimeElapsed(); Clamp(m_dWanderRadius, 0.0f, 100.0f);}
    if (KEYDOWN('N')){m_dWanderRadius-=2.0f*m_pOwner->TimeElapsed(); Clamp(m_dWanderRadius, 0.0f, 100.0f);}

 
    if (m_pOwner->ID() == 0){ gdi->TextAtPos(5,NextSlot, "Jitter(F/V): "); gdi->TextAtPos(160, NextSlot, ttos(m_dWanderJitter));NextSlot+=SlotSize;}
    if (m_pOwner->ID() == 0) {gdi->TextAtPos(5,NextSlot,"Distance(G/B): "); gdi->TextAtPos(160, NextSlot, ttos(m_dWanderDistance));NextSlot+=SlotSize;}
    if (m_pOwner->ID() == 0) {gdi->TextAtPos(5,NextSlot,"Radius(H/N): ");gdi->TextAtPos(160, NextSlot,  ttos(m_dWanderRadius));NextSlot+=SlotSize;}

    
    //calculate the center of the wander circle
    noVec3 m_vTCC = PointToWorldSpace(noVec3(m_dWanderDistance*m_pOwner->BRadius(), 0),
                                         m_pOwner->Heading(),
                                         m_pOwner->Side(),
                                         m_pOwner->Pos());
    //draw the wander circle
    gdi->GreenPen();
    gdi->HollowBrush();
    gdi->Circle(m_vTCC, m_dWanderRadius*m_pOwner->BRadius()); 

    //draw the wander target
    gdi->RedPen();
    gdi->Circle(PointToWorldSpace((m_vWanderTarget + noVec3(m_dWanderDistance,0))*m_pOwner->BRadius(),
                                  m_pOwner->Heading(),
                                  m_pOwner->Side(),
                                  m_pOwner->Pos()), 3);                                  
  }

  //render the detection box if relevant
  if (m_pOwner->World()->RenderDetectionBox())
  {
    gdi->GreyPen();

    //a vertex buffer rqd for drawing the detection box
    static std::vector<noVec3> box(4);

    double length = Prm.MinDetectionBoxLength + 
                  (m_pOwner->Speed()/m_pOwner->MaxSpeed()) *
                  Prm.MinDetectionBoxLength;

    //verts for the detection box buffer
    box[0] = noVec3(0,m_pOwner->BRadius());
    box[1] = noVec3(length, m_pOwner->BRadius());
    box[2] = noVec3(length, -m_pOwner->BRadius());
    box[3] = noVec3(0, -m_pOwner->BRadius());
 
  
    if (!m_pOwner->isSmoothingOn())
    {
      box = WorldTransform(box,m_pOwner->Pos(),m_pOwner->Heading(),m_pOwner->Side());
      gdi->ClosedShape(box);
    }
    else
    {
      box = WorldTransform(box,m_pOwner->Pos(),m_pOwner->SmoothedHeading(),m_pOwner->SmoothedHeading().Perp());
      gdi->ClosedShape(box);
    } 


    //////////////////////////////////////////////////////////////////////////
    //the detection box length is proportional to the agent's velocity
  m_dDBoxLength = Prm.MinDetectionBoxLength + 
                  (m_pOwner->Speed()/m_pOwner->MaxSpeed()) *
                  Prm.MinDetectionBoxLength;

  //tag all obstacles within range of the box for processing
  m_pOwner->World()->TagObstaclesWithinViewRange(m_pOwner, m_dDBoxLength);

  //this will keep track of the closest intersecting obstacle (CIB)
  MovingActor* ClosestIntersectingObstacle = NULL;
 
  //this will be used to track the distance to the CIB
  double DistToClosestIP = MaxDouble;

  //this will record the transformed local coordinates of the CIB
  noVec3 LocalPosOfClosestObstacle;

  std::vector<MovingActor*>::const_iterator curOb = m_pOwner->World()->Obstacles().begin();

  while(curOb != m_pOwner->World()->Obstacles().end())
  {
    //if the obstacle has been tagged within range proceed
    if ((*curOb)->IsTagged())
    {
      //calculate this obstacle's position in local space
      noVec3 LocalPos = PointToLocalSpace((*curOb)->Pos(),
                                             m_pOwner->Heading(),
                                             m_pOwner->Side(),
                                             m_pOwner->Pos());

      //if the local position has a negative x value then it must lay
      //behind the agent. (in which case it can be ignored)
      if (LocalPos.x >= 0)
      {
        //if the distance from the x axis to the object's position is less
        //than its radius + half the width of the detection box then there
        //is a potential intersection.
        if (fabs(LocalPos.y) < ((*curOb)->BRadius() + m_pOwner->BRadius()))
        {
          gdi->ThickRedPen();
          gdi->ClosedShape(box);        
        }
      }
    }

    ++curOb;
  }


/////////////////////////////////////////////////////
  }

  //render the wall avoidnace feelers
  if (On(wall_avoidance) && m_pOwner->World()->RenderFeelers())
  {
    gdi->OrangePen();

    for (unsigned int flr=0; flr<m_Feelers.size(); ++flr)
    {

      gdi->Line(m_pOwner->Pos(), m_Feelers[flr]);
    }            
  }  

  //render path info
  if (On(follow_path) && m_pOwner->World()->RenderPath())
  {
    m_pPath->Render();
  }

  
  if (On(separation))
  {
    if (m_pOwner->ID() == 0){ gdi->TextAtPos(5, NextSlot, "Separation(S/X):");gdi->TextAtPos(160,NextSlot, ttos(m_dWeightSeparation/Prm.SteeringForceTweaker));NextSlot+=SlotSize;}

    if (KEYDOWN('S')){m_dWeightSeparation += 200*m_pOwner->TimeElapsed(); Clamp(m_dWeightSeparation, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
    if (KEYDOWN('X')){m_dWeightSeparation -= 200*m_pOwner->TimeElapsed();Clamp(m_dWeightSeparation, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
  }

  if (On(allignment))
  {
    if (m_pOwner->ID() == 0) {gdi->TextAtPos(5, NextSlot, "Alignment(A/Z):"); gdi->TextAtPos(160, NextSlot, ttos(m_dWeightAlignment/Prm.SteeringForceTweaker));NextSlot+=SlotSize;}

    if (KEYDOWN('A')){m_dWeightAlignment += 200*m_pOwner->TimeElapsed();Clamp(m_dWeightAlignment, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
    if (KEYDOWN('Z')){m_dWeightAlignment -= 200*m_pOwner->TimeElapsed();Clamp(m_dWeightAlignment, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
  }

  if (On(cohesion))
  {
    if (m_pOwner->ID() == 0) {gdi->TextAtPos(5, NextSlot, "Cohesion(D/C):"); gdi->TextAtPos(160, NextSlot, ttos(m_dWeightCohesion/Prm.SteeringForceTweaker));NextSlot+=SlotSize;}
    if (KEYDOWN('D')){m_dWeightCohesion += 200*m_pOwner->TimeElapsed();Clamp(m_dWeightCohesion, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
    if (KEYDOWN('C')){m_dWeightCohesion -= 200*m_pOwner->TimeElapsed();Clamp(m_dWeightCohesion, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
  }

  if (On(follow_path))
  { 
    double sd = sqrt(m_dWaypointSeekDistSq);
    if (m_pOwner->ID() == 0){ gdi->TextAtPos(5, NextSlot, "SeekDistance(D/C):");gdi->TextAtPos(160, NextSlot,ttos(sd));NextSlot+=SlotSize;}
    
    if (KEYDOWN('D')){m_dWaypointSeekDistSq += 1.0;}
    if (KEYDOWN('C')){m_dWaypointSeekDistSq -= 1.0; Clamp(m_dWaypointSeekDistSq, 0.0f, 400.0f);}
  }  
#endif
}





