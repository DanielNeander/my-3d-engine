/************************************************************************
module	:	U2Dx9FrameSection
Author	:	Yun sangyong
Desc	:	Nebula Device 2 
************************************************************************/
#pragma once 
#ifndef U2_DX9_FRAMESECTION_H
#define U2_DX9_FRAMESECTION_H

#include "U2Dx9FramePass.h"

class U2Frame;

class U2_3D U2FrameSection : public U2Object
{
public:
	U2FrameSection();
	virtual ~U2FrameSection();

	void	SetFrame(U2Frame* pFrame);
	U2Frame* GetFrame() const;

	void AddFramePass(U2FramePass* pPass);
	const U2ObjVec<U2FramePassPtr>& GetFramePasses() const;

	U2FramePass& GetPass(unsigned idx) const;
	unsigned int Begin();	

	void End();

	bool FrameInBegin() const;

	void RecreateScreenQuads();


private:
	void Validate();

	friend class U2Frame;

	U2Frame* m_pOwnerFrame;
	bool m_bFrameInBegin;

	U2ObjVec<U2FramePassPtr> m_framePasses;
};

#include "U2Dx9FrameSection.inl"

typedef U2SmartPtr<U2FrameSection> U2FrameSectionPtr;

#endif

