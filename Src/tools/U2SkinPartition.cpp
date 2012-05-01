#include <U2_3D/Src/U23DLibPCH.h>
#include "U2SkinPartition.h"


//-------------------------------------------------------------------------------------------------
U2SkinPartition::U2SkinPartition() 
:m_partitions(64, 64)
{

}

//-------------------------------------------------------------------------------------------------
U2SkinPartition::~U2SkinPartition()
{

}

//------------------------------------------------------------------------------
/**
Does the actual partitioning. For each triangle:

- add it to an existing partition object if it has enough space in
its joint palette
- otherwise add it to a new empty partition
- update the triangle group id in the mesh to account for the
additional partitions
- a group id mapping array will be created which maps new
group ids to old group ids

@param  srcMesh                 the source mesh to partition
@param  dstMesh                 the destination mesh
@param  maxJointPaletteSize     max number of joints in a joint palette
*/
bool U2SkinPartition::Execute(U2MeshBuilder& srcMeshBlder, U2MeshBuilder& dstMeshBlder, 
							  int maxBonePaletteSize)
{
	for(uint32 i=0; i < m_partitions.FilledSize(); ++i)
	{
		U2_DELETE m_partitions[i];
	}
	
	m_partitions.RemoveAll();

	int triIdx;
	int numTris = srcMeshBlder.GetNumTriangles();
	for(triIdx=0; triIdx < numTris; ++triIdx)
	{
		const U2MeshBuilder::Triangle& tri = srcMeshBlder.GetTriangleAt(triIdx);

		// try to add the triangle to an existing partition
		bool bTriAdded = false;
		uint32 numPartitions = m_partitions.FilledSize();
		uint32 partitionIdx;
		for(partitionIdx=0; partitionIdx < numPartitions; ++partitionIdx)
		{
			if(m_partitions[partitionIdx]->GetGroupId() == tri.GetGroupId())
			{
				if(m_partitions[partitionIdx]->CheckAddTriangle(triIdx))
				{
					bTriAdded = true;
					break;
				}
			}
		}
		if(!bTriAdded)
		{
			// triangle didn't fit into any existing partition, create a new partition
			Partition *pNewPartition = U2_NEW Partition(&srcMeshBlder, maxBonePaletteSize,tri.GetGroupId());
			bTriAdded = pNewPartition->CheckAddTriangle(triIdx);
			U2ASSERT(bTriAdded);
			m_partitions.AddElem(pNewPartition);
		}
	}

	// update the triangle group ids
	BuildResultMesh(srcMeshBlder, dstMeshBlder);

	return true;
}

//------------------------------------------------------------------------------
/**
Private helper method which builds the actual destination mesh after
partitioning has happened.
*/
void U2SkinPartition::BuildResultMesh(U2MeshBuilder &srcMeshBlder, U2MeshBuilder &dstMeshBlder)
{
//	FILE_LOG(logDEBUG) << _T("Start BuildResultMesh ...");
	uint32 i;
	for(i=0; i < dstMeshBlder.m_vertexArray.FilledSize(); ++i)
	{
		U2MeshBuilder::Vertex* pVert = dstMeshBlder.m_vertexArray[i];
		U2_DELETE pVert;
	}
	dstMeshBlder.m_vertexArray.RemoveAll();

	for(i=0; i < dstMeshBlder.m_triangleArray.FilledSize(); ++i)
	{
		U2MeshBuilder::Triangle* pTri = dstMeshBlder.m_triangleArray[i];
		U2_DELETE pTri;
	}
	dstMeshBlder.m_triangleArray.RemoveAll();

	int partitionIdx;
	int numPartitions = GetNumPartitions();
	for(partitionIdx=0; partitionIdx < numPartitions; ++partitionIdx)
	{
		const Partition& partition = *m_partitions[partitionIdx];

		// record the original group id in the groupMapArray
		m_groupMappings.AddElem(partition.GetGroupId());

		//for(int i=0; i < partition.GetBonePalette().FilledSize(); ++i)
		//{
		//	FILE_LOG(logDEBUG) << _T("BonePalette : ") << partition.GetBonePalette()[i];
		//}

		// transfer the partition vertices and triangles
		uint32 i;
		const U2PrimitiveVec<int>& triArray = partition.GetTriangleIndices();
		for(i=0; i < triArray.FilledSize(); ++i)
		{
			U2MeshBuilder::Triangle& tri = srcMeshBlder.GetTriangleAt(triArray[i]);

			// transfer the 3 vertices of the triangle
			int origVertexIdx[3];
			tri.GetVertexIndices(origVertexIdx[0], origVertexIdx[1], origVertexIdx[2]);
			int triPoint;
			int newVertexIdx[3];
			for(triPoint=0; triPoint < 3; ++triPoint)
			{
				const U2MeshBuilder::Vertex& v = srcMeshBlder.GetVertexAt(origVertexIdx[triPoint]);

				// convert the global joint indices to partition-local joint indices
				const D3DXVECTOR4& globalBoneIndices = v.GetBoneIndices();
				D3DXVECTOR4 localBoneIndices;
				localBoneIndices.x = float(partition.GlobalToLocalBoneIdx(int(globalBoneIndices.x)));
				localBoneIndices.y = float(partition.GlobalToLocalBoneIdx(int(globalBoneIndices.y)));
				localBoneIndices.z = float(partition.GlobalToLocalBoneIdx(int(globalBoneIndices.z)));
				localBoneIndices.w = float(partition.GlobalToLocalBoneIdx(int(globalBoneIndices.w)));				

				// add the vertex to the destination mesh and correct the joint indices
				newVertexIdx[triPoint] = dstMeshBlder.GetNumVertices();				

				U2MeshBuilder::Vertex *pNewVert = U2_NEW U2MeshBuilder::Vertex;
				memcpy_s(pNewVert, sizeof(v), &v, sizeof(v));

				dstMeshBlder.AddVertex(*pNewVert);
				dstMeshBlder.GetVertexAt(newVertexIdx[triPoint]).SetBoneIndices(localBoneIndices);
			}

			// update the triangle and add to the dest mesh's triangle array
			tri.SetGroupId(partitionIdx);
			tri.SetVertexIndices(newVertexIdx[0], newVertexIdx[1], newVertexIdx[2]);			



			U2MeshBuilder::Triangle* pNewTri = U2_NEW U2MeshBuilder::Triangle;

			memcpy_s(pNewTri, sizeof(U2MeshBuilder::Triangle), &tri, sizeof(U2MeshBuilder::Triangle));
			
			dstMeshBlder.AddTriangle(*pNewTri);
		
		}		
	}

	//for(uint32 i=0; i < dstMeshBlder.GetNumTriangles(); ++i)
	//{
	//	int i0, i1, i2;
	//	dstMeshBlder.GetTriangleAt(i).GetVertexIndices(i0, i1, i2);
	//	FILE_LOG(logDEBUG) << _T("Dst Triindex : ") << i0 << i1 << i2;
	//}

	// finally, do a cleanup on the dest mesh to remove any redundant vertices
	dstMeshBlder.Cleanup(0);

//#ifdef DEBUG
//	for(uint32 vertexIndex=0;vertexIndex < dstMeshBlder.GetNumVertices(); ++vertexIndex)
//	{
//		FILE_LOG(logDEBUG) << _T("Vertex ") << vertexIndex << _T( ": X , ") <<
//			dstMeshBlder.GetVertexAt(vertexIndex).m_vert.x << _T( ": Y , ") << 
//			dstMeshBlder.GetVertexAt(vertexIndex).m_vert.y << _T( ": Z , ") << 
//			dstMeshBlder.GetVertexAt(vertexIndex).m_vert.z;
//	}
//#endif


//	FILE_LOG(logDEBUG) << _T("End BuildResultMesh ...");
}