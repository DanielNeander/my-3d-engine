#include "stdafx.h"
#include "AIDraw.h"
#include "DebugDraw.h"
#include <GameApp/GameApp.h>
#include "NavDebugDraw.h"

using namespace OpenSteer;

NavDebugDraw* OpenSteer::AIDraw::drawPen_;

#define  MAX_VERTS 1000

bool OpenSteer::AIDraw::Init()
{
	drawPen_ = new NavDebugDraw[MAX_DRAW_TYPES];

	for (int i=0; i < MAX_DRAW_TYPES; ++i)
	{
		drawPen_[i].CreateVertexBuffer(NULL, MAX_VERTS, sizeof(DebugVertex) );
		drawPen_[i].SetupShader();
	}	

	if (!drawPen_)
	{
		assert(false);
		return false;
	}

	return true;
}

void OpenSteer::AIDraw::drawAxes( const AbstractLocalSpace& localSpace, const Vec3& size, const Vec3& color )
{
	


}

void OpenSteer::AIDraw::drawCircleOrDisk( const float radius, const Vec3& axis, const Vec3& center, const Vec3& color, const int segments, const bool filled, const bool in3d )
{
	LocalSpace ls;
	if (in3d)
	{
		// define a local space with "axis" as the Y/up direction
		// (XXX should this be a method on  LocalSpace?)
		const Vec3 unitAxis = axis.normalize ();
		const Vec3 unitPerp = findPerpendicularIn3d (axis).normalize ();
		ls.setUp (unitAxis);
		ls.setForward (unitPerp);
		ls.setPosition (center);
		ls.setUnitSideFromForwardAndUp ();
	}

	// make disks visible (not culled) from both sides 
	if (filled) 
		;//beginDoubleSidedDrawing ();

	// point to be rotated about the (local) Y axis, angular step size
	Vec3 pointOnCircle (radius, 0, 0);
	const float step = (2 * noMath::PI) / segments;

	// set drawing color
	//glColor3f (color.x, color.y, color.z);

	// begin drawing a triangle fan (for disk) or line loop (for circle)
	//glBegin (filled ? GL_TRIANGLE_FAN : GL_LINE_LOOP);


	// for the filled case, first emit the center point
	//if (filled) iglVertexnoVec3 (in3d ? ls.position() : center);

	duDebugDrawPrimitives prim;
	if(filled) prim =DU_DRAW_TRIANGLE_PAN; else prim = DU_DRAW_LINES;
	drawPen_[DRAW_CIRCLE].begin(prim);

	uint32 col = duRGBAf(color.x, color.y, color.z, 1.0f);

	if (filled)
	{
		if (in3d) drawPen_[DRAW_CIRCLE].vertex((float*)&ls.position(), col); 
		else drawPen_[DRAW_CIRCLE].vertex(center.x, center.y, center.z, col);
	}


	// rotate p around the circle in "segments" steps
	float sin=0, cos=0;
	const int vertexCount = filled ? segments+1 : segments;
	for (int i = 0; i < vertexCount; i++)
	{
		// emit next point on circle, either in 3d (globalized out
		// of the local space), or in 2d (offset from the center)
		//iglVertexnoVec3 (in3d ?
		//	ls.globalizePosition (pointOnCircle) :
		if (in3d) drawPen_[DRAW_CIRCLE].vertex((float*)&ls.globalizePosition (pointOnCircle), col); 
		else drawPen_[DRAW_CIRCLE].vertex((float*)&(pointOnCircle + center), col);
		

		// rotate point one more step around circle
		pointOnCircle = pointOnCircle.rotateAboutGlobalY (step, sin, cos);
	}

	// close drawing operation
	//glEnd ();
	drawPen_[DRAW_CIRCLE].end();
	//if (filled) endDoubleSidedDrawing ();
}

void OpenSteer::AIDraw::drawXZCircleOrDisk( const float radius, const Vec3& center, const Vec3& color, const int segments, const bool filled )
{

}

void OpenSteer::AIDraw::draw3dCircleOrDisk( const float radius, const Vec3& center, const Vec3& axis, const Vec3& color, const int segments, const bool filled )
{

}

void OpenSteer::AIDraw::drawXZCircle( const float radius, const Vec3& center, const Vec3& color, const int segments )
{

}



class DebugCircle
{
public:
	static void addToBuffer (const float radius,
		const Vec3& axis,
		const Vec3& center,
		const Vec3& color,
		const int segments,
		const bool filled,
		const bool in3d)
	{
		if (index < size)
		{
			deferredCircleArray[index].radius   = radius;
			deferredCircleArray[index].axis     = axis;
			deferredCircleArray[index].center   = center;
			deferredCircleArray[index].color    = color;
			deferredCircleArray[index].segments = segments;
			deferredCircleArray[index].filled   = filled;
			deferredCircleArray[index].in3d     = in3d;
			index++;
		}
		else
		{
			//OpenSteer::OpenSteerDemo::printWarning ("overflow in deferredDrawCircle buffer");
		}
	}

	static void drawAll (void)
	{
		// draw all circles in the buffer
		for (int i = 0; i < index; i++)
		{
			DebugCircle& dc = deferredCircleArray[i];
			AIDraw::drawCircleOrDisk (dc.radius, dc.axis, dc.center, dc.color,
				dc.segments, dc.filled, dc.in3d);
		}

		// reset buffer index
		index = 0;
	}


private:

	float radius;
	Vec3 axis;
	Vec3 center;
	Vec3 color;
	int segments;
	bool filled;
	bool in3d;

	static int index;
	static const int size;
	static DebugCircle deferredCircleArray [];

	NavDebugDraw* drawing_;

};


class DeferredLine
{
public:

	static void addToBuffer (const noVec3& s,
		const noVec3& e,
		const noVec3& c)
	{
		if (index < size)
		{
			deferredLineArray[index].startPoint = s;
			deferredLineArray[index].endPoint = e;
			deferredLineArray[index].color = c;
			index++;
		}
		else
		{
			//OpenSteer::OpenSteerDemo::printWarning ("overflow in deferredDrawLine buffer");
		}
	}

	static void drawAll (void)
	{
		// draw all lines in the buffer
		for (int i = 0; i < index; i++)
		{
			DeferredLine& dl = deferredLineArray[i];
			iDrawLine (dl.startPoint, dl.endPoint, dl.color);
		}

		// reset buffer index
		index = 0;
	}

	static void iDrawLine (const noVec3& startPoint,
		const noVec3& endPoint,
		const noVec3& color);

private:

	noVec3 startPoint;
	noVec3 endPoint;
	noVec3 color;

	static int index;
	static const int size;
	static DeferredLine deferredLineArray [];
};


int DeferredLine::index = 0;
// const int DeferredLine::size = 1000;
// const int DeferredLine::size = 2000;
const int DeferredLine::size = 3000;
DeferredLine DeferredLine::deferredLineArray [DeferredLine::size];

