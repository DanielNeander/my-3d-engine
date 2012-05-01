/************************************************************************
module	:	U2FrameXmlParser
Author	:	Yun sangyong
Desc	:	Nebula Device 2 						
************************************************************************/
#pragma once 
#ifndef U2_FRAMEXMLPARSER_H
#define U2_FRAMEXMLPARSER_H

#include <U2Lib/Src/U2TString.h>

#include <U2_3D/Src/Main/U2Variable.h>
#include <U2_3D/Src/Shader/U2FxShaderParam.h>

#include "tinyxml/tinyxml.h"


class U2_3D U2FrameXmlParser : public U2RefObject
{
public:
	U2FrameXmlParser();

	~U2FrameXmlParser();

	void	SetFrame(U2Frame* pFrame);
	U2Frame* GetFrame();

	void SetFilename(const U2DynString& szFilename);
	const U2DynString& GetFilename() const;


	bool OpenXml();

	void CloseXml();

	bool Parse();

private:
	/// return true if XML attribute exists
	bool HasAttr(TiXmlElement* elm, const char* name);
	/// get an integer attribute from an xml element
	int GetIntAttr(TiXmlElement* elm, const char* name, int defaultValue);
	/// get a float attribute from an xml element
	float GetFloatAttr(TiXmlElement* elm, const char* name, float defaultValue);
	/// get a vector4 attribute from an xml element
	D3DXVECTOR4 GetVector4Attr(TiXmlElement* elm, const char* name, const D3DXVECTOR4& defaultValue);
	/// get a float4 attribute from an xml element
	U2Float4 GetFloat4Attr(TiXmlElement* elm, const char* name, const U2Float4& defaultValue);
	/// get a bool attribute from an xml element
	bool GetBoolAttr(TiXmlElement* elm, const char* name, bool defaultValue);
	/// parse a shader XML element
	void ParseShader(TiXmlElement* elm, U2Frame* renderPath);
	///
	void ParseShaders();
	/// parse RenderTarget XML element
	void ParseRenderTarget(TiXmlElement* elm, U2Frame* renderPath);
	/// create a variable from an XML element
	U2Variable* ParseVariable(U2Variable::Type dataType, TiXmlElement* elm);
	/// parse a global variable under RenderPath
	void ParseGlobalVariable(U2Variable::Type dataType, TiXmlElement* elm, U2Frame* renderPath);
	/// parse a Section XML element
	void ParseFrameSection(TiXmlElement* elm, U2Frame* renderPath);
	/// parse a Pass XML element
	void ParseFramePass(TiXmlElement* elm, U2FrameSection* section);
	/// parse a shader state inside a pass
	void ParseShaderState(U2FxShaderState::Type type, TiXmlElement* elm, U2FramePass* pass);
	/// parse a shader state inside a pass
	void ParseShaderState(U2FxShaderState::Type type, TiXmlElement* elm, U2FrameSequence* seq);
	/// parse a phase
	void ParseFramePhase(TiXmlElement* elm, U2FramePass* pass);
	/// parse a sequence
	void ParseFrameSequence(TiXmlElement* elm, U2FramePhase* phase);



	TiXmlDocument* m_pXmlDoc;
	U2Frame* m_pOwnerFrame;
	U2DynString m_szFullPath;
};

#include "U2FrameXmlParser.inl"

#endif
