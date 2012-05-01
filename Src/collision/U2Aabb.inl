//-------------------------------------------------------------------------------------------------
inline U2Aabb::U2Aabb()
:m_vMin(VECTOR3_ZERO),
m_vMax(VECTOR3_ZERO)
{

}

//-------------------------------------------------------------------------------------------------
inline U2Aabb::U2Aabb(const D3DXVECTOR3& center, const D3DXVECTOR3& extents)
{	
	m_vMin = center - extents;
	m_vMax = center + extents;
}

inline U2Aabb::U2Aabb(const D3DXMATRIX& m)
{
	this->Set(m);
}

inline U2Aabb::U2Aabb(const void* pPoints, int iNumPoints, int iStride)
{
	this->Set(pPoints, iNumPoints, iStride);
}


//-------------------------------------------------------------------------------------------------
inline D3DXVECTOR3 U2Aabb::Center() const
{
	return D3DXVECTOR3((m_vMin + m_vMax) * 0.5f);
}

//-------------------------------------------------------------------------------------------------
inline D3DXVECTOR3 U2Aabb::Extents() const
{
	return D3DXVECTOR3((m_vMax - m_vMin) * 0.5f);
}

//-------------------------------------------------------------------------------------------------
inline D3DXVECTOR3 U2Aabb::Size() const
{
	return D3DXVECTOR3(m_vMax - m_vMin);
}

//-------------------------------------------------------------------------------------------------
inline float U2Aabb::DiagonalSize() const
{
	D3DXVECTOR3 temp(m_vMax - m_vMin);
	return D3DXVec3Length(&temp);
}

//-------------------------------------------------------------------------------------------------
inline void U2Aabb::Set(const D3DXMATRIX& m)
{
	using namespace U2Math;
	float xExtent = Max<float>(Max<float>(FAbs(m._11), FAbs(m._21)), FAbs(m._31));
	float yExtent = Max<float>(Max<float>(FAbs(m._12), FAbs(m._22)), FAbs(m._32));
	float zExtent = Max<float>(Max<float>(FAbs(m._13), FAbs(m._23)), FAbs(m._33));
	D3DXVECTOR3 extent(xExtent, yExtent, zExtent);

	D3DXVECTOR3 center(*(D3DXVECTOR3*)m.m[3]);
	m_vMin = center - extent;
	m_vMax = center + extent;
}

//-------------------------------------------------------------------------------------------------
inline void U2Aabb::Set(const D3DXVECTOR3& center, const D3DXVECTOR3& extents)
{
	m_vMin = center -extents;
	m_vMax = center + extents;
}

//-------------------------------------------------------------------------------------------------
inline void U2Aabb::BeginExtend()
{
	m_vMin = D3DXVECTOR3(MAX_BOUND, MAX_BOUND, MAX_BOUND);
	m_vMax = D3DXVECTOR3(MIN_BOUND, MIN_BOUND, MIN_BOUND);
}

//-------------------------------------------------------------------------------------------------
inline void U2Aabb::Extend(const D3DXVECTOR3& v)
{
	if(v.x < m_vMin.x) m_vMin.x = v.x;
	if(v.x > m_vMax.x) m_vMax.x = v.x;
	if(v.y < m_vMin.y) m_vMin.y = v.y;
	if(v.y > m_vMax.y) m_vMax.y = v.y;
	if(v.z < m_vMin.z) m_vMin.z = v.z;
	if(v.z > m_vMax.z) m_vMax.z = v.z;

}

//-------------------------------------------------------------------------------------------------
inline void U2Aabb::EndExtend()
{
	if(U2Math::Vec3Equal(m_vMin, D3DXVECTOR3(MAX_BOUND, MAX_BOUND, MAX_BOUND), ZERO_TOLERANCE) &&
		U2Math::Vec3Equal(m_vMax, D3DXVECTOR3(MIN_BOUND, MIN_BOUND, MIN_BOUND), ZERO_TOLERANCE))
	{
		m_vMin = VECTOR3_ZERO;
		m_vMax = VECTOR3_ZERO;
	}
}

//-------------------------------------------------------------------------------------------------
inline void U2Aabb::Extend(float x, float y, float z)
{
	if(x < m_vMin.x) m_vMin.x = x;
	if(x > m_vMax.x) m_vMax.x = x;
	if(y < m_vMin.y) m_vMin.y = y;
	if(y > m_vMax.y) m_vMax.y = y;
	if(z < m_vMin.z) m_vMin.z = z;
	if(z > m_vMax.z) m_vMax.z = z;
}

//-------------------------------------------------------------------------------------------------
inline void U2Aabb::Extend(const U2Aabb& box)
{
	if(box.m_vMin.x < m_vMin.x) m_vMin.x = box.m_vMin.x;
	if(box.m_vMin.y < m_vMin.y) m_vMin.y = box.m_vMin.y;
	if(box.m_vMin.z < m_vMin.z) m_vMin.z = box.m_vMin.z;
	if(box.m_vMax.x > m_vMax.x) m_vMax.x = box.m_vMax.x;	
	if(box.m_vMax.y > m_vMax.y) m_vMax.y = box.m_vMax.y;	
	if(box.m_vMax.z > m_vMax.z) m_vMax.z = box.m_vMax.z;

}

//-------------------------------------------------------------------------------------------------
inline D3DXVECTOR3 U2Aabb::CornerPoint(int idx) const
{
	//     6 ----------7
	//	4  --------- 5
	//    
	//     2 ---------- 3   
	// 0 --------- 1
	U2ASSERT((idx >= 0) && (idx < 8));
	switch(idx)
	{
	case 0: return m_vMin;
	/*case 1: return D3DXVECTOR3(m_vMin.x, m_vMax.y, m_vMin.z);
	case 2: return D3DXVECTOR3(m_vMax.x, m_vMax.y, m_vMin.z);
	case 3: return D3DXVECTOR3(m_vMax.x, m_vMin.y, m_vMin.z);
	case 4: return m_vMax;
	case 5: return D3DXVECTOR3(m_vMin.x, m_vMax.y, m_vMax.z);
	case 6: return D3DXVECTOR3(m_vMin.x, m_vMin.y, m_vMax.z);
	default: return D3DXVECTOR3(m_vMax.x, m_vMin.y, m_vMax.z)*/;	
	case 1: return D3DXVECTOR3(m_vMax.x, m_vMin.y, m_vMin.z);
	case 2: return D3DXVECTOR3(m_vMin.x, m_vMin.y, m_vMax.z);
	case 3: return D3DXVECTOR3(m_vMax.x, m_vMin.y, m_vMax.z);	
	case 4: return D3DXVECTOR3(m_vMin.x, m_vMax.y, m_vMin.z);
	case 5: return D3DXVECTOR3(m_vMax.x, m_vMax.y, m_vMin.z);		
	case 6: return D3DXVECTOR3(m_vMin.x, m_vMax.y, m_vMax.z);
	default: return m_vMax;
	}	
}

//-------------------------------------------------------------------------------------------------
inline void U2Aabb::Transform(const D3DXMATRIX& m)
{
	D3DXVECTOR3 temp, min, max, corners[8];
	bool bFirst = true;
	int i;
	/*corners[0] = m_vMin;
	corners[1].x = m_vMin.x; corners[1].y = m_vMax.y; corners[1].z = m_vMin.z;
	corners[2].x = m_vMax.x; corners[2].y = m_vMax.y; corners[2].z = m_vMin.z;
	corners[3].x = m_vMax.x; corners[3].y = m_vMin.y; corners[3].z = m_vMin.z;
	corners[4]   = m_vMax;
	corners[5].x = m_vMin.x; corners[5].y = m_vMax.y; corners[5].z = m_vMax.z;
	corners[6].x = m_vMin.x; corners[6].y = m_vMin.y; corners[6].z = m_vMax.z;
	corners[7].x = m_vMax.x; corners[7].y = m_vMin.y; corners[7].z = m_vMax.z;*/

	//     6 ----------7
	//	4  --------- 5
	//    
	//     2 ---------- 3   
	// 0 --------- 1

	corners[0] = m_vMin;
	corners[1].x = m_vMax.x; corners[1].y = m_vMin.y; corners[1].z = m_vMin.z;
	corners[2].x = m_vMin.x; corners[4].y = m_vMin.y; corners[4].z = m_vMax.z;
	corners[3].x = m_vMax.x; corners[5].y = m_vMin.y; corners[5].z = m_vMax.z;		
	corners[4].x = m_vMin.x; corners[3].y = m_vMax.y; corners[3].z = m_vMin.z;
	corners[5].x = m_vMax.x; corners[2].y = m_vMax.y; corners[2].z = m_vMin.z;
	corners[6].x = m_vMin.x; corners[7].y = m_vMax.y; corners[7].z = m_vMax.z;
	corners[7] = m_vMax;	

	D3DXMATRIX transposeTM;
	D3DXMatrixTranspose(&transposeTM, &m);

	for(i=0; i < 8; ++i)
	{

		D3DXVec3TransformCoord(&temp, &corners[i], &transposeTM);
		if(bFirst || temp.x > max.x) max.x = temp.x;
		if(bFirst || temp.y > max.y) max.y = temp.y;
		if(bFirst || temp.z > max.z) max.z = temp.z;
		if(bFirst || temp.x < min.x) min.x = temp.x;
		if(bFirst || temp.y < min.y) min.y = temp.y;
		if(bFirst || temp.z < min.z) min.z = temp.z;
		bFirst = false;
	}

	m_vMin = min;
	m_vMax = max;
}

// create from minimum and maximum vectors
inline void U2Aabb::Set2(const D3DXVECTOR3 &vMin, const D3DXVECTOR3 &vMax)
{
	//     6 ----------7
	//	4  --------- 5
    //    
	//     2 ---------- 3   
	// 0 --------- 1

	// calculate points
	m_Points[0] = D3DXVECTOR3(vMin.x, vMin.y, vMin.z);
	m_Points[1] = D3DXVECTOR3(vMax.x, vMin.y, vMin.z);
	m_Points[2] = D3DXVECTOR3(vMin.x, vMin.y, vMax.z);
	m_Points[3] = D3DXVECTOR3(vMax.x, vMin.y, vMax.z);
	m_Points[4] = D3DXVECTOR3(vMin.x, vMax.y, vMin.z);
	m_Points[5] = D3DXVECTOR3(vMax.x, vMax.y, vMin.z);
	m_Points[6] = D3DXVECTOR3(vMin.x, vMax.y, vMax.z);
	m_Points[7] = D3DXVECTOR3(vMax.x, vMax.y, vMax.z);
	m_vMin = vMin;
	m_vMax = vMax;
}


// create from set of points
inline void U2Aabb::Set(const void *pPoints, int iNumPoints, int iStride)
{
	m_vMin = D3DXVECTOR3( FLT_MAX, FLT_MAX, FLT_MAX);
	m_vMax = D3DXVECTOR3(-FLT_MAX,-FLT_MAX,-FLT_MAX);

	char *pData = (char *)pPoints;
	for(int i=0; i<iNumPoints; i++)
	{
		const D3DXVECTOR3 &vPoint = *(D3DXVECTOR3*)pData;
		if(vPoint.x < m_vMin.x) m_vMin.x = vPoint.x;
		if(vPoint.y < m_vMin.y) m_vMin.y = vPoint.y;
		if(vPoint.z < m_vMin.z) m_vMin.z = vPoint.z;

		if(vPoint.x > m_vMax.x) m_vMax.x = vPoint.x;
		if(vPoint.y > m_vMax.y) m_vMax.y = vPoint.y;
		if(vPoint.z > m_vMax.z) m_vMax.z = vPoint.z;

		// next position
		pData += iStride;
	}
	// create from vectors
	Set2(m_vMin, m_vMax);

}

