#include <U2_3D/src/U23DLibPCH.h>
#include "U2VertexShader.h"
#include "U2VertexProgram.h"

U2VertexShader::U2VertexShader(const U2DynString& szShadername)
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
		m_spProgram =U2VertexProgram::Load(szShadername);		
	}

	if(m_spProgram)
	{
		FILE_LOG(logDEBUG) << _T("Vertex Shader ") << szShadername.Str() << _T(" loaded");
		OnLoadProgram();
	}
}

U2VertexShader::~U2VertexShader()
{

}

U2VertexProgram* U2VertexShader::GetGPUProgram() const
{
	return SmartPtrCast(U2VertexProgram, m_spProgram);
}

U2VertexShader::U2VertexShader()
{

}
				  
