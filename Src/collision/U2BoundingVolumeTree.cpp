#include <U2_3D/Src/U23DLibPCH.h>
#include "U2BoundingVolumeTree.h"

//-------------------------------------------------------------------------------------------------
U2BoundingVolumeTree::CreatorModel
U2BoundingVolumeTree::ms_afnCreateModelBound[U2BoundingVolume::BV_QUANTITY];

U2BoundingVolumeTree::CreatorWorld
U2BoundingVolumeTree::ms_afnCreateWorldBound[U2BoundingVolume::BV_QUANTITY];

//-------------------------------------------------------------------------------------------------
U2BoundingVolumeTree::U2BoundingVolumeTree(const U2TriList *pMesh)
{
	m_pMesh = pMesh;
	m_pLChild = 0;
	m_pRChild = 0;
	m_usTrisCnt = 0;
	m_pusTriangle = 0;
}

//-------------------------------------------------------------------------------------------------
U2BoundingVolumeTree::U2BoundingVolumeTree(int eBVType, const U2TriList *pMesh, unsigned short usMaxTrisPerLeaf, bool bStoreInteriorTris): m_pMesh(pMesh)
{
	// Centroids of triangles are used for splitting a mesh.  The centroids
	// are projected onto a splitting axis and sorted.  The split is based
	// on the median of the projections.
	uint16 usTriCnt = m_pMesh->GetActiveTriangleCount();
	const uint16* pusIndices = m_pMesh->GetMeshData()->GetIndexArray();
	D3DXVECTOR3* pCentroid = U2_ALLOC(D3DXVECTOR3,usTriCnt);
	const float fOneDivThird = 1.0f / 3.0f;
	uint16 usT, i;
	for(usT = 0, i = 0; usT < usTriCnt; ++usT)
	{
		uint16 i0 =pusIndices[i++];
		uint16 i1 =pusIndices[i++];
		uint16 i2 =pusIndices[i++];
		
		pCentroid[usT] = fOneDivThird * (pMesh->GetVertices()[i0] +
			pMesh->GetVertices()[i1] +pMesh->GetVertices()[i2]);
	}

	// Initialize binary-tree arrays for storing triangle indices.  These
	// are used to store the indices when the mesh is split.
	uint16* pusISplit = U2_ALLOC(uint16, usTriCnt);
	uint16* pusOSplit = U2_ALLOC(uint16, usTriCnt);
	for(usT = 0; usT <usTriCnt; +usT)
	{
		pusISplit[usT]=usT;
	}

	BuildTree(eBVType, usMaxTrisPerLeaf, bStoreInteriorTris, pCentroid, 0,
		usTriCnt -1, pusISplit, pusOSplit);

	U2_FREE(pCentroid);
	U2_FREE(pusISplit);
	U2_FREE(pusOSplit);
}

//-------------------------------------------------------------------------------------------------
U2BoundingVolumeTree::~U2BoundingVolumeTree()
{
	U2_FREE(m_pusTriangle);
	m_pusTriangle =0;
	U2_DELETE m_pLChild;
	m_pLChild = 0;
	U2_DELETE m_pRChild;
	m_pRChild = 0;
}

//-------------------------------------------------------------------------------------------------
void U2BoundingVolumeTree::BuildTree(int eBVType, unsigned short usMaxTrisPerLeaf, bool bStoreInteriorTris, const D3DXVECTOR3 *pCentroid, uint16 i0, uint16 i1, uint16 *pusOSplit, uint16 *pusISplit)
{
	U2ASSERT(i0 <= i1);

	U2Line3D line;
	m_spModelBound = ms_afnCreateModelBound[eBVType](m_pMesh,i0, i1, pusISplit,line);
	m_spWorldBound = ms_afnCreateWorldBound[eBVType]();

	if(i1 - i0 < usMaxTrisPerLeaf)
	{
		// leaf node 
		m_usTrisCnt = i1 - i0 + 1;
		m_pusTriangle = U2_ALLOC(uint16, m_usTrisCnt);
		size_t uSize = m_usTrisCnt * sizeof(uint16);
		memcpy_s(m_pusTriangle, uSize, &pusISplit[i0], uSize);

		m_pLChild = 0;
		m_pRChild = 0;
	}
	else 
	{
		if(bStoreInteriorTris)
		{
			m_usTrisCnt = i1 - i0 + 1;
			m_pusTriangle = U2_ALLOC(uint16, m_usTrisCnt);
			size_t uSize = m_usTrisCnt * sizeof(uint16);
			memcpy_s(m_pusTriangle, uSize, &pusISplit[i0], uSize);			
		}
		else 
		{
			m_usTrisCnt = 0;
			m_pusTriangle = 0;
		}

		uint16 j0, j1;
		SplitTriangles(pCentroid, i0, i1, pusISplit, j0, j1, pusOSplit, line);
		m_pLChild = U2_NEW U2BoundingVolumeTree(m_pMesh);
		m_pLChild->BuildTree(eBVType,usMaxTrisPerLeaf, bStoreInteriorTris, pCentroid, i0, j0, pusOSplit, pusISplit);

		m_pRChild = U2_NEW U2BoundingVolumeTree(m_pMesh);
		m_pRChild->BuildTree(eBVType,usMaxTrisPerLeaf, bStoreInteriorTris, pCentroid, i1, j1, pusOSplit, pusISplit);
	}
}

//-------------------------------------------------------------------------------------------------
int U2BoundingVolumeTree::Compare(const void *pvElement0, const void *pvElement1)
{
	const ProjectionInfo* pInfo0 = (const ProjectionInfo*)pvElement0;
	const ProjectionInfo* pInfo1 = (const ProjectionInfo*)pvElement1;

	if(pInfo0->m_fProjection < pInfo1->m_fProjection)
	{
		return -1;
	}

	if(pInfo0->m_fProjection > pInfo1->m_fProjection)
	{
		return +1;
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------
void U2BoundingVolumeTree::SplitTriangles(const D3DXVECTOR3 *pCentroid, uint16 i0, uint16 i1, uint16 *pusISplit, uint16 &rj0, uint16 &rj1, uint16 *pusOSplit, const U2Line3D &line)
{
	// project onto specified line
	uint16 usCnt = i1 - i0 +1;
	ProjectionInfo* pInfos = U2_NEW ProjectionInfo[usCnt];
	uint16 i, j;
	for(i = i0, j = 0; i <= i1; ++i, ++j)
	{
		uint16 usTriangle = pusISplit[i];
		D3DXVECTOR3 diff = pCentroid[usTriangle] - line.m_vOrigin;
		pInfos[j].m_usTriangle = usTriangle;
		pInfos[j].m_fProjection = D3DXVec3Dot(&line.m_vDir, &diff);
	}

	// find median of projections by sorting
	std::qsort(pInfos, usCnt, sizeof(ProjectionInfo), Compare);
	uint16 usMedian = (usCnt - 1) * 0.5f;

	for(j=0, rj0 = i0 - 1; j <= usMedian; ++j)
	{
		pusOSplit[++rj0] = pInfos[j].m_usTriangle;
	}
	for(rj1= i1+1; j < usCnt; ++j)
	{
		pusOSplit[--rj1]= pInfos[j].m_usTriangle;
	}
	
	U2_DELETE [] pInfos;
}

//-------------------------------------------------------------------------------------------------
void U2BoundingVolumeTree::UpdateWorldBound()
{
	m_spModelBound->TransformBy(m_pMesh->m_tmWorld, m_spWorldBound);
}