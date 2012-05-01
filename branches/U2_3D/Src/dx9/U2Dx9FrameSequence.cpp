#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9FrameSequence.h"
#include <U2_3D/Src/main/U2SceneMgr.h>

//-------------------------------------------------------------------------------------------------
U2FrameSequence::U2FrameSequence()
:m_pOwnerFrame(0),
m_uiFrameShaderIdx((unsigned int)-1),
m_bShaderUpdatesEnabled(true),
m_bOnlyModelViewProj(false)
{

}

//-------------------------------------------------------------------------------------------------
U2FrameSequence::~U2FrameSequence()
{

}

//-------------------------------------------------------------------------------------------------
unsigned int U2FrameSequence::GetShaderBucketIdx() const
{
	U2ASSERT((unsigned int)-1 != m_uiFrameShaderIdx);
	U2ASSERT(m_pOwnerFrame);
	return m_pOwnerFrame->GetShader(m_uiFrameShaderIdx).GetBucketIdx();	
}

//-------------------------------------------------------------------------------------------------
void U2FrameSequence::Validate()
{

	if((unsigned int)-1 == m_uiFrameShaderIdx)
	{
		U2ASSERT(!m_szShaderAlias.IsEmpty());
		m_uiFrameShaderIdx = m_pOwnerFrame->FindFrameShaderIdx(m_szShaderAlias);
		if((unsigned int)-1 == m_uiFrameShaderIdx)
		{
			U2ASSERT(false);
			FDebug("U2Dx9FrameSequence::Validate: couldn't find shader alias '%s'",
				m_szShaderAlias.Str());
			return;
		}
	}
}

//-------------------------------------------------------------------------------------------------
unsigned int U2FrameSequence::Begin()
{	
	U2ASSERT((unsigned int)-1 != m_uiFrameShaderIdx);
	U2ASSERT(m_pOwnerFrame);	

	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();
	U2ASSERT(pRenderer);
	
	pRenderer->SetHint(
		U2Dx9Renderer::MODEL_VIEW_PROJ_ONLY, m_bOnlyModelViewProj);
	U2D3DXEffectShader* pRealShader = m_pOwnerFrame->GetShader(m_uiFrameShaderIdx).GetShader();
	if(m_bShaderUpdatesEnabled)
	{
		UpdateVariableShaderParams();
		pRealShader->SetAttribute(m_shaderAttb);
	}

	if(!m_szTechnique.IsEmpty())
	{
		pRealShader->SetTechnique(m_szTechnique.Str());
	}
	return pRealShader->Begin(false);
}

//-------------------------------------------------------------------------------------------------
void U2FrameSequence::BeginPass(unsigned int pass)
{
	U2ASSERT((unsigned int)-1 != m_uiFrameShaderIdx);
	U2ASSERT(pass >= 0)	;
	U2ASSERT(m_pOwnerFrame);

	m_pOwnerFrame->GetShader(m_uiFrameShaderIdx).GetShader()->BeginPass(pass);
}

//-------------------------------------------------------------------------------------------------
void U2FrameSequence::EndPass()
{
	U2ASSERT((unsigned int)-1 != m_uiFrameShaderIdx);	
	U2ASSERT(m_pOwnerFrame);

	m_pOwnerFrame->GetShader(m_uiFrameShaderIdx).GetShader()->EndPass();	
}

//-------------------------------------------------------------------------------------------------
void U2FrameSequence::End()
{
	U2ASSERT((unsigned int)-1 != m_uiFrameShaderIdx);	
	U2ASSERT(m_pOwnerFrame);

	m_pOwnerFrame->GetShader(m_uiFrameShaderIdx).GetShader()->End();

	U2Dx9Renderer* pRenderer = U2Dx9Renderer::GetRenderer();
	U2ASSERT(pRenderer);
	
	pRenderer->SetHint(
		U2Dx9Renderer::MODEL_VIEW_PROJ_ONLY, false);
}

//------------------------------------------------------------------------------
/**
This gathers the current global variable values from the render path
object and updates the shader parameter block with the new values.
*/
void U2FrameSequence::UpdateVariableShaderParams()
{
	unsigned int varIdx;
	unsigned int numVars = m_varContext.GetNumVariables();
	for(varIdx = 0; varIdx < numVars; ++varIdx)
	{
		const U2Variable& varParam = m_varContext.GetVariableAt(varIdx);

		U2FxShaderState::Param eShaderParam = 
			(U2FxShaderState::Param)varParam.GetInt();

		const U2Variable* pVarVal = 
			U2VariableMgr::Instance()->GetGlobalVariable(varParam.GetHandle());
		U2ASSERT(pVarVal);
		U2ShaderArg shaderArg;
		switch(pVarVal->GetType())
		{
		case U2Variable::Int:
			shaderArg.SetInt(pVarVal->GetInt());
			break;
		case U2Variable::Float:
			shaderArg.SetFloat(pVarVal->GetFloat());
			break;
		case U2Variable::Float4:
			shaderArg.SetFloat4(pVarVal->GetFloat4());
			break;
		case U2Variable::Object:
			shaderArg.SetTexture((U2Dx9BaseTexture*)pVarVal->GetObj());
			break;
		case U2Variable::Matrix:
			shaderArg.SetMatrix44(&pVarVal->GetMatrix());
			break;
		case U2Variable::Vector4:
			shaderArg.SetVector4(pVarVal->GetVector4());
			break;

		default:
			U2ASSERT(!_T("U2Dx9FramePass : Invalid shader arg datatype"));
			break;
		}

		// Update the shader parameter
		m_shaderAttb.SetArg(eShaderParam, shaderArg);
	}

}


