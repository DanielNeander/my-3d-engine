#include "stdafx.h"
#include "ShadowCamera.h"
#include "EngineCore/Math/IntersectionTests.h"
#include "EngineCore/Math/Frustum.h"
#include "DemoSetup.h"
#include "Mesh.h"

extern std::vector<Mesh*>	gShadowReceivers;

// finds scene objects inside the camera frustum
std::vector<Mesh *> ShadowMapCamera::FindReceivers(void)
{
	Frustum cameraFrustum = CalculateFrustum(m_fNearMin, m_fFarMax);
	cameraFrustum.CalculateAABB();

	std::vector<Mesh *> receivers;
	receivers.reserve(gShadowReceivers.size());
	for(unsigned int i=0; i<gShadowReceivers.size(); i++)
	{
		Mesh *pObject = gShadowReceivers[i];

		// intersection test
		if(g_iVisibilityTest == VISTEST_ACCURATE) {
			// test accurately
			if(!IntersectionTest(pObject->m_AABB, cameraFrustum)) continue;
		} else if(g_iVisibilityTest == VISTEST_CHEAP) {
			// test only with AABB of frustum
			if(!IntersectionTest(pObject->m_AABB, cameraFrustum.m_AABB)) continue;
		}

		receivers.push_back(pObject);
	}
	return receivers;
}


// calculates split plane distances in view space
void ShadowMapCamera::CalculateSplitPositions( float *pDistances )
{
	// Practical split scheme:
	//
	// CLi = n*(f/n)^(i/numsplits)
	// CUi = n + (f-n)*(i/numsplits)
	// Ci = CLi*(lambda) + CUi*(1-lambda)
	//
	// lambda scales between logarithmic and uniform
	//

	for(int i = 0; i < g_iNumSplits; i++)
	{
		float fIDM = i / (float)g_iNumSplits;
		float fLog = m_near * powf(m_far/m_near, fIDM);
		float fUniform = m_near + (m_far - m_near) * fIDM;
		pDistances[i] = fLog * g_fSplitSchemeWeight + fUniform * (1 - g_fSplitSchemeWeight);
	}

	// make sure border values are accurate
	pDistances[0] = m_near;
	pDistances[g_iNumSplits] = m_far;
}



// adjust the camera planes to contain the visible scene as tightly as possible
void ShadowMapCamera::AdjustPlanes( const std::vector<Mesh *> &VisibleObjects )
{
	if(VisibleObjects.size() == 0) return;

	// find the nearest and farthest points of given
	// scene objects in camera's view space
	//
	float fMaxZ = 0;
	float fMinZ = FLT_MAX;

	noVec3 vDir = Normalize(m_to - m_from);

	// for each object
	for(unsigned int i = 0; i < VisibleObjects.size(); i++)
	{
		Mesh *pObject = VisibleObjects[i];

		// for each point in AABB
		for(int j = 0; j < 8; j++)
		{
			// calculate z-coordinate distance to near plane of camera
			noVec3 vPointToCam = pObject->m_AABB.m_pPoints[j] - m_from;
			float fZ = Dot(vPointToCam, vDir);

			// find boundary values
			if(fZ > fMaxZ) fMaxZ = fZ;
			if(fZ < fMinZ) fMinZ = fZ;
		}
	}

	// use smallest distance as new near plane
	// and make sure it is not too small
	m_near = Max(fMinZ, m_fNearMin);
	// use largest distance as new far plane
	// and make sure it is larger than nearPlane
	m_far = Max(fMaxZ, m_near + 1.0f);
	// update matrices

	computeModelView();
	ComputeProjection();
}
