/**************************************************************************************************
module	:	U2DefaultShaderEffect
Author	:	Yun sangyong
Desc	:
/*************************************************************************************************/
#pragma once
#ifndef U2_DEFAULTSHADEREFFECT_H
#define U2_DEFAULTSHADEREFFECT_H

#include "U2ShaderEffect.h"

class U2_3D U2DefaultShaderEffect : public U2ShaderEffect
{
	DECLARE_RTTI;
public:
	U2DefaultShaderEffect();
	virtual ~U2DefaultShaderEffect();


protected:
	

};

typedef U2SmartPtr<U2DefaultShaderEffect> U2DefaultShaderEffectPtr;

#endif
