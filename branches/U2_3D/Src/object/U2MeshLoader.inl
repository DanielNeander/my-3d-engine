//-------------------------------------------------------------------------------------------------
inline U2MeshLoader::U2MeshLoader()
:m_pFile(NULL),
m_eIndexType(Index16),
m_uNumVertices(),
m_uVertexStride(0),
m_uFileVertexStride(0),
m_uNumTriangles(0),
m_uNumIndices(0),
m_uNumEdges(0),
m_uVertexComponents(0),
m_uFileVertexComponents(0),
m_uValidVertexComponents(0xFFFF)
{

}

//-------------------------------------------------------------------------------------------------
inline U2MeshLoader::~U2MeshLoader()
{
	U2ASSERT(!m_pFile);		
	m_skinMods.RemoveAll();

	for(unsigned int i=0; i < m_controllers.Size(); ++i)
	{
		U2_DELETE m_controllers[i];
	}
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshLoader::SetFilename(const TCHAR* filename)
{
	m_szFilename = filename;
}

//-------------------------------------------------------------------------------------------------
inline const TCHAR* U2MeshLoader::GetFilename() const
{
	return m_szFilename;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshLoader::SetIndexType(IndexType type)
{
	m_eIndexType = type;
}

//-------------------------------------------------------------------------------------------------
inline U2MeshLoader::IndexType U2MeshLoader::GetIndexType() const
{
	return m_eIndexType;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshLoader::SetValidVertexComponents(unsigned int components)
{
	U2ASSERT(components);
	m_uValidVertexComponents = components;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2MeshLoader::GetValidVertexComponents()
{
	return m_uValidVertexComponents;
}

//-------------------------------------------------------------------------------------------------
inline bool U2MeshLoader::Open()
{
	U2ASSERT(0 != m_uFileVertexStride);
	U2ASSERT(0 != m_uFileVertexComponents);
	U2ASSERT(0 != m_uValidVertexComponents);

	m_uVertexComponents = m_uValidVertexComponents & m_uFileVertexComponents;
	m_uVertexStride = GetVertexChannelCntFromMask(m_uVertexComponents);
	return true;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshLoader::Close()
{
//	U2ERR(!_T("U2MeshLoader::Close() called!"));
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2MeshLoader::GetNumGroups() const
{
	return m_uNumGroups;
}

//-------------------------------------------------------------------------------------------------
inline const nMeshGroup& U2MeshLoader::GetMeshGroup(uint32 idx) const
{
	return *m_meshGroups[idx];
}


//-------------------------------------------------------------------------------------------------
inline unsigned int U2MeshLoader::GetNumVertices() const
{
	return m_uNumVertices;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2MeshLoader::GetVertexChannelCnt() const
{
	return m_uVertexStride;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2MeshLoader::GetNumTriangles() const
{
	return m_uNumTriangles;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2MeshLoader::GetNumIndices() const
{
	return m_uNumIndices;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2MeshLoader::GetNumEdges() const
{
	return m_uNumEdges;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2MeshLoader::GetVertexComponents() const
{
	return m_uVertexComponents;
}

//-------------------------------------------------------------------------------------------------
inline bool U2MeshLoader::ReadVertices(void *pvBuffer, int buffSize)
{
	return false;
}

//-------------------------------------------------------------------------------------------------
inline bool U2MeshLoader::ReadIndices(void *pvBuffer, int buffSize)
{
	return false;
}

//-------------------------------------------------------------------------------------------------
inline bool U2MeshLoader::ReadEdges(void *pvBuffer, int buffSize)
{
	return false;
}


//-------------------------------------------------------------------------------------------------
inline unsigned int U2MeshLoader::GetVertexChannelCntFromMask(int compMask)
{
	int width = 0;
	if (compMask & Coord)    width += 3;
	if (compMask & Normal)   width += 3;
	if (compMask & Uv0)      width += 2;
	if (compMask & Uv1)      width += 2;
	if (compMask & Uv2)      width += 2;
	if (compMask & Uv3)      width += 2;
	if (compMask & Color)    width += 4;
	if (compMask & Tangent)  width += 3;
	if (compMask & Binormal) width += 3;
	if (compMask & Weights)  width += 4;
	if (compMask & JIndices) width += 4;
	if (compMask & Coord4)   width += 4;

	return width;
}

//-------------------------------------------------------------------------------------------------
inline const U2Aabb& U2MeshLoader::GetBoundingBox() const
{
	return m_bbox;
}

