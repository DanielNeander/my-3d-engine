//-------------------------------------------------------------------------------------------------
inline void U2N2Mesh::SetVertexBufferByteSize(unsigned int sizeInBytes)
{
	m_uVbSize = sizeInBytes;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2N2Mesh::GetVertexBufferByteSize() const
{
	return m_uVbSize;
}

//-------------------------------------------------------------------------------------------------
inline void U2N2Mesh::SetIndexBufferByteSize(unsigned int sizeInBytes)
{
	m_uIbSize = sizeInBytes;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2N2Mesh::GetIndexBufferByteSize() const
{
	return m_uIbSize;
}

//-------------------------------------------------------------------------------------------------
inline LPDIRECT3DVERTEXBUFFER9 U2N2Mesh::GetVB() const
{
	return m_pVB;
}



//-------------------------------------------------------------------------------------------------
inline uint32 U2N2Mesh::GetVertexStride() const
{
	return m_uiVertexStride;
}

//-------------------------------------------------------------------------------------------------
inline void U2N2Mesh::SetVertexStride(uint32 uStride )
{
	m_uiVertexStride = uStride;
}

//-------------------------------------------------------------------------------------------------
inline void U2N2Mesh::SetUsages(int useFlags)
{
	this->m_iVertexUsage = useFlags;
	this->m_iIndexUsage  = useFlags;
}

//-------------------------------------------------------------------------------------------------
inline void U2N2Mesh::SetVertexUsage(int useFlag)
{
	m_iVertexUsage = useFlag;
}

inline void U2N2Mesh::SetIndexUsage(int useFlag)
{
	m_iIndexUsage = useFlag;
}

//-------------------------------------------------------------------------------------------------
inline void U2N2Mesh::SetVertexComponents(int compMask)
{
	m_iVertexCompMask = compMask & AllComponents;	
}

//-------------------------------------------------------------------------------------------------
inline void U2N2Mesh::CreateMeshGroups(uint32 num)
{
	U2ASSERT(num > 0);	
	m_uNumGroups = num;
	m_meshGroups.Resize(num);
	for(int i=0; i < num; ++i)
	{
		m_meshGroups.SetElem(i, U2_NEW nMeshGroup);
	}
}

//-------------------------------------------------------------------------------------------------
inline int U2N2Mesh::GetNumGroups() const
{
	return m_uNumGroups;
}

//-------------------------------------------------------------------------------------------------
inline nMeshGroup& U2N2Mesh::Group(uint32 idx) const
{
	return *m_meshGroups[idx];
}

//-------------------------------------------------------------------------------------------------
inline nMeshGroup* U2N2Mesh::GroupPtr(uint32 idx) const
{
	return m_meshGroups[idx];
}

//-------------------------------------------------------------------------------------------------
inline void U2N2Mesh::SetGroupIdx(uint32 i)
{
	m_uiGroupIdx = i;
}

//-------------------------------------------------------------------------------------------------
inline uint32 U2N2Mesh::GetGroupIdx() const
{
	return m_uiGroupIdx;
}

//-------------------------------------------------------------------------------------------------
inline uint16 U2N2Mesh::GetNumEdges() const
{
	return m_usNumEdges;
}

//-------------------------------------------------------------------------------------------------
inline void U2N2Mesh::SetNumEdges(uint16 numEdges)
{
	m_usNumEdges = numEdges;
}

//-------------------------------------------------------------------------------------------------
inline void U2N2Mesh::SetEdgeBufferSize(uint32 sizeInBytes)
{
	m_uiEbSize = sizeInBytes;	
}

//-------------------------------------------------------------------------------------------------
inline uint32 U2N2Mesh::GetEdgeBufferSize() const
{
	return m_uiEbSize;
}
