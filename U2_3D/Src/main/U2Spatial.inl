//-------------------------------------------------------------------------------------------------
inline 
U2Node* U2Spatial::GetParent()
{
	return m_pParent;
}

//-------------------------------------------------------------------------------------------------
inline 
const U2Node* U2Spatial::GetParent() const
{
	return m_pParent;
}

//-------------------------------------------------------------------------------------------------
inline 
bool U2Spatial::IsCulled() const
{
	return GetBit(CULLED_MASK);
}

//-------------------------------------------------------------------------------------------------
inline 
void U2Spatial::SetCulledFlag(bool bAppCulled)
{
	SetBit(bAppCulled, CULLED_MASK);
}


//-------------------------------------------------------------------------------------------------
// transform
inline void	U2Spatial::SetLocalTrans(const D3DXVECTOR3& trans)
{
	m_tmLocal.SetTrans(trans);
}

//-------------------------------------------------------------------------------------------------
inline void	U2Spatial::SetLocalTrans(float x, float y, float z)
{
	m_tmLocal.SetTrans(D3DXVECTOR3(x, y, z));
}

//-------------------------------------------------------------------------------------------------
inline const D3DXVECTOR3& U2Spatial::GetLocalTrans() const
{
	return m_tmLocal.GetTrans();
}

//-------------------------------------------------------------------------------------------------
inline void	U2Spatial::SetLocalRot(const D3DXMATRIX& rot)
{
	m_tmLocal.SetRot(rot);
}

//-------------------------------------------------------------------------------------------------
inline const D3DXMATRIX U2Spatial::GetLocalRot() const
{
	return m_tmLocal.GetRot();
}

//-------------------------------------------------------------------------------------------------
inline void	U2Spatial::SetLocalRot(float angle, float x, float y, float z)
{
	D3DXMATRIX mat;
	D3DXMatrixRotationAxis(&mat, &D3DXVECTOR3(x, y, z), angle);
	m_tmLocal.SetRot(mat);
}

//-------------------------------------------------------------------------------------------------
inline void	U2Spatial::GetLocalRot(float &angle, float &x, float &y, float &z)
{	
	D3DXVECTOR3 scale;
	D3DXVECTOR3 trans;
	D3DXQUATERNION rot;
	D3DXMatrixDecompose(&scale, &rot, &trans, &m_tmLocal.GetRot());

	D3DXVECTOR3 axis;
	D3DXQuaternionToAxisAngle(&rot, &axis, &angle);
	x = axis.x; 
	y = axis.y;
	z = axis.z;
}

//-------------------------------------------------------------------------------------------------
inline void	U2Spatial::SetLocalRot(const D3DXQUATERNION& rot)
{
	D3DXMATRIX mat;
	D3DXVECTOR3 axis;	
	float angle;
	D3DXQuaternionToAxisAngle(&rot, &axis, &angle);
	D3DXMatrixRotationAxis(&mat, &axis, angle);
	m_tmLocal.SetRot(mat);	
}

//-------------------------------------------------------------------------------------------------
inline void U2Spatial::GetLocalRot(D3DXQUATERNION& outRot) const
{
	D3DXVECTOR3 scale;
	D3DXVECTOR3 trans;
	D3DXMatrixDecompose(&scale, &outRot, &trans, &m_tmLocal.GetRot());
}

//-------------------------------------------------------------------------------------------------
inline void	U2Spatial::SetRotateAboutCenter(const D3DXMATRIX& rot, const D3DXVECTOR3& cen, 
										 const D3DXVECTOR3& trans )
{

}

//-------------------------------------------------------------------------------------------------
inline void U2Spatial::SetLocalScale(float scale)
{
	m_tmLocal.SetUniformScale(scale);
}

//-------------------------------------------------------------------------------------------------
inline void	U2Spatial::SetLocalScale(float scaleX, float scaleY, float scaleZ)
{
	m_tmLocal.SetScale(D3DXVECTOR3(scaleX, scaleY, scaleZ));
}

//-------------------------------------------------------------------------------------------------
inline D3DXVECTOR3 U2Spatial::GetLocalScale() const
{
	return m_tmLocal.GetScale();
}

//-------------------------------------------------------------------------------------------------
inline const D3DXVECTOR3 U2Spatial::GetWorldTranslate() const
{
	return m_tmWorld.GetTrans();
}

//-------------------------------------------------------------------------------------------------
inline const D3DXMATRIX	U2Spatial::GetWorldRotate() const
{
	return m_tmWorld.GetRot();
}

//-------------------------------------------------------------------------------------------------
inline D3DXVECTOR3 U2Spatial::GetWorldScale() const
{
	return m_tmWorld.GetScale();
}

//-------------------------------------------------------------------------------------------------
// for occlusion culling
inline void U2Spatial::SetVisibleObject(bool bDisplay)
{
	
}

//-------------------------------------------------------------------------------------------------
inline bool U2Spatial::GetVisibleObject() const
{
	return true;
}

//-------------------------------------------------------------------------------------------------
inline void	U2Spatial::SetWorldTranslate(const D3DXVECTOR3 trans)
{
	m_tmWorld.SetTrans(trans);
}

//-------------------------------------------------------------------------------------------------
inline void	U2Spatial::SetWorldRotate(const D3DXMATRIX& rot)
{
	m_tmWorld.SetRot(rot);
}

//-------------------------------------------------------------------------------------------------
inline void	U2Spatial::SetWorldScale(float fScale)
{
	m_tmWorld.SetUniformScale(fScale);

}

//-------------------------------------------------------------------------------------------------
inline U2RSList& U2Spatial::GetRSList()
{
	return m_renderStateList;
}

//-------------------------------------------------------------------------------------------------
inline const U2RSList& U2Spatial::GetRSList() const
{
	return m_renderStateList;
}

//-------------------------------------------------------------------------------------------------
inline void U2Spatial::AttachRenderState(U2RenderState* pRS)
{
	U2ASSERT(GetRenderState(pRS->Type()) == NULL);
	m_renderStateList.InsertToHead(pRS);
}

//-------------------------------------------------------------------------------------------------
inline void U2Spatial::DetachRenderState(U2RenderState* pRS)	
{
	m_renderStateList.Remove(pRS);
}

//-------------------------------------------------------------------------------------------------
inline void U2Spatial::DetachAllRenderStates()
{
	m_renderStateList.RemoveAll();
}

//-------------------------------------------------------------------------------------------------
inline U2RenderState* U2Spatial::GetRenderState(int rsType) const	
{
	U2ASSERT(rsType >= 0 && rsType < U2RenderState::MAX_TYPES);

	U2ListNode<U2RenderState*>* pNodeIter = m_renderStateList.GetHeadNode();
	U2RenderState* pRS;
	while(pNodeIter)
	{		
		pRS = pNodeIter->m_elem;
		pNodeIter = m_renderStateList.GetNextNode(pNodeIter);
		if(pRS && pRS->Type() == rsType)
			return pRS;
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
inline uint32 U2Spatial::GetLightCnt() const
{
	return m_aspLights.Size();
}

//-------------------------------------------------------------------------------------------------
inline U2Light* U2Spatial::GetLight(uint32 idx)
{
	return m_aspLights.GetElem(idx);
}

//-------------------------------------------------------------------------------------------------
inline void U2Spatial::DetachAllLights()
{
	m_aspLights.RemoveAll();
}

//-------------------------------------------------------------------------------------------------
inline int U2Spatial::GetEffectCnt() const
{
	return m_aspEffects.Size();
}

//-------------------------------------------------------------------------------------------------
inline U2Effect* U2Spatial::GetEffect(uint32 idx) 
{
	return m_aspEffects.GetElem(idx);
}

inline const U2Effect* U2Spatial::GetEffect(uint32 idx) const 
{
	return m_aspEffects.GetElem(idx);
}

//-------------------------------------------------------------------------------------------------
inline void U2Spatial::DetachAllEffects()
{
	m_aspEffects.RemoveAll();
}

//-------------------------------------------------------------------------------------------------
inline void U2Spatial::SetStartEffect(uint32 i)
{
	U2ASSERT(0 <= i && i < m_aspEffects.Size());
	m_uStartEffect = i;
}

//-------------------------------------------------------------------------------------------------
inline uint32 U2Spatial::GetStartEffect() const
{
	return m_uStartEffect;
}

//-------------------------------------------------------------------------------------------------
inline void U2Spatial::UpdateRenderContext(U2RenderContext* pRCxt)
{
	
}

//-------------------------------------------------------------------------------------------------
inline void U2Spatial::SetBoundingBox(const U2Aabb& box)
{
	m_bbox = box;
}

//-------------------------------------------------------------------------------------------------
inline const U2Aabb& U2Spatial::GetBoundingBox() const
{
	return m_bbox;
}

inline void U2Spatial::SetActive(bool bActive)
{
	m_bActive = bActive;
}

//-------------------------------------------------------------------------------------------------
inline bool U2Spatial::GetActive() const
{
	return m_bActive;
}