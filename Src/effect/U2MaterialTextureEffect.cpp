#include <U2_3D/Src/U23DLibPCH.h>
#include "U2MaterialTextureEffect.h"

IMPLEMENT_RTTI(U2MaterialTextureEffect, U2ShaderEffect);

U2MaterialTextureEffect::U2MaterialTextureEffect(const U2DynString& baseName)
:U2ShaderEffect(1)

{	

	m_vsArray.SetElem(0, U2_NEW U2VertexShader(_T("v_MaterialTexture.dx9.wmsp")));
	m_psArray.SetElem(0, U2_NEW U2PixelShader(_T("p_MaterialTexture.dx9.wmsp")));
	m_psArray.GetElem(0)->SetTexture(0, baseName);
}

U2MaterialTextureEffect::U2MaterialTextureEffect()
{

}

U2MaterialTextureEffect::~U2MaterialTextureEffect()
{

}