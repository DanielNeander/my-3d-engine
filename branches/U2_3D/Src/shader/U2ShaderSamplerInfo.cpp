#include <U2_3D/src/U23DLibPCH.h>
#include "U2ShaderSamplerInfo.h"

//-------------------------------------------------------------------------------------------------
U2ShaderSamplerInfo::U2ShaderSamplerInfo(const U2String& name, 
										 SamplerType eSampler, int iTexUnit)
										 :m_strName(name)
{
	m_eSampler = eSampler;
	m_iTextureUnit = iTexUnit;

	switch(m_eSampler)
	{	
	case SAMPLER_1D:
		m_usSamplerDims = 1;
		break;
	case SAMPLER_2D:
		m_usSamplerDims = 2;
		break;
	case SAMPLER_3D:
		m_usSamplerDims = 3;
		break;
	case SAMPLER_CUBE:
		m_usSamplerDims = 2;
		break;
	case SAMPLER_PROJ:
		m_usSamplerDims = 2;
		break;
	default:
		U2ASSERT(FALSE);
		m_usSamplerDims = 0;
		break;		
	}

}

//-------------------------------------------------------------------------------------------------
U2ShaderSamplerInfo::~U2ShaderSamplerInfo()
{

}