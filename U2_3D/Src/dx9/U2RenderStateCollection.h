/************************************************************************
module	:	U2RenderStateCollection
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once 
#ifndef U2_RENDERSTATECOLLECTION_H
#define U2_RENDERSTATECOLLECTION_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2Lib/Src/U2RefObject.h>
#include <U2Lib/Src/U2SmartPtr.h>

#include "U2RenderState.h"

class U2MaterialState;
class U2AlphaState;
class U2ZBufferState;
class U2StencilState;
class U2WireframeState;

class U2_3D U2RenderStateCollection : public U2RefObject
{
public:
	U2RenderStateCollection();
	U2RenderStateCollection(const U2RenderStateCollection&);
	virtual ~U2RenderStateCollection();

	U2RenderStateCollection& operator=(const U2RenderStateCollection& lhs);
	U2RenderStatePtr& operator[](int Type);

	void Reset();
	void SetRenderState(U2RenderState* pRS);

	U2MaterialState* GetMaterial() const;
	U2AlphaState* GetAlpha() const;
	U2StencilState* GetStencil() const;
	U2ZBufferState* GetZBuffer() const;
	U2WireframeState* GetWireframe() const;
private:
	U2RenderStatePtr m_aspRSs[U2RenderState::MAX_TYPES];

};

typedef U2SmartPtr<U2RenderStateCollection> U2RenderStateCollectionPtr;

#endif