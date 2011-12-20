/************************************************************************
module	:	U2WireframeState
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_DX9_WIRERRAMESTATE_H
#define U2_DX9_WIREFRAMESTATE_H

#include "U2RenderState.h"

U2SmartPointer(U2WireframeState);

class U2_3D U2WireframeState : public U2RenderState
{
	DECLARE_RTTI;
	DECLARE_INITIALIZE;
	DECLARE_TERMINATE;
	U2DeclareFlags(unsigned short);
public :
	virtual int Type() const { return U2RenderState::WIREFRAME; }

	U2WireframeState();	

	void SetWireFrame(bool bWireFrame);
	bool GetWireFrame() const;

	
private:
	enum 
	{
		WIREFRAME_MASK = 0x0001
	};

};

#include "U2WireFrameState.inl"


#endif