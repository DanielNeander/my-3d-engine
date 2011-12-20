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
	virtual void UpdateWorldBound ();		// �ٿ�� �ڽ��� Ȯ���Ѵ�.

	
	// Update Frame-persistant Data 
	// Node �ν��Ͻ����� ���� ���� ���� �� �����Ƿ� 
	// Node ���� �����ϸ� �ȵȴ�.
	// �� ������ Nebula Device 2 �������� ��������.
	virtual void UpdateRenderContext(U2RenderContext* pRCxt);
	
protected:
	virtual void UpdateState(U2RenderStateCollectionPtr spRS, U2ObjVec<U2LightPtr>* pLightArray);
	
	void GetVisibleSet (U2Culler& rkCuller, bool bNoCull);	

	U2ObjVec<U2SpatialPtr> m_childArray;
};

typedef U2SmartPtr<U2Node> U2NodePtr;

#endif