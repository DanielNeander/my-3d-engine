inline U2Plane&		U2Plane::operator= (const U2Plane& rhs)
{
	m_vNormal = rhs.m_vNormal;
	m_fDistance = rhs.m_fDistance;
	return *this;

}


inline bool			U2Plane::operator==(const U2Plane& src)
{
	return (m_vNormal == src.m_vNormal) && (m_fDistance == src.m_fDistance);
}


inline void			U2Plane::Set(const D3DXVECTOR3& pt0, const D3DXVECTOR3& pt1, const D3DXVECTOR3& pt2)
{
	
}


inline void			U2Plane::Set(const D3DXVECTOR3& normal, float distance)
{

}


inline void			U2Plane::Set(const D3DXVECTOR3& pt, const D3DXVECTOR3& normal)
{
	
}


inline void			U2Plane::Normalize()
{
	float mag = D3DXVec3Length(&m_vNormal);
	float inv_mag = 1.0f/mag;

	m_vNormal.x *= inv_mag;
	m_vNormal.y *= inv_mag;
	m_vNormal.z *= inv_mag;
	m_fDistance *= inv_mag;
}


inline void			U2Plane::Transform(const D3DXMATRIX& mat)
{
	D3DXVec3TransformNormal(&m_vNormal, &m_vNormal, &mat);
	D3DXVECTOR3 tempNormal = m_vNormal;
	D3DXVec3Normalize(&m_vNormal, &tempNormal);

	D3DXVECTOR3 point = m_vNormal * m_fDistance;
	D3DXVec3TransformCoord(&point, &point, &mat);
	m_fDistance = -D3DXVec3Dot(&m_vNormal, &point);
}

// Simple Intersection Tests For Games
// http://www.gamasutra.com/view/feature/3383/simple_intersection_tests_for_games.php
inline float U2Plane::GetDistance(const D3DXVECTOR3& pt) const
{
	float fDist = D3DXVec3Dot(&m_vNormal, &pt);
	return 	fDist + m_fDistance;
}



// Simple Intersection Tests For Games
// http://www.gamasutra.com/view/feature/3383/simple_intersection_tests_for_games.php
inline int U2Plane::WhichSide (const D3DXVECTOR3& rkQ) const
{
	float fDistance = GetDistance(rkQ);

	if(fDistance < 0.f)
	{
		return -1;
	}

	if(fDistance > 0.f)
	{
		return +1;
	}
	return 0;
}
