/************************************************************************
module	:	U2CameraNode
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_CAMERANODE_H
#define	U2_CAMERANODE_H

#include "U2Camera.h"
#include "U2Node.h"

class U2_3D U2CameraNode : public U2Node 
{
	DECLARE_RTTI;
public:
	U2CameraNode(U2Camera* pCamera = 0);
	virtual ~U2CameraNode();

	void SetCamera(U2Camera* pCamera);

	inline U2Camera* GetCamera() { return m_spCamera; }
	inline const U2Camera* GetCamera() const { return m_spCamera; }

protected:
	virtual void UpdateWorldData(float fAppTime);

	U2CameraPtr m_spCamera;

};

typedef U2SmartPtr<U2CameraNode> U2CameraNodePtr;



#endif
