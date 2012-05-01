#include <U2_3D/src/U23DLibPCH.h>
#include "U2Shader.h"

//-------------------------------------------------------------------------------------------------
U2Shader::U2Shader() 
{

}

//-------------------------------------------------------------------------------------------------
U2Shader::U2Shader(const U2DynString& shaderName)
	:m_szShaderName(shaderName),
	m_spProgram(0)
{

}

//-------------------------------------------------------------------------------------------------
U2Shader::~U2Shader()
{
	m_spProgram = 0;
	m_userDataArray.RemoveAll();
	m_textureArray.RemoveAll();

}

//-------------------------------------------------------------------------------------------------
const U2DynString& U2Shader::GetShaderName() const
{
	return m_szShaderName;
}

//-------------------------------------------------------------------------------------------------
uint32 U2Shader::GetTextureCnt() const
{
	return m_textureArray.Size();
}

//-------------------------------------------------------------------------------------------------
bool U2Shader::SetTexture(int idx, U2Dx9BaseTexturePtr pTex)
{
	if(pTex)
	{
		if(0 <= idx && idx < (int)m_textureArray.Size())
		{
			m_textureArray.SetElem(idx, pTex);
			return true;
		}
	}
	return false;
}

//-------------------------------------------------------------------------------------------------
bool U2Shader::SetTexture(int idx, const U2DynString& name)
{
	if(0 <= idx && idx < m_textureArray.Size())
	{
		U2Dx9BaseTexture* pTex = U2Dx9BaseTexture::GetHead();
		while(pTex)
		{
			if(pTex->GetName() == name)
			{
				m_textureArray.SetElem(idx, pTex);
				FILE_LOG(logDEBUG) << _T("Texture ") << name.Str() << _T(" Cache Loaded");
				return true;
			}

			pTex = pTex->GetNext();			
		}

		if(!pTex)
		{
			U2FilePath fPath;				
			TCHAR fullPath[MAX_PATH];
			fPath.ConvertToAbs(fullPath, MAX_PATH * sizeof(TCHAR) , name.Str(), DATA_PATH);	

			// Load Texture 
			pTex = U2Dx9Texture::Create(fullPath, 
				U2Dx9Renderer::GetRenderer());
			U2ASSERT(pTex);			
			FILE_LOG(logDEBUG) << _T("Texture ") << fullPath << _T(" Created ");
			m_textureArray.SetElem(idx, pTex);
			return true;
		}
	}	

	return false;	
}

//-------------------------------------------------------------------------------------------------
U2Dx9BaseTexturePtr U2Shader::GetTexture(uint32 idx)
{
	if(0 <= idx && idx < m_textureArray.Size())
	{
		return m_textureArray.GetElem(idx);
	}

	return 0;	
}

//-------------------------------------------------------------------------------------------------
const U2Dx9BaseTexturePtr U2Shader::GetTexture(uint32 idx) const
{
	if(0 <= idx && idx < m_textureArray.Size())
	{
		return m_textureArray.GetElem(idx);
	}

	return U2Dx9BaseTexturePtr(0);	
}

//-------------------------------------------------------------------------------------------------
U2Dx9BaseTexturePtr U2Shader::GetTexture(const U2DynString& name)
{
	U2Dx9BaseTexture* pTex = U2Dx9BaseTexture::GetHead();
	while(pTex)
	{
		if(pTex->GetName() == name)
		{
			return pTex;
		}

		pTex = pTex->GetNext();			
		
	}
	return U2Dx9BaseTexturePtr(0);	
	
}

//-------------------------------------------------------------------------------------------------
const U2Dx9BaseTexturePtr U2Shader::GetTexture(const U2DynString& name) const
{
	U2Dx9BaseTexture* pTex = U2Dx9BaseTexture::GetHead();
	while(pTex)
	{
		if(pTex->GetName() == name)
		{
			return pTex;
		}

		pTex = pTex->GetNext();			
	}

	return U2Dx9BaseTexturePtr(0);		

}

//-------------------------------------------------------------------------------------------------
void U2Shader::OnLoadProgram()
{
	int userConstantCnt = m_spProgram->GetUserConstantCnt();
	uint32 i, uChannels;
	U2ShaderUserConstant* pUC;
	for(i=0, uChannels=0; i < userConstantCnt; ++i)
	{
		pUC = m_spProgram->GetUserConstant(i);
		U2ASSERT(pUC);
		uChannels += 4 * pUC->GetRegisterCnt();
	}
	m_userDataArray.Resize(uChannels);

	for(i=0, uChannels=0; i < userConstantCnt; ++i)
	{
		pUC = m_spProgram->GetUserConstant(i);
		U2ASSERT(pUC);
		pUC->SetDataSource(&m_userDataArray.GetElem(uChannels));
		uChannels += 4* pUC->GetRegisterCnt();
	}
	// The number of textures is the number of samplers required by the
	// program.
	uint32 samplerInfoCnt = m_spProgram->GetSamplerInfoCnt();
	if(samplerInfoCnt > 0)
	{
		m_textureArray.Resize(samplerInfoCnt);
	}
}

//-------------------------------------------------------------------------------------------------
