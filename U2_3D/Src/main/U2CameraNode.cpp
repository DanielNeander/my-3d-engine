#include <U2_3D/src/U23DLibPCH.h>
#include "U2CameraNode.h"
//#include "U2Math.h"

IMPLEMENT_RTTI(U2CameraNode, U2Node);

U2CameraNode::U2CameraNode(U2Camera* pCamera /* = 0 */)
	:m_spCamera(pCamera)
{
	if(m_spCamera)
	{
		m_tmLocal.SetTrans(m_spCamera->GetEye());
		D3DXMATRIX mat;		
					
		mat._11 = m_spCamera->GetDir().x;
		mat._12 = m_spCamera->GetUp().x;
		mat._13 = m_spCamera->GetRight().x;		
		mat._14 = 0.f;

		mat._21 = m_spCamera->GetDir().y;
		mat._22 = m_spCamera->GetUp().y;
		mat._23 = m_spCamera->GetRight().y;				
		mat._24 = 0.f;
		
		mat._31 = m_spCamera->GetDir().z;
		mat._32 = m_spCamera->GetUp().z;	
		mat._33 = m_spCamera->GetRight().z;
		mat._34 = 0.f;
		
		mat._41 = 0.f;
		mat._42 = 0.f;
		mat._43 = 0.f;
		mat._44 = 1.f;
		
		m_tmLocal.SetRot(mat);
	}

}

U2CameraNode::~U2CameraNode()
{
	m_spCamera = 0;
}

void U2CameraNode::SetCamera(U2Camera* pCamera)
{
	m_spCamera = pCamera;

	if(m_spCamera)
	{
		m_tmLocal.SetTrans(m_spCamera->GetEye());
		D3DXMATRIX rot;
		U2Math::MakeMatrix(rot, m_spCamera->GetDir(), m_spCamera->GetUp(), 
			m_spCamera->GetRight(), true);
		m_tmLocal.SetRot(rot);
		Update();
	}
}


void U2CameraNode::UpdateWorldData(float fApptime)
{
	U2Node::UpdateWorldData(fApptime);

	if(m_spCamera)
	{
		m_spCamera->SetFrame(m_tmWorld.GetTrans(),
			U2Math::GetCol(m_tmWorld.GetRot(), 0),
			U2Math::GetCol(m_tmWorld.GetRot(), 1),
			U2Math::GetCol(m_tmWorld.GetRot(), 2));			
	}

}