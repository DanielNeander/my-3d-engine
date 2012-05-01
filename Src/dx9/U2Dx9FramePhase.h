/************************************************************************
module	:	U2Dx9FramePhase
Author	:	Yun sangyong
Desc	:	Nebula Device 2 
			Begin ¹Ì±¸Çö
************************************************************************/
#pragma once 
#ifndef U2_DX9_FRAMEPHASE_H
#define U2_DX9_FRAMEPHASE_H

#include <U2_3D/src/main/U2Object.h>
#include <U2Lib/Src/U2TString.h>
#include "U2Dx9FrameSequence.h"
#include "U2Dx9FrameShader.h"
#include <U2Lib/Src/U2StopWatch.h>

class U2Frame;
class U2FrameSection;
class U2FramePass;
U2SmartPointer(U2FrameSequence);

class U2_3D U2FramePhase : public U2Object
{
public:
	enum SortingOrder
	{
		NONE,
		FRONT_TO_BACK,
		BACK_TO_FRONT,
	};

	enum LightMode
	{
		LM_OFF,
		LM_FIXED_FUNC,
		LM_SHADER,
	};

	U2FramePhase();
	~U2FramePhase();

	void	SetFrame(U2Frame* pFrame);
	U2Frame* GetFrame();

	void SetShaderAlias(const U2DynString& p);
	const U2DynString& GetShaderAlias() const;

	void SetTechnique(const U2DynString& tec);
	const U2DynString& GetTechnique() const;

	void SetSortOrder(SortingOrder eSort);
	SortingOrder GetSortOrder() const;

	void SetLightMode(LightMode eMode);
	LightMode GetLightMode() const;

	void AddFrameSeq(U2FrameSequence* pSeq);
	U2FrameSequence* GetFrameSeq(unsigned int idx) const;

	const U2ObjVec<U2FrameSequencePtr>& GetFrameSeqs() const;

	unsigned int Begin();

	void End();

	static SortingOrder StrToSortingOrder(const TCHAR* szStr);
	static LightMode StrToLightMode(const TCHAR* szStr);
	

#ifdef U2_PROFILER
	void SetSection(U2FrameSection* pSection);
	U2FrameSection* GetSection() const;
	void SetPass(U2FramePass* pass);
	U2FramePass* GetPass() const;
	void SetPhase(U2FramePhase* pPhase);
	U2FramePhase* GetPhase() const;
#endif

private:
	friend class U2FramePass;

	void Validate();

	U2Frame* m_pOwnerFrame;
	bool m_bFrameInBegin;
	U2DynString m_szShaderAlias;
	U2DynString m_szTechnique;
	unsigned int m_uiFrameShaderIdx;
	SortingOrder m_eSortOrder;
	LightMode m_eLightMode;
	U2ObjVec<U2FrameSequencePtr> m_frameSeqs;

#ifdef U2_PROFILER
	U2FrameSection* m_pSection;
	U2FramePass* m_pPass;
	U2FramePhase* m_pPhase;
#endif
	
};

typedef U2SmartPtr<U2FramePhase> U2FramePhasePtr;

#include "U2Dx9FramePhase.inl"


#endif