/************************************************************************
module	:	U2IntersectionTest
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once
#ifndef U2_INTERSECTIONTEST_H
#define U2_INTERSECTIONTEST_H

#include "U2Aabb.h"
#include <U2_3D/Src/Main/U2Plane.h>
#include <U2_3D/Src/Main/U2Math.h>
#include <U2_3D/Src/Main/U2Frustum.h>
#include <U2_3D/Src/U2Common.h>
#include <d3dx9math.h>


namespace U2Intersect {;




enum PlaneClassifications
{
	PLANE_FRONT = 0,
	PLANE_BACK,
	PLANE_INTERSECT
};

inline int PlaneClassify(const U2Aabb& box, const U2Plane& plane)
{
	D3DXVECTOR3 minPoint, maxPoint;

	// build two points based on the direction
	// of the plane vector. minPoint 
	// and maxPoint are the two points
	// on the rectangle furthest away from
	// each other along the plane normal

	if(plane.m_vNormal.x > 0.0f)
	{
		minPoint.x = box.m_vMin.x;
		maxPoint.x = box.m_vMax.x;
	}
	else 
	{
		minPoint.x = box.m_vMax.x;
		maxPoint.x = box.m_vMin.x;		
	}

	if(plane.m_vNormal.y > 0.0f)
	{
		minPoint.y = box.m_vMin.y;
		maxPoint.y = box.m_vMax.y;
	}
	else 
	{
		minPoint.y = box.m_vMax.y;
		maxPoint.y = box.m_vMin.y;		
	}

	if(plane.m_vNormal.z > 0.0f)
	{
		minPoint.z = box.m_vMin.z;
		maxPoint.z = box.m_vMax.z;
	}
	else 
	{
		minPoint.z = box.m_vMax.z;
		maxPoint.z = box.m_vMin.z;		
	}

	float minDist = plane.GetDistance(minPoint);
	float maxDist = plane.GetDistance(maxPoint);

	if(maxDist < 0.0f)
	{
		return PLANE_BACK;
	}
	else if(maxDist> 0.0f)
	{
		return PLANE_FRONT;
	}

	return PLANE_INTERSECT;	
}


// AABB vs AABB test, returns true if objects intersect
//
//
template<class TVector>
inline bool IntersectionTest(const U2Aabb &objectBB, const U2Aabb &frustumBB)
{
	// min and max vectors
	const TVector &vFrustumMin = frustumBB.m_vMin;
	const TVector &vFrustumMax = frustumBB.m_vMax;
	const TVector &vObjectMin = objectBB.m_vMin;
	const TVector &vObjectMax = objectBB.m_vMax;

	// test all axes
	//
	if(vObjectMin.x > vFrustumMax.x || vFrustumMin.x > vObjectMax.x) return false;
	if(vObjectMin.y > vFrustumMax.y || vFrustumMin.y > vObjectMax.y) return false;
	if(vObjectMin.z > vFrustumMax.z || vFrustumMin.z > vObjectMax.z) return false;

	// all tests passed - intersection occurs
	return true;
}


// AABB vs AABB sweep test, returns true if intersection can occur if object is translated along given direction
//
//
template<class TVector>
inline bool SweepIntersectionTest(const U2Aabb &objectBB, const U2Aabb &frustumBB, const TVector &vSweepDir)
{
	// min and max vectors of object
	const TVector &vFrustumMin = frustumBB.m_vMin;
	const TVector &vFrustumMax = frustumBB.m_vMax;
	const TVector &vObjectMin = objectBB.m_vMin;
	const TVector &vObjectMax = objectBB.m_vMax;

	// calculate projections along sweep direction
	//

	// project AABB center point
	TVector vFrustumCenter = (vFrustumMin + vFrustumMax) * 0.5f;
	TVector vFrustumHalfSize = (vFrustumMax - vFrustumMin) * 0.5f;
	float fFrustumCenterProj = U2Math::Dot(vFrustumCenter, vSweepDir);
	// project AABB half-size
	float fFrustumHalfSizeProj = vFrustumHalfSize.x * fabs(vSweepDir.x) +
		vFrustumHalfSize.y * fabs(vSweepDir.y) +
		vFrustumHalfSize.z * fabs(vSweepDir.z);
	float fFrustumProjMin = fFrustumCenterProj - fFrustumHalfSizeProj;
	float fFrustumProjMax = fFrustumCenterProj + fFrustumHalfSizeProj;

	// project AABB center poin
	TVector vObjectCenter = (vObjectMin + vObjectMax) * 0.5f;
	TVector vObjectHalfSize = (vObjectMax - vObjectMin) * 0.5f;
	float fObjectCenterProj = U2Math::Dot(vObjectCenter, vSweepDir);
	// project AABB half-size
	float fObjectHalfSizeProj = vObjectHalfSize.x * fabs(vSweepDir.x) +
		vObjectHalfSize.y * fabs(vSweepDir.y) +
		vObjectHalfSize.z * fabs(vSweepDir.z);
	float fObjectProjMin = fObjectCenterProj - fObjectHalfSizeProj;
	float fObjectProjMax = fObjectCenterProj + fObjectHalfSizeProj;

	// find the distance in sweep direction
	// where intersection occurs on all axis.
	//

	// sweep direction intersection
	// starts: fObjectProjMax + fDist = fFrustumProjMin
	//   ends: fObjectProjMin + fDist = fFrustumProjMax
	float fDistMin = fFrustumProjMin - fObjectProjMax;
	float fDistMax = fFrustumProjMax - fObjectProjMin;
	if(fDistMin > fDistMax) Common::Swap(fDistMin, fDistMax);

	// only intersects in opposite of sweep direction
	if(fDistMax < 0) return false;

	// intersection on an axis:
	// starts: vObjectMax.x + fDist*vSweepDir.x = vFrustumMin.x
	//   ends: vObjectMin.x + fDist*vSweepDir.x = vFrustumMax.x

	// test x-axis:
	if(vSweepDir.x == 0)
	{
		// there is never an intersection on this axis
		if(vFrustumMin.x > vObjectMax.x || vObjectMin.x > vFrustumMax.x) return false;
	}
	else
	{
		float fDistMinNew = (vFrustumMin.x - vObjectMax.x) / vSweepDir.x;
		float fDistMaxNew = (vFrustumMax.x - vObjectMin.x) / vSweepDir.x;
		if(fDistMinNew > fDistMaxNew) Common::Swap(fDistMinNew, fDistMaxNew);

		// distance ranges don't overlap
		if(fDistMin > fDistMaxNew || fDistMinNew > fDistMax) return false;
		// otherwise merge ranges
		fDistMin = Max(fDistMin, fDistMinNew);
		fDistMax = Min(fDistMax, fDistMaxNew);
	}

	// test y-axis:
	if(vSweepDir.y == 0)
	{
		// there is never an intersection on this axis
		if(vFrustumMin.y > vObjectMax.y || vObjectMin.y > vFrustumMax.y) return false;
	}
	else
	{
		float fDistMinNew = (vFrustumMin.y - vObjectMax.y) / vSweepDir.y;
		float fDistMaxNew = (vFrustumMax.y - vObjectMin.y) / vSweepDir.y;
		if(fDistMinNew > fDistMaxNew) Common::Swap(fDistMinNew, fDistMaxNew);

		// distance ranges don't overlap
		if(fDistMin > fDistMaxNew || fDistMinNew > fDistMax) return false;
		// otherwise merge ranges
		fDistMin = Max(fDistMin, fDistMinNew);
		fDistMax = Min(fDistMax, fDistMaxNew);
	}

	// test z-axis:
	if(vSweepDir.z == 0)
	{
		// there is never an intersection on this axis
		if(vFrustumMin.z > vObjectMax.z || vObjectMin.z > vFrustumMax.z) return false;
	}
	else
	{
		float fDistMinNew = (vFrustumMin.z - vObjectMax.z) / vSweepDir.z;
		float fDistMaxNew = (vFrustumMax.z - vObjectMin.z) / vSweepDir.z;
		if(fDistMinNew > fDistMaxNew) Common::Swap(fDistMinNew, fDistMaxNew);

		// distance ranges don't overlap
		if(fDistMin > fDistMaxNew || fDistMinNew > fDistMax) return false;
	}

	// all tests passed - intersection occurs
	return true;
}


// Helper function for AABB vs frustum test
//
//
template<class TVector>
inline bool ProjectedIntersection(const TVector &vHalfSize, const TVector &vCenter,
								  const TVector *pFrustumPoints,
								  const TVector &vDir)
{
	using namespace U2Math;

	// project AABB center point to vector
	float fCenter = Dot(vCenter, vDir);
	// project AABB half-size to vector
	float fHalfSize = vHalfSize.x * fabs(vDir.x) +
		vHalfSize.y * fabs(vDir.y) +
		vHalfSize.z * fabs(vDir.z);

	float fMin1 = fCenter - fHalfSize;
	float fMax1 = fCenter + fHalfSize;

	// project frustum points
	float fProj2 = Dot(pFrustumPoints[0], vDir);
	float fMin2 = fProj2;
	float fMax2 = fProj2;
	for(int i=1;i<8;i++)
	{
		fProj2 = Dot(pFrustumPoints[i], vDir);
		fMin2 = Common::Min(fProj2, fMin2);
		fMax2 =Common:: Max(fProj2, fMax2);
	}

	// test for overlap
	if(fMin1 > fMax2 || fMin2 > fMax1) return false;

	return true;
}

// AABB vs Frustum test, returns true if objects intersect
//
//
template<class TVector>
inline bool IntersectionTest(const U2Aabb &objectBB, const U2Frustum &frustum)
{
	// Note that this code is very unoptimal
	//
	//
	TVector vHalfSize = (objectBB.m_vMax - objectBB.m_vMin) * 0.5f;
	TVector vCenter = (objectBB.m_vMin + objectBB.m_vMax) * 0.5f;

	// AABB face normals
	if(!ProjectedIntersection(vHalfSize, vCenter, frustum.m_Points, TVector(1,0,0))) return false;
	if(!ProjectedIntersection(vHalfSize, vCenter, frustum.m_Points, TVector(0,1,0))) return false;
	if(!ProjectedIntersection(vHalfSize, vCenter, frustum.m_Points, TVector(0,0,1))) return false;

	// frustum face normals
	//

	// front and back faces:
	TVector vNorm1 = Normalize(Cross(frustum.m_Points[1] - frustum.m_Points[0],
		frustum.m_Points[3] - frustum.m_Points[0]));
	if(!ProjectedIntersection(vHalfSize, vCenter, frustum.m_Points, vNorm1)) return false;

	// left face:
	TVector vNorm2 = Normalize(Cross(frustum.m_Points[1] - frustum.m_Points[0],
		frustum.m_Points[4] - frustum.m_Points[0]));
	if(!ProjectedIntersection(vHalfSize, vCenter, frustum.m_Points, vNorm2)) return false;

	// right face:
	TVector vNorm3 = Normalize(Cross(frustum.m_Points[2] - frustum.m_Points[3],
		frustum.m_Points[7] - frustum.m_Points[3]));
	if(!ProjectedIntersection(vHalfSize, vCenter, frustum.m_Points, vNorm3)) return false;

	// top face:
	TVector vNorm4 = Normalize(Cross(frustum.m_Points[2] - frustum.m_Points[1],
		frustum.m_Points[5] - frustum.m_Points[1]));
	if(!ProjectedIntersection(vHalfSize, vCenter, frustum.m_Points, vNorm4)) return false;

	// bottom face:
	TVector vNorm5 = Normalize(Cross(frustum.m_Points[3] - frustum.m_Points[0],
		frustum.m_Points[4] - frustum.m_Points[0]));
	if(!ProjectedIntersection(vHalfSize, vCenter, frustum.m_Points, vNorm5)) return false;


	// edge cross edge cases
	//
	TVector pBoxEdges[3] = {TVector(1,0,0), TVector(0,1,0), TVector(0,0,1)};
	for(int i=0;i<3;i++)
	{
		// edge up-down
		TVector vNorm1 = Normalize(Cross(frustum.m_Points[1] - frustum.m_Points[0], pBoxEdges[i]));
		if(!ProjectedIntersection(vHalfSize, vCenter, frustum.m_Points, vNorm1)) return false;

		// edge left-right
		TVector vNorm2 = Normalize(Cross(frustum.m_Points[3] - frustum.m_Points[0], pBoxEdges[i]));
		if(!ProjectedIntersection(vHalfSize, vCenter, frustum.m_Points, vNorm2)) return false;

		// edge bottom left
		TVector vNorm3 = Normalize(Cross(frustum.m_Points[4] - frustum.m_Points[0], pBoxEdges[i]));
		if(!ProjectedIntersection(vHalfSize, vCenter, frustum.m_Points, vNorm3)) return false;

		// edge top left
		TVector vNorm4 = Normalize(Cross(frustum.m_Points[5] - frustum.m_Points[1], pBoxEdges[i]));
		if(!ProjectedIntersection(vHalfSize, vCenter, frustum.m_Points, vNorm4)) return false;

		// edge top right
		TVector vNorm5 = Normalize(Cross(frustum.m_Points[6] - frustum.m_Points[2], pBoxEdges[i]));
		if(!ProjectedIntersection(vHalfSize, vCenter, frustum.m_Points, vNorm5)) return false;

		// edge bottom right
		TVector vNorm6 = Normalize(Cross(frustum.m_Points[7] - frustum.m_Points[3], pBoxEdges[i]));
		if(!ProjectedIntersection(vHalfSize, vCenter, frustum.m_Points, vNorm6)) return false;
	}

	// all tests passed - intersection occurs
	return true;
}


// Helper function for AABB vs Frustum sweep test
//
//

template<class TVector>
inline bool ProjectedSweepIntersection(const TVector &vHalfSize, const TVector &vCenter,
									   const TVector *pFrustumPoints,
									   const TVector &vDir,
									   float &fDistMin, float &fDistMax, const TVector &vSweepDir)
{
	using namespace U2Math;
	using namespace Common;

	// project sweep direction
	float fSweepDir = Dot(vSweepDir, vDir);

	// project AABB center point to vector
	float fCenter = Dot(vCenter, vDir);
	// project AABB half-size to vector
	float fHalfSize = vHalfSize.x * fabs(vDir.x) +
		vHalfSize.y * fabs(vDir.y) +
		vHalfSize.z * fabs(vDir.z);

	float fMin1 = fCenter - fHalfSize;
	float fMax1 = fCenter + fHalfSize;

	// project frustum points
	float fProj2 = Dot(pFrustumPoints[0], vDir);
	float fMin2 = fProj2;
	float fMax2 = fProj2;
	for(int i=1;i<8;i++)
	{
		fProj2 = Dot(pFrustumPoints[i], vDir);
		fMin2 = Min(fProj2, fMin2);
		fMax2 = Max(fProj2, fMax2);
	}

	// sweep can affect intersection
	if(fSweepDir != 0)
	{
		// intersection starts when fMax1 + fSweepDir * t >= fMin2
		//
		float fIntersectionStart = (fMin2 - fMax1) / fSweepDir;

		// intersection ends when fMin1 + fSweepDir * t >= fMax2
		//
		float fIntersectionEnd = (fMax2 - fMin1) / fSweepDir;

		// ranges must be in right order
		if(fIntersectionStart > fIntersectionEnd) Swap(fIntersectionStart, fIntersectionEnd);

		// distance ranges don't overlap
		if(fDistMin > fIntersectionEnd || fIntersectionStart > fDistMax)
		{
			return false;
		}

		// otherwise merge ranges
		fDistMin = Max(fDistMin, fIntersectionStart);
		fDistMax = Min(fDistMax, fIntersectionEnd);
	}
	// sweep doesn't affect intersection
	else
	{
		// no intersectection ever
		if(fMin1 > fMax2 || fMin2 > fMax1)
		{
			return false;
		}
	}

	return true;
}



}

#endif
