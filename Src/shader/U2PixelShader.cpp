#include <U2_3D/src/U23DLibPCH.h>
#include "U2PixelShader.h"
#include "U2PixelProgram.h"

//-------------------------------------------------------------------------------------------------
U2PixelShader::U2PixelShader(const U2DynString& szShadername)
	:U2Shader(szShadername)	
{
	U2GpuProgram* pProgramItr = U2GpuProgram::GetHead();
	bool bFound = false;
	while(pProgramItr)
	{
		if(pProgramItr->GetName() == szShadername)
		{
			m_spProgram = pProgramItr;
			bFound = true;
			break;
		}

		pProgramItr = pProgramItr->GetNext();
	}

	if(!m_spProgram)
	{
		m_spProgram = U2PixelProgram::Load(szShadername);
	}

	if(m_spProgram)
	{
		FILE_LOG(logDEBUG) << _T("Pixel Shader ") << szShadername.Str() << _T(" loaded");
		OnLoadProgram();
	}

}

//-------------------------------------------------------------------------------------------------
U2PixelShader::~U2PixelShader()
{

}

//-------------------------------------------------------------------------------------------------
U2PixelProgram* U2PixelShader::GetGPUProgram() const
{
	return SmartPtrCast(U2PixelProgram, m_spProgram);
}

//-------------------------------------------------------------------------------------------------
U2PixelShader::U2PixelShader()
{

}

