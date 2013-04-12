#pragma once 

#include "Transform2DFuncs.h"

//compares two real numbers. Returns true if they are equal
inline bool isEqual(float a, float b)
{
	if (fabs(a-b) < 1E-12)
	{
		return true;
	}

	return false;
}

//--------------------LineIntersection2D-------------------------
//
//	Given 2 lines in 2D space AB, CD this returns true if an 
//	intersection occurs and sets dist to the distance the intersection
//  occurs along AB
//
//----------------------------------------------------------------- 

inline bool LineIntersection2D(noVec3 A,
	noVec3 B,
	noVec3 C, 
	noVec3 D,
	float &dist)
{

	float rTop = (A.y-C.y)*(D.x-C.x)-(A.x-C.x)*(D.y-C.y);
	float sTop = (A.y-C.y)*(B.x-A.x)-(A.x-C.x)*(B.y-A.y);

	float Bot = (B.x-A.x)*(D.y-C.y)-(B.y-A.y)*(D.x-C.x);


	if (Bot == 0)//parallel
	{
		if (isEqual(rTop, 0) && isEqual(sTop, 0))
		{
			return true;
		}
		return false;
	}

	float r = rTop/Bot;
	float s = sTop/Bot;

	if( (r > 0) && (r < 1) && (s > 0) && (s < 1) )
	{
		dist = (A -B).Length() * r;

		return true;
	}

	else
	{
		dist = 0;

		return false;
	}
}

//-------------------- LineIntersection2D-------------------------
//
//	Given 2 lines in 2D space AB, CD this returns true if an 
//	intersection occurs and sets dist to the distance the intersection
//  occurs along AB. Also sets the 2d vector point to the point of
//  intersection
//----------------------------------------------------------------- 
inline bool LineIntersection2D(noVec3   A,
	noVec3   B,
	noVec3   C, 
	noVec3   D,
	double&     dist,
	noVec3&  point)
{

	double rTop = (A.y-C.y)*(D.x-C.x)-(A.x-C.x)*(D.y-C.y);
	double rBot = (B.x-A.x)*(D.y-C.y)-(B.y-A.y)*(D.x-C.x);

	double sTop = (A.y-C.y)*(B.x-A.x)-(A.x-C.x)*(B.y-A.y);
	double sBot = (B.x-A.x)*(D.y-C.y)-(B.y-A.y)*(D.x-C.x);

	if ( (rBot == 0) || (sBot == 0))
	{
		//lines are parallel
		return false;
	}

	double r = rTop/rBot;
	double s = sTop/sBot;

	if( (r > 0) && (r < 1) && (s > 0) && (s < 1) )
	{
		dist = (A-B).Length() * r;

		point = A + r * (B - A);

		return true;
	}

	else
	{
		dist = 0;

		return false;
	}
}

//----------------------------- TwoCirclesOverlapped ---------------------
//
//  Returns true if the two circles overlap
//------------------------------------------------------------------------
inline bool TwoCirclesOverlapped(double x1, double y1, double r1,
	double x2, double y2, double r2)
{
	double DistBetweenCenters = sqrt( (x1-x2) * (x1-x2) +
		(y1-y2) * (y1-y2));

	if ((DistBetweenCenters < (r1+r2)) || (DistBetweenCenters < fabs(r1-r2)))
	{
		return true;
	}

	return false;
}

//----------------------------- TwoCirclesOverlapped ---------------------
//
//  Returns true if the two circles overlap
//------------------------------------------------------------------------
inline bool TwoCirclesOverlapped(noVec3 c1, double r1,
	noVec3 c2, double r2)
{
	double DistBetweenCenters = sqrt( (c1.x-c2.x) * (c1.x-c2.x) +
		(c1.y-c2.y) * (c1.y-c2.y));

	if ((DistBetweenCenters < (r1+r2)) || (DistBetweenCenters < fabs(r1-r2)))
	{
		return true;
	}

	return false;
}


//--------------------------- TwoCirclesEnclosed ---------------------------
//
//  returns true if one circle encloses the other
//-------------------------------------------------------------------------
inline bool TwoCirclesEnclosed(double x1, double y1, double r1,
	double x2, double y2, double r2)
{
	double DistBetweenCenters = sqrt( (x1-x2) * (x1-x2) +
		(y1-y2) * (y1-y2));

	if (DistBetweenCenters < fabs(r1-r2))
	{
		return true;
	}

	return false;
}

//------------------------ TwoCirclesIntersectionPoints ------------------
//
//  Given two circles this function calculates the intersection points
//  of any overlap.
//
//  returns false if no overlap found
//
// see http://astronomy.swin.edu.au/~pbourke/geometry/2circle/
//------------------------------------------------------------------------ 
inline bool TwoCirclesIntersectionPoints(double x1, double y1, double r1,
	double x2, double y2, double r2,
	double &p3X, double &p3Y,
	double &p4X, double &p4Y)
{
	//first check to see if they overlap
	if (!TwoCirclesOverlapped(x1,y1,r1,x2,y2,r2))
	{
		return false;
	}

	//calculate the distance between the circle centers
	double d = sqrt( (x1-x2) * (x1-x2) + (y1-y2) * (y1-y2));

	//Now calculate the distance from the center of each circle to the center
	//of the line which connects the intersection points.
	double a = (r1 - r2 + (d * d)) / (2 * d);
	double b = (r2 - r1 + (d * d)) / (2 * d);


	//MAYBE A TEST FOR EXACT OVERLAP? 

	//calculate the point P2 which is the center of the line which 
	//connects the intersection points
	double p2X, p2Y;

	p2X = x1 + a * (x2 - x1) / d;
	p2Y = y1 + a * (y2 - y1) / d;

	//calculate first point
	double h1 = sqrt((r1 * r1) - (a * a));

	p3X = p2X - h1 * (y2 - y1) / d;
	p3Y = p2Y + h1 * (x2 - x1) / d;


	//calculate second point
	double h2 = sqrt((r2 * r2) - (a * a));

	p4X = p2X + h2 * (y2 - y1) / d;
	p4Y = p2Y - h2 * (x2 - x1) / d;

	return true;

}

//------------------------ TwoCirclesIntersectionArea --------------------
//
//  Tests to see if two circles overlap and if so calculates the area
//  defined by the union
//
// see http://mathforum.org/library/drmath/view/54785.html
//-----------------------------------------------------------------------
inline double TwoCirclesIntersectionArea(double x1, double y1, double r1,
	double x2, double y2, double r2)
{
	//first calculate the intersection points
	double iX1, iY1, iX2, iY2;

	if(!TwoCirclesIntersectionPoints(x1,y1,r1,x2,y2,r2,iX1,iY1,iX2,iY2))
	{
		return 0.0; //no overlap
	}

	//calculate the distance between the circle centers
	double d = sqrt( (x1-x2) * (x1-x2) + (y1-y2) * (y1-y2));

	//find the angles given that A and B are the two circle centers
	//and C and D are the intersection points
	double CBD = 2 * acos((r2*r2 + d*d - r1*r1) / (r2 * d * 2)); 

	double CAD = 2 * acos((r1*r1 + d*d - r2*r2) / (r1 * d * 2));


	//Then we find the segment of each of the circles cut off by the 
	//chord CD, by taking the area of the sector of the circle BCD and
	//subtracting the area of triangle BCD. Similarly we find the area
	//of the sector ACD and subtract the area of triangle ACD.

	double area = 0.5f*CBD*r2*r2 - 0.5f*r2*r2*sin(CBD) +
		0.5f*CAD*r1*r1 - 0.5f*r1*r1*sin(CAD);

	return area;
}


//-------------------------------- CircleArea ---------------------------
//
//  given the radius, calculates the area of a circle
//-----------------------------------------------------------------------
inline double CircleArea(double radius)
{
	return noMath::PI * radius * radius;
}


//----------------------- PointInCircle ----------------------------------
//
//  returns true if the point p is within the radius of the given circle
//------------------------------------------------------------------------
inline bool PointInCircle(noVec3 Pos,
	double    radius,
	noVec3 p)
{
	double DistFromCenterSquared = (p-Pos).LengthSqr();

	if (DistFromCenterSquared < (radius*radius))
	{
		return true;
	}

	return false;
}

//------------------- GetLineSegmentCircleClosestIntersectionPoint ------------
//
//  given a line segment AB and a circle position and radius, this function
//  determines if there is an intersection and stores the position of the 
//  closest intersection in the reference IntersectionPoint
//
//  returns false if no intersection point is found
//-----------------------------------------------------------------------------
inline bool GetLineSegmentCircleClosestIntersectionPoint(noVec3 A,
	noVec3 B,
	noVec3 pos,
	double    radius,
	noVec3& IntersectionPoint)
{
	noVec3 toBNorm = (B-A);
	toBNorm.Normalize();


	//move the circle into the local space defined by the vector B-A with origin
	//at A
	noVec3 LocalPos = PointToLocalSpace(pos, toBNorm, toBNorm.Perp(), A);

	bool ipFound = false;

	//if the local position + the radius is negative then the circle lays behind
	//point A so there is no intersection possible. If the local x pos minus the 
	//radius is greater than length A-B then the circle cannot intersect the 
	//line segment
	if ( (LocalPos.x+radius >= 0) &&
		( (LocalPos.x-radius)*(LocalPos.x-radius) <= (B-A).LengthSqr()) )
	{
		//if the distance from the x axis to the object's position is less
		//than its radius then there is a potential intersection.
		if (fabs(LocalPos.y) < radius)
		{
			//now to do a line/circle intersection test. The center of the 
			//circle is represented by A, B. The intersection points are 
			//given by the formulae x = A +/-sqrt(r^2-B^2), y=0. We only 
			//need to look at the smallest positive value of x.
			double a = LocalPos.x;
			double b = LocalPos.y;       

			double ip = a - sqrt(radius*radius - b*b);

			if (ip <= 0)
			{
				ip = a + sqrt(radius*radius - b*b);
			}

			ipFound = true;

			IntersectionPoint = A+ toBNorm*ip;
		}
	}

	return ipFound;
}