#include "stdafx.h"
#include "../GameApp.h"
#include "EngineCore/Util/GameLog.h"

#include "Recast.h"
#include "RecastLog.h"
#include "RecastTimer.h"
#include "RecastDebugDraw.h"
#include "RecastDump.h"

#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourDebugDraw.h"

#include "DebugDraw.h"
#include "NavDebugDraw.h"
#include "GameApp/SceneNode.h"
#include "ModelLib/M2Loader.h"
#include "../ObjLoader.h"
#include "../Common.h"


#include "GameWorld.h"
#include "AIMeshLoader.h"
#include "NavSourceMesh.h"

#include "../SceneNode.h"
#include "../Mesh.h"
#include "../Mesh_D3D11.h"
#include "../GameObjectManager.h"
#include "../TestObjects.h"

#include "../Terrain/AdaptiveModelDX11Render.h"

#include "NavTestActor.h"
#include "navigationmesh.h"
#include "navmap.h"

extern CAdaptiveModelDX11Render g_TerrainDX11Render;


#define MAX_PATHSLOT      128 // how many paths we can store
#define MAX_PATHPOLY      256 // max number of polygons in a path
#define MAX_PATHVERT      512 // most verts in a path 

// structure for storing output straight line paths
typedef struct
{
	float PosX[MAX_PATHVERT] ;
	float PosY[MAX_PATHVERT] ;
	float PosZ[MAX_PATHVERT] ;
	int MaxVertex ;
	int Target ;
}
PATHDATA ;


struct Tile
{
	inline Tile() : chf(0), solid(0), cset(0), pmesh(0), dmesh(0), buildTime(0) {}
	inline ~Tile() 
	{ 
		if(chf)
			delete chf; 
		if(cset)
			delete cset; 
		if(solid)
			delete solid; 
		if(pmesh)
			delete pmesh; 
		if(dmesh)
			delete dmesh; 
	}
	int x, y;
	rcCompactHeightfield* chf;
	rcHeightfield* solid;
	rcContourSet* cset;
	rcPolyMesh* pmesh;
	rcPolyMeshDetail* dmesh;
	int buildTime;
};

struct TileSet
{
	inline TileSet() : width(0), height(0), tiles(0) {}
	inline ~TileSet() 
	{ 
		if(tiles) 
			delete [] tiles; 
	}
	int width, height;
	float bmin[3], bmax[3];
	float cs, ch;
	Tile* tiles;
};

TileSet* m_tileSet;
int m_maxTiles;
int m_maxPolysPerTile;
float m_tileSize = 64.0f;

rcBuildTimes m_buildTimes;

unsigned int m_tileCol;
float m_tileBmin[3];
float m_tileBmax[3];
float m_tileBuildTime;
float m_tileMemUsage;
int m_tileTriCount;
bool m_keepInterResults = true;

bool keepInterResults;// = true;
float cellSize = 0.3f;// = 0.3f;
float cellHeight = 0.2f;// = 0.2f;
float agentHeight = 2.0f;// = 2.0f;
float agentRadius = 0.6f;// = 0.6f;
float agentMaxClimb = 2.0f;// = 0.2f;
float agentMaxSlope = 45;// = 45.0f;
float regionMinSize = 50.f;// = 50;
float regionMergeSize = 20.f;// = 20;
float edgeMaxLen = 12.0f;// = 12.0f;
float edgeMaxError = 1.3f; // = 1.3f;
float vertsPerPoly = 6.0f;// = 6.0f;
float detailSampleDist = 6.0f;// = 6.0f;
float detailSampleMaxError = 1.0f;// = 1.0f;

unsigned char* m_triflags;
rcHeightfield* m_solid;
rcCompactHeightfield* m_chf;
rcContourSet* m_cset;
rcPolyMesh* m_pmesh;
rcConfig m_cfg;	
rcPolyMeshDetail* m_dmesh;


PATHDATA m_PathStore[MAX_PATHSLOT] ; 

NavDebugDraw* ddMain;
NavSourceMesh* geom;

GameWorld*	gGameWorld;

rcLog mDbgLog;
static RenderData grid_rd;





unsigned char m_navMeshDrawFlags = DU_DRAWNAVMESH_CLOSEDLIST|DU_DRAWNAVMESH_OFFMESHCONS;

inline unsigned int nextPow2(unsigned int v)
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

inline unsigned int ilog2(unsigned int v)
{
	unsigned int r;
	unsigned int shift;
	r = (v > 0xffff) << 4; v >>= r;
	shift = (v > 0xff) << 3; v >>= shift; r |= shift;
	shift = (v > 0xf) << 2; v >>= shift; r |= shift;
	shift = (v > 0x3) << 1; v >>= shift; r |= shift;
	r |= (v >> 1);
	return r;
}

noVec3 startPos, endPos;
bool bPathFind = false, startFind = true;

//------------------------------------------------------------------------------------
static void getPolyCenter(dtNavMesh* navMesh, dtPolyRef ref, float* center)
{
	const dtPoly* p = navMesh->getPolyByRef(ref);
	if (!p) return;
	const float* verts = navMesh->getPolyVertsByRef(ref);
	center[0] = 0;
	center[1] = 0;
	center[2] = 0;
	for (int i = 0; i < (int)p->vertCount; ++i)
	{
		const float* v = &verts[p->verts[i]*3];
		center[0] += v[0];
		center[1] += v[1];
		center[2] += v[2];
	}
	const float s = 1.0f / p->vertCount;
	center[0] *= s;
	center[1] *= s;
	center[2] *= s;
}



int App::FindPath(float* pStartPos, float* pEndPos, int nPathSlot, int nTarget) 
{	
	float pExtents[3]={32.0f, 32.0f, 32.0f} ; // size of box around start/end points to look for nav polygons
	dtPolyRef StartPoly ;
	float StartNearest[3] ;
	dtPolyRef EndPoly ;
	float EndNearest[3] ;
	dtPolyRef PolyPath[MAX_PATHPOLY] ;
	int nPathCount=0 ;
	float StraightPath[MAX_PATHVERT*3] ;
	int nVertCount=0 ;


	// setup the filter
	dtQueryFilter Filter;
	Filter.includeFlags = 0xFFFF ;
	Filter.excludeFlags = 0 ;
	m_navMesh->setAreaCost(SAMPLE_POLYAREA_GROUND, 1.0f) ;

	// find the start polygon
	StartPoly = m_navMesh->findNearestPoly(pStartPos, pExtents, &Filter, StartNearest) ;	

	// find the end polygon
	EndPoly=m_navMesh->findNearestPoly(pEndPos, pExtents, &Filter, EndNearest) ;	

	nPathCount=m_navMesh->findPath(StartPoly, EndPoly, StartNearest, EndNearest, &Filter, PolyPath, MAX_PATHPOLY) ;
	
	if(nPathCount==0) 
	{
		LOG_ERR << "Path Count = 0";
		return -4 ; // couldn't find a path
	}

	nVertCount=m_navMesh->findStraightPath(StartNearest, EndNearest, PolyPath, nPathCount, StraightPath, NULL, NULL, MAX_PATHVERT) ;	
	if(nVertCount==0)
	{
		LOG_ERR << "Vert Count = 0";
		return -6 ; // couldn't find a path
	}

	int nIndex=0 ;
	for (int i=0; i < nPathCount; ++i)
	{
		float center[3];
		getPolyCenter(m_navMesh, PolyPath[i], center);
		StraightPath[nIndex++] = center[0];
		StraightPath[nIndex++] = center[1];
		StraightPath[nIndex++] = center[2];
	}

	// At this point we have our path.  Copy it to the path store
	nIndex=0 ;
	for(int nVert=0 ; nVert<nPathCount ; nVert++)
	{
		m_PathStore[nPathSlot].PosX[nVert]=StraightPath[nIndex++] ;
		m_PathStore[nPathSlot].PosY[nVert]=StraightPath[nIndex++] ;
		m_PathStore[nPathSlot].PosZ[nVert]=StraightPath[nIndex++] ;

		char dbBuff[1024];
		sprintf(dbBuff, "Path Vert %i, %f %f %f", nVert, m_PathStore[nPathSlot].PosX[nVert], m_PathStore[nPathSlot].PosY[nVert], m_PathStore[nPathSlot].PosZ[nVert]) ;
		LOG_INFO << dbBuff;
	}
	m_PathStore[nPathSlot].MaxVertex=nVertCount ;
	m_PathStore[nPathSlot].Target=nTarget ;

	return nVertCount ;

}


bool					gShowNaviMesh = false;
bool					gAutomatedMode = false;
extern NavigationMesh			NaviMesh;		/* our navigation mesh */
Mesh_D3D11* gNavMap;
Mesh_D3D11* gPlayPen;
Mesh_D3D11* gControl;
Mesh_D3D11* gPath;

RenderData navrd;
RenderData pathrd;
RenderData playpenrd;
RenderData controlrd;

NavActor					PathObject;		/* red cone that does the pathfinding */
NavActor					ControlObject;	/* blue cone controlled by the player */
NavActor					PlayPen;		/* the playpen host environment */


void App::CreateNavScene()
{
	NaviMesh.Clear();

	for(int i = 0; i < map_totalpolys; ++i)
	{
		const noVec3	&vertA = map_points[map_polys[i][0]];
		const noVec3	&vertB = map_points[map_polys[i][1]];
		const noVec3	&vertC = map_points[map_polys[i][2]];

		
		if((vertA != vertB) && (vertB != vertC) && (vertC != vertA))
		{
			NaviMesh.AddCell(vertA, vertB, vertC);
		}
	}

	NaviMesh.LinkCells();

	uint16* indices = new uint16[map_totalpolys * 3];
	for (int i = 0; i < map_totalpolys * 3; i+=3)
	{
		indices[i] = map_polys[i/3][0];
		indices[i+1] = map_polys[i/3][1];
		indices[i+2] = map_polys[i/3][2];
	}

	gNavMap = (Mesh_D3D11*)makeMesh(navrd, map_points, map_totalpoints, indices, map_totalpolys * 3);

	uint16* indices2 = new uint16[playpen_totalpolys * 3];
	for (int i = 0; i < playpen_totalpolys * 3; i+=3)
	{
		indices2[i] = playpen_polys[i/3][0];
		indices2[i+1] = playpen_polys[i/3][1];
		indices2[i+2] = playpen_polys[i/3][2];
	}

	gPlayPen = (Mesh_D3D11*)makeMesh(playpenrd, playpen_points, playpen_totalpoints, indices2, playpen_totalpolys * 3);

	uint16* indices3 = new uint16[cone_totalpolys * 3];
	for (int i = 0; i < cone_totalpolys * 3; i+=3)
	{
		indices3[i] = cone_polys[i/3][0];
		indices3[i+1] = cone_polys[i/3][1];
		indices3[i+2] = cone_polys[i/3][2];
	}

	noVec4 col = noVec4(1.f, 1.f ,1.0f, 1.f);
	gControl = (Mesh_D3D11*)makeMesh(controlrd, cone_points, cone_totalpoints, indices3, cone_totalpolys * 3, col);
	 col = noVec4(1.f, 0.f ,1.0f, 1.f);
	gPath = (Mesh_D3D11*)makeMesh(pathrd, cone_points, cone_totalpoints, indices3, cone_totalpolys * 3, col);

	ControlObject.Create(&NaviMesh, noVec3(0.0f, 0.0f, 0.0f), 0);
	PathObject.Create(&NaviMesh, noVec3(0.0f, 0.0f, 0.0f), 0);
	

	//m_WorldRoot->AddChild(gNavMap);
	
	//rcSetLog(&mDbgLog);
	//objLoader.Create(device, L"Data/dungeon.obj");	
	geom = new NavSourceMesh;

	const char* filename[] = {
		"Data/City.obj",
		"Data/dungeon.obj",
		"Data/NavMesh.obj"
	};
	
	
	noVec3 corner(-100, 0, -100);
	noVec3 sideA(200, 0, 0);
	noVec3 sideB(0, 0, 200);

	//Mesh* pMesh = makeGrid(grid_rd, corner, sideA, sideB, 10, 10, 0, true, false);
	geom->loadMesh(filename[2]);		
	//geom->load(pMesh);
	//GetSceneRoot()->AddChild(pMesh);	

	gGameWorld = new GameWorld;

	if (geom)
	{
		m_WorldRoot->AddChild(const_cast<rcMeshLoaderObj*>(geom->getMesh()));

		const float* bmin = geom->getMeshBoundsMin();
		const float* bmax = geom->getMeshBoundsMax();

		int gw = 0, gh = 0;
		rcCalcGridSize(bmin, bmax, cellSize, &gw, &gh);
		const int ts = (int)m_tileSize;
		const int tw = (gw + ts-1) / ts;
		const int th = (gh + ts-1) / ts;
		int tileBits = rcMin((int)ilog2(nextPow2(tw*th)), 14);
		if (tileBits > 14) tileBits = 14;
		int polyBits = 22 - tileBits;
		m_maxTiles = 1 << tileBits;
		m_maxPolysPerTile = 1 << polyBits;
	}


	ddMain = new NavDebugDraw();
	ddMain->setOffset(0.15f);
	ddMain->CreateVertexBuffer(NULL, geom->getMesh()->getVertCount(), sizeof(DebugVertex));
	//ddMain->CreateIndexBuffer(geom->getMesh()->getTris(), geom->getMesh()->getTriCount() * 3);
	ddMain->SetupShader();

	geom->getMeshObject()->objLoader.rd.cull = renderer->addRasterizerState(CULL_FRONT, SOLID);
	
}

bool App::BuildNavMesh()
{
	if(m_navMesh)
		delete m_navMesh;

	m_navMesh = new dtNavMesh;

	dtNavMeshParams params;
	rcVcopy(params.orig, geom->getMeshBoundsMin());
	params.tileWidth = m_tileSize*cellSize;
	params.tileHeight = m_tileSize*cellSize;
	params.maxTiles = m_maxTiles;
	params.maxPolys = m_maxPolysPerTile;
	params.maxNodes = 2048;

	if (!m_navMesh->init(&params))
	{
		return false;
	}

	buildAllTiles();

	int cnt = mDbgLog.getMessageCount();
	if(cnt > 0)
	{
		for (int i = 0; i < cnt; ++i)
		{
			LOG_ERR << mDbgLog.getMessageText(i);
		}
	}
	gGameWorld->Init(geom);

	return true;
}

void App::RenderNavScene()
{
	if (!geom) return;

	mat4 oldMat = g_World;
	
#if 0
	if (geom->getMeshObject())
	{
		const noTransform& tm = geom->getMeshObject()->objLoader.worldTM_;
		noMat4 worldTM(tm.rotate_ * tm.scale_, tm.trans_);
	
		worldTM.TransposeSelf();
		g_World = ToMat4(worldTM);
		drawModel(geom->getMeshObject()->objLoader.rd);
		g_World = oldMat;
	}
#endif
	
	if (!gAutomatedMode)
	{	
		if(GetKeyDown(VK_UP))
			ControlObject.SetMovementZ(2.f);
		if(GetKeyDown(VK_DOWN))
			ControlObject.SetMovementZ(-2.f);
		if(GetKeyDown(VK_LEFT))
			ControlObject.SetMovementX(2.f);
		if(GetKeyDown(VK_RIGHT))
			ControlObject.SetMovementX(-2.f);	
	}
	else 
	{
		int forceX = (rand() % 20) - 10;
		int forceZ = (rand() % 20) - 10;

		if(ControlObject.Movement().x < 5.0f)
		{
			int forceX = (rand() % 20) - 10;
			int forceZ = (rand() % 20) - 10;

			ControlObject.AddMovement(noVec3(forceX, 0, forceZ));
		}

		if(!PathObject.PathIsActive())
		{
			PathObject.GotoRandomLocation();
		}
	}
	
	float fdelta = 1 / 60.f;
	PathObject.Update(fdelta);
	ControlObject.Update(fdelta);

	
	//drawModel(navrd);	
	//drawModel(playpenrd);
	
	oldMat = g_World;

	noMat4 world;
	world.Translation(PathObject.Position());
	world.TransposeSelf();
	g_World = ToMat4(world);

	//drawModel(pathrd);
	g_World = oldMat;

	
	world;
	world.Translation(ControlObject.Position());
	world.TransposeSelf();
	g_World = ToMat4(world);
	
	//drawModel(controlrd);
	g_World = oldMat;

	
	if(GetKeyDown('r') || GetKeyDown('R'))
	{
		gAutomatedMode = !gAutomatedMode;

	}

	if(GetKeyDown(VK_SPACE))
	{
		if(!gAutomatedMode) 
		{
			PathObject.GotoLocation(ControlObject.Position(), ControlObject.CurrentCell());

			WowActor* pActor = (WowActor*)g_database.Find(0);
			if (pActor)
			{	
				ActorController* pActrl = (ActorController*)pActor->GetStateMachine();

				pActrl->PathFinding()->GotoLocation(ControlObject.Position(), 
					ControlObject.CurrentCell());
			}
		}
	}

	if(GetKeyDown('t') || GetKeyDown('T'))
	{
		gShowNaviMesh = !gShowNaviMesh;

	}

	//drawModel(grid_rd);	
	if(GetKeyDown('2'))
	{
		if (!startFind) 
		{				
			endPos = PickUtil::pickDatas[0].vert;
			startFind = true;
			bPathFind = true;
		}
	}

	if(GetKeyDown('1'))
	{
		
		startPos = PickUtil::pickDatas[0].vert;
		startFind = false;
		bPathFind = false;
	}

	if(GetKeyDown('J'))
	{
		static bool bfirst = false;
		if (!bfirst) 
		{
			BuildNavMesh();			
			bfirst = true;
		}

		if (bPathFind)
		{
			FindPath(startPos.ToFloatPtr(), endPos.ToFloatPtr(), 0, 0);					
		}		

		RenderTiles();			
	}



	gGameWorld->Draw();
}

void App::RenderTiles()
{
	
	duDebugDrawNavMesh(ddMain, *m_navMesh, m_navMeshDrawFlags);

	if (bPathFind)
	{
		int nPathSlot = 0;
		int nVertCount=m_PathStore[nPathSlot].MaxVertex ;
		int col = duRGBA(255,0,0,64);
		ddMain->begin(DU_DRAW_LINES_STRIP);

		for(int nVert=0 ; nVert<nVertCount ; nVert++)
		{
			ddMain->vertex(m_PathStore[nPathSlot].PosX[nVert], m_PathStore[nPathSlot].PosY[nVert]+8.0f, m_PathStore[nPathSlot].PosZ[nVert], col) ;		
		}

		ddMain->end();
	}
}

void App::buildAllTiles()
{
	if (!geom) return;
	if (!m_navMesh) return;

	const float* bmin = geom->getMeshBoundsMin();
	const float* bmax = geom->getMeshBoundsMax();
	int gw = 0, gh = 0;
	rcCalcGridSize(bmin, bmax, cellSize, &gw, &gh);
	const int ts = (int)m_tileSize;
	const int tw = (gw + ts-1) / ts;
	const int th = (gh + ts-1) / ts;
	const float tcs = m_tileSize*cellSize;


	// Calculate the number of tiles in the output and initialize tiles.
	if(m_tileSet)
		delete m_tileSet;
	m_tileSet = new TileSet();

	if (!m_tileSet)
	{
		assert(false);
		LOG_ERR << "Failed ";
		return;
	}

	rcVcopy(m_tileSet->bmin, bmin);
	rcVcopy(m_tileSet->bmax, bmax);
	m_tileSet->cs = cellSize;
	m_tileSet->ch = cellHeight;
	m_tileSet->width = tw;
	m_tileSet->height = th;
	m_tileSet->tiles = new Tile[m_tileSet->height * m_tileSet->width];
	if (!m_tileSet->tiles)
	{
		assert(false);
		LOG_ERR << "Failed ";
		return;
	}

	for (int y = 0; y < th; ++y)
	{
		for (int x = 0; x < tw; ++x)
		{
			Tile& tile = m_tileSet->tiles[x + y*m_tileSet->width];
			tile.x = x;
			tile.y = y;

			m_tileBmin[0] = bmin[0] + x*tcs;
			m_tileBmin[1] = bmin[1];
			m_tileBmin[2] = bmin[2] + y*tcs;

			m_tileBmax[0] = bmin[0] + (x+1)*tcs;
			m_tileBmax[1] = bmax[1];
			m_tileBmax[2] = bmin[2] + (y+1)*tcs;

			int dataSize = 0;
			unsigned char* data = buildTileMesh(x, y, m_tileBmin, m_tileBmax, dataSize); 
			if(m_chf)
			{
				tile.chf = m_chf;
				m_chf = 0;
			}
			if(m_solid)
			{
				tile.solid = m_solid;
				m_solid = 0;
			}
			if(m_cset)
			{
				tile.cset = m_cset;
				m_cset = 0;
			}
			if(m_pmesh)
			{
				tile.pmesh = m_pmesh;
				m_pmesh = 0;
			}
			if(m_dmesh)
			{
				tile.dmesh = m_dmesh;
				m_dmesh = 0;
			}
			tile.buildTime = m_tileBuildTime;

			if (data)
			{
				// Remove any previous data (navmesh owns and deletes the data).
				m_navMesh->removeTile(m_navMesh->getTileRefAt(x,y),0,0);
				// Let the navmesh own the data.
				if (!m_navMesh->addTile(data,dataSize,true))
					delete [] data;
			}
		}
	}

}

void App::cleanupNavDatas()
{
	if(m_triflags)
	{
		delete [] m_triflags;
		m_triflags = 0;
	}
	if(m_pmesh)
	{
		delete m_pmesh;
		m_pmesh = 0;
	}
	if(m_solid)
	{
		delete m_solid;
		m_solid = 0;
	}
	if(m_chf)
	{
		delete m_chf;
		m_chf = 0;
	}
	if(m_cset)
	{
		delete m_cset;
		m_cset = 0;
	}
	if(m_dmesh)
	{
		delete m_dmesh;
		m_dmesh = 0;
	}
}


unsigned char* App::buildTileMesh(const int tx, const int ty, const float* bmin, const float* bmax, int& dataSize)
{
	if (!geom || !geom->getMesh() || !geom->getChunkyMesh())
	{
		
		LOG_ERR << "buildNavigation: Input mesh is not specified.";
		return 0;
	}

	cleanupNavDatas();

	const float* verts = geom->getMesh()->getVerts();
	const int nverts = geom->getMesh()->getVertCount();
	const int ntris = geom->getMesh()->getTriCount();
	const rcChunkyTriMesh* chunkyMesh = geom->getChunkyMesh();

	// Init build configuration from GUI
	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = cellSize;
	m_cfg.ch = cellHeight;
	m_cfg.walkableSlopeAngle = agentMaxSlope;
	m_cfg.walkableHeight = (int)ceilf(agentHeight / m_cfg.ch);
	m_cfg.walkableClimb = (int)floorf(agentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = (int)(edgeMaxLen / cellSize);
	m_cfg.maxSimplificationError = edgeMaxError;
	m_cfg.minRegionSize = (int)rcSqr(regionMinSize);
	m_cfg.mergeRegionSize = (int)rcSqr(regionMergeSize);
	m_cfg.maxVertsPerPoly = (int)vertsPerPoly;
	m_cfg.tileSize = (int)m_tileSize;
	m_cfg.borderSize = m_cfg.walkableRadius + 3; // Reserve enough padding.
	m_cfg.width = m_cfg.tileSize + m_cfg.borderSize*2;
	m_cfg.height = m_cfg.tileSize + m_cfg.borderSize*2;
	m_cfg.detailSampleDist = detailSampleDist < 0.9f ? 0 : cellSize * detailSampleDist;
	m_cfg.detailSampleMaxError = cellHeight * detailSampleMaxError;

	rcVcopy(m_cfg.bmin, bmin);
	rcVcopy(m_cfg.bmax, bmax);
	m_cfg.bmin[0] -= m_cfg.borderSize*m_cfg.cs;
	m_cfg.bmin[2] -= m_cfg.borderSize*m_cfg.cs;
	m_cfg.bmax[0] += m_cfg.borderSize*m_cfg.cs;
	m_cfg.bmax[2] += m_cfg.borderSize*m_cfg.cs;

	// Reset build times gathering.
	memset(&m_buildTimes, 0, sizeof(m_buildTimes));
	rcSetBuildTimes(&m_buildTimes);

	// Start the build process.	
	rcTimeVal totStartTime = rcGetPerformanceTimer();

	if (rcGetLog())
	{
		rcGetLog()->log(RC_LOG_PROGRESS, "Building navigation:");
		rcGetLog()->log(RC_LOG_PROGRESS, " - %d x %d cells", m_cfg.width, m_cfg.height);
		rcGetLog()->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", nverts/1000.0f, ntris/1000.0f);
	}

	// Allocate voxel heighfield where we rasterize our input data to.
	m_solid = new rcHeightfield;
	if (!m_solid)
	{
		if (rcGetLog())
			rcGetLog()->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
		return 0;
	}
	if (!rcCreateHeightfield(*m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
	{
		if (rcGetLog())
			rcGetLog()->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
		return 0;
	}

	// Allocate array that can hold triangle flags.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	m_triflags = new unsigned char[chunkyMesh->maxTrisPerChunk];
	if (!m_triflags)
	{
		if (rcGetLog())
			rcGetLog()->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'triangleFlags' (%d).", chunkyMesh->maxTrisPerChunk);
		return 0;
	}


	float tbmin[2], tbmax[2];
	tbmin[0] = m_cfg.bmin[0];
	tbmin[1] = m_cfg.bmin[2];
	tbmax[0] = m_cfg.bmax[0];
	tbmax[1] = m_cfg.bmax[2];
	int cid[512];// TODO: Make grow when returning too many items.
	const int ncid = rcGetChunksInRect(chunkyMesh, tbmin, tbmax, cid, 512);
	if (!ncid)
		return 0;

	m_tileTriCount = 0;

	for (int i = 0; i < ncid; ++i)
	{
		const rcChunkyTriMeshNode& node = chunkyMesh->nodes[cid[i]];
		const int* tris = &chunkyMesh->tris[node.i*3];
		const int ntris = node.n;

		m_tileTriCount += ntris;

		memset(m_triflags, 0, ntris*sizeof(unsigned char));
		rcMarkWalkableTriangles(m_cfg.walkableSlopeAngle,
			verts, nverts, tris, ntris, m_triflags);

		rcRasterizeTriangles(verts, nverts, tris, m_triflags, ntris, *m_solid, m_cfg.walkableClimb);
	}

	

	if (!m_keepInterResults)
	{
		delete [] m_triflags;
		m_triflags = 0;
	}

	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	//rcFilterLowHangingWalkableObstacles(m_cfg.walkableClimb, *m_solid);
	//rcFilterLedgeSpans(m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	//rcFilterWalkableLowHeightSpans(m_cfg.walkableHeight, *m_solid);

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	m_chf = new rcCompactHeightfield;
	if (!m_chf)
	{
		if (rcGetLog())
			rcGetLog()->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
		return 0;
	}
	if (!rcBuildCompactHeightfield(m_cfg.walkableHeight, m_cfg.walkableClimb, RC_WALKABLE, *m_solid, *m_chf))
	{
		if (rcGetLog())
			rcGetLog()->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
		return 0;
	}

	if (!m_keepInterResults)
	{
		delete m_solid;
		m_solid = 0;
	}

	// Erode the walkable area by agent radius.
	if (!rcErodeArea(RC_WALKABLE_AREA, m_cfg.walkableRadius, *m_chf))
	{
		if (rcGetLog())
			rcGetLog()->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
		return false;
	}

	// (Optional) Mark areas.
	const ConvexVolume* vols = geom->getConvexVolumes();
	for (int i  = 0; i < geom->getConvexVolumeCount(); ++i)
		rcMarkConvexPolyArea(vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);

	// Prepare for region partitioning, by calculating distance field along the walkable surface.
	if (!rcBuildDistanceField(*m_chf))
	{
		if (rcGetLog())
			rcGetLog()->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
		return 0;
	}

	// Partition the walkable surface into simple regions without holes.
	if (!rcBuildRegions(*m_chf, m_cfg.borderSize, m_cfg.minRegionSize, m_cfg.mergeRegionSize))
	{
		if (rcGetLog())
			rcGetLog()->log(RC_LOG_ERROR, "buildNavigation: Could not build regions.");
		return 0;
	}

	// Create contours.
	m_cset = new rcContourSet;
	if (!m_cset)
	{
		if (rcGetLog())
			rcGetLog()->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
		return 0;
	}
	if (!rcBuildContours(*m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		if (rcGetLog())
			rcGetLog()->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
		return 0;
	}

	if (m_cset->nconts == 0)
	{
		return 0;
	}

	// Build polygon navmesh from the contours.
	m_pmesh = new rcPolyMesh;
	if (!m_pmesh)
	{
		if (rcGetLog())
			rcGetLog()->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
		return 0;
	}
	if (!rcBuildPolyMesh(*m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		if (rcGetLog())
			rcGetLog()->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
		return 0;
	}

	// Build detail mesh.
	m_dmesh = new rcPolyMeshDetail;
	if (!m_dmesh)
	{
		if (rcGetLog())
			rcGetLog()->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'dmesh'.");
		return 0;
	}

	if (!rcBuildPolyMeshDetail(*m_pmesh, *m_chf,
		m_cfg.detailSampleDist, m_cfg.detailSampleMaxError,
		*m_dmesh))
	{
		if (rcGetLog())
			rcGetLog()->log(RC_LOG_ERROR, "buildNavigation: Could build polymesh detail.");
		return 0;
	}

	if (!m_keepInterResults)
	{
		delete m_chf;
		m_chf = 0;
		delete m_cset;
		m_cset = 0;
	}

	unsigned char* navData = 0;
	int navDataSize = 0;
	if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		// Remove padding from the polymesh data. TODO: Remove this odditity.
		for (int i = 0; i < m_pmesh->nverts; ++i)
		{
			unsigned short* v = &m_pmesh->verts[i*3];
			v[0] -= (unsigned short)m_cfg.borderSize;
			v[2] -= (unsigned short)m_cfg.borderSize;
		}

		if (m_pmesh->nverts >= 0xffff)
		{
			// The vertex indices are ushorts, and cannot point to more than 0xffff vertices.
			if (rcGetLog())
				rcGetLog()->log(RC_LOG_ERROR, "Too many vertices per tile %d (max: %d).", m_pmesh->nverts, 0xffff);
			return false;
		}

		// Update poly flags from areas.
		for (int i = 0; i < m_pmesh->npolys; ++i)
		{
			if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
				m_pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;

			if (m_pmesh->areas[i] == SAMPLE_POLYAREA_GROUND ||
				m_pmesh->areas[i] == SAMPLE_POLYAREA_GRASS ||
				m_pmesh->areas[i] == SAMPLE_POLYAREA_ROAD)
			{
				m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
			}
			else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_WATER)
			{
				m_pmesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
			}
			else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_DOOR)
			{
				m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
			}
		}

		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = m_pmesh->verts;
		params.vertCount = m_pmesh->nverts;
		params.polys = m_pmesh->polys;
		params.polyAreas = m_pmesh->areas;
		params.polyFlags = m_pmesh->flags;
		params.polyCount = m_pmesh->npolys;
		params.nvp = m_pmesh->nvp;
		params.detailMeshes = m_dmesh->meshes;
		params.detailVerts = m_dmesh->verts;
		params.detailVertsCount = m_dmesh->nverts;
		params.detailTris = m_dmesh->tris;
		params.detailTriCount = m_dmesh->ntris;
		params.offMeshConVerts = geom->getOffMeshConnectionVerts();
		params.offMeshConRad = geom->getOffMeshConnectionRads();
		params.offMeshConDir = geom->getOffMeshConnectionDirs();
		params.offMeshConAreas = geom->getOffMeshConnectionAreas();
		params.offMeshConFlags = geom->getOffMeshConnectionFlags();
		params.offMeshConCount = geom->getOffMeshConnectionCount();
		params.walkableHeight = agentHeight;
		params.walkableRadius = agentRadius;
		params.walkableClimb = agentMaxClimb;
		params.tileX = tx;
		params.tileY = ty;
		rcVcopy(params.bmin, bmin);
		rcVcopy(params.bmax, bmax);
		params.cs = m_cfg.cs;
		params.ch = m_cfg.ch;
		params.tileSize = m_cfg.tileSize;

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			if (rcGetLog())
				rcGetLog()->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
			return 0;
		}
	}
	m_tileMemUsage = navDataSize/1024.0f;

	rcTimeVal totEndTime = rcGetPerformanceTimer();

	// Show performance stats.
	if (rcGetLog())
	{
		const float pc = 100.0f / rcGetDeltaTimeUsec(totStartTime, totEndTime);

		rcGetLog()->log(RC_LOG_PROGRESS, "Rasterize: %.1fms (%.1f%%)", m_buildTimes.rasterizeTriangles/1000.0f, m_buildTimes.rasterizeTriangles*pc);

		rcGetLog()->log(RC_LOG_PROGRESS, "Build Compact: %.1fms (%.1f%%)", m_buildTimes.buildCompact/1000.0f, m_buildTimes.buildCompact*pc);

		rcGetLog()->log(RC_LOG_PROGRESS, "Filter Border: %.1fms (%.1f%%)", m_buildTimes.filterBorder/1000.0f, m_buildTimes.filterBorder*pc);
		rcGetLog()->log(RC_LOG_PROGRESS, "Filter Walkable: %.1fms (%.1f%%)", m_buildTimes.filterWalkable/1000.0f, m_buildTimes.filterWalkable*pc);
		rcGetLog()->log(RC_LOG_PROGRESS, "Filter Reachable: %.1fms (%.1f%%)", m_buildTimes.filterMarkReachable/1000.0f, m_buildTimes.filterMarkReachable*pc);

		rcGetLog()->log(RC_LOG_PROGRESS, "Erode walkable area: %.1fms (%.1f%%)", m_buildTimes.erodeArea/1000.0f, m_buildTimes.erodeArea*pc);

		rcGetLog()->log(RC_LOG_PROGRESS, "Build Distancefield: %.1fms (%.1f%%)", m_buildTimes.buildDistanceField/1000.0f, m_buildTimes.buildDistanceField*pc);
		rcGetLog()->log(RC_LOG_PROGRESS, "  - distance: %.1fms (%.1f%%)", m_buildTimes.buildDistanceFieldDist/1000.0f, m_buildTimes.buildDistanceFieldDist*pc);
		rcGetLog()->log(RC_LOG_PROGRESS, "  - blur: %.1fms (%.1f%%)", m_buildTimes.buildDistanceFieldBlur/1000.0f, m_buildTimes.buildDistanceFieldBlur*pc);

		rcGetLog()->log(RC_LOG_PROGRESS, "Build Regions: %.1fms (%.1f%%)", m_buildTimes.buildRegions/1000.0f, m_buildTimes.buildRegions*pc);
		rcGetLog()->log(RC_LOG_PROGRESS, "  - watershed: %.1fms (%.1f%%)", m_buildTimes.buildRegionsReg/1000.0f, m_buildTimes.buildRegionsReg*pc);
		rcGetLog()->log(RC_LOG_PROGRESS, "    - expand: %.1fms (%.1f%%)", m_buildTimes.buildRegionsExp/1000.0f, m_buildTimes.buildRegionsExp*pc);
		rcGetLog()->log(RC_LOG_PROGRESS, "    - find catchment basins: %.1fms (%.1f%%)", m_buildTimes.buildRegionsFlood/1000.0f, m_buildTimes.buildRegionsFlood*pc);
		rcGetLog()->log(RC_LOG_PROGRESS, "  - filter: %.1fms (%.1f%%)", m_buildTimes.buildRegionsFilter/1000.0f, m_buildTimes.buildRegionsFilter*pc);

		rcGetLog()->log(RC_LOG_PROGRESS, "Build Contours: %.1fms (%.1f%%)", m_buildTimes.buildContours/1000.0f, m_buildTimes.buildContours*pc);
		rcGetLog()->log(RC_LOG_PROGRESS, "  - trace: %.1fms (%.1f%%)", m_buildTimes.buildContoursTrace/1000.0f, m_buildTimes.buildContoursTrace*pc);
		rcGetLog()->log(RC_LOG_PROGRESS, "  - simplify: %.1fms (%.1f%%)", m_buildTimes.buildContoursSimplify/1000.0f, m_buildTimes.buildContoursSimplify*pc);

		rcGetLog()->log(RC_LOG_PROGRESS, "Build Polymesh: %.1fms (%.1f%%)", m_buildTimes.buildPolymesh/1000.0f, m_buildTimes.buildPolymesh*pc);
		rcGetLog()->log(RC_LOG_PROGRESS, "Build Polymesh Detail: %.1fms (%.1f%%)", m_buildTimes.buildDetailMesh/1000.0f, m_buildTimes.buildDetailMesh*pc);
		rcGetLog()->log(RC_LOG_PROGRESS, "Merge Polymeshes: %.1fms (%.1f%%)", m_buildTimes.mergePolyMesh/1000.0f, m_buildTimes.mergePolyMesh*pc);
		rcGetLog()->log(RC_LOG_PROGRESS, "Merge Polymesh Details: %.1fms (%.1f%%)", m_buildTimes.mergePolyMeshDetail/1000.0f, m_buildTimes.mergePolyMeshDetail*pc);


		rcGetLog()->log(RC_LOG_PROGRESS, "Build Polymesh: %.1fms (%.1f%%)", m_buildTimes.buildPolymesh/1000.0f, m_buildTimes.buildPolymesh*pc);

		rcGetLog()->log(RC_LOG_PROGRESS, "Polymesh: Verts:%d  Polys:%d", m_pmesh->nverts, m_pmesh->npolys);

		rcGetLog()->log(RC_LOG_PROGRESS, "TOTAL: %.1fms", rcGetDeltaTimeUsec(totStartTime, totEndTime)/1000.0f);
	}

	m_tileBuildTime = rcGetDeltaTimeUsec(totStartTime, totEndTime)/1000.0f;

	
	dataSize = navDataSize;
	return navData;
}

