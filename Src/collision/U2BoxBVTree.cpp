#include <U2_3D/Src/U23DLibPCH.h>
#include "U2BoxBVTree.h"

IMPLEMENT_INITIALIZE(U2BoxBVTree);

//-------------------------------------------------------------------------------------------------
void U2BoxBVTree::Initialize()
{
	ms_afnCreateModelBound[U2BoundingVolume::BV_BOX] = &U2BoxBVTree::CreateModelBound;

	ms_afnCreateWorldBound[U2BoundingVolume::BV_BOX] = &U2BoxBVTree::CreateWorldBound;
}

//-------------------------------------------------------------------------------------------------
U2BoxBVTree::U2BoxBVTree(const U2TriList *pMesh, uint16 usMaxTrisPerLeaf, bool bStoreInteriorTris) : U2BoundingVolumeTree(U2BoundingVolume::BV_BOX, pMesh, usMaxTrisPerLeaf, bStoreInteriorTris)
{
	
}

//-------------------------------------------------------------------------------------------------
U2BoundingVolume* U2BoxBVTree::CreateModelBound(const U2TriList *pMesh, uint16 i0, uint16 i1, uint16 *pusISplit, U2Line3D &line)
{
	uint16 usVertCnt = pMesh->GetActiveVertexCount();
	const uint16* pusIndices = pMesh->GetMeshData()->GetIndexArray();
	bool* pbValids = U2_ALLOC(bool, usVertCnt);
	memset(pbValids, 0, usVertCnt * sizeof(bool));
	uint16 i;
	for(i = i0; i <= i1; ++i)
	{
		uint16 j = 3 * pusISplit[i];
		pbValids[pusIndices[j++]] = true;
		pbValids[pusIndices[j++]] = true;
		pbValids[pusIndices[j++]] = true;
	}
	
	// Create a contiguous set of vertices in the submesh.
	std::vector<D3DXVECTOR3> meshVertices;
	for(i = 0; i < usVertCnt; ++i)
	{
		if(pbValids[i])
		{
			meshVertices.push_back((pMesh->GetVertices()[i]));
		}
	}

	U2_FREE(pbValids);

}