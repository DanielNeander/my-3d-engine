#include <U2_3D/src/U23DLibPCH.h>
#include "U2ShaderEffect.h"


U2ShaderEffect::Connection::~Connection()
{
	m_pUC = 0;
	m_pfSrc = 0;
}

U2ShaderEffect::ConnContainer::~ConnContainer()
{
	m_connArray.RemoveAll();
}

U2ShaderEffect::U2ShaderEffect(int iPassCnt)
{
	U2ASSERT(iPassCnt);

	SetPassCnt(iPassCnt);
}


U2ShaderEffect::U2ShaderEffect()
{

}


U2ShaderEffect::~U2ShaderEffect()
{
	m_uPassCnt = 0;
	m_vsArray.RemoveAll();
	m_psArray.RemoveAll();
	m_alphaStateArray.RemoveAll();
	m_vsConnect.RemoveAll();
	m_vsConnect.RemoveAll();

}


void U2ShaderEffect::SetPassCnt(int iPassCnt)
{
	U2ASSERT(iPassCnt > 0);
	m_uPassCnt = iPassCnt;

	m_vsArray.Resize(m_uPassCnt);
	m_psArray.Resize(m_uPassCnt);
	m_alphaStateArray.Resize(m_uPassCnt);
	m_vsConnect.Resize(m_uPassCnt);
	uint32 i;
	for(i=0; i < m_vsConnect.Size(); ++i)
	{
		m_vsConnect.SetElem(i, U2_NEW ConnContainer);
	}
	m_psConnect.Resize(m_uPassCnt);
	for(i=0; i < m_psConnect.Size(); ++i)
	{
		m_psConnect.SetElem(i, U2_NEW ConnContainer);
	}
	SetDefaultAlphaState();
}

void U2ShaderEffect::SetRenderState(int iPass, U2Dx9RenderStateMgr* pRSMgr, 
									bool bPrimaryEffect)
{
	if(!bPrimaryEffect || iPass > 0)
	{		
		U2Dx9AlphaStatePtr spAlphaState = m_alphaStateArray.GetElem(iPass);
		spAlphaState->SetAlphaBlending(true);
		pRSMgr->SetAlphaState(spAlphaState);

		if(U2Dx9Renderer::GetRenderer()->m_pCurrRSC)
		{
			U2AlphaState* pSave = U2Dx9Renderer::GetRenderer()->m_pCurrRSC->GetAlpha();
			if(pSave)
			{
				// 주의할 것				
				m_alphaStateArray.SetElem(iPass, pSave);
				U2Dx9Renderer::GetRenderer()->m_pCurrRSC->SetRenderState(spAlphaState);
			}
		}					
	}
}


void U2ShaderEffect::RestoreRenderState(int iPass, U2Dx9RenderStateMgr* pRSMgr,
										bool bPrimaryEffect)
{
	if(!bPrimaryEffect || iPass > 0)
	{	
		if(U2Dx9Renderer::GetRenderer()->m_pCurrRSC)
		{
			U2AlphaState* pSave = U2Dx9Renderer::GetRenderer()->m_pCurrRSC->GetAlpha();
			pRSMgr->SetAlphaState(m_alphaStateArray.GetElem(iPass));	
			if(pSave)
				m_alphaStateArray.SetElem(iPass, pSave);
		}							
	}

}

void U2ShaderEffect::ConnVSConstant(int iPass, const U2DynString &name, float *pfSrc)
{
	U2ShaderUserConstant* pUC = GetVConstant(iPass, name);

	ConnContainerPtr spConns = m_vsConnect.GetElem(iPass);

	U2ObjVec<ConnectionPtr>* pConnArray = &spConns->m_connArray;
	if(pConnArray)
	{
		Connection *pConn = U2_NEW Connection;
		pConn->m_pUC = pUC;
		pConn->m_pfSrc = pfSrc;
		pConnArray->AddElem(pConn);
	}
}

void U2ShaderEffect::ConnPSConstant(int iPass, const U2DynString &name, float *pfSrc)
{
	U2ShaderUserConstant* pUC = GetPConstant(iPass, name);

	ConnContainerPtr spConns = m_psConnect.GetElem(iPass);

	U2ObjVec<ConnectionPtr> *pConnArray = &spConns->m_connArray;
	if(pConnArray)
	{
		Connection *pConn = U2_NEW Connection;
		pConn->m_pUC = pUC;
		pConn->m_pfSrc = pfSrc;
		pConnArray->AddElem(pConn);
	}
}


void U2ShaderEffect::ConnVSConstants(int iPass)
{
	if(0 <= iPass && iPass < m_vsConnect.Size())
	{
		ConnContainerPtr spConns = m_vsConnect.GetElem(iPass);
		U2ObjVec<ConnectionPtr> *pConnArray = &spConns->m_connArray;
		for(uint32 i = 0; i < pConnArray->Size(); ++i)
		{
			Connection* pConn = pConnArray->GetElem(i);
			U2ShaderUserConstant* pUC = pConn->m_pUC;
			float* pfSrc = pConn->m_pfSrc;
			pUC->SetDataSource(pfSrc);
		}
	}
}


void U2ShaderEffect::ConnPSConstants(int iPass)
{
	if(0 <= iPass && iPass < m_psConnect.Size())
	{
		ConnContainerPtr spConns = m_psConnect.GetElem(iPass);

		U2ObjVec<ConnectionPtr> *pConnArray = &spConns->m_connArray;
		for(uint32 i = 0; i < pConnArray->Size(); ++i)
		{
			Connection* pConn = pConnArray->GetElem(i);
			U2ShaderUserConstant* pUC = pConn->m_pUC;
			float* pfSrc = pConn->m_pfSrc;
			pUC->SetDataSource(pfSrc);
		}
	}
}

bool U2ShaderEffect::SetVShader(int iPass, U2VertexShader* pVS)
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	m_vsArray.SetElem(iPass, pVS);

	U2VertexShaderPtr spVS = m_vsArray.GetElem(iPass);
	U2PixelShaderPtr spPS = m_psArray.GetElem(iPass);

	if( spVS && spPS)
	{
		U2VertexProgram* pVProgram =  spVS->GetGPUProgram();
		U2PixelProgram* pPProgram =  spPS->GetGPUProgram();
		return AreProgramsCompatable(pVProgram, pPProgram);
	}

	return true;
}

bool U2ShaderEffect::SetPShader(int iPass, U2PixelShader* pPS)
{
	U2ASSERT(0 <= iPass && iPass < m_uPassCnt);
	m_psArray.SetElem(iPass, pPS);

	U2VertexShaderPtr spVS = m_vsArray.GetElem(iPass);
	U2PixelShaderPtr spPS = m_psArray.GetElem(iPass);

	if( spVS && spPS)
	{
		U2VertexProgram* pVProgram =  spVS->GetGPUProgram();
		U2PixelProgram* pPProgram =  spPS->GetGPUProgram();
		return AreProgramsCompatable(pVProgram, pPProgram);
	}

	return true;
}


void U2ShaderEffect::SetDefaultAlphaState()
{
	// Create default alpha states. The Renderer enables this on a multieffect
	// drawing operation.  The first pass uses the default alpha state
	// (SBF_SRC_ALPHA, DBF_ONE_MINUS_SRC_ALPHA).  All other passes use
	// modulation and all are enabled.  These may be overridden by your
	// application code by accessing the state via effect->GetBlending(pass).
	m_alphaStateArray.SetSafeElem(0, U2_NEW U2AlphaState());
	m_alphaStateArray.GetElem(0)->SetAlphaBlending(true);
	for(uint32 i = 1; i < m_alphaStateArray.Size(); ++i)
	{
		m_alphaStateArray.SetSafeElem(i, U2_NEW U2AlphaState());
		m_alphaStateArray.GetElem(i)->SetAlphaBlending(true);
		m_alphaStateArray.GetElem(i)->SetSrcBlendMode(U2AlphaState::BLEND_DEST_COLOR);
		m_alphaStateArray.GetElem(i)->SetDestBlendMode(U2AlphaState::BLEND_ZERO);		
	}
}

bool U2ShaderEffect::AreProgramsCompatable(const U2VertexProgram* pVProgram, 
										   const U2PixelProgram* pPProgram)
{
	if((pVProgram && !pPProgram) || (!pVProgram && pPProgram))
		return true;

	// Vertex Shader Output and Pixel Shader Input 
	// Ensure that the output of the vertex program and the input of the
	// pixel program are compatible.  Each vertex program always has a clip
	// position output.  This is not relevant to the compatibility check.
	const U2VertexAttributes& voAttr = pVProgram->GetOutputVertAtts();
	const U2VertexAttributes& piAttr = pPProgram->GetInputVertAtts();
	return voAttr.Matches(piAttr, false, true, true, true, true, true, true,
		true, true, true);
}

void U2ShaderEffect::LoadResoures(U2Dx9Renderer* pRenderer, U2Mesh* pMesh)
{
	for(uint32 uPass = 0; uPass < m_uPassCnt; uPass++)
	{
		U2VertexProgram *pVProgram = 0;
		U2VertexShader* pVS = m_vsArray.GetElem(uPass);

		if(pVS)
			pVProgram = pVS->GetGPUProgram();		
		// Load VertexProgram	
		U2PixelShader *pPS = m_psArray.GetElem(uPass);
		// Load PixelProgram
		const uint32 uPTexCnt = GetPTextureQuantity(uPass);
		for(uint32 i = 0; i < uPTexCnt; ++i)
		{
			// Load Texture
		}

		if(pMesh && pVProgram)
		{
			const U2VertexAttributes& InputAttr = pVProgram->GetInputVertAtts();
			const U2VertexAttributes& outputAttr = pVProgram->GetOutputVertAtts();
			// LoadVBuffer

			// Load IndxBuffer
		}		
	}
}


void U2ShaderEffect::ReleaseResources(U2Dx9Renderer* pRenderer, U2Mesh* pMesh)
{


}


int U2ShaderEffect::Begin()
{
	return GetPassCnt();
}


void U2ShaderEffect::End()
{

}


void U2ShaderEffect::BeginEffect(int pass, U2Dx9Renderer* pRenerer, bool bPrimary)
{
	U2MeshData *pData = pRenerer->m_pCurrMesh->GetMeshData();

	this->SetRenderState(pass, pRenerer->m_pRenderState, bPrimary);

	pRenerer->m_uNumActiveSamplers = 0;

	// Set user shader constant
	this->ConnVSConstants(pass);
	this->ConnPSConstants(pass);

	U2VertexProgram* pVProgram = this->GetVProgram(pass);
	U2PixelProgram* pPProgram = this->GetPProgram(pass);	

	const U2VertexAttributes& inputAttr = pVProgram->GetInputVertAtts();
	const U2VertexAttributes& outputAttr = pVProgram->GetOutputVertAtts();

	// Asm Shader Load
	// EnableVetexProgram, EnablePixelProgram을 먼저 로드해야 
	// EnableTexture에 해당 데이터를 얻을 수 있으므로 
	// 순서 주의	
	
	if(pass == 0)
	{	
		pRenerer->EnableVertexProgram(pVProgram);
		pRenerer->EnablePixelProgram(pPProgram);				

		// 순서 주의 .. 
		// PackMeshData함수 안에서 VertexDeclation이 생성됨.		
		pRenerer->PackMeshData(pData, inputAttr, outputAttr);					
	}		

	if(pass == 0 // || DYNAMIC_MEsh && skinning 
		)
	{

	}


	/*pRenerer->m_pRenderState->SetVertexShader(pVProgram->m_pD3DVS);
	pRenerer->m_pRenderState->SetPixelShader(pPProgram->m_pD3DPS);
	pRenerer->m_pRenderState->SetVertexDecl(pData->GetVertexDeclaration());*/
	pRenerer->GetD3DDevice()->SetVertexShader(pVProgram->m_pD3DVS);
	pRenerer->GetD3DDevice()->SetPixelShader(pPProgram->m_pD3DPS);
	pRenerer->GetD3DDevice()->SetVertexDeclaration(pData->GetVertexDeclaration());

	// Keep track of the current sampler to be used in enabling the
	// textures.	
}

void U2ShaderEffect::SetupStage(int pass, U2Dx9Renderer* pRenerer)
{
	pRenerer->m_uCurrSampler = 0;
	const uint32 uVTextureCnt = this->GetVTextureQuantity(pass);
	uint32 uTex;
	bool bChanged = false, bMipmap = false, bNonPow2 = false;
	U2Dx9BaseTexture* pTex;
	for(uTex=0; uTex < uVTextureCnt; ++uTex)
	{
		pTex = 
			SmartPtrCast(U2Dx9BaseTexture, this->GetVTexture(pass, uTex));
		//pRenerer->m_pTexMgr->GetTexture(pTex, bChanged, bMipmap, bNonPow2);
		pRenerer->EnableTexture(pTex);
		pRenerer->m_uCurrSampler++;
	}

	const uint32 uPTextureCnt = this->GetPTextureQuantity(pass);		
	for(uTex=0; uTex < uPTextureCnt; ++uTex)
	{
		pTex = 
			SmartPtrCast(U2Dx9BaseTexture, this->GetPTexture(pass, uTex));
		//pRenerer->m_pTexMgr->GetTexture(pTex, bChanged, bMipmap, bNonPow2);
		pRenerer->EnableTexture(pTex);
		pRenerer->m_uCurrSampler++;
	}

}


void U2ShaderEffect::Commit(int pass, U2Dx9Renderer* pRenerer)
{
	pRenerer->m_pRenderState->CommitShaderConsts();

	U2MeshData *pMeshData = pRenerer->m_pCurrMesh->GetMeshData();

	HRESULT hr;

	for(uint32 i=0;i < pMeshData->GetStreamCount(); ++i)
	{
		//hr = pRenerer->GetD3DDevice()->SetStreamSource(0, pMeshData->GetVBBlock(i)
		//	->GetVB(), 0, pMeshData->GetVertexStride(i));			
		U2ASSERT(SUCCEEDED(hr));
	}

	hr = pRenerer->GetD3DDevice()->SetIndices(pMeshData->GetIB());		
	U2ASSERT(SUCCEEDED(hr));


	if(pMeshData->GetIB())
	{
		uint32 uStartIdx = 0;

		for(uint32 i=0; i < pMeshData->GetNumArrays(); ++i)
		{
			uint32 uiPrimitiveCnt;
			const uint16* pusArrayLength =
				pMeshData->GetArrayLengths();
			if(pusArrayLength)
			{
				// TriStrip 
				uiPrimitiveCnt = pusArrayLength[i] -2;
			}
			else 
				U2ASSERT(pMeshData->GetNumArrays() == 1)
				uiPrimitiveCnt = pMeshData->GetTriCount();


			hr = pRenerer->GetD3DDevice()->DrawIndexedPrimitive(pMeshData->GetPrimitiveType(), 			pMeshData->GetBaseVertexIndex(), 0, 
				pMeshData->GetVertexCount(), uStartIdx, uiPrimitiveCnt);
			U2ASSERT(SUCCEEDED(hr));

			uStartIdx += uiPrimitiveCnt + 2;
		}
	}
	else 
	{
		pRenerer->GetD3DDevice()->DrawPrimitive(pMeshData->GetPrimitiveType(), 
			pMeshData->GetBaseVertexIndex(), pMeshData->GetTriCount());
	}

}


void U2ShaderEffect::EndEffect(int pass, U2Dx9Renderer* pRenderer, bool bPrimary)
{
	pRenderer->m_uCurrSampler = 0;

	// Draw();

	// Disable Texture;
	//for(uTex=0; uTex < uVTextureCnt; ++uTex)
	//{
	//	DisableTexture();
	//}

	//for(uTex=0; uTex < uPTextureCnt; ++uTex)
	//{
	//	DisableTexture();
	//}


	/*hr = m_pD3DDevice9->SetVertexShader(0);
	U2ASSERT(SUCCEEDED(hr));	

	hr = m_pD3DDevice9->SetPixelShader(0);
	U2ASSERT(SUCCEEDED(hr));*/

	this->RestoreRenderState(pass, pRenderer->m_pRenderState, bPrimary);

}




