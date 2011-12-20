inline void U2SkinController::SetAnimEnabled(bool b)
{
	m_bAnimEnabled = b;
}

inline bool U2SkinController::IsAnimEnabled() const
{
	return m_bAnimEnabled;
}

inline void U2SkinController::BeginJoints(uint32 numJoints)
{
	m_pAnimData->GetSkeleton().BeginJoints(numJoints);
}


inline void U2SkinController::SetJoint(int idx, int parentJointIdx, const D3DXVECTOR3& poseTrans,
			  const D3DXQUATERNION& poseRotate, const D3DXVECTOR3& poseScale, const U2DynString& name)
{
	m_pAnimData->GetSkeleton().SetJoint(idx, parentJointIdx, 
		poseTrans, poseRotate, poseScale, name);
}

inline void U2SkinController::EndJoints()
{
	m_pAnimData->GetSkeleton().EndJoints();
}

inline uint32 U2SkinController::GetNumJoints() const
{
	return m_pAnimData->GetSkeleton().GetNumJoints();
}

inline void U2SkinController::GetJoint(int idx, int& parentJointIdx, D3DXVECTOR3& poseTrans,
			  D3DXQUATERNION& poseRotate, D3DXVECTOR3& poseScale, U2DynString& name)
{
	U2Bone& joint = m_pAnimData->GetSkeleton().GetJoint(idx);
	parentJointIdx = joint.GetParentJointIdx();
	poseTrans = joint.GetPoseTrans();
	poseRotate = joint.GetPoseRot();
	poseScale = joint.GetPoseScale();
	name = joint.GetName();
}

inline void U2SkinController::SetAnim(const U2DynString& szFilename)
{
	UnloadAnim();
	m_szAnimname = szFilename;

}

inline const U2DynString& U2SkinController::GetAnim() const
{
	return m_szAnimname;
}


//-------------------------------------------------------------------------------------------------
inline int U2SkinController::GetAnimSetIdxHandle() const
{
	return m_iAnimSetIdx;	
}

//-------------------------------------------------------------------------------------------------
inline int U2SkinController::GetAnimVarIdxHandle() const
{
	return m_iAnimVarIdx;
}