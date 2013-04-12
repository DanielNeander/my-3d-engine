#include "stdafx.h"
#include "stdlib.h"
#include "malloc.h"
#include "AIDemoEngineMob.h"
#include "opensteer/abstractvehicle.h"
#include "opensteer/vec3.h"
#include "../Config.h"

#include <iostream>

using namespace OpenSteer;
using namespace AI;
using namespace Intel;

extern Config g_ConfigFile;

int angle( Vec3 v )
{
	float a=atan2( v.z, v.x )*180.0f/3.1415f;

	if (a<0) a+=360.0;
	return (int)a;
}

void log( const char *fmt, ... )
{
#if 0
	char buffer[512];
	va_list args;

	va_start( args, fmt );
	vsprintf( buffer, fmt, args );
	va_end(args);
	OutputDebugStringA( buffer );
#endif
}

AI::AIObject::AIObject(  lqDB *spatialIndex, unsigned index, DemoEngine *engine  ) 
: m_proxy(spatialIndex), m_engine(engine), m_index(index)
{

	m_proxy.attach(this);

	CFGVAR( MaxForce, 50 );
	CFGVAR( MaxSpeed, 20 );
	CFGVAR( BugRadius, 2 );
	CFGVAR( ForwardSpeed, 3 );
	CFGVAR( InitialSpeed, 0 );
	CFGVAR( CollisionRadius, 10 );
	CFGVAR( CollisionLookahead, 0.5 );
	CFGVAR2( AvoidObstacleTime, 1.0, 8.0 );
	CFGVAR2( AvoidDynamicsTime, 1.0, 8.0 );


	CFGVAR3( SeparationData, 0.5f, -0.707f, 12.0f );
	CFGVAR3( AlignmentData, 0.66f, 0.7f, 8.0f );
	CFGVAR3( CohesionData, 0.9f, -0.15f, 8.0f );

	CFGVAR2( PursuitData, 1.1f, 5.0f );

	reset();
}

AI::AIObject::~AIObject()
{
}

void AI::AIObject::reset() 
{
	SimpleVehicle::reset (); // reset the vehicle 
	m_isAlive=true;
	setSpeed (m_cfgInitialSpeed);            // speed along Forward direction.
	setMaxForce (m_cfgMaxForce);       // steering force is clipped to this magnitude
	setMaxSpeed (m_cfgMaxSpeed);       // velocity is clipped to this magnitude
	setRadius(m_cfgBugRadius);
	setSpeed(m_cfgForwardSpeed);
	setMass(0.5);
	m_proxy.updatePosition(position().x, position().y, position().z);
}


void AI::AIObject::update( const float currentTime, const float elapsedTime ) 
{
	if (!isAlive())
		return;

	typedef SpatialObjectProxy::SPATIALOJECTCOLLECTION OBJECTS;
	typedef std::vector<Vec3> POSITIONS;

	AVGroup			neighbours;
	ObstacleGroup	obstacles;
	AVGroup			movingBlocks;

	POSITIONS		positions;

	OBJECTS objects;

	m_proxy.findObjects( position().x, position().y, position().z, 
		m_cfgCollisionRadius+m_engine->getMaxDynamicsObjectSize(), 
		objects );

	for (OBJECTS::iterator o=objects.begin(); o!=objects.end(); o++ )
	{
		SpatialObjectProxy *proxy=*o;

		if (proxy->isAI())
			neighbours.push_back(proxy->getAIObject());
		else if (proxy->isDynamics())
		{
			if (proxy->getDynamicsObject()->isMoving(0.01f))
			{
				AbstractVehicle *other=proxy->getDynamicsObject()->getVehicleFacade();

				if ((other->position()-position()).lengthSquared()<
					other->radius()*other->radius()+radius()*radius())
				{
					m_isAlive=false;
					return;
				}
				movingBlocks.push_back(proxy->getDynamicsObject()->getVehicleFacade());
			}
			else
			{
				obstacles.push_back(proxy->getDynamicsObject()->getObstacle());
				positions.push_back(proxy->getDynamicsObject()->getPosition());
			}
		}
		else
			assert(true);
	}

	Vec3 avoidObstacle=Vec3::zero;
	Vec3 avoidMovingObjects=Vec3::zero;
	
	if (obstacles.size()>0)
	{
		avoidObstacle=steerToAvoidObstacles( m_cfgAvoidObstacleTime[0], obstacles ).setYtoZero()*m_cfgAvoidObstacleTime[1];
	}
	
	if (movingBlocks.size()>0)
	{
		avoidMovingObjects=steerToAvoidCloseNeighbors(m_cfgAvoidDynamicsTime[0], movingBlocks ).setYtoZero()*m_cfgAvoidDynamicsTime[1];
	}

	if (avoidMovingObjects!=Vec3::zero)	
	{
		applySteeringForce( avoidMovingObjects, elapsedTime );
		if (m_index==0) 
			log( "Queen Move %d (%f) %d (%f)\n", 
			angle(avoidMovingObjects), avoidMovingObjects.length(), 
			angle(forward()), speed() );
	} 
	else if (avoidObstacle!=Vec3::zero)
	{
		applySteeringForce( avoidObstacle, elapsedTime );
		if (m_index==0) 
			log( "Queen Obst %d (%f) %d (%f)\n", 
			angle(avoidObstacle), avoidObstacle.length(), 
			angle(forward()), speed() );
	} 
	else if (this==m_engine->getQueen())
	{
		Vec3 path = forward() + steerToFollowPath( 1, 0.5, m_engine->getQueenPath() );

		applySteeringForce( path.setYtoZero(), elapsedTime );
		log( "Queen %d (%f) %d (%f)\n", 
			angle(path), path.length(), 
			angle(forward()), speed() );
	} else {

		AIObject &queen=*m_engine->getQueen();

		float separationRadius =  m_cfgCollisionRadius*m_cfgSeparationData[RADIUS];
		float separationAngle  = m_cfgSeparationData[ANGLE];
		float separationWeight =  m_cfgSeparationData[WEIGHT];

		float alignmentRadius = m_cfgCollisionRadius*m_cfgAlignmentData[RADIUS];
		float alignmentAngle  = m_cfgAlignmentData[ANGLE];
		float alignmentWeight = m_cfgAlignmentData[WEIGHT];

		float cohesionRadius = m_cfgCollisionRadius*m_cfgCohesionData[RADIUS];
		float cohesionAngle  = m_cfgCohesionData[ANGLE];
		float cohesionWeight = m_cfgCohesionData[WEIGHT];

		const float pursuitWeight = queen.speed()*m_cfgPursuitData[0];

		// determine each of the three component behaviors of flocking
		Vec3 separation = steerForSeparation (separationRadius,
			separationAngle,
			neighbours).setYtoZero();
		Vec3 alignment  = steerForAlignment  (alignmentRadius,
			alignmentAngle,
			neighbours).setYtoZero();
		Vec3 cohesion   = steerForCohesion   (cohesionRadius,
			cohesionAngle,
			neighbours).setYtoZero();
		Vec3 pursuit = steerForPursuit(queen).setYtoZero();
		Vec3 evade = steerForEvasion(queen, m_cfgCollisionLookahead ).setYtoZero();

		// apply weights to components (save in variables for annotation)
		Vec3 separationW = separation * separationWeight;
		Vec3 alignmentW = alignment * alignmentWeight;
		Vec3 cohesionW = cohesion * cohesionWeight;
		Vec3 pursuitW = Vec3::zero;


		if (Vec3::distance(queen.position(), position())>=m_cfgPursuitData[1])
		{
			if (!isAhead(queen.position()))
				pursuitW = pursuit * pursuitWeight;
		}

		applySteeringForce( separationW + alignmentW + cohesionW + pursuitW, elapsedTime );

	}
}

void AI::AIObject::updatePosition() 
{
	m_proxy.updatePosition(position().x, position().y, position().z );
}

Vec3 AI::AIObject::steerToAvoidDynamicsObjects( AVGroup &grp ) 
{
	AVGroup::iterator i;

	for (i=grp.begin(); i!=grp.end(); i++)
	{

	}
	return Vec3::zero;
}

bool AI::AIObject::isAlive() const
{
	return m_isAlive;
}

void AI::AIObject::applySteeringForce( const Vec3& force, const float elapsedTime ) 
{

	const Vec3 adjustedForce = adjustRawSteeringForce (force, elapsedTime);

	// enforce limit on magnitude of steering force
	const Vec3 clippedForce = adjustedForce.truncateLength (maxForce ());

	// compute acceleration and velocity
	Vec3 newAcceleration = (clippedForce / mass());
	Vec3 newVelocity = velocity();

	// Euler integrate (per frame) acceleration into velocity
	newVelocity += newAcceleration * elapsedTime;

	// enforce speed limit
	newVelocity = newVelocity.truncateLength (maxSpeed ());

	// update Speed
	setSpeed (newVelocity.length());

	// Euler integrate (per frame) velocity into position
	setPosition (position() + (newVelocity * elapsedTime));

	// regenerate local space (by default: align vehicle's forward axis with
	// new velocity, but this behavior may be overridden by derived classes.)
	regenerateLocalSpace (newVelocity, elapsedTime);
}

AI::DynamicsObject::DynamicsObject( lqDB *index, Vec3 pos ) 
: m_proxy(index)
{
	m_proxy.attach(this);
	m_pos=pos;
	m_vehicle.reset();
	updatePosition(pos);
}

AI::DynamicsObject::~DynamicsObject() 
{
}

void AI::DynamicsObject::updatePosition( Vec3 pos ) 
{
	m_oldPos=m_pos;
	m_pos=pos;
	m_proxy.updatePosition(pos.x, pos.y, pos.z );
	m_vehicle.setPosition(pos);
	
	Vec3 velocity=getVelocity();

	m_vehicle.setSpeed(velocity.length());
	m_vehicle.regenerateLocalSpace( velocity, 1.0 );
}

Vec3 AI::DynamicsObject::getPosition() const
{
	return m_pos;
}

Vec3 AI::DynamicsObject::getVelocity() const
{
	return m_pos-m_oldPos;
}

bool AI::DynamicsObject::isMoving(float delta) const
{
	return getVelocity().lengthSquared()>delta;
}

OpenSteer::AbstractVehicle* AI::DynamicsObject::getVehicleFacade()
{
	return &m_vehicle;
}

//////////////////////////////////////////////////////////////////////////
// Spatial Index Node

AI::SpatialObjectProxy::SpatialObjectProxy( lqDB *index, DynamicsObject *object ) : m_spatialIndex(index), m_type(OT_DYNAMICS), m_dynamics(object)
{
	attach(object);
}

AI::SpatialObjectProxy::SpatialObjectProxy( lqDB *index, AIObject *object ) : m_spatialIndex(index), m_type(OT_AI), m_ai(object)
{
	attach(object);
}

AI::SpatialObjectProxy::SpatialObjectProxy( lqDB *index )
: m_spatialIndex(index), m_type(OT_NONE), m_object(0)
{
}

AI::SpatialObjectProxy::~SpatialObjectProxy()
{
	if (m_type!=OT_NONE)
		lqRemoveFromBin(&m_spatialData);
}

DynamicsObject * AI::SpatialObjectProxy::getDynamicsObject() const
{
	assert(isDynamics());
	if (isDynamics())
		return m_dynamics;
	return NULL;
}

AIObject * AI::SpatialObjectProxy::getAIObject() const
{
	assert(isAI());
	if (isAI())
		return m_ai;
	return NULL;
}
void AI::SpatialObjectProxy::updatePosition( float x, float y, float z ) 
{
	lqUpdateForNewLocation( m_spatialIndex, &m_spatialData, x, y, z );
}

void AI::SpatialObjectProxy::findObjects(  float x, float y, float z, float radius, SPATIALOJECTCOLLECTION &result  ) 
{
	lqMapOverAllObjectsInLocality( m_spatialIndex, 
		x, y, z, radius, 
		collect, &result );
}

void AI::SpatialObjectProxy::collect( void* clientObject,  float distanceSquared,  void* clientQueryState ) 
{
	SPATIALOJECTCOLLECTION	*group=reinterpret_cast<SPATIALOJECTCOLLECTION*>(clientQueryState);
	SpatialObjectProxy	*v=reinterpret_cast<SpatialObjectProxy*>(clientObject);

	group->push_back(v);
}

void AI::SpatialObjectProxy::attach( DynamicsObject *object ) 
{
	assert(m_type==OT_NONE);

	m_dynamics=object;
	m_type=OT_DYNAMICS;

	lqInitClientProxy( &m_spatialData, this );
}
void AI::SpatialObjectProxy::attach( AIObject *object ) 
{
	assert(m_type==OT_NONE);

	m_ai=object;
	m_type=OT_AI;

	lqInitClientProxy( &m_spatialData, this );
}

