/************************************************************************
module	:	U2Dx9DynamicShadowCaster
Author	:	Yun sangyong
Desc	:	Light를 받는 mesh로부터 Shadow Volume을 만든다.
Static, Skinned Mesh로 구분하여 각각의 확장 클래스를 생성한다.
************************************************************************/
#pragma once 
#ifndef U2_DX9_DYNAMICSHADOWCASTER_H
#define U2_DX9_DYNAMICSHADOWCASTER_H

#include "U2Dx9ShadowCaster.h"

class U2_3D U2Dx9DynamicShadowCaster : public U2Dx9ShadowCaster
{

public:
	U2Dx9DynamicShadowCaster();
	virtual ~U2Dx9DynamicShadowCaster();

	void SetSkeleton(const U2Skeleton* pSkel);

	virtual void SetupShadowVolume(const U2Light& light, const D3DXMATRIX& invModelLitMat);

	virtual void RenderShadowVolume();

private:

	virtual bool LoadResource();

	virtual void UnloadResource();

	void Update();

	const U2Skeleton* m_pSkeleton;
	bool m_bSkeletonDirty;		 // only need to update skinned mesh when skeleton dirty
	U2N2MeshPtr m_spBindPoseMesh;		 // the bind pose shadow mesh, only read by CPU
	U2N2MeshPtr m_spSkinnedMesh;		 // the skinned shadow mesh, special extrusion layout, written and rendered
};

U2SmartPointer(U2Dx9DynamicShadowCaster);


inline void U2Dx9DynamicShadowCaster::SetSkeleton(const U2Skeleton *pSkel)
{
	U2ASSERT(pSkel);
	m_pSkeleton = pSkel;
	m_bSkeletonDirty = true;
}

#endif