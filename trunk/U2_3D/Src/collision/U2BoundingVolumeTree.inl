//-------------------------------------------------------------------------------------------------
inline U2BoundingVolumeTree* U2BoundingVolumeTree::GetLChild()
{
	return m_pLChild;
}

//-------------------------------------------------------------------------------------------------
inline U2BoundingVolumeTree* U2BoundingVolumeTree::GetRChild()
{
	return m_pRChild;
}

//-------------------------------------------------------------------------------------------------
inline bool U2BoundingVolumeTree::IsInteriorNode() const
{
	return m_pLChild || m_pRChild;
}
inline bool U2BoundingVolumeTree::IsLeafNode() const
{
	return !m_pLChild && !m_pRChild;
}

//-------------------------------------------------------------------------------------------------
inline const U2TriList* U2BoundingVolumeTree::GetMesh() const
{
	return m_pMesh;
}

//-------------------------------------------------------------------------------------------------
inline const U2BoundingVolume* U2BoundingVolumeTree::GetWorldBound() const
{
	return m_spWorldBound;
}

//-------------------------------------------------------------------------------------------------
inline unsigned short U2BoundingVolumeTree::GetTriangleCnt() const
{
	return m_usTrisCnt;
}

//-------------------------------------------------------------------------------------------------
inline unsigned short U2BoundingVolumeTree::GetTriangle(unsigned short i) const
{
	return m_pusTriangle[i];
}

//-------------------------------------------------------------------------------------------------
inline const unsigned short* U2BoundingVolumeTree::GetTriangles()const
{
	return m_pusTriangle;
}