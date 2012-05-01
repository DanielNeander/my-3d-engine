#include <U2_3D/src/U23DLibPCH.h>
#include "U2DefaultShaderEffect.h"


IMPLEMENT_RTTI(U2DefaultShaderEffect, U2ShaderEffect);

U2DefaultShaderEffect::U2DefaultShaderEffect()
:U2ShaderEffect(1)

{	
	m_vsArray.SetElem(0, U2_NEW U2VertexShader(_T("v_Default.dx9.wmsp")));
	m_psArray.SetElem(0, U2_NEW U2PixelShader(_T("p_Default.dx9.wmsp")));
}

U2DefaultShaderEffect::~U2DefaultShaderEffect()
{

}