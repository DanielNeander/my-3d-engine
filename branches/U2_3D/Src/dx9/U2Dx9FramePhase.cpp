#include <U2_3D/Src/U23DLibPCH.h>
#include "U2Dx9FramePhase.h"

U2FramePhase::U2FramePhase()
:m_bFrameInBegin(false),
m_uiFrameShaderIdx((unsigned int)-1),
m_eSortOrder(FRONT_TO_BACK),
m_eLightMode(LM_OFF),
m_pOwnerFrame(0)
{

}

//-------------------------------------------------------------------------------------------------
U2FramePhase::~U2FramePhase()
{

}

//-------------------------------------------------------------------------------------------------
unsigned int U2FramePhase::Begin()
{
	U2ASSERT(!m_bFrameInBegin);
	U2ASSERT((unsigned int)-1 != m_uiFrameShaderIdx);
	U2ASSERT(m_pOwnerFrame);

	U2Dx9Renderer *pRenderer = U2Dx9Renderer::GetRenderer();

	// reset scissor rect
	static const U2Rect<float> fullscreenRect(0.0f, 0.0f, 1.0f, 1.0f);
	pRenderer->SetScissorRect(fullscreenRect);



	U2D3DXEffectShader* pRealShader = m_pOwnerFrame->GetShader(m_uiFrameShaderIdx).GetShader();
	if(!m_szTechnique.IsEmpty())
	{
#ifdef UNICODE 
		pRealShader->SetTechnique(ToUnicode(m_szTechnique.Str()));
#else 
		pRealShader->SetTechnique(m_szTechnique.Str());
#endif 
	}	
	pRenderer->SetCurrEffectShader(pRealShader);
	int numShaderPasses = pRealShader->Begin(true);
	U2ASSERT(1 == numShaderPasses); // assum 1-pass phase shader
	pRealShader->BeginPass(0);

	m_bFrameInBegin = true;
	return m_frameSeqs.FilledSize();
}

//-------------------------------------------------------------------------------------------------
void U2FramePhase::End()
{
	if(!m_bFrameInBegin)
	{
		return;
	}

	if((unsigned int)-1 != m_uiFrameShaderIdx)
	{
		U2D3DXEffectShader *pShader = m_pOwnerFrame->GetShader(m_uiFrameShaderIdx).GetShader();
		pShader->EndPass();
		pShader->End();
	}

	m_bFrameInBegin = false;

}

//-------------------------------------------------------------------------------------------------
void U2FramePhase::Validate()
{
	unsigned int i;
	unsigned int numSeq = m_frameSeqs.FilledSize();
	for(i=0; i < numSeq; ++i)
	{
		m_frameSeqs[i]->SetFrame(m_pOwnerFrame);
		m_frameSeqs[i]->Validate();
	}

	if((unsigned int)-1 == m_uiFrameShaderIdx)
	{
		U2ASSERT(!m_szShaderAlias.IsEmpty());
		m_uiFrameShaderIdx = m_pOwnerFrame->FindFrameShaderIdx(m_szShaderAlias);
		if((unsigned int)-1 == m_uiFrameShaderIdx)
		{
			U2ASSERT(false);
			FDebug("U2Dx9FramePass::Validate: couldn't find shader alias '%s'",
				m_szShaderAlias.Str());
			return;
		}
	}
}