#include <U2_3D/src/U23DLibPCH.h>
#include "U2RenderStateCollection.h"


U2RenderStateCollection::U2RenderStateCollection()
{
	/*m_aspRSs = U2_NEW U2RenderStatePtr[U2RenderState::MAX_TYPES];
	memset(m_aspRSs, 0, sizeof(m_aspRSs));*/

	Reset();

}

U2RenderStateCollection::~U2RenderStateCollection()
{
	//U2_DELETE [] m_aspRSs;
}


void U2RenderStateCollection::Reset()
{
	m_aspRSs[U2RenderState::ALPHA] =
		U2RenderState::ms_aspDefault[U2RenderState::ALPHA];
	m_aspRSs[U2RenderState::MATERIAL] =
		U2RenderState::ms_aspDefault[U2RenderState::MATERIAL];
	m_aspRSs[U2RenderState::STENCIL] =
		U2RenderState::ms_aspDefault[U2RenderState::STENCIL];
	m_aspRSs[U2RenderState::ZBUFFER] =
		U2RenderState::ms_aspDefault[U2RenderState::ZBUFFER];
	m_aspRSs[U2RenderState::WIREFRAME] =
		U2RenderState::ms_aspDefault[U2RenderState::WIREFRAME];
}


U2RenderStateCollection::U2RenderStateCollection(
	const U2RenderStateCollection& state)
{
	unsigned int i;
	for(i = 0; i < U2RenderState::MAX_TYPES; ++i)
		m_aspRSs[i] = state.m_aspRSs[i];
}

U2RenderStateCollection& U2RenderStateCollection::operator=(
	const U2RenderStateCollection& lhs)
{
	unsigned int i;
	for(i=0; i < U2RenderState::MAX_TYPES; ++i)
		m_aspRSs[i] = lhs.m_aspRSs[i];

	return *this;
}

void U2RenderStateCollection::SetRenderState(U2RenderState* pRS)
{
	if(!pRS)
		return;
	m_aspRSs[pRS->Type()] = pRS;
}

U2RenderStatePtr& U2RenderStateCollection::operator[](int rsType)
{
	return m_aspRSs[rsType];
}

#define U2GetProp(e) (U2RenderState*)m_aspRSs[e]

U2MaterialState* U2RenderStateCollection::GetMaterial() const
{
	return (U2MaterialState*)U2GetProp(U2RenderState::MATERIAL);
}


U2AlphaState* U2RenderStateCollection::GetAlpha() const
{
	return (U2AlphaState*)U2GetProp(U2RenderState::ALPHA);
}


U2StencilState* U2RenderStateCollection::GetStencil() const
{
	return (U2StencilState*)U2GetProp(U2RenderState::STENCIL);
}


U2ZBufferState* U2RenderStateCollection::GetZBuffer() const
{
	return (U2ZBufferState*)U2GetProp(U2RenderState::ZBUFFER);
}

U2WireframeState* U2RenderStateCollection::GetWireframe() const
{
	return (U2WireframeState*)U2GetProp(U2RenderState::WIREFRAME);
}
