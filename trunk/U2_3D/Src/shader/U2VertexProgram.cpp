#include <U2_3D/src/U23DLibPCH.h>
#include "U2VertexProgram.h"


U2VertexProgram::U2VertexProgram(const U2DynString& szFilename)
	:m_pD3DVS(NULL)
{
	// make name
	//U2String filename = U2String(_T("v_")) + strName + U2String(_T(".")) +
	//	ms_strRenderType + U2String(_T(".wmsp"));

	U2FilePath fPath;	
	//TCHAR szRelPath[128] = _T("texture\\RedSKy.bmp");
	TCHAR fullPath[MAX_PATH];
	fPath.ConvertToAbs(fullPath, MAX_PATH * sizeof(TCHAR) , szFilename.Str(), ASM_SHADER_PATH);	

	bool bLoaded = U2GpuProgram::LoadAndParse(fullPath, this);
	U2ASSERT(bLoaded);
	(void)bLoaded; 
	SetName(szFilename);

	// Check Renderer Support
	U2ASSERT(m_inputVertAtts.GetMaxColors() <= 2);
	U2ASSERT(m_inputVertAtts.GetMaxTCoords() <= 
		U2Dx9Renderer::GetRenderer()->GetMaxSimultaneousTextures());
	U2ASSERT(m_outputvertAtts.GetMaxColors() <= 2);
	U2ASSERT(m_outputvertAtts.GetMaxTCoords() <= 
		U2Dx9Renderer::GetRenderer()->GetMaxSimultaneousTextures());
	U2ASSERT(m_samplerInfos.Size() <= 
		U2Dx9Renderer::GetRenderer()->GetMaxSimultaneousTextures());	
}


U2VertexProgram::~U2VertexProgram()
{
	SAFE_RELEASE(m_pD3DVS);
}

U2VertexProgram* U2VertexProgram::Load(const U2DynString& programName)
{
	return U2_NEW U2VertexProgram(programName);
}

