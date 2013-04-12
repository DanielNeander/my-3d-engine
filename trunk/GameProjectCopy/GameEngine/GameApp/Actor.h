#pragma once 

#include "GameObject.h"
#include "AI/Matrix2D.h"
#include "AI/AIUtil.h"
#include "AI/BaseStates.h"
#include "AI/StateMachine.h"
#ifndef NAVIGATIONCELL_H
#include "AI/navigationcell.h"
#endif
#ifndef NAVIGATIONPATH_H
#include "AI/navigationpath.h"
#endif
#ifndef NAVIGATIONMESH_H
#include "AI/navigationmesh.h"
#endif

class GameWorld;

class Actor : public GameObject
{
public:
	noDeclareRTTI;

	Actor(const std::string filename);

	void update (float elapsedTime);
		
protected:
	
};

MSmartPointer(Actor);



///////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class Smoother
{
private:

	//this holds the history
	std::vector<T>  m_History;

	int           m_iNextUpdateSlot;

	//an example of the 'zero' value of the type to be smoothed. This
	//would be something like Vector2D(0,0)
	T             m_ZeroValue;

public:

	//to instantiate a Smoother pass it the number of samples you want
	//to use in the smoothing, and an exampe of a 'zero' type
	Smoother(int SampleSize, T ZeroValue):m_History(SampleSize, ZeroValue),
		m_ZeroValue(ZeroValue),
		m_iNextUpdateSlot(0)
	{}

	//each time you want to get a new average, feed it the most recent value
	//and this method will return an average over the last SampleSize updates
	T Update(const T& MostRecentValue)
	{  
		//overwrite the oldest value with the newest
		m_History[m_iNextUpdateSlot++] = MostRecentValue;

		//make sure m_iNextUpdateSlot wraps around. 
		if (m_iNextUpdateSlot == m_History.size()) m_iNextUpdateSlot = 0;

		//now to calculate the average of the history list
		T sum = m_ZeroValue;

		std::vector<T>::iterator it = m_History.begin();

		for (it; it != m_History.end(); ++it)
		{
			sum += *it;
		}

		return sum / (double)m_History.size();
	}
};

class NavigationMeshAdapter 
{
public:
	NavigationMeshAdapter(GameObject* owner);

	virtual void					Create
		(
		NavigationMesh	*Parent,
		const noVec3	&Position,
		NavigationCell	*CurrentCell
		);

	NavigationCell					*CurrentCell() const;
	bool							PathIsActive() const;
	
	void					Update(float elapsedTime = 1.0f);	
	void					GotoLocation(const noVec3 &Position, NavigationCell *Cell);
	void					GotoRandomLocation();

	void					AddMovement(const noVec3 &Movement);
	void					SetMovement(const noVec3 &Movement);
	void					SetMovementX(float X);
	void					SetMovementY(float Y);
	void					SetMovementZ(float Z);
	void					SetMaxSpeed(float speed);

	NavigationMesh				*m_Parent;	/* the mesh we are sitting on */
	NavigationCell				*m_CurrentCell; /* our current cell on the mesh */
	noVec3						m_Position;		/* our 3D control point position */
	noVec3						m_Movement;		/* the current movement vector */
	float						m_MaxSpeed;		/* our maximum traveling distance per frame */

	bool						m_PathActive;	/* true when we are using a path to navigate */
	NavigationPath				m_Path;			/* our path object */
	NavigationPath::WayPointID	m_NextWaypoint; /* ID of the next waypoint we will move to */

	GameObject*					m_owner;

	/* HELPER FUNCTIONS UNIMPLEMENTED FUNCTIONS */
	NavigationMeshAdapter(const NavigationMeshAdapter &Src);
	NavigationMeshAdapter &operator				=(const NavigationMeshAdapter &Src);

};


class ActorController : public StateMachine
{
public:
	ActorController(GameObject* pkGameObject, GameWorld* pWorld, noVec3 position,
		double rotation, noVec3 velocity, double mass, double max_force,
		double max_speed, double max_turn_rate);	
	~ActorController();

protected:
	noVec3		m_vVelocity; 
	noVec3		m_vHeading;
	noVec3	    m_vSide; 
	double      m_dMass;
	double      m_dMaxSpeed;
	double      m_dMaxForce;
	double      m_dMaxTurnRate;

	// object to smooth out the framerate for animations and AI
	Smoother<double>*	m_pFrameSmoother;
	
private:	
	//this is a generic flag. 
	bool        m_bTag;

	noVec3		m_vPos;
	//the length of this object's bounding radius
	float		m_dTimeElapsed;
	float	    m_dBoundingRadius;
	GameWorld*	m_World;
	
	static const int MAX_POLYS = 2048; //512;
	static const int MAX_SMOOTH = 12096;

	dtPolyRef m_startRef;
	dtPolyRef m_endRef;
	dtPolyRef m_polys[MAX_POLYS];
	dtPolyRef m_parent[MAX_POLYS];
	int m_npolys;
	float m_straightPath[MAX_POLYS*3];
	unsigned char m_straightPathFlags[MAX_POLYS];
	dtPolyRef m_straightPathPolys[MAX_POLYS];
	int m_nstraightPath;
	float m_polyPickExt[3];
	float m_smoothPath[MAX_SMOOTH*3];
	int m_nsmoothPath;

	float m_spos[3];
	float m_epos[3];
	float m_hitPos[3];
	float m_hitNormal[3];
	bool m_hitResult;
	float m_distanceToWall;
	bool m_sposSet;
	bool m_eposSet;

	int m_pathIterNum;
	const dtPolyRef* m_pathIterPolys; 
	int m_pathIterPolyCount;
	float m_prevIterPos[3], m_iterPos[3], m_steerPos[3], m_targetPos[3];

	static const int MAX_STEER_POINTS = 200;
	float m_steerPoints[MAX_STEER_POINTS*3];
	int m_steerPointCount;
	dtQueryFilter m_filter;
	
	
	bool mHasPath;
	bool mFindingPath;	
	bool mIsWalking;


	//the steering behavior class
	class SteeringBehavior*		m_pSteering;
	Smoother<noVec3>*		m_pHeadingSmoother;
	noVec3				m_vSmoothedHeading;
	bool					m_bSmoothingOn;
	class AIPath*                         m_pPath;

	NavigationMeshAdapter*		pathfinding_;

	float						mIdleTimerToChange;
	float						mIdleTimerCurrent;

	objectID m_curTarget;

	SimpleStateManager*	m_StateMgr_;

	float		mAttackRange;

	struct NavDebugDraw*	dd_;	
	NavDebugDraw*	dd2_;
public:

	NavigationMeshAdapter* PathFinding() const { return pathfinding_; }
		
	void update (float elapsedTime);

	noVec3     Pos()const{return m_vPos;}
	void         SetPos(noVec3 new_pos){m_vPos = new_pos;}
	bool         IsTagged()const{return m_bTag;}
	void         Tag(){m_bTag = true;}
	void         UnTag(){m_bTag = false;}

	float        BRadius()const{return m_dBoundingRadius;}
	void         SetBRadius(float r){m_dBoundingRadius = r;}

	SteeringBehavior*const  Steering()const{return m_pSteering;}
	GameWorld* const		World() const { return m_World;}

	float       TimeElapsed()const{return m_dTimeElapsed;}


	void GetClosestPlayer(GameObject*& gameobj, float& minDist);

	SimpleStateManager* GetStateMgr() const { return m_StateMgr_; }

	//accessors

	// Public Path Finding Stuff
	std::vector<noVec3> mWalkList;
	noVec3 mPathStart;
	noVec3 mPathEnd;

	void setPathStart(float x, float y, float z) { mPathStart.x = x;  mPathStart.y = y;  mPathStart.z = z; m_sposSet = true; }
	void setPathStart(noVec3 pos) {  mPathStart = pos; m_sposSet = true; }
	void setPathStart(float* pos) {  mPathStart.x = pos[0];  mPathStart.y = pos[1];  mPathStart.z = pos[2]; m_sposSet = true; }
	noVec3 getPathStart(void) { return mPathStart; }

	void setPathEnd(float x, float y, float z) { mPathEnd.x = x;  mPathEnd.y = y;  mPathEnd.z = z; m_eposSet = true; }
	void setPathEnd(noVec3 pos) {  mPathEnd = pos; m_eposSet = true; }
	void setPathEnd(float* pos) { mPathEnd.x = pos[0];  mPathEnd.y = pos[1];  mPathEnd.z = pos[2]; m_eposSet = true; }
	noVec3 getPathEnd(void) { return mPathEnd; }

	noVec3    SmoothedHeading()const{return m_vSmoothedHeading;}

	bool        isSmoothingOn()const{return m_bSmoothingOn;}
	void        SmoothingOn(){m_bSmoothingOn = true;}
	void        SmoothingOff(){m_bSmoothingOn = false;}
	void        ToggleSmoothing(){m_bSmoothingOn = !m_bSmoothingOn;}

	noVec3  Velocity()const{return m_vVelocity;}
	void      SetVelocity(const noVec3& NewVel){m_vVelocity = NewVel;}

	double     Mass()const{return m_dMass;}

	noVec3  Side()const{return m_vSide;}

	double     MaxSpeed()const{return m_dMaxSpeed;}                       
	void      SetMaxSpeed(double new_speed){m_dMaxSpeed = new_speed;}

	double     MaxForce()const{return m_dMaxForce;}
	void      SetMaxForce(double mf){m_dMaxForce = mf;}

	bool      IsSpeedMaxedOut()const{return m_dMaxSpeed*m_dMaxSpeed >= m_vVelocity.LengthSqr();}
	double     Speed()const{return m_vVelocity.Length();}
	double     SpeedSq()const{return m_vVelocity.LengthSqr();}

	noVec3  Heading()const{return m_vHeading;}
	void      SetHeading(noVec3 new_heading);
	bool      RotateHeadingToFacePosition(noVec3 target);

	double     MaxTurnRate()const{return m_dMaxTurnRate;}
	void      SetMaxTurnRate(double val){m_dMaxTurnRate = val;}
	void findStartEndPositions();
	noVec3 findValidSpawnPosition(float _rayHeight);
	void recalc(void);
	bool States( State_Machine_Event event, MSG_Object * msg, int state, int substate );

	void sendThinkMessage(void);
	void sendModeChangeMessage(void);
	void sendFindNewPathMessage(void);
	void DrawDebug();
	void GetClosestTarget( const noVec3& pos, GameObject*& output);
	
	inline void ExitFindPath()
	{
		mHasPath = false;
		mFindingPath = true;
	}
	
	void SetStateFindPath();
};

inline bool ActorController::RotateHeadingToFacePosition(noVec3 target)
{
	noVec3 toTarget = (target - m_owner->GetTranslation());
	toTarget.Normalize();

	//first determine the angle between the heading vector and the target
	double angle = acos(m_vHeading * toTarget);

	//return true if the player is facing the target
	if (angle < 0.00001) return true;

	//clamp the amount to turn to the max turn rate
	if (angle > m_dMaxTurnRate) angle = m_dMaxTurnRate;

	//The next few lines use a rotation matrix to rotate the player's heading
	//vector accordingly
	Matrix2D RotationMatrix;

	//notice how the direction of rotation has to be determined when creating
	//the rotation matrix
	RotationMatrix.Rotate(angle * m_vHeading.Sign(toTarget));	
	

	RotationMatrix.TransformVector2Ds(noVec2(m_vHeading.x, m_vHeading.z));
	RotationMatrix.TransformVector2Ds(noVec2(m_vVelocity.x, m_vVelocity.z));

	//finally recreate m_vSide
	m_vSide = noVec3(-m_vHeading.z, m_vHeading.y, m_vHeading.x); // (-y, x)

	return false;
}

inline void ActorController::SetHeading(noVec3 new_heading)
{
	assert( (new_heading.LengthSqr() - 1.0) < 0.00001);

	m_vHeading = new_heading;

	//the side vector must always be perpendicular to the heading
	m_vSide = noVec3(-m_vHeading.z, m_vHeading.y, m_vHeading.x);
}