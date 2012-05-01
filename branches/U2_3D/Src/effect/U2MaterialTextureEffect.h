/************************************************************************
module	:	U2MaterialTextureEffect
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_MATERIALTEXTUREEFFECT_H
#define U2_MATERIALTEXTUREEFFECT_H

#include "U2ShaderEffect.h"

class U2_3D U2MaterialTextureEffect : public U2ShaderEffect 
{
	DECLARE_RTTI;

public:
	U2MaterialTextureEffect(const U2DynString& basename);
	virtual ~U2MaterialTextureEffect();

protected:
	U2MaterialTextureEffect();
};

typedef U2SmartPtr<U2MaterialTextureEffect> U2MaterialTextureEffectPtr;


#endif 