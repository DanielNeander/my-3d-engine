/************************************************************************
module	:	U2Effect
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_EFFECT_H
#define U2_EFFECT_H


#include <U2_3D/src/Main/U2Object.h>

class U2Dx9Renderer;
class U2Mesh;
class U2Spatial;
class U2VisibleObject;

class U2_3D U2Effect : public U2Object 
{
	DECLARE_RTTI;
public:
	virtual ~U2Effect();

	virtual void Render(U2Dx9Renderer* pRenderer, U2Spatial* pGlobalObj, 
		int iStart, int iEnd, U2VisibleObject* pVisible);

	virtual void LoadResoures(U2Dx9Renderer* pRenderer, U2Mesh* pMesh) = 0;
	virtual void ReleaseResources(U2Dx9Renderer* pRenderer, U2Mesh* pMesh) = 0;
	
	virtual int  Begin() = 0;
	virtual void End() = 0;
	virtual void BeginEffect(int pass, U2Dx9Renderer* pRenerer, bool bPrimary) = 0;
	virtual void SetupStage(int pass, U2Dx9Renderer* pRenerer) = 0;
	virtual void Commit(int pass, U2Dx9Renderer* pRenerer) = 0;
	virtual void EndEffect(int pass, U2Dx9Renderer* pRenerer, bool bPrimary) = 0;


protected:
	U2Effect();

};

typedef U2SmartPtr<U2Effect> U2EffectPtr;

#endif
