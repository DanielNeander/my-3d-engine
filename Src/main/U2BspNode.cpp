#include <U2_3D/Src/U23DLibPCH.h>
#include "U2BspNode.h"


IMPLEMENT_RTTI(U2BspNode, U2Node);

U2BspNode::U2BspNode()
	:m_worldPlane(VECTOR3_ZERO, 0.f)
{

}


U2BspNode::U2BspNode(const U2Plane& localPlane)
	:m_localPlane(localPlane)
{

}


U2BspNode::~U2BspNode()
{

}


U2SpatialPtr U2BspNode::AttachFrontChild(U2Spatial *pChild)
{
	return SetChild(0, pChild);
}


U2SpatialPtr U2BspNode::AttachCoplanarChild(U2Spatial* pChild)
{
	return SetChild(1, pChild);
}


U2SpatialPtr U2BspNode::AttachBackChild(U2Spatial* pChild)
{
	return SetChild(2, pChild);
}


U2SpatialPtr U2BspNode::DetachFrontChild()
{
	return DetachChild(0);
}


U2SpatialPtr U2BspNode::DetachCoplanarChild()
{
	return DetachChild(1);
}


U2SpatialPtr U2BspNode::DetachBackChild()
{
	return DetachChild(2);
}

U2SpatialPtr U2BspNode::GetFrontChild()
{
	return GetChild(0);
}


U2SpatialPtr U2BspNode::GetCoplanraChild()
{
	return GetChild(1);
}


U2SpatialPtr U2BspNode::GetBackChild()
{
	return GetChild(2);
}

void U2BspNode::UpdateWorldData(float fAppTime)
{
	U2Node::UpdateWorldData(fAppTime);
	
	
	m_worldPlane = m_tmWorld.Apply(m_localPlane);
}

void U2BspNode::GetVisibleSet(U2Culler& culler, bool bNoCull)
{
	uint32 i;
	for(i=0; i < (int)m_aspEffects.Size(); ++i)
	{		
		culler.Insert(this, m_aspEffects.GetElem(i));
	}

	U2SpatialPtr spFront = GetFrontChild();
	U2SpatialPtr spCoplanar = GetCoplanraChild();
	U2SpatialPtr spBack = GetBackChild();

	const U2Camera* pCamera = culler.GetCamera();
	int iLocSide = m_worldPlane.WhichSide(pCamera->GetEye());
	int iFruSide = culler.WhichSide(m_worldPlane);

	if(iLocSide > 0)
	{
		// camera origin on positive side of plane
		if(iFruSide <=0)
		{
			// The frustum is on the negative side of the plane or straddles
			// the plane.  In either case, the negative child is potentially
			// visible.
			if(spBack)
			{
				spBack->GetVisibleSet(culler, bNoCull)				;
			}
		}

		if(iFruSide == 0)
		{			
			// The frustum straddles the plane.  The coplanar child is
			// potentially visible.
			if(spCoplanar)
			{
				spCoplanar->GetVisibleSet(culler, bNoCull);
			}
		}

		if(iFruSide >= 0)
		{
			// The frustum is on the positive side of the plane or straddles
			// the plane.  In either case, the positive child is potentially
			// visible.
			if(spFront)
			{
				spFront->GetVisibleSet(culler, bNoCull);
			}
		}
	}
	else if(iLocSide < 0)
	{
		if(iFruSide >= 0)
		{
			// The frustum is on the positive side of the plane or straddles
			// the plane.  In either case, the positive child is potentially
			// visible.
			if(spFront)
			{
				spFront->GetVisibleSet(culler, bNoCull);
			}
		}
		

		if(iFruSide == 0)
		{			
			// The frustum straddles the plane.  The coplanar child is
			// potentially visible.
			if(spCoplanar)
			{
				spCoplanar->GetVisibleSet(culler, bNoCull);
			}
		}	


		// camera origin on positive side of plane
		if(iFruSide <=0)
		{
			// The frustum is on the negative side of the plane or straddles
			// the plane.  In either case, the negative child is potentially
			// visible.
			if(spBack)
			{
				spBack->GetVisibleSet(culler, bNoCull)				;
			}
		}
	}
	else 
	{
		// Camera origin on plane itself.  Both sides of the plane are
		// potentially visible as well as the plane itself.  Select the
		// first-to-be-drawn half space to be the one to which the camera
		// direction points.

		float fNDotD = D3DXVec3Dot(&m_worldPlane.m_vNormal, &pCamera->GetDir());
		if(fNDotD >= 0.f)
		{
			if(spFront)
			{
				spFront->GetVisibleSet(culler, bNoCull);
			}

			if(spCoplanar)
			{
				spCoplanar->GetVisibleSet(culler, bNoCull);
			}

			if(spBack)
			{
				spBack->GetVisibleSet(culler, bNoCull);
			}
		}
		else 
		{
			if(spBack)
			{
				spBack->GetVisibleSet(culler, bNoCull);
			}

			if(spCoplanar)
			{
				spCoplanar->GetVisibleSet(culler, bNoCull);
			}

			if(spFront)
			{
				spFront->GetVisibleSet(culler, bNoCull);
			}

		}
	}

	for(i=0; i < m_aspEffects.Size(); ++i)
	{
		culler.Insert(0, 0);
	}
}


U2Spatial* U2BspNode::GetContainingNode(const D3DXVECTOR3& point)
{
	U2SpatialPtr spFront = GetFrontChild();
	U2SpatialPtr spBack = GetBackChild();

	if(spFront || spBack)
	{
		U2BspNode* pBspChild;

		if(m_worldPlane.WhichSide(point) < 0)
		{
			pBspChild = DynamicCast<U2BspNode>(spBack);
			if(pBspChild)
			{
				return pBspChild->GetContainingNode(point);
			}
			else 
			{
				return spBack;
			}
		}
		else 
		{
			pBspChild = DynamicCast<U2BspNode>(spFront);
			
			if(pBspChild)
			{
				return pBspChild->GetContainingNode(point);
			}
			else 
			{
				return spFront;
			}
		}
	}

	return this;
}