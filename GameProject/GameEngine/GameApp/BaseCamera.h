#pragma once 

//#include "SceneNode.h"
#include "GameObject.h"
#include "GameObjectComponent.h"
#include "GameObjectUtil.h"
#include <EngineCore/Math/Frustum.h>


template <class T> class NiRect  
{
public:
	inline NiRect (T left = T(0), T right = T(0), T top = T(0), T bottom = T(0))
	{
		m_left = left;
		m_right = right;
		m_top = top;
		m_bottom = bottom;
	}

	T m_left, m_right, m_top, m_bottom;

	inline T GetWidth() const;
	inline T GetHeight() const;

	// streaming
	/*inline void LoadBinary (NiStream& stream);
	inline void SaveBinary (NiStream& stream);
	inline char* GetViewerString (const char* pPrefix) const;*/
};

//---------------------------------------------------------------------------
template <class T>
inline T NiRect<T>::GetWidth() const
{
	return m_right > m_left ? (m_right - m_left) : (m_left - m_right);
}
//---------------------------------------------------------------------------
template <class T>
inline T NiRect<T>::GetHeight() const
{
	return m_top > m_bottom ? (m_top - m_bottom) : (m_bottom - m_top);
}
//---------------------------------------------------------------------------


typedef unsigned short CAMERA_PROJECTION_MODE;
#define CAMERA_PERSPECTIVE  1
#define CAMERA_ORTHOGRAPHIC 2
#define CAMERA_COMPUTE_ORTHOGRAPHIC 4 // orthographic, but get settings from world size given to pos, fov, etc

#define CAMERA_DEFAULT_NEAR_PLANE   1
#define CAMERA_DEFAULT_FAR_PLANE    5000
#define CAMERA_STANDARD_ASPECT		(4.0f/3.0f)
#define CAMERA_WIDESCREEN_ASPECT	(16.0f/9.0f)
#define CAMERA_DEFAULT_ASPECT		(CAMERA_STANDARD_ASPECT)

typedef unsigned short CAMERA_HANDEDNESS_MODE;
#define CAMERA_HANDEDNESS_RIGHT     1 // default, ala OpenGL
#define CAMERA_HANDEDNESS_LEFT      2

class Frustum;

class BaseCamera : public GameObject
{
public:		
	virtual void UpdateCamera(float fDelta) {}
	
	Frustum CalculateFrustum(float fNear, float fFar);
	
	inline void orthogonalize();	
	inline void ComputeProjection();
	void computeModelView(bool forceIdentity = false);
	void computeModelView( const noMat3& rot, bool forceIdentity = false );
	inline  CAMERA_PROJECTION_MODE getProjectionMode() const;
	inline  void setProjectionMode(CAMERA_PROJECTION_MODE m);
	inline  CAMERA_HANDEDNESS_MODE getHandednessMode() const;
	inline  void setHandednessMode(CAMERA_HANDEDNESS_MODE m);

	/// Project a given (x,y,z) world point into normalized camera space and then scale the 
	/// resultant to the given window width and height. The (x,y) in result (result[0] and [1]) 
	/// are the window x and y coords, which may be outside of the window bounds. The z in result 
	/// (result[2]) is the depth from 0 to 1, where 0 is nearest. 
	void project( float x, float y, float z, int winWidth, int winHeight, noVec3& result ) const;
	void project( float x, float y, float z, int winWidth, int winHeight, const noMat4& world, noVec3& result ) const;

	/// Unproject a window point (x,y,depth) into a world position, where x and y are in window units 
	/// and the window width and height are given. If you unproject (x,y,0) and (x,y,1) you will get a 
	/// line segment with which to pick for example. The method works with single precission floating
	/// point numbers only.
	void unProject( int x, int y, float depth, int winWidth, int winHeight, noVec3& result ) const;
	void unProject( int x, int y, float depth, int winWidth, int winHeight, const noMat4& invWorld, noVec3& result ) const;

	/// This is the same as the unProject() method, but uses double precision floats internally, so is 
	/// slightly more accurate if that is required.
	void unProjectExact( int x, int y, float depth, int winWidth, int winHeight,  float result[3] ) const;

	void pan(float x_amount, float y_amount, bool move_focus = false, bool proportional = true);
	void dolly(float z_amount, bool move_focus = false, bool proportional = true);

	void rotateAboutTo(float angle, const float axis[3], bool upLock = false);
	void rotateAboutFrom(float angle, const float axis[3], bool upLock = false);

	void startTrackball( unsigned int winWidth, unsigned int winHeight, int mouseX, int mouseY );

	void moveTrackball( unsigned int winWidth, unsigned int winHeight, int mouseX, int mouseY );

	inline void		setFrom(float f[3]);
	inline void		setTo(float t[3]);
	inline void		setUp(float u[3]);
	inline void		setFOV(float f) { m_fov = f; }

	inline void		getFrom(float f[3]) const;				
	inline void		getTo(float t[3]) const;
	inline void		getDir(float d[3]) const;
	inline void		getRight(float r[3]) const;

	inline const noVec3& GetFrom() const;
	inline const noVec3& GetTo() const;
	inline const noVec3 GetDir() const;
	inline const noVec3 GetRight() const;
	inline const noVec3& GetUp() const;
	inline float GetFOV() const;
	inline float GetNear() const;
	inline float GetFar() const;
	inline float GetAspect() const;		

	/// Set the camera Far plane, in units from the From point along Dir. This must be positive due to a GameCube restriction. Used by Perspective mode or Compute Orthographic mode only.
	inline void		setFar(float f);

	/// Set the camera Near plane, in units from the From point along Dir. This must be positive due to a GameCube restriction. Used by Perspective mode or Compute Orthographic mode only.
	inline void		setNear(float f);

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

	void computeInverseProjectionMatrix( float inv[16] ) const;

	/// Compute the inverse of the view matrix, of set form (assumes correct matrix type). Returns 
	/// the inverse in the passed matrix, and does not set any camera parameters. 
	void computeInverseViewMatrix( float inv[16] ) const;


	inline const float* getProjectionMatrix() const;
	inline const float* getProjectionMatrix2() const;

	/// Returns the pointer to the view matrix stored in this camera, as computed by computeModelView()
	inline const float* getViewMatrix() const;
	inline const float* getViewMatrix2() const;
	inline void getViewMatrix(XMMATRIX& viewMat);
	inline const noMat4& getInvCameraMatrix() const { return m_invView; }

	inline BaseCamera(const float* from, const float* to, const float* up, float fov);
	inline void SetAspect(float a);	

	inline noVec3 GetWorldRight() const;
	inline noVec3 GetWorldUp() const;
	inline noVec3 GetWorldForward() const;
	inline noVec3 GetWorldPos() const;

	bool WindowPointToRay(long lWx, long lWy, noVec3& kOrigin, noVec3& kDir, uint32 width, uint32 height) const;
	void ViewPointToRay(float fVx, float fVy, noVec3& kOrigin, noVec3& kDir) const;

protected:
	void UpdateInternal(float fDelta);
	CAMERA_PROJECTION_MODE m_mode;
	CAMERA_HANDEDNESS_MODE m_handedness;

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

	NiRect<float>	m_kPort;


	noVec3		m_trackballVec;

	/*__declspec(align(16))*/ float m_viewMat[16];
	/*__declspec(align(16))*/ float m_projMat[16];
	//__declspec(align(16)) float m_camInv[16];

	noMat4				m_proj;
	noMat4				m_view;
	mutable noMat4		m_invView;
	

	mutable bool		m_camInvDirty;
	mutable /*__declspec(align(16))*/ float m_camInv[16];
	mutable bool		m_frustumPlanesDirty;

	mutable float		m_frustumPlanes[6][4]; 

	static noVec3 DEFAULT_FROM;
	static noVec3 DEFAULT_TO;
	static noVec3 DEFAULT_UP;
	static float DEFAULT_FOV;	

	Frustum	m_kViewFrustum;
};

//noWeakPointer(BaseCamera);
MSmartPointer(BaseCamera);


inline BaseCamera::BaseCamera( const float* from, const float* to, const float* up, float fov )
	:	GameObject("FPSCam"), m_mode(CAMERA_PERSPECTIVE),
	m_handedness(CAMERA_HANDEDNESS_RIGHT),
	m_fov(fov),
	m_near(CAMERA_DEFAULT_NEAR_PLANE),
	m_far(CAMERA_DEFAULT_FAR_PLANE),
	m_aspect(CAMERA_DEFAULT_ASPECT),
	//m_cameraName(HK_NULL),
	m_camInvDirty(true), m_frustumPlanesDirty(true)
{
	const unsigned int size3 = sizeof(float)*3;
	::memcpy(&m_from.x, from, size3);
	::memcpy(&m_to.x, to, size3);
	::memcpy(&m_up.x, up, size3);

	m_kPort.m_top = 1.0f;
	m_kPort.m_bottom = 0.0f;
	m_kPort.m_left = 0.0f;
	m_kPort.m_right = 1.0f;

	orthogonalize();

	computeModelView();
	ComputeProjection();
}


inline const noVec3& BaseCamera::GetFrom() const
{
	return m_from;
}

inline const noVec3& BaseCamera::GetTo() const
{
	return m_to;
}

inline const noVec3 BaseCamera::GetDir() const
{
	noVec3 dir(m_to - m_from);
	dir.Normalize();
	return dir;
}

inline const noVec3 BaseCamera::GetRight() const
{
	noVec3 dir = GetDir();
	//left hand 
	//return m_up.Cross(dir);
	//right hand 
	return dir.Cross(m_up);
}

inline const noVec3& BaseCamera::GetUp() const {
	return m_up;
}

inline float BaseCamera::GetFOV() const
{
	return m_fov;
}

inline float BaseCamera::GetNear() const
{
	return m_near;
}

inline float BaseCamera::GetFar() const
{
	return m_far;
}

inline float BaseCamera::GetAspect() const
{
	return m_aspect;
}

inline void BaseCamera::SetAspect(float a) 
{
	m_aspect = a;
}

inline void BaseCamera::ComputeProjection()
{
	switch (m_mode)
	{
	case CAMERA_PERSPECTIVE: 
		ComputePerspective( m_fov, m_aspect, m_near, m_far);
		break;
	case CAMERA_ORTHOGRAPHIC: 
		ComputeOrtho(m_oleft, m_oright, m_obottom, m_otop, m_onear, m_ofar);
		break;
	case CAMERA_COMPUTE_ORTHOGRAPHIC: 
		ComputeOrthoParams();
		ComputeOrtho(m_oleft, m_oright, m_obottom, m_otop, m_onear, m_ofar);
		break;
	default: break;
	}

	memcpy_s(&m_proj, SIZE16, m_projMat, SIZE16);
	//m_proj.TransposeSelf();

	noMat4 transposeProj = m_proj.Transpose();
	CalculateFrustum(GetNear(), GetFar());
	MathHelpers::ComputeFrustumFromProjection(&m_kViewFrustum, transposeProj.ToFloatPtr());
}

inline void BaseCamera::getFrom( float f[3] ) const
{
	::memcpy(f, m_from.ToFloatPtr(),(unsigned)sizeof(float)* 3);
}

inline void BaseCamera::getTo( float t[3] ) const
{
	::memcpy(t, m_to.ToFloatPtr(),(unsigned)sizeof(float)* 3);
}

inline void BaseCamera::getDir( float d[3] ) const
{
	noVec3 dir = m_to - m_from;
	dir.Normalize();
	::memcpy(d, dir.ToFloatPtr(),(unsigned)sizeof(float)* 3);
}

inline void BaseCamera::getRight( float r[3] ) const
{
	noVec3 vr;
	noVec3 dir = GetDir();
	if ( m_handedness == CAMERA_HANDEDNESS_RIGHT)
		vr =dir.Cross(m_up);
	else 
		vr = m_up.Cross(dir);
	::memcpy(r, vr.ToFloatPtr(),(unsigned)sizeof(float)* 3);		
}

inline void BaseCamera::setUp( float u[3] )
{
	m_up.x = u[0];
	m_up.y = u[1];
	m_up.z = u[2];
}

inline void BaseCamera::setFar( float f )
{
	m_far = f;
}

inline void BaseCamera::setNear( float f )
{
	m_near = f;
}

inline void BaseCamera::setFrom( float f[3] ) 
{
	m_from.x = f[0];
	m_from.y = f[1];
	m_from.z = f[2];
}

inline void BaseCamera::setTo( float t[3] ) 
{
	m_to.x = t[0];
	m_to.y = t[1];
	m_to.z = t[2];
}



inline CAMERA_PROJECTION_MODE BaseCamera::getProjectionMode() const
{
	return m_mode;
}

inline void BaseCamera::setProjectionMode( CAMERA_PROJECTION_MODE m )
{
	m_mode = m;
}

inline CAMERA_HANDEDNESS_MODE BaseCamera::getHandednessMode() const
{
	return m_handedness;
}

inline void BaseCamera::setHandednessMode( CAMERA_HANDEDNESS_MODE m )
{
	m_handedness = m;
}

inline const float* BaseCamera::getProjectionMatrix() const
{
	return m_projMat;
}

inline const float* BaseCamera::getProjectionMatrix2() const
{
	return m_proj.ToFloatPtr();
}

inline const float* BaseCamera::getViewMatrix() const
{
	return m_viewMat;
}

inline void  BaseCamera::getViewMatrix(XMMATRIX& viewMat)
{
	const unsigned int size16 = sizeof(float) * 16;
	memcpy_s(&viewMat, size16, m_viewMat, size16) ;	
}

inline const float* BaseCamera::getViewMatrix2() const
{	
	return m_view.ToFloatPtr();
}

inline noVec3 BaseCamera::GetWorldRight() const
{
	return noVec3(m_viewMat[0], m_viewMat[4], m_viewMat[8]);
}

inline noVec3 BaseCamera::GetWorldUp() const 
{
	return noVec3(m_viewMat[1],	m_viewMat[5], m_viewMat[9]);
}

inline noVec3 BaseCamera::GetWorldForward() const
{
	return noVec3(m_viewMat[2], m_viewMat[6], m_viewMat[10]);
}

inline noVec3 BaseCamera::GetWorldPos() const 
{
	return noVec3(m_viewMat[12], m_viewMat[13], m_viewMat[14]);
}
