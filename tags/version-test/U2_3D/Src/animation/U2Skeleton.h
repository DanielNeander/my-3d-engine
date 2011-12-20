/**************************************************************************************************
module	:	U2Skeleton
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_SKELETON_H
#define U2_SKELETON_H

#include <U2Lib/src/U2TFixedArray.h>
#include "U2Bone.h"

class U2Skeleton : public U2Object 
{
public:
	U2Skeleton();
	virtual ~U2Skeleton();

	U2Skeleton(const U2Skeleton& src);
	void operator=(const U2Skeleton& src);

	void Clear();

	void BeginJoints(int num);

	void SetJoint(int idx, int parentJointIdx, const D3DXVECTOR3& poseTrans,
		const D3DXQUATERNION& poseRotate, const D3DXVECTOR3& poseScale, const U2DynString& name);

	void EndJoints();

	uint32 GetNumJoints() const;

	U2Bone& GetJoint(int idx) const;

	int GetJointIdx(const U2DynString szJointname) const;

	void Evaluate();

private:
	void UpdateParentJointPtrs();

	U2FixedPrimitiveArray<U2Bone*> m_jointArray;
};

typedef U2SmartPtr<U2Skeleton> U2SkeletonPtr;

#include "U2Skeleton.inl"


#endif

