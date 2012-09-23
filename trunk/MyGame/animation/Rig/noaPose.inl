
#ifdef _DEBUG
#define CHECK_INVARIANT()					checkPoseValidity();
#define CHECK_INTERNAL_FLAG_IS_CLEAR(f)		CheckInternalFlagIsClear(F_BONE_INTERNAL_FLAG##f);
#else
#define CHECK_INVARIANT()					;
#define CHECK_INTERNAL_FLAG_IS_CLEAR(f)		;
#endif

NO_INLINE	const noaSkeleton* noaPose::GetSkeleton( void ) const
{
	return m_skeleton;
}

NO_INLINE	float noaPose::GetFloatValue( int floatSlotIdx ) const {
	return m_floatSlotValues[ floatSlotIdx ];
}



NO_INLINE	void noaPose::SetFloatSlotValue( int floatSlotIdx, float val ) {
	m_floatSlotValues[ floatSlotIdx ] = val;

}


void noaPose::SyncAll( void ) const
{
	SyncLocalSpace();
	SyncModelSpace();
}

NO_INLINE	noaPose& noaPose::operator=( const noaPose& other )
{
	m_skeleton = other.m_skeleton;
	m_localPose = other.m_localPose;
	m_modelPose = other.m_modelPose;
	m_boneFlags = other.m_boneFlags;
	m_modelInSync = other.m_modelInSync;
	m_localInSync = other.m_localInSync;
	m_floatSlotValues = other.m_floatSlotValues;
}

NO_INLINE	int noaPose::IsFlagOn( int boneIdx, noaPoseFlag enumtoCheck ) const
{
	return m_boneFlags[boneIdx] & enumtoCheck;
}

NO_INLINE	void noaPose::SetFlag( int boneIdx, noaPoseFlag enumtoCheck ) const
{
	m_boneFlags[boneIdx] |= enumtoCheck;
}

NO_INLINE	void noaPose::ClearFlag( int boneIdx, noaPoseFlag enumToCheck ) const {	
	m_boneFlags[boneIdx] &= static_cast<noaPoseFlag>(~enumToCheck);
}

NO_INLINE	int noaPose::IsFlagOnExplicit( noaPoseFlag flag, noaPoseFlag enumToCheck ) const {
	return flag & enumToCheck;
}

NO_INLINE	void noaPose::SetFlagExplicit( noaPoseFlag flag, noaPoseFlag enumToCheck ) const {
	flag &= static_cast<noaPoseFlag>(~enumToCheck);
}

NO_INLINE	void noaPose::ClearFlagExplicit( noaPoseFlag flag, noaPoseFlag enumToCheck ) const {
	flag &= static_cast<noaPoseFlag>(~enumToCheck);
}

NO_INLINE	void noaPose::ClearInternalFlags( void ) const {
	const int numBones = m_boneFlags.getSize();
	for (int i=0; i<numBones; ++i)
	{
		m_boneFlags[i] &= (~M_BONE_INTERNAL_FLAGS);
	}
}

NO_INLINE	void noaPose::MakeAllChildrenLocalSpace( int boneIdx ) const
{
	CHECK_INTERNAL_FLAG_IS_CLEAR(1);

	const int numBones = m_skeleton->m_numBones;

	SetFlag(boneIdx, F_BONE_INTERNAL_FLAG1);

	// Synchronize all descendants local representation
	{
		for (int i= boneIdx+1; i<numBones; ++i)
		{
			const hkInt16 parentId = m_skeleton->m_parentIndices[i];
			if (parentId != -1 && IsFlagOn(parentId, F_BONE_INTERNAL_FLAG1))
			{
				GetBoneLocalSpace( i ); // sync local
				SetFlag( i, F_BONE_INTERNAL_FLAG1);
				m_modelInSync = false;
			}
		} 
	}

	// Dirty all descendants model representation
	{
		for (int i= boneIdx+1; i < numBones; ++i)
		{
			noaPoseFlag f = m_boneFlags[i];
			if (IsFlagOnExplicit( f, F_BONE_INTERNAL_FLAG1))
			{
				SetFlagExplicit( f, F_BONE_MODEL_DIRTY);
				ClearFlagExplicit( f, F_BONE_INTERNAL_FLAG1);
				m_boneFlags[i] = f;
			}
		}
	}
}

NO_INLINE	void noaPose::MakeFirstChildrenModelSpace( int boneIdx ) const
{
	const int numBones = m_skeleton->m_numBones;

	for (int i= boneIdx+1; i<numBones; ++i)
	{
		const hkInt16 parentId = m_skeleton->m_parentIndices[i];
		if (parentId == boneIdx)
		{
			GetBoneModelSpace( i ); // sync model
			m_boneFlags[i] = F_BONE_LOCAL_DIRTY;
			m_localInSync = false;
		}
	}
}

NO_INLINE	void noaPose::SetBoneLocalSpace( int boneIdx, const noQTransform& boneLocal )
{
	// The model transform will be invalidated for all descendants
	// Make also sure the local transform for those is in sync
	MakeAllChildrenLocalSpace(boneIdx);

	m_localPose[boneIdx] = boneLocal;

	m_boneFlags[boneIdx] = F_BONE_MODEL_DIRTY;

	m_modelInSync = false;

	CHECK_INVARIANT();
}

NO_INLINE	void noaPose::SetBoneModelSpace( int boneIdx, const noQTransform& boneModel, PropagateOrNot propagateOrNot )
{
	if (! propagateOrNot )
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

	m_modelPose[boneIdx] = boneModel;

	m_boneFlags[boneIdx] = F_BONE_LOCAL_DIRTY;

	m_localInSync = false;

	CHECK_INVARIANT();
}

int noaPose::GetRequiredMemorySize (const noaSkeleton* skeleton)
{
	const int numBones = skeleton->m_numBones;

	const int floatSizeRoundedUp = HK_NEXT_MULTIPLE_OF( 4, skeleton->m_numFloatSlots);

	const int totalSize =  numBones * ( 2 * hkSizeOf(noQTransform) + hkSizeOf(noaPoseFlag) )
		+ floatSizeRoundedUp * hkSizeOf(hkReal);

	return totalSize;
}

NO_INLINE noaPose::noaPose( const noaSkeleton* skeleton )
	:m_skeleton(skeleton), m_modelInSync(false), m_localInSync(false)
{
	const int numBones = m_skeleton->m_numBones;

	m_modelPose.setSize(numBones);
	m_localPose.setSize(numBones);
	m_boneFlags.setSize(numBones);

	const int sizeRoundedUp = HK_NEXT_MULTIPLE_OF( 4, m_skeleton->m_numFloatSlots);
	m_floatSlotValues.reserveExactly(sizeRoundedUp);
	m_floatSlotValues.setSize(m_skeleton->m_numFloatSlots, 0.0f);

#ifdef _DEBUG
	SetNonInitializedFlags();
#endif
}