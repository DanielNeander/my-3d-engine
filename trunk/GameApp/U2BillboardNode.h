/************************************************************************
module	:	U2BillboardNode
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_BILLBOARDNODE_H
#define	U2_BILLBOARDNODE_H

#include "SceneNode.h"

class ShadowMapCamera;

class U2BillboardNode : public SceneNode 
{
	//DECLARE_RTTI;

public:
	U2BillboardNode(ShadowMapCamera *pCam =0);
	virtual ~U2BillboardNode();

	void AllignTo(ShadowMapCamera* pCam);

protected:
	virtual void UpdateData();

	ShadowMapCamera* m_spCamera;

};

//typedef U2SmartPtr<U2BillboardNode> U2BillboardNodePtr;


#endif 