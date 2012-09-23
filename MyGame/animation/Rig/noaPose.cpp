#include "DXUT.h"
#include "noaPose.h"
#include "noaSkeleton.h"
#include "noaSkeletonUtils.h"





noaPose::noaPose( PoseSpace space, const noaSkeleton* skeleton, const hkArray<noQTransform>& pose )
	:m_skeleton(skeleton)
{
	const int numBones = m_skeleton->m_numBones;

	m_modelPose.setSize(numBones);
	m_localPose.setSize(numBones);
	m_boneFlags.setSize(numBones);

	if (space == LOCAL_SPACE)
	{
		SetPoseLocalSpace(pose);
	}
	else 
	{
		SetPoseModelSpace(pose);
	}

	const int sizeRoundedUp = HK_NEXT_MULTIPLE_OF( 4, m_skeleton->m_numFloatSlots);
	m_floatSlotValues.reserveExactly(sizeRoundedUp);
	m_floatSlotValues.setSize(m_skeleton->m_numFloatSlots, 0.0f);

	CHECK_INVARIANT();
}



const		hkArray<noQTransform>& noaPose::GetSyncedPoseLocalSpace() const
{
	SyncLocalSpace();

	CHECK_INVARIANT();

	return m_localPose;
}

const		hkArray<noQTransform>& noaPose::GetSyncedPoseModelSpace() const
{
	SyncModelSpace();

	CHECK_INVARIANT();

	return m_modelPose;
}

NO_INLINE	const hkArray<float>& noaPose::GetFloatSlotValues() const {
	return m_floatSlotValues;
}

void noaPose::SetPoseLocalSpace( const hkArray<noQTransform>& poseLocal ) {
	const int numBones = m_skeleton->m_numBones;

	ASSERT(poseLocal.getSize() == numBones);

	// Copy local pose
	hkString::memCpy(m_localPose.begin(), poseLocal.begin(), numBones * sizeof (noQTransform));

	for (int b=0; b<numBones; ++b)
	{
		m_boneFlags[b] = F_BONE_MODEL_DIRTY;
	}

	m_localInSync = true;
	m_modelInSync = false;

	CHECK_INVARIANT()
}

void noaPose::SetPoseModelSpace( const hkArray<noQTransform>& poseModel )
{
	const int numBones = m_skeleton->m_numBones;

	HK_ASSERT(0, poseModel.getSize() == numBones);

	// Copy local pose
	hkString::memCpy(m_modelPose.begin(), poseModel.begin(), numBones * sizeof (noQTransform));

	for (int b=0; b<numBones; ++b)
	{
		m_boneFlags[b] = F_BONE_LOCAL_DIRTY;
	}

	m_localInSync = false;
	m_modelInSync = true;

	CHECK_INVARIANT()
}

NO_INLINE	const noQTransform& noaPose::GetBoneLocalSpace( int boneIdx ) const
{
	if (! IsFlagOn (boneIdx, F_BONE_LOCAL_DIRTY))
	{
		return m_localPose[boneIdx];
	}
	else {
		const noQTransform& modelFromBone = m_modelPose[boneIdx];
		const int16 parentIdx = m_skeleton->m_parentIndices[boneIdx];
		if (parentIdx != -1)
		{
			const noQTransform& modelFromParent = GetBoneModelSpace(parentIdx);
			// parentfrombone = inv(modelfromparent) * modelfrombone
			//m_localPose[boneIdx]
		}
		else
		{
			m_localPose[boneIdx] = modelFromBone;
		}

		ClearFlag (boneIdx, F_BONE_LOCAL_DIRTY);

		CHECK_INVARIANT();

		return m_localPose[boneIdx];
	}	
}

NO_INLINE	const noQTransform& noaPose::GetBoneModelSpace( int boneIdx ) const
{
	if (!IsFlagOn (boneIdx, F_BONE_MODEL_DIRTY))
	{
		return m_modelPose[boneIdx];
	}
	else
	{
		return CalculateBoneModelSpace(boneIdx);
	}
}

void noaPose::SyncLocalSpace() const
{
	if (m_localInSync) return;

	const int numBones = m_skeleton->m_numBones;

	for (int b=0; b<numBones; ++b)
	{
		if ( IsFlagOn (b, F_BONE_LOCAL_DIRTY) )
		{
			const noQTransform& modelFromBone = m_modelPose[b];
			//HK_ASSERT2(0x5a3281f6, ! isFlagOn(b, F_BONE_MODEL_DIRTY), "Trying to access uninitialized bone in pose" );

			const hkInt16 parentId = m_skeleton->m_parentIndices[b];
			if (parentId != -1)
			{
				const noQTransform& modelFromParent = GetBoneModelSpace( parentId );
				//HK_ASSERT2(0x5a3281f6, ! isFlagOn(parentId, F_BONE_MODEL_DIRTY), "Trying to access uninitialized bone in pose" );

				// modelfromBone = modelfromparent * parentfrombone
				// parentfrombone = inv(modelfromparent) * modelfrombone
				//m_localPose[b].setMulInverseMul(modelFromParent, modelFromBone);
			}
			else
			{
				m_localPose[b] = modelFromBone;
			}

			ClearFlag (b, F_BONE_LOCAL_DIRTY);
		}
	}

	CHECK_INVARIANT();

	m_localInSync = true;
}

void noaPose::SyncModelSpace() const
{
	if (m_modelInSync) return;

	const int numBones = m_skeleton->m_numBones;

	for (int b=0; b<numBones; ++b)
	{
		if (IsFlagOn (b, F_BONE_MODEL_DIRTY))
		{
			const noQTransform& parentFromBone = m_localPose[b];
			//HK_ASSERT2(0x5a3281f6, !isFlagOn(b, F_BONE_LOCAL_DIRTY), "Trying to access uninitialized bone in pose");

			const hkInt16 parentIdx = m_skeleton->m_parentIndices[b];

			if (parentIdx != -1)
			{
				const noQTransform& modelFromParent = m_modelPose[parentIdx];
				// note that we can assume safely that the parent in model is clean at this stage
				// but not in the general (random access) case
				//HK_ASSERT2(0x5a3281f6, !isFlagOn (parentIdx, F_BONE_MODEL_DIRTY), "Trying to access uninitialized bone in pose");

				// modelfrombone = modelfromparent * parentfrombone
				//m_modelPose[b].setMul(modelFromParent, parentFromBone);
			}
			else
			{
				m_modelPose[b] = parentFromBone;
			}

			ClearFlag (b, F_BONE_MODEL_DIRTY);
		}
	}

	m_modelInSync = true;

	CHECK_INVARIANT()
}



noQTransform& noaPose::AccessBoneLocalSpace( int boneIdx )
{
	// The model transform will be invalidated for all descendants
	// Make also sure the local transform for those is in sync
	MakeAllChildrenLocalSpace(boneIdx);

	GetBoneLocalSpace(boneIdx); // sync

	m_boneFlags[boneIdx] = F_BONE_MODEL_DIRTY;
	m_modelInSync = false;

	CHECK_INVARIANT()

	return m_localPose[boneIdx];
}

noQTransform& noaPose::AccessBoneModelSpace( int boneIdx, PropagateOrNot propagateOrNot /*= DONT_PROPAGATE*/ )
{
	if (propagateOrNot == DONT_PROPAGATE)
	{
		// The local transform will be invalidated for the first generation of descendants
		// Make also sure the model transform for those is in sync
		MakeFirstChildrenModelSpace(boneIdx);
	}
	else
	{
		// Act as if were actually setting the transform in local space
		MakeAllChildrenLocalSpace(boneIdx);
	}

	GetBoneModelSpace(boneIdx); // sync

	m_boneFlags[boneIdx] = F_BONE_LOCAL_DIRTY;
	m_localInSync = false;

	CHECK_INVARIANT()

	return m_modelPose[boneIdx];
}

hkArray<noQTransform>& noaPose::AccessSyncedPoseLocalSpace()
{
	SyncLocalSpace();

	return AccessUnsyncedPoseLocalSpace();
}

hkArray<noQTransform>& noaPose::AccessSyncedPoseModelSpace()
{
	SyncModelSpace();

	return AccessUnsyncedPoseModelSpace();
}

hkArray<noQTransform>& noaPose::AccessUnsyncedPoseLocalSpace()
{
	const int numBones = m_skeleton->m_numBones;
	for (int i=0; i<numBones; ++i)
	{
		m_boneFlags[i] = F_BONE_MODEL_DIRTY;
	}
	m_modelInSync = false;
	m_localInSync = true;

	CHECK_INVARIANT()

	return m_localPose;
}

hkArray<noQTransform>& noaPose::AccessUnsyncedPoseModelSpace()
{
	const int numBones = m_skeleton->m_numBones;
	for (int i=0; i<numBones; ++i)
	{
		m_boneFlags[i] = F_BONE_LOCAL_DIRTY;
	}
	m_localInSync = false;
	m_modelInSync = true;

	CHECK_INVARIANT()

	return m_modelPose;
}

void noaPose::SetToRefPose()
{
	hkString::memCpy( m_localPose.begin(), m_skeleton->m_refPose, sizeof(noQTransform) * m_skeleton->m_numBones );

	for (int b=0; b<m_skeleton->m_numBones; ++b)
	{
		m_boneFlags[b] = F_BONE_MODEL_DIRTY;
	}

	m_modelInSync = false;
	m_localInSync = true;

	CHECK_INVARIANT();

	for (int a=0; a<m_skeleton->m_numFloatSlots; ++a)
	{
		m_floatSlotValues[a] = 0.0f;
	}
}

void noaPose::EnforceSkeletonConstraintLocalSpace()
{
	CHECK_INTERNAL_FLAG_IS_CLEAR (1);

	const int numBones = m_skeleton->m_numBones;

	SyncLocalSpace();

	for (int boneIdx=0; boneIdx< numBones; ++boneIdx)
	{
		const noaBone* bone = m_skeleton->m_bones[boneIdx];

		if (bone->m_lockTrans)
		{
			m_localPose[boneIdx].m_trans = m_skeleton->m_refPose[boneIdx].m_trans;
			SetFlag(boneIdx, F_BONE_MODEL_DIRTY);
			SetFlag(boneIdx, F_BONE_INTERNAL_FLAG1);
		}
		else
		{
			const int16 parentIdx = m_skeleton->m_parentIndices[boneIdx];
			// Use the internal flag to invalidate all descendants
			if (parentIdx !=-1 && IsFlagOn(parentIdx, F_BONE_INTERNAL_FLAG1))
			{
				SetFlag(boneIdx, F_BONE_MODEL_DIRTY);
				SetFlag(boneIdx, F_BONE_INTERNAL_FLAG1);
			}

		}

	}

	ClearInternalFlags();

	CHECK_INVARIANT();
}

void noaPose::EnforceSkeletonConstraintModelSpace()
{
	CHECK_INTERNAL_FLAG_IS_CLEAR (1);

	const int numBones = m_skeleton->m_numBones;

	SyncModelSpace();

	for (int boneIdx=0; boneIdx< numBones; ++boneIdx)
	{
		const noaBone* bone = m_skeleton->m_bones[boneIdx];
		const int16 parentIdx = m_skeleton->m_parentIndices[boneIdx];
		if (bone->m_lockTrans)
		{
			const noVec4& localTranslation = m_skeleton->m_refPose[boneIdx].m_trans;

			if (parentIdx != -1)
			{
				const noQTransform& parentModelSpace = m_modelPose[parentIdx];
				// like setTransformedPos(), but we don't want scale
				//m_modelPose[boneIdx].m_translation.setRotatedDir(parentModelSpace.getRotation(), localTranslation);
				//m_modelPose[boneIdx].m_translation.add4(parentModelSpace.getTranslation());
			}
			else
			{
				m_modelPose[boneIdx].m_trans = localTranslation;
			}

			SetFlag(boneIdx, F_BONE_LOCAL_DIRTY);

			// Mark that we modified this guy
			SetFlag(boneIdx, F_BONE_INTERNAL_FLAG1);


		}
		else
		{
			// Use the internal flag to invalidate first descendants
			if (parentIdx!=-1 && IsFlagOn(parentIdx, F_BONE_INTERNAL_FLAG1))
			{
				SetFlag(boneIdx, F_BONE_LOCAL_DIRTY);
			}
		}
	}

	// Clear the modified flag
	ClearInternalFlags();

	CHECK_INVARIANT();

}

void noaPose::GetModelSpaceABB() const
{

}




const noQTransform& noaPose::CalculateBoneModelSpace( int boneIdx ) const
{
	CHECK_INTERNAL_FLAG_IS_CLEAR (2);

	HK_ASSERT2(0, IsFlagOn (boneIdx, F_BONE_MODEL_DIRTY), "Internal error: Method called at unexpected time");

	HK_ASSERT2(0x5a3281f6, ! IsFlagOn (boneIdx, F_BONE_LOCAL_DIRTY), "Trying to access uninitialized bone in pose" );

	int firstBoneInChain = boneIdx;

	// Flag bones in the chain, backwards
	while (true)
	{
		if (!IsFlagOn(firstBoneInChain, F_BONE_MODEL_DIRTY))
		{
			break;
		}

		const hkInt16 parentIdx = m_skeleton->m_parentIndices[firstBoneInChain];

		if (parentIdx == -1)
		{
			HK_ASSERT2(0x5a3281f6, !IsFlagOn (firstBoneInChain, F_BONE_LOCAL_DIRTY), "Trying to access uninitialized bone in pose" );

			m_modelPose[firstBoneInChain] = m_localPose[firstBoneInChain];
			ClearFlag (firstBoneInChain, F_BONE_MODEL_DIRTY);

			break; // leave the loop
		}

		SetFlag(firstBoneInChain, F_BONE_INTERNAL_FLAG2);
		firstBoneInChain = parentIdx;
	}

	// Accumulate the transform through the parents; the first one is in model space already
	HK_ASSERT2(0, !IsFlagOn (firstBoneInChain, F_BONE_MODEL_DIRTY), "Internal error: Bad logic somewhere" );

	for (int aBoneIdx = firstBoneInChain+1; aBoneIdx <= boneIdx; aBoneIdx++)
	{
		// Use only marked bones (part of the chain)
		if (!IsFlagOn(aBoneIdx, F_BONE_INTERNAL_FLAG2))
		{
			continue;
		}

		const hkInt16 aParentIdx = m_skeleton->m_parentIndices[aBoneIdx];

		HK_ASSERT2(0x5a3281f6, ! IsFlagOn (aBoneIdx, F_BONE_LOCAL_DIRTY), "Trying to access uninitialized bone in pose" );
		HK_ASSERT2(0x5a3281f6, ! IsFlagOn (aParentIdx, F_BONE_MODEL_DIRTY), "Trying to access uninitialized bone in pose" );

		//m_modelPose[aBoneIdx].setMul(m_modelPose[aParentIdx], m_localPose[aBoneIdx]);

		{
			noaPoseFlag f = m_boneFlags[aBoneIdx];
			ClearFlagExplicit (f, F_BONE_INTERNAL_FLAG2);
			ClearFlagExplicit (f, F_BONE_MODEL_DIRTY);
			m_boneFlags[aBoneIdx] = f;
		}
	}

	CHECK_INVARIANT();

	HK_ASSERT2(0, !IsFlagOn (boneIdx, F_BONE_MODEL_DIRTY), "Internal error: Bad logic somewhere" );

	return m_modelPose[boneIdx];
}

void noaPose::SetNonInitializedFlags()
{
	const int numBones = m_skeleton->m_numBones;
	for (int i=0; i < numBones; ++i)
	{
		m_boneFlags[i] = F_BONE_LOCAL_DIRTY | F_BONE_MODEL_DIRTY;
	}
	m_localInSync = false;
	m_modelInSync = false;
}

bool noaPose::CheckInternalFlagIsClear( noaPoseFlag flag ) const
{
#ifdef _DEBUG
	const int numBones = m_skeleton->m_numBones;

	// Check that for each bone we have one clean representation at least
	{
		for (int i=0; i<numBones; i++)
		{
			if (IsFlagOn (i, flag))
			{
				HK_ASSERT2(0x66565434, 0, "Invariant failed : Flag "<<" should be clear for all bones, and it's not for bone "<<i);
				return false;
			}
		}
	}
#endif
	return true;

}

bool noaPose::checkPoseValidity() const
{
		const int numBones = m_skeleton->m_numBones;

	// Check that for each bone we have one clean representation at least
	{
		for (int i=0; i<numBones; i++)
		{
			if (IsFlagOn (i, F_BONE_LOCAL_DIRTY) && IsFlagOn (i, F_BONE_MODEL_DIRTY) )
			{
				// Trying to access uninitialized pose
				HK_ASSERT2(0x5a3281f6, 0, "Invariant failed : both local and model transforms dirty for bone "<<i);				
				return false;
			}
		}
	}

	// Check that the "all in sync" flags are right
	{
		if (m_localInSync)
		{
			for (int i=0; i<numBones; i++)
			{
				if (IsFlagOn (i, F_BONE_LOCAL_DIRTY))
				{
					HK_ASSERT2(0x5a3281f4, 0, "Invariant failed : m_localInSync is true but some local transforms are dirty");
					return false;
				}
			}
		}
		if (m_modelInSync)
		{
			for (int i=0; i<numBones; i++)
			{
				if (IsFlagOn (i, F_BONE_MODEL_DIRTY))
				{
					HK_ASSERT2(0x5a3281f5,0, "Invariant failed : m_modelInSync is true but some model transforms are dirty");
					return false;
				}
			}
		}
	}

	return true;

}