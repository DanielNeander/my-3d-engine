/************************************************************************
module	:	U2ReflectionCameraNode
Author	:	Yun sangyong 
Date	:	2010-07-11
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_REFLECTION_CAMERANODE_H
#define	U2_REFLECTION_CAMERANODE_H

#include "U2CameraNode.h"

class U2ReflectionCameraNode : public U2CameraNode
{
	DECLARE_RTTI;
public:
	U2ReflectionCameraNode(U2Camera* pCamera = 0);
	//virtual ~U2ReflectionCameraNode();

private:
	virtual void UpdateWorldData(float fAppTime);

};

typedef U2SmartPtr<U2ReflectionCameraNode> U2ReflectionCameraNodePtr;

#endif