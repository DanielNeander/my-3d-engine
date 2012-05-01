//-------------------------------------------------------------------------------------------------
inline int U2SphereBV::GetBVType() const
{
	return U2BoundingVolume::BV_SPHERE;
}

//-------------------------------------------------------------------------------------------------
inline void U2SphereBV::SetCenter(const D3DXVECTOR3& center)
{
	m_sphere.m_center = center;
}

//-------------------------------------------------------------------------------------------------
inline void U2SphereBV::SetRadius(float fRadius)
{
	m_sphere.m_fRadius = fRadius;
}

//-------------------------------------------------------------------------------------------------
inline D3DXVECTOR3 U2SphereBV::GetCenter() const
{
	return m_sphere.m_center;
}

//-------------------------------------------------------------------------------------------------
inline float U2SphereBV::GetRadius() const
{
	return m_sphere.m_fRadius;
}

//-------------------------------------------------------------------------------------------------
inline U2Sphere& U2SphereBV::Sphere()
{
	return m_sphere;
}

//-------------------------------------------------------------------------------------------------
inline const U2Sphere& U2SphereBV::Sphere() const
{
	return m_sphere;
}
