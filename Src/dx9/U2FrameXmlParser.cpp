#include <U2_3D/Src/U23DLibPCH.h>
#include "U2Dx9Frame.h"
#include "U2Dx9FrameSection.h"
#include "U2Dx9FramePhase.h"
#include "U2Dx9FramePass.h"
#include "U2Dx9FrameSequence.h"
//#include "U2_3D/Src/Object/U2XmlStream.h"

U2FrameXmlParser::U2FrameXmlParser()
:m_pXmlDoc(NULL)
{

}

U2FrameXmlParser::~U2FrameXmlParser()
{

}

bool U2FrameXmlParser::OpenXml()
{
	U2ASSERT(!m_pXmlDoc);

	// Get FullPath
	m_pXmlDoc = new TiXmlDocument;

	SetFilename(m_pOwnerFrame->GetFilename());

	if(m_pXmlDoc->LoadFile(m_szFullPath.Str()))
	{
		TiXmlHandle docHandle(m_pXmlDoc);
		TiXmlElement* pFrameXmlElem = docHandle.FirstChildElement("Frame").Element();
		U2ASSERT(pFrameXmlElem);
		m_pOwnerFrame->SetName(pFrameXmlElem->Attribute("name"));
		//m_pOwnerFrame->SetShaderPath
		return true;
	}
	delete (m_pXmlDoc);
	m_pXmlDoc = NULL;
	return false;
}

//-------------------------------------------------------------------------------------------------
void U2FrameXmlParser::CloseXml()
{
	delete m_pXmlDoc;
	m_pXmlDoc = NULL;
}

//-------------------------------------------------------------------------------------------------
bool U2FrameXmlParser::Parse()
{
	TiXmlHandle docHandle(m_pXmlDoc);
	TiXmlElement* pFrameElem = docHandle.FirstChildElement("Frame").Element();
	U2ASSERT(pFrameElem);

	

	TiXmlElement* pChild;
	for(pChild = pFrameElem->FirstChildElement(); pChild; pChild = pChild->NextSiblingElement())
	{
#ifdef UNICODE 
		if(ToUnicode(pChild->Value()) == U2DynString(_T("Shaders")))
		{
			ParseShaders();
		}
		else if(ToUnicode(pChild->Value()) == U2DynString(_T("RenderTarget")))
		{
			ParseRenderTarget(pChild, m_pOwnerFrame);
		}
		else if(ToUnicode(pChild->Value()) == U2DynString(_T("Float")))
		{
			ParseGlobalVariable(U2Variable::Float, pChild, m_pOwnerFrame);
		}
		else if(ToUnicode(pChild->Value()) == U2DynString(_T("Float4")))
		{
			ParseGlobalVariable(U2Variable::Vector4, pChild, m_pOwnerFrame);
		}
		else if(ToUnicode(pChild->Value()) == U2DynString(_T("Int")))
		{
			ParseGlobalVariable(U2Variable::Int, pChild, m_pOwnerFrame);
		}
		else if(ToUnicode(pChild->Value()) == U2DynString(_T("Texture")))
		{
			ParseGlobalVariable(U2Variable::Object, pChild, m_pOwnerFrame);
		}
		else if(ToUnicode(pChild->Value()) == U2DynString(_T("RenderTarget")))	
		{
			ParseFrameSection(pChild, m_pOwnerFrame);
		}
#else 
		if(pChild->Value() == U2DynString(_T("Shaders")))

		{
			PROFILER_DECLARE(prof);
			PROFILER_START(prof);

			ParseShaders();

			PROFILER_STOP(prof);
			//FILE_LOG(logINFO) << _T("ParseShaders: ") << prof.GetTimeInSecs();
		}
		else if(pChild->Value() == U2DynString(_T("RenderTarget")))
		{
			PROFILER_DECLARE(prof);
			PROFILER_START(prof);

			ParseRenderTarget(pChild, m_pOwnerFrame);
			
			PROFILER_STOP(prof);
			//FILE_LOG(logINFO) << _T("ParseRenderTarget: ") << prof.GetTimeInSecs();
		}
		else if(pChild->Value() == U2DynString(_T("Float")))

		{
			PROFILER_DECLARE(prof);
			PROFILER_START(prof);
			ParseGlobalVariable(U2Variable::Float, pChild, m_pOwnerFrame);
			PROFILER_STOP(prof);
			//FILE_LOG(logINFO) << _T("ParseGlobalVariable Float: ") << prof.GetTimeInSecs();


		}
		else if(pChild->Value() == U2DynString(_T("Float4")))

		{
			PROFILER_DECLARE(prof);
			PROFILER_START(prof);
			ParseGlobalVariable(U2Variable::Vector4, pChild, m_pOwnerFrame);
			PROFILER_STOP(prof);

			//FILE_LOG(logINFO) << _T("ParseGlobalVariable Float4: ") << prof.GetTimeInSecs();


		}
		else if(pChild->Value() == U2DynString(_T("Int")))

		{
			PROFILER_DECLARE(prof);
			PROFILER_START(prof);
			ParseGlobalVariable(U2Variable::Int, pChild, m_pOwnerFrame);
			PROFILER_STOP(prof);
			//FILE_LOG(logINFO) << _T("ParseGlobalVariable Int: ") << prof.GetTimeInSecs();


		}
		else if(pChild->Value() == U2DynString(_T("Texture")))

		{
			PROFILER_DECLARE(prof);
			PROFILER_START(prof);
			ParseGlobalVariable(U2Variable::Object, pChild, m_pOwnerFrame);
			PROFILER_STOP(prof);
			//FILE_LOG(logINFO) << _T("ParseGlobalVariable Texture: ") << prof.GetTimeInSecs();


		}
		else if(pChild->Value() == U2DynString(_T("FrameSection")))
		{
			PROFILER_DECLARE(prof);
			PROFILER_START(prof);
			// Reset 에러... 2011/09/26 
			ParseFrameSection(pChild, m_pOwnerFrame);
			PROFILER_STOP(prof);
			//FILE_LOG(logINFO) << _T("ParseFrameSection: ") << prof.GetTimeInSecs();


		}
#endif 		
	}
	return true;
}

//-------------------------------------------------------------------------------------------------
void U2FrameXmlParser::ParseShader(TiXmlElement* elem, U2Frame* renderPath)
{
	U2ASSERT(elem && renderPath);
	U2FrameShader *pNewShader = U2_NEW U2FrameShader;
#ifdef UNICODE 
	pNewShader->SetName(ToUnicode(elem->Attribute("name")));
#else 
	pNewShader->SetName(elem->Attribute("name"));
#endif 

#ifdef UNICODE 
	pNewShader->SetFilename(ToUnicode(elem->Attribute("file")));
#else 
	pNewShader->SetFilename(elem->Attribute("name"));
#endif 
	renderPath->AddShader(pNewShader);
}

void U2FrameXmlParser::ParseShaders()
{
	U2XmlStream newXmlStream;
	newXmlStream.SetFilename(m_szFullPath);
	if(!newXmlStream.Open(U2XmlStream::Read))
	{
		FDebug("Failed to open : %s\n", m_szFullPath.Str());
		U2ASSERT(false);
	}
	else 
	{
		newXmlStream.SetToNode(_T("/Frame/Shaders"));
	}

	newXmlStream.SetToFirstChild();

	do 
	{
						
		if(newXmlStream.HasAttr(_T("name")) && newXmlStream.HasAttr(_T("file")))
		{
			PROFILER_DECLARE(prof);
			PROFILER_START(prof);

			U2FrameShader *pNewShader = U2_NEW U2FrameShader;
#ifdef UNICODE 
			pNewShader->SetName(ToUnicode(newXmlStream.GetString("name")));
			pNewShader->SetFilename(ToUnicode(newXmlStream.GetString("file")));
#else 
			pNewShader->SetName(newXmlStream.GetString("name"));
			pNewShader->SetFilename(newXmlStream.GetString("file"));
#endif 
			m_pOwnerFrame->AddShader(pNewShader);			

			//DPrintf("Parsed Shader : %s\n", newXmlStream.GetString("file"));
			

			PROFILER_STOP(prof);
			//FILE_LOG(logINFO) << _T("AddShader: ") << prof.GetTimeInSecs();
		}
		else 
		{
			break;
		}
	} while (newXmlStream.SetToNextChild());
}

//-------------------------------------------------------------------------------------------------
void U2FrameXmlParser::ParseRenderTarget(TiXmlElement* elem, U2Frame* renderPath)
{
	U2ASSERT(elem && renderPath);
	U2FrameRenderTarget* pNewRT = U2_NEW U2FrameRenderTarget;
	pNewRT->SetName(elem->Attribute("name"));
	pNewRT->SetFormat(U2Dx9Texture::StringToPixelFormat(elem->Attribute("format")));
	pNewRT->SetRelativeSize(GetFloatAttr(elem, "relSize", 1.0f));
	if(HasAttr(elem, "width"))
	{
		pNewRT->SetWidth(GetIntAttr(elem, "width", 0));
	}
	if(HasAttr(elem, "height"))
	{
		pNewRT->SetHeight(GetIntAttr(elem, "height", 0));
	}
	renderPath->AddRenderTarget(pNewRT);
}

//-------------------------------------------------------------------------------------------------
U2Variable* U2FrameXmlParser::ParseVariable(U2Variable::Type dataType, TiXmlElement* elem)
{
	U2ASSERT(elem);
	const char* szVar = elem->Attribute("name");
	U2ASSERT(szVar);
	U2Variable::Handle varHandle = U2VariableMgr::Instance()->GetVariableHandleByName(szVar);
	U2Variable* pNewVar = U2_NEW U2Variable(dataType, varHandle);
	switch(dataType)
	{
	case U2Variable::Int:
		pNewVar->SetInt(GetIntAttr(elem, "value", 0));
		break;
	case U2Variable::Float:
		pNewVar->SetFloat(GetFloatAttr(elem, "value", 0.0f));
		break;
	case U2Variable::Vector4:
		{
			D3DXVECTOR4 v(VECTOR4_ZERO);
			pNewVar->SetVector4(GetVector4Attr(elem, "value", v));
		}
		break;
	case U2Variable::Object:
		{
#ifdef UNICODE 
			const wchar_t* szFilename = ToUnicode(elem->Attribute("value"));
#else 
			const char* szFilename = elem->Attribute("value");
#endif 
			U2ASSERT(szFilename);

			U2Dx9BaseTexture* pTexIter = U2Dx9BaseTexture::GetHead();

			U2Dx9BaseTexture* pNexTex = 0;

			while(pTexIter)
			{
				// Create시 SetName을 함..
				if(pTexIter->GetName() == szFilename)
				{
					pNexTex = pTexIter;
					break;
				}

				pTexIter = pTexIter->GetNext();
			}

			if(!pNexTex)
			{
				U2FilePath fPath;				
				TCHAR fullPath[256];
				fPath.ConvertToAbs(fullPath, 256 * sizeof(TCHAR) , szFilename, TEXTURE_PATH);	

				pNexTex = U2Dx9Texture::Create(fullPath, U2Dx9Renderer::GetRenderer());			
			}

			pNewVar->SetObj(pNexTex);			
		}
		break;
	default:
		U2ASSERT(false);
		FDebug("nRpXmlParser::ParseGlobalVariable(): invalid datatype for variable '%s'!",
			szVar);
		
		break;
	}
	return pNewVar;
}

//------------------------------------------------------------------------------
/**
Parse a Float, Vector4, Int or Bool element inside a RenderPath
element (these are global variable definitions.
*/
void U2FrameXmlParser::ParseGlobalVariable(U2Variable::Type dataType, TiXmlElement* elem, 
										   U2Frame* renderPath)
{
	U2ASSERT(elem && renderPath);
	U2Variable* pVar = ParseVariable(dataType, elem);
	renderPath->AddVariable(pVar);
}

//------------------------------------------------------------------------------
/**
Parse a Section element inside a RenderPath element.
*/
void U2FrameXmlParser::ParseFrameSection(TiXmlElement* elem, U2Frame* renderPath)
{
	U2ASSERT(elem && renderPath);

	U2FrameSection* pNewSection = U2_NEW U2FrameSection;
	pNewSection->SetFrame(renderPath);
	
#ifdef UNICODE 
	pNewSection->SetName(ToUnicode(elem->Attribute("name")));
#else 
	pNewSection->SetName(elem->Attribute("name"));
#endif

	TiXmlElement* pChild;
	for(pChild = elem->FirstChildElement(); pChild; pChild = pChild->NextSiblingElement())
	{
#ifdef UNICODE 
		if(ToUnicode(pChild->Value()) == U2DynString("pass"))
#else 
		if(pChild->Value() == U2DynString("Pass"))
#endif
		{
			ParseFramePass(pChild, pNewSection);
		}
	}
	renderPath->AddFrameSection(pNewSection);
}

//------------------------------------------------------------------------------
/**
Parse a Pass element inside a Section element.
*/
void U2FrameXmlParser::ParseFramePass(TiXmlElement* elem, U2FrameSection* section)
{
	U2ASSERT(elem && section);

	U2FramePass* pNewPass = U2_NEW U2FramePass;
	pNewPass->SetFrame(section->GetFrame());

#ifdef UNICODE 
	pNewPass->SetName(ToUnicode(elem->Attribute("name")));
	pNewPass->SetShaderAlias(ToUnicode(elem->Attribute("shader")));
#else 
	pNewPass->SetName(elem->Attribute("name"));
	pNewPass->SetShaderAlias(elem->Attribute("shader"));
#endif 
	U2DynString szRenderTarget(_T("renderTarget"));	
	int i=0;

	unsigned int uClearFlag = 0;
#ifdef UNICODE 
	while(HasAttr(elem, ToAscii(szRenderTarget.Str())))
	{

		//pNewPass->SetRenderTargetName(i, ToUnicode(elem->Attribute(szRenderTarget.Str()));
		szRenderTarget = L("renderTarget");
		szRenderTarget.AppendInt(++i);
	}
#else
	while(HasAttr(elem, szRenderTarget.Str()))
	{
		pNewPass->SetRenderTargetName(i, elem->Attribute(szRenderTarget.Str()));
		szRenderTarget = "renderTarget";
		szRenderTarget.AppendInt(++i);
	}
#endif 

	if(HasAttr(elem, "stats"))
	{
		//pNewPass->SetStatsEnabled(GetBoolAttr(elem, "stats", true));
	}
	int clearFlags = 0;
	if(HasAttr(elem, "clearColor"))
	{
		uClearFlag |= U2Dx9Renderer::CLEAR_BACKBUFFER;		

		D3DXVECTOR4 color = GetVector4Attr(elem, "clearColor", 
			D3DXVECTOR4(0.f, 0.f, 0.f, 1.f));
		pNewPass->SetBackgroundColor(D3DXCOLOR(color.x, color.y, color.z, color.w));
	}

	if(HasAttr(elem, "clearDepth"))
	{
		uClearFlag |= U2Dx9Renderer::CLEAR_ZBUFFER;
		pNewPass->SetDepthClear(GetFloatAttr(elem, "clearDepth", 1.0f));
	}
	if(HasAttr(elem, "clearStencil"))
	{
		uClearFlag |= U2Dx9Renderer::CLEAR_STENCIL;
		pNewPass->SetStencilClear(GetIntAttr(elem, "clearStencil", 0));
	}

	pNewPass->SetClearFlags(uClearFlag);
	
	if(HasAttr(elem, "drawQuad"))
	{
		pNewPass->SetDrawFullscreenQuad(GetBoolAttr(elem, "drawQuad", false));
		// CreateQuad 
		if (pNewPass->GetDrawFullscreenQuad())
			pNewPass->CreateScreenQuad();

	}
	if(HasAttr(elem, "drawShadows"))
	{
#ifdef UNICODE 
		pNewPass->SetShadowTechnique(ToUnicode(U2FramePass::StringToShadowTechnique(elem->Attribute("drawShadows"))));
#else 
pNewPass->SetShadowTechnique(U2FramePass::StringToShadowTechnique(elem->Attribute("drawShadows")));
#endif

	}
	if(HasAttr(elem, "occlusionQuery"))
	{
		pNewPass->SetOcclusionQuery(GetBoolAttr(elem, "occlusionQuery", false));
	}
	if(HasAttr(elem, "drawGui"))
	{
		pNewPass->SetDrawGui(GetBoolAttr(elem, "drawGui", false));

	}
	if(HasAttr(elem, "technique"))
	{
#ifdef UNICODE 
		pNewPass->SetTechnique(ToUnicode(elem->Attribute("technique")));
#else 
		pNewPass->SetTechnique(elem->Attribute("technique"));
#endif 
	}
	if(HasAttr(elem, "shadowEnabledCondition"))
	{
		pNewPass->SetShadowEnabled(GetBoolAttr(elem, "shadowEnabledCondition", false));
	}

	if(HasAttr(elem, "addDepthStencil"))
	{
		pNewPass->SetDepthStencil(GetBoolAttr(elem, "addDepthStencil", false));
	}


	TiXmlElement* pChild;
	for(pChild = elem->FirstChildElement(); pChild; pChild = pChild->NextSiblingElement())
	{
#ifdef UNICODE 
		if(pChild->Value() == U2DynString(L"Float"))
		{
			ParseShaderState(U2FxShaderState::Float, pChild, pNewPass);
		}
		else if(pChild->Value() == U2DynString(L"Float4"))
		{
			ParseShaderState(U2FxShaderState::Float4, pChild, pNewPass);
		}
		else if(pChild->Value() == U2DynString(L"Int"))
		{
			ParseShaderState(U2FxShaderState::Int, pChild, pNewPass);
		}
		else if(pChild->Value() == U2DynString(L"Texture"))
		{
			ParseShaderState(U2FxShaderState::Texture, pChild, pNewPass);
		}
		else if(pChild->Value() == U2DynString(L"FramePhase"))
		{
			ParseFramePhase(pChild, pNewPass);
		}
#else 
		if(pChild->Value() == U2DynString("Float"))
		{
			ParseShaderState(U2FxShaderState::Float, pChild, pNewPass);
		}
		else if(pChild->Value() == U2DynString("Float4"))
		{
			ParseShaderState(U2FxShaderState::Float4, pChild, pNewPass);
		}
		else if(pChild->Value() == U2DynString("Int"))
		{
			ParseShaderState(U2FxShaderState::Int, pChild, pNewPass);
		}
		else if(pChild->Value() == U2DynString("Texture"))
		{
			ParseShaderState(U2FxShaderState::Texture, pChild, pNewPass);
		}
		else if(pChild->Value() == U2DynString("Phase"))
		{
			ParseFramePhase(pChild, pNewPass);
		}
#endif 
	}
	section->AddFramePass(pNewPass);
}

//------------------------------------------------------------------------------
/**
Parse a shader state element inside a Pass XML element.
*/
void U2FrameXmlParser::ParseShaderState(U2FxShaderState::Type type, TiXmlElement* elem, 
										U2FramePass* pass)
{
	U2ASSERT(elem && pass);

	U2FxShaderState::Param eParam = U2FxShaderState::
		StringToParam(elem->Attribute("name"));
	U2ShaderArg* pArg = U2_NEW U2ShaderArg(type);
	if(HasAttr(elem, "value"))
	{
		switch(type)
		{
		case U2FxShaderState::Int:
			pArg->SetInt(GetIntAttr(elem, "value", 0));
			break;

		case U2FxShaderState::Float:
			pArg->SetFloat(GetFloatAttr(elem, "value", 0.0f));
			break;

		case U2FxShaderState::Float4:
			{
				U2Float4 f4 = {0.f};
				pArg->SetFloat4(GetFloat4Attr(elem, "value", f4));
			}			
			break;
		case U2FxShaderState::Texture:
			{
#ifdef UNICODE 
				const wchar_t* szFilename = ToUnicode(elem->Attribute("value"));
#else 
				const char* szFilename = elem->Attribute("value");
#endif 
				U2ASSERT(szFilename);

				U2Dx9BaseTexture* pTexIter = U2Dx9BaseTexture::GetHead();

				U2Dx9BaseTexture* pNexTex = 0;

				while(pTexIter)
				{
					// Create시 SetName을 함..
					if(pTexIter->GetName() == szFilename)
					{
						pNexTex = pTexIter;
						break;
					}

					pTexIter = pTexIter->GetNext();
				}

				if(!pNexTex)
				{
					U2FilePath fPath;				
					TCHAR fullPath[256];
					fPath.ConvertToAbs(fullPath, 256 * sizeof(TCHAR), szFilename, TEXTURE_PATH);	

					pNexTex = U2Dx9Texture::Create(fullPath, U2Dx9Renderer::GetRenderer());			
				}

				pArg->SetTexture(pNexTex);	
			}
			break;
		default:
			FDebug("nRpXmlParser::ParseShaderState(): invalid datatype '%s'!", 
				elem->Attribute("name"));
			break;	
		}
		pass->AddConstantShaderParam(eParam, *pArg);
	}
	else if(HasAttr(elem, "variable"))
	{
#ifdef UNICODE 
		const WCHAR* szVar =ToUnicode(elem->Attribute("variable"));
#else 
		const char* szVar = elem->Attribute("variable");
#endif
		pass->AddVariableShaderParam(szVar, eParam, *pArg);
		

	}
}

//------------------------------------------------------------------------------
/**
Parse a shader state element inside a Pass XML element.
*/
void U2FrameXmlParser::ParseShaderState(U2FxShaderState::Type type, 
										TiXmlElement* elem, U2FrameSequence* seq)
{
	U2ASSERT(elem && seq);

	U2FxShaderState::Param eParam = U2FxShaderState::
		StringToParam(elem->Attribute("name"));

	U2ShaderArg* pArg = U2_NEW U2ShaderArg(type);
	if(HasAttr(elem, "value"))
	{
		switch(type)
		{
		case U2FxShaderState::Int:
			pArg->SetInt(GetIntAttr(elem, "value", 0));
			break;

		case U2FxShaderState::Float:
			pArg->SetFloat(GetFloatAttr(elem, "value", 0.0f));
			break;

		case U2FxShaderState::Float4:
			{
				U2Float4 f4 = {0.f};
				pArg->SetFloat4(GetFloat4Attr(elem, "value", f4));
			}			
			break;
		case U2FxShaderState::Texture:
			{
#ifdef UNICODE 
				const wchar_t* szFilename = ToUnicode(elem->Attribute("value"));
#else 
				const char* szFilename = elem->Attribute("value");
#endif 
				U2ASSERT(szFilename);

				U2Dx9BaseTexture* pTexIter = U2Dx9BaseTexture::GetHead();

				U2Dx9BaseTexture* pNexTex = 0;

				while(pTexIter)
				{
					// Create시 SetName을 함..
					if(pTexIter->GetName() == szFilename)
					{
						pNexTex = pTexIter;
						break;
					}

					pTexIter = pTexIter->GetNext();
				}

				if(!pNexTex)
				{
					U2FilePath fPath;				
					TCHAR fullPath[256];
					fPath.ConvertToAbs(fullPath, 256 * sizeof(TCHAR) , szFilename, TEXTURE_PATH);	

					pNexTex = U2Dx9Texture::Create(fullPath, U2Dx9Renderer::GetRenderer());			
				}

				pArg->SetTexture(pNexTex);	
			}
			break;
		default:
			FDebug("nRpXmlParser::ParseShaderState(): invalid data type '%s'!", 
				elem->Attribute("name"));
			break;	
		}
		seq->AddConstantShaderParam(eParam, *pArg);
	}
	else if(HasAttr(elem, "variable"))
	{
#ifdef UNICODE 
		const WCHAR* szVar =ToUnicode(elem->Attribute("variable"));
#else 
		const char* szVar = elem->Attribute("variable");
#endif
		seq->AddVariableShaderParam(szVar, eParam, *pArg);
	}
}

//------------------------------------------------------------------------------
/**
Parse a Phase XML element.
*/
void U2FrameXmlParser::ParseFramePhase(TiXmlElement* elem, U2FramePass* pass)
{
	U2ASSERT(elem && pass);

	U2FramePhase* pNewPhase = U2_NEW U2FramePhase;

#ifdef UNICODE 
	pNewPhase->SetName(ToUnicode(elem->Attribute("name")));
	pNewPhase->SetShaderAlias(ToUnicode(elem->Attribute("shader")));
	pNewPhase->SetSortOrder(
		ToUnicode(U2FramePhase::StrToSortingOrder(elem->Attribute("sort"))));
	pNewPhase->SetLightMode(
		ToUnicode(U2FramePhase::StrToLightMode(elem->Attribute("lightMode"))));
	if(HasAttr(elem, "technique"))
	{
		pNewPhase->SetTechnique(ToUnicode(elem->Attribute("technique")));
	}
#else 
	pNewPhase->SetName(elem->Attribute("name"));
	pNewPhase->SetShaderAlias(elem->Attribute("shader"));
	pNewPhase->SetSortOrder(U2FramePhase::StrToSortingOrder(elem->Attribute("sort")));
	pNewPhase->SetLightMode(U2FramePhase::StrToLightMode(elem->Attribute("lightMode")));
	if(HasAttr(elem, "technique"))
	{
		pNewPhase->SetTechnique(elem->Attribute("technique"));
	}
#endif

	TiXmlElement* pChild;
	for(pChild = elem->FirstChildElement("FrameSequence"); pChild; 
		pChild = pChild->NextSiblingElement("FrameSequence"))
	{
		ParseFrameSequence(pChild, pNewPhase);
	}

	pass->AddFramePhase(pNewPhase);
}

//------------------------------------------------------------------------------
/**
Parse a Sequence XML element.
*/
void U2FrameXmlParser::ParseFrameSequence(TiXmlElement* elem, 
										  U2FramePhase* phase)
{
	U2ASSERT(elem && phase);
	U2FrameSequence* pNewSeq = U2_NEW U2FrameSequence;
#ifdef UNICODE 
	pNewSeq->SetShaderAlias(ToUnicode(elem->Attribute("shader")));
#else 
	pNewSeq->SetShaderAlias(elem->Attribute("shader"));
#endif

	if(HasAttr(elem, "technique"))
	{
#ifdef UNICODE 
		pNewSeq->SetTechnique(ToUnicode(elem->Attribute("technique")));
#else 
		pNewSeq->SetTechnique(elem->Attribute("technique"));
#endif 	
	}
	pNewSeq->SetShaderUpdateEnabled(GetBoolAttr(elem, "shaderUpdates", true));
	pNewSeq->SetFirstLightAlphaEnabled(GetBoolAttr(elem, "firstLightAlpha", false));
	pNewSeq->SetModelViewProjOnlyHint(GetBoolAttr(elem, "mvpOnly", false));

	TiXmlElement* pChild;
	for(pChild = elem->FirstChildElement(); pChild; 
		pChild = pChild->NextSiblingElement())
	{
#ifdef UNICODE 
		if(pChild->Value() == U2DynString(L"Float"))
		{
			ParseShaderState(U2FxShaderState::Float, pChild, pNewSeq);
		}
		else if(pChild->Value() == U2DynString(L"Float4"))
		{
			ParseShaderState(U2FxShaderState::Float4, pChild, pNewSeq);
		}
		else if(pChild->Value() == U2DynString(L"Int"))
		{
			ParseShaderState(U2FxShaderState::Int, pChild, pNewSeq);
		}
		else if(pChild->Value() == U2DynString(L"Texture"))
		{
			ParseShaderState(U2FxShaderState::Texture, pChild, pNewSeq);
		}	
#else 
		if(pChild->Value() == U2DynString("Float"))
		{
			ParseShaderState(U2FxShaderState::Float, pChild, pNewSeq);
		}
		else if(pChild->Value() == U2DynString("Float4"))
		{
			ParseShaderState(U2FxShaderState::Float4, pChild, pNewSeq);
		}
		else if(pChild->Value() == U2DynString("Int"))
		{
			ParseShaderState(U2FxShaderState::Int, pChild, pNewSeq);
		}
		else if(pChild->Value() == U2DynString("Texture"))
		{
			ParseShaderState(U2FxShaderState::Texture, pChild, pNewSeq);
		}	
#endif 
	}
	phase->AddFrameSeq(pNewSeq);
}



	

	

	




