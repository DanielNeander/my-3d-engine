#pragma once 

#include "DetourNavMesh.h"
#include "CellSpacePartition.h"
#include "AIUtil.h"

class Actor;
class Wall2D;
class AIPath;
class NavSourceMesh;

class GameWorld : public RefCounter
{
private:
	dtNavMesh* m_navMesh;
	dtQueryFilter m_filter;
		
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

	static const int MAX_STEER_POINTS = 100;
	float m_steerPoints[MAX_STEER_POINTS*3];
	int m_steerPointCount;



	std::vector<class ActorController*>		m_Vehicles;

	std::vector<ActorController*>					m_Obstacles;

	std::vector<Wall2D>					m_Walls;

	CellSpacePartition<ActorController*>*	m_pCellSpace;
	noVec3	m_offSetVec;


	AIPath*                         m_pPath;

	//set true to pause the motion
	bool                          m_bPaused;

	//local copy of client window dimensions
	int                           m_cxClient,	m_cyClient;
	int                           m_cxClientMin, m_cyClientMin;

	//the position of the crosshair
	noVec3	                      m_vCrosshair;

	//keeps track of the average FPS
	double                        m_dAvFrameTime;

	NavDebugDraw*	dd_;

	//flags to turn aids and obstacles etc on/off
	bool  m_bShowWalls;
	bool  m_bShowObstacles;
	bool  m_bShowPath;
	bool  m_bShowDetectionBox;
	bool  m_bShowWanderCircle;
	bool  m_bShowFeelers;
	bool  m_bShowSteeringForce;
	bool  m_bShowFPS;
	bool  m_bRenderNeighbors;
	bool  m_bViewKeys;
	bool  m_bShowCellSpaceInfo;


	
	void CreateObstacles();
	void CreateWalls();

public:
	GameWorld();
	~GameWorld();

	void Init(NavSourceMesh* geom);

	void  Update(float time_elapsed);

	void  Render();

	void  NonPenetrationContraint(ActorController* v){EnforceNonPenetrationConstraint(v, m_Vehicles);}

	void  TagVehiclesWithinViewRange(ActorController* pVehicle, double range)
	{
		TagNeighbors(pVehicle, m_Vehicles, range);
	}

	void  TagObstaclesWithinViewRange(ActorController* pVehicle, double range)
	{
		TagNeighbors(pVehicle, m_Obstacles, range);
	}

	const std::vector<Wall2D>&							Walls(){return m_Walls;}                          
	CellSpacePartition<ActorController*>*     CellSpace(){return m_pCellSpace;}
	const std::vector<ActorController*>&					Obstacles()const{return m_Obstacles;}
	const std::vector<ActorController*>&      Agents(){return m_Vehicles;}

	void        TogglePause(){m_bPaused = !m_bPaused;}
	bool        Paused()const{return m_bPaused;}

	noVec3    Crosshair()const{return m_vCrosshair;}
	void        SetCrosshair(const noVec3& pos);
	//void        SetCrosshair(noVec3 v){m_vCrosshair=v;}

	int			cxClient()const{return m_cxClient;}
	int			cyClient()const{return m_cyClient;}
	int			cxClientMin()const{return m_cxClientMin;}
	int			cyClientMin()const{return m_cyClientMin;}

	bool		RenderWalls()const{return m_bShowWalls;}
	bool		RenderObstacles()const{return m_bShowObstacles;}
	bool		RenderPath()const{return m_bShowPath;}
	bool		RenderDetectionBox()const{return m_bShowDetectionBox;}
	bool		RenderWanderCircle()const{return m_bShowWanderCircle;}
	bool		RenderFeelers()const{return m_bShowFeelers;}
	bool		RenderSteeringForce()const{return m_bShowSteeringForce;}

	bool		RenderFPS()const{return m_bShowFPS;}
	void		ToggleShowFPS(){m_bShowFPS = !m_bShowFPS;}

	void		ToggleRenderNeighbors(){m_bRenderNeighbors = !m_bRenderNeighbors;}
	bool		RenderNeighbors()const{return m_bRenderNeighbors;}

	void		ToggleViewKeys(){m_bViewKeys = !m_bViewKeys;}
	bool		ViewKeys()const{return m_bViewKeys;}
	void		recalc();
	class WowActor* AddNewActor(const noVec3& actorPos, SceneNode* root, int slot);
	void		HandleFindPath();
	void		Draw();
	void Hide();
	void TestInterpose();
	void TestOffsetPursuit();
	void TestPursuit();
	void TestSeek();
	void RayCast();
};