inline U2Box::U2Box()
{

}

inline  U2Box::U2Box(const D3DXVECTOR3& vCenter, const D3DXVECTOR3* pAxis, 
			 const float* pfExtent) :m_vCenter(vCenter)
{
	for(uint32 i=0; i < 3; ++i)
	{
		m_avAxis[i] = pAxis[i];
		m_afExtent[i] = pfExtent[i];
	}

}
inline U2Box::U2Box(const D3DXVECTOR3& vCenter, const D3DXVECTOR3& vAxis0, 
	  const D3DXVECTOR3& vAxis1, const D3DXVECTOR3& vAxis2, 
	  float fExtent0, float fExtent1, float fExtent2) 
	  :m_vCenter(vCenter)
{
	m_avAxis[0] = vAxis0;
	m_avAxis[1] = vAxis1;
	m_avAxis[2] = vAxis2;
	m_afExtent[0] = fExtent0;
	m_afExtent[1] = fExtent1;
	m_afExtent[2] = fExtent2;
}

inline void U2Box::ComputeVertices(D3DXVECTOR3 aVertex[8]) const
{
	D3DXVECTOR3 extentAxis[3] = 
	{
		m_afExtent[0] * m_avAxis[0], 
		m_afExtent[1] * m_avAxis[1], 
		m_afExtent[2] * m_avAxis[2] 
	};

	aVertex[0] = m_vCenter - extentAxis[0] - extentAxis[1] - extentAxis[2];
	aVertex[1] = m_vCenter + extentAxis[0] - extentAxis[1] - extentAxis[2];
	aVertex[2] = m_vCenter + extentAxis[0] + extentAxis[1] - extentAxis[2];
	aVertex[3] = m_vCenter - extentAxis[0] + extentAxis[1] - extentAxis[2];
	aVertex[4] = m_vCenter - extentAxis[0] - extentAxis[1] + extentAxis[2];
	aVertex[5] = m_vCenter + extentAxis[0] - extentAxis[1] + extentAxis[2];
	aVertex[6] = m_vCenter + extentAxis[0] + extentAxis[1] + extentAxis[2];
	aVertex[7] = m_vCenter - extentAxis[0] + extentAxis[1] + extentAxis[2];

}

