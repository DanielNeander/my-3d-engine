#ifndef RENDERER_NO_MESH_H
#define RENDERER_NO_MESH_H


class noMeshSection /*: public tRefObj*/ {

public:

	inline noMeshSection()	{}
	inline virtual ~noMeshSection()	{}


	noMeshSection& operator=(const noMeshSection& rhs);
	/// set first vertex
	void SetFirstVertex(int i);
	/// get index of first vertex
	int GetFirstVertex() const;
	/// set number of vertices
	void SetNumVertices(int i);
	/// get number of vertices
	int GetNumVertices() const;
	/// set first index
	void SetFirstIndex(int i);
	/// get index of first primitive index
	int GetFirstIndex() const;
	/// set number of indices
	void SetNumIndices(int i);
	/// get number of indices
	int GetNumIndices() const;

private:
	int firstVertex;
	int numVertices;
	int firstIndex;
	int numIndices;
};


class noGeometryBuffer;

class noMesh /*public tRefObj*/ {

public:
	enum VertexComponent
	{
		Coord    = (1<<0),
		Normal   = (1<<1),
		Uv0      = (1<<2),
		Uv1      = (1<<3),
		Uv2      = (1<<4),
		Uv3      = (1<<5),
		Color    = (1<<6),
		Tangent  = (1<<7),
		Binormal = (1<<8),
		Weights  = (1<<9),
		JIndices = (1<<10),
		Coord4   = (1<<11),

		NumVertexComponents = 12,
		AllComponents = ((1<<NumVertexComponents) - 1),
	};
	
	enum Usage
	{
		// read/write behavior (mutually exclusive)
		WriteOnce = (1<<0),     ///< (default) CPU only fills the vertex buffer once, and never touches it again
		ReadOnly  = (1<<1),     ///< CPU reads from the vertex buffer, which can never be rendered
		WriteOnly = (1<<2),     ///< CPU writes frequently to vertex buffer, but never read data back
		ReadWrite = (1<<3),     ///< CPU writes and reads the vertex buffer, which is also rendered

		// patch types (mutually exclusive)
		NPatch  = (1<<4),
		RTPatch = (1<<5),

		// use as point sprite buffer?
		PointSprite = (1<<6),

		// needs vertex shader?
		NeedsVertexShader = (1<<7),
	};

	enum
	{
		InvalidIndex = 0xffff, // invalid index constant
	};

	void SetVertexBufferByteSize(unsigned int sizeInBytes);
	unsigned int GetVertexBufferByteSize() const;

	void SetIndexBufferByteSize(unsigned int sizeInBytes);
	unsigned int GetIndexBufferByteSize() const;

	bool CreatevertexBuffer();
	float* LockVertices();
	void UnlockVertices();

	bool CreateIndexBuffer();	
	uint16* LockIndices();
	void UnlockIndices();

	void SetUsages(int useFlags);
	void SetVertexUsage(int useFlag);
	void SetIndexUsage(int useFlag);
	void SetVertexComponents(int compMask);

	uint32 GetVertexStride() const;
	void SetVertexStride(uint32 uStride);

	void CreateMeshGroups(uint32 num);
	int GetNumGroups() const;

	noMeshSection& Group(unsigned int idx) const;

	// 2010/06/08 changed 
	// TerrainoMeshSection을 추가하기 위해 protected로 변경.
	noMeshSection* GroupPtr(unsigned int idx) const;
		
private:
	hkArray<noMeshSection*>	m_meshSections;
	
	unsigned int m_vbSize;
	unsigned int m_ibSize;
	//uint32 m_uiEbSize;
	
	noGeometryBuffer* m_buffer;	
	int m_vertexUsage;
	int m_indexUsage;
	int m_vertexCompMask;
	DWORD m_d3dVBLockFlags;
	DWORD m_d3dIBLockFlags;
	
	void* m_vretexBuffer;
	void* m_indexBuffer;

	uint32 m_vertexStride;

	uint32 m_numGroups;

	
};


//------------------------------------------------------------------------------
/**
*/
inline
void noMeshSection::SetFirstVertex(int i)
{
	this->firstVertex = i;
}

//------------------------------------------------------------------------------
/**
*/
inline
int noMeshSection::GetFirstVertex() const
{
	return this->firstVertex;
}

//------------------------------------------------------------------------------
/**
*/
inline
void noMeshSection::SetNumVertices(int i)
{
	this->numVertices = i;
}

//------------------------------------------------------------------------------
/**
*/
inline
int noMeshSection::GetNumVertices() const
{
	return this->numVertices;
}

//------------------------------------------------------------------------------
/**
*/
inline
void noMeshSection::SetFirstIndex(int i)
{
	this->firstIndex = i;
}

//------------------------------------------------------------------------------
/**
*/
inline
int noMeshSection::GetFirstIndex() const
{
	return this->firstIndex;
}

//------------------------------------------------------------------------------
/**
*/
inline
void noMeshSection::SetNumIndices(int i)
{
	this->numIndices = i;
}

//------------------------------------------------------------------------------
/**
*/
inline
int noMeshSection::GetNumIndices() const
{
	return this->numIndices;
}



//-------------------------------------------------------------------------------------------------
inline void noMesh::SetVertexBufferByteSize(unsigned int sizeInBytes)
{
	m_vbSize = sizeInBytes;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int noMesh::GetVertexBufferByteSize() const
{
	return m_vbSize;
}

//-------------------------------------------------------------------------------------------------
inline void noMesh::SetIndexBufferByteSize(unsigned int sizeInBytes)
{
	m_ibSize = sizeInBytes;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int noMesh::GetIndexBufferByteSize() const
{
	return m_ibSize;
}


//-------------------------------------------------------------------------------------------------
inline uint32 noMesh::GetVertexStride() const
{
	return m_vertexStride;
}

//-------------------------------------------------------------------------------------------------
inline void noMesh::SetVertexStride(uint32 uStride )
{
	m_vertexStride = uStride;
}

//-------------------------------------------------------------------------------------------------
inline void noMesh::SetUsages(int useFlags)
{
	this->m_vertexUsage = useFlags;
	this->m_indexUsage  = useFlags;
}

//-------------------------------------------------------------------------------------------------
inline void noMesh::SetVertexUsage(int useFlag)
{
	m_vertexUsage = useFlag;
}

inline void noMesh::SetIndexUsage(int useFlag)
{
	m_indexUsage = useFlag;
}

//-------------------------------------------------------------------------------------------------
inline void noMesh::SetVertexComponents(int compMask)
{
	m_vertexCompMask = compMask & AllComponents;	
}

//-------------------------------------------------------------------------------------------------
inline void noMesh::CreateMeshGroups(uint32 num)
{
	//ASSERT(num > 0);	
	m_numGroups = num;
	m_meshSections.setSize(num);
	for(uint32 i=0; i < num; ++i)
	{
		m_meshSections[i] = new noMeshSection;
	}
}

//-------------------------------------------------------------------------------------------------
inline int noMesh::GetNumGroups() const
{
	return m_numGroups;
}

//-------------------------------------------------------------------------------------------------
inline noMeshSection& noMesh::Group(uint32 idx) const
{
	return *m_meshSections[idx];
}

//-------------------------------------------------------------------------------------------------
inline noMeshSection* noMesh::GroupPtr(uint32 idx) const
{
	return m_meshSections[idx];
}


#endif