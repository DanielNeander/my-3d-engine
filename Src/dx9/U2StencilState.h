/************************************************************************
module	:	U2StencilState
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once 
#ifndef U2_DX9_STENCILSTATE_H
#define U2_DX9_STENCILSTATE_H

#include "U2RenderState.h"

class U2_3D U2StencilState : public U2RenderState
{
	DECLARE_RTTI;
	DECLARE_INITIALIZE;
	DECLARE_TERMINATE;
public:
	U2StencilState();
	virtual ~U2StencilState();
	virtual int Type() const { return U2RenderState::STENCIL; }

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
		CF_QUANTITY
	};

	enum OperationType
	{
		OP_KEEP,
		OP_ZERO,
		OP_REPLACE,
		OP_INCREMENT,
		OP_DECREMENT,
		OP_INVERT,
		MAX_OP_MODE
	};

	enum CullMode
	{
		CM_CCW,  // front faces are counterclockwise ordered
		CM_CW,   // front faces are clockwise ordered
		CM_BOTH, 
		CM_QUANTITY
	};

	
	
	void	SetStencilFlag(bool bOn);
	bool	GetStencilFlag() const;

	void	SetStencilFunc(CompareFunc eFun);
	CompareFunc GetStencilFunc() const;

	void	SetStencilReference(UINT uRef);
	uint32	GetStencilReference() const;

	void	SetStencilMask(UINT32 uMask);
	uint32	GetStencilMask() const;

	void	SetStencilFailOp(OperationType eOpt);
	OperationType GetStencilFailOp() const;

	void	SetStencilZFailOp(OperationType eOpt);
	OperationType GetStencilZFailOp() const;

	void	SetStencilZPassaOp(OperationType eOpt);
	OperationType GetStencilZPassOp() const;
	
	void	SetCullMode(CullMode eCull);
	CullMode GetCullMode() const;


private:
	bool	m_bEnabled;
	CompareFunc m_eCompareFunc;
	uint32 m_uRefVal;
	uint32 m_uMaskVal;
	uint32 m_uWriteMaskVal;
	OperationType m_eFail;		// ���ٽ� �׽�Ʈ�� �������� ��
	OperationType m_eZFail;		// ���ٽ� �׽�Ʈ�� ��������� Z �׽�Ʈ�� �������� ��
	OperationType m_eZPass;		// ���ٽǰ� Z �׽�Ʈ ���
	
	CullMode	m_eCullMode;

	
};



#endif 