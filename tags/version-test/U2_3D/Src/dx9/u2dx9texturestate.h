/************************************************************************
module	:	U2Dx9TexturesTate
Author	:	Yun sangyong
Desc	:	Texture의 Filtering, Clamp, Apply Mode를 설정한다.
************************************************************************/
#pragma once
#ifndef U2_DX9_TEXTURESTATE_H
#define U2_DX9_TEXTURESTATE_H

#include "U2RenderState.h"


class U2_3D U2Dx9TextureState : public U2RenderState
{

public:
	virtual int Type() { return U2RenderState::TEXTURE; }

	

	// 텍스쳐 종류.
	
};

#endif 




