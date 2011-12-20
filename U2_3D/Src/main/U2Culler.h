/************************************************************************
module	:	U2Culler
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_CULLER_H
#define U2_CULLER_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2lib/src/U2RefObject.h>
#include "U2VisibleSet.h"
#include "U2Frustum.h"
//#include "U2Plane.h"
//#include <U2_3D/src/collision/U2Aabb.h>
#include <U2_3D/src/collision/U2IntersectionTest.h>

class U2Camera;
class U2BoundingVolume;
class U2Spatial;
class U2SphereBV;
class U2_3D U2Culler : public U2RefObject
{

public:

	enum
	{
		// bits for the active planes
		NEAR_PLANE     = 0,
		FAR_PLANE      = 1,
		LEFT_PLANE     = 2,
		RIGHT_PLANE    = 3,
		TOP_PLANE      = 4,
		BOTTOM_PLANE   = 5,
		MAX_PLANES     = 6
	};

	U2Culler(int maxCnt =0, int iGrowby = 0, const U2Camera* pCamera = 0);
	virtual ~U2Culler();

	void SetCamera (const U2Camera* pkCamera);
	const U2Camera* GetCamera () const;
	void SetFrustumLH (const U2Frustum& afFrustum);
		
	void SetFrustumRH (const U2Frustum& afFrustum);	


	const U2Frustum& GetFrustum () const;
	U2VisibleSet& GetVisibleSet ();

	bool IsVisible (const U2BoundingVolume* pkBound);


	virtual void Insert(U2Spatial* obj, U2Effect* globalEffect);

	enum { MAX_PLANE_QUANTITY = 32 };
	int GetPlaneQuantity () const;
	const U2Plane* GetPlanes () const;
	void SetPlaneState (unsigned int uiPlaneState);
	unsigned int GetPlaneState () const;
	void PushPlane (const U2Plane& rkPlane);
	void PopPlane ();

	bool IsVisible(uint32 uVertCnt, const D3DXVECTOR3* pVerts, bool bIgnoreNearPlane);

	// Support for BspNode::GetVisibleSet.  Determine if the view frustum is
	// fully on one side of a plane.  The "positive side" of the plane is the
	// half space to which the plane normal points.  The "negative side" is
	// the other half space.  The function returns +1 if the view frustum is
	// fully on the positive side of the plane, -1 if the view frustum is
	// fully on the negative side of the plane, or 0 if the view frustum
	// straddles the plane.  The input plane is in world coordinates and the
	// world camera coordinate system is used for the test.
	int WhichSide(const U2Plane& plane) const;

	 void ComputeVisibleSet (U2Spatial* pkScene);

	 // Fix : Global ·Î º¯°æ
	 bool TestPoint(const D3DXVECTOR3& point) const;
	 bool TestAabb(const U2Aabb& rect) const;	

	 // 0 :  1 : 2 :
	 int TestAabb2(const U2Aabb& bbox) const;

	
	 bool SweptSpherePlaneIntersect(float& t0, float& t1, const U2Plane& plane, const U2SphereBV*,
		 const D3DXVECTOR3* sweepDir) const;

	 bool TestSweptSphere(const U2SphereBV *sphere, const D3DXVECTOR3 *sweepDir) const;


private:	
	U2Frustum m_frustum;

	uint32 m_uPlaneCnt;
	U2Plane m_aPlanes[MAX_PLANE_QUANTITY];
	uint32 m_uPlaneState;

	int nVertexLUT[6];

	const U2Camera* m_pCamera;

	U2VisibleSet* m_pVisibles;

};

inline bool U2Culler::TestPoint(const D3DXVECTOR3& point) const
{
	if(m_aPlanes[NEAR_PLANE].GetDistance(point)		< 0.0f ||
		m_aPlanes[FAR_PLANE].GetDistance(point)		< 0.0f ||
		m_aPlanes[LEFT_PLANE].GetDistance(point)	< 0.0f ||
		m_aPlanes[RIGHT_PLANE].GetDistance(point)	< 0.0f ||
		m_aPlanes[TOP_PLANE].GetDistance(point)		< 0.0f ||
		m_aPlanes[BOTTOM_PLANE].GetDistance(point)	< 0.0f)
	{
		return false;
	}
	
	return true;
}

inline bool U2Culler::TestAabb(const U2Aabb& bbox) const
{
	using namespace U2Intersect;

	if(PlaneClassify(bbox, m_aPlanes[NEAR_PLANE]) == PLANE_BACK ||
		PlaneClassify(bbox, m_aPlanes[FAR_PLANE]) == PLANE_BACK ||
		PlaneClassify(bbox, m_aPlanes[LEFT_PLANE]) == PLANE_BACK ||
		PlaneClassify(bbox, m_aPlanes[RIGHT_PLANE]) == PLANE_BACK ||
		PlaneClassify(bbox, m_aPlanes[TOP_PLANE]) == PLANE_BACK ||
		PlaneClassify(bbox, m_aPlanes[BOTTOM_PLANE]) == PLANE_BACK)
	{
		return false;
	}

	return true;
}

inline int U2Culler::TestAabb2(const U2Aabb& box) const
{
	bool intersect = false;

	for(int i = 0; i < MAX_PLANES; i++)
	{
		// pVertex is diagonally opposed to nVertex	
		int nV = nVertexLUT[i];
		// pVertex is diagonally opposed to nVertex
		D3DXVECTOR3 nVertex( (nV&1)?box.m_vMin.x:box.m_vMax.x, (nV&2)?box.m_vMin.y:box.m_vMax.y, (nV&4)?box.m_vMin.z:box.m_vMax.z );
		D3DXVECTOR3 pVertex( (nV&1)?box.m_vMax.x:box.m_vMin.x, (nV&2)?box.m_vMax.y:box.m_vMin.y, (nV&4)?box.m_vMax.z:box.m_vMin.z );

		if ( m_aPlanes[i].GetDistance(nVertex) < 0.0f)
			return 0;

		if ( m_aPlanes[i].GetDistance(pVertex) < 0.0f)
			intersect = true;			
	}
	
	return (intersect) ? 2 : 1;
}



typedef U2SmartPtr<U2Culler> U2CullerPtr;

#endif