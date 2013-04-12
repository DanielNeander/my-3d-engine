#include "stdafx.h"
#include "AIPath.h"
#include "Wall2D.h"
#include "GameApp/GameApp.h"
#include "GameApp/Mesh.h"
#include "GameApp/SceneNode.h"
#include "GameApp/M2Mesh.h"
#include "GameApp/TestObjects.h"
#include "GameApp/GameObjectManager.h"
#include "GameApp/Common.h"
#include "ModelLib/M2Loader.h"
#include "AICommon.h"
#include "AIConfig.h"
#include "NavDebugDraw.h"
#include "NavSourceMesh.h"
#include "SteeringBehavior.h"
#include "GameWorld.h"
#include "GameApp/Actor.h"
#include "GameApp/ActorInput.h"

#include "Recast.h"

static const char* modelname[] = {
	"Character\\BloodElf\\female\\BloodElfFemale.m2",
	"Creature\\AbyssalOutland\\Abyssal_Outland.m2",
	"Creature\\alexstrasza\\alexstrasza.m2",
	"Creature\\arthas\\arthas.m2",
	"Creature\\FireSpiritsmall\\firespiritsmall.m2"
};



void GameWorld::CreateObstacles()
{
	for (int o=0; o < AICON.NumObstacles; ++o)
	{
		bool bOverlapped = true;
	
		//keep creating tiddlywinks until we find one that doesn't overlap
		//any others.Sometimes this can get into an endless loop because the
		//obstacle has nowhere to fit. We test for this case and exit accordingly

		int NumTrys = 0; int NumAllowableTrys = 2000;

		while (bOverlapped)
		{
			NumTrys++;

			if (NumTrys > NumAllowableTrys) return;

			float scale = 0.1f;
			int radius = RandInt((int)AICON.MinObstacleRadius,  (int)AICON.MaxObstacleRadius);
			radius *= scale;
			const int border                 = 10 * scale;
			const int MinGapBetweenObstacles = 20 * scale;

			noVec3 pos(RandInt(radius+border, m_cxClient-radius-border), 0.0f,  RandInt(radius+border, m_cyClient-radius-30-border));
			WowActor* ob = new WowActor(modelname[0]);
			ob->SetID(g_database.GetNewObjectID());
			ob->SetType(0);
			
			ActorController* pACtrl = new ActorController( ob,
				this, pos, RandFloat() * noMath::TWO_PI, 
				vec3_zero, AICON.VehicleMass, AICON.MaxSteeringForce, AICON.MaxSpeed, AICON.MaxTurnRatePerSecond );
			
			pACtrl->SetBRadius(radius);
			ob->PushStateMachine(*pACtrl);
							

			if (!Overlapped(pACtrl, m_Obstacles, MinGapBetweenObstacles))
			{
				//its not overlapped so we can add it
				m_Obstacles.push_back(pACtrl);

				GameObjectManager::Get()->AddGameObject(ob);
				GetApp()->GetActorRoot()->AddChild(ob->GetNode());

				bOverlapped = false;
			}			
		}
	}
}

void GameWorld::CreateWalls()
{

}

GameWorld::GameWorld()
	:m_bPaused(false),
	m_vCrosshair(noVec3(cxClient()/2.0, 0.0f,  cxClient()/2.0)),
	m_bShowWalls(false),
	m_bShowObstacles(false),
	m_bShowPath(false),
	m_bShowWanderCircle(false),
	m_bShowSteeringForce(false),
	m_bShowFeelers(false),
	m_bShowDetectionBox(false),
	m_bShowFPS(true),
	m_dAvFrameTime(0),
	m_pPath(NULL),
	m_bRenderNeighbors(false),
	m_bViewKeys(false),
	m_bShowCellSpaceInfo(false)	
{
	m_pCellSpace = NULL;
	m_filter.includeFlags = SAMPLE_POLYFLAGS_ALL;
	m_filter.excludeFlags = 0;

	AICON.load("AI.cfg");
}

GameWorld::~GameWorld()
{
	SAFE_DELETE(dd_);
	SAFE_DELETE(m_pCellSpace);
}

void GameWorld::Update( float time_elapsed )
{

}

void GameWorld::Render()
{

}

void GameWorld::Init(NavSourceMesh* pGeom)
{
#if 1
	m_navMesh = GetApp()->getNavMesh();
	recalc();

	// setup the bounds for our steering agents
	const float* maxBound = pGeom->getMeshBoundsMax();
	const float* minBound = pGeom->getMeshBoundsMin();
	m_cxClient = maxBound[0];
	m_cyClient = maxBound[2];
	m_cxClientMin = minBound[0];
	m_cyClientMin = minBound[2];

	float cx = ((m_cxClientMin - m_cxClient) / -1);
	float cy = ((m_cyClientMin - m_cyClient) / -1);
	m_offSetVec.x = m_cxClientMin;
	m_offSetVec.y = m_cyClientMin;
	//setup the spatial subdivision class
	m_pCellSpace = new CellSpacePartition<ActorController*>((double)cx, (double)cy, AICON.NumCellsX, AICON.NumCellsY, AICON.NumAgents, (m_cxClientMin / -1), (m_cyClientMin / -1));
	m_pCellSpace -> SetCellHeight(minBound[1], maxBound[1]);

	if (m_navMesh)
	{
		// Change costs.
		m_navMesh->setAreaCost(SAMPLE_POLYAREA_GROUND, 1.0f);
		m_navMesh->setAreaCost(SAMPLE_POLYAREA_WATER, 10.0f);
		m_navMesh->setAreaCost(SAMPLE_POLYAREA_ROAD, 1.0f);
		m_navMesh->setAreaCost(SAMPLE_POLYAREA_DOOR, 1.0f);
		m_navMesh->setAreaCost(SAMPLE_POLYAREA_GRASS, 2.0f);
		m_navMesh->setAreaCost(SAMPLE_POLYAREA_JUMP, 1.5f);
	}
#endif

	//CreateObstacles();


	dd_ = new NavDebugDraw();
	dd_->setOffset(0.15f);
	dd_->CreateVertexBuffer(NULL, 10000, sizeof(DebugVertex));
	//ddMain->CreateIndexBuffer(geom->getMesh()->getTris(), geom->getMesh()->getTriCount() * 3);
	dd_->SetupShader();


}

void GameWorld::RayCast()
{
	
	{
		m_nstraightPath = 0;
		if (m_sposSet && m_eposSet && m_startRef)
		{
#ifdef DUMP_REQS
			printf("rc  %f %f %f  %f %f %f  0x%x 0x%x\n",
				m_spos[0],m_spos[1],m_spos[2], m_epos[0],m_epos[1],m_epos[2],
				m_filter.includeFlags, m_filter.excludeFlags); 
			rcGetLog()->log(RC_LOG_PROGRESS, "rc  %f %f %f  %f %f %f  0x%x 0x%x\n",
				m_spos[0],m_spos[1],m_spos[2], m_epos[0],m_epos[1],m_epos[2],
				m_filter.includeFlags, m_filter.excludeFlags);
#endif
			float t = 0;
			m_npolys = 0;
			m_nstraightPath = 2;
			m_straightPath[0] = m_spos[0];
			m_straightPath[1] = m_spos[1];
			m_straightPath[2] = m_spos[2];
			m_npolys = m_navMesh->raycast(m_startRef, m_spos, m_epos, &m_filter, t, m_hitNormal, m_polys, MAX_POLYS);
			if (t > 1)
			{
				// No hit
				rcVcopy(m_hitPos, m_epos);
				m_hitResult = false;
			}
			else
			{
				// Hit
				m_hitPos[0] = m_spos[0] + (m_epos[0] - m_spos[0]) * t;
				m_hitPos[1] = m_spos[1] + (m_epos[1] - m_spos[1]) * t;
				m_hitPos[2] = m_spos[2] + (m_epos[2] - m_spos[2]) * t;
				if (m_npolys)
				{
					float h = 0;
					m_navMesh->getPolyHeight(m_polys[m_npolys-1], m_hitPos, &h);
					m_hitPos[1] = h;
				}
				m_hitResult = true;
			}
			rcVcopy(&m_straightPath[3], m_hitPos);
		}
	}
}


void GameWorld::recalc()
{
	if (!m_navMesh)
		return;

	if (m_sposSet)
		m_startRef = m_navMesh->findNearestPoly(m_spos, m_polyPickExt, &m_filter, 0);
	else
		m_startRef = 0;

	if (m_eposSet)
		m_endRef = m_navMesh->findNearestPoly(m_epos, m_polyPickExt, &m_filter, 0);
	else
		m_endRef = 0;

#if 0
	//if (m_toolMode == TOOLMODE_PATHFIND_ITER)
	{
		m_pathIterNum = 0;
		if (m_sposSet && m_eposSet && m_startRef && m_endRef)
		{
#ifdef DUMP_REQS
			printf("pi  %f %f %f  %f %f %f  0x%x 0x%x\n",
				m_spos[0],m_spos[1],m_spos[2], m_epos[0],m_epos[1],m_epos[2],
				m_filter.includeFlags, m_filter.excludeFlags); 
			rcGetLog()->log(RC_LOG_PROGRESS, "pi  %f %f %f  %f %f %f  0x%x 0x%x\n",
				m_spos[0],m_spos[1],m_spos[2], m_epos[0],m_epos[1],m_epos[2],
				m_filter.includeFlags, m_filter.excludeFlags );
#endif

			m_npolys = m_navMesh->findPath(m_startRef, m_endRef, m_spos, m_epos, &m_filter, m_polys, MAX_POLYS);

			m_nsmoothPath = 0;

			if (m_npolys)
			{
				// Iterate over the path to find smooth path on the detail mesh surface.
				const dtPolyRef* polys = m_polys; 
				int npolys = m_npolys;

				float iterPos[3], targetPos[3];
				m_navMesh->closestPointOnPolyBoundary(m_startRef, m_spos, iterPos);
				m_navMesh->closestPointOnPolyBoundary(polys[npolys-1], m_epos, targetPos);

				static const float STEP_SIZE = 0.5f;
				static const float SLOP = 0.01f;

				m_nsmoothPath = 0;

				rcVcopy(&m_smoothPath[m_nsmoothPath*3], iterPos);
				m_nsmoothPath++;

				// Move towards target a small advancement at a time until target reached or
				// when ran out of memory to store the path.
				while (npolys && m_nsmoothPath < MAX_SMOOTH)
				{
					// Find location to steer towards.
					float steerPos[3];
					unsigned char steerPosFlag;
					dtPolyRef steerPosRef;

					if (!getSteerTarget(m_navMesh, iterPos, targetPos, SLOP,
						polys, npolys, steerPos, steerPosFlag, steerPosRef))
						break;

					bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) ? true : false;
					bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ? true : false;

					// Find movement delta.
					float delta[3], len;
					rcVsub(delta, steerPos, iterPos);
					len = sqrtf(rcVdot(delta,delta));
					// If the steer target is end of path or off-mesh link, do not move past the location.
					if ((endOfPath || offMeshConnection) && len < STEP_SIZE)
						len = 1;
					else
						len = STEP_SIZE / len;
					float moveTgt[3];
					rcVmad(moveTgt, iterPos, delta, len);

					// Move
					float result[3];
					int n = m_navMesh->moveAlongPathCorridor(iterPos, moveTgt, result, polys, npolys);
					float h = 0;
					m_navMesh->getPolyHeight(polys[n], result, &h);
					result[1] = h;
					// Shrink path corridor if advanced.
					if (n)
					{
						polys += n;
						npolys -= n;
					}
					// Update position.
					rcVcopy(iterPos, result);

					// Handle end of path and off-mesh links when close enough.
					if (endOfPath && inRange(iterPos, steerPos, SLOP, 1.0f))
					{
						// Reached end of path.
						rcVcopy(iterPos, targetPos);
						if (m_nsmoothPath < MAX_SMOOTH)
						{
							rcVcopy(&m_smoothPath[m_nsmoothPath*3], iterPos);
							m_nsmoothPath++;
						}
						break;
					}
					else if (offMeshConnection && inRange(iterPos, steerPos, SLOP, 1.0f))
					{
						// Reached off-mesh connection.
						float startPos[3], endPos[3];

						// Advance the path up to and over the off-mesh connection.
						dtPolyRef prevRef = 0, polyRef = polys[0];
						while (npolys && polyRef != steerPosRef)
						{
							prevRef = polyRef;
							polyRef = polys[0];
							polys++;
							npolys--;
						}

						// Handle the connection.
						if (m_navMesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, startPos, endPos))
						{
							if (m_nsmoothPath < MAX_SMOOTH)
							{
								rcVcopy(&m_smoothPath[m_nsmoothPath*3], startPos);
								m_nsmoothPath++;
								// Hack to make the dotted path not visible during off-mesh connection.
								if (m_nsmoothPath & 1)
								{
									rcVcopy(&m_smoothPath[m_nsmoothPath*3], startPos);
									m_nsmoothPath++;
								}
							}
							// Move position at the other side of the off-mesh link.
							rcVcopy(iterPos, endPos);
							float h;
							m_navMesh->getPolyHeight(polys[0], iterPos, &h);
							iterPos[1] = h;
						}
					}

					// Store results.
					if (m_nsmoothPath < MAX_SMOOTH)
					{
						rcVcopy(&m_smoothPath[m_nsmoothPath*3], iterPos);
						m_nsmoothPath++;
					}
				}
			}
		}
		else
		{
			m_npolys = 0;
			m_nsmoothPath = 0;
		}
	}
	else if (m_toolMode == TOOLMODE_PATHFIND_STRAIGHT)
	{
		if (m_sposSet && m_eposSet && m_startRef && m_endRef)
		{
#ifdef DUMP_REQS
			printf("ps  %f %f %f  %f %f %f  0x%x 0x%x\n",
				m_spos[0],m_spos[1],m_spos[2], m_epos[0],m_epos[1],m_epos[2],
				m_filter.includeFlags, m_filter.excludeFlags); 
			rcGetLog()->log(RC_LOG_PROGRESS, "ps  %f %f %f  %f %f %f  0x%x 0x%x\n",
				m_spos[0],m_spos[1],m_spos[2], m_epos[0],m_epos[1],m_epos[2],
				m_filter.includeFlags, m_filter.excludeFlags);
#endif
			m_npolys = m_navMesh->findPath(m_startRef, m_endRef, m_spos, m_epos, &m_filter, m_polys, MAX_POLYS);
			m_nstraightPath = 0;
			if (m_npolys)
			{
				m_nstraightPath = m_navMesh->findStraightPath(m_spos, m_epos, m_polys, m_npolys,
					m_straightPath, m_straightPathFlags,
					m_straightPathPolys, MAX_POLYS);
			}
		}
		else
		{
			m_npolys = 0;
			m_nstraightPath = 0;
		}
	}
	
	else if (m_toolMode == TOOLMODE_DISTANCE_TO_WALL)
	{
		m_distanceToWall = 0;
		if (m_sposSet && m_startRef)
		{
#ifdef DUMP_REQS
			printf("dw  %f %f %f  %f  0x%x 0x%x\n",
				m_spos[0],m_spos[1],m_spos[2], 100.0f,
				m_filter.includeFlags, m_filter.excludeFlags); 
			rcGetLog()->log(RC_LOG_PROGRESS, "dw  %f %f %f  %f  0x%x 0x%x\n",
				m_spos[0],m_spos[1],m_spos[2], 100.0f,
				m_filter.includeFlags, m_filter.excludeFlags); 
#endif
			m_distanceToWall = m_navMesh->findDistanceToWall(m_startRef, m_spos, 100.0f, &m_filter, m_hitPos, m_hitNormal);
		}
	}
	else if (m_toolMode == TOOLMODE_FIND_POLYS_AROUND)
	{
		if (m_sposSet && m_startRef && m_eposSet)
		{
			const float dx = m_epos[0] - m_spos[0];
			const float dz = m_epos[2] - m_spos[2];
			float dist = sqrtf(dx*dx + dz*dz);
#ifdef DUMP_REQS
			printf("fp  %f %f %f  %f  0x%x 0x%x\n",
				m_spos[0],m_spos[1],m_spos[2], dist,
				m_filter.includeFlags, m_filter.excludeFlags); 
			rcGetLog()->log(RC_LOG_PROGRESS, "fp  %f %f %f  %f  0x%x 0x%x\n",
				m_spos[0],m_spos[1],m_spos[2], dist,
				m_filter.includeFlags, m_filter.excludeFlags);
#endif
			m_npolys = m_navMesh->findPolysAround(m_startRef, m_spos, dist, &m_filter, m_polys, m_parent, 0, MAX_POLYS);
		}
	}
#endif
}



WowActor* GameWorld::AddNewActor( const noVec3& actorPos, SceneNode* root, int slot )
{
	int id = g_database.GetNewObjectID();
	std::string path;
	
	path = modelname[slot];	

	WowActor* newObj = new WowActor(path.c_str());
	newObj->SetID(id);
	if (newObj->GetID() == 0)
		newObj->SetType(OBJECT_Enemy | OBJECT_Character | OBJECT_Player);
	else 
		newObj->SetType(OBJECT_Enemy | OBJECT_Character);
	GameObjectManager::Get()->AddGameObject(newObj);
	
	newObj->SetScale(1.f);
	newObj->SetTranslation(actorPos);
	
	ActorController* pACtrl = new ActorController( newObj,
		this, actorPos, RandFloat() * noMath::TWO_PI, 
		vec3_zero, AICON.VehicleMass, AICON.MaxSteeringForce, AICON.MaxSpeed, AICON.MaxTurnRatePerSecond );

	pACtrl->setPathStart(actorPos);
	newObj->PushStateMachine(*pACtrl);

	if (newObj->GetID() == 0)	// Player
	{
		ActorInput* pInput = new WowActorInput(newObj);
		newObj->AttachInput(pInput);
	}

	if (m_pCellSpace) m_pCellSpace->AddEntity(pACtrl);

	int npc = OBJECT_Enemy | OBJECT_Character;

	float scale = 0.1f;
	float radius = RandInt((int)AICON.MinObstacleRadius, (int)AICON.MaxObstacleRadius);
	radius *= scale;
	pACtrl->SetBRadius(radius);

	pACtrl->SendMsgToMe(MSG_Idle);

	pACtrl->Steering()->ObstacleAvoidanceOff();

	if (newObj->GetType() ==  npc)	
	{
		//pACtrl->SendMsgToMe(MSG_Wander);
		pACtrl->SmoothingOn();	
		//pACtrl->SetMaxSpeed(30.f);
	}
	else 
	{
		pACtrl->SetBRadius(radius * 0.2f);		
		//pACtrl->SendMsgToMe(MSG_FindPath);
	}

	pACtrl->Steering()->FollowPathOff();
	//pACtrl->Steering()->FlockingOff();
	//pACtrl->Steering()->FlockingOn();	
	//pACtrl->Steering()->ArriveOn();  // 목적지까지 도착.
	//pACtrl->Steering()->FleeOn();	// 특정 지점으로부터 멀어짐.
	//pACtrl->Steering()->SeparationOn();
		
	//pACtrl->Steering()->ToggleSpacePartitioningOnOff();
	//pACtrl->Steering()->WanderOn();
	

	
	

	m_Vehicles.push_back(pACtrl);
	//pACtrl->sendModeChangeMessage();
	

	root->AddChild(newObj->GetNode());
	//root->UpdateTM();
	return newObj;
}

void GameWorld::Hide()
{
	for (size_t i=0; i < m_Vehicles.size(); ++i)
	{
		ActorController* pActrl = m_Vehicles.at(i);
		ActorController* pHunter;
		if (i+1 == m_Vehicles.size())
			pHunter = m_Vehicles.at(0);
		else 
			pHunter = m_Vehicles.at(i+1);
		pActrl->Steering()->HideOn(pHunter);
	}
}

void GameWorld::TestInterpose()
{
	ActorController* pActrl = m_Vehicles.at(0);
	ActorController* pTarget1 = m_Vehicles.at(1);
	ActorController* pTarget2 = m_Vehicles.at(2);

	
	pActrl->Steering()->InterposeOn(pTarget1, pTarget2);

}

void GameWorld::TestPursuit()
{
	if (m_Vehicles.empty()) return;
	
	ActorController* pPursuit = m_Vehicles.at(0);
	pPursuit->Steering()->ArriveOn();
	for (size_t i=1; i < m_Vehicles.size(); ++i)
	{	
		ActorController* pActrl = m_Vehicles.at(i);		
		pActrl->Steering()->PursuitOn(pPursuit);
	}
}

void GameWorld::TestSeek()
{
	if (m_Vehicles.empty()) return;

	//ActorController* pPursuit = m_Vehicles.at(0);	
	for (size_t i=0; i < m_Vehicles.size(); ++i)
	{	
		ActorController* pActrl = m_Vehicles.at(i);		
		pActrl->Steering()->SeekOn();
	}
}



void GameWorld::TestOffsetPursuit()
{
	if (m_Vehicles.empty()) return;

	
	ActorController* pPursuit = m_Vehicles.at(0);
	pPursuit->Steering()->ArriveOn();
	for (size_t i=1; i < m_Vehicles.size(); ++i)
	{	
		ActorController* pActrl = m_Vehicles.at(i);		
		pActrl->Steering()->OffsetPursuitOn(pPursuit, noVec3(0, 0, 15));
	}
}



void GameWorld::HandleFindPath()
{
	for(unsigned int i = 0; i < m_Vehicles.size(); ++i)
	{
		m_Vehicles[i]->sendFindNewPathMessage();
	}
}


void GameWorld::Draw()
{

	if(m_pCellSpace)
		m_pCellSpace->RenderCells(dd_, m_offSetVec.x, m_offSetVec.y);

	for (size_t i=0; i < m_Obstacles.size(); ++i)
	{
		ActorController* pActrl = m_Obstacles.at(i);

		pActrl->DrawDebug();
	}

	for (size_t i=0; i < m_Vehicles.size(); ++i)
	{
		ActorController* pActrl = m_Vehicles.at(i);

		pActrl->DrawDebug();
	}
}

void GameWorld::SetCrosshair(const noVec3& p)
{
	noVec3 ProposedPosition(p);

	//make sure it's not inside an obstacle
	std::vector<ActorController*>::iterator curOb = m_Obstacles.begin();
	for (; curOb != m_Obstacles.end(); ++curOb)
	{
		if (PointInCircle((*curOb)->Pos(), (*curOb)->BRadius(), ProposedPosition))
		{
			return;
		}

	}
	m_vCrosshair.x = (double)p.x;
	m_vCrosshair.y = (double)p.y;
}
