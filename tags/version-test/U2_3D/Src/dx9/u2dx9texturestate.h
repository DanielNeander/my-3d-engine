/************************************************************************
module	:	U2Dx9TexturesTate
Author	:	Yun sangyong
Desc	:	Texture�� Filtering, Clamp, Apply Mode�� �����Ѵ�.
************************************************************************/
#pragma once
#ifndef U2_DX9_TEXTURESTATE_H
#define U2_DX9_TEXTURESTATE_H

#include "U2RenderState.h"


class U2_3D U2Dx9TextureState : public U2RenderState
{

public:
	virtual int Type() { return U2RenderState::TEXTURE; }

	

	// �ؽ��� ����.
	
};

#endif 




