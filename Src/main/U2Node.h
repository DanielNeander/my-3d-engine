/************************************************************************
module	:	U2Node
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_NODE_H
#define	U2_NODE_H

#include <U2Lib/Src/U2TVec.h>
#include "U2Spatial.h"

U2SmartPointer(U2Node);

class U2_3D U2Node : public U2Spatial
{
	DECLARE_RTTI;
public:
	U2Node();
	virtual ~U2Node();

	uint32 GetChildCnt() const;

	virtual void	AttachChild(U2Spatial* pChildNode);
	U2SpatialPtr	DetachChild(U2Spatial* pChildNode);
	U2SpatialPtr	DetachChild(int i);
	U2SpatialPtr	SetChild(unsigned i, U2Spatial* pNewChild);
	U2SpatialPtr	GetChild(uint32 idx);
	
	virtual void UpdateWorldData(float fTime);
	virtual void UpdateWorldBound ();		// 바운딩 박스를 확장한다.

	
	// Update Frame-persistant Data 
	// Node 인스턴스마다 따른 값을 가질 수 있으므로 
	// Node 내에 정의하면 안된다.
	// 이 개념은 Nebula Device 2 엔진에서 빌려왔음.
	virtual void UpdateRenderContext(U2RenderContext* pRCxt);
	
protected:
	virtual void UpdateState(U2RenderStateCollectionPtr spRS, U2ObjVec<U2LightPtr>* pLightArray);
	
	void GetVisibleSet (U2Culler& rkCuller, bool bNoCull);	

	U2ObjVec<U2SpatialPtr> m_childArray;
};

typedef U2SmartPtr<U2Node> U2NodePtr;

#endif