inline U2AnimKeyTrack::U2AnimKeyTrack()
{

}

inline void U2AnimKeyTrack::SetName(const U2DynString& name)
{
	m_szName = name;
}


inline const U2DynString& U2AnimKeyTrack::GetName() const
{
	return m_szName;
}


inline void U2AnimKeyTrack::SetNumKeyDatas(uint32 num)
{
	m_keyDataArray.Resize(num);
}


inline uint32 U2AnimKeyTrack::GetNumKeyDatas() const
{
	return m_keyDataArray.Size();
}

inline void U2AnimKeyTrack::SetKeyData(uint32 idx, const U2AnimKeyData& keyData)
{
	m_keyDataArray.SetElem(idx, (U2AnimKeyData*)&keyData);
}

inline void U2AnimKeyTrack::AddKeyData(const U2AnimKeyData& keyData)
{
	m_keyDataArray.AddElem( (U2AnimKeyData*)&keyData);
}

inline const U2AnimKeyData& U2AnimKeyTrack::GetKeyData(uint32 idx) const
{
	return *m_keyDataArray.GetElem(idx);
}

inline U2PrimitiveVec<U2AnimKeyData*>& U2AnimKeyTrack::KeyDataArray()
{
	return m_keyDataArray;
}
