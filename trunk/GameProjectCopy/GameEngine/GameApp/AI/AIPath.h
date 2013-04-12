#pragma once 

struct NavDebugDraw;

class AIPath
{
private:

	std::list<noVec3>            m_WayPoints;

	//points to the current waypoint
	std::list<noVec3>::iterator  curWaypoint;

	//flag to indicate if the path should be looped
	//(The last waypoint connected to the first)
	bool                           m_bLooped;

	// flag to indicate if we have reached the end of the path
	bool						   m_bFinishedPath;
	// flag to indicate if we have had at least one complete run through of the path
	// used to tell us if we need to start looking for a new path
	bool						   m_bOneLoopDone;

	NavDebugDraw* dd;

public:

	AIPath():m_bLooped(false),m_bFinishedPath(false),m_bOneLoopDone(false),dd(0){ initDD(); }

	//constructor for creating a path with initial random waypoints. MinX/Y
	//& MaxX/Y define the bounding box of the path.
	AIPath(int    NumWaypoints,
		double MinX,
		double MinY,
		double MaxX,
		double MaxY,
		bool   looped):m_bLooped(looped),m_bFinishedPath(false),m_bOneLoopDone(false),dd(0)
	{
		CreateRandomPath(NumWaypoints, MinX, MinY, MaxX, MaxY);

		curWaypoint = m_WayPoints.begin();

		initDD();
	}

	~AIPath() { if(dd){delete dd; dd=0;} }
	// initialize the debug drawer object
	void initDD(void);
	//returns the current waypoint
	noVec3    CurrentWaypoint()const
	{
		assert(*curWaypoint != vec3_zero); 
		return *curWaypoint;
	}

	//returns true if the end of the list has been reached
	bool        Finished(){return !(curWaypoint != m_WayPoints.end());}

	//moves the iterator on to the next waypoint in the list
	inline void SetNextWaypoint();

	//creates a random path which is bound by rectangle described by
	//the min/max values
	std::list<noVec3> CreateRandomPath(int    NumWaypoints,
		double MinX,
		double MinY,
		double MaxX,
		double MaxY);


	void LoopOn(){m_bLooped = true;}
	void LoopOff(){m_bLooped = false;}
	void SetLoop(bool _loop) { m_bLooped = _loop; }
	bool GetLoop(){return m_bLooped;}

	// returns true if the path has been finished, false otherwise
	// used in path following obviously, to avoid asserts on reaching end and waiting
	// for new path to be generated
	bool PathFinished(){ return m_bFinishedPath; }

	// returns true of we have done at least one run-through of the path, without loops
	// false if we have not dont at least one run though of the path yet.
	// used to indicate if we should start look for a new path
	bool GetPathRunThroughOnce(void) { return m_bOneLoopDone; }
	// used to set if we have been through the path once. called after we get a TRUE on GetPathRunThroughOnce()
	// and we have generated a new path.
	void SetPathRunThroughOnce(bool _onceThrough) { m_bOneLoopDone = _onceThrough; }

	//adds a waypoint to the end of the path
	void AddWayPoint(noVec3 new_point) { m_WayPoints.push_back(new_point); curWaypoint = m_WayPoints.begin(); /*curWaypoint++;*/ }

	//methods for setting the path with either another Path or a list of vectors
	void Set(std::list<noVec3> new_path)
	{
		m_WayPoints.resize(0);
		m_WayPoints = new_path;
		curWaypoint = m_WayPoints.begin();
	}
	void Set(const AIPath& path)
	{
		m_WayPoints.resize(0);
		m_WayPoints=path.GetPath(); 
		curWaypoint = m_WayPoints.begin();
	}


	void Clear(){m_WayPoints.clear();}

	std::list<noVec3> GetPath()const{return m_WayPoints;}

	//renders the path in orange
	void Render()const; 
};




//-------------------- Methods -------------------------------------------

inline void AIPath::SetNextWaypoint()
{
	assert (m_WayPoints.size() > 0);

	if(m_bFinishedPath == false)
	{
		if (++curWaypoint == m_WayPoints.end())
		{
			if (m_bLooped)
			{
				curWaypoint = m_WayPoints.begin(); 
			}
			else
			{
				m_bFinishedPath = true;
				m_bOneLoopDone = true;
				std::list<noVec3> new_path;
				new_path.resize(0);

				std::list<noVec3>::const_iterator it = m_WayPoints.begin();
				noVec3 wp = *it;
				while (it != m_WayPoints.end())
				{
					new_path.push_front(wp);
					wp = *it++;
				}
				m_WayPoints.resize(0);
				Set(new_path);
			}
		}
	}
	else if(m_bFinishedPath == true)
	{
		if (++curWaypoint == m_WayPoints.end())
		{
			if (m_bLooped)
			{
				curWaypoint = m_WayPoints.begin(); 
			}
			else
			{
				m_bFinishedPath = false;
				std::list<noVec3> new_path;
				new_path.resize(0);

				std::list<noVec3>::const_iterator it = m_WayPoints.begin();
				noVec3 wp = *it;
				while (it != m_WayPoints.end())
				{
					new_path.push_front(wp);
					wp = *it++;
				}

				m_WayPoints.resize(0);
				Set(new_path);
			}
		}
	}
}  



