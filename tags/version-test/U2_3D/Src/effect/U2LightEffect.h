/************************************************************************
module	:	U2LightEffect
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_LIGHTEFFECT_H
#define U2_LIGHTEFFECT_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2_3D/Src/main/U2TPointerList.h>
#include <U2_3D/src/Dx9/U2Light.h>
#include "U2ShaderEffect.h"


class U2_3D U2LightEffect : public U2ShaderEffect 
{

public:
	U2LightEffect();
	virtual ~U2LightEffect();

	uint32 GetLightCnt() const;
	U2Light* GetLight(uint32 idx) const;
	void AttachLight(U2Light* pLight);
	void DetachLight(U2Light* pLight);
	void DetachAllLights();

	virtual void Configure();

	virtual void SetRenderState(int iPass, U2Dx9RenderStateMgr* pRenderer, 
		bool bPrimaryEffect);
	virtual void RestoreRenderState(int iPAss, U2Dx9RenderStateMgr* pRenderer,
		bool bPrimaryEffect);	

protected:
	D3DXCOLOR m_saveEmissive;
	U2ObjVec<U2LightPtr> m_lights;
};

typedef U2SmartPtr<U2LightEffect> U2LightEffectPtr;



#endif