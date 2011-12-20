#include <U2_3D/src/U23DLibPCH.h>
#include "U2Effect.h"


IMPLEMENT_RTTI(U2Effect, U2Object);

U2Effect::U2Effect()
{

}

U2Effect::~U2Effect()
{

}

void U2Effect::Render(U2Dx9Renderer* pRenderer, U2Spatial* pGlobalObj, 
					int iStart, int iEnd, U2VisibleObject* pVisible)
{
	// The default drawing function for global effects.  Essentially, this is
	// a local effect applied to all the visible leaf geometry.
	for(int i= iStart; i <= iEnd; ++i)
	{
		if(pVisible[i].IsDrawable())
		{
			U2Mesh* pMesh = (U2Mesh*)pVisible[i].m_pObj;
			pMesh->AttachEffect(this);
			pMesh->Render(pRenderer);
			pMesh->DetachEffect(this);
		}
	}

}