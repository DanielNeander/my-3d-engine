#include <U2_3D/src/U23DLibPCH.h>
#include "U2Camera.h"


U2Camera::U2Camera()
{
	//m_pRenderer = 0;
	m_iNumSplits = 4;
	m_fSplitSchemeWeight = 0.5f;

	SetFrustum(-0.5f, 0.5f, -0.5f, 0.5f, 1.0f, 2.0f);
//	SetNearDist(0.f);
//	SetFarDist(1.f);
	SetDepthRange(0.f, 1.f);
	SetFrame(VECTOR3_ZERO, UNIT_Z, UNIT_Y, UNIT_X);
	m_viewPort = U2Rect<float>(0.f, 1.f, 1.f, 0.f);

	m_frustum.m_bOrtho = false;
	
	m_fNearMin = 1.0f;
	m_fFarMax = 400.0f;
	
}

U2Camera::~U2Camera()
{
	 
}


void U2Camera::SetFrustum (float fLeft, float fRight, float fBottom, float fTop,
				 float fNear, float fFar, bool bOrtho)
{
	m_frustum.m_fLeft = fLeft ;
	m_frustum.m_fRight = fRight; 
	m_frustum.m_fNear = fNear;
	m_frustum.m_fFar = fFar;
	m_frustum.m_fBottom = fBottom;
	m_frustum.m_fTop = fTop;
	m_frustum.m_bOrtho = bOrtho;
	// Update view Matrix
}


void U2Camera::SetFrustum (float fUpFovDegrees, float fAspectRatio, float fNear,
				 float fFar)
{
	float fHalfAngleRadians = 0.5f * DegToRad(fUpFovDegrees);
	m_frustum.m_fTop = fNear * U2Math::Tan(fHalfAngleRadians);
	m_frustum.m_fRight = fAspectRatio * m_frustum.m_fTop;
	m_frustum.m_fBottom = -m_frustum.m_fTop;
	m_frustum.m_fLeft = -m_frustum.m_fRight;
	m_frustum.m_fNear = fNear;
	m_frustum.m_fFar = fFar;

	// Update Renderer Frustum
}

bool U2Camera::GetFrustum (float& rfUpFovDegrees, float& rfAspectRatio,
				 float& rfNear, float& rfFar) const
{
	if(m_frustum.m_fLeft == -m_frustum.m_fRight 
		&& m_frustum.m_fBottom == -m_frustum.m_fTop)
	{
		float fTmp = m_frustum.m_fTop/m_frustum.m_fNear;
		rfUpFovDegrees = 2.0f * RadToDeg(U2Math::ATan(fTmp));
		rfAspectRatio = m_frustum.m_fRight/m_frustum.m_fTop;
		rfNear = m_frustum.m_fNear;
		rfFar = m_frustum.m_fFar;
		return true;
	}
	return false;
}


// calculates split plane distances in view space
void U2Camera::CalculateSplitPositions(float *pDistances)
{
	// Practical split scheme:
	//
	// CLi = n*(f/n)^(i/numsplits)
	// CUi = n + (f-n)*(i/numsplits)
	// Ci = CLi*(lambda) + CUi*(1-lambda)
	//
	// lambda scales between logarithmic and uniform
	//
	float m_fNear = m_frustum.m_fNear;
	float m_fFar = m_frustum.m_fFar;

	for(int i = 0; i < m_iNumSplits; i++)
	{
		float fIDM = i / (float)m_iNumSplits;
		float fLog = m_fNear * powf(m_fFar/m_fNear, fIDM);
		float fUniform = m_fNear + (m_fFar - m_fNear) * fIDM;
		pDistances[i] = fLog * m_fSplitSchemeWeight + fUniform * (1 - m_fSplitSchemeWeight);
	}

	// make sure border values are accurate
	pDistances[0] = m_fNear;
	pDistances[m_iNumSplits] = m_fFar;
}

// computes a frustum with given far and near planes
U2Frustum&  U2Camera::CalculateFrustum(float fNear, float fFar)
{
	float fAspectRatio = m_frustum.m_fRight/m_frustum.m_fTop;

	float fTmp = m_frustum.m_fTop/m_frustum.m_fNear;
	float fFov = RadToDeg(U2Math::ATan(fTmp));

	float fNearPlaneHalfHeight = tanf(fFov * 0.5f) * fNear;
	float fNearPlaneHalfWidth = fNearPlaneHalfHeight * fAspectRatio;

	float fFarPlaneHalfHeight = tanf(fFov * 0.5f) * fFar;
	float fFarPlaneHalfWidth = fFarPlaneHalfHeight * fAspectRatio;

	D3DXVECTOR3 vNearPlaneCenter = m_vEye + m_vDir * fNear;
	D3DXVECTOR3 vFarPlaneCenter = m_vEye + m_vDir * fFar;

	// 1 ---- 2
	// |		 |
	// 0 ---- 3
	
	
	m_frustum.m_Points[0] = D3DXVECTOR3(vNearPlaneCenter - m_vRight*fNearPlaneHalfWidth - m_vUp*fNearPlaneHalfHeight);
	m_frustum.m_Points[1] = D3DXVECTOR3(vNearPlaneCenter - m_vRight*fNearPlaneHalfWidth + m_vUp*fNearPlaneHalfHeight);
	m_frustum.m_Points[2] = D3DXVECTOR3(vNearPlaneCenter + m_vRight*fNearPlaneHalfWidth + m_vUp*fNearPlaneHalfHeight);
	m_frustum.m_Points[3] = D3DXVECTOR3(vNearPlaneCenter + m_vRight*fNearPlaneHalfWidth - m_vUp*fNearPlaneHalfHeight);

	m_frustum.m_Points[4] = D3DXVECTOR3(vFarPlaneCenter - m_vRight*fFarPlaneHalfWidth - m_vUp*fFarPlaneHalfHeight);
	m_frustum.m_Points[5] = D3DXVECTOR3(vFarPlaneCenter - m_vRight*fFarPlaneHalfWidth + m_vUp*fFarPlaneHalfHeight);
	m_frustum.m_Points[6] = D3DXVECTOR3(vFarPlaneCenter + m_vRight*fFarPlaneHalfWidth + m_vUp*fFarPlaneHalfHeight);
	m_frustum.m_Points[7] = D3DXVECTOR3(vFarPlaneCenter + m_vRight*fFarPlaneHalfWidth - m_vUp*fFarPlaneHalfHeight);

	// update frustum AABB
	m_frustum.CalculateAABB();

	return m_frustum;
}

bool U2Camera::GetPickRay (int iX, int iY, int iWidth, int iHeight,
				 D3DXVECTOR3& rkOrigin, D3DXVECTOR3& rkDirection) const
{

	float fPortX = ((float)iX)/(float)(iWidth - 1);
	if(fPortX < m_viewPort.m_left || fPortX > m_viewPort.m_right )
	{
		FDebug("Failed to Picking - 뷰포트의 안에 포함되지 않는다.");
		return false;
	}
	float fPortY = ((float)(iHeight-1-iY))/(float)(iHeight-1);
	if(fPortY < m_viewPort.m_bottom || fPortY > m_viewPort.m_top)
	{
		FDebug("Failed to Picking - 뷰포트의 안에 포함되지 않는다.");
		return false;
	}
	float fXWeight = (fPortX - m_viewPort.m_left)/(m_viewPort.m_right - m_viewPort.m_left);
	float fViewX = (1.0f-fXWeight) * m_frustum.m_fLeft + fXWeight * m_frustum.m_fRight;

	float fYWeight = (fPortY - m_viewPort.m_bottom)/(m_viewPort.m_top - m_viewPort.m_bottom);
	float fViewY = (1.0f-fYWeight) * m_frustum.m_fBottom + fYWeight * m_frustum.m_fTop;

	rkOrigin = m_vEye;
	rkDirection = m_frustum.m_fNear * m_vDir + fViewX * m_vRight + fViewY * m_vUp;
	D3DXVec3Normalize(&rkDirection, &rkDirection);
	return true;
}


void U2Camera::AdjustPlanes(U2VisibleSet& visibleObjs)
{	
	if ( 0 ==  visibleObjs.GetCount()) return;

	// find the nearest and farthest points of given
	// scene objects in camera's view space
	//
	float fMaxZ = 0;
	float fMinZ = FLT_MAX;


	for(unsigned int i=0; i < visibleObjs.GetCount(); i++)
	{
		U2VisibleObject& pObject = visibleObjs.GetVisible(i);

		// for each point in AABB
		for (int j=0; j < 8; j++)
		{
			// calculate z-coordinate distance to near plane of camera
			U2Mesh* pObj = (U2Mesh*)pObject.m_pObj;
			D3DXVECTOR3 vPointToCam = pObj->m_pBBox->m_Points[j] - m_vEye;
			float fZ = U2Math::Dot(vPointToCam, m_vDir);

			// find boundary values
			if(fZ > fMaxZ) fMaxZ = fZ;
			if(fZ < fMinZ) fMinZ = fZ;
		}
	}

	m_frustum.m_fNear = Common::Max(fMinZ, m_fNearMin);
	m_frustum.m_fFar = Common::Max(fMaxZ, m_frustum.m_fNear  + 1.0f);
	
	if (U2Dx9Renderer::GetRenderer())
	{
		U2Dx9Renderer::GetRenderer()->SetCamera(this);
		U2Dx9Renderer::GetRenderer()->SetProjectionTM(this);
	}

}