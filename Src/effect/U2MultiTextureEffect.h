/************************************************************************
module	:	U2MultiTextureEffect
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_MULTITEXTUREEFFECT_H
#define U2_MULTITEXTUREEFFECT_H

#include "U2ShaderEffect.h"

class U2_3D U2MultiTextureEffect : public U2ShaderEffect 
{
	DECLARE_RTTI;

public:
	U2MultiTextureEffect(uint32 uTextureCnt);
	virtual ~U2MultiTextureEffect();

	// Selection of the textures to be used by the effect.  The first call
	// should be SetTextureQuantity for the desired number of textures.  For
	// each texture, specify its image with SetImageName.  Texture 0 is used
	// as is (replace mode).  Texture i is blended with texture i-1
	// (for i > 0) according to the modes specified by alpha state i to
	// produce the current colors.  After setting all the texture names and
	// all the blending modes, call Configure() to activate the correct shader
	// program for the current set of textures.
	void SetTextureCnt(uint32 texCnt);
	uint32 GetTextureCnt() const;
	void SetTextureName(int i, const U2DynString& texName);
	const U2DynString& GetTextureName(int i) const;
	void Configure();

protected:
	U2MultiTextureEffect();

	uint32 m_uTextureCnt;
	U2DynString** m_ppTextureNameArray;
};

typedef U2SmartPtr<U2MultiTextureEffect> U2MultiTextureEffectPtr;


#endif 