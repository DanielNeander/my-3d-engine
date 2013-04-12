#ifndef __AIDEMOENGINEMOB_H
#define __AIDEMOENGINEMOB_H

#include "AIDemoEngine.h"
#include "opensteer/simplevehicle.h"
#include "opensteer/lq.h"
#include "opensteer/polylinesegmentedpathwaysingleradius.h"
#include "opensteer/obstacle.h"
//#include "boost/smart_ptr.hpp"
//#include "cylinderobstacle.h"
#include "opensteer/sharedpointer.h"

using namespace OpenSteer;

namespace AI
{
	class DynamicsObject;
	class AIObject;

	class SpatialObjectProxy { 
	public:
		SpatialObjectProxy( lqDB *index, DynamicsObject *object );
		SpatialObjectProxy( lqDB *index, AIObject *object );
		SpatialObjectProxy( lqDB *index );
		virtual ~SpatialObjectProxy();
		void attach( DynamicsObject *object );
		void attach( AIObject *object );
		bool isDynamics() const { return m_type==OT_DYNAMICS; }
		bool isAI() const { return m_type== OT_AI; }
		DynamicsObject *getDynamicsObject() const;
		AIObject *getAIObject() const;
		void updatePosition( float x, float y, float z );
		typedef std::list<SpatialObjectProxy*> SPATIALOJECTCOLLECTION;
		void findObjects( float x, float y, float z, float radius, SPATIALOJECTCOLLECTION &result );
	protected:
		lqClientProxy	m_spatialData;
		lqDB			*m_spatialIndex;
		enum OBJECT_TYPE { OT_NONE, OT_AI, OT_DYNAMICS };		
		OBJECT_TYPE			m_type;
		union {
			void			*m_object;
			DynamicsObject	*m_dynamics;
			AIObject		*m_ai;
		};
		static void collect(void* clientObject,
			float distanceSquared,
			void* clientQueryState);
	};

	class DynamicsObjectVehicle : public OpenSteer::SimpleVehicle
	{
	public:
		void update( const float, const float ){}
	};

	class DynamicsObject
	{
	public:
		DynamicsObject( lqDB *index, Vec3 pos );
		virtual ~DynamicsObject();
		virtual void updatePosition( Vec3 pos );
		virtual void updateRotation( Vec3 forw, Vec3 up ) {}
		virtual OpenSteer::AbstractObstacle *getObstacle()=0;
		Vec3 getPosition() const;
		Vec3 getVelocity() const;
		bool isMoving(float delta=0.0) const;
		OpenSteer::AbstractVehicle* getVehicleFacade();
	protected:
		SpatialObjectProxy	m_proxy;
		Vec3				m_pos;
		Vec3				m_oldPos;
		DynamicsObjectVehicle m_vehicle;
	};

	class SphereDynamicsObject : public DynamicsObject 
	{
	public:
		SphereDynamicsObject( lqDB *index, float radius, Vec3 pos )
			: DynamicsObject( index, pos ), m_obstacle( radius, pos )
		{
			m_vehicle.setRadius(radius);
		}
		virtual void updatePosition( Vec3 pos )
		{
			DynamicsObject::updatePosition(pos);
			m_obstacle.center=pos;
		}
		virtual OpenSteer::AbstractObstacle *getObstacle()
		{
			return &m_obstacle;
		}
	protected:
		OpenSteer::SphereObstacle	m_obstacle;
	};

	class BoxDynamicsObject : public DynamicsObject 
	{
	public:
		BoxDynamicsObject( lqDB *index, float w, float h, float d, Vec3 pos )
			: DynamicsObject(index, pos ), m_obstacle(w, h, d )
		{
			m_vehicle.setRadius(Vec3(w,h,d).length()/2.0f);
		}
		virtual void updatePosition( Vec3 pos )
		{
			DynamicsObject::updatePosition(pos);
			m_obstacle.setPosition(pos);
		}
		virtual void updateRotation( Vec3 forw, Vec3 up )
		{
			m_obstacle.setForward(forw);
			m_obstacle.setUp(up);
			m_obstacle.setUnitSideFromForwardAndUp();
		}
		virtual OpenSteer::AbstractObstacle *getObstacle()
		{
			return &m_obstacle;	
		}
	protected:
		OpenSteer::BoxObstacle	m_obstacle;

	};

	class AIObject : public OpenSteer::SimpleVehicle
	{
	public:
		AIObject( lqDB *spatialIndex, unsigned index, DemoEngine *engine );
		~AIObject();
		void reset();
		void update(const float currentTime, const float elapsedTime);
		void updatePosition();

		bool isAlive() const;

		Vec3 steerToAvoidDynamicsObjects( AVGroup &grp );

		// apply a given steering force to our momentum,
		// adjusting our orientation to maintain velocity-alignment.
		// Replaces SimpleVehicle Implementation
		void applySteeringForce (const Vec3& force, const float deltaTime);

		noVec4	m_extra;

		bool	m_isAlive;

		// Spatialindex helper data
		SpatialObjectProxy	m_proxy;
		DemoEngine			*m_engine;
		unsigned			m_index;

		float	m_cfgMaxForce;
		float	m_cfgMaxSpeed;
		float	m_cfgBugRadius;
		float	m_cfgForwardSpeed;
		float	m_cfgInitialSpeed;
		float   m_cfgCollisionRadius;
		float	m_cfgCollisionLookahead;
		float	m_cfgAvoidObstacleTime[2];
		float	m_cfgAvoidDynamicsTime[2];

		enum { RADIUS=0, ANGLE, WEIGHT, };
		float	m_cfgSeparationData[3];
		float	m_cfgAlignmentData[3];
		float	m_cfgCohesionData[3];

		float	m_cfgPursuitData[2];
	};
}

#endif
