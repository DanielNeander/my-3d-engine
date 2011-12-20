/**************************************************************************************************
module	:	U2ShadowNode
Author	:	Yun sangyong
Date	:	2010/06/11
Desc	:	
*************************************************************************************************/
#pragma once 
#ifndef U2_SHADOWNODE_H
#define U2_SHADOWNODE_H

#include <U2_3D/Src/main/U2Spatial.h>
#include "U2Dx9StaticShadowCaster.h"

class U2_3D U2ShadowNode : public U2Spatial
{
	DECLARE_RTTI;
public:
	U2ShadowNode();
	virtual ~U2ShadowNode();

	virtual bool LoadResources();
	virtual void UnloadResources();

	virtual bool RenderShadowVolume(U2SceneMgr* pSceneMgr, U2RenderContext* pCxt); 

	//virtual bool HasShadow();

	void SetGroupIdx(int i);
	int GetGroupIdx() const;

	// Derived Func
	virtual void GetVisibleSet (U2Culler& rkCuller, bool bNoCull) {}
	virtual void UpdateWorldBound() 
	{ 
			SetBoundingBox(m_spStaticCaster->GetMesh()->GetBoundingBox());
	}
	virtual void UpdateState(U2RenderStateCollectionPtr spRS , 
		U2ObjVec<U2LightPtr>* pLightArray) {}
	
protected:

	static const float ms_fMaxSmallObjDistance;

	int m_iGroupIdx;
	U2Dx9StaticShadowCasterPtr m_spStaticCaster;
};

//-------------------------------------------------------------------------------------------------
inline void U2ShadowNode::SetGroupIdx(int i)
{
	m_iGroupIdx = i;
}

//-------------------------------------------------------------------------------------------------
inline int U2ShadowNode::GetGroupIdx() const
{
	return m_iGroupIdx;
}

#endif 