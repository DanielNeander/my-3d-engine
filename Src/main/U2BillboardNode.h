/************************************************************************
module	:	U2BillboardNode
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_BILLBOARDNODE_H
#define	U2_BILLBOARDNODE_H

#include "U2Node.h"


class U2_3D U2BillboardNode : public U2Node 
{
	DECLARE_RTTI;

public:
	U2BillboardNode(U2Camera *pCam =0);
	virtual ~U2BillboardNode();

	void AllignTo(U2Camera* pCam);

protected:
	virtual void UpdateWorldCamera(float fAppTime);

	U2CameraPtr m_spCamera;

};

typedef U2SmartPtr<U2BillboardNode> U2BillboardNodePtr;


#endif 