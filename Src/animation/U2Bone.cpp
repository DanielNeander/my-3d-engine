#include <U2_3D/src/U23DLibPCH.h>
#include "U2Bone.h"


//void U2Bone::Evaluate()
//{
//	if(!m_bIsUpTodate)
//	{
//		if(m_bMatDirty)
//		{
//			D3DXMatrixIdentity(&m_localScaledMat);
//			D3DXMatrixIdentity(&m_localUnscaledMat);
//
//			D3DXQUATERNION temp = m_quatRot;
//			D3DXQuaternionNormalize(&m_quatRot, &temp);
//
//			D3DXMATRIX matRot;
//			D3DXMatrixRotationQuaternion(&matRot, &m_quatRot);
//
//			m_localUnscaledMat = m_localUnscaledMat * matRot;
//			m_localUnscaledMat._41 += m_trans.x;
//			m_localUnscaledMat._42 += m_trans.y;
//			m_localUnscaledMat._43 += m_trans.z;
//
//			D3DXMatrixScaling(&m_localScaledMat, m_scale.x, m_scale.y, m_scale.z);						
//			D3DXMATRIX variationScaleMat;
//			D3DXMatrixScaling(&variationScaleMat, m_variationScale.x, m_variationScale.y, m_variationScale.z);
//			m_localScaledMat = m_localScaledMat * variationScaleMat;
//			m_localScaledMat = m_localScaledMat * matRot;
//			m_localScaledMat._41 += m_trans.x;
//			m_localScaledMat._42 += m_trans.y;
//			m_localScaledMat._43 += m_trans.z;
//
//			m_bMatDirty = false;
//		}
//
//		if(!m_bLockMat)
//		{
//			m_worldScaledMat = m_localScaledMat;
//			m_worldUnscaledMat = m_localUnscaledMat;
//
//			if(m_pParentJoint)
//			{
//				if(!m_pParentJoint->IsUpTodate())
//				{
//					m_pParentJoint->Evaluate();
//				}
//
//				D3DXVECTOR3 trans(m_worldUnscaledMat._41, m_worldUnscaledMat._42, m_worldUnscaledMat._43);
//				trans.x *= m_pParentJoint->m_scale.x * m_pParentJoint->m_variationScale.x;
//				trans.y *= m_pParentJoint->m_scale.y * m_pParentJoint->m_variationScale.y;
//				trans.z *= m_pParentJoint->m_scale.z * m_pParentJoint->m_variationScale.z;
//				m_worldUnscaledMat._41 = trans.x; 
//				m_worldUnscaledMat._42 = trans.y;
//				m_worldUnscaledMat._43 = trans.z;
//
//				trans.x =m_worldScaledMat._41; trans.y =m_worldScaledMat._42; trans.z =m_worldScaledMat._43;
//				trans.x *= m_pParentJoint->m_scale.x * m_pParentJoint->m_variationScale.x;
//				trans.y *= m_pParentJoint->m_scale.y * m_pParentJoint->m_variationScale.y;
//				trans.z *= m_pParentJoint->m_scale.z * m_pParentJoint->m_variationScale.z;
//				m_worldScaledMat._41 = trans.x;
//				m_worldScaledMat._42 = trans.y;
//				m_worldScaledMat._43 = trans.z;
//
//				// we calculate 2 world matrices
//				// the unscaled one has uniform axis, which our children need to calculate their matrices
//				// the scaled one is the one used to calculate the skin matrix (the applied scaling is the local,
//				// parent scaling which influences the translation of the joint has been handled above)
//				m_worldUnscaledMat = m_worldUnscaledMat * m_pParentJoint->m_worldUnscaledMat;
//				m_worldScaledMat = m_worldScaledMat * m_pParentJoint->m_worldUnscaledMat;
//			}
//		}
//	}
//
//
//
//	m_skinMat44 = m_invPoseMat * m_worldScaledMat;	
//	
//	m_skinMat33 = m_skinMat44;
//	m_skinMat33._41 = m_skinMat33._42 = m_skinMat33._43 = 0.f;
//
//	//LOG_DEBUG << _T("Bone Name : ") << this->GetName().Str();
//
//	//const size_t uiSize = 128;
//
//	//TCHAR atcMsg[uiSize];
//	//_sntprintf(atcMsg, uiSize, _T("Skin Row 1: %f, %f, %f "),
//	//	m_skinMat44._11, m_skinMat44._12, m_skinMat44._13);
//	//LOG_DEBUG <<  atcMsg;
//
//	//_sntprintf(atcMsg, uiSize, _T("Skin Row 2: %f, %f, %f "),
//	//	m_skinMat44._21, m_skinMat44._22, m_skinMat44._23);
//	//
//	//LOG_DEBUG << atcMsg;
//
//	//_sntprintf(atcMsg, uiSize, _T("Skin Row 3: %f, %f, %f "),
//	//	m_skinMat44._31, m_skinMat44._32, m_skinMat44._33);	
//	//LOG_DEBUG << atcMsg;
//
//	m_bIsUpTodate = true;
//}