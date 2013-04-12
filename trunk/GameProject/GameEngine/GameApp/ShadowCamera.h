#ifndef __SHADOWCAMERA_H__
#define __SHADOWCAMERA_H__

#include "BaseCamera.h"

class Mesh;
class ShadowMapCamera : public BaseCamera 
{
public:
	 ShadowMapCamera(const float* from, const float* to, const float* up, float fov);

	//std::vector<Mesh*> FindReceivers(void);

	// calculates split plane distances in view space
	void CalculateSplitPositions(float *pDistances);

	// calculates a frustum with given far and near planes

	// adjust the camera planes to contain the visible scene as tightly as possible
	void AdjustPlanes(const std::vector<Mesh *> &VisibleObjects);
	std::vector<Mesh *> FindReceivers(void);
	// used when adjusting camera planes
	float m_fFarMax;
	float m_fNearMin;

};

inline ShadowMapCamera::ShadowMapCamera( const float* from, const float* to, const float* up, float fov )
	: BaseCamera(from, to, up, fov)
{
	m_fNearMin = 1.0f;
	m_fFarMax = 400.0f;
}


#endif