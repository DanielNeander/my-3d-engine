/************************************************************************
module	:	U2TextureEffect
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_VOLUMEFOGEEFFECT_H
#define U2_VOLUMEFOGEFFECT_H

#include "U2ShaderEffect.h"

class U2_3D U2VolumeFogEffect : public U2ShaderEffect 
{
	DECLARE_RTTI;

public:
	U2VolumeFogEffect(const U2DynString& basename);
	virtual ~U2VolumeFogEffect();

protected:
	U2VolumeFogEffect();
};

typedef U2SmartPtr<U2VolumeFogEffect> U2VolumeFogEffectPtr;


#endif 