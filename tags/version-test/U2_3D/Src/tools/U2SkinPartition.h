/**************************************************************************************************
module	:	U2SkinPartition
Author	:	Yun sangyong
Desc	:	// m_iUsageFlags 정의 필요..
*************************************************************************************************/
#pragma once
#ifndef U2_SKINPARTITION_H
#define U2_SKINPARTITION_H

class U2_3D U2SkinPartition : public U2Object
{
public:
	U2SkinPartition();
	virtual ~U2SkinPartition();

	bool Execute(U2MeshBuilder& srcMeshBlder, U2MeshBuilder& dstMeshBlder, int maxBonePaletteSize);

	int GetNumPartitions() const;

	const U2PrimitiveVec<int>& GetBonePalette(int partitionIdx) const;

	const U2PrimitiveVec<int>& GetTriangleIndices(int partitionIdx) const;

	const U2PrimitiveVec<int>& GetGroupMappings() const;

private:

	void BuildResultMesh(U2MeshBuilder& srcMeshBlder, U2MeshBuilder& dstMeshBlder);

	class Partition : public U2MemObj
	{
	public:
		Partition();

		Partition(U2MeshBuilder* pMeshBlder, int maxBones, int groupId);

		bool CheckAddTriangle(int triIdx);

		const U2PrimitiveVec<int>& GetBonePalette() const;

		const U2PrimitiveVec<int>& GetTriangleIndices() const;

		int GetGroupId() const;

		int GlobalToLocalBoneIdx(int globalBoneIdx) const;

	private:
		void AddUniqueBoneIdx(U2PrimitiveVec<int>& intArray, int idx);

		void GetTriangleBones(const U2MeshBuilder::Triangle& tri, U2PrimitiveVec<int>& triBones);

		void GetBoneIdxDiffSet(const U2PrimitiveVec<int>& triBones, U2PrimitiveVec<int>& diffSet);

		enum 
		{
			MAX_SKELETON_BONES = 1024,
		};

		U2MeshBuilder* m_pSrcMeshBlder;
		int m_iMaxBonePaletteSize;
		int m_iGroupId;
		bool m_bBoneMask[MAX_SKELETON_BONES];
		U2PrimitiveVec<int> m_bonePalette;
		U2PrimitiveVec<int> m_triangleArray;
	};

	U2PrimitiveVec<Partition*> m_partitions;
	U2PrimitiveVec<int> m_groupMappings;

};

#include "U2SkinPartition.inl"


#endif