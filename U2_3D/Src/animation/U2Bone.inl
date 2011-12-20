inline U2Bone::U2Bone() 
	:m_pParentJoint(NULL),
	m_iParentJointIdx(-1),
	m_poseScale(1.f, 1.f, 1.f),
	m_poseTrans(0.0f, 0.0f, 0.0f),
	m_poseRot(0.0f, 0.0f, 0.0f, 0.0f),
	m_trans(0.0f, 0.0f, 0.0f),
	m_quatRot(0.0f, 0.0f, 0.0f, 0.0f),
	m_scale(1.f, 1.f, 1.f),
	m_variationScale(1.f, 1.f, 1.f),
	m_bMatDirty(false),
	m_bLockMat(false),
	m_bIsUpTodate(false)
{

	D3DXMatrixIdentity(&m_poseMat);
	D3DXMatrixIdentity(&m_invPoseMat);
	D3DXMatrixIdentity(&m_skinMat44);	
	D3DXMatrixIdentity(&m_skinMat33);
	D3DXMatrixIdentity(&m_localUnscaledMat);
	D3DXMatrixIdentity(&m_localScaledMat);	
	D3DXMatrixIdentity(&m_worldUnscaledMat);
	D3DXMatrixIdentity(&m_worldScaledMat);

}


inline U2Bone::~U2Bone()
{
	
}


inline void U2Bone::SetParentJointIdx(int idx)
{
	m_iParentJointIdx = idx;	
}


inline int U2Bone::GetParentJointIdx() const
{
	return m_iParentJointIdx;
}


inline void U2Bone::SetParentJoint(U2Bone* pParent)
{
	m_pParentJoint = pParent;
}

inline U2Bone* U2Bone::GetParentJoint() const
{
	return m_pParentJoint;
}


inline void U2Bone::SetPose(const D3DXVECTOR3& trans, 
					  const D3DXQUATERNION& quat, const D3DXVECTOR3& scale)
{
	m_poseTrans = trans;
	m_poseRot = quat;
	m_poseScale = scale;

	D3DXMatrixIdentity(&m_poseMat);
	D3DXMatrixScaling(&m_poseMat, m_poseScale.x, m_poseScale.y, m_poseScale.z);

	D3DXMATRIX temp;
	D3DXMatrixRotationQuaternion(&temp, &m_poseRot);
	m_poseMat = m_poseMat * temp;
	m_poseMat._41 += m_poseTrans.x;
	m_poseMat._42 += m_poseTrans.y;
	m_poseMat._43 += m_poseTrans.z;

	m_localScaledMat = m_poseMat;
	m_worldScaledMat = m_poseMat;

	if(m_pParentJoint)
	{
		m_poseMat = m_poseMat * m_pParentJoint->m_poseMat;
	}
		
	D3DXMatrixInverse(&m_invPoseMat, NULL, &m_poseMat);

}


inline const D3DXVECTOR3& U2Bone::GetPoseTrans() const
{
	return m_poseTrans;
}


inline const D3DXQUATERNION& U2Bone::GetPoseRot() const
{	
	return m_poseRot;
}


inline const D3DXVECTOR3& U2Bone::GetPoseScale() const
{
	return m_poseScale;
}


inline void U2Bone::SetTrans(const D3DXVECTOR3& trans)
{
	m_trans = trans;
	m_bMatDirty = true;
}


inline const D3DXVECTOR3& U2Bone::GetTrans() const
{
	return m_trans;
}


inline void U2Bone::SetRot(D3DXQUATERNION quat)
{
	m_quatRot = quat;
	m_bMatDirty = true;
}


inline const D3DXQUATERNION& U2Bone::GetRot() const
{
	return m_quatRot;
}


inline void U2Bone::SetScale(const D3DXVECTOR3& scale)
{
	m_scale = scale;
	m_bMatDirty = true;
}


inline const D3DXVECTOR3& U2Bone::GetScale() const
{
	return m_scale;
}





inline void U2Bone::SetLocalMat(const D3DXMATRIX& mat)
{
	m_localScaledMat = mat;
	m_bMatDirty = false;
}


inline const D3DXMATRIX& U2Bone::GetPoseMat() const
{
	return m_poseMat;
}


inline const D3DXMATRIX& U2Bone::GetInvPoseMat() const
{
	return m_invPoseMat;
}


inline void U2Bone::SetMat(const D3DXMATRIX& mat)
{
	m_worldScaledMat = mat;
	m_bLockMat = true;
}


inline const D3DXMATRIX& U2Bone::GetMat() const
{
	return m_worldScaledMat;
}


inline const D3DXMATRIX& U2Bone::GetSKinMat44() const
{
	return m_skinMat44;
}


inline const D3DXMATRIX& U2Bone::GetSkinMat33() const
{
	return m_skinMat33;
}


inline void U2Bone::ClearUpTodateFlag()
{
	m_bIsUpTodate = false;
}


inline bool U2Bone::IsUpTodate() const
{
	return m_bIsUpTodate;
}

inline void U2Bone::Evaluate()
{
	if(!m_bIsUpTodate)
	{
		if(m_bMatDirty)
		{
			D3DXMatrixIdentity(&m_localScaledMat);
			D3DXMatrixIdentity(&m_localUnscaledMat);

			//D3DXQUATERNION temp = m_quatRot; // 시간이 오래 걸림
			//D3DXQuaternionNormalize(&m_quatRot, &temp);
			D3DXQuaternionNormalize(&m_quatRot, &m_quatRot);

			D3DXMATRIX matRot;
			D3DXMatrixRotationQuaternion(&matRot, &m_quatRot);

			m_localUnscaledMat = m_localUnscaledMat * matRot;			
			m_localUnscaledMat._41 += m_trans.x;
			m_localUnscaledMat._42 += m_trans.y;
			m_localUnscaledMat._43 += m_trans.z;
			//D3DXMatrixMultiply(&m_localUnscaledMat, &m_localUnscaledMat, &matRot);			

			D3DXMatrixScaling(&m_localScaledMat, m_scale.x, m_scale.y, m_scale.z);						
			D3DXMATRIX variationScaleMat;
			D3DXMatrixScaling(&variationScaleMat, m_variationScale.x, m_variationScale.y, m_variationScale.z);
			m_localScaledMat = m_localScaledMat * variationScaleMat;
			m_localScaledMat = m_localScaledMat * matRot;	// 시간이 오래 걸림
			m_localScaledMat._41 += m_trans.x;
			m_localScaledMat._42 += m_trans.y;
			m_localScaledMat._43 += m_trans.z;
			//D3DXMatrixScaling(&m_localScaledMat, m_variationScale.x, m_variationScale.y, m_variationScale.z);
			//D3DXMatrixMultiply(&m_localScaledMat, &m_localScaledMat, &matRot);			
			


			m_bMatDirty = false;
		}

		if(!m_bLockMat)
		{
			m_worldScaledMat = m_localScaledMat;
			m_worldUnscaledMat = m_localUnscaledMat;

			if(m_pParentJoint)
			{
				if(!m_pParentJoint->IsUpTodate())
				{
					m_pParentJoint->Evaluate();
				}

				D3DXVECTOR3 trans(m_worldUnscaledMat._41, m_worldUnscaledMat._42, m_worldUnscaledMat._43);
				trans.x *= m_pParentJoint->m_scale.x * m_pParentJoint->m_variationScale.x;
				trans.y *= m_pParentJoint->m_scale.y * m_pParentJoint->m_variationScale.y;
				trans.z *= m_pParentJoint->m_scale.z * m_pParentJoint->m_variationScale.z;

				m_worldUnscaledMat._41 = trans.x; 
				m_worldUnscaledMat._42 = trans.y;
				m_worldUnscaledMat._43 = trans.z;

				trans.x =m_worldScaledMat._41; trans.y =m_worldScaledMat._42; trans.z =m_worldScaledMat._43;
				trans.x *= m_pParentJoint->m_scale.x * m_pParentJoint->m_variationScale.x;
				trans.y *= m_pParentJoint->m_scale.y * m_pParentJoint->m_variationScale.y;
				trans.z *= m_pParentJoint->m_scale.z * m_pParentJoint->m_variationScale.z;
				m_worldScaledMat._41 = trans.x;
				m_worldScaledMat._42 = trans.y;
				m_worldScaledMat._43 = trans.z;

				// we calculate 2 world matrices
				// the unscaled one has uniform axis, which our children need to calculate their matrices
				// the scaled one is the one used to calculate the skin matrix (the applied scaling is the local,
				// parent scaling which influences the translation of the joint has been handled above)
				m_worldUnscaledMat = m_worldUnscaledMat * m_pParentJoint->m_worldUnscaledMat;
				m_worldScaledMat = m_worldScaledMat * m_pParentJoint->m_worldUnscaledMat;
				//D3DXMatrixMultiply(&m_worldUnscaledMat, &m_worldUnscaledMat, &m_pParentJoint->m_worldUnscaledMat);
				//D3DXMatrixMultiply(&m_worldScaledMat, &m_worldScaledMat, &m_pParentJoint->m_worldUnscaledMat);
				
			}
		}
	}



	m_skinMat44 = m_invPoseMat * m_worldScaledMat;	
	//D3DXMatrixMultiply(&m_skinMat44, &m_invPoseMat, &m_worldScaledMat);

	m_skinMat33 = m_skinMat44;
	m_skinMat33._41 = m_skinMat33._42 = m_skinMat33._43 = 0.f;

	//LOG_DEBUG << _T("Bone Name : ") << this->GetName().Str();

	//const size_t uiSize = 128;

	//TCHAR atcMsg[uiSize];
	//_sntprintf(atcMsg, uiSize, _T("Skin Row 1: %f, %f, %f "),
	//	m_skinMat44._11, m_skinMat44._12, m_skinMat44._13);
	//LOG_DEBUG <<  atcMsg;

	//_sntprintf(atcMsg, uiSize, _T("Skin Row 2: %f, %f, %f "),
	//	m_skinMat44._21, m_skinMat44._22, m_skinMat44._23);
	//
	//LOG_DEBUG << atcMsg;

	//_sntprintf(atcMsg, uiSize, _T("Skin Row 3: %f, %f, %f "),
	//	m_skinMat44._31, m_skinMat44._32, m_skinMat44._33);	
	//LOG_DEBUG << atcMsg;

	m_bIsUpTodate = true;
}