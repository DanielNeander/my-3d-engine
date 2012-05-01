#include <U2_3D/src/U23DLibPCH.h>
#include "U2ConvexRegion.h"
#include "U2Portal.h"


IMPLEMENT_RTTI(U2Portal, U2Object);

U2Portal::U2Portal(uint32 uVertCnt, D3DXVECTOR3* pLocalVerts, 
		 const U2Plane& localPlane, U2ConvexRegion* pAdajacentRegion,
		 bool bOpen) : m_localPlane(localPlane)
{
	U2ASSERT(uVertCnt >= 3);

	m_uVertCnt = uVertCnt;
	m_pLocalVerts = pLocalVerts;
	m_pWorldVerts = U2_ALLOC(D3DXVECTOR3, m_uVertCnt);
	m_pAdjacentRegion = pAdajacentRegion;
	m_bOpen = bOpen;
}

U2Portal::U2Portal()
{
	m_uVertCnt = 0;
	m_pLocalVerts = 0;
	m_pWorldVerts = 0;
	m_pAdjacentRegion = 0;
	m_bOpen = false;
}

U2Portal::~U2Portal()
{
	U2_FREE(m_pLocalVerts);
	m_pLocalVerts = NULL;
	U2_FREE(m_pWorldVerts);
	m_pWorldVerts = NULL;

}

void U2Portal::UpdateWorldData(const U2Transform& worldTM)
{
	D3DXMATRIX tempRot;
	D3DXMatrixTranspose(&tempRot, &worldTM.GetRot());
	D3DXVec3TransformCoordArray(m_pWorldVerts, sizeof(float) * 3, m_pLocalVerts, 
		sizeof(float) * 3, &tempRot, m_uVertCnt);
}

bool U2Portal::ReduceFrustum(const U2Culler& culler, U2Frustum& reduceFrustum)
{
	// The portal polygon is transformed into the camera coordinate system
	// and projected onto the near plane.  An axis-aligned bounding rectangle
	// is computed for the projected points and clipped against the left,
	// right, bottom, and top frustum planes.  The result is itself an
	// axis-aligned bounding rectangle that is used to define a "reduced
	// frustum" to be used for drawing what is visible through the portal
	// polygon.
	//
	// The algorithm must handle the situation when portal polygon vertices
	// are behind the observer.  Imagine standing in a room with a doorway
	// immediately to your left.  Part of the doorway frame is in front of
	// you (and visible) and part of it is behind you (and not visible).
	// A portal point is represented by P = E + d*D + u*U + r*R, where E is
	// the world location for the eye point, D is the camera's world direction
	// vector, U is the camera's world up vector, and R is the camera's world
	// right vector.  The camera coordinates for the portal point are (d,u,r).
	// If d > 0, P is in front of the eye point and has a projection onto the
	// near plane d = n.  If d < 0, P is behind the eye point and does not
	// have a projection onto the near plane.  If d = 0, P projects to
	// "infinity" on the near plane, a problematic case to deal with.
	//
	// To avoid dealing with d = 0, choose a small value e such that
	// 0 < e < n.  The portal polygon is clipped against the plane d = e,
	// keeping only that portion whose points satisfy d >= e.  The clipped
	// polygon always has a projection onto the near plane.  The axis-aligned
	// bounding box for this projection is computed; clipped against the
	// left, right, bottom, and top of the frustum; and the result used to
	// define the reduced frustum.  All this is designed for an inexact
	// culling of the objects in the adjacent room, so it is useful to avoid
	// preserving the topology of the portal polygon as it is clipped.
	// Instead, the portal polygon vertices with d > e are projected and
	// the intersection points of portal polygon edges with d = e are
	// computed and projected.  The axis-aligned bounding box is computed for
	// the projections, a process that does not require knowing the polygon
	// topology.  The algorithm is essentially the one used for clipping a
	// convex polygon against the view frustum in the software renderer.  The
	// polygon vertices are traversed in-order and the signs of the d values
	// are updated accordingly.  This avoids computing d-signs twice per
	// vertex.

	const U2Camera* pCamera = culler.GetCamera();
	const U2Frustum& frustum = culler.GetFrustum();
	float fLeft = FLT_MAX;
	float fRight = -FLT_MAX;
	float fBottom = FLT_MAX;
	float fTop = -FLT_MAX;
	float fNearDivCamVertX, fCamVertYDotFNearDivCamVertX, 
		fCamVertZDotFNearDivCamVertX, fT;

	D3DXVECTOR3 vDiff, ptCamVert;
	uint32 i;

	if(!frustum.m_bOrtho)
	{
		const float fEpsilon = 1e-6f, fInvEpsilon = 1e+6f;
		int iFirstSign = 0, iLastSign =0;
		bool bSignChanged = false;
		D3DXVECTOR3 ptFirstVert(VECTOR3_ZERO);
		D3DXVECTOR3 ptLastVert(VECTOR3_ZERO);
		
		for(i=0; i < m_uVertCnt; ++i)
		{
			vDiff = m_pWorldVerts[i] - pCamera->GetEye();
			ptCamVert.x = D3DXVec3Dot(&vDiff, &pCamera->GetDir());
			ptCamVert.y = D3DXVec3Dot(&vDiff, &pCamera->GetUp());
			ptCamVert.z = D3DXVec3Dot(&vDiff, &pCamera->GetRight());
			
			if(ptCamVert.x > fEpsilon)
			{
				if(iFirstSign == 0)
				{
					iFirstSign = 1;
					ptFirstVert = ptCamVert;
				}

				fNearDivCamVertX = frustum.m_fNear / ptCamVert.x;
				fCamVertYDotFNearDivCamVertX = ptCamVert.y * fNearDivCamVertX;
				fCamVertZDotFNearDivCamVertX = ptCamVert.z * fNearDivCamVertX;

				if(fCamVertYDotFNearDivCamVertX < fBottom)
				{
					fBottom = fCamVertYDotFNearDivCamVertX;
				}
				if(fCamVertYDotFNearDivCamVertX > fTop)
				{
					fTop = fCamVertYDotFNearDivCamVertX;
				}

				if(fCamVertZDotFNearDivCamVertX < fLeft)
				{
					fLeft = fCamVertZDotFNearDivCamVertX;
				}
				if(fCamVertZDotFNearDivCamVertX > fRight)
				{
					fRight = fCamVertZDotFNearDivCamVertX;
				}

				if(iLastSign < 0)
				{
					bSignChanged = true;
				}

				iLastSign = 1;				
			}
			else 
			{
				if(iFirstSign == 0)
				{
					iFirstSign = -1;
					ptFirstVert = ptCamVert;
				}

				iLastSign = -1;				
			}

			if(bSignChanged)
			{
				vDiff = ptCamVert - ptLastVert;
				fT = (fEpsilon - ptLastVert.x) / vDiff.x;
				fNearDivCamVertX = frustum.m_fNear * fInvEpsilon;
				fCamVertYDotFNearDivCamVertX = (ptLastVert.y + fT * vDiff.y) *
					fNearDivCamVertX;
				fCamVertZDotFNearDivCamVertX = (ptLastVert.z + fT * vDiff.z) *
					fNearDivCamVertX;


				if(fCamVertYDotFNearDivCamVertX < fBottom)
				{
					fBottom = fCamVertYDotFNearDivCamVertX;
				}
				if(fCamVertYDotFNearDivCamVertX > fTop)
				{
					fTop = fCamVertYDotFNearDivCamVertX;
				}

				if(fCamVertZDotFNearDivCamVertX < fLeft)
				{
					fLeft = fCamVertZDotFNearDivCamVertX;
				}
				if(fCamVertZDotFNearDivCamVertX > fRight)
				{
					fRight = fCamVertZDotFNearDivCamVertX;
				}

				bSignChanged = false;
				
			}

			ptLastVert = ptCamVert;			
		}

		if(iFirstSign * iLastSign < 0)
		{
			vDiff = ptFirstVert - ptLastVert;
			fT = (fEpsilon - ptLastVert.x) / vDiff.x;
			fCamVertYDotFNearDivCamVertX = (ptLastVert.y + fT * vDiff.y) *
				fInvEpsilon;
			fCamVertZDotFNearDivCamVertX = (ptLastVert.z + fT * vDiff.z) *
				fInvEpsilon;

			if(fCamVertYDotFNearDivCamVertX < fBottom)
			{
				fBottom = fCamVertYDotFNearDivCamVertX;
			}
			if(fCamVertYDotFNearDivCamVertX > fTop)
			{
				fTop = fCamVertYDotFNearDivCamVertX;
			}

			if(fCamVertZDotFNearDivCamVertX < fLeft)
			{
				fLeft = fCamVertZDotFNearDivCamVertX;
			}
			if(fCamVertZDotFNearDivCamVertX > fRight)
			{
				fRight = fCamVertZDotFNearDivCamVertX;
			}
		}
	}
	else 
	{
		for(i=0; i < m_uVertCnt; ++i)
		{
			vDiff = m_pWorldVerts[i] - pCamera->GetEye();
			ptCamVert.y = D3DXVec3Dot(&vDiff, &pCamera->GetUp());
			ptCamVert.z = D3DXVec3Dot(&vDiff, &pCamera->GetRight());

			if(ptCamVert.y < fBottom)
			{
				fBottom = ptCamVert.y;
			}
			if(ptCamVert.y > fTop)
			{
				fTop = ptCamVert.y;
			}

			if(ptCamVert.z < fLeft)
			{
				fLeft = ptCamVert.z;
			}

			if(ptCamVert.z > fRight)
			{
				fRight = ptCamVert.z;
			}

			// Test if the axis-aligned bounding rectangle is outside the current
			// frustum.  If it is, the adjoining room need not be visited. 
			if((frustum.m_fLeft >= fRight) || (frustum.m_fRight <= fLeft) || 
				(frustum.m_fBottom >= fTop) || (frustum.m_fTop <= fBottom))
			{
				return false;
			}			
		}
	}
		

		// The axis-aligned bounding rectangle intersects the current frustum.
		// Reduce the frustum for use in drawing the adjoining room.
		reduceFrustum = frustum;

		if(reduceFrustum.m_fLeft < fLeft)
		{
			reduceFrustum.m_fLeft = fLeft;
		}

		
		if(reduceFrustum.m_fRight > fRight)
		{
			reduceFrustum.m_fRight = fRight;
		}

		
		if(reduceFrustum.m_fBottom < fBottom)
		{
			reduceFrustum.m_fBottom = fBottom;
		}

		if(reduceFrustum.m_fTop > fTop)
		{
			reduceFrustum.m_fTop = fTop;
		}
	
		return true;

}


void U2Portal::GetVisibleSet(U2Culler& culler, bool bNoCull)
{
	if(!m_bOpen)
	{
		return;
	}

	if(!culler.IsVisible(m_uVertCnt, m_pWorldVerts, true))
	{
		return;
	}


	// It is possible that this portal is visited along a path of portals
	// from the current room containing the camera.  Such portals might
	// have a back-facing polygon relative to the camera.  It is not possible
	// to see through these, so cull them.
	const U2Camera* pCamera = culler.GetCamera();
	if(m_worldPlane.WhichSide(pCamera->GetEye()) < 0)
	{
		return;
	}

	// Save the current frustum.
	U2Frustum saveFrustum;
	saveFrustum = culler.GetFrustum();	

	// If the observer can see through the portal, the culler's frustum may
	// be reduced in size based on the portal geometry.
	U2Frustum reduceFrustum;
	if(ReduceFrustum(culler, reduceFrustum))
	{
		// Use the reduced frustum for drawing the adjacent region.
		culler.SetFrustumLH(reduceFrustum);

		// Visit the adjacent region and any nonculled objects in it.
		m_pAdjacentRegion->GetVisibleSet(culler, bNoCull);

		// Restore the previous frustum.
		culler.SetFrustumLH(saveFrustum);

	}
}
