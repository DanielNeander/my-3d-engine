#include <u2_3d/src/U23DLibPCH.h>
#include "U2BillboardNode.h"


IMPLEMENT_RTTI(U2BillboardNode, U2Node);
U2BillboardNode::U2BillboardNode(U2Camera *pCam)
	:m_spCamera(pCam)
{

}

U2BillboardNode::~U2BillboardNode()
{
	

}


void U2BillboardNode::UpdateWorldCamera(float fAppTime)
{
	U2Spatial::UpdateWorldData(fAppTime);

	if(m_spCamera)
	{
		D3DXVECTOR3 loc = m_tmWorld.ApplyInverse(m_spCamera->GetEye());


		// To align the billboard, the projection of the camera to the
		// xz-plane of the billboard's model space determines the angle of
		// rotation about the billboard's model y-axis.  If the projected
		// camera is on the model axis (x = 0 and z = 0), ATan2 returns zero
		// (rather than NaN), so there is no need to trap this degenerate
		// case and handle it separately.
		float fAngle = U2Math::ATan2(loc.x, loc.z);
		D3DXMATRIXA16 orient;
		D3DXMatrixRotationAxis(&orient, &UNIT_Y, fAngle);
		m_tmWorld.SetRot(m_tmWorld.GetRot() * orient);
	}
	U2Spatial *pChild;

	for(uint32 i=0; i < m_childArray.Size();++i)
	{
		pChild = m_childArray.GetElem(i);
		if(pChild)
		{
			pChild->Update(fAppTime, false);
		}		
	}
}