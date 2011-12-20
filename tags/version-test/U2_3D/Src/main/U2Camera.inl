inline 
void U2Camera::SetFrame(const D3DXVECTOR3& loc, const D3DXVECTOR3& dir, const D3DXVECTOR3& up, 
			  const D3DXVECTOR3& right)
{
	m_vEye = loc;
	SetAxes(dir, up, right);
}

inline 
void U2Camera::SetEye(const D3DXVECTOR3& loc)
{
	m_vEye = loc;

	// Update View Matrix 

}

inline U2Frustum& U2Camera::GetFrustum() 
{
	return m_frustum; 
}


inline 
void U2Camera::SetAxes(const D3DXVECTOR3& dir, const D3DXVECTOR3& up, const D3DXVECTOR3& right)
{
	m_vDir = dir;
	m_vUp = up;
	m_vRight = right;


	D3DXVECTOR3 vCross;
	D3DXVec3Cross(&vCross, &m_vUp, &m_vRight);	
	float fADet = U2Math::FAbs(D3DXVec3Dot(&m_vDir, &vCross));
	if(U2Math::FAbs(1.0f-fADet) > 0.01f)
	{
		D3DXVec3Cross(&m_vRight,  &m_vUp, &m_vDir);	
		D3DXVec3Normalize(&m_vRight, &m_vRight);
		D3DXVec3Cross(&m_vUp, &m_vRight, &m_vDir);
		D3DXVec3Normalize(&m_vUp, &m_vUp);
	}

	//D3DXVECTOR3 vCross;
	//D3DXVec3Cross(&vCross, &m_vUp, &m_vRight);	
	//float fADet = U2Math::FAbs(D3DXVec3Dot(&m_vDir, &vCross));
	//if(U2Math::FAbs(1.0f-fADet) > 0.01f)
	//{
	//	
	//	// Orthonormal Test
	//	U2Math::Orthonormalize(m_vDir, m_vUp, m_vRight);
	//}	
}

inline D3DXVECTOR3 U2Camera::GetEye() const
{
	return m_vEye;
}

inline D3DXVECTOR3 U2Camera::GetDir() const
{
	return m_vDir;
}

inline D3DXVECTOR3 U2Camera::GetUp() const
{
	return m_vUp;
}


inline D3DXVECTOR3 U2Camera::GetRight() const
{
	return m_vRight;
}


inline 
void U2Camera::SetDepthRange(float fNear, float fFar)
{
	m_fPortNear = fNear;
	m_fPortFar = fFar;

	// Update viewport
}

inline 
void U2Camera::SetViewport (float fLeft, float fRight, float fTop, float fBottom)
{
	m_viewPort.m_left = fLeft;
	m_viewPort.m_right = fRight;
	m_viewPort.m_top = fTop;
	m_viewPort.m_bottom = fBottom;

	// Update View port

}

inline 
void U2Camera::GetViewport (float& rfLeft, float& rfRight, float& rfTop,
							float& rfBottom)
{
	rfLeft = m_viewPort.m_left;
	rfRight = m_viewPort.m_right;
	rfTop = m_viewPort.m_top;
	rfBottom = m_viewPort.m_bottom;	
}


inline 
void U2Camera::GetDepthRange(float& fNear, float& fFar)
{
	fNear = m_fPortNear;
	fFar = m_fPortFar;
}

inline 
const U2Frustum& U2Camera::GetFrustum() const
{
	return m_frustum;
}