#include "stdafx.h"

#include <process.h>
#include <malloc.h>

#include "AIDemoEngine.h"
#include "AIDemoEngineMob.h"

#include "opensteer/clock.h"
#include "opensteer/proximity.h"
#include "opensteer/vec3.h"
#include "opensteer/unusedparameter.h"
#include "opensteer/simplevehicle.h"
#include "../Config.h"

using namespace OpenSteer;
using namespace AI;
using namespace Intel;

extern Config g_ConfigFile;

// Make compiler happy with pure virtual destructor of base class
Engine::~Engine()
{
}


DemoEngine::DemoEngine()
	: m_spatialIndex(0), m_maxDynamicsObjectsSize(0)
{
	CFGVAR( WorldWidth, 3000 );
	CFGVAR( WorldHeight, 2000 );
	CFGVAR( WorldHDiv, 100 );
	CFGVAR( WorldVDiv, 50 );
	CFGVAR( WorldScale, 20 );
	CFGVAR( TicksPerSecond, 60 );
	CFGVAR( MaxThreads, 4 );
	CFGVAR( DynamicsObjMag, 1.1f );
}

DemoEngine::~DemoEngine()
{
	destroy();
}

void DemoEngine::setWorldSize( 
	float hsize, float vsize,  int divh, int divv, float scale )
{
	m_cfgWorldWidth=hsize;
	m_cfgWorldHeight=vsize;
	m_cfgWorldHDiv=float(divh);
	m_cfgWorldVDiv=float(divv);
	m_cfgWorldScale=scale;
}

int	g_iNumAIThreads = 1;

// Engine Interface
bool DemoEngine::create(unsigned int num_characters, PATH *path, int num_threads )
{
	if (destroy())
	{
		if (path)
		{
			PATH::iterator i;

			for (i=path->begin(); i!=path->end(); i++)
				m_path.push_back(cvt(*i));
		}
		float scale=1.0f/getWorldScale();
		if (m_path.empty())
		{
			Vec3 wayPoints[]=
			{
				Vec3(-0.5, 0, -0.5 ),
					Vec3(+0.5, 0, -0.5 ),
					Vec3(+0.5, 0, +0.5 ),
					Vec3(-0.5, 0, +0.5 ),
			};
			const int N=sizeof(wayPoints)/sizeof(*wayPoints);

			for (int i=0; i<N; i++)
				wayPoints[i]*=scale;
			m_pathway.setPathway(N, wayPoints, 1, true );
		} 
		else
		{
			Vec3 *wayPoints=(Vec3*)alloca(sizeof(Vec3)*m_path.size());

			for (unsigned i=0; i<m_path.size(); i++)
				wayPoints[i]=m_path[i]*scale;
			m_pathway.setPathway( m_path.size(), wayPoints, 2, true );
		}

		m_spatialIndex=lqCreateDatabase(
			-m_cfgWorldWidth/2, -m_cfgWorldWidth/2, 0, 
			m_cfgWorldWidth, m_cfgWorldWidth, m_cfgWorldHeight, 
			(int)m_cfgWorldHDiv, (int)m_cfgWorldHDiv, (int)m_cfgWorldVDiv );

		m_clock.setFixedFrameRate((int)m_cfgTicksPerSecond);
		m_clock.setAnimationMode(true);

		if (!m_spatialIndex)
			return false;
		m_aiobjects.reserve(num_characters);
		for (unsigned i=0; i<num_characters; i++)
		{
			m_aiobjects.push_back(OpenSteer::SharedPointer<AIObject>(new AIObject(m_spatialIndex, i, this)));
		}
		m_queenIndex=0;

		num_threads = (int)m_cfgMaxThreads;
		if (num_threads>1)
		{
			////////////////////////////////////////////////////////////////////////////
			///																		 ///
			///	Lab 4 Activity 1 Step 2: Create and Initialize ThreadContext array	 ///
			///																		 ///
			///	1) Create an instance of the ThreadContext wrapper class for each of ///
			///	   the Win32 threads. 												 ///
			///	2) 'init'ialize each ThreadContext structure						 ///
			/// 3) Push a shared pointer to each ThreadContext onto the m_contexts	 ///
			///    vector.															 ///

			/// ------------------------- INSERT CODE HERE ------------------------- ///
			for (int i=0; i<num_threads; i++)
			{
				ThreadContextPtr cp(new ThreadContext(this, i));

				if (!cp->init())
					break;
				m_contexts.push_back(cp);
			}
			
			///																		 ///
			////////////////////////////////////////////////////////////////////////////
		}
	}
	return true;
}

bool DemoEngine::destroy()
{
	if (m_spatialIndex)
	{
		lqRemoveAllObjects(m_spatialIndex);
		lqDeleteDatabase(m_spatialIndex);
		m_spatialIndex=0;
		m_maxDynamicsObjectsSize=0;
	}	
	m_aiobjects.clear();
	m_path.clear();
	m_dynamicobjects.clear();
	return true;
}

bool DemoEngine::tick()
{
	unsigned threads=(unsigned)m_contexts.size();

	threads = g_iNumAIThreads;

	m_clock.update();

	float currentTime = m_clock.getTotalSimulationTime();
	float elapsedTime = m_clock.getElapsedSimulationTime();

	if (threads<=1)
	{
		for (AIOBJECT_CONTAINER::iterator i=m_aiobjects.begin(); i!=m_aiobjects.end(); i++)
			(*i)->update(currentTime, elapsedTime);
	} 
	else 
	{
		HANDLE *threadEvents=(HANDLE*)alloca(sizeof(HANDLE)*threads);

		unsigned blkSize=getNumMOBS()/threads;
		////////////////////////////////////////////////////////////////////////////
		///																		 ///
		///	Lab 4 Activity 2 Step 1: Split data and Kick off Worker threads		 ///
		///																		 ///
		///	Split the AI Workload to available threads and issue a SetEvent() to ///
		/// let the worker threads start processing. This tick() function will	 ///
		/// wait till the worker threads' tick() functions complete.			 ///
		///																		 ///
		/// 1) Split getNumMOBS() amount of work between 'threads'.				 ///
		/// 2) Loop through and call the Workers' tick() functions using the	 ///
		///    ThreadContext vector with the specified chunk of data.			 ///
		/// 3) Finally, wait for all Worker threads to finish using a			 ///
		///	   WaitForMultipleObjects() call.									 ///

		/// ------------------------- INSERT CODE HERE ------------------------- ///
		for (unsigned i=0; i<threads; i++)
		{
			unsigned from=i*blkSize;
			unsigned to=from+blkSize;

			if (to>=getNumMOBS())
				to=getNumMOBS();
			m_contexts[i]->tick(from, to, currentTime, elapsedTime );
			threadEvents[i]=m_contexts[i]->getDoneEvent();
		}
		::WaitForMultipleObjects(threads, threadEvents, TRUE, INFINITE);
		
	    ///																		 ///
	    ////////////////////////////////////////////////////////////////////////////
	}
	updatePositions();
	if (!getQueen()->isAlive())
	{
		while(m_queenIndex<(int)m_aiobjects.size()-1)
		{
			m_queenIndex++;
			if (getQueen()->isAlive())
				break;
		}
	}
	return true;
}

void AI::DemoEngine::tick( unsigned from, unsigned to, float currentTime, float elapsedTime ) 
{
	AIOBJECT_CONTAINER::iterator start;
	AIOBJECT_CONTAINER::iterator end;

	start=m_aiobjects.begin();
	advance(start,from);
	end=start;
	advance(end,to-from);

	for (AIOBJECT_CONTAINER::iterator i=start; i!=end; i++)
		(*i)->update(currentTime, elapsedTime);	
}


void AI::DemoEngine::updatePositions() 
{
	for (AIOBJECT_CONTAINER::iterator i=m_aiobjects.begin(); i!=m_aiobjects.end(); i++)
		(*i)->updatePosition();
}


void DemoEngine::getInfo( AIObject &mob, AIObjectInfo &info )
{
	D3DXMATRIX		&mat=info.m_transformation;

	*reinterpret_cast<OpenSteer::Vec3*>(&mat._11)=mob.side();
	*reinterpret_cast<OpenSteer::Vec3*>(&mat._21)=mob.up();
	*reinterpret_cast<OpenSteer::Vec3*>(&mat._31)=mob.forward();
	*reinterpret_cast<OpenSteer::Vec3*>(&mat._41)=mob.position()*m_cfgWorldScale;
	mat._14=mat._24=mat._34=0; mat._44=1;

	info.m_extra=mob.m_extra;
	info.m_extra.z=mob.isAlive() ? 0.0f : 1.0f;
}

void DemoEngine::setInfo( AIObject &mob, const AIObjectInfo &info )
{
	D3DXMATRIX		mat(info.m_transformation);

	mob.setSide(*reinterpret_cast<OpenSteer::Vec3*>(&mat._11));
	mob.setUp(*reinterpret_cast<OpenSteer::Vec3*>(&mat._21));
	mob.setForward(*reinterpret_cast<OpenSteer::Vec3*>(&mat._31));
	mob.setPosition(*reinterpret_cast<OpenSteer::Vec3*>(&mat._41)*(1/m_cfgWorldScale));

	mob.m_extra = info.m_extra;
}

bool DemoEngine::getPositions( AIOBJECTINFOS &mobs )
{
	AIObjectInfo			tmp;

	mobs.reserve(m_aiobjects.size());
	for (AIOBJECT_CONTAINER::iterator i=m_aiobjects.begin(); i!=m_aiobjects.end(); i++)
	{
		AIObject &mob=**i;

		getInfo(mob, tmp);
		mobs.push_back(tmp);
	}
	return true;
}

unsigned DemoEngine::getNumMOBS()
{
	return (unsigned)m_aiobjects.size();
}

bool DemoEngine::getMOBInfo(unsigned index, AIObjectInfo &info)
{
	if (index>=getNumMOBS())
		return false;

	AIObject &mob=*m_aiobjects[index];
	getInfo(mob, info);
	return true;
}

bool AI::DemoEngine::setMOBInfo( unsigned index, AIObjectInfo &info ) 
{
	if (index>=getNumMOBS())
		return false;

	AIObject &mob=*m_aiobjects[index];
	setInfo(mob, info);
	return true;
}

AI::DemoEngine::AIObjectPtr AI::DemoEngine::getMOB( unsigned index ) 
{
	if (index>=getNumMOBS())
		return AIObjectPtr();

	return m_aiobjects[index];	
}

void AI::DemoEngine::updateDynamicsObject( unsigned int index , noVec3 pos, D3DXMATRIX rot, float radius) 
{
	Vec3 _pos(pos.x, pos.y, pos.z );
	_pos*=1.0f/m_cfgWorldScale;

	if (index>=m_dynamicobjects.size()) {
		radius/=m_cfgWorldScale;
		if (radius>m_maxDynamicsObjectsSize)
			m_maxDynamicsObjectsSize=radius;
		DynamicsObjectPtr obj(new SphereDynamicsObject(m_spatialIndex, radius, _pos ));
		m_dynamicobjects.push_back(obj);	
	} else {
		DynamicsObjectPtr obj=m_dynamicobjects[index];

		obj->updatePosition(_pos);
	}
}

void AI::DemoEngine::updateDynamicsObject( unsigned int index , noVec3 pos, D3DXMATRIX rot, float w, float h, float d ) 
{
	Vec3 _pos(pos.x, pos.y, pos.z );
	_pos*=1.0f/m_cfgWorldScale;

	Vec3 forw=*reinterpret_cast<OpenSteer::Vec3*>(&rot._31);
	Vec3 up=*reinterpret_cast<OpenSteer::Vec3*>(&rot._21);

	if (index>=m_dynamicobjects.size())
	{
		float scale=m_cfgDynamicsObjMag/m_cfgWorldScale;
		w*=scale;
		h*=scale;
		d*=scale;

		float radius=w>h ? w : h;

		if (radius<d)
			radius=d;
		radius/=2;

		if (radius>m_maxDynamicsObjectsSize)
			m_maxDynamicsObjectsSize=radius;
		DynamicsObjectPtr obj(new BoxDynamicsObject(m_spatialIndex, w, h, d, _pos ));
		m_dynamicobjects.push_back(obj);
		obj->updateRotation(forw, up);
	} else {
		DynamicsObjectPtr obj=m_dynamicobjects[index];

		obj->updatePosition(_pos);
		obj->updateRotation(forw, up);
	}
}

float AI::DemoEngine::getMaxDynamicsObjectSize() const
{
	return m_maxDynamicsObjectsSize;
}

AIObject * AI::DemoEngine::getQueen() 
{
	return m_aiobjects[m_queenIndex].get();
}

OpenSteer::PolylineSegmentedPathwaySingleRadius & AI::DemoEngine::getQueenPath() 
{
	return m_pathway;
}

AI::Engine *AI::factory(unsigned num_characters, PATH *path )
{
	std::auto_ptr<DemoEngine> p(new DemoEngine);

	if (p.get() && p->create(num_characters, path))
		return p.release();
	return 0;
}

// Thread context functions
AI::DemoEngine::ThreadContext::ThreadContext( DemoEngine *engine, unsigned index ) 
: m_index(index), m_engine(engine), m_stopped(false)
#ifdef THREADCONTEXT_EVENTS_DECLARED
,m_evtStart(0), m_evtDone(0), m_thread(0)
#endif
{}

AI::DemoEngine::ThreadContext::~ThreadContext() 
{
	close();
}

inline static void SafeCloseHandle( HANDLE &h )
{
	if (h)
		::CloseHandle(h);
	h=0;
}

void AI::DemoEngine::ThreadContext::close() 
{
	haltThread();
#ifdef THREADCONTEXT_EVENTS_DECLARED
	SafeCloseHandle(m_evtStart);
	SafeCloseHandle(m_evtDone);
	SafeCloseHandle(m_thread);
#endif
}

bool AI::DemoEngine::ThreadContext::init() 
{
	m_stopped=false;
	////////////////////////////////////////////////////////////////////////////
	///																		 ///
	///	Lab 4 Activity 1 Step 3: Initialize ThreadContext					 ///
	///																		 ///
	/// 1) Create Start and Done events used to signal the worker threads	 ///
	///	2) Create a Win32 Thread using _beginthreadex() with a 5K stack set  ///
	///    with entry point as _WorkerThreadProc						     ///
	///																		 ///

	/// ------------------------- INSERT CODE HERE ------------------------- ///
	m_evtStart=::CreateEvent(0, FALSE, FALSE, 0 );
	m_evtDone=::CreateEvent(0, FALSE, FALSE, 0 );

	if (m_evtDone!=0 && m_evtStart!=0)
	{
		m_thread=(HANDLE)::_beginthreadex(NULL, 512*1024, _WorkerThreadProc, this, 0, NULL );
		if (m_thread!=0 && m_thread!=INVALID_HANDLE_VALUE)
		{
			return true;
		}
	}
	
    ///																		 ///
	////////////////////////////////////////////////////////////////////////////

	close();
	return false;
}

void AI::DemoEngine::ThreadContext::tick( unsigned from, unsigned to, float currentTime, float elapsedTime ) 
{
	m_from=from;
	m_to=to;
	m_currentTime=currentTime;
	m_elapsedTime=elapsedTime;
#ifdef THREADCONTEXT_EVENTS_DECLARED
	::SetEvent(m_evtStart);
#endif
}

void AI::DemoEngine::ThreadContext::haltThread() 
{
#ifdef THREADCONTEXT_EVENTS_DECLARED
	if (m_thread==0)
		return;
	m_stopped=true;
	SetEvent(m_evtStart);
	::WaitForSingleObject(m_thread,INFINITE);	
#endif
}

unsigned __stdcall AI::DemoEngine::ThreadContext::_WorkerThreadProc( void *arg ) 
{
	ThreadContext *ctxt=static_cast<ThreadContext*>(arg);
	assert(ctxt);

	ctxt->WorkerThreadProc();
	return 0;
}

void AI::DemoEngine::ThreadContext::WorkerThreadProc() 
{
	for(;;)
	{
		////////////////////////////////////////////////////////////////////////////
		///																		 ///
		///	Lab 4 Activity 2 Step 2: Implement Worker thread function			 ///
		///																		 ///
		/// This function will wait till a Start event signal from the main AI	 ///
		/// thread, and then call the DemoEngine's tick with the given slice of	 ///
		/// data.																 ///
		///	1) Wait for m_evtStart event										 ///
		///	2) Check m_stopped to see if the AI DemoEngine is halted			 ///
		///	3) Issue the DemoEngine tick to handle the subset of data.			 ///
		///	4) SetEvent m_evtDone to let the main AIThread know we're done.		 ///
		///																		 ///

		/// ------------------------- INSERT CODE HERE ------------------------- ///
		::WaitForSingleObject(m_evtStart, INFINITE);
		if (m_stopped)
			break;
		m_engine->tick(m_from, m_to, m_currentTime,m_elapsedTime);
		::SetEvent(m_evtDone);
 
        ///																		 ///
		////////////////////////////////////////////////////////////////////////////
	} 
}
HANDLE AI::DemoEngine::ThreadContext::getDoneEvent() 
{
#ifdef THREADCONTEXT_EVENTS_DECLARED
	return m_evtDone;
#else
	return NULL;
#endif
}
