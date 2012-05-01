//-------------------------------------------------------------------------------------------------
inline U2SkinPartition::Partition::Partition()
:m_pSrcMeshBlder(0),
m_iMaxBonePaletteSize(0),
m_iGroupId(0),
m_bonePalette(32, 32),
m_triangleArray(2048, 2048)
{
	memset(m_bBoneMask, 0, sizeof(m_bBoneMask));
}

//-------------------------------------------------------------------------------------------------
inline U2SkinPartition::Partition::Partition(U2MeshBuilder* pMeshBlder, int maxBones, int groupId)
:m_pSrcMeshBlder(pMeshBlder),
m_iMaxBonePaletteSize(maxBones),
m_iGroupId(groupId),
m_bonePalette(32, 32),
m_triangleArray(2048, 2048)
{
	U2ASSERT(maxBones <= MAX_SKELETON_BONES);
	memset(m_bBoneMask, 0, sizeof(m_bBoneMask));
}

//-------------------------------------------------------------------------------------------------
inline int U2SkinPartition::Partition::GetGroupId() const 
{
	return m_iGroupId;
}

//------------------------------------------------------------------------------
/**
Add an unique index to an int array.
*/
inline void U2SkinPartition::Partition::AddUniqueBoneIdx(U2PrimitiveVec<int>& intArry, int idx)
{
	uint32 num = intArry.Size();
	uint32 i;
	for(i=0; i < num; ++i)
	{
		if(intArry[i] == idx)
		{
			return;
		}
	}

	intArry.AddElem(idx);
}

//------------------------------------------------------------------------------
/**
Fills the provided int array with the unique joint indices in the
triangle.
*/
inline void U2SkinPartition::Partition::GetTriangleBones(const U2MeshBuilder::Triangle& tri, U2PrimitiveVec<int>& triBones)
{
	U2ASSERT(m_pSrcMeshBlder);

	int idx[3];
	tri.GetVertexIndices(idx[0], idx[1], idx[2]);
	int i;
	for(i=0; i < 3; ++i)
	{
		U2MeshBuilder::Vertex& v = m_pSrcMeshBlder->GetVertexAt(idx[i]);
		const D3DXVECTOR4& weights = v.GetWeights();
		const D3DXVECTOR4& indices = v.GetBoneIndices();		
		
		if(weights.x > 0.0f)
		{
			AddUniqueBoneIdx(triBones, int(indices.x));
		}
		if(weights.y > 0.0f)
		{
			AddUniqueBoneIdx(triBones, int(indices.y));
		}
		if(weights.z > 0.0f)
		{
			AddUniqueBoneIdx(triBones, int(indices.z));
		}
		if(weights.w > 0.0f)
		{
			AddUniqueBoneIdx(triBones, int(indices.w));
		}

	}
}

//------------------------------------------------------------------------------
/**
Fills the provided diffSet int array with all joint indices in the triJoints
array which are not currently in the joint palette.
*/
inline void U2SkinPartition::Partition::GetBoneIdxDiffSet(const U2PrimitiveVec<int> &triBones, 
														  U2PrimitiveVec<int> &diffSet)
{
	uint32 num = triBones.Size();
	uint32 i;
	for(i=0; i < num; ++i)
	{
		if(!m_bBoneMask[triBones[i]])
		{
			diffSet.AddElem(triBones[i]);
		}
	}
}

//-------------------------------------------------------------------------------------------------
inline bool U2SkinPartition::Partition::CheckAddTriangle(int triIdx)
{
	U2ASSERT(m_pSrcMeshBlder);
	const U2MeshBuilder::Triangle& tri = m_pSrcMeshBlder->GetTriangleAt(triIdx);

	// get the unique joints in the triangle
	U2PrimitiveVec<int> triBones;
	GetTriangleBones(tri, triBones);

	// get the difference set between the triangle joint indices 
	// and the indices in the joint palette
	U2PrimitiveVec<int> diffSet;
	GetBoneIdxDiffSet(triBones, diffSet);
	

	if(m_bonePalette.FilledSize() + diffSet.FilledSize() <= m_iMaxBonePaletteSize)
	{
		FILE_LOG(logDEBUG) << _T("CheckAddTriangle : ") << triIdx;

		m_triangleArray.AddElem(triIdx);
		int i;
		for(i=0; i < diffSet.FilledSize(); ++i)
		{
			m_bonePalette.AddElem(diffSet[i]);
			FILE_LOG(logDEBUG) << _T("diffset : ") << diffSet[i];
			m_bBoneMask[diffSet[i]] = true;
		}
		return true;
	}

	// fallthrough: could not add the triangle to this partition, because there
	// the partition is or would be full
	return false;
}

//-------------------------------------------------------------------------------------------------
inline const U2PrimitiveVec<int>& U2SkinPartition::Partition::GetBonePalette() const
{
	return m_bonePalette;
}

//-------------------------------------------------------------------------------------------------
inline const U2PrimitiveVec<int>& U2SkinPartition::Partition::GetTriangleIndices() const
{
	return m_triangleArray;
}

//------------------------------------------------------------------------------
/**
Converts a global joint palette index into a partition-local joint index.
If the global joint index cannot be found in the local index, a 0 index
will be returned.
*/
inline int U2SkinPartition::Partition::GlobalToLocalBoneIdx(int globalBoneIdx) const
{
	uint32 num = m_bonePalette.FilledSize();
	uint32 i;
	for(i=0; i < num; ++i)
	{
		if(globalBoneIdx == m_bonePalette[i])
		{
			return i;
		}
	}
	// fallthrough: globalJointIndex not in partition
	return 0;
}

//-------------------------------------------------------------------------------------------------
inline int U2SkinPartition::GetNumPartitions() const
{
	return (int)m_partitions.FilledSize();
}

//-------------------------------------------------------------------------------------------------
inline const U2PrimitiveVec<int>& U2SkinPartition::GetBonePalette(int partitionIdx) const
{
	return m_partitions[partitionIdx]->GetBonePalette();
}

//-------------------------------------------------------------------------------------------------
inline const U2PrimitiveVec<int>& U2SkinPartition::GetTriangleIndices(int partitionIdx) const 
{
	return m_partitions[partitionIdx]->GetTriangleIndices();
}

//-------------------------------------------------------------------------------------------------
inline const U2PrimitiveVec<int>& U2SkinPartition::GetGroupMappings() const 
{
	return m_groupMappings;
}

