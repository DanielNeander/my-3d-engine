#include <U2_3D/src/U23DLibPCH.h>
#include "U2Culler.h"
#include "U2BoundingVolume.h"
#include <U2_3D/src/main/U2SphereBV.h>


U2Culler::U2Culler(int maxCnt /* =0 */, int iGrowby /* = 0 */, const U2Camera* pCamera /* = 0 */)
	:m_pVisibles(U2_NEW U2VisibleSet(maxCnt, iGrowby)),
	m_pCamera(pCamera)
{
	m_uPlaneCnt = 6;

	// Nvidia Pratical PSM..
	//  build a bit-field that will tell us the indices for the nearest and farthest vertices from each plane...
	for (int i=0; i<m_uPlaneCnt; i++)
		nVertexLUT[i] = ((m_aPlanes[i].m_vNormal[0]<0.f)?1:0) | ((m_aPlanes[i].m_vNormal[1]<0.f)?2:0) | ((m_aPlanes[i].m_vNormal[2]<0.f)?4:0);
}

U2Culler::~U2Culler()
{
	U2_DELETE m_pVisibles;
	m_pVisibles = NULL;
}



void U2Culler::SetCamera (const U2Camera* pkCamera)
{
	m_pCamera = pkCamera;
}


const U2Camera* U2Culler::GetCamera () const
{
	return m_pCamera;
}


//void U2Culler::SetFrustum (const float* afFrustum)
//{
//	
//}
//

const U2Frustum& U2Culler::GetFrustum () const
{
	return m_frustum;
}


U2VisibleSet& U2Culler::GetVisibleSet ()
{
	return *m_pVisibles;
}


void U2Culler::Insert(U2Spatial* obj, U2Effect* globalEffect)
{
	m_pVisibles->Insert(obj, globalEffect);
}


void U2Culler::SetFrustumLH (const U2Frustum& frustum)
{
	U2ASSERT(m_pCamera);

	if(!m_pCamera)
	{
		return;
	}

	size_t uSize = 6 * sizeof(float);
	memcpy_s(&m_frustum, uSize, &frustum, uSize);
	float fNear2 = m_frustum.m_fNear * m_frustum.m_fNear;
	float fLeft2 = m_frustum.m_fLeft * m_frustum.m_fLeft;
	float fRight2 = m_frustum.m_fRight * m_frustum.m_fRight;
	float fBottom2 = m_frustum.m_fBottom * m_frustum.m_fBottom;
	float fTop2 = m_frustum.m_fTop * m_frustum.m_fTop;


	D3DXVECTOR3 vLoc = m_pCamera->GetEye();
	D3DXVECTOR3 vDir = m_pCamera->GetDir();
	D3DXVECTOR3 vUp = m_pCamera->GetUp();
	D3DXVECTOR3 vRight = m_pCamera->GetRight();
	float fDdE = D3DXVec3Dot(&vDir, &vLoc);

	// Update Near Plane
	m_aPlanes[NEAR_PLANE].m_vNormal = vDir;
	m_aPlanes[NEAR_PLANE].m_fDistance = -(fDdE + m_frustum.m_fNear);

	// Update Far Plane
	m_aPlanes[FAR_PLANE].m_vNormal = -vDir;
	m_aPlanes[FAR_PLANE].m_fDistance = 
		(fDdE + m_frustum.m_fFar);

	// Update  Bottom Plane
	float fInvLen = 1.0f / U2Math::Sqrt(fNear2 + fBottom2);
	float fC0 = -m_frustum.m_fBottom * fInvLen; 
	float fC1 = +m_frustum.m_fNear * fInvLen;
	m_aPlanes[BOTTOM_PLANE].m_vNormal = fC0 * vDir + fC1 * vUp;
	m_aPlanes[BOTTOM_PLANE].m_fDistance = 
		-D3DXVec3Dot(&vLoc, &m_aPlanes[BOTTOM_PLANE].m_vNormal);

	// Update Top Plane
	fInvLen = 1.0f / U2Math::Sqrt(fNear2 + fTop2);
	fC0 = +m_frustum.m_fTop * fInvLen; 
	fC1 = -m_frustum.m_fNear * fInvLen;
	m_aPlanes[TOP_PLANE].m_vNormal = fC0 * vDir + fC1 * vUp;
	m_aPlanes[TOP_PLANE].m_fDistance = 
		-D3DXVec3Dot(&vLoc, &m_aPlanes[TOP_PLANE].m_vNormal);

	// Update Left Plane
	fInvLen = 1.0f / U2Math::Sqrt(fNear2 + fLeft2);
	fC0 = -m_frustum.m_fLeft * fInvLen; 
	fC1 = +m_frustum.m_fNear * fInvLen;
	m_aPlanes[LEFT_PLANE].m_vNormal = fC0 * vDir + fC1 * vRight;
	m_aPlanes[LEFT_PLANE].m_fDistance = 
		-D3DXVec3Dot(&vLoc, &m_aPlanes[LEFT_PLANE].m_vNormal);

	// Update Right Plane
	fInvLen = 1.0f / U2Math::Sqrt(fNear2 + fRight2);
	fC0 = +m_frustum.m_fRight * fInvLen; 
	fC1 = -m_frustum.m_fNear * fInvLen;
	m_aPlanes[RIGHT_PLANE].m_vNormal = fC0 * vDir + fC1 * vRight;
	m_aPlanes[RIGHT_PLANE].m_fDistance = 
		-D3DXVec3Dot(&vLoc, &m_aPlanes[RIGHT_PLANE].m_vNormal);

	m_uPlaneState = 0xffffffff;
}


void U2Culler::SetFrustumRH (const U2Frustum& frustum)
{
	U2ASSERT(m_pCamera);

	if(!m_pCamera)
	{
		return;
	}

	size_t uSize = 6 * sizeof(float);
	memcpy_s(&m_frustum, uSize, &frustum, uSize);
	float fNear2 = m_frustum.m_fNear * m_frustum.m_fNear;
	float fLeft2 = m_frustum.m_fLeft * m_frustum.m_fLeft;
	float fRight2 = m_frustum.m_fRight * m_frustum.m_fRight;
	float fBottom2 = m_frustum.m_fBottom * m_frustum.m_fBottom;
	float fTop2 = m_frustum.m_fTop * m_frustum.m_fTop;


	D3DXVECTOR3 vLoc = m_pCamera->GetEye();
	D3DXVECTOR3 vDir = m_pCamera->GetDir();
	D3DXVECTOR3 vUp = m_pCamera->GetUp();
	D3DXVECTOR3 vRight = m_pCamera->GetRight();
	float fDdE = D3DXVec3Dot(&vDir, &vLoc);

	// Update Near Plane
	m_aPlanes[NEAR_PLANE].m_vNormal = vDir;
	m_aPlanes[NEAR_PLANE].m_fDistance = fDdE + m_frustum.m_fNear;

	// Update Far Plane
	m_aPlanes[FAR_PLANE].m_vNormal = -vDir;
	m_aPlanes[FAR_PLANE].m_fDistance = 
		-(fDdE + m_frustum.m_fFar);

	// Update  Bottom Plane
	float fInvLen = 1.0f / U2Math::Sqrt(fNear2 + fBottom2);
	float fC0 = -m_frustum.m_fBottom * fInvLen; 
	float fC1 = +m_frustum.m_fNear * fInvLen;
	m_aPlanes[BOTTOM_PLANE].m_vNormal = fC0 * vDir + fC1 * vUp;
	m_aPlanes[BOTTOM_PLANE].m_fDistance = 
		D3DXVec3Dot(&vLoc, &m_aPlanes[BOTTOM_PLANE].m_vNormal);

	// Update Top Plane
	fInvLen = 1.0f / U2Math::Sqrt(fNear2 + fTop2);
	fC0 = +m_frustum.m_fTop * fInvLen; 
	fC1 = -m_frustum.m_fNear * fInvLen;
	m_aPlanes[TOP_PLANE].m_vNormal = fC0 * vDir + fC1 * vUp;
	m_aPlanes[TOP_PLANE].m_fDistance = 
		D3DXVec3Dot(&vLoc, &m_aPlanes[TOP_PLANE].m_vNormal);

	// Update Left Plane
	fInvLen = 1.0f / U2Math::Sqrt(fNear2 + fLeft2);
	fC0 = -m_frustum.m_fLeft * fInvLen; 
	fC1 = +m_frustum.m_fNear * fInvLen;
	m_aPlanes[LEFT_PLANE].m_vNormal = fC0 * vDir + fC1 * vRight;
	m_aPlanes[LEFT_PLANE].m_fDistance = 
		D3DXVec3Dot(&vLoc, &m_aPlanes[LEFT_PLANE].m_vNormal);

	// Update Right Plane
	fInvLen = 1.0f / U2Math::Sqrt(fNear2 + fRight2);
	fC0 = +m_frustum.m_fRight * fInvLen; 
	fC1 = -m_frustum.m_fNear * fInvLen;
	m_aPlanes[RIGHT_PLANE].m_vNormal = fC0 * vDir + fC1 * vRight;
	m_aPlanes[RIGHT_PLANE].m_fDistance = 
		D3DXVec3Dot(&vLoc, &m_aPlanes[RIGHT_PLANE].m_vNormal);

	m_uPlaneState = 0xffffffff;
}



void U2Culler::ComputeVisibleSet (U2Spatial* pkScene)
{
	U2ASSERT(m_pCamera && pkScene);
	if(m_pCamera && pkScene)
	{
		SetFrustumLH(m_pCamera->GetFrustum());
		m_pVisibles->RemoveAll();
		pkScene->OnVisibleSet(*this, false);
	}
}


bool U2Culler::IsVisible (const U2BoundingVolume* pkBound)
{
	// Start with the last pushed plane, which is potentially the most
	// restrictive plane.
	int iP = m_uPlaneCnt - 1;
	unsigned int uiMask = 1 << iP;

	for (uint32 i = 0; i < m_uPlaneCnt; i++, iP--, uiMask >>= 1)
	{
		if (m_uPlaneState & uiMask)
		{
			int iSide = pkBound->WhichSide(m_aPlanes[iP]);

			if (iSide < 0)
			{
				// Object is on negative side.  Cull it.
				return false;
			}

			if (iSide > 0)
			{
				// Object is on positive side of plane.  There is no need to
				// compare subobjects against this plane, so mark it as
				// inactive.
				m_uPlaneState &= ~uiMask;
			}
		}
	}

	return true;
}


int U2Culler::GetPlaneQuantity () const
{
	return m_uPlaneCnt;
}


const U2Plane* U2Culler::GetPlanes () const
{
	return m_aPlanes;
}


void U2Culler::SetPlaneState (unsigned int uiPlaneState)
{
	m_uPlaneState = uiPlaneState;
}


unsigned int U2Culler::GetPlaneState () const
{
	return m_uPlaneState;
}


void U2Culler::PushPlane (const U2Plane& rkPlane)
{
	if(m_uPlaneCnt < MAX_PLANE_QUANTITY)
	{
		m_aPlanes[m_uPlaneCnt++] = rkPlane;
	}
}

void U2Culler::PopPlane ()
{
	if(m_uPlaneCnt > 6)
	{
		m_uPlaneCnt--;
	}
}


bool U2Culler::IsVisible(uint32 uVertCnt, const D3DXVECTOR3* pVerts, bool bIgnoreNearPlane)
{
	// The Boolean variable bIgnoreNearPlane should be set to 'true' when
	// the test polygon is a portal.  This avoids the situation when the
	// portal is in the view pyramid (eye+left/right/top/bottom), but is
	// between the eye and near plane.  In such a situation you do not want
	// the portal system to cull the portal.  This situation typically occurs
	// when the camera moves through the portal from current region to
	// adjacent region.

	// Start with last pushed plane, which is potentially the most
	// restrictive plane.
	uint32 uPlaneCnt = m_uPlaneCnt - 1;
	for(uint32 i=0; i < m_uPlaneCnt; ++i, --uPlaneCnt)
	{
		U2Plane& plane = m_aPlanes[uPlaneCnt];
		if(bIgnoreNearPlane && uPlaneCnt == 0)
		{
			continue;
		}

		uint32 j;
		for(j=0; j < uVertCnt; ++j)
		{
			int iSide = plane.WhichSide(pVerts[j]);
			if(iSide >= 0)
			{
				// polygon is not totally outside this plane
				break;
			}
		}

		if(j == uVertCnt)
		{
			// polygon is totally outside this plane
			return false;
		}
	}

	return true;
}



int U2Culler::WhichSide(const U2Plane& plane) const
{
	// The plane is N*(X-C) = 0 where the * indicates dot product.  The signed
	// distance from the camera location E to the plane is N*(E-C).
	float fNDotEmC = plane.GetDistance(m_pCamera->GetEye());

	float fNDotDir = D3DXVec3Dot(&plane.m_vNormal, &m_pCamera->GetDir());
	float fNDotUp =  D3DXVec3Dot(&plane.m_vNormal, &m_pCamera->GetUp()); 
	float fNDotRight =  D3DXVec3Dot(&plane.m_vNormal, &m_pCamera->GetRight()); 
	float fFarDivNear = m_frustum.m_fFar / m_frustum.m_fNear;

	int iPositive =0, iNegative = 0;
	float fSignDist;

	 // check near-plane vertices
	float fPNear = m_frustum.m_fNear * fNDotDir;
	float fNBottom = m_frustum.m_fBottom * fNDotUp;
	float fNTop = m_frustum.m_fTop * fNDotUp;
	float fNLeft = m_frustum.m_fLeft * fNDotRight;
	float fNRight = m_frustum.m_fRight * fNDotRight;

	// N : plane의 Normal
	// C : plane의 distance
	// E : camera의 Eye
	// D : camera의 Dir 
	// U : camera의 Up 
	// R : camera의 Right

	// V = E + near*D + bottom*U + left*R
	// N*(V-C) = N*(E-C) + near*(N*D) + bottom*(N*U) + left*(N*R)
	fSignDist = fNDotEmC + fPNear + fNBottom + fNLeft;
	if(fSignDist > 0.f)
	{
		iPositive++;
	}
	else if(fSignDist < 0.f)
	{
		iNegative++;
	}

	// V = E + near*D + bottom*U + right*R
	// N*(V-C) = N*(E-C) + near*(N*D) + bottom*(N*U) + right*(N*R)
	fSignDist = fNDotEmC + fPNear + fNBottom + fNRight;
	if(fSignDist > 0.f)
	{
		iPositive++;
	}
	else if(fSignDist < 0.f)
	{
		iNegative++;
	}

	// V = E + near*D + top*U + left*R
	// N*(V-C) = N*(E-C) + near*(N*D) + top*(N*U) + left*(N*R)
	fSignDist = fNDotEmC + fPNear + fNTop + fNLeft;
	if(fSignDist > 0.f)
	{
		iPositive++;
	}
	else if(fSignDist < 0.f)
	{
		iNegative++;
	}

	// V = E + near*D + top*U + right*R
	// N*(V-C) = N*(E-C) + near*(N*D) + top*(N*U) + right*(N*R)
	fSignDist = fNDotEmC + fPNear + fNTop + fNRight;
	if(fSignDist > 0.f)
	{
		iPositive++;
	}
	else if(fSignDist < 0.f)
	{
		iNegative++;
	}


	// check far-plane vertices
	float fPFar = m_frustum.m_fFar * fNDotDir;
	float fFBottom = fFarDivNear * fNBottom;
	float fFTop = fFarDivNear * fNTop;
	float fFLeft = fFarDivNear * fNLeft;
	float fFRight = fFarDivNear * fNRight;

	// N : plane의 Normal
	// C : plane의 distance
	// E : camera의 Eye
	// D : camera의 Dir 
	// U : camera의 Up 
	// R : camera의 Right

	// V = E + near*D + bottom*U + left*R
	// N*(V-C) = N*(E-C) + near*(N*D) + bottom*(N*U) + left*(N*R)
	fSignDist = fNDotEmC + fPFar + fFBottom + fFLeft;
	if(fSignDist > 0.f)
	{
		iPositive++;
	}
	else if(fSignDist < 0.f)
	{
		iNegative++;
	}

	// V = E + near*D + bottom*U + right*R
	// N*(V-C) = N*(E-C) + near*(N*D) + bottom*(N*U) + right*(N*R)
	fSignDist = fNDotEmC + fPFar + fFBottom + fFRight;
	if(fSignDist > 0.f)
	{
		iPositive++;
	}
	else if(fSignDist < 0.f)
	{
		iNegative++;
	}

	// V = E + near*D + top*U + left*R
	// N*(V-C) = N*(E-C) + near*(N*D) + top*(N*U) + left*(N*R)
	fSignDist = fNDotEmC + fPFar + fFTop + fFLeft;
	if(fSignDist > 0.f)
	{
		iPositive++;
	}
	else if(fSignDist < 0.f)
	{
		iNegative++;
	}

	// V = E + near*D + top*U + right*R
	// N*(V-C) = N*(E-C) + near*(N*D) + top*(N*U) + right*(N*R)
	fSignDist = fNDotEmC + fPFar + fFTop + fFRight;
	if(fSignDist > 0.f)
	{
		iPositive++;
	}
	else if(fSignDist < 0.f)
	{
		iNegative++;
	}


	if(iPositive > 0)
	{
		if(iNegative > 0)
		{
			// frustum straddles the plane
			return 0;
		}

		// frustum is fully on the positive side
		return +1;
	}

	// frustum is fully on the negative side
	return -1;
}

//  this function tests if the projection of a bounding sphere along the light direction intersects
//  the view frustum 

bool U2Culler::SweptSpherePlaneIntersect(float& t0, float& t1, const U2Plane& plane, const U2SphereBV* pSphereBV,
							   const D3DXVECTOR3* sweepDir) const
{
	float b_dot_n = plane.GetDistance(pSphereBV->GetCenter());

	U2Plane distZeroPlane = plane;
	distZeroPlane.m_fDistance = 0.0f;

	float d_dot_n = distZeroPlane.GetDistance(*sweepDir);

	if (d_dot_n == 0.f)
	{
		if (b_dot_n <= pSphereBV->GetRadius())
		{
			// effectively infinity
			t0 = 0.f;
			t1 = 1e32f;
			return true;
		}
		else 
			return false;
	}
	else 
	{
		float tmp0 = ( pSphereBV->GetRadius() - b_dot_n) / d_dot_n;
		float tmp1 = ( pSphereBV->GetRadius() - b_dot_n) / d_dot_n;
		t0 = min(tmp0, tmp1);
		t1 = max(tmp0, tmp1);
		return true;
	}
}

bool U2Culler::TestSweptSphere(const U2SphereBV *sphereBV, const D3DXVECTOR3 *sweepDir) const
{
	//  algorithm -- get all 12 intersection points of the swept sphere with the view frustum
	//  for all points >0, displace sphere along the sweep driection.  if the displaced sphere
	//  is inside the frustum, return TRUE.  else, return FALSE
	float displacements[12];
	int cnt = 0;
	float a,b;
	bool inFrustum = false;
	
	for (int i = 0; i < 6; i++)
	{
		if ( SweptSpherePlaneIntersect(a, b, m_aPlanes[i], sphereBV, sweepDir))
		{
			if (a >= 0.f)
				displacements[cnt++] = a;
			if (b >= 0.f)
				displacements[cnt++] = b;
		}
	}

	for (int i = 0; i < cnt; i++)
	{
	/*	U2SphereBV displacedSphere(*sphereBV);
		displacedSphere.SetCenter( displacedSphere.GetCenter() + (*sweepDir) * displacements[i] );
		displacedSphere.SetRadius( displacedSphere.GetRadius() * 1.1f );
		inFrustum |= this->IsVisible(&displacedSphere);*/
	}

	  return inFrustum;
}

