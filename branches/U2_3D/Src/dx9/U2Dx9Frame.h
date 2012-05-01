/************************************************************************
module	:	U2Dx9Frame
Author	:	Yun sangyong
Desc	:	Nebula Device 2 
************************************************************************/
#pragma once 
#ifndef U2_DX9_FRAME_H
#define U2_DX9_FRAME_H

#include "U2Dx9FrameSection.h"
#include "U2FrameRenderTarget.h"
#include "U2FrameXmlParser.h"

class U2Dx9FxShaderEffect;
class U2FrameRenderTarget;

class U2_3D U2Frame : public U2Object
{

public:
	U2Frame();
	virtual ~U2Frame();

	bool OpenXml();

	void CloseXml();

	bool Initialize();
	void Terminate();

	void SetFilename(const U2DynString& szFilename);
	const U2DynString& GetFilename() const;

	void AddFrameSection(U2FrameSection* pFrameSection);
	const U2ObjVec<U2FrameSectionPtr>& GetFrameSections() const;
	U2FrameSection* GetFrameSection(int i) const;

	unsigned int FindFrameSectionIdx(const U2DynString& str) const;

	U2Dx9FxShaderEffect* m_pOwnerEffect;

	void AddShader(U2FrameShader* pShader);

	void AddRenderTarget(U2FrameRenderTarget* pRT);
	unsigned int FindRenderTargetIdx(const U2DynString& str) const;
	U2FrameRenderTargetPtr& GetRenderTarget(unsigned int idx);

	void RecreateScreenQuads();

	const U2ObjVec<U2FrameShaderPtr>& GetShaders() const;
	U2FrameShader& GetShader(unsigned int idx) const;
	U2FrameShader& GetShader(const char* shadername) const;

	unsigned int FindFrameShaderIdx(const U2DynString& szShader) const;
	
	void AddVariable(U2Variable* pVar);
	const U2PrimitiveVec<U2Variable::Handle>& GetVariableHandles() const;

	void UpdateVariable(U2Variable* pVar);

	unsigned int GetSequenceShaderAndIncrement();

	bool IsOpen() const;

	U2Dx9FxShaderEffect *m_pEffect;
		
private:

	void Validate();

	bool m_bIsOpen;
	U2DynString m_szXmlFilename;

	U2RenderTargetPtr m_spRenderTarget;


	U2FrameXmlParser m_xmlParser;

	U2ObjVec<U2FrameSectionPtr> m_frameSections;
	U2ObjVec<U2FrameRenderTargetPtr> m_renderTargets;
	U2ObjVec<U2FrameShaderPtr> m_frameShaders;
	U2PrimitiveVec<U2Variable::Handle> m_varHandles;
	unsigned int m_uiFrameSeqShaderIdx;		
	
};

#include "U2Dx9Frame.inl"

typedef U2SmartPtr<U2Frame> U2FramePtr;

#endif