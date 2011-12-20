inline U2MeshBuilder::Vertex::Vertex()
:m_usCompMask(0),
m_usFlags(0),
m_tangent(VECTOR3_ZERO),
m_binormal(VECTOR3_ZERO)
{

}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::SetVert(const D3DXVECTOR3& v)
{
	m_vert = v;
	m_usCompMask |= VERT;
}

//-------------------------------------------------------------------------------------------------
inline const D3DXVECTOR3& U2MeshBuilder::Vertex::GetVert() const
{
	return m_vert;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::SetNormal(const D3DXVECTOR3& v)
{
	m_normal = v;
	m_usCompMask |= NORMAL;
}

//-------------------------------------------------------------------------------------------------
inline const D3DXVECTOR3& U2MeshBuilder::Vertex::GetNormal() const
{
	return m_normal;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::SetTangent(const D3DXVECTOR3& v)
{
	m_tangent = v;
	m_usCompMask |= TANGENT;
}

//-------------------------------------------------------------------------------------------------
inline const D3DXVECTOR3& U2MeshBuilder::Vertex::GetTangent() const
{
	return m_tangent;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::SetBinormal(const D3DXVECTOR3& v)
{
	m_binormal = v;
	m_usCompMask |= BINORMAL;
}

//-------------------------------------------------------------------------------------------------
inline const D3DXVECTOR3& U2MeshBuilder::Vertex::GetBinormal() const
{
	return m_binormal;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::SetColor(const D3DXVECTOR4& v)
{
	m_color = v;
	m_usCompMask |= COLOR;
}

//-------------------------------------------------------------------------------------------------
inline const D3DXVECTOR4& U2MeshBuilder::Vertex::GetColor() const
{
	return m_color;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::SetUv(int layer, const D3DXVECTOR2& v)
{
	U2ASSERT(0 <= layer && layer < MAX_TEXTURE_LAYERS);
	m_uv[layer] = v;
	switch(layer)
	{
	case 0: m_usCompMask |= UV0; break;
	case 1: m_usCompMask |= UV1; break;
	case 2: m_usCompMask |= UV2; break;
	case 3: m_usCompMask |= UV3; break;
	}
}

//-------------------------------------------------------------------------------------------------
inline const D3DXVECTOR2& U2MeshBuilder::Vertex::GetUv(int layer) const
{
	U2ASSERT(0 <= layer && layer < MAX_TEXTURE_LAYERS);
	return m_uv[layer];
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::SetWeights(const D3DXVECTOR4& w)
{
	m_boneWeights = w;
	m_usCompMask |= BONE_WEIGHTS;

}

//-------------------------------------------------------------------------------------------------
inline const D3DXVECTOR4& U2MeshBuilder::Vertex::GetWeights() const
{
	return m_boneWeights;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::SetBoneIndices(const D3DXVECTOR4& boneIndices)
{
	m_boneIndices = boneIndices;
	m_usCompMask |= BONE_INDICES;
}

//-------------------------------------------------------------------------------------------------
inline const D3DXVECTOR4& U2MeshBuilder::Vertex::GetBoneIndices() const
{
	return m_boneIndices;
}

//-------------------------------------------------------------------------------------------------
inline bool U2MeshBuilder::Vertex::HasComponent(Component c) const
{
	return (m_usCompMask & c) == c;
}
//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::SetComponent(Component c)
{
	m_usCompMask = c;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::ZeroComponent(Component c)
{
	m_usCompMask |= c;
	switch(c)
	{
	case VERT:			m_vert = VECTOR3_ZERO; break;
	case NORMAL:		m_normal = VECTOR3_ZERO; break;
	case TANGENT:		m_tangent = VECTOR3_ZERO; break;
	case BINORMAL:		m_binormal = VECTOR3_ZERO; break;
	case COLOR:			m_color = VECTOR4_ZERO; break;
	case UV0:			m_uv[0] = VECTOR2_ZERO; break;
	case UV1:			m_uv[1] = VECTOR2_ZERO; break;
	case UV2:			m_uv[2] = VECTOR2_ZERO; break;
	case UV3:			m_uv[3] = VECTOR2_ZERO; break;
	case BONE_WEIGHTS:	m_boneIndices = VECTOR4_ZERO; break;
	case BONE_INDICES:	m_boneIndices = VECTOR4_ZERO; break;
	default:
		U2ASSERT(_T("INVALID VERTEX COMPONENT!"));
		break;		
	}
}

//-------------------------------------------------------------------------------------------------
inline void  U2MeshBuilder::Vertex::DelComponent(Component c)
{
	m_usCompMask &= ~c;
}

//-------------------------------------------------------------------------------------------------
inline int U2MeshBuilder::Vertex::GetComponentMask() const
{
	return m_usCompMask;
}


//-------------------------------------------------------------------------------------------------
inline bool U2MeshBuilder::Vertex::operator==(const Vertex& rhs) const
{
	return Compare(rhs) == 0;	
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::SetFlag(VertexFlag f)
{
	m_usFlags |= f;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::UnsetFlag(VertexFlag f)
{
	m_usFlags &= ~f;
}

//-------------------------------------------------------------------------------------------------
inline bool U2MeshBuilder::Vertex::CheckFlag(VertexFlag f) const
{
	return (m_usFlags & f) == f;
}

//-------------------------------------------------------------------------------------------------
inline int U2MeshBuilder::Vertex::GetVertexStride() const
{
	int w = 0;
	if (this->HasComponent(VERT))				w += 3;
	if (this->HasComponent(NORMAL))				w += 3;
	if (this->HasComponent(TANGENT))			w += 3;
	if (this->HasComponent(BINORMAL))			w += 3;
	if (this->HasComponent(COLOR))				w += 4;
	if (this->HasComponent(UV0))				w += 2;
	if (this->HasComponent(UV1))				w += 2;
	if (this->HasComponent(UV2))				w += 2;
	if (this->HasComponent(UV3))				w += 2;
	if (this->HasComponent(BONE_WEIGHTS))		w += 4;
	if (this->HasComponent(BONE_INDICES))		w += 4;
	return w;

}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::Transform(const D3DXMATRIX& m44, const D3DXMATRIX& m33)
{
	D3DXMATRIX transpose;
	D3DXMatrixIdentity(&transpose);
	D3DXVECTOR3 tempV;

	if (this->HasComponent(VERT))
	{		
		tempV = m_vert;
		D3DXMatrixTranspose(&transpose, &m44);
		D3DXVec3TransformCoord(&m_vert, &tempV, &transpose);		
	}
	if (this->HasComponent(NORMAL))
	{
		transpose = m44;		
		tempV = m_normal;
		D3DXMatrixTranspose(&transpose, &m44);
		transpose._14 = transpose._24 = transpose._34 = 0.f;
		D3DXVec3TransformCoord(&m_normal, &tempV, &transpose);		
		
	}
	if (this->HasComponent(TANGENT))
	{
		
		tempV = m_tangent;
		D3DXMatrixTranspose(&transpose, &m44);
		transpose._14 = transpose._24 = transpose._34 = 0.f;
		D3DXVec3TransformCoord(&m_tangent, &tempV, &transpose);		
	}
	if (this->HasComponent(BINORMAL))
	{
		
		tempV = m_binormal;
		D3DXMatrixTranspose(&transpose, &m44);
		transpose._14 = transpose._24 = transpose._34 = 0.f;
		D3DXVec3TransformCoord(&m_binormal, &tempV, &transpose);		
	}
	
}

//------------------------------------------------------------------------------
/**
Fills the vertex with the interpolated result of the parameters.
This ignores weights and bone indices. This method is useful
for clipping.
*/
//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::Interpolate(const Vertex& v0, const Vertex& v1, float lerp)
{
	
	if (v0.HasComponent(VERT))
	{
		this->SetVert(v0.m_vert + ((v1.m_vert - v0.m_vert) * lerp));
	}
	if (v0.HasComponent(NORMAL))
	{
		this->SetNormal(v0.m_normal + ((v1.m_normal - v0.m_normal) * lerp));
		D3DXVECTOR3 temp = m_normal;	
		D3DXVec3Normalize(&m_normal, &temp);
	}
	if (v0.HasComponent(TANGENT))
	{
		this->SetTangent(v0.m_tangent + ((v1.m_tangent - v0.m_tangent) * lerp));
		D3DXVECTOR3 temp = m_tangent;	
		D3DXVec3Normalize(&m_tangent, &temp);
	}
	if (v0.HasComponent(BINORMAL))
	{
		this->SetBinormal(v0.m_binormal + ((v1.m_binormal - v0.m_binormal) * lerp));
		D3DXVECTOR3 temp = m_binormal;	
		D3DXVec3Normalize(&m_binormal, &temp);
		
	}
	if (v0.HasComponent(COLOR))
	{
		this->SetColor(v0.m_color + ((v1.m_color - v0.m_color) * lerp));		
	}
	if (v0.HasComponent(UV0))
	{
		this->SetUv(0, v0.m_uv[0] + ((v1.m_uv[0] - v0.m_uv[0]) * lerp));
	}
	if (v0.HasComponent(UV1))
	{
		this->SetUv(1, v0.m_uv[1] + ((v1.m_uv[1] - v0.m_uv[1]) * lerp));
	}
	if (v0.HasComponent(UV2))
	{
		this->SetUv(2, v0.m_uv[2] + ((v1.m_uv[2] - v0.m_uv[2]) * lerp));
	}
	if (v0.HasComponent(UV3))
	{
		this->SetUv(3, v0.m_uv[3] + ((v1.m_uv[3] - v0.m_uv[3]) * lerp));
	}

}


//------------------------------------------------------------------------------
/**
Copy vertex component defined by component mask from source vertex.

@param  src     the source vertex
@param  mask    component mask which defines which components to copy
*/
//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::CopyComponentFromVertex(const Vertex& src, int mask)
{
	if (mask & Vertex::VERT)       this->SetVert(src.m_vert);
	if (mask & Vertex::NORMAL)      this->SetNormal(src.m_normal);
	if (mask & Vertex::TANGENT)     this->SetTangent(src.m_tangent);
	if (mask & Vertex::BINORMAL)    this->SetBinormal(src.m_binormal);
	if (mask & Vertex::COLOR)       this->SetColor(src.m_color);
	if (mask & Vertex::UV0)         this->SetUv(0, src.m_uv[0]);
	if (mask & Vertex::UV1)         this->SetUv(1, src.m_uv[1]);
	if (mask & Vertex::UV2)         this->SetUv(2, src.m_uv[2]);
	if (mask & Vertex::UV3)         this->SetUv(3, src.m_uv[3]);
	if (mask & Vertex::BONE_WEIGHTS)     this->SetWeights(src.m_boneWeights);
	if (mask & Vertex::BONE_INDICES)    this->SetBoneIndices(src.m_boneIndices);
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Vertex::CopyComponentFromComponent(Component from, Component to)
{
	// yet implmented

}

//-------------------------------------------------------------------------------------------------
inline U2MeshBuilder::Triangle::Triangle()
:m_usCompMask(0),
m_iUsageFlags(0),
m_iGroupId(0),
m_iMaterialId(0)
{
	int i;
	for(i=0; i < 3; ++i)
	{
		m_iVertIdx[i] = 0;
	}
}

//-------------------------------------------------------------------------------------------------
inline U2MeshBuilder::Triangle::~Triangle()
{

}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Triangle::SetVertexIndices(int i0, int i1, int i2)
{
	m_iVertIdx[0] = i0;
	m_iVertIdx[1] = i1;
	m_iVertIdx[2] = i2;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Triangle::GetVertexIndices(int& i0, int& i1, int& i2) const
{
	i0 = m_iVertIdx[0];
	i1 = m_iVertIdx[1];
	i2 = m_iVertIdx[2];
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Triangle::SetGroupId(int i)
{
	m_iGroupId = i;
	m_usCompMask |= GROUP_ID;
}

//-------------------------------------------------------------------------------------------------
inline int U2MeshBuilder::Triangle::GetGroupId() const
{
	return m_iGroupId;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Triangle::SetUsageFlags(int f)
{
	m_iUsageFlags = f;
}

//-------------------------------------------------------------------------------------------------
inline int U2MeshBuilder::Triangle::GetUsageFlags() const
{
	return m_iUsageFlags;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Triangle::SetMaterialId(int i)
{
	m_iMaterialId = i;
}

//-------------------------------------------------------------------------------------------------
inline int U2MeshBuilder::Triangle::GetMaterialId() const
{
	return m_iMaterialId;
}

//-------------------------------------------------------------------------------------------------
inline bool U2MeshBuilder::Triangle::HasComponent(Component c) const
{
	return (m_usCompMask & c ) == c;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Triangle::SetNormal(const D3DXVECTOR3& v)
{
	m_normal = v;
	m_usCompMask |= NORMAL;
}

//-------------------------------------------------------------------------------------------------
inline const D3DXVECTOR3& U2MeshBuilder::Triangle::GetNormal() const
{
	return m_normal;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Triangle::SetTangent(const D3DXVECTOR3& v)
{
	m_tangent = v;
	m_usCompMask |= TANGENT;
}

//-------------------------------------------------------------------------------------------------
inline const D3DXVECTOR3& U2MeshBuilder::Triangle::GetTangent() const
{
	return m_tangent;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Triangle::SetBinormal(const D3DXVECTOR3& v)
{
	m_binormal;
	m_usCompMask |= BINORMAL;
}

//-------------------------------------------------------------------------------------------------
inline const D3DXVECTOR3& U2MeshBuilder::Triangle::GetBinormal() const
{
	return m_binormal;
}

//-------------------------------------------------------------------------------------------------
inline bool U2MeshBuilder::Triangle::Equals(Triangle &other) const
{
	if (((this->m_iVertIdx[0] == other.m_iVertIdx[0])||
		(this->m_iVertIdx[0] == other.m_iVertIdx[1])||
		(this->m_iVertIdx[0] == other.m_iVertIdx[2])) &&
		((this->m_iVertIdx[1] == other.m_iVertIdx[0])||
		(this->m_iVertIdx[1] == other.m_iVertIdx[1])||
		(this->m_iVertIdx[1] == other.m_iVertIdx[2])) &&
		((this->m_iVertIdx[2] == other.m_iVertIdx[0])||
		(this->m_iVertIdx[2] == other.m_iVertIdx[1])||
		(this->m_iVertIdx[2] == other.m_iVertIdx[2])))
	{
		// all vertexIndices of this triangle are also in the other
		// that means that they are equal
		return true;
	};
	return false;
}


//-------------------------------------------------------------------------------------------------
inline U2MeshBuilder::Group::Group()
:m_iId(0),
m_iUsageFlags(0),
m_iFirstTriangle(0),
m_iNumTriangles(0),
m_pSkinMod(0)
{
	
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Group::SetId(int i)
{
	m_iId = i;
}

//-------------------------------------------------------------------------------------------------
inline int U2MeshBuilder::Group::GetId() const
{
	return m_iId;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Group::SetMaterialId(int i)
{
	m_iMaterialId = i;
}

//-------------------------------------------------------------------------------------------------
inline int U2MeshBuilder::Group::GetMaterialId() const
{
	return m_iMaterialId;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Group::SetFirstTriangle(int i)
{
	m_iFirstTriangle = i;
}

//-------------------------------------------------------------------------------------------------
inline int U2MeshBuilder::Group::GetFirstTriangle() const
{
	return m_iFirstTriangle;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Group::SetNumTriangles(int i)
{
	m_iNumTriangles = i;
}

//-------------------------------------------------------------------------------------------------
inline int U2MeshBuilder::Group::GetNumTriangles() const
{
	return m_iNumTriangles;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Group::SetUsageFlags(int f)
{
	m_iUsageFlags = f;
}

//-------------------------------------------------------------------------------------------------
inline int U2MeshBuilder::Group::GetUsageFlags() const
{
	return m_iUsageFlags;
}

//-------------------------------------------------------------------------------------------------
inline void U2MeshBuilder::Group::SetSkinModifier(U2SkinModifier* pSkinMod)
{
	m_pSkinMod = pSkinMod;
}

//-------------------------------------------------------------------------------------------------
inline U2SkinModifier* U2MeshBuilder::Group::GetSkinModifier() const
{
	return m_pSkinMod;
}

//-------------------------------------------------------------------------------------------------
inline bool U2MeshBuilder::HasVertexComponent(Vertex::Component c) const
{
	if(GetNumVertices() > 0)
	{
		return GetVertexAt(0).HasComponent(c);
	}
	else 
		return false;

}

inline void U2MeshBuilder::SetComponent(Vertex::Component c)
{
	for(int i=0; i < GetNumVertices(); ++i)
	{
		m_vertexArray[i]->SetComponent(c);
	}

}

/// delete a vertex component
inline void U2MeshBuilder::DelVertexComponent(Vertex::Component c)
{
	for(int i=0; i < GetNumVertices(); ++i)
	{
		m_vertexArray[i]->DelComponent(c);
	}

}
/// add a vertex
inline void U2MeshBuilder::AddVertex(const Vertex& v)
{
	m_vertexArray.AddElem((Vertex*)&v);
}
/// add a triangle
inline void U2MeshBuilder::AddTriangle(const Triangle& t)
{
	m_triangleArray.AddElem((Triangle*)&t);
}


inline int U2MeshBuilder::GetNumTriangles() const
{
	return (int)m_triangleArray.FilledSize();
}

//-------------------------------------------------------------------------------------------------
inline U2MeshBuilder::Triangle& U2MeshBuilder::GetTriangleAt(int index) const
{
	return *m_triangleArray[index];
}

//-------------------------------------------------------------------------------------------------
inline int U2MeshBuilder::GetNumVertices() const
{
	return (int)m_vertexArray.FilledSize();
}

//-------------------------------------------------------------------------------------------------
inline U2MeshBuilder::Vertex& U2MeshBuilder::GetVertexAt(int index) const
{
	return *m_vertexArray[index];
}

//-------------------------------------------------------------------------------------------------
inline int U2MeshBuilder::GetNumEdges() const
{
	return (int)m_groupEdgeArray.FilledSize();
}

//-------------------------------------------------------------------------------------------------
inline U2MeshBuilder::GroupedEdge& U2MeshBuilder::GetEdgeAt(int index) const
{
	return *m_groupEdgeArray[index];
}


//------------------------------------------------------------------------------
/**
This method copies a triangle with its vertices from the source mesh
to this mesh. Vertices will only be copied if they don't already exist
in this mesh. To accomplish this, an indexMap array must be provided. The
array must contain int entries and its size must be identical to the
number of vertices in the source mesh. The array elements must be
initialized with -1. The copy method will record any copied vertices
into the index map, so that it can find out at a later iteration if
the vertex has already been copied. This method makes an extra cleanup
pass unnecessary, since not redundant vertex data will be generated
during the copy.
*/

inline void U2MeshBuilder::CopyTriangle(const U2MeshBuilder& srcMesh, int triIndex,
								 U2PrimitiveVec<int>& indexMap)
{
	//
}
