#include <U2_3D/Src/U23DLibPCH.h>
#include "U2VolumeFogEffect.h"

IMPLEMENT_RTTI(U2VolumeFogEffect, U2ShaderEffect);

U2VolumeFogEffect::U2VolumeFogEffect(const U2DynString& baseName)
:U2ShaderEffect(1)

{	

	m_vsArray.SetElem(0, U2_NEW U2VertexShader(_T("v_VolumeFog.dx9.wmsp")));
	m_psArray.SetElem(0, U2_NEW U2PixelShader(_T("p_VolumeFog.dx9.wmsp")));
	m_psArray.GetElem(0)->SetTexture(0, baseName);
}

U2VolumeFogEffect::U2VolumeFogEffect()
{

}

U2VolumeFogEffect::~U2VolumeFogEffect()
{

}