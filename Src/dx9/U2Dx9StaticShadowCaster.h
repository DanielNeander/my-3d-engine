/************************************************************************
module	:	U2Dx9StaticShadowCaster
Author	:	Yun sangyong
Desc	:	Light를 받는 mesh로부터 Shadow Volume을 만든다.
Static, Skinned Mesh로 구분하여 각각의 확장 클래스를 생성한다.
************************************************************************/
#pragma once 
#ifndef U2_DX9_STATICSHADOWCASTER_H
#define U2_DX9_STATICSHADOWCASTER_H

#include "U2Dx9ShadowCaster.h"

class U2_3D U2Dx9StaticShadowCaster : public U2Dx9ShadowCaster
{

public:
	U2Dx9StaticShadowCaster();
	virtual ~U2Dx9StaticShadowCaster();	

	virtual void SetupShadowVolume(const U2Light& light, const D3DXMATRIX& invModelLitMat);

	virtual void RenderShadowVolume();

	virtual bool LoadResource();

	virtual void UnloadResource();

	inline U2N2Mesh* GetMesh() const { return m_spMesh; }

private:

	

	U2N2MeshPtr m_spMesh;		// contains the shadow mesh data
};

U2SmartPointer(U2Dx9StaticShadowCaster);

#endif 