#include "stdafx.h"
#include "DebugDraw.h"
#include "NavDebugDraw.h"
#include "Transform2DFuncs.h"
#include "AIPath.h"

//------------------------------------------------------------------------------------
// will have to create new methods based on this but working with the detour data
// so that we can generate valid paths randomly and efficiently - dont want to worry
// about selecting geom with raycasting and getting something outside the navmesh
std::list<noVec3> AIPath::CreateRandomPath(int   NumWaypoints,
	double MinX,
	double MinY,
	double MaxX,
	double MaxY)
{
	m_WayPoints.clear();

	double midX = (MaxX+MinX)/2.0;
	double midY = (MaxY+MinY)/2.0;

	double smaller = Min(midX, midY);

	double spacing = noMath::TWO_PI/(double)NumWaypoints;

	for (int i=0; i<NumWaypoints; ++i)
	{
		double RadialDist = RandInRange(smaller*0.2f, smaller);

		noVec3 temp(RadialDist, 0.0f, 0.0f);

		//Vec2DRotateAroundOrigin(temp, i*spacing);
		temp.x += midX; temp.y += midY;

		m_WayPoints.push_back(temp);

	}

	curWaypoint = m_WayPoints.begin();

	return m_WayPoints;
}

//------------------------------------------------------------------------------------
// -- DEBUG ONLY VISUALS
void AIPath::Render()const
{
	//dd->clear();
	////gdi->OrangePen();

	//std::list<noVec3>::const_iterator it = m_WayPoints.begin();
	//dd->depthMask(false);
	//dd->begin(DU_DRAW_LINES_STRIP, 10);
	//noVec3 wp = *it;

	//while (it != m_WayPoints.end())
	//{
	//	dd->vertex(wp.x, wp.yUP+10.0f, wp.y, (unsigned int)0);

	//	wp = *it++;
	//}
	//dd->vertex(wp.x, wp.yUP+10.0f, wp.y, (unsigned int)0);

	//if (m_bLooped) 
	//	dd->vertex((*m_WayPoints.begin()).x, (*m_WayPoints.begin()).yUP+10.0f, (*m_WayPoints.begin()).y, (unsigned int)0);

	//dd->depthMask(true);
	//dd->end();
}

void AIPath::initDD(void)
{
	if(dd != NULL)
		delete dd; 
	dd = new NavDebugDraw();
	//dd->setMaterialScript(std::string("EntityLines"));
	dd->setOffset(0.35f);
}