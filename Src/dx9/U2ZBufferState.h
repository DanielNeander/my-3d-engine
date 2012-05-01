/************************************************************************
module	:	U2ZBufferState
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_DX9_ZBufferState_H
#define U2_DX9_ZBufferState_H

#include "U2RenderState.h"


class U2_3D U2ZBufferState : public U2RenderState
{
	DECLARE_RTTI;
	DECLARE_INITIALIZE;
	DECLARE_TERMINATE;
public :
	virtual int Type() const { return U2RenderState::ZBUFFER; }

	U2ZBufferState();
	virtual ~U2ZBufferState();

	enum CompareFunc
	{
		CF_NEVER,
		CF_LESS,
		CF_EQUAL,
		CF_LESSEQUAL,
		CF_GREATER,
		CF_NOTEQUAL,
		CF_GREATEREQUAL,
		CF_ALWAYS,
		MAX_CFS
	};

	void	SetZEnable(bool bZEnable);
	bool	GetZEnable() const;
	void	SetZWritable(bool bZWritable);
	bool	GetZWritable() const;
	void	SetZTestFunc(CompareFunc eFunc);
	CompareFunc GetZTestFunc() const;
	

private:
	bool	m_bZEnabled;
	bool	m_bZWritable;
	CompareFunc m_eCompareFunc;

};

typedef U2SmartPtr<U2ZBufferState> U2ZBufferStatePtr;



#endif