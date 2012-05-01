/************************************************************************
module	:	U2VertexColor3Effect
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_VERTEXCOLOR3EFFECT_H
#define U2_VERTEXCOLOR3EFFECT_H

#include "U2ShaderEffect.h"

class U2_3D U2VertexColor3Effect : public U2ShaderEffect 
{
	DECLARE_RTTI;

public:
	U2VertexColor3Effect();	
	virtual ~U2VertexColor3Effect();

protected:
	
};

typedef U2SmartPtr<U2VertexColor3Effect> U2VertexColor3EffectPtr;


#endif 