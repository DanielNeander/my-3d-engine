#ifndef __AIDEMOENGINE_H
#define __AIDEMOENGINE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <vector>
#include <list>
#include <iterator>
//#include <boost/shared_ptr.hpp>
#include "opensteer/sharedpointer.h"
#include "opensteer/clock.h"
#include "opensteer/lq.h"
#include "AIEngine.h"
#include "opensteer/obstacle.h"
#include "opensteer/polylinesegmentedpathwaysingleradius.h"

namespace AI
{
	class DynamicsObject;
	class AIObject;

	typedef std::vector<OpenSteer::Vec3>	PATHIMP;

	struct DemoEngine : public Engine {
		DemoEngine();
		// Engine Interface
		virtual ~DemoEngine();
		virtual void setWorldSize( float hsize, float vsize,  int divh, int divv, float scale );
		virtual bool create(unsigned int num_characters, PATH *path, int num_threads=-1 );
		virtual bool destroy();
		virtual bool tick();
		virtual bool getPositions( AIOBJECTINFOS &mobs );
		virtual unsigned getNumMOBS();
		virtual bool getMOBInfo(unsigned index, AIObjectInfo &info);
		virtual bool setMOBInfo(unsigned index, AIObjectInfo &info);
		virtual void updateDynamicsObject( unsigned int index , noVec3 pos, D3DXMATRIX rot, float radius);
		virtual void updateDynamicsObject( unsigned int index , noVec3 pos, D3DXMATRIX rot, float w, float h, float d);

		void tick( unsigned from, unsigned to, float currentTime, float elapsedTime );
		void updatePositions();

		float getMaxDynamicsObjectSize() const;

		typedef OpenSteer::SharedPointer<DynamicsObject> DynamicsObjectPtr;
		typedef OpenSteer::SharedPointer<AIObject> AIObjectPtr;

		AIObjectPtr getMOB(unsigned index);

		AIObject *getQueen();

		float getWorldWidth() const { return m_cfgWorldWidth; }
		float getWorldScale() const { return m_cfgWorldScale; }

		const PATHIMP &getPath() const { return m_path; }

		OpenSteer::PolylineSegmentedPathwaySingleRadius &getQueenPath();

	private:

		void				getInfo( AIObject &mob, AIObjectInfo &info );
		void				setInfo( AIObject &mob, const AIObjectInfo &info );

		PATHIMP				m_path;

		//
		typedef std::vector<AIObjectPtr>		AIOBJECT_CONTAINER;
		typedef std::vector<DynamicsObjectPtr>	DYNAMICSOBJECT_CONTAINER;

		// 
		int					m_queenIndex;

		OpenSteer::PolylineSegmentedPathwaySingleRadius m_pathway;

		OpenSteer::Clock	m_clock;
		lqDB				*m_spatialIndex;
		float				m_maxDynamicsObjectsSize;

		AIOBJECT_CONTAINER			m_aiobjects;
		DYNAMICSOBJECT_CONTAINER	m_dynamicobjects;

		// Config values
		float				m_cfgWorldWidth;
		float				m_cfgWorldHeight;
		float 				m_cfgWorldHDiv;
		float				m_cfgWorldVDiv;

		float				m_cfgWorldScale;

		float				m_cfgTicksPerSecond;

		float				m_cfgMaxThreads;

		float				m_cfgDynamicsObjMag;

		////////////////////////////////////////////////////////////////////////////
		///																		 ///
		///	Lab 4 Activity 1 Step 1: Declare ThreadContext structure			 ///
		///																		 ///
		///	Declare a wrapper class for Win32 threads. Each instance's tick()	 ///
		/// function copies thread local data and wakes up the thread by issuing ///
		///	a SetEvent() call.													 ///
		///	1) As part of the lab, uncomment the macro definition for			 ///
		///    THREADCONTEXT_EVENTS_DECLARED									 ///
		/// 2) Add a declaration for the Thread handle (m_thread)				 ///
		///	3) Add declarations for the Start and Done event handles (m_evtStart ///
		///    and m_evtDone)													 ///
		///																		 ///

		/// ------------------ INCLUDE THE FOLLOWING BLOCK ---------------------- ///
#define THREADCONTEXT_EVENTS_DECLARED

		class ThreadContext
		{
		public:
			ThreadContext(DemoEngine *engine, unsigned index);
			~ThreadContext();
			bool init();
			void close();
			void tick( unsigned from, unsigned to, float currentTime, float elapsedTime );
			void haltThread();
			HANDLE getDoneEvent();

		protected:
			unsigned	m_index;
			HANDLE		m_evtStart;
			HANDLE		m_evtDone;
			HANDLE		m_thread;
			DemoEngine	*m_engine;
			bool		m_stopped;
			unsigned	m_from, m_to;
			float		m_currentTime, m_elapsedTime;

			void WorkerThreadProc();
			static unsigned __stdcall _WorkerThreadProc( void *arg );
		};

		typedef OpenSteer::SharedPointer<ThreadContext>	ThreadContextPtr;
		std::vector<ThreadContextPtr>				m_contexts;
	};
}

// Some helpers to init config vars
#define CFGVAR( v, a ) m_cfg##v = g_ConfigFile.GetEntryDataWithDefault( "AI_"#v, a )
#define CFGVAR2( v, a, b ) g_ConfigFile.GetEntryDataWithDefault( "AI_"#v, m_cfg##v, a, b )
#define CFGVAR3( v, a, b, c ) g_ConfigFile.GetEntryDataWithDefault( "AI_"#v, m_cfg##v, a, b, c )

inline OpenSteer::Vec3 cvt( const noVec3 &v )
{
	return OpenSteer::Vec3( v.x, v.y, v.z );
}

inline noVec3 cvt( const OpenSteer::Vec3 &v )
{
	return noVec3( v.x, v.y, v.z );
}


#endif
