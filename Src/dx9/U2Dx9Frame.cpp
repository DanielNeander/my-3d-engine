#include <U2_3D/Src/U23DLibPCH.h>
#include "U2Dx9Frame.h"


U2Frame::U2Frame()
	:m_bIsOpen(false),
	m_uiFrameSeqShaderIdx(0)	
{
	
}

U2Frame::~U2Frame()
{
	Terminate();
}

bool U2Frame::OpenXml()
{
	m_xmlParser.SetFrame(this);
	if(!m_xmlParser.OpenXml())
	{
		return false;
	}
	U2ASSERT(!m_szName.IsEmpty());
	
	return true;
}

//------------------------------------------------------------------------------
/**
Open the XML document. This will just load the XML document and
initialize the shader path. The rest of the initialization happens
inside nRenderPath2::Open(). This 2-step approach is necessary to
prevent a shader initialization chicken/egg problem
*/
void U2Frame::CloseXml()
{
	m_xmlParser.CloseXml();
}

//------------------------------------------------------------------------------
/**
Open the render path. This will parse the xml file which describes
the render path and configure the render path object from it.
*/
bool U2Frame::Initialize()
{
	U2ASSERT(!m_bIsOpen);
	m_uiFrameSeqShaderIdx = 0;
 	if(!m_xmlParser.Parse())
	{
		return false;
	}
	Validate();
	m_bIsOpen = true;
	return true;
}

void U2Frame::Terminate()
{
	m_frameSections.RemoveAll();
	m_renderTargets.RemoveAll();		
	
	//for(unsigned int i = 0; i < m_frameShaders.Size(); ++i)
	//{
	//	U2_DELETE m_frameShaders[i];
	//}
	m_frameShaders.RemoveAll();
	m_varHandles.RemoveAll();

}


void U2Frame::Validate()
{
	unsigned int sectionIdx;
	unsigned int numSections = m_frameSections.FilledSize();
	for(sectionIdx = 0; sectionIdx < numSections; ++sectionIdx)
	{
		m_frameSections[sectionIdx]->SetFrame(this);
		m_frameSections[sectionIdx]->Validate();
	}
}

//-------------------------------------------------------------------------------------------------
unsigned int U2Frame::FindFrameSectionIdx(const U2DynString& str) const
{
	unsigned int i;
	unsigned int numSections = m_frameSections.FilledSize();
	for(i=0; i < numSections; ++i)
	{
		if(m_frameSections[i]->GetName() == str)
		{
			return i;
		}
	}

	return -1;
}

//------------------------------------------------------------------------------
/**
Find a shader definition index by its name. Return -1 if not found.
*/
unsigned int U2Frame::FindFrameShaderIdx(const U2DynString& szShader) const
{
	unsigned int i;
	unsigned int numShaders = m_frameShaders.FilledSize();
	for(i=0; i < numShaders; ++i)
	{
		if(m_frameShaders[i]->GetName() == szShader)
		{
			return i;
		}
	}

	return -1;	
}


//-------------------------------------------------------------------------------------------------
unsigned int U2Frame::FindRenderTargetIdx(const U2DynString& name) const
{
	unsigned int i;
	unsigned int numRTs = m_renderTargets.FilledSize();
	for(i=0; i < numRTs; ++i)
	{
		if(m_renderTargets[i]->GetName() == name)
		{
			return i;
		}
	}

	return -1;
}


//-------------------------------------------------------------------------------------------------
void U2Frame::RecreateScreenQuads()
{
	unsigned int i;
	unsigned int numSections = m_frameSections.FilledSize();
	for(i=0; i < numSections; ++i)
	{
		m_frameSections[i]->RecreateScreenQuads();
	}
}

