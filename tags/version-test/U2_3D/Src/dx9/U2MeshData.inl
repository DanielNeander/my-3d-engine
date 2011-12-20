inline 
unsigned short U2MeshData::GetVertexCount() const
{
	return m_uiNumVerts;
}


inline 
D3DXVECTOR3* U2MeshData::GetVertices()
{
	return m_pVerts;
}


inline 
const D3DXVECTOR3* U2MeshData::GetVertices() const
{
	return m_pVerts;
}

inline 
D3DXVECTOR3* U2MeshData::GetNormals()
{
	return m_pNorms;
}

inline 
const D3DXVECTOR3* U2MeshData::GetNormals() const
{
	return m_pNorms;
}

inline 
D3DXCOLOR* U2MeshData::GetColors()
{
	return m_pColors;
}


inline 
const D3DXCOLOR* U2MeshData::GetColors() const
{
	return m_pColors;
}


inline
D3DXVECTOR2* U2MeshData::GetTexCoordSet(unsigned short texCoordIdx)
{
	if(!m_pTexCoords)
		return NULL;
	return &m_pTexCoords[m_uiNumVerts * texCoordIdx];
}


inline
const D3DXVECTOR2* U2MeshData::GetTexCoordSet(unsigned short texCoordIdx) const
{
	if(!m_pTexCoords)
		return NULL;
	return &m_pTexCoords[m_uiNumVerts * texCoordIdx];
}


inline
D3DXVECTOR2* U2MeshData::GetTexCoords()
{
	return m_pTexCoords;
}


inline
const D3DXVECTOR2* U2MeshData::GetTexCoords() const
{
	return m_pTexCoords;

}

inline 
unsigned short U2MeshData::GetTexCoordSetCnt() const
{
	return m_usNumTextureSets;
}

inline void U2MeshData::SetIB(LPDIRECT3DINDEXBUFFER9 pkIB, 
										unsigned int uiIndexCount, unsigned int uiIBSize)
{
	m_pIB = pkIB;
	m_uIndexCnt = uiIndexCount;
	m_uIBSize = uiIBSize;
}


inline void U2MeshData::SetBaseVertexIndex(
	unsigned int uiBaseVertexIndex)
{
	m_uBaseVertIdx = uiBaseVertexIndex;
}

//---------------------------------------------------------------------------
inline void U2MeshData::SetVertexStride(unsigned int uiStream, 
												  unsigned int uiStride)
{
	if (uiStream < m_uStreamCnt)
		m_puVertexStride[uiStream] = uiStride;
}

inline void U2MeshData::SetType(D3DPRIMITIVETYPE ePrimType)
{
	m_eType = ePrimType;
}


inline void U2MeshData::SetIndices(unsigned int uiTriCount, 
											 unsigned int uiMaxTriCount, unsigned short* pusIndexArray, 
											 unsigned short* pusArrayLengths, unsigned int uiNumArrays)
{
	U2ASSERT(uiMaxTriCount >= uiTriCount);
	m_uTriCnt = uiTriCount;
	m_uMaxTriCnt = uiMaxTriCount;
	m_pusIndexArray = pusIndexArray;
	m_pusArrayLengths = pusArrayLengths;
	m_uiNumArrays = uiNumArrays;
}


inline void U2MeshData::SetSoftwareVP(bool bSoftwareVP)
{
	m_bSoftwareVP = bSoftwareVP;
}


inline bool U2MeshData::GetSoftwareVP() const
{
	return m_bSoftwareVP;
}

inline unsigned int U2MeshData::GetFVF() const
{
	return m_uFVF;
}


inline LPDIRECT3DVERTEXDECLARATION9 U2MeshData::GetVertexDeclaration() const
{
	return m_pVertDecl;
}


inline unsigned int U2MeshData::GetStreamCount() const
{
	return m_uStreamCnt;
}

inline unsigned int U2MeshData::GetVertexStride(unsigned int uiIndex) const
{
	if(uiIndex < m_uStreamCnt)
		return m_puVertexStride[uiIndex];
	else 
		return 0;
}


inline void U2MeshData::SetVertexCount(unsigned int uiVertCount, 
									   unsigned int uiMaxVertCount)
{
	U2ASSERT(uiMaxVertCount >= uiVertCount);
	m_uiActiveVerts = uiVertCount;
	m_uiNumVerts = uiMaxVertCount;
}



inline void U2MeshData::SetActiveVertexCount(unsigned short /*usActive*/)
{
	// stub for derived classes
}
//---------------------------------------------------------------------------
inline unsigned short U2MeshData::GetActiveVertexCount() const
{
	return m_uiActiveVerts;
}

inline unsigned int U2MeshData::GetNumArrays() const
{
	return m_uiNumArrays;
}

inline const unsigned short* U2MeshData::GetArrayLengths() const
{
	return m_pusArrayLengths;
}
//---------------------------------------------------------------------------
inline const unsigned short* U2MeshData::GetIndexArray() const
{
	return m_pusIndexArray;
}

inline D3DPRIMITIVETYPE U2MeshData::GetPrimitiveType() const
{
	return m_eType;
}

inline void U2MeshData::SetIndexArray(unsigned short* pIdxArray)
{
	m_pusIndexArray = pIdxArray;
}

inline void U2MeshData::SetArrayLengths(unsigned short* pIdxLengths)
{
	m_pusArrayLengths = pIdxLengths;
}

inline unsigned int U2MeshData::GetBaseVertexIndex() const
{
	return m_uBaseVertIdx;
}


inline unsigned int U2MeshData::GetIBSize() const
{
	return m_uIBSize;
}



inline  unsigned int U2MeshData::GetTriCount() const
{
	return m_uTriCnt;
}

inline LPDIRECT3DINDEXBUFFER9 U2MeshData::GetIB() const
{
	return m_pIB;
}


inline void U2MeshData::SetVertexAttbs(U2VertexAttributes* pAttb)
{
	m_pVertsAttb = pAttb;
}


inline U2VertexAttributes* U2MeshData::GetVertexAttbs() const
{
	return m_pVertsAttb;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2MeshData::GetIndexCount() const
{
	return m_uIndexCnt;
}

