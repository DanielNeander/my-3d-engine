#include <U2_3D/Src/U23DLibPCH.h>
#include "U2VertexColor3Effect.h"

IMPLEMENT_RTTI(U2VertexColor3Effect, U2ShaderEffect);

U2VertexColor3Effect::U2VertexColor3Effect()
:U2ShaderEffect(1)

{	

	m_vsArray.SetElem(0, U2_NEW U2VertexShader(_T("v_VertexColor3.dx9.wmsp")));
	m_psArray.SetElem(0, U2_NEW U2PixelShader(_T("p_PassThrough3.dx9.wmsp")));
}


U2VertexColor3Effect::~U2VertexColor3Effect()
{

}