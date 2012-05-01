inline U2Skeleton::U2Skeleton()
{

}

inline U2Skeleton::~U2Skeleton()
{
	unsigned int i;
	for(i = 0; i < m_jointArray.Size(); ++i)
	{
		U2_DELETE m_jointArray[i];
	}

}

inline void U2Skeleton::UpdateParentJointPtrs()
{
	uint32 i;
	for(i=0; i < GetNumJoints(); ++i)
	{
		U2Bone *pJoint = m_jointArray[i];
		int parentJointIdx = pJoint->GetParentJointIdx();
		if(parentJointIdx != -1)
		{
			pJoint->SetParentJoint((m_jointArray[parentJointIdx]));
		}
	}
}


inline U2Skeleton::U2Skeleton(const U2Skeleton& src)
{
	m_jointArray = src.m_jointArray;
	UpdateParentJointPtrs();
}


inline void U2Skeleton::operator=(const U2Skeleton& src)
{
	m_jointArray = src.m_jointArray;
	UpdateParentJointPtrs();
}

inline void U2Skeleton::Clear()
{
	m_jointArray.SetSize(0);
}

inline void U2Skeleton::BeginJoints(int num)
{
	U2ASSERT(num > 0);
	m_jointArray.SetSize(num);
}

inline void U2Skeleton::SetJoint(int idx, int parentJointIdx, const D3DXVECTOR3& poseTrans,
			  const D3DXQUATERNION& poseRotate, const D3DXVECTOR3& poseScale, const U2DynString& name)
{
	U2Bone* pNewJoint = U2_NEW U2Bone;
	pNewJoint->SetParentJointIdx(parentJointIdx);
	if(-1 == parentJointIdx)
	{
		pNewJoint->SetParentJoint(NULL);
	}
	else 
	{
		pNewJoint->SetParentJoint((this->m_jointArray[parentJointIdx]));
	}
	pNewJoint->SetPose(poseTrans, poseRotate, poseScale);
	pNewJoint->SetName(name);
	m_jointArray[idx] = pNewJoint;
}


inline void U2Skeleton::EndJoints()
{

}

inline uint32 U2Skeleton::GetNumJoints() const
{
	return m_jointArray.Size();
}

inline U2Bone& U2Skeleton::GetJoint(int idx) const
{
	return *m_jointArray[idx];
}

inline int U2Skeleton::GetJointIdx(const U2DynString szJointname) const
{
	int idx;
	for(idx = 0; idx < (int)GetNumJoints(); ++idx)
	{
		if(m_jointArray[idx]->GetName() == szJointname)
		{
			return idx;
		}
	}
	return -1;
}

inline void U2Skeleton::Evaluate()
{
	uint32 i;
	uint32 num = m_jointArray.Size();
	for(i=0; i < num; ++i)
	{
		m_jointArray[i]->ClearUpTodateFlag();
	}

	for(i=0; i < num; ++i)
	{
		m_jointArray[i]->Evaluate();
	}

}

