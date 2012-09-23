#ifndef NO_RIG_POSE_H
#define NO_RIG_POSE_H

#include <Math/QTransform.h>
#include "noaSkeleton.h"

class noaSkeleton;
class noaPose 
{
public:
	enum PoseSpace 
	{
		MODEL_SPACE,
		LOCAL_SPACE
	};

	NO_INLINE		noaPose(const noaSkeleton* skeleton);
	NO_INLINE		noaPose(PoseSpace space, const noaSkeleton* skeleton, const hkArray<noQTransform>& pose);
	NO_INLINE static int GetRequiredMemorySize (const noaSkeleton* skeleton);

	
	enum PropagateOrNot
	{
		DONT_PROPAGATE = 0,
		PROPAGATE = 1
	};	
	
	NO_INLINE	const noaSkeleton*		GetSkeleton( void ) const;
	const		hkArray<noQTransform>&	GetSyncedPoseLocalSpace() const;
	const		hkArray<noQTransform>&	GetSyncedPoseModelSpace() const;
	NO_INLINE	const hkArray<float>&	GetFloatSlotValues() const;
	void		SetPoseLocalSpace(const hkArray<noQTransform>& poseLocal);
	void		SetPoseModelSpace(const hkArray<noQTransform>& poseModel);	

	NO_INLINE	const noQTransform&		GetBoneLocalSpace(int boneIdx) const;
	NO_INLINE	const noQTransform&		GetBoneModelSpace(int boneIdx) const;
	NO_INLINE	float					GetFloatValue(int floatSlotIdx) const;
	NO_INLINE	void					SetBoneLocalSpace(int boneIdx, const noQTransform& boneLocal);
	NO_INLINE	void					SetBoneModelSpace(int boneIdx, const noQTransform& boneModel, PropagateOrNot propagateOrNot);
	NO_INLINE	void					SetFloatSlotValue(int floatSlotIdx, float val);

	void		SyncLocalSpace() const;
	void		SyncModelSpace() const;	
	inline		void					SyncAll(void) const;
	
	noQTransform& AccessBoneLocalSpace( int boneIdx );
	noQTransform& AccessBoneModelSpace(int boneIdx, PropagateOrNot propagateOrNot = DONT_PROPAGATE);
	hkArray<noQTransform>& AccessSyncedPoseLocalSpace();
	hkArray<noQTransform>& AccessSyncedPoseModelSpace();

	hkArray<noQTransform>& AccessUnsyncedPoseLocalSpace();
	hkArray<noQTransform>& AccessUnsyncedPoseModelSpace();

	void SetToRefPose();
	void EnforceSkeletonConstraintLocalSpace();
	void EnforceSkeletonConstraintModelSpace();
	void GetModelSpaceABB() const;

	NO_INLINE	noaPose&				operator=( const noaPose& other );

private:
	typedef unsigned int		noaPoseFlag;
	const noaSkeleton* m_skeleton;

	mutable hkArray<noQTransform> m_localPose;
	mutable hkArray<noQTransform> m_modelPose;
	mutable hkArray<noaPoseFlag> m_boneFlags;

	mutable hkBool m_modelInSync;
	mutable hkBool m_localInSync;

	hkArray<hkReal> m_floatSlotValues;

	enum
	{
		F_BONE_LOCAL_DIRTY = 0x1,
		F_BONE_MODEL_DIRTY = 0x2,
		F_BONE_INTERNAL_FLAG1 = 0x4, // internally used (enforceConstraints, makeAllChildrenLocal)
		F_BONE_INTERNAL_FLAG2 = 0x8, // internally used (calculateBoneModelSpace)
		F_BONE_INTERNAL_FLAG3 = 0x10, // internally used
	};

	// Masks
	enum
	{
		M_BONE_INTERNAL_FLAGS = F_BONE_INTERNAL_FLAG1 | F_BONE_INTERNAL_FLAG2 | F_BONE_INTERNAL_FLAG3
	};

	NO_INLINE	int						IsFlagOn( int boneIdx, noaPoseFlag enumtoCheck) const;
	NO_INLINE	void					SetFlag( int boneIdx,	noaPoseFlag enumToCheck) const;
	NO_INLINE	void					ClearFlag( int boneIdx, noaPoseFlag enumToCheck) const;
	NO_INLINE	int						IsFlagOnExplicit( noaPoseFlag flag, noaPoseFlag enumToCheck) const;
	NO_INLINE	void					SetFlagExplicit( noaPoseFlag flag, noaPoseFlag enumToCheck) const;
	NO_INLINE	void					ClearFlagExplicit( noaPoseFlag flag, noaPoseFlag enumToCheck) const;
	NO_INLINE	void					ClearInternalFlags( void ) const;
	NO_INLINE	void	MakeAllChildrenLocalSpace( int boneIdx ) const;
	NO_INLINE	void	MakeFirstChildrenModelSpace ( int boneIdx ) const;

	const noQTransform& CalculateBoneModelSpace (int boneIdx) const;

	void SetNonInitializedFlags();
	bool CheckInternalFlagIsClear(noaPoseFlag flag) const;

public:

	// For unit testing
	bool checkPoseValidity () const;
};

#include <animation/Rig/noaPose.inl>

#endif