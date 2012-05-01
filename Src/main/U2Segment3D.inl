inline U2Segment3D::U2Segment3D()
{

}


inline U2Segment3D::U2Segment3D(const D3DXVECTOR3 &origin, const D3DXVECTOR3 &dir, 
						 float fExtent)
						 :m_vOrigin(origin),
						 m_vDir(dir),
						 m_fExtent(fExtent)
{

}

inline D3DXVECTOR3 U2Segment3D::GetPosEnd() const 
{
	return m_vOrigin + m_fExtent * m_vDir;
}

inline D3DXVECTOR3 U2Segment3D::GetNegEnd() const 
{
	return m_vOrigin - m_fExtent * m_vDir;
}