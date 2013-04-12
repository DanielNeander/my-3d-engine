#include "stdafx.h"
#include "EngineCore/Math/GameMath.h"
#include "EngineCore/Math/IntersectionTests.h"
#include "EngineCore/Math/Frustum.h"
#include "BaseCamera.h"
#include "GameApp.h"


float BaseCamera::DEFAULT_FOV = 45.0f;

noVec3 BaseCamera::DEFAULT_UP = noVec3(0, 0, 0);

noVec3 BaseCamera::DEFAULT_TO = noVec3(0, 0, 1);

noVec3 BaseCamera::DEFAULT_FROM = noVec3(4,10,2);

const float g_sourceFrustumPlanePntRight[8][4] = 
{
	/*0*/{ -1.0f, -1.0f,  -1.0f, 1.0f}, // xyzw
	/*1*/{  1.0f, -1.0f,  -1.0f, 1.0f}, // Xyzw
	/*2*/{ -1.0f,  1.0f,  -1.0f, 1.0f}, // xYzw
	/*3*/{  1.0f,  1.0f,  -1.0f, 1.0f}, // XYzw
	/*4*/{ -1.0f, -1.0f,  1.0f, 1.0f}, // xyZw
	/*5*/{  1.0f, -1.0f,  1.0f, 1.0f}, // XyZw
	/*6*/{ -1.0f,  1.0f,  1.0f, 1.0f}, // xYZw
	/*7*/{  1.0f,  1.0f,  1.0f, 1.0f} // XYZw
};

const float g_sourceFrustumPlanePntLeft[8][4] = 
{
	/*0*/{ -1.0f, -1.0f,  1.0f, 1.0f}, // xyzw
	/*1*/{  1.0f, -1.0f,  1.0f, 1.0f}, // Xyzw
	/*2*/{ -1.0f,  1.0f,  1.0f, 1.0f}, // xYzw
	/*3*/{  1.0f,  1.0f,  1.0f, 1.0f}, // XYzw
	/*4*/{ -1.0f, -1.0f,  -1.0f, 1.0f}, // xyZw
	/*5*/{  1.0f, -1.0f,  -1.0f, 1.0f}, // XyZw
	/*6*/{ -1.0f,  1.0f,  -1.0f, 1.0f}, // xYZw
	/*7*/{  1.0f,  1.0f,  -1.0f, 1.0f} // XYZw
};

inline void hkgVec3Rotate( float out[3], const float in[3], const float r[16] )
{
	out[0] = in[0]*r[0] + in[1]*r[4] + in[2]*r[8];
	out[1] = in[0]*r[1] + in[1]*r[5] + in[2]*r[9];
	out[2] = in[0]*r[2] + in[1]*r[6] + in[2]*r[10];
}

void BaseCamera::UpdateInternal(float fDelta)
{
	/*if (!IsActivated())
	{
		return;
	}*/
	UpdateCamera(fDelta);
	GameObject::UpdateInternal(fDelta);
}


void BaseCamera::orthogonalize()
{	
	noVec3 dir = GetDir();
	noVec3 right;
	right = m_up.Cross(dir);
	m_up = dir.Cross(right);
}

void BaseCamera::computeModelView( const noMat3& rot, bool forceIdentity /*= false*/ )
{
	m_camInvDirty = true;
	m_frustumPlanesDirty = true;

	if (forceIdentity)
	{
		memcpy_s(m_viewMat, SIZE16, mat4_identity.ToFloatPtr(), SIZE16);
		return;
	}
	noVec3 eye = m_from;
	noVec3 dir, right, up;
	dir = rot[0];
	up = rot[1];
	right = rot[2];

	m_viewMat[0] = right[0]; m_viewMat[4] = right[1];	m_viewMat[8] = right[2];
	m_viewMat[1] = up[0];	 m_viewMat[5] = up[1];		m_viewMat[9] = up[2];   
	m_viewMat[2] = -dir[0];	 m_viewMat[6] = -dir[1];	m_viewMat[10] = -dir[2];

	m_viewMat[3] = 0.0f; m_viewMat[7] = 0.0f; m_viewMat[11] = 0.0f;

	eye *= -1;

	m_viewMat[12] = (m_viewMat[0]*eye[0]) + (m_viewMat[4]*eye[1]) + (m_viewMat[8] *eye[2]);
	m_viewMat[13] = (m_viewMat[1]*eye[0]) + (m_viewMat[5]*eye[1]) + (m_viewMat[9] *eye[2]);
	m_viewMat[14] = (m_viewMat[2]*eye[0]) + (m_viewMat[6]*eye[1]) + (m_viewMat[10]*eye[2]);

	m_viewMat[15] = 1.0f;

	memcpy_s(&m_view, SIZE16, m_viewMat, SIZE16);
}

void BaseCamera::computeModelView( bool forceIdentity /*= false*/ )
{
	m_camInvDirty = true;
	m_frustumPlanesDirty = true;

	if (forceIdentity)
	{
		memcpy_s(m_viewMat, SIZE16, mat4_identity.ToFloatPtr(), SIZE16);
		return;
	}

	orthogonalize();	
	noVec3 eye = m_from;
	noVec3 center = m_to;

	noVec3 dir;
	if (m_handedness == CAMERA_HANDEDNESS_RIGHT)
		dir = center - eye;
	else 
		dir = eye - center;

	dir.Normalize();
	m_up.Normalize();

	noVec3 right;
	noVec3 up;

	right = dir.Cross(m_up);
	up    = right.Cross(dir);

	m_viewMat[0] = right[0]; m_viewMat[4] = right[1];	m_viewMat[8] = right[2];
	m_viewMat[1] = up[0];	 m_viewMat[5] = up[1];		m_viewMat[9] = up[2];   
	m_viewMat[2] = -dir[0];	 m_viewMat[6] = -dir[1];	m_viewMat[10] = -dir[2];
	
	m_viewMat[3] = 0.0f; m_viewMat[7] = 0.0f; m_viewMat[11] = 0.0f;

	eye *= -1;

	m_viewMat[12] = (m_viewMat[0]*eye[0]) + (m_viewMat[4]*eye[1]) + (m_viewMat[8] *eye[2]);
	m_viewMat[13] = (m_viewMat[1]*eye[0]) + (m_viewMat[5]*eye[1]) + (m_viewMat[9] *eye[2]);
	m_viewMat[14] = (m_viewMat[2]*eye[0]) + (m_viewMat[6]*eye[1]) + (m_viewMat[10]*eye[2]);

	m_viewMat[15] = 1.0f;

	memcpy_s(&m_view, SIZE16, m_viewMat, SIZE16);
	//m_view.TransposeSelf();
}

void BaseCamera::project( float x, float y, float z, int winWidth, int winHeight, const noMat4& world, noVec3& result ) const
{
	noVec4 ws;
	ws[0] = x;
	ws[1] = y;
	ws[2] = z;
	ws[3] = 1.0f;

	noMat4 w2c;
	noMat4 proj(&m_projMat[0]);
	noMat4 view(&m_viewMat[0]);

	proj.TransposeSelf();
	view.TransposeSelf();
	
	w2c =  proj* view * world;

	noVec4 cs;
	cs = w2c * ws;

	result[0] = cs[0]/cs[3];
	result[1] = cs[1]/cs[3];
	result[2] = cs[2]/cs[3];

	// convert:

	noVec3 one(1.0f, 1.0f, 1.0f);
	result += one ;
	result *= 0.5f;

	// ranges now in 0-->1 for window,
	// want to convert x,y into window coords

	result[0] *= winWidth;
	result[1] *= winHeight;
}

void BaseCamera::project( float x, float y, float z, int winWidth, int winHeight, noVec3& result ) const
{
	noVec4 ws;
	ws[0] = x;
	ws[1] = y;
	ws[2] = z;
	ws[3] = 1.0f;
	
	noMat4 w2c;
	noMat4 proj(&m_projMat[0]);
	noMat4 view(&m_viewMat[0]);

	proj.TransposeSelf();
	view.TransposeSelf();
	
	w2c =  proj* view;

	noVec4 cs;
	cs = w2c * ws;

	result[0] = cs[0]/cs[3];
	result[1] = cs[1]/cs[3];
	result[2] = cs[2]/cs[3];

	// convert:

	noVec3 one(1.0f, 1.0f, 1.0f);
	result += one ;
	result *= 0.5f;

	// ranges now in 0-->1 for window,
	// want to convert x,y into window coords

	result[0] *= winWidth;
	result[1] *= winHeight;
}


#define HKG_X 0
#define HKG_Y 1
#define HKG_Z 2
#define HKG_W 3

inline void hkgVec4Transform( float out[4], const float in[4], const float t[16] )
{
	out[0] = in[HKG_X]*t[0] + in[HKG_Y]*t[4] + in[HKG_Z]*t[8]  + in[HKG_W]*t[12];
	out[1] = in[HKG_X]*t[1] + in[HKG_Y]*t[5] + in[HKG_Z]*t[9]  + in[HKG_W]*t[13];
	out[2] = in[HKG_X]*t[2] + in[HKG_Y]*t[6] + in[HKG_Z]*t[10] + in[HKG_W]*t[14];
	out[3] = in[HKG_X]*t[3] + in[HKG_Y]*t[7] + in[HKG_Z]*t[11] + in[HKG_W]*t[15];
}

void BaseCamera::unProject( int x, int y, float depth, int winWidth, int winHeight, noVec3& result ) const
{
	float cs[4];
	cs[0] = ((2 * x)/(float)winWidth) - 1.0f;
	cs[1] = 1.0f - ((2 * y)/(float)winHeight);
	// Use midPoint rather than far point to avoid issues with infinite projection
	cs[2] = (2*depth) - 1.0f;
	cs[3] = 1.0f;

	if (m_camInvDirty)
	{
		ComputeCameraInverse();		
	}
		
	//noVec4 ws;	
	//m_invView.UnprojectVector(cs, ws);
	float ws[4];
	hkgVec4Transform( ws, cs, m_camInv );

	result[0] = ws[0]/ws[3];
	result[1] = ws[1]/ws[3];
	result[2] = ws[2]/ws[3];
}

void BaseCamera::unProject( int x, int y, float depth, int winWidth, int winHeight, const noMat4& invWorld, noVec3& result ) const
{
	noVec4 cs;
	cs[0] = ((2 * x)/(float)winWidth) - 1.0f;
	cs[1] = ((2 * y)/(float)winHeight) - 1.0f;
	cs[2] = (2*depth) - 1.0f;
	cs[3] = 1.0f;

	if (m_camInvDirty)
	{
		ComputeCameraInverse();		
	}

	noMat4 invWorldView = m_invView * invWorld;	
	noVec4 ws;	
	invWorldView.UnprojectVector(cs, ws);
	
	result[0] = ws[0]/ws[3];
	result[1] = ws[1]/ws[3];
	result[2] = ws[2]/ws[3];
}

void BaseCamera::unProjectExact( int x, int y, float depth, int winWidth, int winHeight, float result[3] ) const
{

}

void BaseCamera::pan( float x_amount, float y_amount, bool move_focus /*= false*/, bool proportional /*= true*/ )
{
	noVec3 dir = m_from - m_to;

	float len = proportional ? dir.Length() : 1.0f;

	noVec3 move;
	move.Zero();
	if (x_amount != 0.0f) 
	{
		
		noVec3 right =GetRight();
		move = right * (len * x_amount);	
	}
	if (y_amount != 0.0f)
	{
		move.x += m_up.x * (len * y_amount);
		move.y += m_up.y * (len * y_amount);
		move.z += m_up.z * (len * y_amount);
	}
	m_from = m_from + move;

	if (move_focus)
		m_to = m_to + move;

	computeModelView();
	if (m_mode == CAMERA_COMPUTE_ORTHOGRAPHIC)
		ComputeProjection();
}

void BaseCamera::dolly( float z_amount, bool move_focus /*= false*/, bool proportional /*= true*/ )
{
	noVec3 dir;
	dir = m_from - m_to;

	if (proportional)
	{
		dir = dir * z_amount;
	}
	else 
	{
		float len = dir.Length(); 
		dir = dir * z_amount / len;
	}
	m_from = dir + m_from;

	if (move_focus)
		m_to = dir + m_to;

	computeModelView();
	if (m_mode == CAMERA_COMPUTE_ORTHOGRAPHIC)
		ComputeProjection();
}

void BaseCamera::rotateAboutTo( float angle, const float axis[3], bool upLock /*= false*/ )
{
	noQuat q;
	//q.FromAngleAxis(angle, noVec3(axis[0], axis[1], axis[2]));
	noRotation rot(vec3_zero, noVec3(axis[0], axis[1], axis[2]), angle);
	q = rot.ToQuat();

	
	
	noVec3 dir;
	dir = m_from - m_to;

	dir = q * dir;
	//dir = q.RotateVec3(dir);
	
	if (!upLock)
		//m_up = q.RotateVec3(m_up);
		m_up = q * m_up;

	m_from = m_to + dir;

	computeModelView();
	if (m_mode == CAMERA_COMPUTE_ORTHOGRAPHIC)
		ComputeProjection();
}

void BaseCamera::rotateAboutFrom( float angle, const float axis[3], bool upLock /*= false*/ )
{
	noQuat q;
	//q.FromAngleAxis((angle), noVec3(axis[0], axis[1], axis[2]));
	noRotation rot(vec3_zero, noVec3(axis[0], axis[1], axis[2]), angle);
	q = rot.ToQuat();

	noVec3 dir;
	dir =  m_to - m_from;

	dir = q * dir;

	if (!upLock)
		m_up = q * m_up;

	m_to = m_from + dir;

	computeModelView();
	if (m_mode == CAMERA_COMPUTE_ORTHOGRAPHIC)
		ComputeProjection();
}

static void trackballPointToVector(int x, int y, 
	unsigned int w, unsigned int h, 
	noVec3& v, bool leftHanded)
{

	// project x, y onto a hemi-sphere centered within width, height. 
	v[0] = (2.0f * x - w) / w;
	v[1] = (2.0f * y - h) / h;

	const float d = noMath::Sqrt(v[0] * v[0] + v[1] * v[1]);

	v[2] = noMath::Cos((noMath::PI / 2.0f) * ((d < 1.0f) ? d : 1.0f));

	if (leftHanded)
		v[2] *= -1;

	v.Normalize();
}

static void trackballAxisAngle(int x, int y, 
	unsigned int width, unsigned int height,
	noVec3& lastPos, noVec3& axis, float &angle, bool leftHanded)
{
	noVec3 curPos;
	noVec3 d;

	trackballPointToVector( x, y, width, height, curPos, leftHanded);

	d =  curPos - lastPos ;

	angle = ( noMath::PI * 0.5f ) * d.Length(); 
	//angle = d.Length(); 

	 /*if (leftHanded)
		axis = curPos.Cross( lastPos );
	 else*/
		axis =  lastPos.Cross( curPos );

	lastPos =  curPos;
}


void BaseCamera::startTrackball( unsigned int winWidth, unsigned int winHeight, int mouseX, int mouseY )
{
	trackballPointToVector(mouseX, mouseY, winWidth, winHeight, m_trackballVec, m_handedness == CAMERA_HANDEDNESS_LEFT);

}

void BaseCamera::moveTrackball( unsigned int winWidth, unsigned int winHeight, int mouseX, int mouseY )
{
	noVec3 axis;
	float angle;
	trackballAxisAngle(mouseX, mouseY, winWidth, winHeight, m_trackballVec, axis, angle, m_handedness == CAMERA_HANDEDNESS_LEFT);

	// as we just use the rotational part of the view mat, and we know 
	// it is orthoganal, we can just use the transpose as the inverse.
	noMat4 temp(m_viewMat);
	temp.TransposeSelf(); // inverse 
	
	
	hkgVec3Rotate(axis.ToFloatPtr(), axis.ToFloatPtr(), temp.ToFloatPtr());
	//axis = temp * axis;
	axis.Normalize();

	rotateAboutTo(RAD2DEG(angle), axis.ToFloatPtr(), false);

}


void BaseCamera::ComputeOrtho( float left, float right, float bottom, float top, float znear, float zfar )
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

	ZeroMemory(m_projMat, SIZE16);

	m_projMat[0] = A;
	m_projMat[5] = B;

	if (m_handedness == CAMERA_HANDEDNESS_RIGHT)
	{
		m_projMat[10] = C;
	}
	else
	{
		m_projMat[10] = -C;
	}

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

	if (m_mode == CAMERA_PERSPECTIVE) // as can be computeortho too
		m_mode = CAMERA_ORTHOGRAPHIC; 
}

void BaseCamera::ComputeOrthoParams()
{
	noVec3 pov;

	orthogonalize();

	pov = m_from - m_to;
	const float fov = m_fov * noMath::PI / 180.f;
	const float dist = pov.Length();

	const float h = 2.0f * dist * noMath::Tan( fov/2.0f );
	const float w = h * m_aspect;

	m_oleft		= -w/2.0f;
	m_oright	= w/2.0f;
	m_obottom	= -h/2.0f;
	m_otop		= h/2.0f;
	m_onear		= m_near;
	m_ofar		= m_far;
}

void BaseCamera::ComputePerspective( float _fov, float aspect, float znear, float zfar )
{
	const float fov = _fov * 3.141592654f / 180.0f;
	const float h = 2.0f * znear * noMath::Tan( fov/2.0f );
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
	//const float D = -( 2*f*n ) / ( f - n );
	const float D = -( 2*f*n ) / ( f - n );
	const float E = (2*n) / ( r - l);
	const float F = (2*n) / ( t - b);

	memset( m_projMat, 0, sizeof(float) * 16 );

	m_projMat[0] = E;
	m_projMat[5] = F;

	if (m_handedness == CAMERA_HANDEDNESS_RIGHT)
	{
		m_projMat[8] = A;
		m_projMat[9] = B;
		m_projMat[10] = C;
		m_projMat[11] = -1;
	}
	else
	{
		m_projMat[8] = -A;
		m_projMat[9] = -B;
		m_projMat[10] = -C;
		m_projMat[11] = 1;
	}

	m_projMat[14] = D;

	m_camInvDirty = true;
	m_frustumPlanesDirty = true;

	m_near = znear;
	m_far = zfar;
	m_aspect = aspect;
	m_fov = _fov;

	m_mode = CAMERA_PERSPECTIVE;
}

void BaseCamera::ComputeFrustumPlanes() const
{
	if (m_camInvDirty)
		ComputeCameraInverse();

	float transformedFrustumPlanePnt[8][4];

	for( int i = 0; i < 8; i++ )
	{
		if (m_handedness == CAMERA_HANDEDNESS_RIGHT)
		{
			noVec4 temp(g_sourceFrustumPlanePntRight[i][0],g_sourceFrustumPlanePntRight[i][1], 
				g_sourceFrustumPlanePntRight[i][2], g_sourceFrustumPlanePntRight[i][3]);						
			noVec4 transformedPnt = m_invView * temp;
			memcpy(transformedFrustumPlanePnt[i], &transformedPnt, sizeof(float) * 4);
		}
		else 
		{
			noVec4 temp(g_sourceFrustumPlanePntLeft[i][0],g_sourceFrustumPlanePntLeft[i][1], 
				g_sourceFrustumPlanePntLeft[i][2], g_sourceFrustumPlanePntLeft[i][3]);						
			noVec4 transformedPnt = m_invView * temp;
			memcpy(transformedFrustumPlanePnt[i], &transformedPnt, sizeof(float) * 4);
		}


		*transformedFrustumPlanePnt[i] /= transformedFrustumPlanePnt[i][3];
	}

	PlaneFrom3Points( m_frustumPlanes[0], transformedFrustumPlanePnt[2], 
		transformedFrustumPlanePnt[3], transformedFrustumPlanePnt[6] ); // Top

	PlaneFrom3Points( m_frustumPlanes[1],transformedFrustumPlanePnt[1], 
		transformedFrustumPlanePnt[0], transformedFrustumPlanePnt[4] ); // Bottom

	PlaneFrom3Points( m_frustumPlanes[2], transformedFrustumPlanePnt[2], 
		transformedFrustumPlanePnt[6], transformedFrustumPlanePnt[4] ); // Left

	PlaneFrom3Points( m_frustumPlanes[3], transformedFrustumPlanePnt[7], 
		transformedFrustumPlanePnt[3], transformedFrustumPlanePnt[5] ); // Right

	PlaneFrom3Points( m_frustumPlanes[4], transformedFrustumPlanePnt[0], 
		transformedFrustumPlanePnt[1], transformedFrustumPlanePnt[2] ); // Near

	PlaneFrom3Points( m_frustumPlanes[5], transformedFrustumPlanePnt[6], 
		transformedFrustumPlanePnt[7], transformedFrustumPlanePnt[5] ); // Far




	m_frustumPlanesDirty = false;
}

void BaseCamera::ComputeCameraInverse() const
{
	float viewInv[16];
	float projInv[16];

	computeInverseProjectionMatrix( projInv );
	computeInverseViewMatrix( viewInv );
	
	noMat4 invView(viewInv), invProj(projInv);
	m_invView = invProj * invView;	
	memcpy(m_camInv, m_invView.ToFloatPtr(), SIZE16);
	m_invView.TransposeSelf();
	m_camInvDirty = false;	
}

void BaseCamera::computeInverseProjectionMatrix( float inv[16] ) const
{
	memset(inv, 0, sizeof(float) * 16);

	switch (m_mode)
	{
	case CAMERA_PERSPECTIVE: 
		{
			const float fov = m_fov * noMath::PI / 180.0f;
			const float h = 2.0f * m_near * noMath::Tan( fov/2.0f );
			const float w = h * m_aspect;

			const float l = -w/2.0f;
			const float r = w/2.0f;
			const float b = -h/2.0f;
			const float t = h/2.0f;
			const float n = m_near;
			const float f = m_far;

			const float A = (r - l) / (2*n);
			const float B = (t - b) / (2*n);
			const float C = -(f - n) / (2*f*n);

			const float D = (r + l) / (2*n);
			const float E = (t + b) / (2*n);
			const float F = (f + n) / (2*f*n);

			inv[0] = A;
			inv[5] = B;
			inv[11] = C;

			inv[12] = D;
			inv[13] = E;

			if (m_handedness == CAMERA_HANDEDNESS_RIGHT)
				inv[14] = -1;
			else
				inv[14] = 1;

			inv[15] = F;
		}
		break;


	case CAMERA_COMPUTE_ORTHOGRAPHIC: 
		// no break;
	case CAMERA_ORTHOGRAPHIC: 

		{
			const float A = (m_oright - m_oleft) / 2.0f;
			const float B = (m_otop - m_obottom)/ 2.0f;
			const float C = (m_ofar - m_onear) / -2.0f;

			const float D = (m_oright + m_oleft) / 2.0f;
			const float E = (m_otop + m_obottom)/ 2.0f;
			const float F = (m_ofar + m_onear) / -2.0f;

			inv[0] = A;
			inv[5] = B; 

			if (m_handedness == CAMERA_HANDEDNESS_RIGHT)
			{
				inv[10] = C;
				inv[14] = F;
			}			
			else
			{
				inv[10] = -C;	
				inv[14] = -F;
			}

			inv[12] = D;
			inv[13] = E;
			inv[15] = 1;

		}
		break;
	}
}

void BaseCamera::computeInverseViewMatrix( float inv[16] ) const
{
	// Model view is easy to invert as it is orthonormal in our system
	// Transpose the initial rotation and translate by the inverse of the original translation:

	inv[0] = m_viewMat[0];
	inv[1] = m_viewMat[4];
	inv[2] = m_viewMat[8];
	inv[3] = 0;

	inv[4] = m_viewMat[1];
	inv[5] = m_viewMat[5];
	inv[6] = m_viewMat[9];
	inv[7] = 0;

	inv[8] = m_viewMat[2];
	inv[9] = m_viewMat[6];
	inv[10] = m_viewMat[10];
	inv[11] = 0;


	// For [12]:
	// --> viewInv[12]*s[0] + viewInv[13]*s[1] + viewInv[14]*s[2] + 1*m_viewMat[12] == 0
	// where m_viewMat[12], == s[0]e[0] + s[1]e[1] + s[2]e[2]
	// so viewInv[12] = -e[0], viewInv[13] == -e[1], etc
	// and e == -m_from, 

	inv[12] = m_from[0];
	inv[13] = m_from[1];
	inv[14] = m_from[2];

	inv[15] = 1.0f;
}

// computes a frustum with given far and near planes
Frustum BaseCamera::CalculateFrustum( float fNear, float fFar )
{
	noVec3 vZ = Normalize(m_to - m_from);
	noVec3 vX = Normalize(Cross(m_up, vZ));
	noVec3 vY = Normalize(Cross(vZ, vX));

	float fAspect = GetApp()->GetAspectRatio();

	float fNearPlaneHalfHeight = tanf(m_fov * 0.5f) * fNear;
	float fNearPlaneHalfWidth = fNearPlaneHalfHeight * fAspect;

	float fFarPlaneHalfHeight = tanf(m_fov * 0.5f) * fFar;
	float fFarPlaneHalfWidth = fFarPlaneHalfHeight * fAspect;

	noVec3 vNearPlaneCenter = m_from + vZ * fNear;
	noVec3 vFarPlaneCenter = m_from + vZ * fFar;

	Frustum frustum;
	frustum.m_pPoints[0] = noVec3(vNearPlaneCenter - vX*fNearPlaneHalfWidth - vY*fNearPlaneHalfHeight);
	frustum.m_pPoints[1] = noVec3(vNearPlaneCenter - vX*fNearPlaneHalfWidth + vY*fNearPlaneHalfHeight);
	frustum.m_pPoints[2] = noVec3(vNearPlaneCenter + vX*fNearPlaneHalfWidth + vY*fNearPlaneHalfHeight);
	frustum.m_pPoints[3] = noVec3(vNearPlaneCenter + vX*fNearPlaneHalfWidth - vY*fNearPlaneHalfHeight);

	frustum.m_pPoints[4] = noVec3(vFarPlaneCenter - vX*fFarPlaneHalfWidth - vY*fFarPlaneHalfHeight);
	frustum.m_pPoints[5] = noVec3(vFarPlaneCenter - vX*fFarPlaneHalfWidth + vY*fFarPlaneHalfHeight);
	frustum.m_pPoints[6] = noVec3(vFarPlaneCenter + vX*fFarPlaneHalfWidth + vY*fFarPlaneHalfHeight);
	frustum.m_pPoints[7] = noVec3(vFarPlaneCenter + vX*fFarPlaneHalfWidth - vY*fFarPlaneHalfHeight);

	// update frustum AABB
	frustum.CalculateAABB();

	return frustum;
}



bool BaseCamera::WindowPointToRay(long lWx, long lWy, noVec3& kOrigin,
	noVec3& kDir, uint32 width, uint32 height) const
{
	// Window points (wx,wy) satisfy 0 <= wx < wxmax and 0 <= wy < wymax,
	// where wxmax is the screen width and wymax is the screen height, both
	// measured in pixels.  Buffer points (bx,by) are in [0,1]^2 with
	//   bx = wx/wxmax
	//   by = 1 - wy/wymax
	// Notice that the y values are reflected to change handedness of the
	// coordinates.  Buffer point (bx,by) is the relative location of (wx,wy)
	// on the full screen.
	float fBx, fBy;
	bool bConverted = GetApp()->MapWindowPointToBufferPoint(
		(unsigned int)lWx, (unsigned int)lWy, fBx, fBy, width, height);
	if (!bConverted || fBx > m_kPort.m_right || fBx < m_kPort.m_left ||
		fBy > m_kPort.m_top || fBy < m_kPort.m_bottom)
	{
		return false;
	}

	// Viewport coordinates (px,py) are in [0,1]^2 with
	//   px = (bx - port.L)/(port.R - port.L)
	//   py = (by - port.B)/(port.T - port.B)
	// Viewport point (px,py) is the relative location of (wx,wy) in the
	// viewport, a subrectangle of the screen.
	float fPx = (fBx - m_kPort.m_left) / (m_kPort.m_right - m_kPort.m_left);
	float fPy = (fBy - m_kPort.m_bottom) / (m_kPort.m_top - m_kPort.m_bottom);

	// View plane coordinates (vx,vy) satisfy frustum.L <= vx <= frustum.R
	// and frustum.B <= vy <= frustum.T.  The mapping is
	//   vx = frustum.L + px * (frustum.R - frustum.L)
	//   vy = frustum.B + py * (frustum.T - frustum.b)
	float fVx = m_kViewFrustum.LeftSlope + fPx * 
		(m_kViewFrustum.RightSlope - m_kViewFrustum.LeftSlope);
	float fVy = m_kViewFrustum.BottomSlope + fPy * 
		(m_kViewFrustum.TopSlope - m_kViewFrustum.BottomSlope);

	// Convert view plane coordinates to a ray in world coordinates.  The
	// output kOrigin is the camera world location and kDir is the world
	// direction from the camera world location to the point on the view
	// plane.
	ViewPointToRay(fVx, fVy, kOrigin, kDir);
	return true;
}


void BaseCamera::ViewPointToRay(float fVx, float fVy, noVec3& kOrigin,
	noVec3& kDir) const
{
	if (m_mode == CAMERA_ORTHOGRAPHIC || m_mode == CAMERA_COMPUTE_ORTHOGRAPHIC)
	{
		kDir = 	GetWorldForward();
		kDir.Normalize();
		kOrigin = GetFrom() + GetWorldRight() * fVx + GetWorldUp() * fVy;
	}
	else
	{
		kDir = GetWorldForward() + GetWorldRight() * fVx + GetWorldUp() * fVy;
		kDir.Normalize();
		kOrigin = GetFrom();
	}		
}
