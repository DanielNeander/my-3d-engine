inline U2AnimKeyData::U2AnimKeyData()
:m_fTime(1.f),
m_scale(1.f, 1.f, 1.f),
m_fAccumWeight(0.f)
{
	
}


inline void U2AnimKeyData::SetTime(float fTime)
{
	m_fTime = fTime;
}


inline float U2AnimKeyData::GetTime() const
{
	return m_fTime;
}


inline void U2AnimKeyData::SetTrans(const D3DXVECTOR3& t)
{
	m_trans = t;
}


inline const D3DXVECTOR3& U2AnimKeyData::GetTrans() const
{
	return m_trans;
}

inline void U2AnimKeyData::SetQuatRot(const D3DXQUATERNION& q)
{
	m_quatRot = q;
}


inline const D3DXQUATERNION& U2AnimKeyData::GetQuatRot() const
{
	return m_quatRot;
}

inline void U2AnimKeyData::SetScale(const D3DXVECTOR3& s)
{
	m_scale = s;
}


inline const D3DXVECTOR3& U2AnimKeyData::GetScale() const
{
	return m_scale;
}


inline D3DXMATRIX U2AnimKeyData::GetMatrix() const
{	
	D3DXMATRIX m;
	D3DXMatrixIdentity(&m);
	D3DXMatrixScaling(&m, m_scale.x, m_scale.y, m_scale.z);
	D3DXVECTOR3 axis;
	float fAngle;
	D3DXQuaternionToAxisAngle(&m_quatRot, &axis, &fAngle);
	D3DXMATRIX temp;
	D3DXMatrixRotationAxis(&temp, &axis, fAngle);
	m = m * temp;
	m._41 = m_trans.x;
	m._42 = m_trans.y;
	m._43 = m_trans.z;
	return m;
}


inline void U2AnimKeyData::SetAccumWeight(float weight)
{
	m_fAccumWeight = weight;
}


inline float U2AnimKeyData::GetAccumWeight() const
{
	return m_fAccumWeight;
}