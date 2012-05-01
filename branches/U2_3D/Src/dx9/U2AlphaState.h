/************************************************************************
module	:	U2AlphaState
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once 
#ifndef U2_DX9_ALPHASTATE_H
#define U2_DX9_ALPHASTATE_H

#include "U2RenderState.h"
#include <U2_3D/src/U2MainMacro.h>




class U2_3D	U2AlphaState : public U2RenderState
{
	DECLARE_RTTI;
	DECLARE_INITIALIZE;
	DECLARE_TERMINATE;
	
public:
	U2AlphaState();
	virtual ~U2AlphaState();

	enum BlendMode 
	{
		BLEND_ZERO,
		BLEND_ONE,
		BLEND_SRC_COLOR,
		BLEND_ONE_MINUS_SRC_COLOR,		
		BLEND_DEST_COLOR,
		BLEND_ONE_MINUS_DEST_COLOR,
		BLEND_SRC_ALPHA,
		BLEND_ONE_MINUS_SRC_ALPHA,
		BLEND_DEST_ALPHA,
		BLEND_ONE_MINUS_DEST_ALPHA,
		BLEND_SRC_ALPHA_SATURATE,
		//BLEND_CONSTANT_COLOR,
		//BLEND_ONE_MINUS_CONSTANT_COLOR,
		//BLEND_CONSTANT_ALPHA,
		//BLEND_ONE_MINUS_CONSTANT_ALPHA,
		MAX_BLEND_MODES
	};

	enum TestMode
	{
		TEST_ALWAYS,
		TEST_LESS,
		TEST_EQUAL,
		TEST_LESSEQUAL,
		TEST_GREATER,
		TEST_NOTEQUAL,
		TEST_GREATEREQUAL,
		TEST_NEVER,
		MAX_TEST_MODES
	};
	
	void SetAlphaBlending(bool bBlend);
	bool GetAlphaBlending() const;

	void SetSrcBlendMode(BlendMode eSrcBlend);
	BlendMode GetSrcBlendMode() const;

	void SetDestBlendMode(BlendMode eDestBlend);
	BlendMode GetDestBlendMode() const;

	void	SetAlphaTesting(bool bAlpah);
	bool	GetAlphaTesting() const;

	void	SetTestMode(TestMode eTestFunc);
	TestMode GetTestMode() const;

	void	SetTestRef(float fRef);
	float	GetTestRef() const;

	void	SetNoSort(bool bNoSort);
	bool	GetNoSort() const;

	virtual int Type() const { return U2AlphaState::ALPHA; }
	
	

private:	
	bool m_bNoSort;
	bool m_bBlendMode;
	BlendMode m_eSrcBlend;
	BlendMode m_eDestBlend;
	bool m_bTestMode;
	TestMode m_eTestMode;
	float m_fTestRef;	
};

typedef U2SmartPtr<U2AlphaState> U2Dx9AlphaStatePtr;

#endif