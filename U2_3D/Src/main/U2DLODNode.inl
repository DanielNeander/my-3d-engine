inline D3DXVECTOR3& U2DLODNode::GetLocalCenter()
{
	return m_vLocalLODCenter;
}


inline const D3DXVECTOR3& U2DLODNode::GetLocalCenter() const
{
	return m_vLocalLODCenter;

}


inline const D3DXVECTOR3& U2DLODNode::GetWorldCenter() const
{
	return m_vWorldLODCenter;
}


inline float U2DLODNode::GetLocalMinDistance(int i) const
{
	U2ASSERT(0 <= i && i < GetChildCnt());
	return m_localMinDists.GetElem(i);
}


inline float U2DLODNode::GetLocalMaxDistance(int i) const
{
	U2ASSERT(0 <= i && i < GetChildCnt());
	return m_localMaxDists.GetElem(i);
}


inline float U2DLODNode::GetWorldMinDistance(int i) const
{
	U2ASSERT(0 <= i && i < GetChildCnt());
	return m_worldMinDists.GetElem(i);
}	


inline float U2DLODNode::GetWorldMaxDistance(int i) const
{
	U2ASSERT(0 <= i && i < GetChildCnt());
	return m_worldMaxDists.GetElem(i);
}