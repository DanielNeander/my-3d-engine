#include <U2_3D/Src/U23DLibPCH.h>
#include "U2Dx9FrameSection.h"

//-------------------------------------------------------------------------------------------------
U2FrameSection::U2FrameSection()
:m_bFrameInBegin(false),
m_pOwnerFrame(0)
{

}

//-------------------------------------------------------------------------------------------------
U2FrameSection::~U2FrameSection()
{

}

//------------------------------------------------------------------------------
/**
Validate the section object. This will invoke Validate() on all owned pass objects.
*/
void U2FrameSection::Validate()
{
	U2ASSERT(m_pOwnerFrame);

	uint32 i;
	uint32 numPass = m_framePasses.FilledSize();
	for(i=0; i < numPass; ++i)
	{
		m_framePasses[i]->SetFrame(m_pOwnerFrame);
		m_framePasses[i]->Validate();
	}
}



void U2FrameSection::RecreateScreenQuads()
{
	uint32 i;
	uint32 numPass = m_framePasses.FilledSize();
	for(i=0; i < numPass; ++i)
	{
		m_framePasses[i]->RecreateScreenQuad();
	}	
}