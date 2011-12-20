/************************************************************************
module	:	U2TextureEffect
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_TEXTUREEFFECT_H
#define U2_TEXTUREEFFECT_H

#include "U2ShaderEffect.h"

class U2_3D U2TextureEffect : public U2ShaderEffect 
{
	DECLARE_RTTI;

public:
	U2TextureEffect(const U2DynString& basename);
	virtual ~U2TextureEffect();

protected:
	U2TextureEffect();
};

typedef U2SmartPtr<U2TextureEffect> U2TextureEffectPtr;


#endif 