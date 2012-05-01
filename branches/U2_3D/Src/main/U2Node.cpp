#include <U2_3D/src/U23DLibPCH.h>
#include "U2Node.h"


IMPLEMENT_RTTI(U2Node, U2Spatial);

U2Node::U2Node()
{

}

U2Node::~U2Node()
{
	for(uint32 i=0; i < m_childArray.Size(); ++i)
		DetachChild(i);	
}

uint32 U2Node::GetChildCnt() const
{
	return m_childArray.Size();
}


void U2Node::UpdateState(U2RenderStateCollectionPtr spRSC, 
						 U2ObjVec<U2LightPtr>* pLightArray)
{
	for(int i = 0; i < m_childArray.Size(); ++i)
	{
		U2Spatial* pChild = m_childArray.GetElem(i);
		if(pChild)
			pChild->UpdateRenderStates(spRSC, pLightArray);
	}

}


void U2Node::UpdateWorldData(float fTime)
{
	U2Spatial::UpdateWorldData(fTime);

	for(unsigned int i = 0; i < m_childArray.Size(); ++i)
	{
		U2Spatial* pChild = m_childArray.GetElem(i);
		if(pChild)
			pChild->Update(fTime, false);
	}
}


// Child를 Attach하면 UpdateWorldBound를 호출해서 Bound 크기를 항상 
// 최신의 상태로 유지하도록 해야 한다.
void U2Node::UpdateWorldBound ()
{
	
	if(!m_bCurrWorldBound)
	{
		bool bFoundfirstBound = false;
		for(uint32 i=0; i < m_childArray.Size(); ++i)
		{
			U2Spatial* pChild = m_childArray.GetElem(i);
			if(pChild)
			{
				if(bFoundfirstBound)
				{
					this->m_spWorldBound->GrowToContain(pChild->m_spWorldBound);
				}				
				else
				{
					// set world bound to first non-null child world bound
					bFoundfirstBound = true;
					this->m_spWorldBound->CopyFrom(pChild->m_spWorldBound);				
				}

				// Merge Aabb
				this->m_bbox.Extend(pChild->m_bbox);		
			}
		}				
	}

	
}


void U2Node::AttachChild(U2Spatial* pChild)
{
	U2ASSERT(pChild); // for debug mode
	if(!pChild) // for release
		return;

	pChild->IncRefCount();
	pChild->AttachParent(this);
	m_childArray.AddElem(pChild);

	U2ASSERT(pChild->GetRefCount() >= 2);
	pChild->DecRefCount();
}


U2SpatialPtr U2Node::DetachChild(U2Spatial* pChild)
{
	for(unsigned int i = 0; i < m_childArray.Size(); ++i)
	{
		U2SpatialPtr spChild = m_childArray.GetElem(i);
		if(spChild && spChild == pChild)
		{
			spChild->DetachParent();
			m_childArray.Remove(i);
			return spChild;
		}
	}
	return 0;
}


U2SpatialPtr U2Node::DetachChild( int i)
{
	if(i < m_childArray.Size())
	{
		U2SpatialPtr spChild = m_childArray.GetElem(i);
		if(spChild)
		{
			spChild->DetachParent();
			m_childArray.Remove(i);
		}
		return spChild;
	}
	else 
		return 0;
}

U2SpatialPtr	U2Node::SetChild(unsigned i, U2Spatial* pNewChild)
{
	if(m_childArray.Size() <= i)
	{
		if(pNewChild)
			pNewChild->AttachParent(this);
		m_childArray.SetSafeElem(i, pNewChild);
		return NULL;
	}
	else 
	{
		U2SpatialPtr spPrevChild = m_childArray.GetElem(i);
		if(spPrevChild)
			spPrevChild->DetachParent();
		if(pNewChild)
			pNewChild->AttachParent(this);

		m_childArray.SetElem(i, pNewChild);
		return spPrevChild;
	}
}


U2SpatialPtr	U2Node::GetChild(uint32 idx)
{
	if(0 <=  idx  && idx < m_childArray.Size())
	{
		return m_childArray.GetElem(idx);
	}
	return NULL;
}


void U2Node::GetVisibleSet (U2Culler& rkCuller, bool bNoCull)
{
	uint32 i=0;
	for(uint32 i=0; i < m_aspEffects.Size(); ++i)
	{
		// This is a global effect.  Place a 'begin' marker in the visible
		// set to indicate the effect is active.
		rkCuller.Insert(this, m_aspEffects.GetElem(i));
	}

	// All Geometry objects in the subtree are added to the visible set.  If
	// a global effect is active, the Geometry objects in the subtree will be
	// drawn using it.
	for(i = 0; i < m_childArray.Size(); ++i)
	{
		U2Spatial* pChild = m_childArray.GetElem(i);
		if(pChild)
		{
			pChild->OnVisibleSet(rkCuller, bNoCull);
		}
	}

	for(i = 0;i < m_aspEffects.Size(); ++i)
	{
		// Place an 'end' marker in the visible set to indicate that the
		// global effect is inactive.
		rkCuller.Insert(0, 0);
	}
}


void U2Node::UpdateRenderContext(U2RenderContext* pRCxt)
{
	for(unsigned int i=0; i < GetNumControllers(); ++i)
	{
		if(!GetController(i)->IsInitialize())
			GetController(i)->Initialize(pRCxt);

		GetController(i)->Update(0.f, pRCxt);
	}

	unsigned childIdx;
	for(childIdx = 0; childIdx < m_childArray.FilledSize(); ++childIdx)
	{
		if(NULL != m_childArray[childIdx])
		{
			m_childArray[childIdx]->UpdateRenderContext(pRCxt);
		}
	}
}