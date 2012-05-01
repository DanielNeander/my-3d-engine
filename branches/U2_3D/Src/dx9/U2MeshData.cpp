#include <U2_3D/src/U23DLibPCH.h>
#include "U2MeshData.h"

IMPLEMENT_RTTI(U2MeshData, U2Object);

U2MeshData::U2MeshData(unsigned int uiNumVerts, 
						   D3DXVECTOR3* pVert, 
						   D3DXVECTOR3 *pNormal, 
						   D3DXCOLOR* pColor, 
						   D3DXVECTOR2* pTexCoords,
						   unsigned short numTexCoordSets)
						   :m_uiNumVerts(uiNumVerts),
						   m_uiActiveVerts(uiNumVerts),
						   m_pVerts(pVert),
						   m_pNorms(pNormal),
						   m_pColors(pColor),
						   m_pTexCoords(pTexCoords),
						   m_usNumTextureSets(numTexCoordSets),
						   m_puVertexStride(NULL),						   
						   m_uIBSize(0),
						   m_pIB(0),
						   m_uIndexCnt(0),
						   m_pusArrayLengths(NULL),
						   m_pusIndexArray(NULL),						   
						   m_pVertDecl(0),
						   m_uStreamCnt(0),
						   m_bSoftwareVP(false),
						   m_pVertsAttb(NULL)

{

	SetMeshState(STATIC_MESH);

}

U2MeshData::U2MeshData()
	:m_uiNumVerts(0),
	m_uiActiveVerts(0),
	m_pVerts(0),
	m_pNorms(0),
	m_pColors(0),
	m_pTexCoords(0),
	m_usNumTextureSets(0),
	m_puVertexStride(NULL),
	m_uIBSize(0),
	m_pIB(0),
	m_uIndexCnt(0),
	m_pusArrayLengths(NULL),
	m_pusIndexArray(NULL),	
	m_pVertDecl(0),
	m_uStreamCnt(0),
	m_bSoftwareVP(false),
	m_pVertsAttb(NULL)	
{

	SetMeshState(STATIC_MESH);	
}


U2MeshData::~U2MeshData()
{



	RemoveIB();
	
	U2_FREE(m_puVertexStride);
	m_puVertexStride = NULL;
	
	U2_FREE(m_pusIndexArray);
	m_pusIndexArray = NULL;

	if(m_pVerts)
	{	
		U2_FREE(m_pVerts);
		m_pVerts = NULL;
	}

	if(m_pNorms)
	{	
		U2_FREE(m_pNorms);
		m_pNorms = NULL;
	}

	if(m_pColors)
	{
		U2_FREE(m_pColors);
		m_pColors = NULL;
	}

	if(m_pTexCoords)
	{
		U2_FREE(m_pTexCoords);
		m_pTexCoords = NULL;
	}	

	SAFE_RELEASE(m_pVertDecl);

	if(m_pVertsAttb)
	{
		U2_DELETE m_pVertsAttb;
		m_pVertsAttb = NULL;
	}
}

void U2MeshData::CreateVertices()
{
	U2ASSERT(m_uiNumVerts > 0);
	if(!m_pVerts)
		m_pVerts = U2_ALLOC(D3DXVECTOR3, m_uiNumVerts);

	memset(m_pVerts, 0, sizeof(m_pVerts[0]) * m_uiNumVerts);
}



void U2MeshData::CreateNormals(bool bInit)
{
	U2ASSERT(m_uiNumVerts > 0);
	if(!m_pNorms)
		m_pNorms = U2_ALLOC(D3DXVECTOR3, m_uiNumVerts);

	if(bInit)
		memset(m_pNorms, 0, sizeof(m_pNorms[0]) * m_uiNumVerts);
}



void U2MeshData::CreateColors()
{
	U2ASSERT(m_uiNumVerts > 0);
	if(!m_pColors)
		m_pColors = U2_ALLOC(D3DXCOLOR, m_uiNumVerts);

	for(unsigned short idx=0; idx < m_uiNumVerts; ++idx)
		m_pColors[idx] = D3DXCOLOR(0, 0, 0, 1);

}


void U2MeshData::CreateTexCoords(unsigned short numTexCoordSets) 
{
	U2ASSERT(m_uiNumVerts > 0);
	if(m_pTexCoords && numTexCoordSets != m_usNumTextureSets)
	{
		U2_FREE(m_pTexCoords);
		m_pTexCoords = NULL;
		m_usNumTextureSets = numTexCoordSets;
	}

	m_usNumTextureSets = numTexCoordSets;

	unsigned short numTexcoords = numTexCoordSets * m_uiNumVerts;
	if(!m_pTexCoords)
	{
		m_pTexCoords = U2_ALLOC(D3DXVECTOR2, numTexcoords);
	}

	memset(m_pTexCoords, 0, sizeof(m_pTexCoords[0]) * numTexcoords);

}



void U2MeshData::AppendTexCoords(D3DXVECTOR2* pAddedTexCoords)
{
	U2ASSERT(pAddedTexCoords);
	
	unsigned int idx;

	//1: 새로 만드는 경우 
	//2: 기존의 텍스쳐좌표집합에 추가하는 경우 

	if(!m_pTexCoords)
	{
		m_pTexCoords = U2_ALLOC(D3DXVECTOR2, m_uiNumVerts);
		U2ASSERT(m_pTexCoords);		
		m_uiNumVerts = 1;
		for(idx = 0; idx < m_uiNumVerts; ++idx)
			m_pTexCoords[idx] = pAddedTexCoords[idx];
	}
	else 
	{
		unsigned short existTexCoords = m_uiNumVerts * m_usNumTextureSets;
		D3DXVECTOR2* pNewTexCoords = 
			U2_ALLOC(D3DXVECTOR2 ,existTexCoords + m_uiNumVerts);
		for(idx = 0; idx < existTexCoords; idx++)
			pNewTexCoords[idx] = m_pTexCoords[idx];
		U2_FREE(m_pTexCoords);
		m_pTexCoords = 0;
		m_pTexCoords = pNewTexCoords;

		for(idx = 0; idx < m_uiNumVerts; ++idx)
			m_pTexCoords[existTexCoords + idx] = pNewTexCoords[idx];

		// Update a Number of Texcoord Set
		m_usNumTextureSets++;
	}
}


void U2MeshData::SetStreamCount(unsigned int uiStreamCount)
{
	if (uiStreamCount == m_uStreamCnt)
		return;

	U2ASSERT(uiStreamCount > 0);
	
	//U2VBBlock** ppVBBlock = U2_ALLOC(U2VBBlock*, uiStreamCount);
	//unsigned int* puiVertexStride = U2_ALLOC(unsigned int, uiStreamCount);
	//unsigned int uiByteSize;

	//if (uiStreamCount < m_uStreamCnt)
	//{		
	//	uiByteSize = uiStreamCount * sizeof(*ppVBBlock);
	//	memcpy_s(ppVBBlock, uiByteSize, m_ppVBB, uiByteSize);

	//	uiByteSize = uiStreamCount * sizeof(*puiVertexStride);
	//	memcpy_s(puiVertexStride, sizeof(puiVertexStride),  m_puVertexStride, uiByteSize);
	//	if(m_pVBGroup)
	//	{
	//		for(uint32 i= uiStreamCount; i < m_uStreamCnt; ++i)
	//		{
	//			m_pVBGroup->ReleaseVBBlock(this, i);
	//		}
	//	}
	//	
	//}
	//else
	//{		
	//	uiByteSize = m_uStreamCnt * sizeof(*ppVBBlock);
	//	memcpy_s(ppVBBlock, uiByteSize, m_ppVBB, uiByteSize);

	//	uiByteSize = m_uStreamCnt * sizeof(*puiVertexStride);
	//	memcpy_s(puiVertexStride, sizeof(puiVertexStride), m_puVertexStride, uiByteSize);
	//	
	//	memset(ppVBBlock + m_uStreamCnt, 0, (uiStreamCount - m_uStreamCnt) 
	//		* sizeof(*ppVBBlock));
	//	memset(puiVertexStride + m_uStreamCnt, 0, 
	//		(uiStreamCount - m_uStreamCnt) * sizeof(*puiVertexStride)); 
	//}
	//
	//U2_DELETE [] m_ppVBB;
	//U2_FREE(m_puVertexStride);	
	//m_ppVBB = ppVBBlock;
	//m_puVertexStride = puiVertexStride;
	m_uStreamCnt = uiStreamCount;
}
//---------------------------------------------------------------------------
void U2MeshData::SetFVF(unsigned int uiFVF)
{
	m_uFVF = uiFVF;
	SAFE_RELEASE(m_pVertDecl);			
}
//---------------------------------------------------------------------------
void U2MeshData::SetVertexDeclaration(
	LPDIRECT3DVERTEXDECLARATION9 pVertDecl)
{
	if (pVertDecl != m_pVertDecl)
	{
		SAFE_RELEASE(m_pVertDecl);			

		m_pVertDecl = pVertDecl;

		if(m_pVertDecl)
			m_pVertDecl->AddRef();
	}
	m_uFVF = 0;
}


void U2MeshData::SetMeshState(MeshState eState)
{
	m_usDirtyFlags = eState;
}

U2MeshData::MeshState U2MeshData::GetMeshState() const
{
	return (U2MeshData::MeshState)m_usDirtyFlags;
}


void U2MeshData::RemoveIB()
{
	SAFE_RELEASE(m_pIB);
	m_uIBSize = 0;
}


