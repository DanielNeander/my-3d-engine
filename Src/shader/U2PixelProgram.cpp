#include <U2_3D/src/U23DLibPCH.h>
#include "U2PixelProgram.h"

//-------------------------------------------------------------------------------------------------
U2PixelProgram::U2PixelProgram(const U2DynString& szFilename)
	:m_pD3DPS(NULL)	
{
	// make name
	//U2String filename = U2String(_T("p_")) + strName + 
	//	U2String(_T("."))+ ms_strRenderType + U2String(_T(".wmsp"));

	U2FilePath fPath;		
	TCHAR fullPath[MAX_PATH];
	fPath.ConvertToAbs(fullPath, MAX_PATH * sizeof(TCHAR), szFilename.Str(), ASM_SHADER_PATH);	

	bool bLoaded = U2GpuProgram::LoadAndParse(fullPath, this);
	U2ASSERT(bLoaded);
	(void)bLoaded; 
	SetName(szFilename);


	// Check Renderer Support
	U2ASSERT(m_inputVertAtts.GetMaxColors() <= 2);
	U2ASSERT(m_inputVertAtts.GetMaxTCoords() <= 
		(int)U2Dx9Renderer::GetRenderer()->GetMaxSimultaneousTextures());
	U2ASSERT(m_samplerInfos.Size() <= 
		U2Dx9Renderer::GetRenderer()->GetMaxSimultaneousTextures());	

}

//-------------------------------------------------------------------------------------------------
U2PixelProgram::~U2PixelProgram()	
{
	SAFE_RELEASE(m_pD3DPS);
}

//-------------------------------------------------------------------------------------------------
U2PixelProgram* U2PixelProgram::Load(const U2DynString& programName)
{
	return U2_NEW U2PixelProgram(programName);
}

