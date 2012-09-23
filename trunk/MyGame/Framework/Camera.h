#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

#include <Graphics/Common/Camera/hkgCameraDefines.h>

#include <Math/Vector.h>
#include <Math/Quaternion.h>

class CameraBase 
{
	inline void Orthogonalize();

	inline void ComputeProjection();

	void ComputeModelView(bool forceIdentity = false);

	/// Pan the camera long x (Right) or y (Up) by the given amount. 
	/// If you want to move the To, or focus, the same amount as well you can.
	/// The amount is the fraction of the length from To to From, so is relative 
	/// to the distance from the point of interest.
	/// This method will automatically recompute the affected matrices.
	/// If proportional is false the the x and y amount are actual distances not ratios wrt to the distance to the poi.
	void Pan(float x_amount, float y_amount, bool move_focus = false, bool proportional = true);

	/// Dolly the camera in or out along the Dir vector. The amount is the relative amount to
	/// move wrt to the distance from To to From. 
	/// This method will automatically recompute the affected matrices.
	/// If proportional is false the the z amount is an actual distance not ratios wrt to the distance to the poi.
	void Dolly(float z_amount, bool move_focus = false, bool proportional = true);

	inline const noVec3& GetFrom() const;
	inline const noVec3& GetTo() const;
	inline const noVec3 GetDir() const;
	inline const noVec3 GetRight() const;
	inline const noVec3& GetUp() const;
	inline float GetFOV() const;
	inline float GetNear() const;
	inline float GetFar() const;
	inline float GetAspect() const;		

	/// Compute the orthographic projection matrix give the 6 plane distances. Will set the cameras orthographic parameters to match the give values too. 
	/// This method is called automatically by computeProjection() if needed so there should be no need to call it directly.
	void ComputeOrtho( float left, float right, float bottom, float top, float znear, float zfar);

	/// Compute the orthographic projection parameters based on keeping the To point in view, with the current FoV and From point. Will set the cameras
	/// orthographic parameters to match the computed values. This method is called automatically by computeProjection() if needed so there should be no 
	/// need to call it directly.
	void ComputeOrthoParams();

	/// Compute the perspective matrix based on the current camera Fov, Aspect and clip planes.
	void ComputePerspective( float fov, float aspect, float znear, float zfar);

	/// Compute the planes that represent the frustum. These planes can then be used to clip against for visibility.
	void ComputeFrustumPlanes() const;	

	void ComputeCameraInverse() const; // will store as m_camInv locally. Will call the following 2 funcs



private:
	HKG_CAMERA_PROJECTION_MODE m_mode;


	noVec3	m_from;
	noVec3	m_to;
	noVec3	m_up;
	float		m_fov;
	float		m_near; 
	float		m_far; 
	float		m_aspect;
	
	float		m_oleft;
	float		m_oright;
	float		m_otop;
	float		m_obottom;
	float		m_onear;
	float		m_ofar;

	noVec3		m_trackballVec;

	float m_viewMat[16];
	float m_projMat[16];

	mutable bool		m_camInvDirty;
	mutable bool		m_frustumPlanesDirty;

	mutable float		m_frustumPlanes[6][4]; 

	static noVec3 DEFAULT_FROM;
	static noVec3 DEFAULT_TO;
	static noVec3 DEFAULT_UP;
	static float DEFAULT_FOV;	
};


inline const noVec3& CameraBase::GetFrom() const
{
	return m_from;
}

inline const noVec3& CameraBase::GetTo() const
{
	return m_to;
}

inline const noVec3 CameraBase::GetDir() const
{
	noVec3 dir(m_to - m_from);
	dir.Normalize();
	return dir;
}

inline const noVec3 CameraBase::GetRight() const
{
	noVec3 dir = GetDir();
	//left hand 
	//return m_up.Cross(dir);
	//right hand 
	return dir.Cross(m_up);
}

inline const noVec3& CameraBase::GetUp() const {
	return m_up;
}

inline float CameraBase::GetFOV() const
{
	return m_fov;
}

inline float CameraBase::GetNear() const
{
	return m_near;
}

inline float CameraBase::GetFar() const
{
	return m_far;
}

inline float CameraBase::GetAspect() const
{
	return m_aspect;
}

inline void CameraBase::ComputeProjection()
{
	switch (m_mode)
	{
	case HKG_CAMERA_PERSPECTIVE: 
		ComputePerspective( m_fov, m_aspect, m_near, m_far);
		break;
	case HKG_CAMERA_ORTHOGRAPHIC: 
		ComputeOrtho(m_oleft, m_oright, m_obottom, m_otop, m_onear, m_ofar);
		break;
	case HKG_CAMERA_COMPUTE_ORTHOGRAPHIC: 
		ComputeOrthoParams();
		ComputeOrtho(m_oleft, m_oright, m_obottom, m_otop, m_onear, m_ofar);
		break;
	default: break;
	}
}



class ThirdPersonCamera : public CameraBase	{

public:

};

//class FirstPersonCamera
//{
//
//};

class FlyCamera : public CameraBase {

public:


};

#endif