#include <U2_3D/Src/U23DLibPCH.h>
#include "U2TextureEffect.h"

IMPLEMENT_RTTI(U2TextureEffect, U2ShaderEffect);

U2TextureEffect::U2TextureEffect(const U2DynString& baseName)
	:U2ShaderEffect(1)

{	

	m_vsArray.SetElem(0, U2_NEW U2VertexShader(_T("v_Texture.dx9.wmsp")));
	m_psArray.SetElem(0, U2_NEW U2PixelShader(_T("p_Texture.dx9.wmsp")));
	m_psArray.GetElem(0)->SetTexture(0, baseName);
}

U2TextureEffect::U2TextureEffect()
{

}

U2TextureEffect::~U2TextureEffect()
{

}