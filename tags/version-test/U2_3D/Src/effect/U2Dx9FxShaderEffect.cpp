#include <U2_3D/src/U23DLibPCH.h>
#include "U2Dx9FxShaderEffect.h"
#include <U2_3D/Src/Main/U2SceneMgr.h>


IMPLEMENT_RTTI(U2Dx9FxShaderEffect, U2Effect);

U2Dx9FxShaderEffect::TextureNode::TextureNode()
:m_eShaderParam(U2FxShaderState::InvalidParameter),
m_spTexture(0)
{

}

U2Dx9FxShaderEffect::TextureNode::TextureNode(U2FxShaderState::Param eSahderParam, 
											  const TCHAR* szTexturename)
											  :m_eShaderParam(eSahderParam),
											  m_szTexname(szTexturename),
											  m_spTexture(0)
{

}

//-------------------------------------------------------------------------------------------------
U2Dx9FxShaderEffect::U2Dx9FxShaderEffect(U2D3DXEffectShader* pShader, U2Dx9Renderer *pRenderer)
	:m_pShader(pShader),
	m_pRenderer(pRenderer),
	m_bShaderUpdateEnabled(true),	
	m_spFrame(NULL),
	m_uiShaderIdx((uint32)-1)
{
	U2ASSERT(pShader && pRenderer);
}

//-------------------------------------------------------------------------------------------------
U2Dx9FxShaderEffect::~U2Dx9FxShaderEffect()
{
	
	m_spFrame = 0;
}

//-------------------------------------------------------------------------------------------------
void U2Dx9FxShaderEffect::Initialize()
{
	
}

//-------------------------------------------------------------------------------------------------
void U2Dx9FxShaderEffect::Render(U2Dx9Renderer* pRenderer, U2Spatial* pGlobalObj, 
					  int , int, U2VisibleObject* pVisible)
{


}

//-------------------------------------------------------------------------------------------------
void U2Dx9FxShaderEffect::AddShader(U2D3DXEffectShader* pShader)
{
	//m_d3dShaderArray.AddElem(pShader);
}

//-------------------------------------------------------------------------------------------------
U2ShaderAttribute& U2Dx9FxShaderEffect::GetShaderAttb()
{
	return m_shaderAttb;
}


//-------------------------------------------------------------------------------------------------
int U2Dx9FxShaderEffect::Begin()
{

//	U2ASSERT(0 <= m_iActiveShader && m_iActiveShader < m_d3dShaderArray.Size());
//	return m_d3dShaderArray.GetElem(m_iActiveShader)->Begin(false);
	return 1;
}

//-------------------------------------------------------------------------------------------------
void U2Dx9FxShaderEffect::End()
{

//	U2ASSERT(0 <= m_iActiveShader && m_iActiveShader < m_d3dShaderArray.Size());

//	m_d3dShaderArray.GetElem(m_iActiveShader)->End();	
}

//-------------------------------------------------------------------------------------------------
void U2Dx9FxShaderEffect::BeginEffect(int pass, U2Dx9Renderer* pRenerer, bool bPrimary)
{
//	U2ASSERT(0 <= m_iActiveShader && m_iActiveShader < m_d3dShaderArray.Size());

//	m_d3dShaderArray.GetElem(m_iActiveShader)->BeginPass(pass);	
	
}

//-------------------------------------------------------------------------------------------------
void U2Dx9FxShaderEffect::SetupStage(int pass, U2Dx9Renderer* pRenerer)
{
	
}

//-------------------------------------------------------------------------------------------------
void U2Dx9FxShaderEffect::Commit(int pass, U2Dx9Renderer* pRenerer)
{
//	m_d3dShaderArray.GetElem(m_iActiveShader)->CommitChanges();
}

//-------------------------------------------------------------------------------------------------
void U2Dx9FxShaderEffect::EndEffect(int pass, U2Dx9Renderer* pRenerer, bool bPrimary)
{

//	U2ASSERT(0 <= m_iActiveShader && m_iActiveShader < m_d3dShaderArray.Size());

//	m_d3dShaderArray.GetElem(m_iActiveShader)->EndPass();
}

//-------------------------------------------------------------------------------------------------
void U2Dx9FxShaderEffect::LoadResoures(U2Dx9Renderer* pRenderer, U2Mesh* pMesh)
{
	if(m_pShader)
	{
		const U2Frame* pFrame = U2SceneMgr::Instance()->m_spFrame;
		uint32 shaderIdx = pFrame->FindFrameShaderIdx(this->GetFrameShaderName());
		U2ASSERT((uint32)-1 != shaderIdx);
		const U2FrameShader& frameShader = pFrame->GetShader(shaderIdx);
		m_uiShaderIdx = frameShader.GetBucketIdx();

		uint32 numTexture = m_textureNodes.FilledSize();
		for(uint32 i=0; i < numTexture; ++i)
		{
			LoadTexture(i);
		}
	}	
}

//-------------------------------------------------------------------------------------------------
void U2Dx9FxShaderEffect::ReleaseResources(U2Dx9Renderer* pRenderer, U2Mesh* pMesh)
{

}

//-------------------------------------------------------------------------------------------------




bool U2Dx9FxShaderEffect::LoadTexture(uint32 idx)
{
	TextureNode& texNode = *m_textureNodes[idx];
	if(!texNode.m_spTexture && !texNode.m_szTexname.IsEmpty())
	{
		U2Dx9BaseTexture* pTex = U2Dx9BaseTexture::GetHead();
		
		while(pTex)
		{
			if(pTex->GetName() == texNode.m_szTexname)
			{
				texNode.m_spTexture = pTex;
				m_shaderAttb.SetArg(texNode.m_eShaderParam, U2ShaderArg(pTex));
				//FILE_LOG(logDEBUG) << _T("Texture ") << texNode.m_szTexname.Str() 
				//	<< _T(" Cache Loaded");
				return true;
			}

			pTex = pTex->GetNext();			
		}

		if(!pTex)
		{
			U2FilePath fPath;				
			TCHAR fullPath[MAX_PATH];
			fPath.ConvertToAbs(fullPath, MAX_PATH * sizeof(TCHAR), texNode.m_szTexname.Str(), TEXTURE_PATH);	

			// Load Texture 
			pTex = U2Dx9Texture::Create(fullPath, 
				U2Dx9Renderer::GetRenderer());
			U2ASSERT(pTex);			
			FILE_LOG(logDEBUG) << _T("Texture ") << fullPath << _T(" Created ");
			texNode.m_spTexture = pTex;
			m_shaderAttb.SetArg(texNode.m_eShaderParam, U2ShaderArg(pTex));			
			
			return true;
		}
	}

	return false;
}


void U2Dx9FxShaderEffect::UnLoadTexture(uint32 idx)
{
	TextureNode& texNode = *m_textureNodes[idx];

	texNode.m_spTexture = 0;
}

void U2Dx9FxShaderEffect::SetTexture(const TCHAR* strParam, const TCHAR* szTexturename)
{
	SetTexture(U2FxShaderState::StringToParam(strParam), szTexturename);
}

const TCHAR* U2Dx9FxShaderEffect::GetTexture(const TCHAR* strParam) const
{
	return GetTexture(U2FxShaderState::StringToParam(strParam));
}

void U2Dx9FxShaderEffect::SetInt(const TCHAR* strParam, int val)
{
	SetInt(U2FxShaderState::StringToParam(strParam), val);
}

int U2Dx9FxShaderEffect::GetInt(const TCHAR* strParam) const
{
	return GetInt(U2FxShaderState::StringToParam(strParam));
}

void U2Dx9FxShaderEffect::SetBool(const TCHAR* strParam, bool val)
{
	SetBool(U2FxShaderState::StringToParam(strParam), val);
}

bool U2Dx9FxShaderEffect::GetBool(const TCHAR* strParam) const
{
	return GetBool(U2FxShaderState::StringToParam(strParam));
}

void U2Dx9FxShaderEffect::SetFloat(const TCHAR* strParam, float val)
{
	SetFloat(U2FxShaderState::StringToParam(strParam), val);
}

float U2Dx9FxShaderEffect::GetFloat(const TCHAR* strParam) const
{
	return GetFloat(U2FxShaderState::StringToParam(strParam));
}

void U2Dx9FxShaderEffect::SetVector(const TCHAR* strParam, const D3DXVECTOR4& val)
{
	SetVector(U2FxShaderState::StringToParam(strParam), val);
}

void U2Dx9FxShaderEffect::SetVector(const TCHAR* strParam, float r, float g, float b, float a)
{
	SetVector(U2FxShaderState::StringToParam(strParam), r, g, b, a);
}

const D3DXVECTOR4& U2Dx9FxShaderEffect::GetVector(const TCHAR* strParam) const
{
	return GetVector(U2FxShaderState::StringToParam(strParam));
}

//-------------------------------------------------------------------------------------------------
void U2Dx9FxShaderEffect::SetTexture(U2FxShaderState::Param param, const TCHAR* szTexturename)
{
	U2ASSERT(szTexturename);

	if(U2FxShaderState::InvalidParameter == param)
	{
		FDebug("WARNING: invalid shader parameter in object '%s'\n", GetName());
	}

	uint32 i;
	uint32 numTextures = m_textureNodes.FilledSize();
	for(i=0; i < numTextures; ++i)
	{
		if(m_textureNodes[i]->m_eShaderParam == param)
		{
			break;
		}
	}
	if(i == numTextures)
	{
		TextureNode* pNewTexNode = U2_NEW TextureNode(param, szTexturename);
		m_textureNodes.AddElem(pNewTexNode);
	}
	else 
	{
		UnLoadTexture(i);
		m_textureNodes[i]->m_szTexname = szTexturename;
	}	
}


const TCHAR* U2Dx9FxShaderEffect::GetTexture(U2FxShaderState::Param param) const
{
	uint32 i;
	uint32 numTextures = m_textureNodes.FilledSize();
	for(i=0; i < numTextures; ++i)
	{
		if(m_textureNodes[i]->m_eShaderParam == param)
		{
			m_textureNodes[i]->m_szTexname.Str();
		}
	}

	// Invalid variable name
	return 0;
}


bool U2Dx9FxShaderEffect::HasParam(U2FxShaderState::Param param)
{
	return m_shaderAttb.IsValid(param);
}



