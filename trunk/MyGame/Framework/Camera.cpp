#include "DXUT.h"


#include "Camera.h"

float CameraBase::DEFAULT_FOV = 45.f;

noVec3 CameraBase::DEFAULT_UP;

noVec3 CameraBase::DEFAULT_TO;

noVec3 CameraBase::DEFAULT_FROM;


void CameraBase::Orthogonalize()
{
	// make sure up is orthoganal
	noVec3 dir;
	dir = GetDir();	
	noVec3 right;
	right = m_up.Cross(dir);
	m_up = dir.Cross(right);	
}

void CameraBase::ComputeModelView( bool forceIdentity /*= false*/ )
{
	m_camInvDirty = true;
	m_frustumPlanesDirty = true;

	if (forceIdentity)
	{
		hkgMat4Identity(m_viewMat);
		return;
	}

	Orthogonalize();

	noVec3 eye;
	eye = m_from;
		
	noVec3 f;
	// right hand 
	f = m_to - eye;
	// left hand
	//f = eye - m_to;

	f.Normalize();
	m_up.Normalize();

	noVec3 s,u;
	s = f.Cross(m_up);
	u = s.Cross(f);

	m_viewMat[0] = s[0];  m_viewMat[4] = s[1];  m_viewMat[8] = s[2]; 
	m_viewMat[1] = u[0];  m_viewMat[5] = u[1];  m_viewMat[9] = u[2];	
	m_viewMat[2] = -f[0]; m_viewMat[6] = -f[1]; m_viewMat[10] = -f[2];

	m_viewMat[3] = 0.0f; m_viewMat[7] = 0.0f; m_viewMat[11] = 0.0f;
	eye = -eye;
	m_viewMat[12] = (m_viewMat[0]*eye[0]) + (m_viewMat[4]*eye[1]) + (m_viewMat[8] *eye[2]);
	m_viewMat[13] = (m_viewMat[1]*eye[0]) + (m_viewMat[5]*eye[1]) + (m_viewMat[9] *eye[2]);
	m_viewMat[14] = (m_viewMat[2]*eye[0]) + (m_viewMat[6]*eye[1]) + (m_viewMat[10]*eye[2]);

	m_viewMat[15] = 1.0f;
}

void CameraBase::Pan( float x_amount, float y_amount, bool move_focus /*= false*/, bool proportional /*= true*/ )
{
	noVec3 dir;
	dir = m_from - m_to;
	
	float len = proportional ? dir.Length() : 1.0f;

	noVec3 move;
	move = vec3_zero;

	if (x_amount != 0.0f) {
		noVec3 right;
		right = GetRight();
		move = right * (len * x_amount);		
	}
	if (y_amount != 0.0f) {
		move = m_up  * (len * y_amount);		
	}

	m_from = m_from + move;

	if (move_focus)
		m_to = m_to + move;

	ComputeModelView();
	if (m_mode == HKG_CAMERA_COMPUTE_ORTHOGRAPHIC)
		ComputeProjection();
}

void CameraBase::Dolly( float z_amount, bool move_focus /*= false*/, bool proportional /*= true*/ )
{
	noVec3 dir;
	dir = m_from - m_to;

	if (proportional) {
		dir = dir * z_amount;
	}
	else {
		float len = dir.Length();
		dir = dir * (z_amount / len);
	}
	m_from = dir + m_from;
	if (move_focus) 
		m_to = dir + m_to;

	ComputeModelView();
	if (m_mode == HKG_CAMERA_COMPUTE_ORTHOGRAPHIC)
		ComputeProjection();
}

void CameraBase::ComputeOrtho( float left, float right, float bottom, float top, float znear, float zfar )
{
	const float rl = right - left;
	const float tb = top - bottom;
	const float fn = zfar - znear;

	const float A = 2.0f / rl;
	const float B = 2.0f / tb;
	const float C = -2.0f / fn;

	const float D =  -(right + left) / rl;
	const float E =  -(top + bottom) / tb;
	const float F =  -(zfar + znear) / fn;

	hkgMat4Zero( m_projMat );

	m_projMat[0] = A;
	m_projMat[5] = B;

	//if (m_handedness == HKG_CAMERA_HANDEDNESS_RIGHT)
	{
		m_projMat[10] = C;
	}
	//else	// left hand
	//{
	//	m_projMat[10] = -C;
	//}

	m_projMat[12] = D;
	m_projMat[13] = E;
	m_projMat[14] = F;
	m_projMat[15] = 1;

	m_camInvDirty = true;
	m_frustumPlanesDirty = true;

	m_onear = znear;
	m_ofar = zfar;
	m_oleft = left;
	m_oright = right;
	m_otop = top;
	m_obottom = bottom;

	if (m_mode == HKG_CAMERA_PERSPECTIVE) // as can be computeortho too
		m_mode = HKG_CAMERA_ORTHOGRAPHIC; 
}

void CameraBase::ComputeOrthoParams()
{

}

void CameraBase::ComputePerspective( float _fov, float aspect, float znear, float zfar )
{
	ASSERT(znear >=0 && zfar >= 0);

	const float fov = _fov * 3.141592654f / 180.0f;
	const float h = 2.0f * znear * hkg_tan( fov/2.0f );
	const float w = h * aspect;

	const float l = -w/2.0f;
	const float r = w/2.0f;
	const float b = -h/2.0f;
	const float t = h/2.0f;
	const float n = znear;
	const float f = zfar;

	const float A = ( r + l ) / ( r - l );
	const float B = ( t + b ) / ( t - b );
	const float C = -( f + n ) / ( f - n );
	const float D = -( 2*f*n ) / ( f - n );
	const float E = (2*n) / ( r - l);
	const float F = (2*n) / ( t - b);

	hkgMat4Zero( m_projMat );

	m_projMat[0] = E;
	m_projMat[5] = F;

	//if (m_handedness == HKG_CAMERA_HANDEDNESS_RIGHT)
	{
		m_projMat[8] = A;
		m_projMat[9] = B;
		m_projMat[10] = C;
		m_projMat[11] = -1;
	}
	//else	// left
	//{
	//	m_projMat[8] = -A;
	//	m_projMat[9] = -B;
	//	m_projMat[10] = -C;
	//	m_projMat[11] = 1;
	//}
	m_projMat[14] = D;

	m_camInvDirty = true;
	m_frustumPlanesDirty = true;

	m_near = znear;
	m_far = zfar;
	m_aspect = aspect;
	m_fov = _fov;

	m_mode = HKG_CAMERA_PERSPECTIVE;
}

void CameraBase::ComputeFrustumPlanes() const
{
	if (m_camInvDirty)
		ComputeCameraInverse();

	float transformedFrustumPlanePnt[8][4];


}

void CameraBase::ComputeCameraInverse() const
{

}