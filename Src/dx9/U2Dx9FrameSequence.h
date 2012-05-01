/************************************************************************
module	:	U2Dx9FrameSequence
Author	:	Yun sangyong
Desc	:	Nebula Device 2 
************************************************************************/
#pragma once 
#ifndef U2_DX9_FRAMESEQUENCE_H
#define U2_DX9_FRAMESEQUENCE_H

#include "U2Dx9FrameShader.h"
#include <U2_3D/Src/Main/U2VariableContext.h>
#include <U2_3D/Src/Main/U2VariableMgr.h>
#include <U2Lib/Src/U2StopWatch.h>

class U2Frame;
class U2FrameSection;
class U2FramePass;
class U2FramePhase;

class U2_3D U2FrameSequence : public U2RefObject
{
public:
	U2FrameSequence();
	virtual ~U2FrameSequence();

	void SetFrame(U2Frame* pFrame);
	U2Frame* GetFrame();

	void SetShaderAlias(const U2DynString& p);
	const U2DynString& GetShaderAlias() const;

	void SetTechnique(const U2DynString& tec);
	const U2DynString& GetTechnique() const;

	unsigned int GetShaderBucketIdx() const;
	void SetFirstLightAlphaEnabled(bool bEnable);
	bool GetFirstLightAlphaEnabled() const;

	void SetShaderUpdateEnabled(bool bUpdate);
	bool GetShaderUpdatesEnabled() const;

	void SetModelViewProjOnlyHint(bool b);
	bool GetModelViewProjOnlyHint() const;

	unsigned int Begin();

	void BeginPass(unsigned int pass);

	void EndPass();

	void End();

	void AddConstantShaderParam(U2FxShaderState::Param eParam, const U2ShaderArg& arg);

	void AddVariableShaderParam(const U2DynString& szVar, U2FxShaderState::Param eParam, 
		const U2ShaderArg& arg);

#ifdef U2_PROFILER
	void SetSection(U2FrameSection* pSection);
	U2FrameSection* GetSection() const;
	void SetPass(U2FramePass* pass);
	U2FramePass* GetPass() const;
	void SetPhase(U2FramePhase* pPhase);
	U2FramePhase* GetPhase() const;
#endif

private:
	friend class U2FramePhase;

	/// validate the sequence object
	void Validate();

	/// update the variable shader parameters
	void UpdateVariableShaderParams();

	U2Frame* m_pOwnerFrame;

	U2VariableContext m_varContext;

	U2DynString m_szShaderAlias;
	U2ShaderAttribute m_shaderAttb;
	U2DynString m_szTechnique;
	unsigned int m_uiFrameShaderIdx;
	bool m_bFirstLightAlphaEnabled;
	bool m_bShaderUpdatesEnabled;
	bool m_bOnlyModelViewProj;			// true if sequence shader only required the ModelViewProjection matrix};

#ifdef U2_PROFILER
	U2FrameSection* m_pSection;
	U2FramePass* m_pPass;
	U2FramePhase* m_pPhase;
#endif

};

typedef U2SmartPtr<U2FrameSequence> U2FrameSequencePtr;

#include "U2Dx9FrameSequence.inl"

#endif